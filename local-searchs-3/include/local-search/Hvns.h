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
// entries (hvns_sa_rls/hvns_sa_best_edge_insertion below).
//
// `time_limit_ms` (--time-limit-ms) is a synthetic safety cap, not part
// of the origin: hvns_best_swap alone is O(n^3*m), and on a large instance
// (e.g. J500) repeated restarts can otherwise take minutes.
void hvns_shaking(Solution &s, Instance &instance, double time_limit_ms);

// Ported verbatim from HVNS::sa_rls (HVNS/src/HVNS.cpp:399-435): RLS with
// Metropolis acceptance and geometric cooling, reshuffling its reference on
// every accepted move. `current`/`best` and `T` are updated in place, mirroring
// the origin's member-state usage (`m_T`, `current`, `best`).
//
// `time_limit_ms` (--time-limit-ms) is a synthetic safety cap, not part
// of the origin: the `cnt < n` loop only terminates once n consecutive trials
// are rejected, and `beta` is calibrated for a run of `--hvns-n-iter` steps
// (default 1.8M) shared across many outer VNS iterations, so a standalone
// call on a large instance can stay in high-acceptance territory (T still
// high, few resets) for a very long time before it naturally converges.
void hvns_sa_rls(Solution &current, Solution &best, Instance &instance, double &T, double beta,
                 double time_limit_ms);

// Ported verbatim from HVNS::sa_best_edge_insertion (HVNS/src/HVNS.cpp:437-482),
// with the same synthetic `time_limit_ms` safety cap as hvns_sa_rls above.
// There is no `sa_best_swap` counterpart in the origin repository.
void hvns_sa_best_edge_insertion(Solution &current, Solution &best, Instance &instance, double &T, double beta,
                                 double time_limit_ms);

#endif
