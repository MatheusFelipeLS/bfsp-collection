#!/usr/bin/env python3
"""Gera a tabela LaTeX de resumo das heurísticas construtivas a partir de
constructions_results.xlsx (no formato de table_example.txt).

Lê as abas por algoritmo (colunas: Instância, Valor Objetivo, Tempo (ms),
Gap (%)), ignora os grupos de instâncias em IGNORED_GROUPS e calcula, por
algoritmo:
    ARPD (%)          média dos gaps
    Gap Min/Máx (%)   menor/maior gap
    Desv. Padrão (%)  desvio padrão amostral dos gaps
    Tempo Mín/Máx     menor/maior tempo (ms)

Linhas ordenadas por ARPD; o melhor valor (menor) de cada coluna fica em
negrito.
"""

import argparse
import os
import re
import statistics
import sys

from openpyxl import load_workbook

# J200M5 = 200x5, J500M5 = 500x5, J500M10 = 500x10
IGNORED_GROUPS = {"J200M5", "J500M5", "J500M10"}
INSTANCE_RE = re.compile(r"^(?P<group>J\d+M\d+)N\d+$")

COLUMNS = [
    # (cabeçalho, chave, negritar o menor?, casas decimais)
    ("ARPD médio (\\%)", "arpd", True, 3),
    ("ARPD Min (\\%)", "gmin", True, 3),
    ("ARPD Máx (\\%)", "gmax", True, 3),
    ("Desv. Padrão (\\%)", "std", True, 3),
    ("Tempo Mín (ms)", "tmin", True, 3),
    ("Tempo Máx (ms)", "tmax", True, 3),
]


def read_stats(xlsx_path):
    wb = load_workbook(xlsx_path, read_only=True, data_only=True)
    stats = []
    for ws in wb.worksheets:
        if ws.title == "Resumo":
            continue
        gaps, times = [], []
        for instance, _cost, time_ms, gap in ws.iter_rows(min_row=2, max_col=4, values_only=True):
            if instance is None:
                continue
            m = INSTANCE_RE.match(str(instance))
            if m and m.group("group") in IGNORED_GROUPS:
                continue
            if gap is None:
                print(f"warning: {ws.title}/{instance}: sem gap, ignorada", file=sys.stderr)
                continue
            gaps.append(float(gap))
            times.append(float(time_ms))
        if not gaps:
            continue
        stats.append({
            "name": ws.title,
            "n": len(gaps),
            "arpd": statistics.mean(gaps),
            "gmin": min(gaps),
            "gmax": max(gaps),
            "std": statistics.stdev(gaps) if len(gaps) > 1 else 0.0,
            "tmin": min(times),
            "tmax": max(times),
        })
    stats.sort(key=lambda s: s["arpd"])
    return stats


def fmt(value, digits):
    return f"{value:.{digits}f}".replace(".", ",")


def build_table(stats):
    best = {key: min(round(s[key], d) for s in stats) for _, key, bold, d in COLUMNS if bold}

    header = " & ".join(["\\textbf{Algoritmo}"] + [f"\\textbf{{{h}}}" for h, *_ in COLUMNS])
    name_width = max(len(s["name"].replace("_", "\\_")) for s in stats)

    lines = [
        "\\begin{table}[htbp]",
        "\\centering",
        "\\caption{Resumo do desempenho global das heurísticas construtivas no BFSP.}",
        "\\label{tab:resumo_construtivos}",
        "\\resizebox{\\textwidth}{!}{",
        "\\begin{tabular}{l" + "c" * len(COLUMNS) + "}",
        "\\toprule",
        header + " \\\\",
        "\\midrule",
    ]
    for s in stats:
        cells = [s["name"].replace("_", "\\_").ljust(name_width)]
        for _, key, bold, digits in COLUMNS:
            text = fmt(s[key], digits)
            if bold and round(s[key], digits) == best[key]:
                text = f"\\textbf{{{text}}}"
            cells.append(text)
        lines.append(" & ".join(cells) + " \\\\")
    lines += [
        "\\bottomrule",
        "\\end{tabular}",
        "}",
        "\\end{table}",
    ]
    return "\n".join(lines) + "\n"


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("xlsx", nargs="?", default=os.path.join(script_dir, "constructions_results.xlsx"),
                        help="planilha de resultados (default: ./constructions_results.xlsx)")
    parser.add_argument("--output", default=os.path.join(script_dir, "constructions_table.tex"),
                        help="arquivo .tex a gravar (default: ./constructions_table.tex)")
    args = parser.parse_args()

    stats = read_stats(args.xlsx)
    if not stats:
        sys.exit("nenhum resultado encontrado")

    table = build_table(stats)
    with open(args.output, "w") as f:
        f.write(table)

    print(table)
    counts = sorted({s["n"] for s in stats})
    print(f"wrote {args.output} ({len(stats)} algoritmos, instâncias por algoritmo: {counts}; "
          f"ignorados: {', '.join(sorted(IGNORED_GROUPS))})", file=sys.stderr)


if __name__ == "__main__":
    main()
