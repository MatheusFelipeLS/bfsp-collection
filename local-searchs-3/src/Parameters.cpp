#include "Parameters.h"

#include "argparse/argparse.hpp"

#include <iostream>

namespace {
void config_argparse(argparse::ArgumentParser &cli) {
    cli.add_argument("instance").help("instance path");

    cli.add_argument("-p", "--local-search")
        .help("local search to plug into the IG_IJ loop (see README.md for the full catalog; omit to run all of "
              "them)")
        .metavar("LOCAL_SEARCH");

    cli.add_argument("-n", "--iters")
        .help("benchmark repetitions per (local search, instance)")
        .metavar("ITERS")
        .default_value(size_t(5))
        .scan<'i', size_t>();

    cli.add_argument("-s", "--seed")
        .help("random number generator seed; also fixes the shared PF_NEH starting solution")
        .metavar("SEED")
        .scan<'i', size_t>();

    cli.add_argument("-v", "--verbose")
        .help("print extra progress information")
        .metavar("VERBOSE")
        .default_value(false)
        .flag();

    cli.add_argument("--jp")
        .help("jumping probability used ONLY by the ig-ij-dispatch entry (IG_IJ/include/Parameters.h) - irrelevant "
              "to every other entry")
        .metavar("JP")
        .default_value(0.001)
        .scan<'f', double>();

    cli.add_argument("--hvns-n-iter")
        .help("n_iter used to shape hvns-sa-rls/hvns-sa-edge-insertion's cooling rate (HVNS/include/Parameters.h)")
        .metavar("N_ITER")
        .default_value(size_t(1800000))
        .scan<'i', size_t>();

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

    cli.add_argument("-dS", "--destroy")
        .help("IG_IJ destruction size - number of jobs removed/reinserted each iteration (IG_IJ/include/Parameters.h)")
        .metavar("D")
        .default_value(size_t(8))
        .scan<'i', size_t>();

    cli.add_argument("-tP", "--temperature")
        .help("IG_IJ acceptance-criterion temperature factor (IG_IJ/include/Parameters.h)")
        .metavar("TEMP")
        .default_value(0.5)
        .scan<'f', double>();

    cli.add_argument("--time-limit-ms")
        .help("wall-clock budget for the WHOLE IG run (one repetition), shared by every entry - see README.md "
              "\"O que está sendo medido\"")
        .metavar("MS")
        .default_value(2000.0)
        .scan<'f', double>();

    cli.add_argument("--slot-time-limit-ms")
        .help("per-call budget for the self-budgeted entries (hvns-shaking, hvns-sa-rls, hvns-sa-edge-insertion, "
              "tpa-sa) - see README.md \"Entradas auto-orçadas\"")
        .metavar("MS")
        .default_value(20.0)
        .scan<'f', double>();

    cli.add_argument("--trajectory-max-rows")
        .help("cap on how many per-call trajectory rows to emit (repetition 1 only) - the run itself is NOT "
              "truncated, only what gets logged")
        .metavar("N")
        .default_value(size_t(2000))
        .scan<'i', size_t>();

    cli.add_argument("--no-trajectory")
        .help("skip trajectory logging entirely (faster, less stdout)")
        .default_value(false)
        .flag();
}
} // namespace

Parameters::Parameters(int argc, char **argv) {

    argparse::ArgumentParser cli("local-searchs-3", "1.0", argparse::default_arguments::help);

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
    m_tpa_n_iter = cli.get<size_t>("--tpa-n-iter");
    m_tpa_final_temp = cli.get<double>("--tpa-final-temp");
    m_destroy_size = cli.get<size_t>("--destroy");
    m_temperature = cli.get<double>("--temperature");
    m_time_limit_ms = cli.get<double>("--time-limit-ms");
    m_slot_time_limit_ms = cli.get<double>("--slot-time-limit-ms");
    m_trajectory_max_rows = cli.get<size_t>("--trajectory-max-rows");
    m_no_trajectory = cli.get<bool>("--no-trajectory");
}
