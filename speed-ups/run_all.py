#!/usr/bin/env python3
"""Roda o benchmark de speed ups em várias instâncias e grava um arquivo por
speed up em results/, consumível pelo output_to_excel.py.

Para cada instância executa `./build/speed-ups <instância> -n <iters> -s <seed>`,
que emite uma linha CSV por speed up, e reparte essas linhas em
results/<speedup>.csv (colunas: instance,naive_ms,accel_ms,factor,correct,note --
sem n/m/iters).

Ordem: instâncias da menor para a maior, desempate por (jobs, máquinas, número).
"""

import argparse
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

BINARY = os.path.join("build", "speed-ups")
INSTANCES_DIR = "instances"

INSTANCE_RE = re.compile(r"^J(\d+)M(\d+)N(\d+)$")
# Colunas emitidas pelo binário, na ordem do CSV.
BINARY_COLUMNS = ["speedup", "n", "m", "iters", "naive_ms", "accel_ms", "factor", "correct", "note"]
# O que sobra por speed up depois de tirar speedup/n/m/iters.
SHEET_COLUMNS = ["instance", "naive_ms", "accel_ms", "factor", "correct", "note"]

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


def run_one(instance_path, iters, seed):
    """Retorna {speedup: [instance, naive_ms, accel_ms, factor, correct, note]}."""
    cmd = ["./" + BINARY, instance_path, "-n", str(iters), "-s", str(seed)]
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
        rows[rec["speedup"]] = [
            instance, rec["naive_ms"], rec["accel_ms"], rec["factor"], rec["correct"], rec["note"]
        ]
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--no-build", action="store_true", help="não roda meson/ninja antes")
    parser.add_argument("--all", action="store_true", help="varre todas as instâncias (default: só *N1)")
    parser.add_argument("--iters", type=int, default=10, help="repetições por speed up (default: 5)")
    parser.add_argument("--seed", type=int, default=42, help="seed do RNG (default: 42)")
    parser.add_argument("--results-dir", default="results", help="diretório de saída (default: results/)")
    parser.add_argument("-j", "--threads", type=int, default=1,
                        help="nº de execuções do binário em paralelo (default: 1). Atenção: rodar em "
                             "paralelo pode afetar as medições de tempo")
    args = parser.parse_args()
    if args.threads < 1:
        parser.error("--threads deve ser >= 1")

    os.chdir(SCRIPT_DIR)

    if not args.no_build:
        build()
    if not os.path.exists(BINARY):
        sys.exit(f"binário não encontrado: {BINARY} (rode sem --no-build)")

    instances = discover_instances(args.all)
    os.makedirs(args.results_dir, exist_ok=True)

    # speedup -> [linha, ...], preservando a ordem menor->maior das instâncias.
    per_speedup = {}
    with ThreadPoolExecutor(max_workers=args.threads) as executor:
        futures = [executor.submit(run_one, p, args.iters, args.seed) for p in instances]
        for i, (instance_path, future) in enumerate(zip(instances, futures), start=1):
            for speedup, row in future.result().items():
                per_speedup.setdefault(speedup, []).append(row)
            log(f"[{i}/{len(instances)}] {instance_path}")

    for speedup, rows in per_speedup.items():
        out_path = os.path.join(args.results_dir, f"{speedup}.csv")
        with open(out_path, "w") as f:
            f.write(",".join(SHEET_COLUMNS) + "\n")
            for row in rows:
                f.write(",".join(row) + "\n")
        log(f"  {out_path}: {len(rows)} linhas")

    log(f"pronto: {len(per_speedup)} arquivos em {args.results_dir}/")


if __name__ == "__main__":
    main()
