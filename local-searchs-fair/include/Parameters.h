#ifndef CLI_H
#define CLI_H

#include <cstddef>
#include <optional>
#include <string>

class Parameters {
  public:
    Parameters(int argc, char **argv);

    const std::string &instance_path() const { return m_instance_path; }
    // empty optional means "run every local search"
    std::optional<std::string> local_search() const { return m_local_search; }
    size_t iters() const { return m_iters; }
    bool verbose() const { return m_verbose; }
    std::optional<size_t> seed() const { return m_seed; }

    double jp() const { return m_jp; }                  // IG_IJ/include/Parameters.h:30
    size_t hvns_n_iter() const { return m_hvns_n_iter; } // HVNS/include/Parameters.h:27
    size_t tpa_n_iter() const { return m_tpa_n_iter; }   // TPA/include/Parameters.h
    double tpa_final_temp() const { return m_tpa_final_temp; } // TPA/include/Parameters.h
    // Single time budget shared by EVERY entry (see README.md "Comparação
    // justa") - replaces the old per-family --svns-time-limit-ms /
    // --hvns-time-limit-ms / --tpa-time-limit-ms flags from local-searchs/,
    // which gave different (and, for SVNS_D, instance-size-dependent) budgets
    // to different entries.
    double time_limit_ms() const { return m_time_limit_ms; }

  private:
    std::string m_instance_path;
    std::optional<std::string> m_local_search; // which local search to run; empty = all
    size_t m_iters = 5;                        // benchmark repetitions per (local search, instance)
    bool m_verbose = false;
    std::optional<size_t> m_seed;

    double m_jp = 0.001;
    size_t m_hvns_n_iter = 1800000;
    size_t m_tpa_n_iter = 1800000;
    double m_tpa_final_temp = 1.0;
    double m_time_limit_ms = 2000.0;
};

#endif
