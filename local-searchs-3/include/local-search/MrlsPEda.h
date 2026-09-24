#ifndef MRLS_P_EDA_H
#define MRLS_P_EDA_H

#include <vector>

#include "Instance.h"
#include "Solution.h"

// P_EDA's `mrls`, ported verbatim from P_EDA/src/P_EDA.cpp:37-83 (originally
// P_EDA::mrls, turned into a free function - no other member state was used).
//
// Same insertion move as `rls`, but the loop guard is `cnt <= n` (one extra
// iteration versus `rls`'s `cnt < n`), and whenever the cyclic index `j` wraps
// past `n` the reference vector is reshuffled in place via RNG before
// continuing - a periodic randomized re-ordering of the scan order not present
// in any other RLS variant. `ref` is mutated by this function.
bool mrls_p_eda(Solution &s, std::vector<size_t> &ref, Instance &instance);

#endif
