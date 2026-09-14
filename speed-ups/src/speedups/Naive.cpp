#include "speedups/Naive.h"

#include <algorithm>
#include <limits>

#include "Core.h"
#include "Instance.h"
#include "Solution.h"

// Every candidate is scored with a fresh Solution + core::recalculate_solution,
// i.e. the full O(n*m) schedule recompute (and a fresh departure-times buffer) on
// every move - that is exactly the un-accelerated cost each speed up removes.

std::pair<size_t, size_t> naive::best_insertion(const std::vector<size_t> &seq, size_t job, Instance &instance) {
    std::vector<size_t> candidate = seq;
    candidate.insert(candidate.begin(), job);

    size_t best_index = 0;
    size_t best_value = std::numeric_limits<size_t>::max();

    for (size_t i = 0; i <= seq.size(); i++) {
        if (i > 0) {
            std::rotate(candidate.begin() + (long)(i - 1), candidate.begin() + (long)i, candidate.begin() + (long)i + 1);
        }
        Solution probe;
        probe.sequence = candidate;
        core::recalculate_solution(instance, probe);
        if (probe.cost < best_value) {
            best_value = probe.cost;
            best_index = i;
        }
    }

    return {best_index, best_value};
}

Solution naive::neh_construct(std::vector<size_t> phi, Instance &instance) {
    Solution s;
    s.sequence.reserve(instance.num_jobs());
    s.sequence = {phi.front()};
    phi.erase(phi.begin());

    while (!phi.empty()) {
        const size_t job = phi.front();
        auto [best_index, makespan] = naive::best_insertion(s.sequence, job, instance);
        s.sequence.insert(s.sequence.begin() + (long)best_index, job);
        s.cost = makespan;
        phi.erase(phi.begin());
    }

    return s;
}

bool naive::swap_neighborhood_best(Solution &s, Instance &instance) {
    core::recalculate_solution(instance, s);

    std::vector<size_t> seq = s.sequence;
    size_t best_i = 0;
    size_t best_j = 0;
    size_t best_obj = s.cost;

    for (size_t i = 0; i < seq.size(); i++) {
        for (size_t j = i + 1; j < seq.size(); j++) {
            std::swap(seq[i], seq[j]);
            Solution probe;
            probe.sequence = seq;
            core::recalculate_solution(instance, probe);
            std::swap(seq[i], seq[j]);
            if (probe.cost < best_obj) {
                best_obj = probe.cost;
                best_i = i;
                best_j = j;
            }
        }
    }

    if (best_obj == s.cost) {
        return false;
    }

    std::swap(s.sequence[best_i], s.sequence[best_j]);
    core::recalculate_solution(instance, s);
    return true;
}

std::pair<size_t, size_t> naive::best_edge_insertion(const std::vector<size_t> &seq, std::pair<size_t, size_t> jobs,
                                                     Instance &instance) {
    std::vector<size_t> candidate = seq;
    candidate.insert(candidate.begin(), jobs.second);
    candidate.insert(candidate.begin(), jobs.first);

    size_t best_index = 0;
    size_t best_value = std::numeric_limits<size_t>::max();

    // NOTE: upper bound is `< seq.size()`, not `<= seq.size()`, to match the
    // position range actually scanned by EdgeInsertion::taillard_best_edge_insertion
    // (ported from HVNS, which never tries appending the pair at the very end).
    for (size_t i = 0; i < seq.size(); i++) {
        if (i > 0) {
            // move the 2-job block one step to the right
            std::rotate(candidate.begin() + (long)(i - 1), candidate.begin() + (long)(i + 1),
                        candidate.begin() + (long)(i + 2));
        }
        Solution probe;
        probe.sequence = candidate;
        core::recalculate_solution(instance, probe);
        if (probe.cost < best_value) {
            best_value = probe.cost;
            best_index = i;
        }
    }

    return {best_index, best_value};
}

bool naive::edge_insertion_local_search(Solution &s, Instance &instance) {
    core::recalculate_solution(instance, s);

    size_t best_job = 0;
    size_t best_index = std::numeric_limits<size_t>::max();
    size_t best_obj = s.cost;

    for (size_t i = 0; i < instance.num_jobs() - 1; i++) {
        std::pair<size_t, size_t> jobs = {s.sequence[i], s.sequence[i + 1]};
        s.sequence.erase(s.sequence.begin() + (long)i, s.sequence.begin() + (long)i + 2);

        auto [index, obj] = naive::best_edge_insertion(s.sequence, jobs, instance);

        s.sequence.insert(s.sequence.begin() + (long)i, jobs.first);
        s.sequence.insert(s.sequence.begin() + (long)i + 1, jobs.second);

        if (obj < best_obj) {
            best_obj = obj;
            best_index = index;
            best_job = i;
        }
    }

    if (s.cost == best_obj) {
        return false;
    }

    if (best_job < best_index) {
        std::rotate(s.sequence.begin() + (long)best_job, s.sequence.begin() + (long)best_job + 2,
                    s.sequence.begin() + (long)best_index + 2);
    } else {
        std::rotate(s.sequence.begin() + (long)best_index, s.sequence.begin() + (long)best_job,
                    s.sequence.begin() + (long)best_job + 2);
    }

    core::recalculate_solution(instance, s);
    s.cost = best_obj;
    return true;
}

std::vector<size_t> naive::new_departure_time_full(const std::vector<size_t> &partial_seq, size_t node,
                                                   Instance &instance) {
    Solution tmp;
    tmp.sequence = partial_seq;
    tmp.sequence.push_back(node);
    core::recalculate_solution(instance, tmp);
    return tmp.departure_times.back();
}
