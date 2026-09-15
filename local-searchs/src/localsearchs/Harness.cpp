#include "localsearchs/Harness.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <numeric>
#include <sstream>
#include <utility>

#include "Core.h"
#include "RNG.h"

#include "constructions/NEH.h"
#include "local-search/BestSwapIg.h"
#include "local-search/EdgeInsertion.h"
#include "local-search/Grabowski.h"
#include "local-search/Hvns.h"
#include "local-search/MrlsPEda.h"
#include "local-search/RLS.h"
#include "local-search/RlsDeAbc.h"
#include "local-search/Swap.h"
#include "local-search/SvnsD.h"
#include "local-search/SvnsS.h"
#include "local-search/Tpa.h"
#include "local-search/Vnd.h"

namespace {

// Same convention as speed-ups/src/speedups/Bench.cpp's random_perm().
std::vector<size_t> random_perm(size_t n) {
    std::vector<size_t> v(n);
    std::iota(v.begin(), v.end(), 0);
    std::shuffle(v.begin(), v.end(), RNG::instance().gen());
    return v;
}

// Re-runs `setup()` (reseeds RNG + resets working state to the shared
// `initial` solution) before every one of `iters` repetitions, so every
// repetition of every entry starts from the identical state - required
// because, unlike speed-ups/Bench.cpp (which times a pure function), each
// local search here mutates its working Solution cumulatively.
double time_reset_ms(const std::function<void()> &setup, const std::function<void()> &fn, size_t iters) {
    double total = 0.0;
    for (size_t it = 0; it < iters; it++) {
        setup();
        const auto start = std::chrono::steady_clock::now();
        fn();
        const auto end = std::chrono::steady_clock::now();
        total += std::chrono::duration<double, std::milli>(end - start).count();
    }
    return total / (double)iters;
}

ls::Result blank(const char *name, Instance &instance, const Parameters &params, size_t initial_cost) {
    ls::Result r;
    r.name = name;
    r.n = instance.num_jobs();
    r.m = instance.num_machines();
    r.iters = params.iters();
    r.initial_cost = initial_cost;
    return r;
}

void finish(ls::Result &r, size_t final_cost) {
    r.final_cost = final_cost;
    r.improvement_abs = (double)r.initial_cost - (double)final_cost;
    r.improvement_pct = r.initial_cost > 0 ? (r.improvement_abs / (double)r.initial_cost * 100.0) : 0.0;
}

// ---------------------------------------------------------------------------
// #1/#2 - canonical RLS (partial-recalc cluster: MA/DIWO/MFFO/SaDIWO/HDDE/speed-ups)
// ---------------------------------------------------------------------------
ls::Result bench_rls(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("rls", instance, params, initial.cost);
    Solution s;
    std::vector<size_t> ref;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
            ref = random_perm(instance.num_jobs());
        },
        [&] { rls(s, ref, instance); }, params.iters());
    finish(r, s.cost);
    return r;
}

ls::Result bench_rls_grabowski(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("rls-grabowski", instance, params, initial.cost);
    Solution s;
    std::vector<size_t> ref;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
            ref = random_perm(instance.num_jobs());
        },
        [&] { rls_grabowski(s, ref, instance); }, params.iters());
    finish(r, s.cost);
    return r;
}

// ---------------------------------------------------------------------------
// #3 - DE_ABC's single-pass rls (own signature, no ref parameter)
// ---------------------------------------------------------------------------
ls::Result bench_rls_de_abc(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("rls-de-abc", instance, params, initial.cost);
    Solution s;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
        },
        [&] { rls_de_abc(s, instance); }, params.iters());
    finish(r, s.cost);
    return r;
}

// ---------------------------------------------------------------------------
// #4 - P_EDA's mrls (periodic RNG reshuffle of the reference on wrap)
// ---------------------------------------------------------------------------
ls::Result bench_mrls_p_eda(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("mrls-p-eda", instance, params, initial.cost);
    Solution s;
    std::vector<size_t> ref;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
            ref = random_perm(instance.num_jobs());
        },
        [&] { mrls_p_eda(s, ref, instance); }, params.iters());
    finish(r, s.cost);
    return r;
}

// ---------------------------------------------------------------------------
// #5/#6 - BestSwap (IG_IJ/IG_VND1/IG_VND2): faithful bug + fixed comparison
// ---------------------------------------------------------------------------
ls::Result bench_best_swap_ig(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("best-swap-ig", instance, params, initial.cost);
    Solution s;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
        },
        [&] { best_swap_ig(s, instance); }, params.iters());
    finish(r, s.cost);
    r.note = "comparison bug reproduced faithfully: always a no-op, see README";
    return r;
}

ls::Result bench_best_swap_ig_fixed(Instance &instance, const Solution &initial, const Parameters &params,
                                    size_t seed) {
    ls::Result r = blank("best-swap-ig-fixed", instance, params, initial.cost);
    Solution s;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
        },
        [&] { best_swap_ig_fixed(s, instance); }, params.iters());
    finish(r, s.cost);
    return r;
}

// ---------------------------------------------------------------------------
// #7 - IG::local_search (first-improvement swap-only)
// ---------------------------------------------------------------------------
ls::Result bench_swap_first_ig(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("swap-first-ig", instance, params, initial.cost);
    Solution s;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
        },
        [&] { swap_local_search_first(s, instance); }, params.iters());
    finish(r, s.cost);
    return r;
}

// ---------------------------------------------------------------------------
// #8/#9 - VND drivers (IG_VND1, IG_VND2)
// ---------------------------------------------------------------------------
ls::Result bench_vnd1(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("vnd1", instance, params, initial.cost);
    Solution s;
    std::vector<size_t> reference;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
            reference = initial.sequence;
        },
        [&] { vnd1(s, reference, instance); }, params.iters());
    finish(r, s.cost);
    r.note = "k=2 (best-swap-ig) never contributes: see best-swap-ig's bug";
    return r;
}

ls::Result bench_vnd2(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("vnd2", instance, params, initial.cost);
    Solution s;
    std::vector<size_t> reference;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
            reference = initial.sequence;
        },
        [&] { vnd2(s, reference, instance); }, params.iters());
    finish(r, s.cost);
    r.note = "k=1 (best-swap-ig) is dead: see best-swap-ig's bug";
    return r;
}

// ---------------------------------------------------------------------------
// #10/#11 - vnd1/vnd2 with the BestSwap comparison bug fixed (not in the repo)
// ---------------------------------------------------------------------------
ls::Result bench_vnd1_fixed(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("vnd1-fixed", instance, params, initial.cost);
    Solution s;
    std::vector<size_t> reference;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
            reference = initial.sequence;
        },
        [&] { vnd1_fixed(s, reference, instance); }, params.iters());
    finish(r, s.cost);
    return r;
}

ls::Result bench_vnd2_fixed(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("vnd2-fixed", instance, params, initial.cost);
    Solution s;
    std::vector<size_t> reference;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
            reference = initial.sequence;
        },
        [&] { vnd2_fixed(s, reference, instance); }, params.iters());
    finish(r, s.cost);
    return r;
}

// ---------------------------------------------------------------------------
// #12 - IG_IJ's probabilistic dispatch (not a VND)
// ---------------------------------------------------------------------------
ls::Result bench_ig_ij_dispatch(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("ig-ij-dispatch", instance, params, initial.cost);
    Solution s;
    std::vector<size_t> reference;
    bool took_best_swap = false;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
            reference = initial.sequence;
        },
        [&] { took_best_swap = ig_ij_dispatch(s, reference, instance, params.jp()); }, params.iters());
    finish(r, s.cost);
    r.note = std::string("branch=") + (took_best_swap ? "best-swap-ig" : "rls") + " jp=" + std::to_string(params.jp());
    return r;
}

// ---------------------------------------------------------------------------
// #13/#14 - SVNS_S (single pass per call)
// ---------------------------------------------------------------------------
ls::Result bench_svns_s_swap(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("svns-s-swap", instance, params, initial.cost);
    Solution s;
    std::vector<size_t> reference;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
            reference = random_perm(instance.num_jobs());
        },
        [&] { ls1_s_swap(s, reference, instance); }, params.iters());
    finish(r, s.cost);
    return r;
}

ls::Result bench_svns_s_insertion(Instance &instance, const Solution &initial, const Parameters &params,
                                  size_t seed) {
    ls::Result r = blank("svns-s-insertion", instance, params, initial.cost);
    Solution s;
    std::vector<size_t> reference;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
            reference = random_perm(instance.num_jobs());
        },
        [&] { ls2_s_insertion(s, reference, instance); }, params.iters());
    finish(r, s.cost);
    return r;
}

// ---------------------------------------------------------------------------
// #15/#16 - SVNS_D (iterated to convergence/time-limit - the driver from solve())
// ---------------------------------------------------------------------------
ls::Result bench_svns_d_swap(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("svns-d-swap", instance, params, initial.cost);
    const double time_limit_ms =
        params.svns_time_limit_ms().value_or((double)(instance.num_jobs() * instance.num_machines()));
    Solution s;
    std::vector<size_t> reference;
    size_t passes = 0;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
            reference.resize(instance.num_jobs());
            std::iota(reference.begin(), reference.end(), 0);
            passes = 0;
        },
        [&] {
            const auto start = std::chrono::steady_clock::now();
            while (std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count() <=
                   time_limit_ms) {
                std::shuffle(reference.begin(), reference.end(), RNG::instance().gen());
                passes++;
                if (!ls1_d_swap(s, reference, instance)) {
                    break;
                }
            }
        },
        params.iters());
    finish(r, s.cost);
    r.note = "passes=" + std::to_string(passes) + " time_limit_ms=" + std::to_string(time_limit_ms);
    return r;
}

ls::Result bench_svns_d_insertion(Instance &instance, const Solution &initial, const Parameters &params,
                                  size_t seed) {
    ls::Result r = blank("svns-d-insertion", instance, params, initial.cost);
    const double time_limit_ms =
        params.svns_time_limit_ms().value_or((double)(instance.num_jobs() * instance.num_machines()));
    Solution s;
    std::vector<size_t> reference;
    size_t passes = 0;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
            reference.resize(instance.num_jobs());
            std::iota(reference.begin(), reference.end(), 0);
            passes = 0;
        },
        [&] {
            const auto start = std::chrono::steady_clock::now();
            while (std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count() <=
                   time_limit_ms) {
                std::shuffle(reference.begin(), reference.end(), RNG::instance().gen());
                passes++;
                if (!ls2_d_insertion(s, reference, instance)) {
                    break;
                }
            }
        },
        params.iters());
    finish(r, s.cost);
    r.note = "passes=" + std::to_string(passes) + " time_limit_ms=" + std::to_string(time_limit_ms);
    return r;
}

// ---------------------------------------------------------------------------
// #17/#18/#19 - HVNS's three deterministic neighbourhoods (reused from speed-ups)
// ---------------------------------------------------------------------------
ls::Result bench_hvns_best_insertion(Instance &instance, const Solution &initial, const Parameters &params,
                                     size_t seed) {
    ls::Result r = blank("hvns-best-insertion", instance, params, initial.cost);
    EdgeInsertion ei(instance);
    Solution s;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
        },
        [&] { ei.single_insertion_local_search(s); }, params.iters());
    finish(r, s.cost);
    return r;
}

ls::Result bench_hvns_best_edge_insertion(Instance &instance, const Solution &initial, const Parameters &params,
                                          size_t seed) {
    ls::Result r = blank("hvns-best-edge-insertion", instance, params, initial.cost);
    EdgeInsertion ei(instance);
    Solution s;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
        },
        [&] { ei.edge_insertion_local_search(s); }, params.iters());
    finish(r, s.cost);
    return r;
}

ls::Result bench_hvns_best_swap(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("hvns-best-swap", instance, params, initial.cost);
    Solution s;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
        },
        [&] { swap_local_search_best(s, instance); }, params.iters());
    finish(r, s.cost);
    return r;
}

// ---------------------------------------------------------------------------
// #20 - HVNS's VNS restart cycle (k=1..3), without the interleaved SA phase
// ---------------------------------------------------------------------------
ls::Result bench_hvns_shaking(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("hvns-shaking", instance, params, initial.cost);
    Solution s;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            s = initial;
        },
        [&] { hvns_shaking(s, instance, params.hvns_time_limit_ms()); }, params.iters());
    finish(r, s.cost);
    return r;
}

// ---------------------------------------------------------------------------
// #21/#22 - HVNS's SA-flavoured local searches
// ---------------------------------------------------------------------------
ls::Result bench_hvns_sa_rls(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("hvns-sa-rls", instance, params, initial.cost);
    const HvnsSaParams sa = hvns_compute_sa_params(instance, params.hvns_n_iter());
    Solution current;
    Solution best;
    double T = 0.0;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            current = initial;
            best = initial;
            T = sa.t_init;
        },
        [&] { hvns_sa_rls(current, best, instance, T, sa.beta, params.hvns_time_limit_ms()); }, params.iters());
    finish(r, best.cost);
    r.note = "final_T=" + std::to_string(T) + " current_cost=" + std::to_string(current.cost);
    return r;
}

ls::Result bench_hvns_sa_edge_insertion(Instance &instance, const Solution &initial, const Parameters &params,
                                        size_t seed) {
    ls::Result r = blank("hvns-sa-edge-insertion", instance, params, initial.cost);
    const HvnsSaParams sa = hvns_compute_sa_params(instance, params.hvns_n_iter());
    Solution current;
    Solution best;
    double T = 0.0;
    r.time_ms = time_reset_ms(
        [&] {
            RNG::instance().set_seed(seed);
            current = initial;
            best = initial;
            T = sa.t_init;
        },
        [&] { hvns_sa_best_edge_insertion(current, best, instance, T, sa.beta, params.hvns_time_limit_ms()); },
        params.iters());
    finish(r, best.cost);
    r.note = "final_T=" + std::to_string(T) + " current_cost=" + std::to_string(current.cost);
    return r;
}

// ---------------------------------------------------------------------------
// #23 - TPA's SimulatedAnnealing::anneal
// ---------------------------------------------------------------------------
ls::Result bench_tpa_sa(Instance &instance, const Solution &initial, const Parameters &params, size_t seed) {
    ls::Result r = blank("tpa-sa", instance, params, initial.cost);
    TpaSaResult sa_result;
    r.time_ms = time_reset_ms(
        [&] { RNG::instance().set_seed(seed); },
        [&] {
            sa_result = tpa_anneal_sa(initial, instance, params.tpa_n_iter(), params.tpa_final_temp(),
                                      params.tpa_time_limit_ms());
        },
        params.iters());
    finish(r, sa_result.best.cost);
    r.note = "iters=" + std::to_string(sa_result.iterations) + " final_T=" + std::to_string(sa_result.final_temp) +
             " current_cost=" + std::to_string(sa_result.final_current.cost);
    return r;
}

using LsFn = std::function<ls::Result(Instance &, const Solution &, const Parameters &, size_t)>;

const std::vector<std::pair<std::string, LsFn>> &registry() {
    static const std::vector<std::pair<std::string, LsFn>> reg = {
        {"rls", bench_rls},
        {"rls-grabowski", bench_rls_grabowski},
        {"rls-de-abc", bench_rls_de_abc},
        {"mrls-p-eda", bench_mrls_p_eda},
        {"best-swap-ig", bench_best_swap_ig},
        {"best-swap-ig-fixed", bench_best_swap_ig_fixed},
        {"swap-first-ig", bench_swap_first_ig},
        {"vnd1", bench_vnd1},
        {"vnd2", bench_vnd2},
        {"vnd1-fixed", bench_vnd1_fixed},
        {"vnd2-fixed", bench_vnd2_fixed},
        {"ig-ij-dispatch", bench_ig_ij_dispatch},
        {"svns-s-swap", bench_svns_s_swap},
        {"svns-s-insertion", bench_svns_s_insertion},
        {"svns-d-swap", bench_svns_d_swap},
        {"svns-d-insertion", bench_svns_d_insertion},
        {"hvns-best-insertion", bench_hvns_best_insertion},
        {"hvns-best-edge-insertion", bench_hvns_best_edge_insertion},
        {"hvns-best-swap", bench_hvns_best_swap},
        {"hvns-shaking", bench_hvns_shaking},
        {"hvns-sa-rls", bench_hvns_sa_rls},
        {"hvns-sa-edge-insertion", bench_hvns_sa_edge_insertion},
        {"tpa-sa", bench_tpa_sa},
    };
    return reg;
}

} // namespace

std::vector<ls::Result> ls::run(const std::optional<std::string> &which, Instance &instance, const Solution &initial,
                                const Parameters &params, size_t seed) {
    std::vector<ls::Result> out;
    for (const auto &[name, fn] : registry()) {
        if (which && *which != name) {
            continue;
        }
        out.push_back(fn(instance, initial, params, seed));
    }
    return out;
}

std::string ls::header() {
    return "localsearch,n,m,iters,initial_cost,final_cost,improvement_abs,improvement_pct,time_ms,note";
}

std::string ls::format(const ls::Result &r) {
    std::ostringstream os;
    os << r.name << ',' << r.n << ',' << r.m << ',' << r.iters << ',' << r.initial_cost << ',' << r.final_cost << ','
       << r.improvement_abs << ',' << r.improvement_pct << ',' << r.time_ms << ',' << r.note;
    return os.str();
}
