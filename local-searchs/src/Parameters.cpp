#include "Parameters.h"

#include "argparse/argparse.hpp"

#include <iostream>

namespace {
void config_argparse(argparse::ArgumentParser &cli) {
    cli.add_argument("instance").help("instance path");

    cli.add_argument("-p", "--local-search")
        .help("local search to run (see README.md for the full catalog; omit to run all of them)")
        .metavar("LOCAL_SEARCH");

    cli.add_argument("-n", "--iters")
        .help("benchmark repetitions per (local search, instance)")
        .metavar("ITERS")
        .default_value(size_t(5))
        .scan<'i', size_t>();

    cli.add_argument("-s", "--seed")
        .help("random number generator seed; also fixes the shared random starting solution")
        .metavar("SEED")
        .scan<'i', size_t>();

    cli.add_argument("-v", "--verbose")
        .help("print the resulting job sequence in addition to the cost")
        .metavar("VERBOSE")
        .default_value(false)
        .flag();

    cli.add_argument("--jp")
        .help("ig-ij-dispatch jumping probability (IG_IJ/include/Parameters.h)")
        .metavar("JP")
        .default_value(0.001)
        .scan<'f', double>();

    cli.add_argument("--hvns-n-iter")
        .help("n_iter used to shape hvns-sa-rls/hvns-sa-edge-insertion's cooling rate (HVNS/include/Parameters.h)")
        .metavar("N_ITER")
        .default_value(size_t(1800000))
        .scan<'i', size_t>();

    cli.add_argument("--hvns-time-limit-ms")
        .help("synthetic time budget for hvns-shaking/hvns-sa-rls/hvns-sa-edge-insertion (on large instances "
              "their unbounded-by-design loops can otherwise take minutes; see README.md)")
        .metavar("MS")
        .default_value(5000.0)
        .scan<'f', double>();

    cli.add_argument("--tpa-n-iter")
        .help("n_iter used to shape tpa-sa's cooling rate (TPA/include/Parameters.h)")
        .metavar("N_ITER")
        .default_value(size_t(1800000))
        .scan<'i', size_t>();

    cli.add_argument("--tpa-final-temp")
        .help("tpa-sa final temperature (TPA/include/Parameters.h)")
        .metavar("TEMP")
        .default_value(1.0)
        .scan<'f', double>();

    cli.add_argument("--tpa-time-limit-ms")
        .help("synthetic time budget for tpa-sa (the origin's real time_limit depends on the outer TPA "
              "metaheuristic's own parameters and is unavailable standalone)")
        .metavar("MS")
        .default_value(2000.0)
        .scan<'f', double>();

    cli.add_argument("--svns-time-limit-ms")
        .help("synthetic time budget for svns-d-swap/svns-d-insertion's iterate-to-convergence driver "
              "(defaults to n*m ms when omitted; the origin's real time_limit is derived from the outer SVNS_D "
              "metaheuristic's own parameters and is unavailable standalone)")
        .metavar("MS")
        .scan<'f', double>();
}
} // namespace

Parameters::Parameters(int argc, char **argv) {

    argparse::ArgumentParser cli("local-searchs", "1.0", argparse::default_arguments::help);

    config_argparse(cli);

    try {
        cli.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cout << err.what() << '\n';
        std::cout << cli;
        exit(EXIT_FAILURE);
    }

    m_instance_path = cli.get<std::string>("instance");
    m_local_search = cli.present<std::string>("--local-search");
    m_iters = cli.get<size_t>("--iters");
    m_verbose = cli.get<bool>("--verbose");
    m_seed = cli.present<size_t>("--seed");
    m_jp = cli.get<double>("--jp");
    m_hvns_n_iter = cli.get<size_t>("--hvns-n-iter");
    m_hvns_time_limit_ms = cli.get<double>("--hvns-time-limit-ms");
    m_tpa_n_iter = cli.get<size_t>("--tpa-n-iter");
    m_tpa_final_temp = cli.get<double>("--tpa-final-temp");
    m_tpa_time_limit_ms = cli.get<double>("--tpa-time-limit-ms");
    m_svns_time_limit_ms = cli.present<double>("--svns-time-limit-ms");
}
