# constructions

Coleção de **heurísticas construtivas** para o **BFSP** (*Blocking Flow Shop
Scheduling Problem*, objetivo: minimizar o makespan), implementadas em C++17 e
avaliadas em um único binário. Faz parte do repositório `bfsp-collection`.

## Heurísticas

| Algoritmo | Arquivo | Descrição |
|-----------|---------|-----------|
| `NEH` | `src/constructions/NEH.cpp` | NEH com aceleração de Taillard, partindo da ordem LPT |
| `PF` | `PF.cpp` | Profile Fitting |
| `PF_NEH` | `PF_NEH.cpp` | PF nas primeiras posições + NEH (ponto de corte `--lambda`) |
| `PFT` | `PFT.cpp` | PF com desempate (*ties*) |
| `PFT_NEH` | `PFT_NEH.cpp` | PFT + NEH (ponto de corte `--lambda`) |
| `LPT` | `LPT.cpp` | Ordenação por maior tempo total de processamento |
| `MinMax` | `MinMax.cpp` | MinMax de Ronconi (parâmetro `--minmax-alpha`) |
| `mNEH` | `mNEH.cpp` | NEH modificado (parâmetro `--mneh-alpha`) |
| `PW` | `PW.cpp` | Pan & Wang |
| `GRASP_NEH` | `GRASP_NEH.cpp` | GRASP + NEH (`--delta`, `--beta`, `--seed`) |

Código compartilhado: `src/Core.cpp` (cálculo de makespan, tempos de saída e
caudas), `src/Instance.cpp` (leitura de instâncias), `include/` (headers, RNG,
`Solution`).

## Compilação

Requer um compilador C++17, [Meson](https://mesonbuild.com/) e Ninja. A
dependência `argparse` vem de `subprojects/`.

```sh
meson setup build
ninja -C build
```

O build usa `release`, LTO e `-march=native`.

## Uso

```sh
./build/constructions <instância> [opções]
```

| Opção | Padrão | Significado |
|-------|--------|-------------|
| `-a, --algorithm` | todos | Heurística a rodar (omitido = roda todas) |
| `-s, --seed` | — | Semente do RNG (só afeta `GRASP_NEH`) |
| `-v, --verbose` | `false` | Imprime também a sequência de jobs |
| `--iter` | `10` | Nº de repetições (para medir tempo) |
| `--lambda` | `n > 200 ? 200 : n` | Ponto de corte de `PF_NEH`/`PFT_NEH` |
| `--minmax-alpha` | `0.60` | α do MinMax |
| `--mneh-alpha` | `0.8` | α do mNEH |
| `--delta` | `20` | Ponto de corte GRASP/NEH do `GRASP_NEH` |
| `--beta` | `5e-4` | Limiar da RCL do `GRASP_NEH` |

Saída por execução: `makespan,tempo_ms`.

Exemplo:

```sh
./build/constructions instances/J20M5/J20M5N1 -a NEH -v
```

## Instâncias

Ficam em `instances/J<jobs>M<máquinas>/J<jobs>M<máquinas>N<k>`, cobrindo
20, 50, 100, 200 e 500 jobs com 5, 10 e 20 máquinas (15 grupos).

## Rodando o benchmark completo

```sh
python3 run_all.py                 # todos os algoritmos × todas as instâncias
python3 run_all.py --algorithms NEH,PF --seed 1 -j 4
python3 run_all.py --dry-run       # só mostra a ordem de execução
```

Opções: `--no-build`, `--algorithms`, `--results-dir` (padrão `results/`),
`--seed`, `--dry-run`, `-j/--threads`.

Os algoritmos rodam em ordem alfabética e as instâncias da menor para a maior.
Cada algoritmo gera `results/<algoritmo>.txt`, em pares de linhas terminados por
`-`:

```
instances/J20M5/J20M5N1
1462,0.054045
...
-
```

## Gerando planilha e relatório

```sh
python3 output_to_excel.py         # results/*.txt -> constructions_results.xlsx
python3 ../xlsx_to_md.py           # planilha -> Markdown em ../outputs/
```

A planilha tem uma aba **Resumo** (gap médio/mín/máx e nº de instâncias por
algoritmo, ordenado por gap médio) e uma aba por algoritmo (instância, valor
objetivo, tempo em ms, gap %).

`gap% = (obtido − melhor conhecido) / melhor conhecido × 100`, usando
`../best_ribas.json`; instâncias sem valor de referência ficam sem gap.

## Estrutura

```
constructions/
├── include/            headers (Core, Instance, Solution, RNG, Parameters, ...)
├── src/
│   ├── main.cpp        CLI e despacho dos algoritmos
│   ├── Core.cpp        makespan / Taillard
│   └── constructions/  uma heurística por arquivo
├── instances/          instâncias BFSP
├── results/            saídas brutas por algoritmo
├── run_all.py          executa o benchmark
├── run.sh              versão simples em shell
└── output_to_excel.py  consolida resultados em .xlsx
```

## Licença

MIT.
