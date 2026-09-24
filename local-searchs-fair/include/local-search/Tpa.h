#ifndef TPA_SA_H
#define TPA_SA_H

#include "Instance.h"
#include "Solution.h"

// TPA::SimulatedAnnealing, ported from TPA/src/SimulatedAnnealing.cpp: single
// job removal + reinsertion via NEH::mtaillard_best_insertion (already
// vendored, constructions/NEH.h), with Metropolis acceptance and geometric
// cooling (solve(): lines 39-104; anneal(): lines 123-136).
//
// The origin terminates on a real-world `time_limit` (uptime() in SECONDS,
// derived from the outer TPA metaheuristic's own parameters, unavailable
// standalone); here it is replaced by a synthetic `time_limit_ms` budget
// (--time-limit-ms). Everything else (temperature schedule, acceptance
// rule, best/current bookkeeping) is unchanged.
struct TpaSaResult {
    Solution best;          // solve()'s `best_solution`: may differ from `final_current`, SA accepts worsening moves
    Solution final_current; // solve()'s `current_solution` at termination
    size_t iterations = 0;
    double final_temp = 0.0;
};

TpaSaResult tpa_anneal_sa(const Solution &initial, Instance &instance, size_t n_iter, double final_temp,
                          double time_limit_ms);

#endif
