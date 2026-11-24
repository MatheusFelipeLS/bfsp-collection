#ifndef HVNS_LS_H
#define HVNS_LS_H

#include "Instance.h"
#include "Solution.h"

// Temperature/cooling-rate triple used by HVNS's SA-flavoured local searches,
// computed with the exact formula from HVNS::HVNS() (HVNS/src/HVNS.cpp:32-35).
struct HvnsSaParams {
    double t_init = 0.0;
    double t_fin = 0.0;
    double beta = 0.0;
};

HvnsSaParams hvns_compute_sa_params(Instance &instance, size_t n_iter);

// The HVNS VNS "shaking" restart cycle: k=1..3 over
// {hvns_best_insertion, hvns_best_edge_insertion, hvns_best_swap} (reusing
// EdgeInsertion/Swap.h), restarting at k=1 on any improvement, ported from the
// outer k-loop of HVNS::solve() (HVNS/src/HVNS.cpp:~500-555) WITHOUT the
// interleaved SA phase (sa_rls/sa_best_edge_insertion), which are their own
// entries (hvns_sa_rls/hvns_sa_best_edge_insertion below). Terminates on its
// own: every restart strictly lowers the cost, and it stops once k=3 fails.
void hvns_shaking(Solution &s, Instance &instance);

// Ported verbatim from HVNS::sa_rls (HVNS/src/HVNS.cpp:399-435): RLS with
// Metropolis acceptance and geometric cooling, reshuffling its reference on
// every accepted move. `current`/`best` and `T` are updated in place, mirroring
// the origin's member-state usage (`m_T`, `current`, `best`).
//
// No time budget: like the origin, it stops once n consecutive trials are
// rejected. `T` is meant to persist across calls (as `m_T` does in HVNS), so
// it keeps cooling over the host metaheuristic's run.
void hvns_sa_rls(Solution &current, Solution &best, Instance &instance, double &T, double beta);

// Ported verbatim from HVNS::sa_best_edge_insertion (HVNS/src/HVNS.cpp:437-482),
// with the same natural stopping rule as hvns_sa_rls above.
// There is no `sa_best_swap` counterpart in the origin repository.
void hvns_sa_best_edge_insertion(Solution &current, Solution &best, Instance &instance, double &T, double beta);

#endif
