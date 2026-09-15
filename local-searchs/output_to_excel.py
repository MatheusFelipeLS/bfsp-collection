#!/usr/bin/env python3
"""Converte os results/<entrada>.csv gerados pelo run_all.py num único
output.xlsx.

Uma aba "Resumo" no início agrega a melhoria por entrada (média/min/max %,
tempo médio, nº de instâncias). Depois, uma aba por entrada, na ordem do
catálogo do README (#1..#23), com:

    Instância | Custo inicial | Custo final | Melhoria absoluta
             | Melhoria (%) | Tempo (ms) | Nota

Formato de results/<entrada>.csv (cabeçalho + uma linha por instância):
    instance,initial_cost,final_cost,improvement_abs,improvement_pct,time_ms,note
    J20M5N1,1828,1447,381,20.8425,0.058863,
"""

import argparse
import csv
import os
import sys

from openpyxl import Workbook
from openpyxl.utils import get_column_letter

# Ordem do catálogo (#1..#23) do README, usada nas abas e no Resumo.
ENTRY_ORDER = [
    "rls",
    "rls-grabowski",
    "rls-de-abc",
    "mrls-p-eda",
    "best-swap-ig",
    "best-swap-ig-fixed",
    "swap-first-ig",
    "vnd1",
    "vnd2",
    "vnd1-fixed",
    "vnd2-fixed",
    "ig-ij-dispatch",
    "svns-s-swap",
    "svns-s-insertion",
    "svns-d-swap",
    "svns-d-insertion",
    "hvns-best-insertion",
    "hvns-best-edge-insertion",
    "hvns-best-swap",
    "hvns-shaking",
    "hvns-sa-rls",
    "hvns-sa-edge-insertion",
    "tpa-sa",
]

SHEET_HEADER = ["Instância", "Custo inicial", "Custo final", "Melhoria absoluta", "Melhoria (%)", "Tempo (ms)", "Nota"]


def parse_results_file(path):
    """Retorna [{instance, initial_cost, final_cost, improvement_abs,
    improvement_pct, time_ms, note}, ...]."""
    rows = []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for rec in reader:
            rows.append({
                "instance": rec["instance"],
                "initial_cost": int(rec["initial_cost"]),
                "final_cost": int(rec["final_cost"]),
                "improvement_abs": float(rec["improvement_abs"]),
                "improvement_pct": float(rec["improvement_pct"]),
                "time_ms": float(rec["time_ms"]),
                "note": rec["note"],
            })
    return rows


def discover_results(results_dir):
    """Retorna {entrada: rows} para os results/<entrada>.csv existentes,
    na ordem de ENTRY_ORDER (extras vão para o fim, em ordem alfabética)."""
    found = {}
    for name in os.listdir(results_dir):
        if not name.endswith(".csv"):
            continue
        entry = os.path.splitext(name)[0]
        found[entry] = parse_results_file(os.path.join(results_dir, name))
    if not found:
        sys.exit(f"nenhum results/*.csv encontrado em {results_dir} (rode run_all.py antes)")

    ordered = [e for e in ENTRY_ORDER if e in found]
    ordered += sorted(e for e in found if e not in ENTRY_ORDER)
    return {e: found[e] for e in ordered}


def autosize_columns(sheet):
    """Ajusta a largura de cada coluna ao maior valor (cabeçalho incluído)."""
    for col in sheet.iter_cols():
        widest = max((len(str(cell.value)) for cell in col if cell.value is not None), default=0)
        sheet.column_dimensions[get_column_letter(col[0].column)].width = widest + 2


def sheet_rows_for(rows):
    return [
        [r["instance"], r["initial_cost"], r["final_cost"], r["improvement_abs"], r["improvement_pct"],
         r["time_ms"], r["note"]]
        for r in rows
    ]


def write_workbook(results, out_path):
    wb = Workbook()
    wb.remove(wb.active)

    summary_sheet = wb.create_sheet(title="Resumo")
    summary_sheet.append(
        ["Entrada", "Melhoria média (%)", "Melhoria mín (%)", "Melhoria máx (%)", "Tempo médio (ms)", "N instâncias"]
    )

    summary = []
    for entry, rows in results.items():
        if not rows:
            continue
        pcts = [r["improvement_pct"] for r in rows]
        times = [r["time_ms"] for r in rows]
        summary.append((entry, sum(pcts) / len(pcts), min(pcts), max(pcts), sum(times) / len(times), len(rows)))

    for row in summary:
        summary_sheet.append(list(row))
    autosize_columns(summary_sheet)

    for entry, rows in results.items():
        sheet = wb.create_sheet(title=entry[:31])
        sheet.append(SHEET_HEADER)
        for row in sheet_rows_for(rows):
            sheet.append(row)
        autosize_columns(sheet)

    wb.save(out_path)
    return summary


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    parser.add_argument("results", nargs="?", default=os.path.join(script_dir, "results"),
                        help="diretório com os results/*.csv (default: ./results)")
    parser.add_argument("--output", default=os.path.join(script_dir, "output.xlsx"),
                        help="xlsx a gravar (default: ./output.xlsx)")
    args = parser.parse_args()

    results = discover_results(args.results)
    summary = write_workbook(results, args.output)

    total_rows = sum(len(rows) for rows in results.values())
    print(f"{'Entrada':<26} {'Melhoria média':>15} {'mín':>10} {'máx':>10} {'Tempo médio (ms)':>18} {'N inst':>8}")
    for entry, avg, pmin, pmax, avg_time, count in summary:
        print(f"{entry:<26} {avg:>14.3f}% {pmin:>9.3f}% {pmax:>9.3f}% {avg_time:>18.3f} {count:>8}")

    print(f"\nwrote {total_rows} results across {len(results)} sheets to {args.output}")


if __name__ == "__main__":
    main()
