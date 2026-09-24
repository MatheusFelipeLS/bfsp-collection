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

    double jp() const { return m_jp; }                  // IG_IJ/include/Parameters.h:30 - only used by ig-ij-dispatch
    size_t hvns_n_iter() const { return m_hvns_n_iter; } // HVNS/include/Parameters.h:27
    size_t tpa_n_iter() const { return m_tpa_n_iter; }   // TPA/include/Parameters.h
    double tpa_final_temp() const { return m_tpa_final_temp; } // TPA/include/Parameters.h

    // IG_IJ host loop parameters (IG_IJ/include/Parameters.h) - fixed across
    // every entry, so the ONLY thing that varies between runs is which local
    // search occupies the "Local Search" slot at IG_IJ.cpp:126-131.
    size_t destroy_size() const { return m_destroy_size; } // -dS/--destroy in IG_IJ
    double temperature() const { return m_temperature; }   // -tP/--temperature in IG_IJ

    // Wall-clock budget for the WHOLE IG run (one repetition) - see
    // README.md "O que está sendo medido". Replaces IG_IJ's own
    // ro*n*m/-t sizing with one flat, entry-independent value, consistent
    // with local-searchs-fair's --time-limit-ms.
    double time_limit_ms() const { return m_time_limit_ms; }

    // Per-call budget for the local searches that are themselves
    // self-budgeted mini-metaheuristics (hvns-shaking, hvns-sa-*, tpa-sa) -
    // without this they would consume the ENTIRE time_limit_ms on their
    // first call, defeating the point of measuring convergence over many IG
    // calls. See README.md "Entradas auto-orçadas".
    double slot_time_limit_ms() const { return m_slot_time_limit_ms; }

    // Trajectory logging (one row per IG call, cost before/after the local
    // search, accepted?, current/best cost, elapsed time) - only emitted for
    // repetition 1 of each (entry, instance), capped at this many rows.
    size_t trajectory_max_rows() const { return m_trajectory_max_rows; }
    bool no_trajectory() const { return m_no_trajectory; }

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

    size_t m_destroy_size = 8;
    double m_temperature = 0.5;

    double m_time_limit_ms = 2000.0;
    double m_slot_time_limit_ms = 20.0;
    size_t m_trajectory_max_rows = 2000;
    bool m_no_trajectory = false;
};

#endif
