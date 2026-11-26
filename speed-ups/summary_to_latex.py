#!/usr/bin/env python3
"""Gera a tabela LaTeX de resumo (fator médio/mínimo/máximo por speed up),
no formato de table_example.txt, agregando todas as instâncias de results/*.csv
exceto as classes em EXCLUDED.

Uso: python3 summary_to_latex.py [--results-dir results] [-o tabela.tex]
"""
import argparse
import csv
import os
import re
import sys

ROWS = [  # (arquivo em results/, rótulo na tabela)
    ("taillard-insertion", "NEH-Taillard-Cutoff"),
    ("rls-taillard", "RLS-Taillard"),
    ("rls-taillard-cutoff", "RLS-Taillard-Cutoff"),
    ("rls-grabowski-blocks", "RLS-Grabowski"),
    ("rls-grabowski", "RLS-Taillard-Grabowski"),
    ("partial-recalc", "Swap-Partial"),
    ("hyprr-swap-lb", "Swap-Ding"),  # LB O(1) de troca (Ding et al., blocos do caminho crítico)
]
EXCLUDED = {(200, 5), (500, 5), (500, 10)}  # classes (n, m) ignoradas
INSTANCE_RE = re.compile(r"^J(\d+)M(\d+)N\d+$")


def load_factors(path):
    factors = []
    with open(path, newline="") as f:
        for rec in csv.DictReader(f):
            mt = INSTANCE_RE.match(rec["instance"])
            if mt and rec["factor"] and (int(mt[1]), int(mt[2])) not in EXCLUDED:
                factors.append(float(rec["factor"]))
    return factors


def fmt(x):
    return f"{x:.2f}".replace(".", ",")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--results-dir", default="results")
    ap.add_argument("-o", "--output", help="arquivo de saída (padrão: stdout)")
    args = ap.parse_args()

    lines = [
        r"\begin{table}[htbp]",
        r"\centering",
        r"\caption{Fatores de aceleração computacional obtidos sobre as instâncias do \textit{benchmark} de Taillard.}",
        r"\label{tab:fatores_aceleracao}",
        r"\resizebox{\textwidth}{!}{",
        r"\begin{tabular}{lccc}",
        r"\hline",
        r"\textbf{Procedimento de Aceleração} & \textbf{Fator Médio ($\bar{S}$)} & "
        r"\textbf{Fator Mínimo} & \textbf{Fator Máximo} \\ \hline",
    ]
    for name, label in ROWS:
        path = os.path.join(args.results_dir, name + ".csv")
        factors = load_factors(path) if os.path.exists(path) else []
        if not factors:
            sys.exit(f"sem resultados em {path}")
        lines.append(
            rf"\texttt{{{label}}} & {fmt(sum(factors) / len(factors))} & "
            rf"{fmt(min(factors))} & {fmt(max(factors))} \\"
        )
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
