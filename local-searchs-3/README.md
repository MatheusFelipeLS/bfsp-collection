# local-searchs-3

Terceira forma de avaliar as buscas locais do catálogo de
`local-searchs/`/`local-searchs-fair/` (aqui reduzido a **15 entradas ativas**,
incluindo a `hyprr-vnd` acrescentada depois — ver "As entradas") — em vez de rodar cada uma isolada
(com ou sem orçamento de tempo igual), esta pasta planta cada uma, uma de
cada vez, **dentro do loop de uma metaheurística real** (o IG_IJ) no lugar do
passo de busca local que ele já usa, e mede como isso afeta a convergência do
IG_IJ como um todo. A pergunta que as outras duas pastas respondem é "qual
busca local é melhor sozinha"; esta responde "qual busca local faz uma
metaheurística de verdade convergir mais rápido e chegar mais longe".

## O que está sendo medido

`IG_IJ::solve()` (`IG_IJ/src/IG_IJ.cpp:85-161`) é um Iterated Greedy
clássico: destrói um pedaço da solução, reconstrói via NEH, aplica **uma**
busca local, e aceita ou rejeita o resultado com um critério estilo
Metropolis (temperatura constante, não resfriada) — repete até o tempo
acabar. O passo de busca local (`IG_IJ.cpp:126-131`) hoje é fixo: com
probabilidade `jp` roda `BestSwap` (o `best-swap-ig` do catálogo, bugado),
senão roda `rls`. Esta pasta generaliza esse único ponto de troca para
aceitar **qualquer uma das entradas do registry** — tudo o mais (tamanho
de destruição, temperatura de aceitação, construção inicial via PF_NEH,
orçamento de tempo) fica fixo e idêntico entre as rodadas, então a única
variável é qual busca local ocupa o slot.

Para cada (entrada, instância, repetição), o host mede:

- **`n_calls`** — quantas vezes o IG chamou a busca local dentro do
  orçamento de tempo (throughput: buscas mais caras por chamada cabem menos
  vezes no mesmo tempo).
- **`avg_improvement_per_call`** — melhoria média de custo (antes → depois
  da busca local, medida ANTES do critério de aceitação decidir se o
  resultado fica ou não) a cada chamada. Esta é a métrica "melhoria a cada
  vez que o algoritmo chama a local search" pedida na proposta original.
- **`call_of_best`** — em qual chamada o `best` global foi encontrado pela
  última vez. Comparado a `n_calls`, mostra se a busca local fez o IG
  convergir cedo e depois estagnar (`call_of_best` << `n_calls`) ou se
  continuou rendendo até o fim do orçamento.
- **`final_best_cost`** — o resultado que de fato importa: qual busca local,
  plugada no IG_IJ, chega mais perto do melhor custo possível. Reportado
  como **RPD** (não custo bruto) em `output_to_excel.py`, mesma convenção de
  `local-searchs-fair/`.
- Opcionalmente, a **trajetória completa** (custo antes/depois de cada
  chamada, aceito ou não, custo corrente/melhor após a decisão, tempo
  decorrido) — uma linha por chamada, só na repetição 1, para plotar a curva
  de convergência de uma entrada específica. Ver `trajectories/`.

> Entrada aqui = a lista de 23 buscas locais comportamentalmente distintas
> do catálogo original, **podada para 14**, mais a `hyprr-vnd` (ver abaixo). Ver
> `local-searchs-fair/README.md` para a tabela completa de
> origem/deduplicação.

## As entradas

Em relação ao catálogo original de 23 entradas, quatro foram removidas do
registry (`src/localsearchs/IgHost.cpp`, função `registry()`) e o código
correspondente apagado:

- `rls-grabowski` — descartada.
- `best-swap-ig`, `vnd1`, `vnd2` **bugadas** — como o `BestSwap` original
  nunca aplica nenhuma troca (ver `include/local-search/BestSwapIg.h`), essas
  três só mediam o efeito do bug. Saíram, e as versões corrigidas
  (`best-swap-ig-fixed`, `vnd1-fixed`, `vnd2-fixed`) **assumiram os nomes
  curtos** `best-swap-ig`, `vnd1`, `vnd2`. Ou seja: em `results/` e
  `trajectories/`, esses três nomes já são as versões **sem o bug**.

Depois disso, mais cinco entradas saíram do registry, mas só **comentadas**
em `registry()` — o código (`bench_*`) continua lá, e basta descomentar a
linha para voltar a rodá-las:

- `svns-s-insertion`, `svns-d-swap`, `svns-d-insertion`, `hvns-best-swap`
  — retiradas do sweep; os CSVs delas em `results/` foram apagados (as
  trajetórias antigas continuam em `trajectories/`). O `hvns-best-swap`
  continua sendo usado **internamente** pelo `hvns-shaking` (k=3).
- `tpa-sa` — ver "`tpa-sa`" abaixo.

Na tabela, essas entradas estão marcadas como *fora do registry*.

| # | Entrada | Origem (arquivo:linhas) | O que faz | Algoritmos de origem |
|---|---------|--------------------------|-----------|------------------------|
| 1 | `rls` | `speed-ups/src/local-search/RLS.cpp:64-94` | Busca local de inserção (Taillard), converge (`cnt` reseta a cada melhoria) | MA, DIWO, MFFO, SaDIWO, HDDE, speed-ups |
| 2 | `rls-de-abc` | `DE_ABC/src/local-search/RLS.cpp:185-213` | Assinatura própria `(Solution&, Instance&)` (sem `ref`, construída internamente); passe único | DE_ABC |
| 3 | `mrls-p-eda` | `P_EDA/src/P_EDA.cpp:37-83` | Igual a `rls`, mas reembaralha `ref` via RNG sempre que o índice cíclico dá a volta | P_EDA |
| 4 | `best-swap-ig` | `IG_IJ/src/IG_IJ.cpp:40-82` (≡ IG_VND1, IG_VND2), **com o bug corrigido** (compara `copy.cost`, não `solution.cost`) — implementado em `best_swap_ig_fixed` | Troca melhor-melhoria | IG_IJ, IG_VND1, IG_VND2 |
| 5 | `swap-first-ig` | `IG/src/IG.cpp:109-144` | Troca primeira-melhoria, converge | IG |
| 6 | `vnd1` | `IG_VND1/src/IG_VND1.cpp:129-143`, com #4 corrigido (`vnd1_fixed`) | VND: k=1 `rls` → k=2 `best-swap-ig` | IG_VND1 |
| 7 | `vnd2` | `IG_VND2/src/IG_VND2.cpp:129-143`, com #4 corrigido (`vnd2_fixed`) | VND: k=1 `best-swap-ig` → k=2 `rls` | IG_VND2 |
| 8 | `ig-ij-dispatch` | `IG_IJ/src/IG_IJ.cpp:118-133` | **A entrada baseline**: `RNG(0,1) < jp → best-swap-ig, senão rls`. Ver "A entrada baseline" | IG_IJ |
| 9 | `svns-s-swap` | `SVNS_S/src/SVNS_S.cpp:63-92` | Troca melhor-melhoria por posição, recálculo total, passe único | SVNS_S |
| 10 | `svns-s-insertion` | `SVNS_S/src/SVNS_S.cpp:94-117` | Inserção, passe único — *fora do registry* | SVNS_S |
| 11 | `svns-d-swap` | `SVNS_D/src/SVNS_D.cpp:61-94` | Mesma troca, recálculo parcial — *fora do registry* | SVNS_D |
| 12 | `svns-d-insertion` | `SVNS_D/src/SVNS_D.cpp:96-120` | Inserção, recálculo parcial — *fora do registry* | SVNS_D |
| 13 | `hvns-best-insertion` | `HVNS/src/HVNS.cpp:268-303` | Inserção de 1 job, melhor-melhoria | HVNS |
| 14 | `hvns-best-edge-insertion` | `HVNS/src/HVNS.cpp:305-341` | Inserção de bloco de 2 jobs, melhor-melhoria | HVNS |
| 15 | `hvns-best-swap` | `HVNS/src/HVNS.cpp:343-377` | Troca melhor-melhoria, recálculo parcial — *fora do registry* (só usada dentro de #16) | HVNS |
| 16 | `hvns-shaking` | `HVNS/src/HVNS.cpp:379-397` | Ciclo de restart VNS k=1..3 sobre {#13,#14,#15}, converge sozinho — ver "Entradas HVNS" | HVNS |
| 17 | `hvns-sa-rls` | `HVNS/src/HVNS.cpp:399-435` | RLS com aceitação Metropolis + resfriamento geométrico, para após n rejeições seguidas | HVNS |
| 18 | `hvns-sa-edge-insertion` | `HVNS/src/HVNS.cpp:437-482` | Mesma aceitação SA, inserção de bloco | HVNS |
| 20 | `hyprr-vnd` | `hyprr/` commit `0cec09d`, `HyPRR/LocalSearch.cpp` (`runSearchTotal`, `relocate`, `twoOpt`, `swap`) — port em `HyprrLS` (cópia de `speed-ups/src/local-search/HyprrLS.cpp`) | VND de primeira melhoria: inserção → inserção em bloco x=2, 3, 4 → troca (sem troca quando n = 500), volta à 1ª vizinhança a cada melhoria; desempate por tempo ocioso+bloqueado; Taillard generalizado + poda pelo caminho crítico (block-skip + LB de Ding na inserção). Ver "`hyprr-vnd`" | HyPRR |
| 19 | `tpa-sa` | `TPA/src/SimulatedAnnealing.cpp:123-136` | Remoção+reinserção via NEH, aceitação SA — *fora do registry*, ver "`tpa-sa`" | TPA |

Deduplicação, bugs conhecidos (o no-op de `BestSwap`, o `getchar()` órfão do
`IG_VND1` original, etc.) e a justificativa de exclusão da variante de RLS do
TPA são exatamente as de `local-searchs-fair/README.md` — não repetidas aqui
para não divergir por edição em só um lugar.

## A entrada baseline: `ig-ij-dispatch`

Diferente de `local-searchs-fair/`, aqui o "hospedeiro" (IG_IJ) já tinha,
antes desta pasta existir, uma escolha própria de busca local — exatamente o
que virou a entrada `ig-ij-dispatch`. Todas as OUTRAS 14 entradas ativas são a
pergunta real: "e se, no lugar da escolha original do IG_IJ, eu usasse esta
outra busca local?".

**Atenção**: como o `BestSwap` bugado foi removido, o ramo de troca do
`ig-ij-dispatch` agora chama a versão **corrigida** (`best_swap_ig_fixed`,
`src/local-search/Vnd.cpp`). Logo, ele não é mais byte-a-byte o passo do
`IG_IJ` original — é o IG_IJ "como deveria ser". Na prática a diferença é
pequena: com `jp = 0.001` (default), o ramo de troca é sorteado em ~0,1% das
chamadas e o resto é `rls`.

## Construção inicial

Diferente de `local-searchs/`/`local-searchs-fair/` (permutação aleatória),
aqui a solução inicial de CADA run é construída via `PF_NEH`
(`IG_IJ/src/constructions/PF_NEH.h`, copiado verbatim) — a mesma construção
que `IG_IJ::solve()` usa originalmente (`IG_IJ.cpp:91-93`, `lambda = n>200 ?
20 : n`). Isso é proposital: o ponto desta pasta é "qual busca local encaixa
melhor no IG_IJ tal como ele é", então o ponto de partida também precisa ser
o dele, não um genérico.

## O host loop (`src/localsearchs/IgHost.cpp`)

Porta `IG_IJ::solve()` quase linha a linha — `destroy`,
`acceptance_criterion`, a condição de parada por tempo, a lógica de
aceitar/rejeitar/atualizar `best` são as MESMAS (mesma ordem, incluindo o
detalhe de que a chamada que estoura o orçamento de tempo é descartada sem
passar por aceitação, igual ao original `IG_IJ.cpp:138-141`). A única
mudança estrutural é o parâmetro extra `step` no lugar do bloco
`if/else(BestSwap/rls)` hardcoded.

**Referência (`reference`) usada pela busca local**: igual ao IG_IJ original
(`reference = current.sequence`, a sequência de jobs da construção PF_NEH,
**fixa durante toda a rodada** — nunca reembaralhada), para as entradas #1-3,
#6-8 (família RLS/VND/dispatch, que espera um vetor de **job ids**). As
entradas #9-12 (família SVNS, das quais só a #9 está ativa; espera um vetor de **posições** — ver
`SvnsS.h`/`SvnsD.h`) mantêm sua própria referência separada, também fixa
durante a rodada: `svns-s-*` começa embaralhada (`random_perm`), `svns-d-*`
começa em ordem (`iota`), replicando o setup que `local-searchs-fair/` já
usava para essas duas famílias.

### Entradas HVNS (`hvns-shaking`, `hvns-sa-rls`, `hvns-sa-edge-insertion`)

Rodam **sem orçamento de tempo por chamada**: cada chamada vai até o
critério de parada natural da origem, como as outras entradas.

- `hvns-shaking`: VND sobre {#13, #14, #15} (k=1..3, volta a k=1 quando
  melhora). Termina sozinho, já que cada reinício reduz o custo
  estritamente. Observação: no HVNS original, `shaking(s, k)` aplica **uma**
  vizinhança por chamada (o laço de k fica em `solve()`); aqui ela virou o
  ciclo completo. Em instâncias grandes cada chamada é cara (o `best_swap`
  é O(n³m)), então `n_calls` fica baixo.
- `hvns-sa-rls`/`hvns-sa-edge-insertion`: param depois de n tentativas
  rejeitadas seguidas (`cnt < n`), igual a `HVNS.cpp:399-482`. A temperatura
  `T` é persistida entre chamadas dentro de uma rodada (resetada a cada
  repetição), como o `m_T` do HVNS, então o resfriamento continua de uma
  chamada do IG para a próxima. Devolvem o melhor visitado (`best`), não o
  `current` final.

Antes essas três recebiam um burst de `--slot-time-limit-ms` (20 ms). Ele
foi removido: no original nenhuma delas tem orçamento de tempo, e o valor
fixo não escalava com a instância (cortava as SA antes da convergência em
J500 e não era respeitado pelo `hvns-shaking`, que só checava o prazo entre
vizinhanças). **Os resultados em `results/` gerados antes dessa mudança não
são comparáveis para estas três entradas.**

### `hyprr-vnd`

Busca local completa do HyPRR (Teixeira, Subramanian & Kramer, `hyprr/articles/HyPRR.pdf`,
Sec. 4.2), portada do código original dos autores como entrou em `hyprr/` (commit `0cec09d`,
igual a `hyprr/versions/HyPRR2`) — **não** de `hyprr/versions/HyPRR`, uma variante posterior que
embaralha a ordem de varredura e põe a troca como 2ª vizinhança. No HyPRR ela é aplicada a cada
indivíduo novo depois do ruin-and-recreate; aqui ocupa o slot do IG_IJ. Cada chamada roda o VND até
convergir; ignora `reference`.

- Vizinhanças, na ordem: inserção, inserção em bloco com x = 2, 3, 4 e troca (`nViz` = 5, ou 4
  quando n = 500, como no original). Cada uma é uma varredura de primeira melhoria: aplica o
  movimento na hora e segue varrendo a partir da solução nova.
- Aceitação: makespan menor, ou igual com menor `T` (soma dos tempos ociosos + bloqueados).
- Aceleração: Taillard generalizado para bloco e troca; poda por block-skip (inserção e troca)
  e pela LB de Ding et al. (2016) na inserção. A LB de troca do artigo fica **desligada**, como no
  código original (lá a chamada está comentada). Ver `speed-ups/README.md` (#5 e #6).
- O port foi verificado contra o binário original: mesma sequência e mesmo makespan em 150
  varreduras isoladas e 25 VNDs completos.

Essa entrada foi adicionada depois do sweep principal e rodada à parte (`python3 run_all.py
--all -p hyprr-vnd`; sem `-p`, o `run_all.py` reroda todas as entradas). O
`results/hyprr-vnd.csv` versionado cobre **120 instâncias** (1200 linhas): não tem J200M5,
J500M5 nem J500M10 — exatamente os grupos que o `output_to_excel.py` ignora (ver "Saída do
binário"), então a comparação com as outras entradas no xlsx é sobre o mesmo conjunto.

### `tpa-sa` (fora do registry)

O SA do TPA não é uma busca local: no TPA ele é a própria metaheurística
(MNEH + SA), com o orçamento de tempo inteiro e resfriamento calibrado para
`n_iter` = 1,8M passos. Como não tem parada natural por chamada, plugado no
IG ele precisa de um burst de `--slot-time-limit-ms`, e com esse burst nunca
chega a esfriar (além de reiniciar `T` em toda chamada). Por isso a linha
está comentada no `registry()`. O código continua lá, e o
`--slot-time-limit-ms` só tem efeito sobre ele.

## Orçamento de tempo: `10 · n · m` ms

O orçamento de tempo de parede de cada rodada do IG **não** vem mais de
`--time-limit-ms`: está fixado no código em `10 · n · m` milissegundos
(`src/localsearchs/IgHost.cpp`, `run_ig_once`; a linha que lia
`params.time_limit_ms()` está comentada). Exemplos: J20M5 → 1 s,
J100M20 → 20 s, J500M20 → 100 s. A coluna `time_limit_ms` do CSV reporta
esse valor efetivo.

Consequência: `--time-limit-ms` (no binário e no `run_all.py`) continua
sendo aceito, mas **é ignorado**. Para voltar a um orçamento fixo, é preciso
descomentar a linha em `IgHost.cpp`.

Custo de um sweep completo (`--all`, 150 instâncias, 10 repetições, 15
entradas): soma de `10·n·m` sobre as instâncias (≈ 3045 s) × 150 ≈ 5,3 dias de
CPU em série; daí a opção `-j` do `run_all.py`.

## Saída do binário

`./build/local-searchs-3 <instância>` roda cada entrada (por padrão, todas)
plugada no host loop, por `--iters` repetições. Duas famílias de linha CSV no
`stdout` (cabeçalhos em `stderr`), disambiguadas pelo primeiro campo:

```
summary,localsearch,n,m,iteration,iters,seed,initial_cost,final_best_cost,n_calls,call_of_best,avg_improvement_per_call,time_ms,time_limit_ms,note
trajectory,localsearch,call_index,cost_before_ls,cost_after_ls,accepted,current_cost,best_cost,elapsed_ms
```

`summary`: uma linha por (entrada, repetição) — como o IG inteiro se saiu.
`trajectory`: uma linha por chamada da busca local — só na repetição 1 de
cada entrada, até `--trajectory-max-rows` (default 2000; `--no-trajectory`
desliga completamente).

`run_all.py` varre as instâncias e separa essas duas famílias em
`results/<entrada>.csv` (resumos, uma linha por repetição) e
`trajectories/<entrada>__<instância>.csv` (trajetórias brutas, para inspeção
manual/plot — não são agregadas por `output_to_excel.py`).
`output_to_excel.py` agrupa os resumos por instância, calcula RPD (mesma
convenção de `local-searchs-fair/`: melhor conhecido da literatura via
`../best_ribas.json` quando disponível, senão o melhor obtido nesta própria
rodada) e monta `local_searchs_results.xlsx` com uma aba **Resumo** ranqueada por RPD
médio + uma aba por entrada. `../xlsx_to_md.py` converte esse xlsx em
`../outputs/local_searchs_results.md`.

**Grupos ignorados na agregação**: o `output_to_excel.py` descarta as instâncias de
**J200M5, J500M5 e J500M10** (`IGNORED_GROUPS`) — elas não entram em nenhuma aba, nem no RPD,
nem no Resumo, mesmo que estejam nos CSVs. Restam 12 grupos × 10 = 120 instâncias por entrada.
O xlsx também não tem mais as colunas "Fonte RPD" (abas por entrada) e "N instâncias" (Resumo).

**Repetições (`--iters`)**: cada repetição roda o IG inteiro do zero
(mesma seed, reconstrução PF_NEH do zero) — como a condição de parada é por
tempo de parede, não por número de iterações, há alguma variância natural em
`n_calls`/`final_best_cost` entre repetições vindo puramente de ruído de
agendamento do SO, mesmo com a mesma seed. É por isso que `--iters` > 1
continua útil aqui, ao contrário do que se poderia pensar de um processo
"determinístico dado o seed".

## Uso

```sh
meson setup build
ninja -C build

./build/local-searchs-3 instances/J100M20/J100M20N1                       # todas as entradas
./build/local-searchs-3 instances/J500M20/J500M20N1 -p rls
./build/local-searchs-3 instances/J100M10/J100M10N1 -n 20 -s 1            # 20 repetições, seed 1
./build/local-searchs-3 instances/J100M10/J100M10N1 --no-trajectory       # sem trajectory, mais rápido/leve

python3 -m venv .venv && .venv/bin/pip install openpyxl   # (uma vez) dep do xlsx
python3 run_all.py                                        # só instâncias *N1 -> results/ + trajectories/
python3 run_all.py --all -j 4                             # todas as 150 instâncias, 4 em paralelo
.venv/bin/python output_to_excel.py                        # results/*.csv -> local_searchs_results.xlsx
```

Opções do binário: `-p/--local-search <nome>`, `-n/--iters <k>` (padrão
10), `-s/--seed <s>` (padrão 42), `-v`, `--jp` (só usado por `ig-ij-dispatch`),
`--hvns-n-iter`, `--tpa-n-iter`, `--tpa-final-temp`, `-dS/--destroy` (padrão
8, tamanho de destruição do IG), `-tP/--temperature` (padrão 0.5, fator de
temperatura do critério de aceitação), `--time-limit-ms` (**ignorado**, ver
"Orçamento de tempo"), `--slot-time-limit-ms` (padrão 20, burst por
chamada do `tpa-sa`, que está fora do registry), `--trajectory-max-rows` (padrão 2000),
`--no-trajectory`.
Opções do `run_all.py`: `--all` (sem ele, só as instâncias `*N1`),
`--iters` (padrão 10), `--seed`, `--no-build`, `--results-dir`,
`--trajectories-dir`, `--time-limit-ms` (repassado, mas ignorado pelo
binário), `--slot-time-limit-ms`, `--trajectory-max-rows`, `--no-trajectory`,
`-p/--local-search <entrada>` (só essa entrada; só os arquivos dela são reescritos),
`-j/--threads` (padrão 1; execuções do binário em paralelo — pode afetar as
medições de tempo).
Opções do `output_to_excel.py`: `[results_dir]`, `--output` (default
`local_searchs_results.xlsx`), `--literature` (default `../best_ribas.json`).

## Resultados versionados

`results/` contém um CSV por entrada ativa (15 arquivos), todos com 10
repetições, seed 42, orçamento `10·n·m` ms:

- 14 entradas do sweep `--all` — 150 instâncias (J20–J500 × M5/M10/M20, 10
  por grupo), 1500 linhas cada;
- `hyprr-vnd` — 120 instâncias (sem J200M5/J500M5/J500M10), 1200 linhas.

`trajectories/` ainda guarda as trajetórias das 5 entradas que saíram do
registry (`svns-s-insertion`, `svns-d-swap`, `svns-d-insertion`,
`hvns-best-swap`, `tpa-sa`), além das 15 ativas (150 instâncias cada; 120 para
`hyprr-vnd`). Agregado em `local_searchs_results.xlsx` e
`../outputs/local_searchs_results.md` (120 instâncias por entrada, ver
"Grupos ignorados na agregação").

## Mapa de arquivos

```
include/ src/                 infra copiada de local-searchs-fair/ (Core, Instance, Solution, RNG, Log)
  constructions/
    NEH.*                     reconstrução (NEH::second_step) - dependência do host loop
    PF.*, PF_NEH.*            construção inicial - copiados verbatim de IG_IJ/ (não existiam nas outras pastas)
  local-search/                bodies de local-searchs-fair/, menos os removidos (rls-grabowski, BestSwap
                               bugado e os VNDs que o usavam); HyprrLS.* copiado de speed-ups/ (hyprr-vnd)
  localsearchs/
    IgHost.*                  host loop (destroy/reconstrói/chama a busca local/aceita) + registry das 15
                               entradas ativas como adaptadores "step" + laço de repetição + serialização CSV (summary
                               + trajectory)
src/main.cpp                  driver CLI
run_all.py                    varre instâncias -> results/<entrada>.csv + trajectories/<entrada>__<instância>.csv
output_to_excel.py            results/*.csv -> local_searchs_results.xlsx (RPD + convergência + Resumo ranqueado;
                               ignora J200M5/J500M5/J500M10)
results/, trajectories/       saída do último sweep (ver "Resultados versionados")
```

Nenhum arquivo fora de `local-searchs-3/` foi alterado — `IG_IJ/` e as
demais pastas continuam com suas cópias canônicas, e `local-searchs/`/
`local-searchs-fair/` continuam existindo intactas ao lado desta.
