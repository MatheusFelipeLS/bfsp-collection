#include <array>
#include <chrono>
#include <iostream>

#include "Instance.h"
#include "Parameters.h"
#include "RNG.h"
#include "Solution.h"

#include "constructions/GRASP_NEH.h"
#include "constructions/LPT.h"
#include "constructions/MinMax.h"
#include "constructions/NEH.h"
#include "constructions/PF.h"
#include "constructions/PFT.h"
#include "constructions/PFT_NEH.h"
#include "constructions/PF_NEH.h"
#include "constructions/PW.h"
#include "constructions/mNEH.h"

namespace {

constexpr std::array<const char *, 10> ALGORITHMS = {"NEH",  "PF",     "PF_NEH", "PFT", "PFT_NEH",
                                                      "LPT",  "MinMax", "mNEH",   "PW",  "GRASP_NEH"};

void print_solution(const std::string &algorithm, const Solution &s, double elapsed_ms, bool verbose) {
    std::cout << s.cost << ',' << elapsed_ms << '\n';

    if (verbose) {
        std::cout << "sequence: [";
        for (size_t i = 0; i < s.sequence.size(); i++) {
            std::cout << s.sequence[i] << (i + 1 != s.sequence.size() ? ", " : "");
        }
        std::cout << "]\n";
    }
}

size_t default_lambda(const Parameters &params, const Instance &instance) {
    if (auto l = params.lambda()) {
        return *l;
    }
    return instance.num_jobs() > 200 ? 200 : instance.num_jobs();
}

// Returns false when the algorithm name is not recognized.
bool run_algorithm(const std::string &alg, Instance &instance, const Parameters &params) {

    const auto start = std::chrono::steady_clock::now();
    Solution solution;

    if (alg == "NEH") {
        NEH neh(instance);
        solution = neh.solve(LPT::solve(instance).sequence);
    } else if (alg == "PF") {
        PF pf(instance);
        solution = pf.solve();
    } else if (alg == "PF_NEH") {
        PF_NEH pf_neh(instance);
        solution = pf_neh.solve(default_lambda(params, instance));
    } else if (alg == "PFT") {
        PFT pft(instance);
        solution = pft.solve();
    } else if (alg == "PFT_NEH") {
        PFT_NEH pft_neh(instance);
        solution = pft_neh.solve(default_lambda(params, instance));
    } else if (alg == "LPT") {
        solution = LPT::solve(instance);
    } else if (alg == "MinMax") {
        MinMax mm(instance, params.minmax_alpha());
        solution = mm.solve();
    } else if (alg == "mNEH") {
        solution = MNEH::solve(params.mneh_alpha(), instance);
    } else if (alg == "PW") {
        PW pw(instance);
        solution = pw.solve();
    } else if (alg == "GRASP_NEH") {
        GRASP_NEH grasp_neh(instance, params.delta(), params.beta());
        solution = grasp_neh.solve();
    } else {
        return false;
    }

    const auto end = std::chrono::steady_clock::now();
    const double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();

    print_solution(alg, solution, elapsed_ms, params.verbose());

    return true;
}

} // namespace

int main(int argc, char *argv[]) {

    Parameters params(argc, argv);
    Instance instance(params.instance_path());

    if (auto seed = params.seed()) {
        RNG::instance().set_seed(*seed);
    }

    if (auto alg = params.algorithm()) {
        if (!run_algorithm(*alg, instance, params)) {
            std::cerr << "unknown algorithm: " << *alg << '\n';
            std::cerr << "available: NEH, PF, PF_NEH, PFT, PFT_NEH, LPT, MinMax, mNEH, PW, GRASP_NEH\n";
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    for (const char *alg : ALGORITHMS) {
        run_algorithm(alg, instance, params);
    }

    return EXIT_SUCCESS;
}
