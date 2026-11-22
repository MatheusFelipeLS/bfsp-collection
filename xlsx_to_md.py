#!/usr/bin/env python3
"""
xlsx_para_md.py - Converte arquivos .xlsx/.xlsm em Markdown otimizado para LLMs.

Uso:
    1. Edite a secao CONFIGURACAO logo abaixo (caminhos e opcoes).
    2. Execute:  python xlsx_para_md.py

Dependencia:
    pip install openpyxl

O que o script faz:
  - Cada aba vira uma secao (## Nome da aba) com uma tabela Markdown.
  - Celulas mescladas tem o valor replicado em todo o intervalo.
  - Usa os valores calculados (cache do Excel); se nao existirem, cai para a formula.
  - Datas, percentuais e numeros sao formatados de forma legivel.
  - Remove linhas e colunas totalmente vazias.
  - Lista comentarios de celulas e formulas (opcional) em secoes separadas.
  - Abas ocultas sao ignoradas (a menos que INCLUIR_OCULTAS = True).
"""

import datetime as dt
import sys
from pathlib import Path

from openpyxl import load_workbook
from openpyxl.utils import get_column_letter

# ============================================================================
# CONFIGURACAO - edite aqui
# ============================================================================
# Arquivos .xlsx/.xlsm e/ou pastas (todas as planilhas da pasta serao convertidas).
# Dica: use o prefixo r"..." para caminhos do Windows (evita problemas com "\").
ENTRADAS = [
    r"/home/matheus/Documents/bfsp-collection/constructions/constructions_results.xlsx", # constructions
    r"/home/matheus/Documents/bfsp-collection/speed-ups/speed_ups_results.xlsx", # speed ups
    r"/home/matheus/Documents/bfsp-collection/local-searchs-3/local_searchs_results.xlsx", #local searchs
]

# Pasta onde os arquivos convertidos serao salvos.
# Use None para salvar ao lado de cada arquivo original.
PASTA_SAIDA = "outputs"

SALVAR_COMO_TXT = False   # True = .txt | False = .md
INCLUIR_FORMULAS = False  # True = adiciona secao com as formulas
INCLUIR_OCULTAS = False   # True = inclui abas ocultas
MAX_LINHAS = None         # ex.: 500 para limitar as linhas de dados por aba
# ============================================================================

EXTENSOES = {".xlsx", ".xlsm"}


# ----------------------------------------------------------------------------
# Formatacao de valores
# ----------------------------------------------------------------------------
def formatar_valor(valor, number_format="General"):
    if valor is None:
        return ""
    if isinstance(valor, bool):
        return "TRUE" if valor else "FALSE"
    if isinstance(valor, dt.datetime):
        if valor.time() == dt.time(0, 0):
            return valor.date().isoformat()
        return valor.isoformat(sep=" ")
    if isinstance(valor, dt.date):
        return valor.isoformat()
    if isinstance(valor, dt.time):
        return valor.isoformat()
    if isinstance(valor, dt.timedelta):
        return str(valor)
    if isinstance(valor, int):
        return str(valor)
    if isinstance(valor, float):
        if "%" in (number_format or ""):
            return f"{valor * 100:.2f}".rstrip("0").rstrip(".") + "%"
        if valor == int(valor) and abs(valor) < 1e15:
            return str(int(valor))
        return f"{valor:.10g}"
    return str(valor)


def limpar_texto(texto):
    """Escapa caracteres que quebram tabelas Markdown."""
    return (
        texto.replace("\r\n", "\n")
        .replace("\r", "\n")
        .replace("|", "\\|")
        .replace("\n", "<br>")
        .strip()
    )


# ----------------------------------------------------------------------------
# Leitura da planilha
# ----------------------------------------------------------------------------
def montar_grade(ws_val, ws_form):
    """Retorna (grade, formulas, comentarios).

    grade: lista de listas de strings
    formulas: lista de (coordenada, formula, valor)
    comentarios: lista de (coordenada, texto)
    """
    max_row, max_col = ws_val.max_row or 0, ws_val.max_column or 0
    grade = [[""] * max_col for _ in range(max_row)]
    formulas, comentarios = [], []

    for row in ws_val.iter_rows(min_row=1, max_row=max_row, max_col=max_col):
        for cel in row:
            valor = cel.value
            form_cel = ws_form[cel.coordinate]
            eh_formula = isinstance(form_cel.value, str) and form_cel.value.startswith("=")

            if eh_formula:
                if valor is None:  # sem cache calculado: usa a formula
                    texto = form_cel.value
                else:
                    texto = formatar_valor(valor, cel.number_format)
                formulas.append((cel.coordinate, form_cel.value, texto))
            else:
                texto = formatar_valor(valor, cel.number_format)

            grade[cel.row - 1][cel.column - 1] = limpar_texto(texto)

            if form_cel.comment and form_cel.comment.text:
                comentarios.append((cel.coordinate, limpar_texto(form_cel.comment.text)))

    # Replica valores de celulas mescladas
    for intervalo in ws_val.merged_cells.ranges:
        topo = grade[intervalo.min_row - 1][intervalo.min_col - 1]
        for r in range(intervalo.min_row, intervalo.max_row + 1):
            for c in range(intervalo.min_col, intervalo.max_col + 1):
                if r <= max_row and c <= max_col:
                    grade[r - 1][c - 1] = topo

    return grade, formulas, comentarios


def remover_vazios(grade):
    """Remove linhas/colunas vazias. Retorna (grade, letras_das_colunas)."""
    linhas = [l for l in grade if any(c for c in l)]
    if not linhas:
        return [], []
    n_cols = max(len(l) for l in grade)
    cols_ok = [j for j in range(n_cols) if any(j < len(l) and l[j] for l in linhas)]
    nova = [[l[j] if j < len(l) else "" for j in cols_ok] for l in linhas]
    return nova, [get_column_letter(j + 1) for j in cols_ok]


# ----------------------------------------------------------------------------
# Geracao de Markdown
# ----------------------------------------------------------------------------
def tabela_md(grade, letras, max_linhas=None):
    if not grade:
        return "_(aba vazia)_\n"

    cabecalho = [c if c else f"Coluna {letras[i]}" for i, c in enumerate(grade[0])]
    corpo = grade[1:]
    truncado = 0
    if max_linhas and len(corpo) > max_linhas:
        truncado = len(corpo) - max_linhas
        corpo = corpo[:max_linhas]

    linhas = ["| " + " | ".join(cabecalho) + " |", "|" + "|".join(["---"] * len(cabecalho)) + "|"]
    for l in corpo:
        linhas.append("| " + " | ".join(l) + " |")
    if truncado:
        linhas.append(f"\n_(... {truncado} linhas omitidas por MAX_LINHAS)_")
    return "\n".join(linhas) + "\n"


def converter_arquivo(caminho, incluir_formulas=False, incluir_ocultas=False, max_linhas=None):
    wb_val = load_workbook(caminho, data_only=True)
    wb_form = load_workbook(caminho, data_only=False)

    partes = [f"# Planilha: {caminho.name}\n"]
    abas = [ws for ws in wb_val.worksheets if incluir_ocultas or ws.sheet_state == "visible"]
    partes.append(f"Abas: {', '.join(ws.title for ws in abas)}\n")

    for ws in abas:
        grade, formulas, comentarios = montar_grade(ws, wb_form[ws.title])
        grade, letras = remover_vazios(grade)

        partes.append(f"\n## Aba: {ws.title}\n")
        if ws.sheet_state != "visible":
            partes.append("_(aba oculta)_\n")
        if grade:
            partes.append(f"Dimensao: {len(grade) - 1} linhas de dados x {len(grade[0])} colunas\n")
        partes.append(tabela_md(grade, letras, max_linhas))

        if incluir_formulas and formulas:
            partes.append("\n### Formulas\n")
            partes.append("| Celula | Formula | Resultado |")
            partes.append("|---|---|---|")
            for coord, form, res in formulas:
                partes.append(f"| {coord} | `{limpar_texto(form)}` | {res} |")

        if comentarios:
            partes.append("\n### Comentarios\n")
            for coord, txt in comentarios:
                partes.append(f"- **{coord}**: {txt}")

    wb_val.close()
    wb_form.close()
    return "\n".join(partes) + "\n"


# ----------------------------------------------------------------------------
# Execucao
# ----------------------------------------------------------------------------
def coletar_arquivos(entradas):
    arquivos = []
    for e in entradas:
        p = Path(e)
        if p.is_dir():
            arquivos += sorted(
                f for f in p.iterdir()
                if f.suffix.lower() in EXTENSOES and not f.name.startswith("~$")
            )
        elif p.is_file() and p.suffix.lower() in EXTENSOES:
            arquivos.append(p)
        else:
            print(f"[aviso] ignorado: {e}", file=sys.stderr)
    return arquivos


def main():
    arquivos = coletar_arquivos(ENTRADAS)
    if not arquivos:
        sys.exit("Nenhum arquivo .xlsx/.xlsm encontrado. Confira ENTRADAS na configuracao.")

    pasta_saida = Path(PASTA_SAIDA) if PASTA_SAIDA else None
    if pasta_saida:
        pasta_saida.mkdir(parents=True, exist_ok=True)

    for arq in arquivos:
        try:
            texto = converter_arquivo(arq, INCLUIR_FORMULAS, INCLUIR_OCULTAS, MAX_LINHAS)
        except Exception as exc:
            print(f"[erro] {arq}: {exc}", file=sys.stderr)
            continue
        destino = (pasta_saida or arq.parent) / (arq.stem + (".txt" if SALVAR_COMO_TXT else ".md"))
        destino.write_text(texto, encoding="utf-8")
        print(f"[ok] {arq} -> {destino}")


if __name__ == "__main__":
    main()