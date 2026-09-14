#include "Parameters.h"

#include "argparse/argparse.hpp"

#include <iostream>

namespace {
void config_argparse(argparse::ArgumentParser &cli) {
    cli.add_argument("instance").help("instance path");

    cli.add_argument("-a", "--algorithm")
        .help("construction heuristic to run: NEH, PF, PF_NEH, PFT, PFT_NEH, LPT, MinMax, mNEH, PW, GRASP_NEH "
              "(omit to run all of them)")
        .metavar("ALGORITHM");

    cli.add_argument("-p", "--speedup")
        .help("speed up to benchmark: taillard-insertion, grabowski-block, partial-recalc, edge-insertion, "
              "alpha-sigma, preallocated-buffers (omit to benchmark all of them)")
        .metavar("SPEEDUP");

    cli.add_argument("-n", "--iters")
        .help("benchmark repetitions per (speed up, instance)")
        .metavar("ITERS")
        .default_value(size_t(5))
        .scan<'i', size_t>();

    cli.add_argument("-s", "--seed")
        .help("set random number generator seed (used by GRASP_NEH)")
        .metavar("SEED")
        .scan<'i', size_t>();

    cli.add_argument("-v", "--verbose")
        .help("print the resulting job sequence in addition to the cost")
        .metavar("VERBOSE")
        .default_value(false)
        .flag();

    cli.add_argument("--lambda")
        .help("PF_NEH/PFT_NEH split point; defaults to (n > 200 ? 20 : n)")
        .metavar("LAMBDA")
        .scan<'i', size_t>();

    cli.add_argument("--minmax-alpha")
        .help("MinMax alpha parameter (Ronconi, https://doi.org/10.1016/S0925-5273(03)00065-3)")
        .metavar("ALPHA")
        .default_value(0.60)
        .scan<'f', double>();

    cli.add_argument("--mneh-alpha")
        .help("mNEH alpha parameter")
        .metavar("ALPHA")
        .default_value(0.8)
        .scan<'f', double>();

    cli.add_argument("--beta")
        .help("GRASP_NEH RCL threshold parameter")
        .metavar("BETA")
        .default_value(5e-4)
        .scan<'f', double>();

    cli.add_argument("--delta")
        .help("GRASP_NEH split point between the GRASP phase and the NEH phase")
        .metavar("DELTA")
        .default_value(size_t(20))
        .scan<'i', size_t>();
}
} // namespace

Parameters::Parameters(int argc, char **argv) {

    argparse::ArgumentParser cli("speed-ups", "1.0", argparse::default_arguments::help);

    config_argparse(cli);

    try {
        cli.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cout << err.what() << '\n';
        std::cout << cli;
        exit(EXIT_FAILURE);
    }

    m_instance_path = cli.get<std::string>("instance");
    m_algorithm = cli.present<std::string>("--algorithm");
    m_speedup = cli.present<std::string>("--speedup");
    m_iters = cli.get<size_t>("--iters");
    m_verbose = cli.get<bool>("--verbose");
    m_seed = cli.present<size_t>("--seed");
    m_lambda = cli.present<size_t>("--lambda");
    m_minmax_alpha = cli.get<double>("--minmax-alpha");
    m_mneh_alpha = cli.get<double>("--mneh-alpha");
    m_beta = cli.get<double>("--beta");
    m_delta = cli.get<size_t>("--delta");
}
