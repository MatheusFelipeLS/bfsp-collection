#include "localsearchs/IgHost.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <numeric>
#include <sstream>
#include <utility>

#include "Core.h"
#include "RNG.h"

#include "constructions/NEH.h"
#include "constructions/PF_NEH.h"
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

// Same convention as local-searchs-fair's Harness.cpp.
std::vector<size_t> random_perm(size_t n) {
    std::vector<size_t> v(n);
    std::iota(v.begin(), v.end(), 0);
    std::shuffle(v.begin(), v.end(), RNG::instance().gen());
    return v;
}

// Ported from IG_IJ::destroy (IG_IJ/src/IG_IJ.cpp:163-178), unchanged.
std::vector<size_t> ig_destroy(Solution &s, size_t d) {
    const size_t destroy_size = std::min(s.sequence.size() - 1, d);
    std::vector<size_t> removed;
    removed.reserve(destroy_size);
    for (size_t i = 0; i < destroy_size; i++) {
        const long chose = RNG::instance().generate<long>(0, (long)s.sequence.size() - 1);
        removed.push_back(s.sequence[chose]);
        s.sequence.erase(s.sequence.begin() + chose);
    }
    return removed;
}

// Ported from IG_IJ::acceptance_criterion (IG_IJ/src/IG_IJ.cpp:34-38), unchanged.
double ig_acceptance_criterion(const Solution &pi_0, const Solution &pi_2, double T) {
    const double delta = (double)pi_0.cost - (double)pi_2.cost;
    return std::exp(-delta / T);
}

// The "local search slot" every catalog entry plugs into: given the
// incumbent (already destroyed+reconstructed by the host loop) and the SAME
// job-id reference IG_IJ itself uses (`reference = current.sequence` from
// the initial PF_NEH solution, fixed for the whole run - IG_IJ.cpp:95),
// improve `s` in place. Entries that don't use a reference, or that need a
// differently-shaped one (the SVNS "positions" family), ignore the third
// argument and close over their own state instead - see the bench_* functions
// below.
using StepFn = std::function<void(Solution &, Instance &, std::vector<size_t> &)>;

// ---------------------------------------------------------------------------
// The host loop itself - IG_IJ::solve() (IG_IJ/src/IG_IJ.cpp:85-161),
// generalized with a pluggable `step` in place of the hardcoded
// `if (rand<jp) BestSwap(); else rls();` block (lines 126-131), and
// instrumented to report convergence data instead of just the final best
// cost. Everything else - PF_NEH construction, destroy/reconstruct,
// SA-style accept/reject, the wall-clock stopping condition - is unchanged.
// ---------------------------------------------------------------------------
ig::RunResult run_ig_once(const char *name, Instance &instance, const Parameters &params, size_t seed,
                          const StepFn &step, std::vector<ig::CallRecord> *trajectory) {
    RNG::instance().set_seed(seed);

    const size_t lambda = instance.num_jobs() > 200 ? 20 : instance.num_jobs();
    PF_NEH pf_neh(instance);
    Solution current = pf_neh.solve(lambda);
    Solution best = current;
    Solution incumbent = current;
    const size_t initial_cost = current.cost;
    std::vector<size_t> reference = current.sequence; // IG_IJ.cpp:95, fixed for the whole run

    NEH neh(instance);
    const double T = params.temperature() * (double)instance.all_processing_times_sum() /
                      (10.0 * (double)instance.num_jobs() * (double)instance.num_machines());

    const double time_limit_ms = params.time_limit_ms();
    const size_t destroy_size = params.destroy_size();
    const size_t trajectory_max_rows = params.trajectory_max_rows();

    size_t n_calls = 0;
    size_t call_of_best = 0;
    double improvement_sum = 0.0;

    const auto start = std::chrono::steady_clock::now();

    while (true) {
        std::vector<size_t> removed = ig_destroy(incumbent, destroy_size);
        neh.second_step(std::move(removed), incumbent);

        const size_t cost_before_ls = incumbent.cost;
        step(incumbent, instance, reference);
        const size_t cost_after_ls = incumbent.cost;
        n_calls++;
        improvement_sum += (double)cost_before_ls - (double)cost_after_ls;

        const double now_ms =
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();

        const bool record_row = trajectory != nullptr && trajectory->size() < trajectory_max_rows;
        if (record_row) {
            trajectory->push_back(
                {n_calls, cost_before_ls, cost_after_ls, false, current.cost, best.cost, now_ms});
        }

        // Program should not accept any solution if the time is out - matches IG_IJ.cpp:138-141 exactly:
        // the very last (over-budget) call's incumbent is discarded, never folded into current/best.
        if (now_ms > time_limit_ms) {
            break;
        }

        bool accepted = false;
        if (incumbent.cost < current.cost) {
            current = incumbent;
            if (incumbent.cost < best.cost) {
                call_of_best = n_calls;
                best = current = std::move(incumbent);
            }
            accepted = true;
        } else if (RNG::instance().generate_real_number(0, 1) < ig_acceptance_criterion(incumbent, current, T)) {
            current = std::move(incumbent);
            accepted = true;
        }

        if (record_row) {
            trajectory->back().accepted = accepted;
            trajectory->back().current_cost = current.cost;
            trajectory->back().best_cost = best.cost;
        }

        incumbent = current;
    }

    ig::RunResult r;
    r.name = name;
    r.n = instance.num_jobs();
    r.m = instance.num_machines();
    r.iters = params.iters();
    r.seed = seed;
    r.initial_cost = initial_cost;
    r.final_best_cost = best.cost;
    r.n_calls = n_calls;
    r.call_of_best = call_of_best;
    r.avg_improvement_per_call = n_calls > 0 ? improvement_sum / (double)n_calls : 0.0;
    r.time_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    r.time_limit_ms = time_limit_ms;
    return r;
}

// Runs `params.iters()` repetitions of one catalog entry through the host
// loop above. `setup()` (re)initializes any entry-specific persistent state
// (a position-vector reference, an EdgeInsertion buffer, a cooling
// temperature, ...) fresh for each repetition - it runs BEFORE `step` is
// used but AFTER nothing else consumes RNG, and reseeds the RNG itself if it
// needs randomness, so it stays reproducible independent of run_ig_once's
// own internal reseed. Trajectory rows are only collected for repetition 1
// (see Parameters::no_trajectory/trajectory_max_rows).
template <typename SetupFn>
std::vector<ig::RunResult> run_entry(const char *name, Instance &instance, const Parameters &params, size_t seed,
                                     SetupFn setup, const StepFn &step, const std::string &note,
                                     std::vector<ig::TrajectoryBlock> &trajectories) {
    std::vector<ig::RunResult> out;
    out.reserve(params.iters());
    for (size_t it = 0; it < params.iters(); it++) {
        setup();

        std::vector<ig::CallRecord> *traj_ptr = nullptr;
        if (it == 0 && !params.no_trajectory()) {
            trajectories.push_back({name, {}});
            traj_ptr = &trajectories.back().calls;
        }

        ig::RunResult r = run_ig_once(name, instance, params, seed, step, traj_ptr);
        r.iteration = it + 1;
        r.note = note;
        out.push_back(std::move(r));
    }
    return out;
}

void noop_setup() {}

// ---------------------------------------------------------------------------
// #1/#2 - canonical RLS. No extra state: uses the host loop's own job-id
// reference directly, exactly like IG_IJ's own `rls(incumbent, reference,
// m_instance)` call.
// ---------------------------------------------------------------------------
std::vector<ig::RunResult> bench_rls(Instance &instance, const Parameters &params, size_t seed,
                                     std::vector<ig::TrajectoryBlock> &trajectories) {
    StepFn step = [](Solution &s, Instance &i, std::vector<size_t> &ref) { rls(s, ref, i); };
    return run_entry("rls", instance, params, seed, noop_setup, step, "", trajectories);
}

std::vector<ig::RunResult> bench_rls_grabowski(Instance &instance, const Parameters &params, size_t seed,
                                               std::vector<ig::TrajectoryBlock> &trajectories) {
    StepFn step = [](Solution &s, Instance &i, std::vector<size_t> &ref) { rls_grabowski(s, ref, i); };
    return run_entry("rls-grabowski", instance, params, seed, noop_setup, step, "", trajectories);
}

// ---------------------------------------------------------------------------
// #3 - DE_ABC's single-pass rls (own signature, no ref parameter)
// ---------------------------------------------------------------------------
std::vector<ig::RunResult> bench_rls_de_abc(Instance &instance, const Parameters &params, size_t seed,
                                            std::vector<ig::TrajectoryBlock> &trajectories) {
    StepFn step = [](Solution &s, Instance &i, std::vector<size_t> &) { rls_de_abc(s, i); };
    return run_entry("rls-de-abc", instance, params, seed, noop_setup, step, "", trajectories);
}

// ---------------------------------------------------------------------------
// #4 - P_EDA's mrls (periodic RNG reshuffle of the reference on wrap - the
// reshuffles now persist ACROSS IG calls within a run, since `reference` is
// owned by the host loop and mutated in place, same object every call)
// ---------------------------------------------------------------------------
std::vector<ig::RunResult> bench_mrls_p_eda(Instance &instance, const Parameters &params, size_t seed,
                                            std::vector<ig::TrajectoryBlock> &trajectories) {
    StepFn step = [](Solution &s, Instance &i, std::vector<size_t> &ref) { mrls_p_eda(s, ref, i); };
    return run_entry("mrls-p-eda", instance, params, seed, noop_setup, step, "", trajectories);
}

// ---------------------------------------------------------------------------
// #5/#6 - BestSwap (IG_IJ/IG_VND1/IG_VND2): faithful bug + fixed comparison.
// `best_swap_ig` is byte-identical to IG_IJ's own private `BestSwap` method
// (IG_IJ.cpp:40-82), bug included.
// ---------------------------------------------------------------------------
std::vector<ig::RunResult> bench_best_swap_ig(Instance &instance, const Parameters &params, size_t seed,
                                              std::vector<ig::TrajectoryBlock> &trajectories) {
    StepFn step = [](Solution &s, Instance &i, std::vector<size_t> &) { best_swap_ig(s, i); };
    return run_entry("best-swap-ig", instance, params, seed, noop_setup, step,
                     "comparison bug reproduced faithfully: always a no-op, see README", trajectories);
}

std::vector<ig::RunResult> bench_best_swap_ig_fixed(Instance &instance, const Parameters &params, size_t seed,
                                                    std::vector<ig::TrajectoryBlock> &trajectories) {
    StepFn step = [](Solution &s, Instance &i, std::vector<size_t> &) { best_swap_ig_fixed(s, i); };
    return run_entry("best-swap-ig-fixed", instance, params, seed, noop_setup, step, "", trajectories);
}

// ---------------------------------------------------------------------------
// #7 - IG::local_search (first-improvement swap-only)
// ---------------------------------------------------------------------------
std::vector<ig::RunResult> bench_swap_first_ig(Instance &instance, const Parameters &params, size_t seed,
                                               std::vector<ig::TrajectoryBlock> &trajectories) {
    StepFn step = [](Solution &s, Instance &i, std::vector<size_t> &) { swap_local_search_first(s, i); };
    return run_entry("swap-first-ig", instance, params, seed, noop_setup, step, "", trajectories);
}

// ---------------------------------------------------------------------------
// #8/#9 - VND drivers (IG_VND1, IG_VND2) - use the host loop's job-id
// reference directly.
// ---------------------------------------------------------------------------
std::vector<ig::RunResult> bench_vnd1(Instance &instance, const Parameters &params, size_t seed,
                                      std::vector<ig::TrajectoryBlock> &trajectories) {
    StepFn step = [](Solution &s, Instance &i, std::vector<size_t> &ref) { vnd1(s, ref, i); };
    return run_entry("vnd1", instance, params, seed, noop_setup, step,
                     "k=2 (best-swap-ig) never contributes: see best-swap-ig's bug", trajectories);
}

std::vector<ig::RunResult> bench_vnd2(Instance &instance, const Parameters &params, size_t seed,
                                      std::vector<ig::TrajectoryBlock> &trajectories) {
    StepFn step = [](Solution &s, Instance &i, std::vector<size_t> &ref) { vnd2(s, ref, i); };
    return run_entry("vnd2", instance, params, seed, noop_setup, step,
                     "k=1 (best-swap-ig) is dead: see best-swap-ig's bug", trajectories);
}

// ---------------------------------------------------------------------------
// #10/#11 - vnd1/vnd2 with the BestSwap comparison bug fixed (not in the repo)
// ---------------------------------------------------------------------------
std::vector<ig::RunResult> bench_vnd1_fixed(Instance &instance, const Parameters &params, size_t seed,
                                            std::vector<ig::TrajectoryBlock> &trajectories) {
    StepFn step = [](Solution &s, Instance &i, std::vector<size_t> &ref) { vnd1_fixed(s, ref, i); };
    return run_entry("vnd1-fixed", instance, params, seed, noop_setup, step, "", trajectories);
}

std::vector<ig::RunResult> bench_vnd2_fixed(Instance &instance, const Parameters &params, size_t seed,
                                            std::vector<ig::TrajectoryBlock> &trajectories) {
    StepFn step = [](Solution &s, Instance &i, std::vector<size_t> &ref) { vnd2_fixed(s, ref, i); };
    return run_entry("vnd2-fixed", instance, params, seed, noop_setup, step, "", trajectories);
}

// ---------------------------------------------------------------------------
// #12 - IG_IJ's own probabilistic dispatch - THE BASELINE/CONTROL entry:
// this reproduces vanilla IG_IJ's local-search step verbatim, so its numbers
// are what "unmodified IG_IJ" looks like under this same instrumentation.
// ---------------------------------------------------------------------------
std::vector<ig::RunResult> bench_ig_ij_dispatch(Instance &instance, const Parameters &params, size_t seed,
                                                std::vector<ig::TrajectoryBlock> &trajectories) {
    const double jp = params.jp();
    StepFn step = [jp](Solution &s, Instance &i, std::vector<size_t> &ref) { ig_ij_dispatch(s, ref, i, jp); };
    return run_entry("ig-ij-dispatch", instance, params, seed, noop_setup, step,
                     "jp=" + std::to_string(jp) + " - reproduces vanilla IG_IJ's own local-search step (baseline)",
                     trajectories);
}

// ---------------------------------------------------------------------------
// #13/#14 - SVNS_S's local searches. These expect a "positions" reference
// (SvnsS.h), a different convention from the job-id one the host loop owns -
// so each bench function keeps its OWN position-vector, shuffled once per
// repetition (mirrors local-searchs-fair's svns-s-* setup), and ignores the
// host loop's job-id reference parameter.
// ---------------------------------------------------------------------------
std::vector<ig::RunResult> bench_svns_s_swap(Instance &instance, const Parameters &params, size_t seed,
                                             std::vector<ig::TrajectoryBlock> &trajectories) {
    std::vector<size_t> pos_ref;
    auto setup = [&] {
        RNG::instance().set_seed(seed);
        pos_ref = random_perm(instance.num_jobs());
    };
    StepFn step = [&](Solution &s, Instance &i, std::vector<size_t> &) { ls1_s_swap(s, pos_ref, i); };
    return run_entry("svns-s-swap", instance, params, seed, setup, step, "", trajectories);
}

std::vector<ig::RunResult> bench_svns_s_insertion(Instance &instance, const Parameters &params, size_t seed,
                                                   std::vector<ig::TrajectoryBlock> &trajectories) {
    std::vector<size_t> pos_ref;
    auto setup = [&] {
        RNG::instance().set_seed(seed);
        pos_ref = random_perm(instance.num_jobs());
    };
    StepFn step = [&](Solution &s, Instance &i, std::vector<size_t> &) { ls2_s_insertion(s, pos_ref, i); };
    return run_entry("svns-s-insertion", instance, params, seed, setup, step, "", trajectories);
}

// ---------------------------------------------------------------------------
// #15/#16 - SVNS_D's local searches - same "positions" convention as #13/#14,
// but identity-ordered (0..n-1) rather than shuffled, matching
// local-searchs-fair's svns-d-* setup.
// ---------------------------------------------------------------------------
std::vector<ig::RunResult> bench_svns_d_swap(Instance &instance, const Parameters &params, size_t seed,
                                             std::vector<ig::TrajectoryBlock> &trajectories) {
    std::vector<size_t> pos_ref;
    auto setup = [&] {
        pos_ref.resize(instance.num_jobs());
        std::iota(pos_ref.begin(), pos_ref.end(), 0);
    };
    StepFn step = [&](Solution &s, Instance &i, std::vector<size_t> &) { ls1_d_swap(s, pos_ref, i); };
    return run_entry("svns-d-swap", instance, params, seed, setup, step, "", trajectories);
}

std::vector<ig::RunResult> bench_svns_d_insertion(Instance &instance, const Parameters &params, size_t seed,
                                                   std::vector<ig::TrajectoryBlock> &trajectories) {
    std::vector<size_t> pos_ref;
    auto setup = [&] {
        pos_ref.resize(instance.num_jobs());
        std::iota(pos_ref.begin(), pos_ref.end(), 0);
    };
    StepFn step = [&](Solution &s, Instance &i, std::vector<size_t> &) { ls2_d_insertion(s, pos_ref, i); };
    return run_entry("svns-d-insertion", instance, params, seed, setup, step, "", trajectories);
}

// ---------------------------------------------------------------------------
// #17/#18/#19 - HVNS's three deterministic neighbourhoods (reused from
// speed-ups). EdgeInsertion's buffers are "refreshed on entry" per call, so
// one instance safely serves every repetition of a run.
// ---------------------------------------------------------------------------
std::vector<ig::RunResult> bench_hvns_best_insertion(Instance &instance, const Parameters &params, size_t seed,
                                                      std::vector<ig::TrajectoryBlock> &trajectories) {
    EdgeInsertion ei(instance);
    StepFn step = [&](Solution &s, Instance &, std::vector<size_t> &) { ei.single_insertion_local_search(s); };
    return run_entry("hvns-best-insertion", instance, params, seed, noop_setup, step, "", trajectories);
}

std::vector<ig::RunResult> bench_hvns_best_edge_insertion(Instance &instance, const Parameters &params, size_t seed,
                                                           std::vector<ig::TrajectoryBlock> &trajectories) {
    EdgeInsertion ei(instance);
    StepFn step = [&](Solution &s, Instance &, std::vector<size_t> &) { ei.edge_insertion_local_search(s); };
    return run_entry("hvns-best-edge-insertion", instance, params, seed, noop_setup, step, "", trajectories);
}

std::vector<ig::RunResult> bench_hvns_best_swap(Instance &instance, const Parameters &params, size_t seed,
                                                std::vector<ig::TrajectoryBlock> &trajectories) {
    StepFn step = [](Solution &s, Instance &i, std::vector<size_t> &) { swap_local_search_best(s, i); };
    return run_entry("hvns-best-swap", instance, params, seed, noop_setup, step, "", trajectories);
}

// ---------------------------------------------------------------------------
// #20/#21/#22/#23 - self-budgeted entries (see README.md "Entradas
// auto-orçadas"): each IG call gives them a short `--slot-time-limit-ms`
// burst instead of a full pass. hvns-sa-* carry a persistent cooling
// temperature `T` across calls within a run (reset each repetition);
// tpa-sa's signature has no such in/out parameter, so it restarts its own
// cooling schedule from scratch every call.
// ---------------------------------------------------------------------------
std::vector<ig::RunResult> bench_hvns_shaking(Instance &instance, const Parameters &params, size_t seed,
                                              std::vector<ig::TrajectoryBlock> &trajectories) {
    const double slot_ms = params.slot_time_limit_ms();
    StepFn step = [slot_ms](Solution &s, Instance &i, std::vector<size_t> &) { hvns_shaking(s, i, slot_ms); };
    return run_entry("hvns-shaking", instance, params, seed, noop_setup, step,
                     "per-call burst of " + std::to_string(slot_ms) + "ms", trajectories);
}

std::vector<ig::RunResult> bench_hvns_sa_rls(Instance &instance, const Parameters &params, size_t seed,
                                             std::vector<ig::TrajectoryBlock> &trajectories) {
    const HvnsSaParams sa = hvns_compute_sa_params(instance, params.hvns_n_iter());
    const double slot_ms = params.slot_time_limit_ms();
    double T = sa.t_init;
    auto setup = [&] { T = sa.t_init; };
    StepFn step = [&](Solution &s, Instance &i, std::vector<size_t> &) {
        Solution local_current = s;
        Solution local_best = s;
        hvns_sa_rls(local_current, local_best, i, T, sa.beta, slot_ms);
        s = local_best;
    };
    return run_entry("hvns-sa-rls", instance, params, seed, setup, step,
                     "per-call burst of " + std::to_string(slot_ms) + "ms; T cools across calls within a run",
                     trajectories);
}

std::vector<ig::RunResult> bench_hvns_sa_edge_insertion(Instance &instance, const Parameters &params, size_t seed,
                                                         std::vector<ig::TrajectoryBlock> &trajectories) {
    const HvnsSaParams sa = hvns_compute_sa_params(instance, params.hvns_n_iter());
    const double slot_ms = params.slot_time_limit_ms();
    double T = sa.t_init;
    auto setup = [&] { T = sa.t_init; };
    StepFn step = [&](Solution &s, Instance &i, std::vector<size_t> &) {
        Solution local_current = s;
        Solution local_best = s;
        hvns_sa_best_edge_insertion(local_current, local_best, i, T, sa.beta, slot_ms);
        s = local_best;
    };
    return run_entry("hvns-sa-edge-insertion", instance, params, seed, setup, step,
                     "per-call burst of " + std::to_string(slot_ms) + "ms; T cools across calls within a run",
                     trajectories);
}

std::vector<ig::RunResult> bench_tpa_sa(Instance &instance, const Parameters &params, size_t seed,
                                        std::vector<ig::TrajectoryBlock> &trajectories) {
    const size_t tpa_n_iter = params.tpa_n_iter();
    const double tpa_final_temp = params.tpa_final_temp();
    const double slot_ms = params.slot_time_limit_ms();
    StepFn step = [&](Solution &s, Instance &i, std::vector<size_t> &) {
        TpaSaResult res = tpa_anneal_sa(s, i, tpa_n_iter, tpa_final_temp, slot_ms);
        s = res.best;
    };
    return run_entry("tpa-sa", instance, params, seed, noop_setup, step,
                     "per-call burst of " + std::to_string(slot_ms) +
                         "ms; restarts its own cooling schedule every call (no persistent T in this signature)",
                     trajectories);
}

using BenchFn = std::function<std::vector<ig::RunResult>(Instance &, const Parameters &, size_t,
                                                         std::vector<ig::TrajectoryBlock> &)>;

const std::vector<std::pair<std::string, BenchFn>> &registry() {
    static const std::vector<std::pair<std::string, BenchFn>> reg = {
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

std::vector<ig::RunResult> ig::run(const std::optional<std::string> &which, Instance &instance,
                                   const Parameters &params, size_t seed,
                                   std::vector<ig::TrajectoryBlock> &trajectories) {
    std::vector<ig::RunResult> out;
    for (const auto &[name, fn] : registry()) {
        if (which && *which != name) {
            continue;
        }
        auto results = fn(instance, params, seed, trajectories);
        out.insert(out.end(), std::make_move_iterator(results.begin()), std::make_move_iterator(results.end()));
    }
    return out;
}

std::string ig::header() {
    return "localsearch,n,m,iteration,iters,seed,initial_cost,final_best_cost,n_calls,call_of_best,"
           "avg_improvement_per_call,time_ms,time_limit_ms,note";
}

std::string ig::format(const ig::RunResult &r) {
    std::ostringstream os;
    os << r.name << ',' << r.n << ',' << r.m << ',' << r.iteration << ',' << r.iters << ',' << r.seed << ','
       << r.initial_cost << ',' << r.final_best_cost << ',' << r.n_calls << ',' << r.call_of_best << ','
       << r.avg_improvement_per_call << ',' << r.time_ms << ',' << r.time_limit_ms << ',' << r.note;
    return os.str();
}

std::string ig::trajectory_header() {
    return "localsearch,call_index,cost_before_ls,cost_after_ls,accepted,current_cost,best_cost,elapsed_ms";
}

std::string ig::format(const std::string &name, const ig::CallRecord &c) {
    std::ostringstream os;
    os << name << ',' << c.call_index << ',' << c.cost_before_ls << ',' << c.cost_after_ls << ','
       << (c.accepted ? 1 : 0) << ',' << c.current_cost << ',' << c.best_cost << ',' << c.elapsed_ms;
    return os.str();
}
