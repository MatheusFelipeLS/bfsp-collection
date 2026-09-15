#include "local-search/BestSwapIg.h"

#include <algorithm>

#include "Core.h"

// Ported verbatim (bug included) from IG_IJ::BestSwap (IG_IJ/src/IG_IJ.cpp:40-82).
void best_swap_ig(Solution &solution, Instance &instance) { // NOLINT
    Solution copy = solution;
    size_t original_cost = solution.cost;
    size_t best_j = 0;
    size_t best_i = 0;
    size_t best_cost = original_cost;

    for (size_t i = 0; i < solution.sequence.size() - 1; i++) {
        for (size_t j = i + 1; j < solution.sequence.size(); j++) {
            std::swap(copy.sequence[i], copy.sequence[j]);

            if (i == 0) {
                core::recalculate_solution(instance, copy);
            } else {
                core::partial_recalculate_solution(instance, copy, i);
            }

            // BUG (reproduced faithfully): should compare copy.cost, not solution.cost.
            if (solution.cost <= best_cost) {
                best_cost = solution.cost;
                best_j = j;
                best_i = i;
            }
            std::swap(copy.sequence[i], copy.sequence[j]);
        }
        if (i == 0) {
            core::recalculate_solution(instance, copy);
        } else {
            core::partial_recalculate_solution(instance, copy, i);
        }
    }

    if (best_cost < original_cost) {
        solution.cost = best_cost;
        std::swap(solution.sequence[best_i], solution.sequence[best_j]);
    }
}

// Same neighbourhood with the comparison bug fixed.
void best_swap_ig_fixed(Solution &solution, Instance &instance) { // NOLINT
    Solution copy = solution;
    size_t original_cost = solution.cost;
    size_t best_j = 0;
    size_t best_i = 0;
    size_t best_cost = original_cost;

    for (size_t i = 0; i < solution.sequence.size() - 1; i++) {
        for (size_t j = i + 1; j < solution.sequence.size(); j++) {
            std::swap(copy.sequence[i], copy.sequence[j]);

            if (i == 0) {
                core::recalculate_solution(instance, copy);
            } else {
                core::partial_recalculate_solution(instance, copy, i);
            }

            if (copy.cost <= best_cost) {
                best_cost = copy.cost;
                best_j = j;
                best_i = i;
            }
            std::swap(copy.sequence[i], copy.sequence[j]);
        }
        if (i == 0) {
            core::recalculate_solution(instance, copy);
        } else {
            core::partial_recalculate_solution(instance, copy, i);
        }
    }

    if (best_cost < original_cost) {
        solution.cost = best_cost;
        std::swap(solution.sequence[best_i], solution.sequence[best_j]);
    }
}
