#!/usr/bin/env python3
"""Gera a tabela LaTeX de trabalho poupado por classe (n x m), no formato de
table_example.txt: máquinas podadas (NEH-Taillard-Cutoff, RLS-Taillard-Cutoff,
RLS-Grabowski, RLS-Taillard-Grabowski) e posições descartadas
(RLS-Grabowski / RLS-Taillard-Grabowski e Swap-Ding).
Cada célula é a média das instâncias da classe em results/*.csv; as classes em
EXCLUDED são ignoradas.

Uso: python3 pruning_by_class_to_latex.py [--results-dir results] [-o tabela.tex]
"""
import argparse
import csv
import os
import re
import sys
from collections import defaultdict

# (arquivo em results/, coluna do CSV) na ordem das colunas da tabela
COLUMNS = [
    ("taillard-insertion", "pruned_machines_pct"),
    ("rls-taillard-cutoff", "pruned_machines_pct"),
    ("rls-grabowski-blocks", "pruned_machines_pct"),
    ("rls-grabowski", "pruned_machines_pct"),
    ("rls-grabowski", "discarded_positions_pct"),
    ("hyprr-swap-lb", "discarded_positions_pct"),
]
EXCLUDED = {(200, 5), (500, 5), (500, 10)}  # classes (n, m) ignoradas
INSTANCE_RE = re.compile(r"^J(\d+)M(\d+)N\d+$")


def load(path, column):
    """{(n, m): [valores]}"""
    groups = defaultdict(list)
    if not os.path.exists(path):
        sys.exit(f"sem resultados em {path}")
    with open(path, newline="") as f:
        for rec in csv.DictReader(f):
            mt = INSTANCE_RE.match(rec["instance"])
            if mt and rec[column] not in ("", None):
                groups[(int(mt[1]), int(mt[2]))].append(float(rec[column]))
    return groups


def fmt(x):
    return "--" if x is None else (f"{x:.2f}".replace(".", ",") + r"\%")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--results-dir", default="results")
    ap.add_argument("-o", "--output", help="arquivo de saída (padrão: stdout)")
    args = ap.parse_args()

    data = [load(os.path.join(args.results_dir, f + ".csv"), col) for f, col in COLUMNS]
    classes = sorted({k for g in data for k in g} - EXCLUDED)
    if not classes:
        sys.exit("nenhum resultado encontrado")

    lines = [
        r"\begin{table}[htbp]",
        r"\centering",
        r"\caption{Percentual médio de trabalho poupado por classe de dimensão ($n \times m$).}",
        r"\label{tab:escalabilidade_podas}",
        r"\resizebox{\textwidth}{!}{",
        r"\begin{tabular}{l" + "c" * len(COLUMNS) + "}",
        r"\hline",
        r"\textbf{Classe ($n \times m$)} & \multicolumn{4}{c}{\textbf{Máquinas Podadas (\%)}} & "
        r"\multicolumn{2}{c}{\textbf{Posições Descartadas (\%)}} \\ \cline{2-7}",
        r" & \texttt{NEH-Taillard-Cutoff} & \texttt{RLS-Taillard-Cutoff} & \texttt{RLS-Grabowski} & "
        r"\texttt{RLS-Taillard-Grabowski} & \texttt{RLS-Grabowski} / \texttt{RLS-Taillard-Grabowski} & "
        r"\texttt{Swap-Ding} \\ \hline",
    ]
    for n, m in classes:
        cells = []
        for g in data:
            v = g.get((n, m))
            cells.append(fmt(sum(v) / len(v)) if v else "--")
        lines.append(rf"${n} \times {m}$ & " + " & ".join(cells) + r" \\")
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
