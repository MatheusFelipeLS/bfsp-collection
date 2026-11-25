# speed-ups

Catálogo executável das técnicas de **aceleração de avaliação** ("speed ups")
usadas pelas metaheurísticas de BFSP deste repositório.

Assim como `constructions/` isola os heurísticos construtivos num projeto Meson
próprio, esta pasta isola os speed ups **e os componentes que os consomem**
(NEH, RLS, busca local de troca) e mede cada um contra
a reavaliação ingênua equivalente.

> Componente aqui = subrotina (NEH, RLS, busca local de troca, ...), **não** a
> metaheurística de alto nível (MA, IG, HVNS, ...).

## Os speed ups avaliados

| # | Speed up | O que faz | Cópia canônica aqui | Componentes que o usam no repo |
|---|----------|-----------|---------------------|--------------------------------|
| 1 | **Aceleração de Taillard — vizinhança de inserção** | Matrizes cabeça `e` (`departure_times`) e cauda `q` (`tail`); o makespan de inserir um job em qualquer das `n+1` posições sai em `O(m)`, logo todas as posições em `O(n·m)` em vez de `O(n²·m)`. | `core::calculate_departure_times`/`calculate_tail` (`src/Core.cpp`); `NEH::taillard_best_insertion` (`src/constructions/NEH.cpp`) | NEH e derivados (PF_NEH, PFT_NEH, mNEH, GRASP_NEH); `rls`; `rls_grabowski`; ramo de inserção do VND; reconstrução do IG |
| 2 | **Corte antecipado (early cut-off) da inserção** | Dentro de `NEH::insert_calculation`, a avaliação de uma posição varre as máquinas e aborta assim que o limitante parcial alcança o melhor makespan já encontrado. Não muda a trajetória, só poupa máquinas. | `NEH::insert_calculation` (`src/constructions/NEH.cpp`); `rls_taillard` (`src/local-search/RLS.cpp`) | Tudo que usa #1 (é o `rls` padrão do repo) |
| 3 | **Restrição ao bloco crítico de Grabowski–Wodecki** | Percorre o caminho crítico do grafo do blocking flow shop de trás para frente, particiona-o em blocos NORMAL/ANTI/VERT e devolve só as faixas de posição **fora** do bloco crítico do job — as únicas onde a reinserção pode melhorar. | `grabowski_reinsertion` (`src/local-search/Grabowski.cpp`, extraído de `MA/src/local-search/RLS.cpp`); `NEH::taillard_grabowski_best_ins` (`src/constructions/NEH.cpp`); `rls_grabowski_blocks` (`src/local-search/RLS.cpp`, versão sem Taillard) | `rls_grabowski` (MA, IG_IJ, IG_VND1/2, IG_RIS, DIWO, SaDIWO, MFFO, HDDE, DE_ABC) |
| 4 | **Recomputação parcial/incremental do makespan** | Após um movimento, recalcula `departure_times` só do índice alterado até o fim (`O((n−i)·m)`) e restaura apenas a linha "suja" da matriz em vez de copiar a `Solution` inteira. | `core::partial_recalculate_solution` (`src/Core.cpp`); truque da linha suja em `src/local-search/Swap.cpp` | Buscas locais de **troca** (`IG::local_search`, `BestSwap` do VND, `HVNS::best_swap`, SVNS `LS1_*_swap`, MA/DE_ABC) e a atualização pós-movimento do `rls_grabowski` |

`rls_grabowski` combina #1 + #2 + #3 (e #4 após cada movimento aceito).
`rls_grabowski_blocks` isola #3: mesma busca do `rls_grabowski`, mas **sem** as matrizes
`e`/`q` de Taillard (cada posição permitida é reavaliada para frente, com corte por linha,
e #4 após cada movimento aceito).

### Speed ups que deixaram de ser avaliados

Foram removidos do benchmark (linhas comentadas em `src/speedups/Bench.cpp`; o código-fonte
das técnicas continua no repo e nas metaheurísticas de origem):

- **Delta incremental de ocioso/bloqueio (α / σ)** — `alpha-sigma`
- **Taillard estendido à inserção de par adjacente (edge)** — `edge-insertion` (`EdgeInsertion`, de `HVNS`)
- **Buffers pré-alocados reutilizados** — `preallocated-buffers`
- **`grabowski-block`** — substituído pelo grupo `rls`, em que `rls_grabowski` passa a ser
  comparado contra o mesmo `naive::rls` das demais linhas.

## O benchmark

`./build/speed-ups <instância>` roda, para cada speed up, a versão acelerada **e** a
baseline ingênua equivalente, cronometra as duas e verifica a corretude. Todos os
benchmarks partem da **mesma solução inicial aleatória** (e da mesma ordem de referência
do RLS), sorteada uma vez por instância. Saída CSV (cabeçalho em `stderr`, dados em `stdout`):

```
speedup,n,m,iters,naive_ms,accel_ms,factor,pruned_machines_pct,discarded_positions_pct,note
```

`run_all.py` varre as instâncias e reparte essas linhas em `results/<speedup>.csv`
(colunas `instance,naive_ms,accel_ms,factor,pruned_machines_pct,discarded_positions_pct,note`
— sem `n`/`m`/`iters`); `output_to_excel.py` consolida os `results/*.csv` em
`speed_ups_results.xlsx` com uma aba **Resumo** + **uma aba por speed up**.

- `factor = naive_ms / accel_ms` — quantas vezes o componente acelerado é mais rápido.
- `pruned_machines_pct` — % das avaliações de máquina poupadas pelo corte antecipado.
- `discarded_positions_pct` — % das posições de inserção descartadas pela restrição de Grabowski.
- `correct = 1` (verificado internamente; falhas saem como `AVISO` em `stderr`).

| linha | acelerado | baseline ingênua | `correct` verifica |
|-------|-----------|------------------|--------------------|
| `taillard-insertion` | `NEH::solve` (matrizes `e`/`q`, com corte) | `naive::neh_construct` (recompute total por posição, `O(n²·m)` por passo) | sequência e makespan idênticos |
| `rls-taillard` | `rls_taillard_no_cutoff` (Taillard `e`/`q`, todas as máquinas) | `naive::rls` (recompute `O(n·m)` por posição) | mesma trajetória: sequência e makespan idênticos |
| `rls-taillard-cutoff` | `rls_taillard` (Taillard + corte antecipado, `== rls`) | `naive::rls` | mesma trajetória: sequência e makespan idênticos |
| `rls-grabowski` | `rls_grabowski` (vizinhança restrita ao bloco) | `naive::rls` | ambos terminam em escalonamento válido com custo ≤ inicial |
| `rls-grabowski-blocks` | `rls_grabowski_blocks` (**só** restrição de bloco + corte por linha + recomputação parcial; **sem** matrizes `e`/`q` de Taillard: cada posição permitida é reavaliada para frente a partir do ponto de inserção, `O((n−i)·m)`) | `naive::rls` | ambos terminam em escalonamento válido com custo ≤ inicial |
| `partial-recalc` | `swap_local_search_best` (`partial_recalculate_solution` + linha suja) | `naive::swap_neighborhood_best` (recompute total por par) | sequência e makespan idênticos |

As quatro linhas `rls-*` (grupo `rls`, aceito por `-p rls`) compartilham a mesma solução inicial
e **uma única** medição de `naive::rls`, formando uma escada de ablação: Taillard →
+ corte antecipado → + restrição de Grabowski; `rls-grabowski-blocks` mede a restrição de
Grabowski **sozinha** (sem Taillard). `rls-grabowski` e `rls-grabowski-blocks` usam outra regra
de aceitação, então podem terminar em outro ótimo local (o `note` registra `naive=`, `grabowski=` e
`same_trajectory`); as duas seguem a mesma trajetória entre si, logo descartam as mesmas posições.
O `factor` mede tempo, `correct` confirma validade.

### Uso

```sh
meson setup build
ninja -C build

./build/speed-ups instances/J100M20/J100M20N1            # todos os speed ups
./build/speed-ups instances/J500M20/J500M20N1 -p rls     # grupo rls (4 linhas)
./build/speed-ups instances/J100M10/J100M10N1 -p partial-recalc -n 20 -s 1

python3 -m venv .venv && .venv/bin/pip install openpyxl   # (uma vez) dep do xlsx
python3 run_all.py                                        # varre instâncias -> results/<speedup>.csv
.venv/bin/python output_to_excel.py                       # results/*.csv -> speed_ups_results.xlsx

# tabelas LaTeX (ignoram as classes 200x5, 500x5 e 500x10)
python3 table_to_latex.py                                          # -> speed_ups_table.tex
python3 summary_to_latex.py -o speed_ups_summary_table.tex
python3 pruning_to_latex.py -o speed_ups_pruning_table.tex
python3 pruning_by_class_to_latex.py -o speed_ups_pruning_by_class_table.tex
```

Nomes aceitos por `-p`: `taillard-insertion`, `rls` (ou `rls-taillard`, `rls-taillard-cutoff`,
`rls-grabowski`, `rls-grabowski-blocks`), `partial-recalc`. Opções do binário: `-p/--speedup`, `-n/--iters <k>`,
`-s/--seed <s>` (padrão 42), `-v`. Opções do `run_all.py`: `--all` (todas as instâncias, não só
`*N1`), `--iters`, `--seed`, `--no-build`, `--results-dir`.

### Resultados (fator médio sobre as 10 instâncias de cada tamanho, `results/*.csv`)

```
                       J20M5     J100M20    J500M20
taillard-insertion      13x        48x        282x
rls-taillard           5.2x        26x        141x
rls-taillard-cutoff    5.2x        27x        151x
rls-grabowski          3.1x        28x        194x
rls-grabowski-blocks   1.2x       2.3x       2.6x
partial-recalc         6.7x       3.9x       4.2x
```

Sem Taillard, a restrição de blocos sozinha (`rls-grabowski-blocks`) quase não acelera: ela
descarta poucas posições (≤ 5%) e cada posição restante custa `O((n−i)·m)` em vez de `O(m)`.

Fração do trabalho poupada (média das instâncias N1):

```
                       J20M5     J100M20    J500M20
cutoff: máq. podadas
  taillard-insertion    25%        45%        62%
  rls-taillard-cutoff   50%        64%        74%
rls-grabowski
  máq. podadas          64%        79%        84%
  posições descartadas  4,2%       1,1%       0,3%
rls-grabowski-blocks
  máq. podadas          5,8%       2,1%       0,8%
  posições descartadas  4,2%       1,1%       0,3%
```

## Mapa de arquivos

```
include/ src/                 infra copiada de constructions/ (Core, Instance, Solution,
                              Parameters, RNG, Log, Stats) + NEH
  constructions/NEH.*         #1, #2 (corte), #3 (metade NEH)
  local-search/
    RLS.*                     rls, rls_taillard(_no_cutoff), rls_grabowski (de MA/),
                              rls_grabowski_blocks (#3 sem Taillard)
    Grabowski.*               #3 — máquina de blocos críticos extraída do RLS.cpp
    Swap.*                    #4 — busca local de troca extraída de IG/HVNS
    EdgeInsertion.*           (não avaliado) portado de HVNS
  speedups/
    Naive.*                   baselines ingênuos
    Bench.*                   harness de tempo + verificação de corretude
  Stats.h                     contadores de trabalho (máquinas/posições) p/ as colunas de %
src/main.cpp                  driver CLI
run_all.py                    varre instâncias -> results/<speedup>.csv
output_to_excel.py            results/*.csv -> speed_ups_results.xlsx (Resumo + aba por speed up)
table_to_latex.py             results/*.csv -> speed_ups_table.tex (fator médio por classe n x m)
summary_to_latex.py           results/*.csv -> fator médio/mín./máx. por speed up
pruning_to_latex.py           results/*.csv -> máquinas podadas / posições descartadas (média)
pruning_by_class_to_latex.py  results/*.csv -> idem, por classe n x m
```

Nenhum arquivo fora de `speed-ups/` foi alterado — as cópias canônicas continuam nas
metaheurísticas de origem.
