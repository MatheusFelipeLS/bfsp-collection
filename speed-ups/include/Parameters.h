#ifndef CLI_H
#define CLI_H

#include <cstddef>
#include <optional>
#include <string>

class Parameters {
  public:
    Parameters(int argc, char **argv);

    const std::string &instance_path() const { return m_instance_path; }
    // empty optional means "run every algorithm"
    std::optional<std::string> algorithm() const { return m_algorithm; }
    // empty optional means "benchmark every speed up"
    std::optional<std::string> speedup() const { return m_speedup; }
    size_t iters() const { return m_iters; }
    bool verbose() const { return m_verbose; }
    std::optional<size_t> seed() const { return m_seed; }
    std::optional<size_t> lambda() const { return m_lambda; }
    double minmax_alpha() const { return m_minmax_alpha; }
    double mneh_alpha() const { return m_mneh_alpha; }
    double beta() const { return m_beta; }
    size_t delta() const { return m_delta; }

  private:
    std::string m_instance_path;
    std::optional<std::string> m_algorithm;
    std::optional<std::string> m_speedup; // which speed up to benchmark; empty = all
    size_t m_iters = 5;                   // benchmark repetitions per (speed up, instance)
    bool m_verbose = false;
    std::optional<size_t> m_seed;
    std::optional<size_t> m_lambda; // PF_NEH/PFT_NEH split point; defaults to (n > 200 ? 20 : n),
                                    // used identically in IG_IJ, IG_RIS, IG_VND2 and MA (LAMBDA_MAX=20,
                                    // https://doi.org/10.1109/TASE.2012.2219860)
    double m_minmax_alpha = 0.60;   // MinMax default (Ronconi, https://doi.org/10.1016/S0925-5273(03)00065-3),
                                    // used as-is by IG/src/IG.cpp; DE_ABC retunes it to 0.75 for its own metaheuristic
    double m_mneh_alpha = 0.8;      // mNEH default, only reference found is TPA/include/Parameters.h
    double m_beta = 5e-4;           // GRASP_NEH RCL threshold, only reference found is DE_PLS/include/Parameters.h
    size_t m_delta = 20;            // GRASP_NEH split point between the GRASP phase and the NEH phase, same source
};

#endif
