#!/usr/bin/env python3
"""Gera a tabela LaTeX de resumo das buscas locais plugadas no IG_IJ a partir
da aba "Resumo" de local_searchs_results.xlsx (no formato de table_example.txt).

A aba "Resumo" já vem agregada e ranqueada por RPD médio pelo
output_to_excel.py (que também descarta J200M5/J500M5/J500M10). Este script só
renomeia as entradas (DISPLAY_NAMES), formata os números no padrão pt-BR e
negrita o melhor valor das colunas em COLUMNS (menor RPD médio/mín/máx,
maior N chamadas, menor % até o melhor, maior ΔC/chamada).

Uso:
    .venv/bin/python summary_to_latex.py                      # -> local_searchs_table.tex
    .venv/bin/python summary_to_latex.py arquivo.xlsx -o t.tex
"""

import argparse
import os
import statistics
import sys

from openpyxl import load_workbook

# Nome da entrada (results/<entrada>.csv) -> nome exibido na tabela (LaTeX).
DISPLAY_NAMES = {
    "mrls-p-eda": r"\texttt{mRLS}",
    "rls": r"\texttt{RLS}",
    "ig-ij-dispatch": r"\texttt{Dispatch}",
    "hvns-sa-rls": r"\texttt{SA-RLS}",
    "hyprr-vnd": r"\texttt{VND3}",
    "rls-de-abc": r"\texttt{RLS}$_{pu}$",
    "vnd1": r"\texttt{VND1}",
    "vnd2": r"\texttt{VND2}",
    "hvns-shaking": r"\texttt{Shaking}",
    "hvns-sa-edge-insertion": r"\texttt{SA-Edge-Insertion}",
    "hvns-best-edge-insertion": r"\texttt{Best-Edge-Insertion}",
    "hvns-best-insertion": r"\texttt{Best-Insertion}",
    "hvns-best-swap": r"\texttt{Best-Swap}",
    "swap-first-ig": r"\texttt{Swap-First}",
    "svns-s-swap": r"\texttt{Referenced-Swap}",
    "best-swap-ig": r"\texttt{Best-Swap}",
}

# Coluna -> (casas decimais, critério do negrito: min, max ou None = sem negrito).
COLUMNS = [
    ("avg", 3, min),
    ("std", 3, None),
    ("min", 3, min),
    ("max", 3, min),
    ("calls", 1, max),
    ("pct", 2, min),
    ("improve", 2, max),
]

RPD_COL = 3  # coluna "RPD (%)" das abas por entrada

HEADER = (
    r"\textbf{Busca Local} & \textbf{RPD Médio (\%)} & \textbf{DP RPD (\%)} & \textbf{RPD Mín (\%)} & \textbf{RPD Máx (\%)} & "
    r"\textbf{$N_{\text{chamadas}}$ (méd)} & \textbf{\% até o Melhor} & "
    r"\textbf{$\Delta C_{\text{méd}}$/chamada} \\"
)


def display_name(entry):
    if entry in DISPLAY_NAMES:
        return DISPLAY_NAMES[entry]
    # Entrada nova sem mapeamento: tira o prefixo hvns-, o sufixo -ig e mostra em \texttt.
    name = entry.removeprefix("hvns-").removesuffix("-ig")
    print(f"aviso: '{entry}' sem nome em DISPLAY_NAMES, usando '{name}'", file=sys.stderr)
    return rf"\texttt{{{name}}}"


def fmt(value, digits):
    """Formato pt-BR: milhar com ponto, decimal com vírgula."""
    text = f"{value:,.{digits}f}"
    return text.replace(",", "X").replace(".", ",").replace("X", ".")


def rpd_stdev(wb, entry):
    """Desvio padrão amostral do RPD (%) entre as instâncias da aba da entrada."""
    rpds = [row[RPD_COL] for row in wb[entry[:31]].iter_rows(min_row=2, values_only=True)
            if row[0] and row[RPD_COL] is not None]
    return statistics.stdev(rpds) if len(rpds) > 1 else 0.0


def read_summary(xlsx_path):
    wb = load_workbook(xlsx_path, read_only=True, data_only=True)
    if "Resumo" not in wb.sheetnames:
        sys.exit(f"{xlsx_path}: aba 'Resumo' não encontrada (rode output_to_excel.py antes)")
    rows = []
    for row in wb["Resumo"].iter_rows(min_row=2, max_col=7, values_only=True):
        if row[0] is None:
            continue
        entry, avg, pmin, pmax, calls, pct, improve = row
        rows.append({"entry": entry, "avg": avg, "std": rpd_stdev(wb, entry), "min": pmin, "max": pmax,
                     "calls": calls, "pct": pct, "improve": improve})
    rows.sort(key=lambda r: r["avg"])
    return rows


def build_table(rows):
    best = {key: pick(round(r[key], digits) for r in rows) for key, digits, pick in COLUMNS if pick}
    lines = [
        r"\begin{table}[htbp]",
        r"\centering",
        r"\small",
        rf"\caption{{Desempenho comparativo global das {len(rows)} buscas locais integradas ao algoritmo "
        r"IG\_IJ no benchmark BFSP.}",
        r"\label{tab:resumo_buscas_locais}",
        r"\resizebox{\textwidth}{!}{",
        r"\begin{tabular}{l" + "c" * len(COLUMNS) + "}",
        r"\toprule",
        HEADER,
        r"\midrule",
    ]
    for r in rows:
        cells = [display_name(r["entry"])]
        for key, digits, pick in COLUMNS:
            text = fmt(r[key], digits)
            if pick and round(r[key], digits) == best[key]:
                text = rf"\textbf{{{text}}}"
            cells.append(text)
        lines.append(" & ".join(cells) + r" \\")
    lines += [r"\bottomrule", r"\end{tabular}", "}", r"\end{table}"]
    return "\n".join(lines) + "\n"


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("xlsx", nargs="?", default=os.path.join(script_dir, "local_searchs_results.xlsx"),
                        help="xlsx gerado pelo output_to_excel.py (default: ./local_searchs_results.xlsx)")
    parser.add_argument("-o", "--output", default=os.path.join(script_dir, "local_searchs_table.tex"),
                        help="arquivo .tex a gravar (default: ./local_searchs_table.tex)")
    args = parser.parse_args()

    table = build_table(read_summary(args.xlsx))
    with open(args.output, "w") as f:
        f.write(table)
    print(table, end="")
    print(f"\ntabela gravada em {args.output}", file=sys.stderr)


if __name__ == "__main__":
    main()
