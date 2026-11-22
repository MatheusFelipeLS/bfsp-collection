#!/usr/bin/env python3
"""Gera a tabela LaTeX de ARPD médio por grupo de instâncias (n x m) e por
algoritmo, a partir de constructions_results.xlsx (formato de
table_example.txt).

Lê as abas por algoritmo (colunas: Instância, Valor Objetivo, Tempo (ms),
Gap (%)) e ignora os grupos em IGNORED_GROUPS.

- Linhas: grupos ordenados por n e depois m, com \\midrule entre valores de n.
- Colunas: algoritmos ordenados do maior para o menor ARPD global.
- O menor ARPD de cada grupo (linha) fica em negrito; empates também.
"""

import argparse
import os
import re
import statistics
import sys
from collections import defaultdict

from openpyxl import load_workbook

# J200M5 = 200x5, J500M5 = 500x5, J500M10 = 500x10
IGNORED_GROUPS = {"J200M5", "J500M5", "J500M10"}
DIGITS = 3  # casas decimais dos valores
INSTANCE_RE = re.compile(r"^J(?P<n>\d+)M(?P<m>\d+)N\d+$")


def read_gaps(xlsx_path):
    """Retorna {algoritmo: {(n, m): [gaps]}}."""
    wb = load_workbook(xlsx_path, read_only=True, data_only=True)
    gaps = {}
    for ws in wb.worksheets:
        if ws.title == "Resumo":
            continue
        by_group = defaultdict(list)
        for instance, _cost, _time, gap in ws.iter_rows(min_row=2, max_col=4, values_only=True):
            if instance is None:
                continue
            m = INSTANCE_RE.match(str(instance))
            if not m:
                print(f"warning: {ws.title}/{instance}: nome de instância desconhecido", file=sys.stderr)
                continue
            if f"J{m['n']}M{m['m']}" in IGNORED_GROUPS:
                continue
            if gap is None:
                print(f"warning: {ws.title}/{instance}: sem gap, ignorada", file=sys.stderr)
                continue
            by_group[(int(m["n"]), int(m["m"]))].append(float(gap))
        if by_group:
            gaps[ws.title] = by_group
    return gaps


def fmt(value, bold=False):
    text = f"{value:.{DIGITS}f}".replace(".", ",")
    if text.startswith("-") and float(text[1:].replace(",", ".")) == 0:
        text = text[1:]
    return f"\\textbf{{{text}}}" if bold else text


def build_table(gaps):
    # ordem das colunas: maior ARPD global primeiro
    overall = {a: statistics.mean(g for gs in groups.values() for g in gs) for a, groups in gaps.items()}
    algorithms = sorted(gaps, key=lambda a: overall[a], reverse=True)
    groups = sorted({key for g in gaps.values() for key in g})

    rows = []
    for n, m in groups:
        cells = [f"${n} \\times {m}$"]
        means = {a: statistics.mean(gaps[a][(n, m)]) for a in algorithms if gaps[a].get((n, m))}
        best = min(round(v, DIGITS) for v in means.values())
        for a in algorithms:
            if a in means:
                cells.append(fmt(means[a], bold=round(means[a], DIGITS) == best))
            else:
                cells.append("--")
        rows.append((n, cells))

    # alinha as colunas no .tex
    widths = [max(len(cells[i]) for _, cells in rows) for i in range(len(rows[0][1]))]

    header = " & ".join(["\\textbf{Grupo ($n \\times m$)}"]
                        + [f"\\textbf{{{a.replace('_', chr(92) + '_')}}}" for a in algorithms])
    lines = [
        "\\begin{table}[htbp]",
        "\\centering",
        "\\caption{Desvio Percentual Relativo Médio (ARPD \\%) por grupo de dimensão ($n \\times m$).}",
        "\\label{tab:arpd_por_grupo}",
        "\\begin{scriptsize}",
        "\\resizebox{\\textwidth}{!}{",
        "\\begin{tabular}{" + "c" * (len(algorithms) + 1) + "}",
        "\\toprule",
        header + " \\\\",
        "\\midrule",
    ]
    previous_n = None
    for n, cells in rows:
        if previous_n is not None and n != previous_n:
            lines.append("\\midrule")
        previous_n = n
        padded = [c.ljust(w) if i < len(cells) - 1 else c for i, (c, w) in enumerate(zip(cells, widths))]
        lines.append(" & ".join(padded) + " \\\\")
    lines += [
        "\\bottomrule",
        "\\end{tabular}",
        "}",
        "\\end{scriptsize}",
        "\\end{table}",
    ]
    return "\n".join(lines) + "\n"


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("xlsx", nargs="?", default=os.path.join(script_dir, "constructions_results.xlsx"),
                        help="planilha de resultados (default: ./constructions_results.xlsx)")
    parser.add_argument("--output", default=os.path.join(script_dir, "constructions_table_grupos.tex"),
                        help="arquivo .tex a gravar (default: ./constructions_table_grupos.tex)")
    args = parser.parse_args()

    gaps = read_gaps(args.xlsx)
    if not gaps:
        sys.exit("nenhum resultado encontrado")

    table = build_table(gaps)
    with open(args.output, "w") as f:
        f.write(table)

    print(table)
    print(f"wrote {args.output} ({len(gaps)} algoritmos; ignorados: {', '.join(sorted(IGNORED_GROUPS))})",
          file=sys.stderr)


if __name__ == "__main__":
    main()
