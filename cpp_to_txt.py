#!/usr/bin/env python3
"""
Para cada arquivo .cpp de uma pasta (incluindo subpastas), cria um arquivo .txt
com o mesmo nome e o mesmo conteúdo (ex.: main.cpp -> main.txt),
salvando o resultado na pasta de saída escolhida.

A estrutura de subpastas é mantida na saída, para evitar que arquivos
com o mesmo nome em pastas diferentes se sobrescrevam.
"""
import shutil
from pathlib import Path


def main():
    # Pasta de onde os .cpp serão lidos
    pasta = Path(r"/home/matheus/Documents/bfsp-collection/speed-ups")

    # Pasta onde os .txt serão salvos (será criada se não existir)
    pasta_saida = Path(r"/home/matheus/Documents/bfsp-collection/speed-ups_txt")

    busca = pasta.rglob("*")
    total = 0

    for arquivo in busca:
        if arquivo.is_file() and arquivo.suffix.lower() == ".cpp":
            # mantém o caminho relativo à pasta de origem
            relativo = arquivo.relative_to(pasta)
            destino = (pasta_saida / relativo).with_suffix(".txt")
            destino.parent.mkdir(parents=True, exist_ok=True)

            shutil.copyfile(arquivo, destino)  # copia o conteúdo exatamente como está
            print(f"{relativo} -> {destino}")
            total += 1

    print(f"\nConcluído: {total} arquivo(s) convertido(s) em {pasta_saida}.")


if __name__ == "__main__":
    main()