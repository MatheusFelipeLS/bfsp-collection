#!/usr/bin/env python3
"""Gera a tabela LaTeX (formato de table_example.txt) com o fator de aceleração
médio de cada speed up para TODOS os grupos de instâncias (n x m) presentes em
results/*.csv.

Uso: python3 table_to_latex.py [--results-dir results] [-o tabela.tex]
"""
import argparse
import csv
import os
import re
import sys
from collections import defaultdict

COLUMNS = [  # (arquivo em results/, rótulo na tabela)
    ("taillard-insertion", "NEH-Taillard"),
    ("rls-taillard", "RLS-Taillard"),
    ("rls-taillard-cutoff", "RLS-Taillard-Cutoff"),
    ("rls-grabowski-blocks", "RLS-Grabowski"),
    ("rls-grabowski", "RLS-Taillard-Grabowski"),
    ("partial-recalc", "Swap-Partial"),
]
EXCLUDED = {(200, 5), (500, 5), (500, 10)}  # classes ignoradas na tabela
INSTANCE_RE = re.compile(r"^J(\d+)M(\d+)N\d+$")


def load(path):
    """{(n, m): [fatores]}"""
    groups = defaultdict(list)
    if not os.path.exists(path):
        return groups
    with open(path, newline="") as f:
        for rec in csv.DictReader(f):
            mt = INSTANCE_RE.match(rec["instance"])
            if mt and rec["factor"]:
                groups[(int(mt[1]), int(mt[2]))].append(float(rec["factor"]))
    return groups


def fmt(x):
    return f"${x:.1f}\\times$".replace(".", ",")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--results-dir", default="results")
    ap.add_argument("-o", "--output", default="speed_ups_table.tex", help="arquivo de saída (padrão: stdout)")
    args = ap.parse_args()

    data = {name: load(os.path.join(args.results_dir, name + ".csv")) for name, _ in COLUMNS}
    classes = sorted({k for g in data.values() for k in g} - EXCLUDED)
    if not classes:
        sys.exit("nenhum resultado encontrado")

    lines = [
        r"\begin{table}[htbp]",
        r"\centering",
        r"\caption{Evolução dos fatores de aceleração médios ($S$) por classe de dimensão do problema.}",
        r"\label{tab:escalabilidade_fatores}",
        r"\resizebox{\textwidth}{!}{",
        r"\begin{tabular}{l" + "c" * len(COLUMNS) + "}",
        r"\toprule",
        r"\textbf{Classe ($n \times m$)} & "
        + " & ".join(rf"\texttt{{{label}}}" for _, label in COLUMNS)
        + r" \\ \midrule",
    ]
    prev_n = None
    for n, m in classes:
        if prev_n is not None and n != prev_n:
            lines.append(r"\midrule")  # separa os grupos por n
        prev_n = n
        cells = []
        for name, _ in COLUMNS:
            v = data[name].get((n, m))
            cells.append(fmt(sum(v) / len(v)) if v else "--")
        lines.append(rf"${n} \times {m}$ & " + " & ".join(cells) + r" \\")
    lines += [r"\bottomrule", r"\end{tabular}", "}", r"\end{table}"]

    out = "\n".join(lines) + "\n"
    if args.output:
        with open(args.output, "w") as f:
            f.write(out)
    else:
        print(out, end="")


if __name__ == "__main__":
    main()
