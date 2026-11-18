#ifndef STATS_H
#define STATS_H

#include <cstddef>

// Work counters read by the benchmark (speedups/Bench.cpp) to report how much
// of the neighbourhood each speed up actually skips. They are bumped once per
// evaluated position / per reinserted job - never inside the per-machine loop -
// so the cost they add to the timed code is negligible.
namespace stats {

struct Counters {
    // Early cut-off of NEH::insert_calculation (Taillard positions 1..n).
    size_t machines_scanned = 0; // machines actually evaluated before returning
    size_t machines_total = 0;   // m per evaluated position (what a scan without cut-off would do)

    // Grabowski critical-block restriction (NEH::taillard_grabowski_best_ins).
    size_t positions_evaluated = 0; // positions inside the allowed ranges
    size_t positions_total = 0;     // every insertion position (n per reinserted job)
};

inline Counters &get() {
    static Counters counters;
    return counters;
}

inline void reset() { get() = Counters{}; }

} // namespace stats

#endif
