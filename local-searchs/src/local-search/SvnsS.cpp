#include "local-search/SvnsS.h"

#include <algorithm>

#include "Core.h"
#include "constructions/NEH.h"

// Ported verbatim from SVNS_S::LS1_S_swap (SVNS_S/src/SVNS_S.cpp:63-92).
bool ls1_s_swap(Solution &solution, std::vector<size_t> &reference, Instance &instance) { // NOLINT
    size_t original_cost = solution.cost;

    for (size_t i = 0; i < solution.sequence.size() - 1; i++) {
        size_t index = reference[i];
        size_t best_j = 0;
        size_t best_cost = solution.cost;

        for (size_t j = index + 1; j < solution.sequence.size(); j++) {
            size_t cost_temp = solution.cost;
            std::swap(solution.sequence[index], solution.sequence[j]);

            core::recalculate_solution(instance, solution);

            if (solution.cost <= best_cost) {
                best_cost = solution.cost;
                best_j = j;
            }
            std::swap(solution.sequence[index], solution.sequence[j]);
            solution.cost = cost_temp;
        }
        if (best_cost < solution.cost) {
            std::swap(solution.sequence[index], solution.sequence[best_j]);
            solution.cost = best_cost;
        }
    }

    return solution.cost < original_cost;
}

// Ported verbatim from SVNS_S::LS2_S_insertion (SVNS_S/src/SVNS_S.cpp:94-117).
bool ls2_s_insertion(Solution &solution, std::vector<size_t> &reference, Instance &instance) { // NOLINT
    size_t original_cost = solution.cost;

    NEH helper(instance);

    for (size_t j = 0; j < reference.size(); j++) {
        const size_t job = reference[j];

        for (size_t i = 0; i < solution.sequence.size(); i++) {
            if (solution.sequence[i] == job) {
                solution.sequence.erase(solution.sequence.begin() + (long)i);
            }
        }

        auto [best_index, makespan] = helper.taillard_best_insertion(solution.sequence, job);
        solution.sequence.insert(solution.sequence.begin() + (long)best_index, job);

        if (makespan < solution.cost) {
            solution.cost = makespan;
        }
    }

    return solution.cost < original_cost;
}
