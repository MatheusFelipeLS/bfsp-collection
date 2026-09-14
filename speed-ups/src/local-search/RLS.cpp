#include "local-search/RLS.h"

#include "Core.h"
#include "Instance.h"
#include "Solution.h"
#include "constructions/NEH.h"
#include "local-search/Grabowski.h"

#include <vector>

// Referenced Local Search (RIS).
//
// Copied from MA/src/local-search/RLS.cpp - the "full" RLS flavour shared, byte
// for byte, by MA, IG_IJ, IG_VND1, IG_VND2, DIWO, SaDIWO, MFFO, HDDE and DE_ABC.
// The only change here is that the Grabowski critical-block machinery was moved
// to local-search/Grabowski.{h,cpp} so the benchmark can reuse it.
//
// Speed ups exercised:
//   - rls           : #1 (Taillard insertion, NEH::taillard_best_insertion)
//   - rls_grabowski : #1 + #2 (block restriction) + #3 (partial recompute of the
//                     accepted move via core::partial_recalculate_solution)

bool rls_grabowski(Solution &s, const std::vector<size_t> &ref, Instance &instance) {
    bool improved = false;
    size_t j = 0;
    size_t cnt = 0;
    NEH helper(instance);
    core::recalculate_solution(instance, s); // Set departure times matrix
    while (cnt < instance.num_jobs()) {
        j = (j + 1) % instance.num_jobs();

        const size_t job = ref[j];

        const auto ranges = grabowski_reinsertion(instance, s, job);

        size_t og_index = 0;
        for (size_t i = 0; i < s.sequence.size(); i++) {
            if (s.sequence[i] == job) {
                s.sequence.erase(s.sequence.begin() + (long)i);
                og_index = i;
                break;
            }
        }

        const auto [best_index, makespan] = helper.taillard_grabowski_best_ins(s, job, ranges);

        if (makespan < s.cost) {
            s.sequence.insert(s.sequence.begin() + (long)best_index, job);
            cnt = 0;
            s.cost = makespan;
            improved = true;
            core::partial_recalculate_solution(instance, s, std::min(og_index, best_index));
            continue;
        }

        s.sequence.insert(s.sequence.begin() + (long)og_index, job);

        cnt++;
    }

    return improved;
}

bool rls(Solution &s, const std::vector<size_t> &ref, Instance &instance) {

    bool improved = false;
    size_t j = 0;
    size_t cnt = 0;
    NEH helper(instance);
    while (cnt < instance.num_jobs()) {
        j = (j + 1) % instance.num_jobs();

        const size_t job = ref[j];
        for (size_t i = 0; i < s.sequence.size(); i++) {
            if (s.sequence[i] == job) {
                s.sequence.erase(s.sequence.begin() + (long)i);
            }
        }

        auto [best_index, makespan] = helper.taillard_best_insertion(s.sequence, job);
        s.sequence.insert(s.sequence.begin() + (long)best_index, job);

        if (makespan < s.cost) {
            cnt = 0;
            s.cost = makespan;
            improved = true;
            continue;
        }

        cnt++;
    }

    return improved;
}
