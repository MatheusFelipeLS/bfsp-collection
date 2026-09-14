#include "speedups/Bench.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <limits>
#include <numeric>
#include <sstream>
#include <utility>

#include "Core.h"
#include "Instance.h"
#include "RNG.h"
#include "Solution.h"

#include "constructions/NEH.h"
#include "local-search/EdgeInsertion.h"
#include "local-search/RLS.h"
#include "local-search/Swap.h"
#include "speedups/Naive.h"

namespace {

double time_ms(const std::function<void()> &fn, size_t iters) {
    const auto start = std::chrono::steady_clock::now();
    for (size_t i = 0; i < iters; i++) {
        fn();
    }
    const auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count() / (double)iters;
}

std::vector<size_t> random_perm(size_t n) {
    std::vector<size_t> v(n);
    std::iota(v.begin(), v.end(), 0);
    std::shuffle(v.begin(), v.end(), RNG::instance().gen());
    return v;
}

size_t recompute_cost(const std::vector<size_t> &seq, Instance &instance) {
    Solution tmp;
    tmp.sequence = seq;
    core::recalculate_solution(instance, tmp);
    return tmp.cost;
}

bench::Result blank(const char *name, Instance &instance, const Parameters &params) {
    bench::Result r;
    r.speedup = name;
    r.n = instance.num_jobs();
    r.m = instance.num_machines();
    r.iters = params.iters();
    return r;
}

// ---------------------------------------------------------------------------
// #1 - Taillard's acceleration for the insertion neighbourhood (NEH)
// ---------------------------------------------------------------------------
bench::Result bench_taillard_insertion(Instance &instance, const Parameters &params) {
    bench::Result r = blank("taillard-insertion", instance, params);
    const std::vector<size_t> phi = core::stpt_sort(instance);

    Solution accel;
    Solution naive_sol;

    r.accel_ms = time_ms(
        [&] {
            NEH neh(instance);
            accel = neh.solve(phi);
        },
        params.iters());
    r.naive_ms = time_ms([&] { naive_sol = naive::neh_construct(phi, instance); }, params.iters());

    r.correct = accel.sequence == naive_sol.sequence && accel.cost == naive_sol.cost &&
                accel.cost == recompute_cost(accel.sequence, instance);
    r.factor = r.accel_ms > 0 ? r.naive_ms / r.accel_ms : 0.0;
    r.note = "makespan=" + std::to_string(accel.cost);
    return r;
}

// ---------------------------------------------------------------------------
// #2 - Grabowski-Wodecki critical-block restriction (rls_grabowski vs rls)
// ---------------------------------------------------------------------------
bench::Result bench_grabowski_block(Instance &instance, const Parameters &params) {
    bench::Result r = blank("grabowski-block", instance, params);

    NEH neh(instance);
    Solution start = neh.solve(core::stpt_sort(instance));
    core::recalculate_solution(instance, start);
    const std::vector<size_t> ref = random_perm(instance.num_jobs());

    Solution restricted;
    Solution full;

    r.accel_ms = time_ms(
        [&] {
            restricted = start;
            rls_grabowski(restricted, ref, instance);
        },
        params.iters());
    r.naive_ms = time_ms(
        [&] {
            full = start;
            rls(full, ref, instance);
        },
        params.iters());

    const bool restricted_ok =
        restricted.cost == recompute_cost(restricted.sequence, instance) && restricted.cost <= start.cost;
    const bool full_ok = full.cost == recompute_cost(full.sequence, instance) && full.cost <= start.cost;
    r.correct = restricted_ok && full_ok;
    r.factor = r.accel_ms > 0 ? r.naive_ms / r.accel_ms : 0.0;
    r.note = "start=" + std::to_string(start.cost) + " restricted=" + std::to_string(restricted.cost) +
             " full=" + std::to_string(full.cost);
    return r;
}

// ---------------------------------------------------------------------------
// #3 - Incremental makespan recomputation (swap neighbourhood, one best-move scan)
// ---------------------------------------------------------------------------
bench::Result bench_partial_recalc(Instance &instance, const Parameters &params) {
    bench::Result r = blank("partial-recalc", instance, params);
    const std::vector<size_t> raw = core::stpt_sort(instance); // deliberately mediocre -> the scan finds a move

    Solution accel;
    Solution naive_sol;

    r.accel_ms = time_ms(
        [&] {
            accel.sequence = raw;
            accel.cost = std::numeric_limits<size_t>::max();
            swap_local_search_best(accel, instance);
        },
        params.iters());
    r.naive_ms = time_ms(
        [&] {
            naive_sol.sequence = raw;
            naive_sol.cost = std::numeric_limits<size_t>::max();
            naive::swap_neighborhood_best(naive_sol, instance);
        },
        params.iters());

    r.correct = accel.sequence == naive_sol.sequence && accel.cost == naive_sol.cost &&
                accel.cost == recompute_cost(accel.sequence, instance);
    r.factor = r.accel_ms > 0 ? r.naive_ms / r.accel_ms : 0.0;
    r.note = "makespan=" + std::to_string(accel.cost);
    return r;
}

// ---------------------------------------------------------------------------
// #4 - Taillard's acceleration extended to the adjacent-pair (edge) neighbourhood
// ---------------------------------------------------------------------------
bench::Result bench_edge_insertion(Instance &instance, const Parameters &params) {
    bench::Result r = blank("edge-insertion", instance, params);
    const std::vector<size_t> raw = core::stpt_sort(instance);

    Solution accel;
    Solution naive_sol;

    r.accel_ms = time_ms(
        [&] {
            accel.sequence = raw;
            accel.cost = std::numeric_limits<size_t>::max();
            EdgeInsertion ei(instance);
            ei.edge_insertion_local_search(accel);
        },
        params.iters());
    r.naive_ms = time_ms(
        [&] {
            naive_sol.sequence = raw;
            naive_sol.cost = std::numeric_limits<size_t>::max();
            naive::edge_insertion_local_search(naive_sol, instance);
        },
        params.iters());

    r.correct = accel.sequence == naive_sol.sequence && accel.cost == naive_sol.cost &&
                accel.cost == recompute_cost(accel.sequence, instance);
    r.factor = r.accel_ms > 0 ? r.naive_ms / r.accel_ms : 0.0;
    r.note = "makespan=" + std::to_string(accel.cost);
    return r;
}

// ---------------------------------------------------------------------------
// #5 - Incremental idle/blocking delta (alpha/sigma) for greedy construction
// ---------------------------------------------------------------------------
bench::Result bench_alpha_sigma(Instance &instance, const Parameters &params) {
    bench::Result r = blank("alpha-sigma", instance, params);
    const size_t n = instance.num_jobs();
    const std::vector<size_t> perm = random_perm(n);

    // Per construction step k, the partial schedule's departure-times matrix `d`
    // is state the greedy heuristic already maintains; only the candidate probe
    // differs (O(m) incremental vs O(k*m) full recompute).
    std::vector<std::vector<std::vector<size_t>>> d_per_step;
    std::vector<size_t> node_per_step;
    d_per_step.reserve(n);
    for (size_t k = 1; k + 1 < n; k++) {
        Solution partial;
        partial.sequence.assign(perm.begin(), perm.begin() + (long)k);
        core::recalculate_solution(instance, partial);
        d_per_step.push_back(partial.departure_times);
        node_per_step.push_back(perm[k]);
    }

    bool correct = true;
    r.accel_ms = time_ms(
        [&] {
            for (size_t s = 0; s < node_per_step.size(); s++) {
                auto nd = core::calculate_new_departure_time(instance, d_per_step[s], node_per_step[s]);
                (void)core::calculate_alpha(instance, d_per_step[s], nd, node_per_step[s], s + 1);
            }
        },
        params.iters());
    r.naive_ms = time_ms(
        [&] {
            for (size_t s = 0; s < node_per_step.size(); s++) {
                std::vector<size_t> partial(perm.begin(), perm.begin() + (long)(s + 1));
                (void)naive::new_departure_time_full(partial, node_per_step[s], instance);
            }
        },
        params.iters());

    for (size_t s = 0; s < node_per_step.size() && correct; s++) {
        std::vector<size_t> partial(perm.begin(), perm.begin() + (long)(s + 1));
        auto fast = core::calculate_new_departure_time(instance, d_per_step[s], node_per_step[s]);
        auto slow = naive::new_departure_time_full(partial, node_per_step[s], instance);
        correct = fast == slow;
    }

    r.correct = correct;
    r.factor = r.accel_ms > 0 ? r.naive_ms / r.accel_ms : 0.0;
    r.note = "steps=" + std::to_string(node_per_step.size());
    return r;
}

// ---------------------------------------------------------------------------
// #6 - Reused preallocated buffers (one NEH instance vs a fresh one per call)
// ---------------------------------------------------------------------------
bench::Result bench_preallocated_buffers(Instance &instance, const Parameters &params) {
    bench::Result r = blank("preallocated-buffers", instance, params);
    constexpr size_t CALLS = 2000;

    std::vector<size_t> seq = random_perm(instance.num_jobs());
    const size_t job = seq.back();
    seq.pop_back();

    std::pair<size_t, size_t> reused_res;
    std::pair<size_t, size_t> fresh_res;

    r.accel_ms = time_ms(
        [&] {
            NEH neh(instance);
            for (size_t c = 0; c < CALLS; c++) {
                reused_res = neh.taillard_best_insertion(seq, job);
            }
        },
        params.iters());
    r.naive_ms = time_ms(
        [&] {
            for (size_t c = 0; c < CALLS; c++) {
                NEH neh(instance);
                fresh_res = neh.taillard_best_insertion(seq, job);
            }
        },
        params.iters());

    r.correct = reused_res == fresh_res;
    r.factor = r.accel_ms > 0 ? r.naive_ms / r.accel_ms : 0.0;
    r.note = "calls=" + std::to_string(CALLS);
    return r;
}

using BenchFn = std::function<bench::Result(Instance &, const Parameters &)>;

const std::vector<std::pair<std::string, BenchFn>> &registry() {
    static const std::vector<std::pair<std::string, BenchFn>> reg = {
        {"taillard-insertion", bench_taillard_insertion},
        {"grabowski-block", bench_grabowski_block},
        {"partial-recalc", bench_partial_recalc},
        {"edge-insertion", bench_edge_insertion},
        {"alpha-sigma", bench_alpha_sigma},
        {"preallocated-buffers", bench_preallocated_buffers},
    };
    return reg;
}

} // namespace

std::vector<bench::Result> bench::run(const std::optional<std::string> &which, Instance &instance,
                                      const Parameters &params) {
    std::vector<bench::Result> out;
    for (const auto &[name, fn] : registry()) {
        if (which && *which != name) {
            continue;
        }
        out.push_back(fn(instance, params));
    }
    return out;
}

std::string bench::header() { return "speedup,n,m,iters,naive_ms,accel_ms,factor,correct,note"; }

std::string bench::format(const bench::Result &r) {
    std::ostringstream os;
    os << r.speedup << ',' << r.n << ',' << r.m << ',' << r.iters << ',' << r.naive_ms << ',' << r.accel_ms << ','
       << r.factor << ',' << (r.correct ? 1 : 0) << ',' << r.note;
    return os.str();
}
