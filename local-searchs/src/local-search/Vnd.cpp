#include "local-search/Vnd.h"

#include "RNG.h"
#include "local-search/BestSwapIg.h"
#include "local-search/RLS.h"

void vnd1(Solution &incumbent, std::vector<size_t> &reference, Instance &instance) {
    size_t k_max = 2;
    size_t k = 1;
    Solution temporal = incumbent;

    while (k <= k_max) {
        if (k == 1) {
            rls(incumbent, reference, instance);
        } else if (k == 2) {
            best_swap_ig(incumbent, instance);
        }

        if (incumbent.cost < temporal.cost) {
            temporal = incumbent;
            k = 1;
        } else {
            k += 1;
        }
    }

    incumbent = temporal;
}

void vnd2(Solution &incumbent, std::vector<size_t> &reference, Instance &instance) {
    size_t k_max = 2;
    size_t k = 1;
    Solution temporal = incumbent;

    while (k <= k_max) {
        if (k == 1) {
            best_swap_ig(incumbent, instance);
        } else if (k == 2) {
            rls(incumbent, reference, instance);
        }

        if (incumbent.cost < temporal.cost) {
            temporal = incumbent;
            k = 1;
        } else {
            k += 1;
        }
    }

    incumbent = temporal;
}

void vnd1_fixed(Solution &incumbent, std::vector<size_t> &reference, Instance &instance) {
    size_t k_max = 2;
    size_t k = 1;
    Solution temporal = incumbent;

    while (k <= k_max) {
        if (k == 1) {
            rls(incumbent, reference, instance);
        } else if (k == 2) {
            best_swap_ig_fixed(incumbent, instance);
        }

        if (incumbent.cost < temporal.cost) {
            temporal = incumbent;
            k = 1;
        } else {
            k += 1;
        }
    }

    incumbent = temporal;
}

void vnd2_fixed(Solution &incumbent, std::vector<size_t> &reference, Instance &instance) {
    size_t k_max = 2;
    size_t k = 1;
    Solution temporal = incumbent;

    while (k <= k_max) {
        if (k == 1) {
            best_swap_ig_fixed(incumbent, instance);
        } else if (k == 2) {
            rls(incumbent, reference, instance);
        }

        if (incumbent.cost < temporal.cost) {
            temporal = incumbent;
            k = 1;
        } else {
            k += 1;
        }
    }

    incumbent = temporal;
}

bool ig_ij_dispatch(Solution &incumbent, std::vector<size_t> &reference, Instance &instance, double jp) {
    if (RNG::instance().generate_real_number(0, 1) < jp) {
        best_swap_ig(incumbent, instance);
        return true;
    }

    rls(incumbent, reference, instance);
    return false;
}
