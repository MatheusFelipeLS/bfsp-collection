#include "local-search/MrlsPEda.h"

#include "RNG.h"
#include "constructions/NEH.h"

// Ported verbatim from P_EDA/src/P_EDA.cpp:37-83 (P_EDA::mrls -> mrls_p_eda).
bool mrls_p_eda(Solution &s, std::vector<size_t> &ref, Instance &instance) {
    bool improved = false;
    size_t n = instance.num_jobs();
    size_t j = 0;
    size_t cnt = 0;
    NEH helper(instance);
    while (cnt <= n) {
        j++;
        if (j >= n) {
            j = j % n;
            std::vector<size_t> shuffled;
            shuffled.reserve(n);

            while (!ref.empty()) {
                const size_t k = RNG::instance().generate(size_t{0}, ref.size() - 1);

                shuffled.push_back(ref[k]);
                ref.erase(ref.begin() + (long)k);
            }

            ref = shuffled;
        }

        const size_t job = ref[j];
        for (size_t i = 0; i < s.sequence.size(); i++) {
            if (s.sequence[i] == job) {
                s.sequence.erase(s.sequence.begin() + (long)i);
                break;
            }
        }

        auto [best_index, makespan] = helper.taillard_best_insertion(s.sequence, job);
        s.sequence.insert(s.sequence.begin() + (long)best_index, job);

        if (makespan < s.cost) {
            cnt = 0;
            s.cost = makespan;
            improved = true;
            continue;
        } else {
            cnt++;
        }
    }

    return improved;
}
