# speed-ups

Catálogo executável das técnicas de **aceleração de avaliação** ("speed ups")
usadas pelas metaheurísticas de BFSP deste repositório.

Assim como `constructions/` isola os heurísticos construtivos num projeto Meson
próprio, esta pasta isola os speed ups **e os componentes que os consomem**
(NEH, RLS, busca local de troca, inserção de par adjacente) e mede cada um contra
a reavaliação ingênua equivalente.

> Componente aqui = subrotina (NEH, RLS, busca local de troca, ...), **não** a
> metaheurística de alto nível (MA, IG, HVNS, ...).

## Os speed ups

| # | Speed up | O que faz | Cópia canônica aqui | Componentes que o usam no repo |
|---|----------|-----------|---------------------|--------------------------------|
| 1 | **Aceleração de Taillard — vizinhança de inserção** | Matrizes cabeça `e` (`departure_times`) e cauda `q` (`tail`); o makespan de inserir um job em qualquer das `n+1` posições sai em `O(m)`, logo todas as posições em `O(n·m)` em vez de `O(n²·m)`. `insert_calculation` ainda aborta cedo quando o limitante parcial alcança o incumbente. | `core::calculate_departure_times`/`calculate_tail` (`src/Core.cpp`); `NEH::insert_calculation`/`taillard_best_insertion` (`src/constructions/NEH.cpp`) | NEH e derivados (PF_NEH, PFT_NEH, mNEH, GRASP_NEH); `rls`; `rls_grabowski`; ramo de inserção do VND; reconstrução do IG |
| 2 | **Restrição ao bloco crítico de Grabowski–Wodecki** | Percorre o caminho crítico do grafo do blocking flow shop de trás para frente, particiona-o em blocos NORMAL/ANTI/VERT e devolve só as faixas de posição **fora** do bloco crítico do job — as únicas onde a reinserção pode melhorar. | `grabowski_reinsertion` (`src/local-search/Grabowski.cpp`, extraído de `MA/src/local-search/RLS.cpp`); `NEH::taillard_grabowski_best_ins` (`src/constructions/NEH.cpp`) | `rls_grabowski` (MA, IG_IJ, IG_VND1/2, IG_RIS, DIWO, SaDIWO, MFFO, HDDE, DE_ABC) |
| 3 | **Recomputação parcial/incremental do makespan** | Após um movimento, recalcula `departure_times` só do índice alterado até o fim (`O((n−i)·m)`) e restaura apenas a linha "suja" da matriz em vez de copiar a `Solution` inteira. | `core::partial_recalculate_solution` (`src/Core.cpp`); truque da linha suja em `src/local-search/Swap.cpp` | Buscas locais de **troca** (`IG::local_search`, `BestSwap` do VND, `HVNS::best_swap`, SVNS `LS1_*_swap`, MA/DE_ABC) e a atualização pós-movimento do `rls_grabowski` |
| 4 | **Taillard estendido à inserção de par adjacente (edge)** | Monta uma matriz `m_f` com o primeiro job do par já presente em cada prefixo; o segundo job é então avaliado contra a cauda `q` em `O(m)` por posição, dando todas as posições de um movimento de bloco de 2 jobs em `O(n·m)`. | `EdgeInsertion` (`src/local-search/EdgeInsertion.cpp`, portado de `HVNS/src/HVNS.cpp`) | `HVNS::best_edge_insertion`, `HVNS::sa_best_edge_insertion` |
| 5 | **Delta incremental de ocioso/bloqueio (α / σ)** | Durante a construção gulosa, o custo de anexar um candidato ao fim da sequência parcial sai em `O(m)` a partir da última linha de `departure_times` (que a heurística já mantém), sem reavaliar o escalonamento parcial inteiro. `σ` é um índice de look-ahead ponderado sobre `α`. | `core::calculate_new_departure_time`/`calculate_alpha`/`calculate_sigma` (`src/Core.cpp`) | Construtivos PF, PFT, PW, GRASP_NEH |
| 6 | **Buffers pré-alocados reutilizados** | As matrizes `e`/`q`/`m_f` são alocadas uma única vez no construtor da classe e reescritas a cada chamada, evitando um `malloc` por movimento; `core::recalculate_solution` só cresce a matriz preguiçosamente. | ctor de `NEH` (`src/constructions/NEH.cpp`) e de `EdgeInsertion`; `core::recalculate_solution` (`src/Core.cpp`) | Transversal — otimiza #1, #2, #4 |

### Variações preservadas

- `EdgeInsertion::taillard_best_insertion(seq, job, original_position)` — a versão do
  HVNS de #1, com matriz `m_f` completa e o parâmetro `original_position` que pula o
  próprio slot do job (movimento identidade).
- `NEH::mtaillard_best_insertion(seq, job, original_position)` — a versão do TPA de #1,
  que parte o laço de posições em torno de `original_position`. Usada pelo SA do TPA.
- `rls_grabowski` combina #1 + #2 + #3 (`core::partial_recalculate_solution` após cada
  movimento aceito).

## O benchmark

`./build/speed-ups <instância>` roda, para cada speed up, a versão acelerada **e** a
reavaliação ingênua equivalente, cronometra as duas e **verifica que produzem o mesmo
resultado**. Saída CSV (cabeçalho em `stderr`, dados em `stdout`):

```
speedup,n,m,iters,naive_ms,accel_ms,factor,correct,note
```

`run_all.py` varre as instâncias e reparte essas linhas em `results/<speedup>.csv`
(colunas `instance,naive_ms,accel_ms,factor,correct,note` — sem `n`/`m`/`iters`);
`output_to_excel.py` consolida os `results/*.csv` num `output.xlsx` com uma aba
**Resumo** (fator médio/mín/máx por speed up) + **uma aba por speed up**, cada
uma só com as colunas que fazem sentido: as de tempo puro
(`Instância | Tempo ingênuo (ms) | Tempo acelerado (ms) | Fator | Correto`) e,
para `grabowski-block` (que compara duas buscas locais diferentes), também os
custos `Custo inicial | Custo rls | Custo grabowski`.

- `factor = naive_ms / accel_ms` — quantas vezes o componente acelerado é mais rápido.
- `correct = 1` — o resultado acelerado bate **exatamente** com o da baseline.

| linha | acelerado | baseline ingênua | `correct` verifica |
|-------|-----------|------------------|--------------------|
| `taillard-insertion` | `NEH::solve` (matrizes `e`/`q`) | `naive::neh_construct` (recompute total por posição, `O(n²·m)` por passo) | sequência e makespan idênticos |
| `grabowski-block` | `rls_grabowski` (vizinhança restrita ao bloco) | `rls` (vizinhança de inserção completa) | ambas terminam em escalonamento válido com custo ≤ inicial |
| `partial-recalc` | `swap_local_search_best` (`partial_recalculate_solution` + linha suja) | `naive::swap_neighborhood_best` (recompute total por par) | sequência e makespan idênticos |
| `edge-insertion` | `EdgeInsertion::edge_insertion_local_search` (matriz `m_f`) | `naive::edge_insertion_local_search` (recompute total por posição) | sequência e makespan idênticos |
| `alpha-sigma` | `core::calculate_new_departure_time` (`O(m)`) | `naive::new_departure_time_full` (recompute do parcial, `O(k·m)`) | vetor de tempos de saída idêntico em todo passo |
| `preallocated-buffers` | 1 instância de `NEH` reutilizada em N chamadas | uma `NEH` nova por chamada (realoca `e`/`q`) | mesmo `{índice, makespan}` |

`grabowski-block` compara duas buscas locais **diferentes** (restrita vs. completa), então
seus resultados não são idênticos por construção — o `factor` mostra o efeito da poda e
`correct` confirma que ambas continuam válidas. Em `n` pequeno o custo da caminhada no
grafo supera a poda (`factor < 1`); o ganho aparece conforme `n` cresce.

### Uso

```sh
meson setup build
ninja -C build

./build/speed-ups instances/J100M20/J100M20N1            # todos os speed ups
./build/speed-ups instances/J500M20/J500M20N1 -p edge-insertion
./build/speed-ups instances/J100M10/J100M10N1 -n 20 -s 1  # 20 repetições, seed 1

python3 -m venv .venv && .venv/bin/pip install openpyxl   # (uma vez) dep do xlsx
python3 run_all.py                                        # varre instâncias -> results/<speedup>.csv
.venv/bin/python output_to_excel.py                       # results/*.csv -> output.xlsx
```

Opções do binário: `-p/--speedup <nome>`, `-n/--iters <k>`, `-s/--seed <s>` (padrão 42), `-v`.
Opções do `run_all.py`: `--all` (todas as instâncias, não só `*N1`), `--iters`, `--seed`,
`--no-build`, `--results-dir`.

### Números de referência (uma máquina, `-march=native`, release)

```
                       J20M5      J100M20     J500M20
taillard-insertion      15x         72x         350x
edge-insertion          22x         83x         390x
alpha-sigma             17x         70x         310x
partial-recalc         8.5x        4.8x         4.7x
preallocated-buffers   4.3x        2.8x         2.3x
grabowski-block        0.76x       0.40x        1.3x
```

## Mapa de arquivos

```
include/ src/                 infra copiada de constructions/ (Core, Instance, Solution,
                              Parameters, RNG, Log) + os 10 construtivos
  constructions/NEH.*         #1, #2 (metade NEH), #6, + mtaillard (variação do TPA)
  local-search/
    RLS.*                     rls + rls_grabowski (de MA/), consumidores de #1/#2/#3
    Grabowski.*               #2 — máquina de blocos críticos extraída do RLS.cpp
    Swap.*                    #3 — busca local de troca extraída de IG/HVNS
    EdgeInsertion.*           #4 (+ #6) — portado de HVNS
  speedups/
    Naive.*                   baselines ingênuos
    Bench.*                   harness de tempo + verificação de corretude
src/main.cpp                  driver CLI
run_all.py                    varre instâncias -> results/<speedup>.csv
output_to_excel.py            results/*.csv -> output.xlsx (Resumo + aba por speed up)
```

Nenhum arquivo fora de `speed-ups/` foi alterado — as cópias canônicas continuam nas
metaheurísticas de origem.
