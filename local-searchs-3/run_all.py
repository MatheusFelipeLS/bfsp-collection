#!/usr/bin/env python3
"""Roda cada busca local do catálogo PLUGADA no loop do IG_IJ (destroy ->
reconstrução NEH -> a busca local escolhida -> aceitação estilo SA), em vez
de rodá-la isolada - ver README.md "O que está sendo medido". Grava um
arquivo por entrada em results/ (resumo por repetição) e, quando o
trajectory logging está ligado (default), um arquivo por (entrada,instância)
em trajectories/ (uma linha por chamada da busca local, só na repetição 1).

Para cada instância executa `./build/local-searchs-3 <instância> -n <iters>
-s <seed> --time-limit-ms <time_limit_ms> [--slot-time-limit-ms ...]
[--no-trajectory]`, que emite duas famílias de linha CSV (prefixo
"summary,"/"trajectory," no stdout, ver main.cpp):

- summary: uma linha por (entrada, repetição) - como o IG inteiro se saiu
  (n_calls, call_of_best, avg_improvement_per_call, final_best_cost, ...).
  Vai para results/<entrada>.csv.
- trajectory: uma linha por chamada da busca local (repetição 1 apenas,
  até --trajectory-max-rows). Vai para trajectories/<entrada>__<instância>.csv.

Ordem: instâncias da menor para a maior, desempate por (jobs, máquinas, número).
"""

import argparse
import os
import re
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

BINARY = os.path.join("build", "local-searchs-3")
INSTANCES_DIR = "instances"

INSTANCE_RE = re.compile(r"^J(\d+)M(\d+)N(\d+)$")

SUMMARY_COLUMNS = [
    "record", "localsearch", "n", "m", "iteration", "iters", "seed", "initial_cost", "final_best_cost", "n_calls",
    "call_of_best", "avg_improvement_per_call", "time_ms", "time_limit_ms", "note",
]
# O que sobra por entrada depois de tirar record/localsearch/n/m/iters/seed.
SUMMARY_SHEET_COLUMNS = [
    "instance", "iteration", "initial_cost", "final_best_cost", "n_calls", "call_of_best",
    "avg_improvement_per_call", "time_ms", "time_limit_ms", "note",
]

TRAJECTORY_COLUMNS = [
    "record", "localsearch", "call_index", "cost_before_ls", "cost_after_ls", "accepted", "current_cost",
    "best_cost", "elapsed_ms",
]
TRAJECTORY_SHEET_COLUMNS = [
    "call_index", "cost_before_ls", "cost_after_ls", "accepted", "current_cost", "best_cost", "elapsed_ms",
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
    """Retorna (summary_rows, trajectory_rows):
    - summary_rows: {localsearch: [[instance, iteration, initial_cost, final_best_cost, n_calls, call_of_best,
      avg_improvement_per_call, time_ms, time_limit_ms, note], ...]}
    - trajectory_rows: {localsearch: [[call_index, cost_before_ls, cost_after_ls, accepted, current_cost,
      best_cost, elapsed_ms], ...]} (repetição 1 apenas, vindo direto do binário)
    """
    cmd = ["./" + BINARY, instance_path, "-n", str(iters), "-s", str(seed),
           "--time-limit-ms", str(time_limit_ms)] + extra_args
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        log(f"  AVISO: {instance_path} -> rc={proc.returncode} stderr={proc.stderr.strip()!r}")
        return {}, {}
    instance = os.path.basename(instance_path)
    summary_rows = {}
    trajectory_rows = {}
    for line in proc.stdout.strip().splitlines():
        if line.startswith("summary,"):
            parts = line.split(",", len(SUMMARY_COLUMNS) - 1)
            if len(parts) != len(SUMMARY_COLUMNS):
                continue
            rec = dict(zip(SUMMARY_COLUMNS, parts))
            summary_rows.setdefault(rec["localsearch"], []).append([
                instance, rec["iteration"], rec["initial_cost"], rec["final_best_cost"], rec["n_calls"],
                rec["call_of_best"], rec["avg_improvement_per_call"], rec["time_ms"], rec["time_limit_ms"],
                rec["note"],
            ])
        elif line.startswith("trajectory,"):
            parts = line.split(",", len(TRAJECTORY_COLUMNS) - 1)
            if len(parts) != len(TRAJECTORY_COLUMNS):
                continue
            rec = dict(zip(TRAJECTORY_COLUMNS, parts))
            trajectory_rows.setdefault(rec["localsearch"], []).append([
                rec["call_index"], rec["cost_before_ls"], rec["cost_after_ls"], rec["accepted"],
                rec["current_cost"], rec["best_cost"], rec["elapsed_ms"],
            ])
    return summary_rows, trajectory_rows


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--no-build", action="store_true", help="não roda meson/ninja antes")
    parser.add_argument("--all", action="store_true", help="varre todas as instâncias (default: só *N1)")
    parser.add_argument("--iters", type=int, default=5, help="repetições por (entrada, instância) (default: 5)")
    parser.add_argument("--seed", type=int, default=42, help="seed do RNG / solução PF_NEH compartilhada (default: 42)")
    parser.add_argument("--results-dir", default="results", help="diretório de saída dos resumos (default: results/)")
    parser.add_argument("--trajectories-dir", default="trajectories",
                        help="diretório de saída das trajetórias (default: trajectories/)")
    parser.add_argument("--time-limit-ms", type=float, default=2000.0,
                        help="orçamento de tempo do loop IG inteiro, compartilhado por TODA entrada (default: 2000ms)")
    parser.add_argument("--slot-time-limit-ms", type=float, default=None,
                        help="repassado ao binário --slot-time-limit-ms (default do binário: 20ms) - orçamento por "
                             "CHAMADA das entradas auto-orçadas (hvns-shaking, hvns-sa-*, tpa-sa)")
    parser.add_argument("--no-trajectory", action="store_true",
                        help="não grava trajectories/ (sweep mais rápido e leve)")
    parser.add_argument("--trajectory-max-rows", type=int, default=None,
                        help="repassado ao binário --trajectory-max-rows (default do binário: 2000)")
    args = parser.parse_args()

    os.chdir(SCRIPT_DIR)

    if not args.no_build:
        build()
    if not os.path.exists(BINARY):
        sys.exit(f"binário não encontrado: {BINARY} (rode sem --no-build)")

    extra_args = []
    if args.no_trajectory:
        extra_args.append("--no-trajectory")
    if args.slot_time_limit_ms is not None:
        extra_args += ["--slot-time-limit-ms", str(args.slot_time_limit_ms)]
    if args.trajectory_max_rows is not None:
        extra_args += ["--trajectory-max-rows", str(args.trajectory_max_rows)]

    instances = discover_instances(args.all)
    os.makedirs(args.results_dir, exist_ok=True)
    if not args.no_trajectory:
        os.makedirs(args.trajectories_dir, exist_ok=True)

    # entrada -> [linha, ...], preservando a ordem menor->maior das instâncias.
    per_entry_summary = {}
    # (entrada, instância) -> [linha, ...]
    per_entry_instance_trajectory = {}
    for i, instance_path in enumerate(instances, start=1):
        log(f"[{i}/{len(instances)}] {instance_path}")
        instance = os.path.basename(instance_path)
        summary_rows, trajectory_rows = run_one(instance_path, args.iters, args.seed, args.time_limit_ms, extra_args)
        for entry, rows in summary_rows.items():
            per_entry_summary.setdefault(entry, []).extend(rows)
        for entry, rows in trajectory_rows.items():
            per_entry_instance_trajectory[(entry, instance)] = rows

    for entry, rows in per_entry_summary.items():
        out_path = os.path.join(args.results_dir, f"{entry}.csv")
        with open(out_path, "w") as f:
            f.write(",".join(SUMMARY_SHEET_COLUMNS) + "\n")
            for row in rows:
                f.write(",".join(row) + "\n")
        log(f"  {out_path}: {len(rows)} linhas")

    for (entry, instance), rows in per_entry_instance_trajectory.items():
        out_path = os.path.join(args.trajectories_dir, f"{entry}__{instance}.csv")
        with open(out_path, "w") as f:
            f.write(",".join(TRAJECTORY_SHEET_COLUMNS) + "\n")
            for row in rows:
                f.write(",".join(row) + "\n")

    log(f"pronto: {len(per_entry_summary)} arquivos de resumo em {args.results_dir}/"
        + ("" if args.no_trajectory else f", {len(per_entry_instance_trajectory)} trajetórias em {args.trajectories_dir}/"))


if __name__ == "__main__":
    main()
