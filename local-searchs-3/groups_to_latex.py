#!/usr/bin/env python3
"""Gera uma tabela LaTeX com o RPD médio (%) de cada busca local por grupo de
instâncias (J<n>M<m>, ex.: J20M5 agrupa J20M5N1..J20M5N10).

Lê as abas por entrada de local_searchs_results.xlsx (geradas pelo
output_to_excel.py, que já descarta J200M5/J500M5/J500M10). Por padrão as
linhas são os grupos (ordenados por n e depois m) e as colunas as buscas locais
(ordenadas pelo RPD médio global da aba "Resumo"); a última linha traz a média
geral. O menor RPD de cada grupo fica em negrito, só quando
é único (empates no melhor ficam sem negrito). Com --transpose, linhas e
colunas são trocadas.

Uso:
    .local-searchs-3/bin/python groups_to_latex.py                       # -> local_searchs_groups_table.tex
    .local-searchs-3/bin/python groups_to_latex.py arquivo.xlsx -o t.tex --transpose
"""

import argparse
import os
import re
import sys

from openpyxl import load_workbook

from summary_to_latex import display_name, fmt

DIGITS = 3
GROUP_RE = re.compile(r"^J(\d+)M(\d+)")
RPD_COL = 3  # coluna "RPD (%)" das abas por entrada


def read_groups(xlsx_path):
    """Retorna (entradas na ordem do Resumo, grupos ordenados, {entrada: {grupo: rpd médio}})."""
    wb = load_workbook(xlsx_path, read_only=True, data_only=True)
    if "Resumo" not in wb.sheetnames:
        sys.exit(f"{xlsx_path}: aba 'Resumo' não encontrada (rode output_to_excel.py antes)")
    entries = [row[0] for row in wb["Resumo"].iter_rows(min_row=2, max_col=1, values_only=True) if row[0]]

    data, groups = {}, set()
    for entry in entries:
        sums = {}
        for row in wb[entry[:31]].iter_rows(min_row=2, values_only=True):
            if not row[0] or row[RPD_COL] is None:
                continue
            m = GROUP_RE.match(row[0])
            if not m:
                print(f"aviso: instância '{row[0]}' fora do padrão J<n>M<m>, ignorada", file=sys.stderr)
                continue
            group = (int(m.group(1)), int(m.group(2)))
            groups.add(group)
            acc = sums.setdefault(group, [0.0, 0])
            acc[0] += row[RPD_COL]
            acc[1] += 1
        data[entry] = {g: s / n for g, (s, n) in sums.items()}

    groups = sorted(groups)
    for entry in entries:
        missing = [g for g in groups if g not in data[entry]]
        if missing:
            names = ", ".join(f"J{n}M{m}" for n, m in missing)
            print(f"aviso: '{entry}' sem resultados para {names}", file=sys.stderr)
    return entries, groups, data


def group_name(group):
    n, m = group
    return f"{n}$\\times${m}"


def cell(value, best):
    if value is None:
        return "--"
    text = fmt(value, DIGITS)
    return rf"\textbf{{{text}}}" if round(value, DIGITS) == best else text


def build_table(entries, groups, data, transpose):
    # Média geral de cada entrada = média dos grupos (cada grupo tem o mesmo nº de instâncias).
    overall = {}
    for e in entries:
        vals = [data[e][g] for g in groups if g in data[e]]
        overall[e] = sum(vals) / len(vals) if vals else None

    def best_of(values):
        # Negrito só quando o menor valor é único (empate no melhor -> sem negrito).
        values = [round(v, DIGITS) for v in values if v is not None]
        if not values or values.count(min(values)) > 1:
            return None
        return min(values)

    best_group = {g: best_of(data[e].get(g) for e in entries) for g in groups}
    best_overall = best_of(overall.values())

    if not transpose:
        # Linhas = grupos, colunas = buscas locais.
        col_spec = "l" + "c" * len(entries)
        header = r"\textbf{$n \times m$} & " + " & ".join(
            rf"\rotatebox{{90}}{{{display_name(e)}}}" for e in entries) + r" \\"
        # \midrule ao fim de cada bloco de mesmo n (20x*, 50x*, ...) e antes da média.
        body = []
        for i, g in enumerate(groups):
            body.append(" & ".join([group_name(g)] + [cell(data[e].get(g), best_group[g]) for e in entries]) + r" \\")
            if i == len(groups) - 1 or groups[i + 1][0] != g[0]:
                body.append(r"\midrule")
        body.append(" & ".join([r"\textbf{Média}"] + [cell(overall[e], best_overall) for e in entries]) + r" \\")
    else:
        # Linhas = buscas locais, colunas = grupos.
        col_spec = "l" + "c" * (len(groups) + 1)
        header = r"\textbf{Busca Local} & " + " & ".join(
            rf"\textbf{{{group_name(g)}}}" for g in groups) + r" & \textbf{Média} \\"
        body = [
            " & ".join([display_name(e)] + [cell(data[e].get(g), best_group[g]) for g in groups]
                       + [cell(overall[e], best_overall)]) + r" \\"
            for e in entries
        ]

    lines = [
        r"\begin{table}[htbp]",
        r"\centering",
        r"\small",
        rf"\caption{{RPD médio (\%) por grupo de instâncias ($n \times m$) das {len(entries)} buscas locais "
        r"integradas ao algoritmo IG\_IJ no benchmark BFSP. Em negrito, o menor RPD de cada grupo.}",
        r"\label{tab:buscas_locais_por_grupo}",
        r"\resizebox{\textwidth}{!}{",
        rf"\begin{{tabular}}{{{col_spec}}}",
        r"\toprule",
        header,
        r"\midrule",
        *body,
        r"\bottomrule",
        r"\end{tabular}",
        "}",
        r"\end{table}",
    ]
    return "\n".join(lines) + "\n"


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("xlsx", nargs="?", default=os.path.join(script_dir, "local_searchs_results.xlsx"),
                        help="xlsx gerado pelo output_to_excel.py (default: ./local_searchs_results.xlsx)")
    parser.add_argument("-o", "--output", default=os.path.join(script_dir, "local_searchs_groups_table.tex"),
                        help="arquivo .tex a gravar (default: ./local_searchs_groups_table.tex)")
    parser.add_argument("--transpose", action="store_true",
                        help="linhas = buscas locais, colunas = grupos (default: o contrário)")
    args = parser.parse_args()

    table = build_table(*read_groups(args.xlsx), transpose=args.transpose)
    with open(args.output, "w") as f:
        f.write(table)
    print(table, end="")
    print(f"\ntabela gravada em {args.output}", file=sys.stderr)


if __name__ == "__main__":
    main()
