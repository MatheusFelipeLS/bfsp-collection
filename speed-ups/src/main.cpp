#include <iostream>

#include "Instance.h"
#include "Parameters.h"
#include "RNG.h"

#include "speedups/Bench.h"

// Benchmark driver for the BFSP evaluation speed ups.
//
//   ./speed-ups <instance>                     # benchmark every speed up
//   ./speed-ups <instance> -p edge-insertion   # just one
//   ./speed-ups <instance> -n 20 -s 1          # 20 reps, RNG seed 1
//
// Prints a CSV header on stderr and one CSV row per speed up on stdout:
//   speedup,n,m,iters,naive_ms,accel_ms,factor,correct,note
//
// `factor` is naive_ms / accel_ms (how many times faster the accelerated
// component is). `correct` is 1 when the accelerated result matches the
// un-accelerated baseline exactly. See README.md for what each row measures.
int main(int argc, char *argv[]) {

    Parameters params(argc, argv);
    Instance instance(params.instance_path());

    // Deterministic by default so runs are reproducible; -s overrides.
    RNG::instance().set_seed(params.seed().value_or(42));

    const auto results = bench::run(params.speedup(), instance, params);

    std::cerr << bench::header() << '\n';
    for (const auto &r : results) {
        std::cout << bench::format(r) << '\n';
    }

    return EXIT_SUCCESS;
}
