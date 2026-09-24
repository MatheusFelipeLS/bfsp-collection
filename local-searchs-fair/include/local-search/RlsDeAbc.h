#ifndef RLS_DE_ABC_H
#define RLS_DE_ABC_H

#include "Instance.h"
#include "Solution.h"

// DE_ABC's own `rls`, ported verbatim from DE_ABC/src/local-search/RLS.cpp:185-213.
//
// Unlike the canonical `rls` (local-search/RLS.h), this variant has its own
// signature (no `ref` parameter - it builds the reference internally from
// `s.sequence`) and does a SINGLE linear pass over the n jobs: `cnt` is never
// reset to 0 on an improving move and there is no `continue`, so it stops
// after one sweep instead of iterating to convergence.
bool rls_de_abc(Solution &s, Instance &instance);

#endif
