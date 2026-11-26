#!/usr/bin/env python3
"""Gera a tabela LaTeX de trabalho poupado (máquinas podadas e posições
descartadas, em média %), no formato de table_example.txt, sobre todas as
instâncias de results/*.csv exceto as classes em EXCLUDED.

Uso: python3 pruning_to_latex.py [--results-dir results] [-o tabela.tex]
"""
import argparse
import csv
import os
import re
import sys

ROWS = [  # (arquivo em results/, rótulo na tabela)
    ("taillard-insertion", "NEH-Taillard-Cutoff"),
    ("rls-taillard-cutoff", "RLS-Taillard-Cutoff"),
    ("rls-grabowski-blocks", "RLS-Grabowski"),
    ("rls-grabowski", "RLS-Taillard-Grabowski"),
    ("hyprr-insertion-ding", "Insertion-Ding"),  # % de movimentos de inserção podados pelo LB
    ("hyprr-insertion-ding-plus", "Insertion-Taillard-Grabowski-Ding"),  # LB + block-skip
]
EXCLUDED = {(200, 5), (500, 5), (500, 10)}  # classes (n, m) ignoradas
INSTANCE_RE = re.compile(r"^J(\d+)M(\d+)N\d+$")


def load_means(path):
    """Retorna (média de pruned_machines_pct, média de discarded_positions_pct);
    None quando a coluna está vazia."""
    cols = {"pruned_machines_pct": [], "discarded_positions_pct": []}
    with open(path, newline="") as f:
        for rec in csv.DictReader(f):
            mt = INSTANCE_RE.match(rec["instance"])
            if not mt or (int(mt[1]), int(mt[2])) in EXCLUDED:
                continue
            for key, vals in cols.items():
                if rec[key] not in ("", None):
                    vals.append(float(rec[key]))
    return tuple(sum(v) / len(v) if v else None for v in cols.values())


def fmt(x):
    return "--" if x is None else (f"{x:.2f}".replace(".", ",") + r"\%")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--results-dir", default="results")
    ap.add_argument("-o", "--output", help="arquivo de saída (padrão: stdout)")
    args = ap.parse_args()

    lines = [
        r"\begin{table}[htbp]",
        r"\centering",
        r"\caption{Métricas de trabalho computacional poupado pelas técnicas de poda e restrição de espaço de busca.}",
        r"\label{tab:podas_descartes}",
        r"\resizebox{\textwidth}{!}{",
        r"\begin{tabular}{lcc}",
        r"\hline",
        r"\textbf{Procedimento de Aceleração} & \textbf{Máquinas Podadas (Média \%)} & "
        r"\textbf{Posições Descartadas (Média \%)} \\ \hline",
    ]
    for name, label in ROWS:
        path = os.path.join(args.results_dir, name + ".csv")
        if not os.path.exists(path):
            sys.exit(f"sem resultados em {path}")
        pruned, discarded = load_means(path)
        lines.append(rf"\texttt{{{label}}} & {fmt(pruned)} & {fmt(discarded)} \\")
    lines[-1] += r" \hline"
    lines += [r"\end{tabular}", "}", r"\end{table}"]

    out = "\n".join(lines) + "\n"
    if args.output:
        with open(args.output, "w") as f:
            f.write(out)
    else:
        print(out, end="")


if __name__ == "__main__":
    main()
