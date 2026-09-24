#include "local-search/Swap.h"

#include <algorithm>
#include <limits>

#include "Core.h"
#include "Instance.h"
#include "Solution.h"

// first-improvement swap - extracted from IG::local_search (IG/src/IG.cpp).
bool swap_local_search_first(Solution &s, Instance &instance) {
    core::recalculate_solution(instance, s); // set departure-times matrix

    Solution copy = s;
    const size_t start_cost = s.cost;
    bool improved = true;

    while (improved) {
        improved = false;
        for (size_t i = 0; i < s.sequence.size() - 1; i++) {
            for (size_t j = i + 1; j < s.sequence.size(); j++) {
                std::swap(copy.sequence[i], copy.sequence[j]);
                if (i == 0) {
                    // swapping the first job invalidates the whole matrix
                    core::recalculate_solution(instance, copy);
                } else {
                    core::partial_recalculate_solution(instance, copy, i);
                }

                if (copy.cost < s.cost) {
                    s = copy;
                    improved = true;
                } else {
                    std::swap(copy.sequence[i], copy.sequence[j]);
                }
            }
            // Restore only the one departure-times row dirtied above that the next
            // iteration will read - cheaper than copying the whole Solution.
            copy.departure_times[i] = s.departure_times[i];
        }
    }

    return s.cost < start_cost;
}

// best-improvement swap, single pass - extracted from HVNS::best_swap (HVNS/src/HVNS.cpp).
bool swap_local_search_best(Solution &s, Instance &instance) {
    core::recalculate_solution(instance, s);

    Solution copy = s;
    size_t best_i = 0;
    size_t best_j = 0;
    size_t best_obj = s.cost;

    for (size_t i = 0; i < s.sequence.size(); i++) {
        for (size_t j = i + 1; j < s.sequence.size(); j++) {
            std::swap(copy.sequence[i], copy.sequence[j]);

            if (i == 0) {
                core::recalculate_solution(instance, copy);
            } else {
                core::partial_recalculate_solution(instance, copy, i);
            }

            std::swap(copy.sequence[i], copy.sequence[j]);

            if (copy.cost < best_obj) {
                best_obj = copy.cost;
                best_i = i;
                best_j = j;
            }
        }

        copy.departure_times[i] = s.departure_times[i];
    }

    if (best_obj == s.cost) {
        return false;
    }

    std::swap(s.sequence[best_i], s.sequence[best_j]);
    if (best_i == 0) {
        core::recalculate_solution(instance, s);
    } else {
        core::partial_recalculate_solution(instance, s, best_i);
    }

    return true;
}
