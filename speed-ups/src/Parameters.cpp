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
        .default_value(size_t(10))
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
}
