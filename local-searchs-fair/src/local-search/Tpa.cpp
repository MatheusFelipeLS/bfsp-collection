#include "local-search/Tpa.h"

#include <chrono>
#include <cmath>

#include "Core.h"
#include "RNG.h"
#include "constructions/NEH.h"

namespace {

// Ported verbatim from SimulatedAnnealing::anneal (TPA/src/SimulatedAnnealing.cpp:123-136).
Solution tpa_anneal_move(Solution &current_solution, size_t position, Instance &instance) {
    NEH helper(instance);

    Solution new_sol = current_solution;
    size_t job = current_solution.sequence[position];
    new_sol.sequence.erase(new_sol.sequence.begin() + (long)position);

    auto [best_index, makespan] = helper.mtaillard_best_insertion(new_sol.sequence, job, position);

    new_sol.sequence.insert(new_sol.sequence.begin() + (long)best_index, job);
    new_sol.cost = makespan;

    return new_sol;
}

} // namespace

TpaSaResult tpa_anneal_sa(const Solution &initial, Instance &instance, size_t n_iter, double final_temp,
                          double time_limit_ms) {
    // Ported from SimulatedAnnealing::calculate_initial_temp/calculate_decay
    // (TPA/src/SimulatedAnnealing.cpp:106-121).
    const double initial_temp =
        static_cast<double>(instance.all_processing_times_sum()) /
        (static_cast<double>(5 * instance.num_jobs() * instance.num_machines()));
    const double decay = (initial_temp - final_temp) / ((double)(n_iter - 1) * initial_temp * final_temp);

    double current_temp = initial_temp;
    const size_t n_jobs = instance.num_jobs();

    TpaSaResult result;
    result.best = initial;
    result.final_current = initial;
    size_t best_cost = initial.cost;

    Solution current_solution = initial;

    const auto start = std::chrono::steady_clock::now();

    // Ported from SimulatedAnnealing::solve() (TPA/src/SimulatedAnnealing.cpp:39-104),
    // with the uptime()-based real time_limit replaced by a synthetic ms budget.
    while (true) {
        const double elapsed_ms =
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
        if (elapsed_ms > time_limit_ms) {
            break;
        }

        auto position = RNG::instance().generate<size_t>(0, n_jobs - 1);
        Solution new_sol = tpa_anneal_move(current_solution, position, instance);
        result.iterations++;

        int delta = (int)new_sol.cost - (int)current_solution.cost;

        if (delta <= 0) {
            current_solution.sequence = new_sol.sequence;
            core::recalculate_solution(instance, current_solution);

            if (new_sol.cost < best_cost) {
                result.best = new_sol;
                best_cost = new_sol.cost;
            }
        } else {
            double acceptance_probability = std::exp(-delta / current_temp);
            auto random = RNG::instance().generate_real_number(0, 1);

            if (acceptance_probability > random) {
                current_solution.sequence = new_sol.sequence;
                core::recalculate_solution(instance, current_solution);
            }
        }

        current_temp = current_temp / (1 + (decay * current_temp));
    }

    result.final_current = current_solution;
    result.final_temp = current_temp;
    return result;
}
