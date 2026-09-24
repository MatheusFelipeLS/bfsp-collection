# local-searchs-3

Terceira forma de avaliar as **mesmas 23 buscas locais** do catálogo de
`local-searchs/`/`local-searchs-fair/` — em vez de rodar cada uma isolada
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
aceitar **qualquer uma das 23 entradas do catálogo** — tudo o mais (tamanho
de destruição, temperatura de aceitação, construção inicial via PF_NEH,
orçamento de tempo) fica fixo e idêntico entre as 23 rodadas, então a única
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

> Entrada aqui = a mesma lista de 23 buscas locais comportamentalmente
> distintas do catálogo original. Ver `local-searchs-fair/README.md` para a
> tabela completa de origem/deduplicação — reproduzida abaixo por
> conveniência, sem alterações no conteúdo dos algoritmos em si.

## As entradas

| # | Entrada | Origem (arquivo:linhas) | O que faz | Algoritmos de origem |
|---|---------|--------------------------|-----------|------------------------|
| 1 | `rls` | `speed-ups/src/local-search/RLS.cpp:64-94` | Busca local de inserção (Taillard), converge (`cnt` reseta a cada melhoria) | MA, DIWO, MFFO, SaDIWO, HDDE, speed-ups |
| 2 | `rls-grabowski` | mesmo arquivo, `:23-62` | `rls` restrita aos blocos críticos de Grabowski–Wodecki + recálculo parcial pós-movimento | idem |
| 3 | `rls-de-abc` | `DE_ABC/src/local-search/RLS.cpp:185-213` | Assinatura própria `(Solution&, Instance&)` (sem `ref`, construída internamente); passe único | DE_ABC |
| 4 | `mrls-p-eda` | `P_EDA/src/P_EDA.cpp:37-83` | Igual a `rls`, mas reembaralha `ref` via RNG sempre que o índice cíclico dá a volta | P_EDA |
| 5 | `best-swap-ig` | `IG_IJ/src/IG_IJ.cpp:40-82` (≡ IG_VND1, IG_VND2) | Troca melhor-melhoria. **Bug real**: compara `solution.cost` em vez de `copy.cost` → nunca aplica nenhuma troca. É exatamente o `BestSwap` que o `IG_IJ::solve()` original chama — aqui usado byte-a-byte, não reimplementado | IG_IJ, IG_VND1, IG_VND2 |
| 6 | `best-swap-ig-fixed` | idem, comparação corrigida (`copy.cost`) | Mesma vizinhança, sem o bug | — (versão de comparação) |
| 7 | `swap-first-ig` | `IG/src/IG.cpp:109-144` | Troca primeira-melhoria, converge | IG |
| 8 | `vnd1` | `IG_VND1/src/IG_VND1.cpp:129-143` | VND: k=1 `rls` → k=2 `best-swap-ig` (morto pelo bug de #5) | IG_VND1 |
| 9 | `vnd2` | `IG_VND2/src/IG_VND2.cpp:129-143` | VND: k=1 `best-swap-ig` (morto) → k=2 `rls` | IG_VND2 |
| 10 | `vnd1-fixed` | idem #8, com #6 no lugar de #5 | VND de #8 com a troca funcionando de verdade | — (versão de comparação) |
| 11 | `vnd2-fixed` | idem #9, com #6 no lugar de #5 | VND de #9 com a troca corrigida | — (versão de comparação) |
| 12 | `ig-ij-dispatch` | `IG_IJ/src/IG_IJ.cpp:118-133` | **A entrada baseline**: `RNG(0,1) < jp → best-swap-ig, senão rls` — reproduz exatamente o passo de busca local que `IG_IJ::solve()` já usa. Ver "A entrada baseline" | IG_IJ |
| 13 | `svns-s-swap` | `SVNS_S/src/SVNS_S.cpp:63-92` | Troca melhor-melhoria por posição, recálculo total, passe único | SVNS_S |
| 14 | `svns-s-insertion` | `SVNS_S/src/SVNS_S.cpp:94-117` | Inserção, passe único | SVNS_S |
| 15 | `svns-d-swap` | `SVNS_D/src/SVNS_D.cpp:61-94` | Mesma troca, recálculo parcial | SVNS_D |
| 16 | `svns-d-insertion` | `SVNS_D/src/SVNS_D.cpp:96-120` | Inserção, recálculo parcial | SVNS_D |
| 17 | `hvns-best-insertion` | `HVNS/src/HVNS.cpp:268-303` | Inserção de 1 job, melhor-melhoria | HVNS |
| 18 | `hvns-best-edge-insertion` | `HVNS/src/HVNS.cpp:305-341` | Inserção de bloco de 2 jobs, melhor-melhoria | HVNS |
| 19 | `hvns-best-swap` | `HVNS/src/HVNS.cpp:343-377` | Troca melhor-melhoria, recálculo parcial | HVNS |
| 20 | `hvns-shaking` | `HVNS/src/HVNS.cpp:379-397` | Ciclo de restart VNS k=1..3 sobre {#17,#18,#19} — **auto-orçada**, ver abaixo | HVNS |
| 21 | `hvns-sa-rls` | `HVNS/src/HVNS.cpp:399-435` | RLS com aceitação Metropolis + resfriamento geométrico — **auto-orçada** | HVNS |
| 22 | `hvns-sa-edge-insertion` | `HVNS/src/HVNS.cpp:437-482` | Mesma aceitação SA, inserção de bloco — **auto-orçada** | HVNS |
| 23 | `tpa-sa` | `TPA/src/SimulatedAnnealing.cpp:123-136` | Remoção+reinserção via NEH, aceitação SA — **auto-orçada** | TPA |

Deduplicação, bugs conhecidos (o no-op de `BestSwap`, o `getchar()` órfão do
`IG_VND1` original, etc.) e a justificativa de exclusão da variante de RLS do
TPA são exatamente as de `local-searchs-fair/README.md` — não repetidas aqui
para não divergir por edição em só um lugar; os algoritmos em si não mudaram,
só o hospedeiro em que rodam.

## A entrada baseline: `ig-ij-dispatch`

Diferente de `local-searchs-fair/`, aqui o "hospedeiro" (IG_IJ) já tinha,
antes desta pasta existir, uma escolha própria de busca local — exatamente o
que virou a entrada `ig-ij-dispatch`. Isso dá um baseline de sanidade
embutido: se `ig-ij-dispatch` rodar aqui e produzir números na mesma faixa
de um `IG_IJ` original não instrumentado (mesma seed/parâmetros), o host loop
está fiel ao original. Todas as OUTRAS 22 entradas são a pergunta real:
"e se, no lugar da escolha original do IG_IJ, eu usasse esta outra busca
local?".

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
detalhe de que a chamada que estoura `--time-limit-ms` é descartada sem
passar por aceitação, igual ao original `IG_IJ.cpp:138-141`). A única
mudança estrutural é o parâmetro extra `step` no lugar do bloco
`if/else(BestSwap/rls)` hardcoded.

**Referência (`reference`) usada pela busca local**: igual ao IG_IJ original
(`reference = current.sequence`, a sequência de jobs da construção PF_NEH,
**fixa durante toda a rodada** — nunca reembaralhada), para as entradas #1-4,
#8-12 (família RLS/VND/dispatch, que espera um vetor de **job ids**). As
entradas #13-16 (família SVNS, que espera um vetor de **posições** — ver
`SvnsS.h`/`SvnsD.h`) mantêm sua própria referência separada, também fixa
durante a rodada: `svns-s-*` começa embaralhada (`random_perm`), `svns-d-*`
começa em ordem (`iota`), replicando o setup que `local-searchs-fair/` já
usava para essas duas famílias.

### Entradas auto-orçadas

`hvns-shaking`, `hvns-sa-rls`, `hvns-sa-edge-insertion` e `tpa-sa` já são,
elas mesmas, pequenas metaheurísticas com laço próprio de "rodar até acabar
o tempo". Plugadas sem ajuste no slot do IG_IJ, a primeira chamada consumiria
sozinha o `--time-limit-ms` inteiro, e o IG nunca chamaria a busca local uma
segunda vez — inviabilizando qualquer medida de convergência ao longo de
várias chamadas. Por isso essas quatro recebem, a cada chamada, apenas um
**burst curto** (`--slot-time-limit-ms`, default 20ms) em vez do orçamento
inteiro:

- `hvns-shaking`/`tpa-sa`: cada chamada é independente, sem estado
  persistente entre chamadas (a assinatura de `tpa_anneal_sa` não expõe uma
  temperatura de entrada/saída) — `tpa-sa` reinicia seu próprio cronograma de
  resfriamento do zero em toda chamada, uma limitação conhecida desta
  assinatura, não deste harness.
- `hvns-sa-rls`/`hvns-sa-edge-insertion`: a temperatura `T` É persistida
  entre chamadas dentro de uma mesma rodada (resetada a cada repetição), já
  que a assinatura das funções aceita `T` por referência — então o
  resfriamento continua de onde parou a cada nova chamada do IG, em vez de
  reiniciar.

Isso significa que, para essas quatro entradas, "uma chamada da busca local"
não é "um passe até convergência" como nas outras 19 — é "uma rajada de
`--slot-time-limit-ms`". Documentado aqui para não confundir `n_calls`/
`avg_improvement_per_call` dessas quatro com as demais sem o contexto.

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
rodada) e monta `output.xlsx` com uma aba **Resumo** ranqueada por RPD médio
+ uma aba por entrada.

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
./build/local-searchs-3 instances/J500M20/J500M20N1 -p rls-grabowski
./build/local-searchs-3 instances/J100M10/J100M10N1 -n 20 -s 1            # 20 repetições, seed 1
./build/local-searchs-3 instances/J100M10/J100M10N1 --time-limit-ms 5000  # IG com orçamento maior
./build/local-searchs-3 instances/J100M10/J100M10N1 --no-trajectory       # sem trajectory,, mais rápido/leve

python3 -m venv .venv && .venv/bin/pip install openpyxl   # (uma vez) dep do xlsx
python3 run_all.py                                        # varre instâncias -> results/ + trajectories/
.venv/bin/python output_to_excel.py                        # results/*.csv -> output.xlsx (RPD + convergência)
```

Opções do binário: `-p/--local-search <nome>`, `-n/--iters <k>`, `-s/--seed
<s>` (padrão 42), `-v`, `--jp` (só usado por `ig-ij-dispatch`),
`--hvns-n-iter`, `--tpa-n-iter`, `--tpa-final-temp`, `-dS/--destroy` (padrão
8, tamanho de destruição do IG), `-tP/--temperature` (padrão 0.5, fator de
temperatura do critério de aceitação), `--time-limit-ms` (padrão 2000,
orçamento do IG inteiro), `--slot-time-limit-ms` (padrão 20, orçamento por
chamada das 4 entradas auto-orçadas), `--trajectory-max-rows` (padrão 2000),
`--no-trajectory`.
Opções do `run_all.py`: `--all`, `--iters`, `--seed`, `--no-build`,
`--results-dir`, `--trajectories-dir`, `--time-limit-ms`,
`--slot-time-limit-ms`, `--trajectory-max-rows`, `--no-trajectory`.
Opções do `output_to_excel.py`: `[results_dir]`, `--output`, `--literature`
(default `../best_ribas.json`).

## Mapa de arquivos

```
include/ src/                 infra copiada de local-searchs-fair/ (Core, Instance, Solution, RNG, Log)
  constructions/
    NEH.*                     reconstrução (NEH::second_step) - dependência do host loop
    PF.*, PF_NEH.*            construção inicial - copiados verbatim de IG_IJ/ (não existiam nas outras pastas)
  local-search/                mesmos 23-entry bodies de local-searchs-fair/, sem nenhuma mudança
  localsearchs/
    IgHost.*                  host loop (destroy/reconstrói/chama a busca local/aceita) + registry das 23
                               entradas como adaptadores "step" + laço de repetição + serialização CSV (summary
                               + trajectory)
src/main.cpp                  driver CLI
run_all.py                    varre instâncias -> results/<entrada>.csv + trajectories/<entrada>__<instância>.csv
output_to_excel.py            results/*.csv -> output.xlsx (RPD + métricas de convergência + Resumo ranqueado)
```

Nenhum arquivo fora de `local-searchs-3/` foi alterado — `IG_IJ/` e as
demais pastas continuam com suas cópias canônicas, e `local-searchs/`/
`local-searchs-fair/` continuam existindo intactas ao lado desta.
