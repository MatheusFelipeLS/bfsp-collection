#include "local-search/EdgeInsertion.h"

#include <algorithm>
#include <limits>

#include "Core.h"
#include "Instance.h"
#include "Solution.h"

EdgeInsertion::EdgeInsertion(Instance &instance) : m_instance(instance) {
    const size_t n = m_instance.num_jobs();
    const size_t m = m_instance.num_machines();

    m_inner.departure_times = std::vector<std::vector<size_t>>(n, std::vector<size_t>(m, 0));
    m_inner.tail = std::vector<std::vector<size_t>>(n, std::vector<size_t>(m, 0));
    m_f = std::vector<std::vector<size_t>>(n, std::vector<size_t>(m, 0));
}

size_t EdgeInsertion::insert_calculation(const size_t i, const size_t k, const size_t best_value) {
    size_t max_value = 0;

    auto p = [this](size_t a, size_t b) { return m_instance.p(a, b); };

    auto &q = m_inner.tail;
    auto &e = m_inner.departure_times;

    auto set_f_and_max = [this, &max_value, &q](size_t a, size_t b, size_t value) {
        m_f[a][b] = value;
        max_value = std::max(value + q[a][b], max_value);
    };

    size_t value = std::max(e[i - 1][0] + p(k, 0), e[i - 1][1]);
    set_f_and_max(i, 0, value);

    if (max_value >= best_value) {
        return max_value;
    }

    for (size_t j = 1; j < m_instance.num_machines() - 1; j++) {
        value = std::max(m_f[i][j - 1] + p(k, j), e[i - 1][j + 1]);
        set_f_and_max(i, j, value);

        if (max_value >= best_value) {
            return max_value;
        }
    }

    value = m_f[i][m_instance.num_machines() - 2] + p(k, m_instance.num_machines() - 1);
    set_f_and_max(i, m_instance.num_machines() - 1, value);

    return max_value;
}

std::pair<size_t, size_t> EdgeInsertion::taillard_best_insertion(const std::vector<size_t> &s, size_t job,
                                                                 size_t original_position) {
    m_inner.sequence = s;

    core::calculate_departure_times(m_instance, m_inner);
    core::calculate_tail(m_instance, m_inner);

    auto &q = m_inner.tail;
    auto p = [this](size_t a, size_t b) { return m_instance.p(a, b); };

    size_t max_value = 0;
    auto set_f_and_max = [this, &max_value, &q](size_t a, size_t b, size_t value) {
        m_f[a][b] = value;
        max_value = std::max(value + q[a][b], max_value);
    };

    size_t best_index = 0;
    size_t best_value = 0;
    if (original_position != 0) {
        set_f_and_max(0, 0, p(job, 0));
        for (size_t j = 1; j < m_instance.num_machines(); j++) {
            set_f_and_max(0, j, m_f[0][j - 1] + p(job, j));
        }
        best_index = 0;
        best_value = max_value;
    } else {
        best_index = 1;
        best_value = std::numeric_limits<size_t>::max();
    }

    for (size_t i = 1; i <= s.size(); i++) {
        if (original_position == i) {
            continue;
        }

        max_value = insert_calculation(i, job, best_value);

        if (max_value < best_value) {
            best_value = max_value;
            best_index = i;
        }
    }

    return {best_index, best_value};
}

std::pair<size_t, size_t> EdgeInsertion::taillard_best_edge_insertion(const std::vector<size_t> &s,
                                                                      std::pair<size_t, size_t> &jobs,
                                                                      size_t original_position) {
    const size_t m = m_instance.num_machines();
    m_inner.sequence = s;
    core::calculate_departure_times(m_instance, m_inner);
    core::calculate_tail(m_instance, m_inner);

    auto &q = m_inner.tail;
    auto p = [this](size_t a, size_t b) { return m_instance.p(a, b); };

    // forward matrix already carrying the first job of the pair at every prefix
    m_f[0][0] = p(jobs.first, 0);
    for (size_t j = 1; j < m; j++) {
        m_f[0][j] = m_f[0][j - 1] + p(jobs.first, j);
    }

    for (size_t i = 1; i < m_inner.sequence.size(); i++) {
        m_f[i][0] = std::max(m_inner.departure_times[i - 1][0] + p(jobs.first, 0), m_inner.departure_times[i - 1][1]);
        size_t j = 0;
        for (j = 1; j < m_f[i].size() - 1; j++) {
            m_f[i][j] = std::max(m_f[i][j - 1] + p(jobs.first, j), m_inner.departure_times[i - 1][j + 1]);
        }
        m_f[i][j] = m_f[i][j - 1] + p(jobs.first, j);
    }

    size_t max_value = 0;
    auto set_f_and_max = [this, &q, &max_value](size_t a, size_t b, size_t value) {
        m_f[a][b] = value;
        max_value = std::max(value + q[a][b], max_value);
    };

    size_t best_index = 0;
    size_t best_value = 0;
    if (original_position != 0) {
        set_f_and_max(0, 0, std::max(m_f[0][0] + p(jobs.second, 0), m_f[0][1]));
        size_t j = 0;
        for (j = 1; j < m_instance.num_machines() - 1; j++) {
            set_f_and_max(0, j, std::max(m_f[0][j - 1] + p(jobs.second, j), m_f[0][j + 1]));
        }
        set_f_and_max(0, j, m_f[0][j - 1] + p(jobs.second, j));

        best_index = 0;
        best_value = max_value;
    } else {
        best_index = 1;
        best_value = std::numeric_limits<size_t>::max();
    }

    for (size_t i = 1; i < m_inner.sequence.size(); i++) {
        if (original_position == i) {
            continue;
        }

        max_value = 0;
        size_t value = std::max(m_f[i][0] + p(jobs.second, 0), m_f[i][1]);
        set_f_and_max(i, 0, value);

        for (size_t j = 1; j < m_instance.num_machines() - 1; j++) {
            value = std::max(m_f[i][j - 1] + p(jobs.second, j), m_f[i][j + 1]);
            set_f_and_max(i, j, value);
        }
        value = m_f[i][m_instance.num_machines() - 2] + p(jobs.second, m_instance.num_machines() - 1);
        set_f_and_max(i, m_instance.num_machines() - 1, value);

        if (max_value < best_value) {
            best_value = max_value;
            best_index = i;
        }
    }

    return {best_index, best_value};
}

bool EdgeInsertion::single_insertion_local_search(Solution &s) {
    core::recalculate_solution(m_instance, s);

    size_t best_job_index = 0;
    size_t best_obj = s.cost;
    size_t best_index = std::numeric_limits<size_t>::max();

    for (size_t i = 0; i < m_instance.num_jobs(); i++) {
        const size_t job = s.sequence[i];
        s.sequence.erase(s.sequence.begin() + (long)i);

        auto [index, obj] = taillard_best_insertion(s.sequence, job, std::numeric_limits<size_t>::max());

        s.sequence.insert(s.sequence.begin() + (long)i, job);

        if (obj < best_obj) {
            best_obj = obj;
            best_index = index;
            best_job_index = i;
        }
    }

    if (s.cost == best_obj) {
        return false;
    }

    if (best_job_index < best_index) {
        std::rotate(s.sequence.begin() + (long)best_job_index, s.sequence.begin() + (long)best_job_index + 1,
                    s.sequence.begin() + (long)best_index);
    } else {
        std::rotate(s.sequence.begin() + (long)best_index, s.sequence.begin() + (long)best_job_index,
                    s.sequence.begin() + (long)best_job_index + 1);
    }

    core::recalculate_solution(m_instance, s);
    return true;
}

bool EdgeInsertion::edge_insertion_local_search(Solution &s) {
    core::recalculate_solution(m_instance, s);

    size_t best_job = 0;
    size_t best_index = std::numeric_limits<size_t>::max();
    size_t best_obj = s.cost;

    for (size_t i = 0; i < m_instance.num_jobs() - 1; i++) {
        std::pair<size_t, size_t> jobs = {s.sequence[i], s.sequence[i + 1]};
        s.sequence.erase(s.sequence.begin() + (long)i, s.sequence.begin() + (long)i + 2);

        auto [index, obj] = taillard_best_edge_insertion(s.sequence, jobs, std::numeric_limits<size_t>::max());

        s.sequence.insert(s.sequence.begin() + (long)i, jobs.first);
        s.sequence.insert(s.sequence.begin() + (long)i + 1, jobs.second);

        if (obj < best_obj) {
            best_obj = obj;
            best_index = index;
            best_job = i;
        }
    }

    if (s.cost == best_obj) {
        return false;
    }

    if (best_job < best_index) {
        std::rotate(s.sequence.begin() + (long)best_job, s.sequence.begin() + (long)best_job + 2,
                    s.sequence.begin() + (long)best_index + 2);
    } else {
        std::rotate(s.sequence.begin() + (long)best_index, s.sequence.begin() + (long)best_job,
                    s.sequence.begin() + (long)best_job + 2);
    }

    core::recalculate_solution(m_instance, s);
    s.cost = best_obj;
    return true;
}
