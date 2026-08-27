#!/usr/bin/env python3
"""Converts a constructions/output.txt benchmark log into an .xlsx workbook
with one sheet per algorithm (columns: instance, objective value, time)."""

import argparse
import os
import re
import sys

from openpyxl import Workbook

DATA_LINE_RE = re.compile(r"^(\w+),(\d+),([0-9.eE+-]+)$")


def parse_output(path):
    """Returns {algorithm: [(instance, cost, time_ms), ...]}, preserving the
    order in which each algorithm first appears."""

    results = {}
    current_instance = None

    with open(path) as f:
        for lineno, raw_line in enumerate(f, start=1):
            line = raw_line.strip()

            if not line or line == "-":
                continue

            match = DATA_LINE_RE.match(line)
            if match:
                if current_instance is None:
                    raise ValueError(f"{path}:{lineno}: result line before any instance path: {line!r}")

                algorithm, cost, time_ms = match.groups()
                results.setdefault(algorithm, []).append((current_instance, int(cost), float(time_ms)))
                continue

            depth = line.count("/")
            if depth >= 2:
                current_instance = os.path.basename(line)
            elif depth == 1:
                pass  # group header line (e.g. "instances/J100M10"), nothing to record
            else:
                print(f"warning: {path}:{lineno}: ignoring unrecognized line: {line!r}", file=sys.stderr)

    return results


def write_workbook(results, out_path):
    wb = Workbook()
    wb.remove(wb.active)

    for algorithm, rows in results.items():
        sheet = wb.create_sheet(title=algorithm[:31])
        sheet.append(["Instância", "Valor Objetivo", "Tempo (ms)"])
        for instance, cost, time_ms in rows:
            sheet.append([instance, cost, time_ms])

    wb.save(out_path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", nargs="?", default="output.txt", help="benchmark log (default: output.txt)")
    parser.add_argument("output", nargs="?", default="output.xlsx", help="xlsx file to write (default: output.xlsx)")
    args = parser.parse_args()

    results = parse_output(args.input)
    if not results:
        sys.exit(f"no results found in {args.input}")

    write_workbook(results, args.output)
    print(f"wrote {sum(len(rows) for rows in results.values())} results across {len(results)} sheets to {args.output}")


if __name__ == "__main__":
    main()
