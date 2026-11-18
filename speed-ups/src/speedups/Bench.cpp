#include "speedups/Bench.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <tuple>
#include <utility>

#include "Core.h"
#include "Instance.h"
#include "RNG.h"
#include "Solution.h"
#include "Stats.h"

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

// Starting point shared by every benchmark of one instance, drawn ONCE in
// bench::run: every component (and its naive baseline) starts from exactly the
// same solution, whatever the order of the benchmarks or of their own RNG draws.
struct Input {
    std::vector<size_t> start; // completely random starting sequence
    std::vector<size_t> ref;   // reference order for the RLS family
};

size_t recompute_cost(const std::vector<size_t> &seq, Instance &instance) {
    Solution tmp;
    tmp.sequence = seq;
    core::recalculate_solution(instance, tmp);
    return tmp.cost;
}

double pct(size_t part, size_t whole) { return whole > 0 ? 100.0 * (double)part / (double)whole : 0.0; }

// Runs `fn` once more, outside any timed region, with fresh counters, and
// returns what it did. Used to fill the % columns.
stats::Counters count_work(const std::function<void()> &fn) {
    stats::reset();
    fn();
    return stats::get();
}

double pruned_machines_pct(const stats::Counters &c) { return pct(c.machines_total - c.machines_scanned, c.machines_total); }
double discarded_positions_pct(const stats::Counters &c) {
    return pct(c.positions_total - c.positions_evaluated, c.positions_total);
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
// NEH builds a solution from scratch, so it ignores input.start and keeps
// its own insertion order (stpt_sort).
bench::Result bench_taillard_insertion(Instance &instance, const Parameters &params, const Input & /*input*/) {
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
    r.pruned_machines_pct = pruned_machines_pct(count_work([&] {
        NEH neh(instance);
        (void)neh.solve(phi);
    }));
    r.note = "makespan=" + std::to_string(accel.cost);
    return r;
}

// ---------------------------------------------------------------------------
// #2 - Grabowski-Wodecki critical-block restriction (rls_grabowski vs rls)
// ---------------------------------------------------------------------------
[[maybe_unused]] bench::Result bench_grabowski_block(Instance &instance, const Parameters &params) {
    bench::Result r = blank("grabowski-block", instance, params);

    // Completely random starting solution: leaves the local search real work to do.
    Solution start;
    start.sequence = random_perm(instance.num_jobs());
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
// RLS family - #1 Taillard (with and without the early cut-off) and #2 Grabowski,
// each measured inside the referenced local search against naive::rls.
//
// All rows share ONE random start, ONE reference order and ONE naive::rls
// measurement (the baseline is identical for the three, and it is by far the
// most expensive part of the benchmark).
//   rls-taillard        rls_taillard_no_cutoff  same trajectory as naive -> identical result
//   rls-taillard-cutoff rls_taillard            same trajectory as naive -> identical result
//   rls-grabowski       rls_grabowski           different acceptance rule -> may end in a
//                                               different local optimum; checked for validity
// ---------------------------------------------------------------------------
std::vector<bench::Result> bench_rls(Instance &instance, const Parameters &params, const Input &input) {
    Solution start;
    start.sequence = input.start;
    core::recalculate_solution(instance, start);
    const std::vector<size_t> &ref = input.ref;

    Solution naive_sol;
    const double naive_ms = time_ms(
        [&] {
            naive_sol = start;
            naive::rls(naive_sol, ref, instance);
        },
        params.iters());
    const bool naive_valid = naive_sol.cost == recompute_cost(naive_sol.sequence, instance);

    auto measure = [&](const char *name, auto &&fn) {
        bench::Result r = blank(name, instance, params);
        Solution out;
        const auto run_once = [&] {
            out = start;
            fn(out, ref, instance);
        };
        r.accel_ms = time_ms(run_once, params.iters());
        r.naive_ms = naive_ms;
        r.factor = r.accel_ms > 0 ? r.naive_ms / r.accel_ms : 0.0;
        const stats::Counters work = count_work(run_once);
        return std::make_tuple(r, out, work);
    };

    std::vector<bench::Result> out;

    for (const auto &[name, fn] : {std::make_pair("rls-taillard", &rls_taillard_no_cutoff),
                                   std::make_pair("rls-taillard-cutoff", &rls_taillard)}) {
        auto [r, sol, work] = measure(name, fn);
        r.correct = naive_valid && sol.sequence == naive_sol.sequence && sol.cost == naive_sol.cost;
        if (fn == &rls_taillard) { // the no-cut-off variant has nothing to report
            r.pruned_machines_pct = pruned_machines_pct(work);
        }
        r.note = "start=" + std::to_string(start.cost) + " makespan=" + std::to_string(sol.cost);
        out.push_back(r);
    }

    {
        auto [r, sol, work] = measure("rls-grabowski", &rls_grabowski);
        r.pruned_machines_pct = pruned_machines_pct(work);
        r.discarded_positions_pct = discarded_positions_pct(work);
        const bool valid = sol.cost == recompute_cost(sol.sequence, instance) && sol.cost <= start.cost;
        r.correct = naive_valid && valid;
        r.note = "start=" + std::to_string(start.cost) + " naive=" + std::to_string(naive_sol.cost) +
                 " grabowski=" + std::to_string(sol.cost) +
                 " same_trajectory=" + (sol.sequence == naive_sol.sequence ? "1" : "0");
        out.push_back(r);
    }

    return out;
}

// ---------------------------------------------------------------------------
// #3 - Incremental makespan recomputation (swap neighbourhood, one best-move scan)
// ---------------------------------------------------------------------------
bench::Result bench_partial_recalc(Instance &instance, const Parameters &params, const Input &input) {
    bench::Result r = blank("partial-recalc", instance, params);
    const std::vector<size_t> &raw = input.start;

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
    r.note = "start=" + std::to_string(recompute_cost(raw, instance)) + " makespan=" + std::to_string(accel.cost);
    return r;
}

// ---------------------------------------------------------------------------
// #4 - Taillard's acceleration extended to the adjacent-pair (edge) neighbourhood
// ---------------------------------------------------------------------------
[[maybe_unused]] bench::Result bench_edge_insertion(Instance &instance, const Parameters &params) {
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
[[maybe_unused]] bench::Result bench_alpha_sigma(Instance &instance, const Parameters &params) {
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
[[maybe_unused]] bench::Result bench_preallocated_buffers(Instance &instance, const Parameters &params) {
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

using BenchFn = std::function<std::vector<bench::Result>(Instance &, const Parameters &, const Input &)>;

struct Entry {
    std::string group;             // name accepted by -p for the whole group
    std::vector<std::string> rows; // row names it emits (also accepted by -p)
    BenchFn fn;
};

BenchFn single(bench::Result (*fn)(Instance &, const Parameters &, const Input &)) {
    return [fn](Instance &instance, const Parameters &params, const Input &input) {
        return std::vector<bench::Result>{fn(instance, params, input)};
    };
}

// For the disabled benchmarks below, which still draw their own random input.
[[maybe_unused]] BenchFn single(bench::Result (*fn)(Instance &, const Parameters &)) {
    return [fn](Instance &instance, const Parameters &params, const Input & /*input*/) {
        return std::vector<bench::Result>{fn(instance, params)};
    };
}

const std::vector<Entry> &registry() {
    static const std::vector<Entry> reg = {
        {"taillard-insertion", {"taillard-insertion"}, single(bench_taillard_insertion)},
        {"rls", {"rls-taillard", "rls-taillard-cutoff", "rls-grabowski"}, bench_rls},
        {"partial-recalc", {"partial-recalc"}, single(bench_partial_recalc)},
        // superseded by the "rls" group (rls_grabowski is now compared against naive::rls):
        // {"grabowski-block", {"grabowski-block"}, single(bench_grabowski_block)},
        // {"edge-insertion", {"edge-insertion"}, single(bench_edge_insertion)},
        // {"alpha-sigma", {"alpha-sigma"}, single(bench_alpha_sigma)},
        // {"preallocated-buffers", {"preallocated-buffers"}, single(bench_preallocated_buffers)},
    };
    return reg;
}

} // namespace

std::vector<bench::Result> bench::run(const std::optional<std::string> &which, Instance &instance,
                                      const Parameters &params) {
    std::vector<bench::Result> out;
    const size_t seed = RNG::instance().seed();

    // One starting point per instance, shared by every benchmark (see Input).
    RNG::instance().set_seed(seed);
    Input input;
    input.start = random_perm(instance.num_jobs());
    input.ref = random_perm(instance.num_jobs());

    for (const auto &entry : registry()) {
        const bool whole_group = !which || *which == entry.group;
        const bool one_row =
            which && std::find(entry.rows.begin(), entry.rows.end(), *which) != entry.rows.end();
        if (!whole_group && !one_row) {
            continue;
        }
        // Only matters for benchmarks that still draw their own randomness (the
        // disabled ones): keeps them independent of which benchmarks ran before.
        RNG::instance().set_seed(seed);
        for (auto &r : entry.fn(instance, params, input)) {
            if (!r.correct) {
                std::cerr << "AVISO: verificação falhou em " << r.speedup << " (" << r.note << ")\n";
            }
            if (whole_group || r.speedup == *which) {
                out.push_back(std::move(r));
            }
        }
    }
    return out;
}

std::string bench::header() {
    return "speedup,n,m,iters,naive_ms,accel_ms,factor,pruned_machines_pct,discarded_positions_pct,note";
}

std::string bench::format(const bench::Result &r) {
    std::ostringstream os;
    os << r.speedup << ',' << r.n << ',' << r.m << ',' << r.iters << ',' << r.naive_ms << ',' << r.accel_ms << ','
       << r.factor << ',';
    if (r.pruned_machines_pct) {
        os << *r.pruned_machines_pct;
    }
    os << ',';
    if (r.discarded_positions_pct) {
        os << *r.discarded_positions_pct;
    }
    os << ',' << r.note;
    return os.str();
}
