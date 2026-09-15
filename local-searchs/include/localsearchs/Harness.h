#ifndef LOCALSEARCHS_HARNESS_H
#define LOCALSEARCHS_HARNESS_H

#include <optional>
#include <string>
#include <vector>

#include "Instance.h"
#include "Parameters.h"
#include "Solution.h"

namespace ls {

struct Result {
    std::string name;
    size_t n = 0;
    size_t m = 0;
    size_t iters = 0;
    size_t initial_cost = 0;
    size_t final_cost = 0;
    double improvement_abs = 0.0; // initial_cost - final_cost
    double improvement_pct = 0.0; // improvement_abs / initial_cost * 100
    double time_ms = 0.0;         // averaged over `iters` repetitions
    std::string note;             // free text: iterations, final T, branch taken, ...
};

// Runs one local search (by name) or all of them when `which` is empty, all
// starting from the same `initial` solution (a random permutation with a
// fixed seed, generated once in main.cpp - see README.md "Solução inicial").
std::vector<Result> run(const std::optional<std::string> &which, Instance &instance, const Solution &initial,
                        const Parameters &params, size_t seed);

// CSV: localsearch,n,m,iters,initial_cost,final_cost,improvement_abs,improvement_pct,time_ms,note
std::string header();
std::string format(const Result &r);

} // namespace ls

#endif
