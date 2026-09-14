#ifndef BENCH_H
#define BENCH_H

#include <optional>
#include <string>
#include <vector>

#include "Instance.h"
#include "Parameters.h"

namespace bench {

struct Result {
    std::string speedup;
    size_t n = 0;
    size_t m = 0;
    size_t iters = 0;
    double naive_ms = 0.0; // time of the un-accelerated baseline
    double accel_ms = 0.0; // time of the accelerated component
    double factor = 0.0;   // naive_ms / accel_ms
    bool correct = false;  // accelerated result matches the baseline
    std::string note;      // extra context (final costs, move counts, ...)
};

// Runs one speed up (by name) or all of them when `which` is empty.
std::vector<Result> run(const std::optional<std::string> &which, Instance &instance, const Parameters &params);

// CSV: speedup,n,m,iters,naive_ms,accel_ms,factor,correct,note
std::string header();
std::string format(const Result &r);

} // namespace bench

#endif
