#!/usr/bin/env python3
"""Gera a tabela LaTeX de tempo médio de execução (ms) por grupo de
instâncias (n x m) e por algoritmo, a partir de constructions_results.xlsx
(formato de table_example.txt).

Lê as abas por algoritmo (colunas: Instância, Valor Objetivo, Tempo (ms),
Gap (%)) e ignora os grupos em IGNORED_GROUPS.

- Linhas: grupos ordenados por n e depois m, com \\midrule entre valores de n.
- Colunas: algoritmos ordenados do maior para o menor ARPD global (mesma
  ordem da tabela de ARPD por grupo).
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


def read_results(xlsx_path):
    """Retorna ({algoritmo: {(n, m): [tempos]}}, {algoritmo: [gaps]})."""
    wb = load_workbook(xlsx_path, read_only=True, data_only=True)
    times, gaps = {}, {}
    for ws in wb.worksheets:
        if ws.title == "Resumo":
            continue
        by_group = defaultdict(list)
        algo_gaps = []
        for instance, _cost, time_ms, gap in ws.iter_rows(min_row=2, max_col=4, values_only=True):
            if instance is None:
                continue
            m = INSTANCE_RE.match(str(instance))
            if not m:
                print(f"warning: {ws.title}/{instance}: nome de instância desconhecido", file=sys.stderr)
                continue
            if f"J{m['n']}M{m['m']}" in IGNORED_GROUPS:
                continue
            if time_ms is None:
                print(f"warning: {ws.title}/{instance}: sem tempo, ignorada", file=sys.stderr)
                continue
            by_group[(int(m["n"]), int(m["m"]))].append(float(time_ms))
            if gap is not None:
                algo_gaps.append(float(gap))
        if by_group:
            times[ws.title] = by_group
            gaps[ws.title] = algo_gaps
    return times, gaps


def fmt(value):
    return f"{value:.{DIGITS}f}".replace(".", ",")


def build_table(times, gaps):
    # ordem das colunas: maior ARPD global primeiro
    overall = {a: statistics.mean(g) if g else float("inf") for a, g in gaps.items()}
    algorithms = sorted(times, key=lambda a: overall[a], reverse=True)
    groups = sorted({key for t in times.values() for key in t})

    rows = []
    for n, m in groups:
        cells = [f"${n} \\times {m}$"]
        for a in algorithms:
            values = times[a].get((n, m))
            cells.append(fmt(statistics.mean(values)) if values else "--")
        rows.append((n, cells))

    # alinha as colunas no .tex
    widths = [max(len(cells[i]) for _, cells in rows) for i in range(len(rows[0][1]))]

    header = " & ".join(["\\textbf{Grupo ($n \\times m$)}"]
                        + [f"\\textbf{{{a.replace('_', chr(92) + '_')}}}" for a in algorithms])
    lines = [
        "\\begin{table}[htbp]",
        "\\centering",
        "\\caption{Tempo médio de execução em milissegundos (ms) por grupo de dimensão ($n \\times m$).}",
        "\\label{tab:tempo_por_grupo}",
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
    parser.add_argument("--output", default=os.path.join(script_dir, "constructions_table_tempos.tex"),
                        help="arquivo .tex a gravar (default: ./constructions_table_tempos.tex)")
    args = parser.parse_args()

    times, gaps = read_results(args.xlsx)
    if not times:
        sys.exit("nenhum resultado encontrado")

    table = build_table(times, gaps)
    with open(args.output, "w") as f:
        f.write(table)

    print(table)
    print(f"wrote {args.output} ({len(times)} algoritmos; ignorados: {', '.join(sorted(IGNORED_GROUPS))})",
          file=sys.stderr)


if __name__ == "__main__":
    main()
