#include "local-search/Hvns.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <numeric>

#include "RNG.h"
#include "local-search/EdgeInsertion.h"
#include "local-search/Swap.h"

namespace {
bool deadline_hit(const std::chrono::steady_clock::time_point &start, double time_limit_ms) {
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count() >
           time_limit_ms;
}
} // namespace

HvnsSaParams hvns_compute_sa_params(Instance &instance, size_t n_iter) {
    // Ported from HVNS::HVNS() (HVNS/src/HVNS.cpp:25-35).
    HvnsSaParams p;
    p.t_init = static_cast<double>(instance.all_processing_times_sum()) /
               (static_cast<double>(instance.num_jobs()) * static_cast<double>(instance.num_machines()) * 5.0);
    p.t_fin = p.t_init * 0.1;
    p.beta = (p.t_init - p.t_fin) / (static_cast<double>(n_iter) * p.t_init * p.t_fin);
    return p;
}

void hvns_shaking(Solution &s, Instance &instance, double time_limit_ms) {
    const auto start = std::chrono::steady_clock::now();
    EdgeInsertion ei(instance);

    Solution best = s;
    size_t k = 1;
    constexpr size_t KMAX = 3;

    while (k <= KMAX && !deadline_hit(start, time_limit_ms)) {
        Solution copy = best;
        switch (k) {
        case 1:
            ei.single_insertion_local_search(copy);
            break;
        case 2:
            ei.edge_insertion_local_search(copy);
            break;
        case 3:
            swap_local_search_best(copy, instance);
            break;
        default:
            break;
        }

        if (copy.cost < best.cost) {
            best = copy;
            k = 1;
        } else {
            k++;
        }
    }

    s = best;
}

// Ported verbatim from HVNS::sa_rls (HVNS/src/HVNS.cpp:399-435), plus a
// deadline: the origin's `cnt < n` loop only terminates once n consecutive
// trials are rejected, and `beta` is calibrated for a run of `--hvns-n-iter`
// steps (default 1.8M) shared across many outer VNS iterations - called
// standalone on a large instance it can stay in high-acceptance territory for
// a very long time before T cools enough. --time-limit-ms caps that.
void hvns_sa_rls(Solution &current, Solution &best, Instance &instance, double &T, double beta,
                 double time_limit_ms) {
    const auto start = std::chrono::steady_clock::now();
    EdgeInsertion ei(instance);

    std::vector<size_t> ref(instance.num_jobs());
    std::iota(ref.begin(), ref.end(), 0);
    std::shuffle(ref.begin(), ref.end(), RNG::instance().gen());

    size_t cnt = 0;
    while (cnt < instance.num_jobs() && !deadline_hit(start, time_limit_ms)) {
        const size_t job = ref[cnt];
        size_t i = 0;
        for (i = 0; i < current.sequence.size(); i++) {
            if (current.sequence[i] == job) {
                current.sequence.erase(current.sequence.begin() + (long)i);
                break;
            }
        }

        auto [best_index, best_nei_obj] = ei.taillard_best_insertion(current.sequence, job, i);

        const double delta = static_cast<double>(current.cost) - static_cast<double>(best_nei_obj);

        if (delta != 0 &&
            (!(best_nei_obj > current.cost) || RNG::instance().generate_real_number(0, 1) < std::exp(delta / T))) {
            current.sequence.insert(current.sequence.begin() + (long)best_index, job);
            current.cost = best_nei_obj;
            std::shuffle(ref.begin(), ref.end(), RNG::instance().gen());
            cnt = 0;
        } else {
            current.sequence.insert(current.sequence.begin() + (long)i, job);
            cnt++;
        }

        if (best_nei_obj < best.cost) {
            best = current;
        }

        T = T / (1 + (beta * T));
    }
}

// Ported verbatim from HVNS::sa_best_edge_insertion (HVNS/src/HVNS.cpp:437-482),
// plus the same --time-limit-ms deadline as hvns_sa_rls above.
void hvns_sa_best_edge_insertion(Solution &current, Solution &best, Instance &instance, double &T, double beta,
                                 double time_limit_ms) {
    const auto start = std::chrono::steady_clock::now();
    EdgeInsertion ei(instance);

    std::vector<size_t> ref(instance.num_jobs());
    std::iota(ref.begin(), ref.end(), 0);
    std::shuffle(ref.begin(), ref.end(), RNG::instance().gen());

    size_t cnt = 0;
    while (cnt < instance.num_jobs() && !deadline_hit(start, time_limit_ms)) {
        std::pair<size_t, size_t> jobs;
        jobs.first = ref[cnt];

        size_t i = 0;
        for (i = 0; i < current.sequence.size() - 1; i++) {
            if (current.sequence[i] == jobs.first) {
                jobs.second = current.sequence[i + 1];
                current.sequence.erase(current.sequence.begin() + (long)i, current.sequence.begin() + (long)i + 2);
                break;
            }
        }

        if (i == instance.num_jobs() - 1) {
            cnt++;
            continue;
        }

        auto [best_index, best_nei_obj] = ei.taillard_best_edge_insertion(current.sequence, jobs, i);

        const double delta = static_cast<double>(current.cost) - static_cast<double>(best_nei_obj);

        if (delta != 0 &&
            (!(best_nei_obj > current.cost) || RNG::instance().generate_real_number(0, 1) < std::exp(delta / T))) {
            current.sequence.insert(current.sequence.begin() + (long)best_index, jobs.first);
            current.sequence.insert(current.sequence.begin() + (long)best_index + 1, jobs.second);
            current.cost = best_nei_obj;
            std::shuffle(ref.begin(), ref.end(), RNG::instance().gen());
            cnt = 0;
        } else {
            current.sequence.insert(current.sequence.begin() + (long)i, jobs.first);
            current.sequence.insert(current.sequence.begin() + (long)i + 1, jobs.second);
            cnt++;
        }

        if (best_nei_obj < best.cost) {
            best = current;
        }

        T = T / (1 + (beta * T));
    }
}
