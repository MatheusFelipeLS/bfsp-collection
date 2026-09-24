#!/usr/bin/env python3
"""Roda o benchmark de buscas locais em várias instâncias e grava um arquivo
por entrada em results/, consumível pelo output_to_excel.py.

Esta é a variante "comparação justa" (ver README.md): TODA entrada roda sob o
MESMO orçamento de tempo (--time-limit-ms, repassado ao binário), em vez de
algumas convergirem em uma única passada e outras (svns-d-*/hvns-*/tpa-sa)
consumirem um orçamento próprio - às vezes maior, às vezes dependente do
tamanho da instância, como acontecia em local-searchs/.

Para cada instância executa `./build/local-searchs-fair <instância> -n <iters>
-s <seed> --time-limit-ms <time_limit_ms>`, que emite, por busca local, uma
linha CSV POR REPETIÇÃO (todas partindo da MESMA solução inicial aleatória,
gerada com a seed dada) - ou seja, `iters` linhas por (instância, entrada),
sem nenhuma agregação. Essas linhas são repartidas em results/<entrada>.csv
(colunas: instance,iteration,initial_cost,final_cost,improvement_abs,
improvement_pct,time_ms,time_limit_ms,note -- sem n/m/iters), um arquivo por
entrada com `iters` linhas por instância. A agregação (médias, RPD) fica por
conta do output_to_excel.py.

Ordem: instâncias da menor para a maior, desempate por (jobs, máquinas, número).
"""

import argparse
import os
import re
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

BINARY = os.path.join("build", "local-searchs-fair")
INSTANCES_DIR = "instances"

INSTANCE_RE = re.compile(r"^J(\d+)M(\d+)N(\d+)$")
# Colunas emitidas pelo binário, na ordem do CSV.
BINARY_COLUMNS = [
    "localsearch", "n", "m", "iteration", "iters", "initial_cost", "final_cost",
    "improvement_abs", "improvement_pct", "time_ms", "time_limit_ms", "note",
]
# O que sobra por entrada depois de tirar localsearch/n/m/iters.
SHEET_COLUMNS = [
    "instance", "iteration", "initial_cost", "final_cost", "improvement_abs", "improvement_pct", "time_ms",
    "time_limit_ms", "note",
]

# Subconjunto representativo (1 instância por grupo de tamanho). Use --all para varrer todas.
DEFAULT_PICK = "N1"


def log(msg):
    print(msg, file=sys.stderr, flush=True)


def build():
    if not os.path.isdir("build"):
        log("meson setup build")
        subprocess.run(["meson", "setup", "build"], check=True)
    log("ninja -C build")
    subprocess.run(["ninja", "-C", "build"], check=True)


def discover_instances(pick_all):
    found = []
    for group in sorted(os.listdir(INSTANCES_DIR)):
        group_dir = os.path.join(INSTANCES_DIR, group)
        if not os.path.isdir(group_dir):
            continue
        for name in os.listdir(group_dir):
            m = INSTANCE_RE.match(name)
            if not m:
                continue
            if not pick_all and not name.endswith(DEFAULT_PICK):
                continue
            jobs, machines, number = (int(x) for x in m.groups())
            found.append(((jobs, machines, number), os.path.join(group_dir, name)))
    found.sort(key=lambda item: item[0])
    return [path for _, path in found]


def run_one(instance_path, iters, seed, time_limit_ms, extra_args):
    """Retorna {localsearch: [[instance, iteration, initial_cost, final_cost,
    improvement_abs, improvement_pct, time_ms, time_limit_ms, note], ...]} -
    uma linha por repetição (iters linhas por entrada)."""
    cmd = ["./" + BINARY, instance_path, "-n", str(iters), "-s", str(seed),
           "--time-limit-ms", str(time_limit_ms)] + extra_args
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        log(f"  AVISO: {instance_path} -> rc={proc.returncode} stderr={proc.stderr.strip()!r}")
        return {}
    instance = os.path.basename(instance_path)
    rows = {}
    for line in proc.stdout.strip().splitlines():
        parts = line.split(",", len(BINARY_COLUMNS) - 1)
        if len(parts) != len(BINARY_COLUMNS):
            continue
        rec = dict(zip(BINARY_COLUMNS, parts))
        rows.setdefault(rec["localsearch"], []).append([
            instance, rec["iteration"], rec["initial_cost"], rec["final_cost"], rec["improvement_abs"],
            rec["improvement_pct"], rec["time_ms"], rec["time_limit_ms"], rec["note"],
        ])
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--no-build", action="store_true", help="não roda meson/ninja antes")
    parser.add_argument("--all", action="store_true", help="varre todas as instâncias (default: só *N1)")
    parser.add_argument("--iters", type=int, default=5, help="repetições por busca local (default: 5)")
    parser.add_argument("--seed", type=int, default=42, help="seed do RNG / solução inicial compartilhada (default: 42)")
    parser.add_argument("--results-dir", default="results", help="diretório de saída (default: results/)")
    parser.add_argument("--time-limit-ms", type=float, default=2000.0,
                        help="orçamento de tempo compartilhado por TODA entrada (default: 2000ms, repassado ao "
                             "binário's --time-limit-ms)")
    args = parser.parse_args()

    os.chdir(SCRIPT_DIR)

    if not args.no_build:
        build()
    if not os.path.exists(BINARY):
        sys.exit(f"binário não encontrado: {BINARY} (rode sem --no-build)")

    instances = discover_instances(args.all)
    os.makedirs(args.results_dir, exist_ok=True)

    # entrada -> [linha, ...], preservando a ordem menor->maior das instâncias.
    per_entry = {}
    for i, instance_path in enumerate(instances, start=1):
        log(f"[{i}/{len(instances)}] {instance_path}")
        for entry, rows in run_one(instance_path, args.iters, args.seed, args.time_limit_ms, []).items():
            per_entry.setdefault(entry, []).extend(rows)

    for entry, rows in per_entry.items():
        out_path = os.path.join(args.results_dir, f"{entry}.csv")
        with open(out_path, "w") as f:
            f.write(",".join(SHEET_COLUMNS) + "\n")
            for row in rows:
                f.write(",".join(row) + "\n")
        log(f"  {out_path}: {len(rows)} linhas")

    log(f"pronto: {len(per_entry)} arquivos em {args.results_dir}/")


if __name__ == "__main__":
    main()
