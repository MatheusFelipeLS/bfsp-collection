#!/usr/bin/env python3
"""Converte os results/<speedup>.csv gerados pelo run_all.py num único
output.xlsx.

Uma aba "Resumo" no início agrega, por speed up, o fator de aceleração
(médio/min/max), o nº de instâncias e a média das porcentagens de trabalho
evitado. Depois, uma aba por speed up com só as colunas que fazem sentido:

  Instância | Tempo ingênuo (ms) | Tempo acelerado (ms) | Fator
    + Máquinas podadas (%)      -> onde há poda (taillard-insertion,
                                   rls-taillard-cutoff, rls-grabowski)
    + Posições descartadas (%)  -> onde há restrição de Grabowski (rls-grabowski)
    + Custo inicial | Custo ingênuo | Custo grabowski | Mesma trajetória
                                -> só rls-grabowski, que pode terminar noutro
                                   ótimo local que o naive::rls

Formato de results/<speedup>.csv (cabeçalho + uma linha por instância; as
colunas de % ficam vazias quando não se aplicam):
    instance,naive_ms,accel_ms,factor,pruned_machines_pct,discarded_positions_pct,note
    J20M5N1,0.111773,0.022277,5.01742,31.2,,start=1828 makespan=1425
"""

import argparse
import csv
import os
import re
import sys

from openpyxl import Workbook
from openpyxl.utils import get_column_letter

# Ordem natural dos speed ups, usada nas abas e no Resumo.
SPEEDUP_ORDER = [
    "taillard-insertion",
    "rls-taillard",
    "rls-taillard-cutoff",
    "rls-grabowski",
    "rls-grabowski-blocks",
    "partial-recalc",
    "hyprr-insertion-taillard",
    "hyprr-insertion-lb",
    "hyprr-insertion-ding",
    "hyprr-insertion-ding-plus",
    "hyprr-block-insertion-taillard",
    "hyprr-swap-taillard",
    "hyprr-swap-block-skip",
    "hyprr-swap-lb",
    "hyprr-swap-lb-only",
    "grabowski-block",
    "edge-insertion",
    "alpha-sigma",
    "preallocated-buffers",
]

BASE_HEADER = ["Instância", "Tempo ingênuo (ms)", "Tempo acelerado (ms)", "Fator"]
PRUNED_HEADER = "Máquinas podadas (%)"
DISCARDED_HEADER = "Posições descartadas (%)"
GRABOWSKI_COSTS_HEADER = ["Custo inicial", "Custo ingênuo", "Custo grabowski", "Mesma trajetória"]

GRABOWSKI_NOTE_RE = re.compile(r"start=(\d+)\s+naive=(\d+)\s+grabowski=(\d+)\s+same_trajectory=(\d)")
# Linhas hyprr-* com poda (block-skip / lower bounds): note traz o custo final,
# se a trajetória coincidiu com a do naive e quantos movimentos podados teriam
# melhorado o makespan (auditoria; 0 = a poda nunca cortou um movimento bom).
HYPRR_PRUNE_HEADER = ["Custo inicial", "Custo ingênuo", "Custo acelerado", "Mesma trajetória",
                      "Movimentos melhores podados"]
HYPRR_PRUNE_NOTE_RE = re.compile(
    r"start=(\d+)\s+naive=(\d+)\s+accel=(\d+)\s+same_trajectory=(\d)\s+pruned_improving=(\d+)")


def optional_float(text):
    return float(text) if text not in ("", None) else None


def parse_results_file(path):
    """Retorna [{instance, naive_ms, accel_ms, factor, pruned, discarded, note}, ...]."""
    rows = []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for rec in reader:
            rows.append({
                "instance": rec["instance"],
                "naive_ms": float(rec["naive_ms"]),
                "accel_ms": float(rec["accel_ms"]),
                "factor": float(rec["factor"]),
                "pruned": optional_float(rec.get("pruned_machines_pct")),
                "discarded": optional_float(rec.get("discarded_positions_pct")),
                "note": rec["note"],
            })
    return rows


def discover_results(results_dir):
    """Retorna {speedup: rows} para os results/<speedup>.csv existentes,
    na ordem de SPEEDUP_ORDER (extras vão para o fim, em ordem alfabética)."""
    found = {}
    for name in os.listdir(results_dir):
        if not name.endswith(".csv"):
            continue
        speedup = os.path.splitext(name)[0]
        found[speedup] = parse_results_file(os.path.join(results_dir, name))
    if not found:
        sys.exit(f"nenhum results/*.csv encontrado em {results_dir} (rode run_all.py antes)")

    ordered = [s for s in SPEEDUP_ORDER if s in found]
    ordered += sorted(s for s in found if s not in SPEEDUP_ORDER)
    return {s: found[s] for s in ordered}


def autosize_columns(sheet):
    """Ajusta a largura de cada coluna ao maior valor (cabeçalho incluído)."""
    for col in sheet.iter_cols():
        widest = max((len(str(cell.value)) for cell in col if cell.value is not None), default=0)
        sheet.column_dimensions[get_column_letter(col[0].column)].width = widest + 2


def has(rows, key):
    return any(r[key] is not None for r in rows)


def mean_of(rows, key):
    values = [r[key] for r in rows if r[key] is not None]
    return sum(values) / len(values) if values else None


def header_for(speedup, rows):
    header = list(BASE_HEADER)
    if has(rows, "pruned"):
        header.append(PRUNED_HEADER)
    if has(rows, "discarded"):
        header.append(DISCARDED_HEADER)
    if speedup in ("rls-grabowski", "rls-grabowski-blocks"):
        header += GRABOWSKI_COSTS_HEADER
    if any(HYPRR_PRUNE_NOTE_RE.search(r["note"]) for r in rows):
        header += HYPRR_PRUNE_HEADER
    return header


def sheet_rows_for(speedup, rows):
    """Converte os registros brutos nas linhas da aba daquele speed up."""
    with_pruned, with_discarded = has(rows, "pruned"), has(rows, "discarded")
    with_hyprr = any(HYPRR_PRUNE_NOTE_RE.search(r["note"]) for r in rows)
    out = []
    for r in rows:
        line = [r["instance"], r["naive_ms"], r["accel_ms"], r["factor"]]
        if with_pruned:
            line.append(r["pruned"])
        if with_discarded:
            line.append(r["discarded"])
        if speedup in ("rls-grabowski", "rls-grabowski-blocks"):
            m = GRABOWSKI_NOTE_RE.search(r["note"])
            line += [int(x) for x in m.groups()] if m else [None] * 4
        if with_hyprr:
            m = HYPRR_PRUNE_NOTE_RE.search(r["note"])
            line += [int(x) for x in m.groups()] if m else [None] * 5
        out.append(line)
    return out


def write_workbook(results, out_path):
    wb = Workbook()
    wb.remove(wb.active)

    summary_sheet = wb.create_sheet(title="Resumo")
    summary_sheet.append([
        "Speedup", "Fator médio", "Fator mínimo", "Fator máximo", "N instâncias",
        "Máquinas podadas médio (%)", "Posições descartadas médio (%)",
    ])

    summary = []
    for speedup, rows in results.items():
        if not rows:
            continue
        factors = [r["factor"] for r in rows]
        summary.append((
            speedup, sum(factors) / len(factors), min(factors), max(factors), len(rows),
            mean_of(rows, "pruned"), mean_of(rows, "discarded"),
        ))

    for row in summary:
        summary_sheet.append(list(row))
    autosize_columns(summary_sheet)

    for speedup, rows in results.items():
        sheet = wb.create_sheet(title=speedup[:31])
        sheet.append(header_for(speedup, rows))
        for row in sheet_rows_for(speedup, rows):
            sheet.append(row)
        autosize_columns(sheet)

    wb.save(out_path)
    return summary


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    parser.add_argument("results", nargs="?", default=os.path.join(script_dir, "results"),
                        help="diretório com os results/*.csv (default: ./results)")
    parser.add_argument("--output", default=os.path.join(script_dir, "speed_ups_results.xlsx"),
                        help="xlsx a gravar (default: ./speed_ups_results.xlsx)")
    args = parser.parse_args()

    results = discover_results(args.results)
    summary = write_workbook(results, args.output)

    total_rows = sum(len(rows) for rows in results.values())
    fmt_pct = lambda v: f"{v:>10.1f}" if v is not None else f"{'-':>10}"
    print(f"{'Speedup':<22} {'Fator médio':>12} {'Fator min':>12} {'Fator max':>12} {'N inst':>8}"
          f" {'Podadas%':>10} {'Descart%':>10}")
    for speedup, avg, fmin, fmax, count, pruned, discarded in summary:
        print(f"{speedup:<22} {avg:>12.3f} {fmin:>12.3f} {fmax:>12.3f} {count:>8}"
              f" {fmt_pct(pruned)} {fmt_pct(discarded)}")

    print(f"\nwrote {total_rows} results across {len(results)} sheets to {args.output}")


if __name__ == "__main__":
    main()
