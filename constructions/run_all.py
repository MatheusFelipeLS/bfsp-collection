#!/usr/bin/env python3
"""Roda todos os construtivos em todas as instâncias (equivalente ao run.sh),
mas em ordem algoritmo-major: um algoritmo por vez, percorrendo as instâncias.

Ordem de execução:
  - algoritmos em ordem alfabética (case-insensitive)
  - instâncias da menor para a maior, com desempate numérico por
    (nº de jobs, nº de máquinas, nº da instância) -- assim J50M20* vem antes de
    J500M20* e ...N2 vem antes de ...N10.

Grava um arquivo por algoritmo em results/<algoritmo>.txt, consumível pelo
output_to_excel.py.
"""

import argparse
import os
import re
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Espelha ALGORITHMS em src/main.cpp (omitir -a roda todos; aqui rodamos um a um).
ALGORITHMS = ["NEH", "PF", "PF_NEH", "PFT", "PFT_NEH", "LPT", "MinMax", "mNEH", "PW", "GRASP_NEH"]

BINARY = os.path.join("build", "constructions")
INSTANCES_DIR = "instances"

INSTANCE_RE = re.compile(r"^J(\d+)M(\d+)N(\d+)$")
RESULT_RE = re.compile(r"^\w+,\d+,[0-9.eE+-]+$")


def log(msg):
    print(msg, file=sys.stderr, flush=True)


def build():
    if not os.path.isdir("build"):
        log("meson setup build")
        subprocess.run(["meson", "setup", "build"], check=True)
    log("ninja -C build")
    subprocess.run(["ninja", "-C", "build"], check=True)


def discover_instances():
    """Lista os caminhos das instâncias, da menor para a maior."""
    found = []
    for group in sorted(os.listdir(INSTANCES_DIR)):
        group_dir = os.path.join(INSTANCES_DIR, group)
        if not os.path.isdir(group_dir):
            continue
        for name in os.listdir(group_dir):
            m = INSTANCE_RE.match(name)
            if not m:
                continue
            jobs, machines, number = (int(x) for x in m.groups())
            found.append(((jobs, machines, number), os.path.join(group_dir, name)))
    found.sort(key=lambda item: item[0])
    return [path for _, path in found]


def run_one(algorithm, instance_path, seed):
    cmd = ["./" + BINARY, instance_path, "-a", algorithm]
    if seed is not None:
        cmd += ["-s", str(seed)]
    proc = subprocess.run(cmd, capture_output=True, text=True)
    out = proc.stdout.strip()
    if proc.returncode != 0 or not RESULT_RE.match(out):
        log(f"  AVISO: {algorithm} {instance_path} -> rc={proc.returncode} stdout={out!r} "
            f"stderr={proc.stderr.strip()!r}")
    return out


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--no-build", action="store_true", help="não roda meson/ninja antes")
    parser.add_argument("--algorithms", help="lista separada por vírgula para sobrescrever o conjunto padrão")
    parser.add_argument("--results-dir", default="results", help="diretório de saída (default: results/)")
    parser.add_argument("--seed", type=int, help="seed passada para o binário (só afeta GRASP_NEH)")
    parser.add_argument("--dry-run", action="store_true", help="só imprime a ordem, não executa nada")
    args = parser.parse_args()

    os.chdir(SCRIPT_DIR)

    algorithms = args.algorithms.split(",") if args.algorithms else list(ALGORITHMS)
    algorithms = sorted(algorithms, key=str.lower)

    instances = discover_instances()

    if args.dry_run:
        print("Algoritmos (nesta ordem):")
        for a in algorithms:
            print(f"  {a}")
        print(f"\nInstâncias (nesta ordem, {len(instances)}):")
        for p in instances:
            print(f"  {p}")
        print(f"\nTotal de execuções: {len(algorithms) * len(instances)}")
        return

    if not args.no_build:
        build()

    if not os.path.exists(BINARY):
        sys.exit(f"binário não encontrado: {BINARY} (rode sem --no-build)")

    total = len(algorithms) * len(instances)
    done = 0
    os.makedirs(args.results_dir, exist_ok=True)
    for ai, algorithm in enumerate(algorithms, start=1):
        out_path = os.path.join(args.results_dir, f"{algorithm}.txt")
        with open(out_path, "w") as f:
            log(f"[{ai}/{len(algorithms)}] {algorithm}")
            for instance_path in instances:
                result = run_one(algorithm, instance_path, args.seed)
                f.write(f"{instance_path}\n{result}\n")
                f.flush()
                done += 1
                log(f"  {done}/{total} {instance_path} -> {result}")
            f.write("-\n")

    log(f"pronto: {done} execuções em {args.results_dir}/")


if __name__ == "__main__":
    main()
