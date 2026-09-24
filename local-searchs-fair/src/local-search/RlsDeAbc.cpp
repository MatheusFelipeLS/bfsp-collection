#include "local-search/RlsDeAbc.h"

#include "constructions/NEH.h"

// Ported verbatim from DE_ABC/src/local-search/RLS.cpp:185-213 (only renamed
// rls -> rls_de_abc to avoid clashing with the canonical local-search/RLS.h).
bool rls_de_abc(Solution &s, Instance &instance) {

    bool improved = false;
    size_t cnt = 0;
    NEH helper(instance);
    std::vector<size_t> ref = s.sequence;
    while (cnt < instance.num_jobs()) {

        const size_t job = ref[cnt];
        for (size_t i = 0; i < s.sequence.size(); i++) {
            if (s.sequence[i] == job) {
                s.sequence.erase(s.sequence.begin() + (long)i);
                break;
            }
        }

        auto [best_index, makespan] = helper.taillard_best_insertion(s.sequence, job);
        s.sequence.insert(s.sequence.begin() + (long)best_index, job);

        if (makespan < s.cost) {
            s.cost = makespan;
            improved = true;
        }

        cnt++;
    }

    return improved;
}
