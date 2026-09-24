# local-searchs-fair

Catálogo executável de **todas as buscas locais** usadas pelas metaheurísticas
de BFSP deste repositório — mesmo catálogo de algoritmos que `local-searchs/`,
mas com uma metodologia de comparação diferente. Ver "Comparação justa"
abaixo para o motivo de existir como pasta separada em vez de uma mudança em
`local-searchs/`.

Assim como `constructions/` isola os heurísticos construtivos e `speed-ups/`
isola as técnicas de aceleração de avaliação, esta pasta isola as buscas locais
**e mede cada uma a partir do mesmo ponto de partida** — uma permutação
aleatória com seed fixa, gerada uma vez por instância — para que melhoria e
tempo sejam diretamente comparáveis entre entradas.

## Comparação justa

`local-searchs/` mede cada entrada a partir da mesma solução inicial, mas
**não** sob o mesmo orçamento de busca: `rls`, `best-swap-ig`, `vnd1`, etc.
rodam uma única passada até convergir (tipicamente frações de milissegundo),
enquanto `svns-d-*`, `hvns-*` e `tpa-sa` consomem deliberadamente um orçamento
de tempo próprio — e, pior, o de `svns-d-*` cresce com o tamanho da instância
(`n*m` ms), enquanto os de `hvns-*`/`tpa-sa` ficam fixos. Comparar
`final_cost`/`improvement_pct` nessas condições favorece estruturalmente quem
recebeu mais tempo, não necessariamente quem é o melhor algoritmo. Esta pasta
ataca isso em duas frentes:

1. **Orçamento de tempo único (`--time-limit-ms`, default 2000ms) para TODA
   entrada.** As entradas que já eram de passe único agora reiniciam com a
   ordem de varredura reembaralhada (o mesmo truque que `svns-d-swap`/
   `svns-d-insertion` já usavam em `local-searchs/`) até uma passada parar de
   melhorar ou o orçamento acabar — ver `run_ref_to_budget`/
   `run_call_to_budget` em `src/localsearchs/Harness.cpp`. Quem já era
   auto-orçado (`svns-d-*`, `hvns-*`, `tpa-sa`) passa a receber o MESMO
   orçamento em vez de flags separadas (`--svns-time-limit-ms`,
   `--hvns-time-limit-ms`, `--tpa-time-limit-ms`, removidas). O CSV ganha as
   colunas `time_limit_ms` (o teto compartilhado) e `passes=N` em `note` —
   então quando uma entrada não consegue aproveitar um orçamento maior (por
   não ter de onde tirar mais diversificação, ex. `best-swap-ig`), isso fica
   **visível** em vez de escondido.
2. **RPD (Relative Percentage Deviation) em vez de `improvement_pct`.**
   `improvement_pct` mede o quanto uma busca melhorou a solução inicial
   aleatória — não diz nada sobre quão perto do melhor resultado possível ela
   chegou, então uma busca fraca numa instância fácil pode parecer melhor que
   uma busca forte numa instância difícil. RPD mede a distância até uma
   referência fixa por instância: o melhor custo conhecido da literatura
   (`best_ribas.json` na raiz do repo — Taillard/Ribas) quando disponível,
   senão o melhor custo médio obtido por qualquer entrada nesta própria
   rodada. Ver `output_to_excel.py`.

O catálogo de algoritmos, os bugs reproduzidos fielmente (`best-swap-ig`
continua um no-op silencioso, de propósito — ver "Bugs e peculiaridades") e a
solução inicial compartilhada são **idênticos** a `local-searchs/`; só a
forma de orçar tempo e de reportar qualidade mudou. As duas pastas continuam
existindo lado a lado porque respondem perguntas diferentes: `local-searchs/`
= "quanto cada busca melhora, rodando do jeito mais próximo do seu uso
original nas metaheurísticas de origem"; `local-searchs-fair/` = "quem é
realmente melhor, dando a todos o mesmo tempo de busca".

> Entrada aqui = uma versão *comportamentalmente distinta* de busca local, não
> uma cópia por algoritmo de origem. Quando várias metaheurísticas usam o
> **mesmo** código (byte-idêntico), isso vira uma única entrada, documentada
> abaixo com a lista de quem a usa. Ver "Deduplicação".

## As entradas

| # | Entrada | Origem (arquivo:linhas) | O que faz | Algoritmos de origem |
|---|---------|--------------------------|-----------|------------------------|
| 1 | `rls` | `speed-ups/src/local-search/RLS.cpp:64-94` | Busca local de inserção (Taillard), converge (`cnt` reseta a cada melhoria) | MA, DIWO, MFFO, SaDIWO, HDDE, speed-ups |
| 2 | `rls-grabowski` | mesmo arquivo, `:23-62` | `rls` restrita aos blocos críticos de Grabowski–Wodecki + recálculo parcial pós-movimento | idem |
| 3 | `rls-de-abc` | `DE_ABC/src/local-search/RLS.cpp:185-213` | Assinatura própria `(Solution&, Instance&)` (sem `ref`, construída internamente); **passe único** — sem reset de `cnt`, sem `continue` na melhoria | DE_ABC |
| 4 | `mrls-p-eda` | `P_EDA/src/P_EDA.cpp:37-83` | Igual a `rls`, mas reembaralha `ref` via RNG sempre que o índice cíclico dá a volta | P_EDA |
| 5 | `best-swap-ig` | `IG_IJ/src/IG_IJ.cpp:40-82` (≡ IG_VND1, IG_VND2) | Troca melhor-melhoria. **Bug real e não documentado**: a comparação usa `solution.cost` em vez de `copy.cost` → nunca aplica nenhuma troca. Ver "Bugs e peculiaridades" | IG_IJ, IG_VND1, IG_VND2 |
| 6 | `best-swap-ig-fixed` | idem, comparação corrigida (`copy.cost`) | Mesma vizinhança, sem o bug — para comparação lado a lado com #5 | — (versão de comparação, não existe no repo) |
| 7 | `swap-first-ig` | `IG/src/IG.cpp:109-144` (≡ `speed-ups`'s `swap_local_search_first`) | Troca primeira-melhoria, converge | IG |
| 8 | `vnd1` | `IG_VND1/src/IG_VND1.cpp:129-143` | VND: k=1 `rls` → k=2 `best-swap-ig`, reinicia em k=1 a cada ganho. Na prática degrada a um único `rls` (k=2 nunca contribui, dado o bug de #5) | IG_VND1 |
| 9 | `vnd2` | `IG_VND2/src/IG_VND2.cpp:129-143` | VND: k=1 `best-swap-ig` → k=2 `rls`. k=1 é morto (mesmo bug), então na prática é um `rls` precedido de um no-op | IG_VND2 |
| 10 | `vnd1-fixed` | idem #8, com `best-swap-ig-fixed` no lugar de `best-swap-ig` | Mesmo VND de #8, mas com a troca funcionando de verdade — melhora genuinamente além de `rls`/`vnd1` | — (versão de comparação, não existe no repo) |
| 11 | `vnd2-fixed` | idem #9, com `best-swap-ig-fixed` no lugar de `best-swap-ig` | Mesmo VND de #9, com a troca corrigida | — (versão de comparação, não existe no repo) |
| 12 | `ig-ij-dispatch` | `IG_IJ/src/IG_IJ.cpp:118-133` | **Não é VND**: escolha probabilística por chamada — `RNG(0,1) < jp` → `best-swap-ig`, senão `rls` (`jp` default 0.001) | IG_IJ |
| 13 | `svns-s-swap` | `SVNS_S/src/SVNS_S.cpp:63-92` | Troca melhor-melhoria por posição, recálculo **total** por candidato, **passe único** | SVNS_S |
| 14 | `svns-s-insertion` | `SVNS_S/src/SVNS_S.cpp:94-117` | Inserção, passe único | SVNS_S |
| 15 | `svns-d-swap` | `SVNS_D/src/SVNS_D.cpp:61-94` + driver de `solve()` | Mesma troca, mas recálculo **parcial**; a função em si é um passe único — o driver genérico do harness (`run_ref_to_budget`, o mesmo usado por toda entrada aqui) a **itera** (reembaralhando a referência) até não melhorar mais ou estourar `--time-limit-ms` | SVNS_D |
| 16 | `svns-d-insertion` | `SVNS_D/src/SVNS_D.cpp:96-120` + driver | Mesmo padrão iterativo, vizinhança de inserção | SVNS_D |
| 17 | `hvns-best-insertion` | `HVNS/src/HVNS.cpp:268-303` (≡ `speed-ups`'s `EdgeInsertion::single_insertion_local_search`) | Inserção de 1 job, melhor-melhoria | HVNS |
| 18 | `hvns-best-edge-insertion` | `HVNS/src/HVNS.cpp:305-341` (≡ `edge_insertion_local_search`) | Inserção de bloco de 2 jobs adjacentes, melhor-melhoria | HVNS |
| 19 | `hvns-best-swap` | `HVNS/src/HVNS.cpp:343-377` (≡ `swap_local_search_best`) | Troca melhor-melhoria, recálculo parcial | HVNS |
| 20 | `hvns-shaking` | `HVNS/src/HVNS.cpp:379-397` + ciclo externo de `solve()` (~500-555) | Ciclo de restart VNS k=1..3 sobre {#17, #18, #19}, reinicia em k=1 a cada melhoria — **sem** a fase SA intercalada (essa é #21/#22); termina por `--time-limit-ms` | HVNS |
| 21 | `hvns-sa-rls` | `HVNS/src/HVNS.cpp:399-435` | RLS com aceitação de Metropolis e resfriamento geométrico; reembaralha a referência a cada movimento aceito; termina por `--time-limit-ms` | HVNS |
| 22 | `hvns-sa-edge-insertion` | `HVNS/src/HVNS.cpp:437-482` | Mesma aceitação SA, vizinhança de inserção de bloco. Não existe `sa_best_swap` correspondente no repositório — não foi inventado um aqui; termina por `--time-limit-ms` | HVNS |
| 23 | `tpa-sa` | `TPA/src/SimulatedAnnealing.cpp:123-136` (`anneal`) + laço de `:64-101` | Remoção+reinserção de 1 job via `NEH::mtaillard_best_insertion`, aceitação SA, resfriamento geométrico; termina por `--time-limit-ms` | TPA |

**Excluído explicitamente**: a variante de RLS do `TPA/src/local-search/RLS.cpp`
(usa `j = (j + 1) % ref.size()` em vez de `% instance.num_jobs()` — decisão
tomada com o autor deste catálogo, ver "Deduplicação").

## Deduplicação

O repositório tem **16 cópias físicas** da família RLS (uma por algoritmo:
DE_ABC, DE_PLS, DIWO, HDDE, hmgHS, IG_IJ, IG_RIS, IG_VND1, IG_VND2, MA, MFFO,
P_EDA, speed-ups, SVNS_S, TPA), mas só **6 comportamentos distintos**. Como
foram tratadas:

- **Cluster "partial-recalc"** — `MA`, `DIWO`, `MFFO`, `SaDIWO`, `HDDE`,
  `speed-ups` (e a metade `rls_grabowski` de `DE_ABC`) são byte-idênticos:
  viraram as entradas canônicas #1 `rls` / #2 `rls-grabowski`.
- **Cluster "full-recalc"** — `IG_IJ`, `IG_RIS`, `IG_VND1`, `IG_VND2` têm um
  `rls`/`rls_grabowski` que usa `core::recalculate_solution` (completo) em vez
  de `partial_recalculate_solution` após um movimento aceito. **Não** virou
  entrada separada — comportamentalmente equivalente ao cluster partial-recalc
  para fins de qualidade da busca (mesma sequência de movimentos, só recálculo
  mais caro), então é coberto por #1/#2.
- **Cluster "curto" (sem `rls_grabowski`)** — `DE_PLS`, `hmgHS`, `P_EDA`,
  `SVNS_S` só têm o `rls` simples, idêntico ao das entradas #1/#2 (mesmo corpo,
  sem o `break` do laço de erase de `rls_grabowski` — isso não é uma
  divergência de comportamento entre cópias, é só a diferença entre as duas
  funções `rls` e `rls_grabowski`, presente identicamente em toda cópia).
  Coberto por #1.
- **`DE_ABC`** — sua metade `rls_grabowski` é idêntica ao cluster
  partial-recalc (coberta por #2), mas seu `rls` próprio (linhas 185-213) é
  estruturalmente diferente (assinatura sem `ref`, passe único) → entrada
  separada #3.
- **`P_EDA`** — `mrls` reembaralha a referência periodicamente, único no
  repositório → entrada separada #4.
- **`TPA`** — seu `rls` usa `% ref.size()` em vez de `% instance.num_jobs()`
  no laço cíclico. Como normalmente `ref` tem o mesmo tamanho de
  `instance.num_jobs()` isso quase sempre coincide, mas é uma divergência de
  código real (comportamento diferente se algum dia `ref` for mais curto) —
  **excluído** do catálogo por decisão explícita, para não conflitar com as
  entradas #1-#4 sem agregar uma versão genuinamente nova de comportamento.

`BestSwap` segue o mesmo padrão: `IG_IJ`, `IG_VND1`, `IG_VND2` têm o *mesmo*
código (byte-idêntico, bug incluído) → uma única entrada #5, mais a #6 como
comparação corrigida.

## Bugs e peculiaridades conhecidas do repositório

- **`BestSwap` é um no-op silencioso** (`IG_IJ/src/IG_IJ.cpp:61`, idêntico em
  `IG_VND1.cpp`/`IG_VND2.cpp`):
  ```cpp
  std::swap(copy.sequence[i], copy.sequence[j]);
  core::partial_recalculate_solution(m_instance, copy, i);   // muta copy.cost
  if (solution.cost <= best_cost) {                          // BUG: devia ser copy.cost
      best_cost = solution.cost; best_j = j; best_i = i;
  }
  ```
  `solution.cost` é o custo *original*, constante durante toda a função.
  Como `best_cost` começa igual a `solution.cost`, a condição é **sempre**
  verdadeira, então `best_i`/`best_j` só acabam apontando para o último par
  testado, e o guard final `best_cost < original_cost` nunca passa —
  `BestSwap` nunca aplica nenhuma troca, em nenhuma das 3 cópias. Consequência
  direta: o passo k=2 de `vnd1` nunca contribui (degrada a um `rls` isolado) e
  o k=1 de `vnd2` é morto (degrada a um no-op seguido de `rls`). Reproduzido
  fielmente em `best-swap-ig` (#5); `best-swap-ig-fixed` (#6) existe só para
  comparação lado a lado. Pelo mesmo motivo, `vnd1-fixed`/`vnd2-fixed` (#10/#11)
  existem só para comparação — são `vnd1`/`vnd2` trocando `best-swap-ig` por
  `best-swap-ig-fixed`, e mostram o ganho de qualidade que o VND *teria* se a
  troca funcionasse: em `J20M5N1`, por exemplo, `vnd1-fixed`/`vnd2-fixed` chegam
  a ~1399-1400 de custo final contra 1447 de `rls`/`vnd1`/`vnd2`.
- **`getchar()` órfão em `IG_VND1/src/IG_VND1.cpp:93`** — um
  `std::cout << current << std::endl; getchar();` de debug deixado logo após
  a construção da solução inicial, que trava qualquer execução não-interativa
  de `IG_VND1`. Não é reproduzido aqui (a entrada `vnd1` porta só o laço k=1..2
  de `solve()`, que fica depois desse trecho).
- **Resolução de `uptime()` inconsistente entre origens** — `SVNS_S` mede em
  milissegundos, enquanto `SVNS_D`, `HVNS` e `TPA` medem em segundos. Como
  `svns-d-*`, `hvns-sa-*` e `tpa-sa` aqui usam seus próprios orçamentos de
  tempo sintéticos (ver "O harness"), essa inconsistência da origem não se
  propaga para este catálogo.

## Solução inicial

Todas as 23 entradas partem da **mesma permutação aleatória** por instância —
não de uma construção NEH. Gerada uma vez em `main.cpp`: `RNG` semeada com
`--seed` (padrão 42), embaralha `0..n-1`, recalcula custo. Cada entrada recebe
uma cópia fresca dessa solução, garantindo que `initial_cost` seja idêntico em
todas as linhas de uma mesma instância — a melhoria (`improvement_abs`/`_pct`)
e o tempo (`time_ms`) ficam diretamente comparáveis entre entradas.

## O harness

`./build/local-searchs-fair <instância>` roda cada busca local (por padrão,
todas) a partir da solução inicial compartilhada, sob o mesmo orçamento de
tempo, e mede custo inicial, custo final e tempo. Saída CSV (cabeçalho em
`stderr`, dados em `stdout`):

```
localsearch,n,m,iteration,iters,initial_cost,final_cost,improvement_abs,improvement_pct,time_ms,time_limit_ms,note
```

`run_all.py` varre as instâncias e reparte essas linhas (uma por repetição,
sem nenhuma agregação) em `results/<entrada>.csv` (colunas
`instance,iteration,initial_cost,final_cost,improvement_abs,improvement_pct,time_ms,time_limit_ms,note`
— sem `n`/`m`/`iters`); `output_to_excel.py` agrupa por instância, calcula o
RPD e consolida os `results/*.csv` num `output.xlsx` com uma aba **Resumo**
já ordenada por RPD médio (menor = melhor, funciona como ranking) + **uma
aba por entrada**
(`Instância | Custo inicial | Custo final (méd) | RPD (%) | Fonte RPD | Tempo médio/mín/máx (ms) | Orçamento (ms) | Uso do orçamento (%) | N iterações | Nota`).

**Repetição por entrada**: a cada uma das `--iters` repetições, o `RNG` é
reposicionado na mesma seed e a solução de trabalho é resetada para a cópia
inicial *antes* de rodar a entrada — diferente de `speed-ups/Bench.cpp` (que
cronometra uma função pura), cada busca local aqui muta seu estado
cumulativamente, então sem esse reset a repetição 2+ já começaria convergida.
Cada repetição gera sua própria linha (`iteration=1..iters`) com seu próprio
`time_ms` — nada é pré-calculado em média no C++, isso fica para
`output_to_excel.py`.

**Semântica de `reference`/`ref` por família** (importante para reprodutibilidade):

- Família RLS (#1-#4): vetor de **job ids**, reembaralhado via RNG antes de
  CADA passada pelo driver de orçamento (`run_ref_to_budget`) — não só uma
  vez por repetição como em `local-searchs/`.
- `vnd1`/`vnd2`/`vnd1-fixed`/`vnd2-fixed`/`ig-ij-dispatch` (#8-#12): começa em
  `initial.sequence`, como em `IG_VND1.cpp:95` (`reference = current.sequence`),
  mas — diferente de `local-searchs/`, onde nunca era reembaralhado — também
  passa a ser reembaralhado a cada passada do driver de orçamento, para que
  estas entradas consigam usar um orçamento maior do que uma única passada.
- `svns-s-*`/`svns-d-*` (#13-#16): vetor de **posições** `0..n-1`,
  reembaralhado a cada passada, como em `SVNS_S.cpp:186-191`/`SVNS_D.cpp:171-172,191`.
- `hvns-sa-*` (#21-#22): `T_init`/`T_fin`/`beta` calculados uma vez por
  chamada a partir da instância + `--hvns-n-iter`, com a mesma fórmula do
  construtor `HVNS::HVNS()` (`HVNS.cpp:32-35`).
- `tpa-sa` (#23): retorna a *melhor* solução encontrada (`best`), que pode
  divergir de `current` porque SA aceita pioras — `final_cost` reporta
  `best.cost`, e `note` registra `current.cost` também.

**Orçamento de tempo único (`--time-limit-ms`, default 2000ms)** — toda
entrada roda sob o MESMO teto, ao contrário de `local-searchs/` (onde
`svns-d-*` usava `n*m` ms, `hvns-*` usava `--hvns-time-limit-ms` e `tpa-sa`
usava `--tpa-time-limit-ms`, três orçamentos diferentes e um deles
dependente do tamanho da instância). As entradas que já tinham um laço
interno de "rodar até o tempo acabar" (`svns-d-*`, `hvns-shaking`,
`hvns-sa-*`, `tpa-sa`) simplesmente recebem esse valor único em vez da flag
antiga. As demais (#1-#4, #7-#12, #13-#14, #17-#19) passam a rodar dentro do
mesmo tipo de laço via `run_ref_to_budget`/`run_call_to_budget` em
`Harness.cpp` — reembaralha a ordem de varredura e roda de novo até uma
passada não melhorar ou o orçamento acabar. Quando não há nada para
reembaralhar (`best-swap-ig` e as outras sem `reference`), o "reembaralhar" é
simplesmente omitido; nesse caso, repetir a chamada determinística quase
sempre converge em 1-2 passadas bem abaixo do orçamento — o que é o ponto:
a coluna `passes=N` em `note` e `time_ms`/`time_limit_ms` deixam isso visível
em vez de escondido atrás de uma comparação que parecia neutra.

**`vnd1-fixed`/`vnd2-fixed`, ao contrário de `local-searchs/`, agora têm
teto** — lá, essas duas rodavam sem orçamento sintético de propósito (decisão
documentada: preferia-se ver a convergência real do VND corrigido, o que
podia levar ~11,6 minutos numa chamada em `J500M20N1`). Aqui isso deixaria de
ser uma comparação justa com as outras 22 entradas, então elas também
respeitam `--time-limit-ms` como qualquer outra — se quiser reproduzir a
convergência real sem teto, use `local-searchs/` ou passe um
`--time-limit-ms` bem maior.

**Limite conhecido: o orçamento só é checado ENTRE passadas, não dentro de
uma** — `run_ref_to_budget`/`run_call_to_budget` olham o relógio antes de
cada passada nova, não durante. Para a maioria das entradas uma passada é
barata (RLS é O(n·m), por exemplo) e isso não importa. Mas `best-swap-ig`/
`best-swap-ig-fixed`/`vnd1-fixed`/`vnd2-fixed` avaliam a vizinhança BestSwap
inteira por passada, O(n³·m) — numa instância `J500`, uma ÚNICA passada pode
estourar `--time-limit-ms` em vários minutos antes do driver sequer ter a
chance de checar o relógio de novo. Ou seja, "todo mundo recebe o mesmo
orçamento" é verdade no nível de "quantas passadas cabem", mas não protege
contra uma passada individual desproporcionalmente cara — isso é uma
propriedade do próprio algoritmo (mesma razão pela qual `local-searchs/`
deixava `vnd1-fixed`/`vnd2-fixed` sem teto), não um bug do driver. Rodar
`run_all.py --all` (ou mesmo o default `*N1`, que inclui `J500M5/M10/M20`)
com essas entradas habilitadas pode demorar bem mais que
`--time-limit-ms` sugere; se quiser um sweep rápido, use `-p` para excluir
essas três nas instâncias grandes, ou rode-as separadamente.

### Uso

```sh
meson setup build
ninja -C build

./build/local-searchs-fair instances/J100M20/J100M20N1              # todas as entradas
./build/local-searchs-fair instances/J500M20/J500M20N1 -p rls-grabowski
./build/local-searchs-fair instances/J100M10/J100M10N1 -n 20 -s 1    # 20 repetições, seed 1
./build/local-searchs-fair instances/J100M10/J100M10N1 --time-limit-ms 5000  # orçamento maior p/ todos

python3 -m venv .venv && .venv/bin/pip install openpyxl   # (uma vez) dep do xlsx
python3 run_all.py                                        # varre instâncias -> results/<entrada>.csv
.venv/bin/python output_to_excel.py                        # results/*.csv -> output.xlsx (com RPD)
```

Opções do binário: `-p/--local-search <nome>`, `-n/--iters <k>`,
`-s/--seed <s>` (padrão 42), `-v`, `--jp`, `--hvns-n-iter`, `--tpa-n-iter`,
`--tpa-final-temp`, `--time-limit-ms` (padrão `2000`, compartilhado por toda
entrada).
Opções do `run_all.py`: `--all` (todas as instâncias, não só `*N1`), `--iters`,
`--seed`, `--no-build`, `--results-dir`, `--time-limit-ms`.
Opções do `output_to_excel.py`: `[results_dir]`, `--output`, `--literature`
(default `../best_ribas.json`).

## Mapa de arquivos

```
include/ src/                 infra copiada de local-searchs/ (Core, Instance, Solution, RNG, Log)
  constructions/NEH.*         única construção necessária - dependência interna de rls/mrls/svns/tpa-sa
  local-search/
    RLS.*, Grabowski.*,       #1, #2, #7, #17-19 - copiados verbatim de local-searchs/
    Swap.*, EdgeInsertion.*
    RlsDeAbc.*                #3
    MrlsPEda.*                #4
    BestSwapIg.*              #5, #6
    Vnd.*                     #8, #9, #10, #11, #12
    SvnsS.*                   #13, #14
    SvnsD.*                   #15, #16 (só as funções de passe único - o driver de orçamento vive em Harness.cpp)
    Hvns.*                    #20, #21, #22
    Tpa.*                     #23
  localsearchs/
    Harness.*                 registry das 23 entradas + run_ref_to_budget/run_call_to_budget (orçamento
                               compartilhado) + laço de repetição/reset + serialização CSV
src/main.cpp                  driver CLI - gera a solução inicial compartilhada
run_all.py                    varre instâncias -> results/<entrada>.csv (uma linha por iteração)
output_to_excel.py            results/*.csv -> output.xlsx (RPD + Resumo ranqueado + aba por entrada)
```

Nenhum arquivo fora de `local-searchs-fair/` foi alterado — as cópias
canônicas continuam nas metaheurísticas de origem, e `local-searchs/`
continua existindo intacto ao lado desta pasta.
