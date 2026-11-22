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

  private:
    std::string m_instance_path;
    std::optional<std::string> m_algorithm;
    std::optional<std::string> m_speedup; // which speed up to benchmark; empty = all
    size_t m_iters = 5;                   // benchmark repetitions per (speed up, instance)
    bool m_verbose = false;
    std::optional<size_t> m_seed;
};

#endif
