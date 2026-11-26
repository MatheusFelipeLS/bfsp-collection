#include "local-search/HyprrLS.h"

#include <algorithm>

#include "Core.h"

// See include/local-search/HyprrLS.h. Original source references below are to
// hyprr/ at commit 0cec09d (HyPRR/LocalSearch.cpp, HyPRR/Individual.cpp).

HyprrLS::HyprrLS(Instance &instance, Options options)
    : m_instance(instance), m_opt(options), m_n(instance.num_jobs()), m_m(instance.num_machines()) {
    m_zero.assign(m_m, 0);
    m_rh.assign(m_n, std::vector<size_t>(m_m, 0));
    m_rt.assign(m_n + 1, std::vector<size_t>(m_m, 0));
    m_buf.assign(m_n + 2, std::vector<size_t>(m_m, 0));
    m_reduced.reserve(m_n);
    m_probe.sequence.reserve(m_n);
    m_jobs.resize(m_n);
}

// ---------------------------------------------------------------------------
// Row recurrences. A zero row stands for "no previous / no next job", which
// gives exactly the first-job (resp. last-job) formulas.
// ---------------------------------------------------------------------------
void HyprrLS::head_row(const std::vector<size_t> &prev, size_t job, std::vector<size_t> &out) const {
    const size_t m = m_m;
    out[0] = std::max(prev[0] + (size_t)m_instance.p(job, 0), prev[1]);
    for (size_t c = 1; c + 1 < m; c++) {
        out[c] = std::max(out[c - 1] + (size_t)m_instance.p(job, c), prev[c + 1]);
    }
    out[m - 1] = out[m - 2] + (size_t)m_instance.p(job, m - 1);
}

void HyprrLS::tail_row(const std::vector<size_t> &next, size_t job, std::vector<size_t> &out) const {
    const size_t m = m_m;
    out[m - 1] = std::max(next[m - 1] + (size_t)m_instance.p(job, m - 1), next[m - 2]);
    for (size_t c = m - 2; c >= 1; c--) {
        out[c] = std::max(out[c + 1] + (size_t)m_instance.p(job, c), next[c - 1]);
    }
    out[0] = out[1] + (size_t)m_instance.p(job, 0);
}

size_t HyprrLS::row_sum(const std::vector<size_t> &row) {
    size_t sum = 0;
    for (size_t v : row) {
        sum += v;
    }
    return sum;
}

// Departure times (partial, from `from`), tails, T and - when some pruning is
// on - the critical path. Plays the role of Individual::updateCost +
// Individual::caminhoCritico after every accepted move.
void HyprrLS::refresh(Solution &s, size_t from) {
    if (s.departure_times.size() != m_n) {
        s.departure_times.assign(m_n, std::vector<size_t>(m_m, 0));
        from = 0;
    }
    if (from == 0) {
        core::recalculate_solution(m_instance, s);
    } else {
        core::partial_recalculate_solution(m_instance, s, from);
    }

    if (s.tail.size() != m_n + 1) {
        s.tail.assign(m_n + 1, std::vector<size_t>(m_m, 0));
    }
    std::fill(s.tail[m_n].begin(), s.tail[m_n].end(), 0);
    for (size_t r = m_n; r-- > 0;) {
        tail_row(s.tail[r + 1], s.sequence[r], s.tail[r]);
    }

    m_t_cur = row_sum(s.departure_times[m_n - 1]);

    if (pruning()) {
        critical_path(s);
    }
}

// ---------------------------------------------------------------------------
// Individual::caminhoCritico, verbatim. E(i, j) is the original 1-based
// departure matrix (E[0][*] = 0, E[i][0] = E[i-1][1]). The path is walked back
// from (n, m) to (1, 1) and cut into blocks: T-sequences (type 0, vertical
// arcs), normal blocks (type 1, horizontal arcs) and anti-blocks (type -1,
// blocking arcs). m_blocks[0] is the block that ends at (n, m).
// ---------------------------------------------------------------------------
void HyprrLS::critical_path(const Solution &s) {
    const int n = (int)m_n;
    const int m = (int)m_m;

    auto E = [&](int i, int j) -> long {
        if (i <= 0) {
            return 0;
        }
        if (j == 0) {
            return i >= 2 ? (long)s.departure_times[i - 2][0] : 0;
        }
        return (long)s.departure_times[i - 1][j - 1];
    };
    // jobsToolsMatrix[chromosome[i-1]][j-1]
    auto P = [&](int i, int j) -> long { return m_instance.p(s.sequence[i - 1], j - 1); };

    m_blocks.clear();

    int j = m;
    int i = n;
    int cont = 0;
    int sub = 0;
    int last_machine = j;
    int last_job = i;

    Block block{};
    block.index = cont;
    block.ji = n - 1;
    block.jf = n - 1;
    block.em = m - 1;
    block.type = sub;
    block.joined = false;

    while (i != 1 || j != 1) {
        if (i != 1 && j != m && E(i, j) == E(i - 1, j + 1)) {
            i = i - 1;
            j = j + 1;

            if (sub != -1) {
                block.sm = last_machine - 1;
                block.ji = last_job - 1;
                if (sub == 1) {
                    block.joined = true;
                    m_blocks.push_back(block);
                    m_jobs[i] = {2, cont + 1, cont};
                } else {
                    block.joined = false;
                    m_blocks.push_back(block);
                    m_jobs[i] = {3, cont, cont};
                }
                ++cont;
                ++block.index;
                block.em = last_machine - 1;
                block.jf = last_job - 1;
                block.type = -1;
                sub = -1;
            } else {
                m_jobs[i] = {1, cont, cont};
            }

        } else if (i != 1 && E(i, j) == E(i - 1, j) + P(i, j)) {
            i = i - 1;

            if (sub != 1) {
                block.sm = last_machine - 1;
                block.ji = last_job - 1;
                if (sub == -1) {
                    block.joined = true;
                    m_blocks.push_back(block);
                    m_jobs[i] = {2, cont + 1, cont};
                } else {
                    block.joined = false;
                    m_blocks.push_back(block);
                    m_jobs[i] = {3, cont, cont};
                }
                ++cont;
                ++block.index;
                block.em = last_machine - 1;
                block.jf = last_job - 1;
                block.type = 1;
                sub = 1;
            } else {
                m_jobs[i] = {1, cont, cont};
            }

        } else {
            j = j - 1;

            if (sub != 0) {
                block.sm = last_machine - 1;
                block.ji = last_job - 1;
                block.joined = false;
                ++cont;
                m_blocks.push_back(block);
                ++block.index;
                block.em = last_machine - 1;
                block.jf = last_job - 1;
                block.type = 0;
                sub = 0;
            }
        }

        last_machine = j;
        last_job = i;
    }

    block.sm = last_machine - 1;
    block.ji = last_job - 1;
    if (block.type == 0) {
        m_jobs[i - 1] = {2, block.index, block.index};
    } else {
        m_jobs[i - 1] = {1, block.index, block.index};
    }
    m_blocks.push_back(block);
}

// ---------------------------------------------------------------------------
// LocalSearch::getInsertionLowerBound(k, i), verbatim (Theorems 2-4 of Ding et
// al., 2016). Returns delta; the move is skipped when delta >= 0.
// The original falls off the end of the function (undefined behaviour) when
// the job's block is a T-sequence and the insertion block is also one, and when
// the job is in a normal block and the insertion block is a T-sequence; it also
// reads blocos[-1] / blocos[b+1] out of range in corner cases. All of those
// return -1 here (no bound: the move is evaluated).
// ---------------------------------------------------------------------------
int HyprrLS::insertion_lb(const Solution &s, int k, int i) const {
    const int n = (int)m_n;
    const int nb = (int)m_blocks.size();
    auto p = [&](size_t job, int machine) -> int { return (int)m_instance.p(job, (size_t)machine); };
    auto job = [&](int pos) -> size_t { return s.sequence[(size_t)pos]; };

    const bool edge = (i == 0 || i == n - 1);

    int b_ins;
    if (m_jobs[i].n_blocks == 1 || edge) {
        b_ins = m_jobs[i].i1;
    } else if (m_jobs[i].n_blocks == 2) {
        b_ins = i < k ? m_jobs[i].i1 + 1 : m_jobs[i].i1;
    } else {
        b_ins = i < k ? m_jobs[i].i1 + 1 : m_jobs[i].i1 - 1;
    }
    if (b_ins < 0 || b_ins >= nb) {
        return -1;
    }

    const Block &ins = m_blocks[b_ins];
    const int kb = m_jobs[k].b;
    const Block &own = m_blocks[kb];
    int pi_zero = (int)job(ins.jf);
    int delta = 0;

    // Theorem 4
    if (own.type == 0) {
        for (int l = own.sm; l <= own.em; l++) {
            delta -= p(job(k), l);
        }
        if (k != 0) {
            for (int l = own.sm + 1; l <= own.em; l++) {
                delta += p(job(k - 1), l);
            }
        } else {
            for (int l = own.em - 1; l >= own.sm; l--) {
                delta += p(job(k + 1), l);
            }
        }

        if (ins.type == 1 || edge) {
            delta += (i != n - 1) ? p(job(k), ins.sm) : p(job(k), ins.em);
            return delta;
        } else if (ins.type == -1) {
            delta += p((size_t)pi_zero, ins.em);
            return delta;
        }
    }

    // Theorem 2
    if (own.type == 1) {
        if (ins.type == 1 || edge) {
            const int a = (i != n - 1) ? p(job(k), ins.sm) : p(job(k), ins.em);
            return a - p(job(k), own.sm);
        } else if (ins.type == -1) {
            return p((size_t)pi_zero, ins.em) - p(job(k), own.sm);
        }
    }

    // Theorem 3
    if (own.type == -1) {
        if (!own.joined && kb + 1 >= nb) {
            return -1;
        }
        const int sub = own.joined ? p(job(own.ji), own.sm) : p(job(m_blocks[kb + 1].ji), m_blocks[kb + 1].em);

        if (ins.type == -1) {
            pi_zero = (int)job(ins.jf);
            return p((size_t)pi_zero, ins.em) - sub;
        } else if (ins.type == 1 || edge) {
            const int a = (i != n - 1) ? p(job(k), ins.sm) : p(job(k), ins.em);
            return a - sub;
        }
    }

    return -1;
}

// ---------------------------------------------------------------------------
// LocalSearch::getSwapLowerBound(k, i), k < i, verbatim. Commented out at its
// only call site in the original (LocalSearch::swap). Note it does not match
// eqs. (8)-(10) of the paper term by term (e.g. T/T only subtracts p(pi_i)).
// ---------------------------------------------------------------------------
int HyprrLS::swap_lb(const Solution &s, int k, int i) const {
    auto p = [&](size_t job, int machine) -> int { return (int)m_instance.p(job, (size_t)machine); };
    const size_t jk = s.sequence[(size_t)k];
    const size_t ji = s.sequence[(size_t)i];
    const Block &bk = m_blocks[m_jobs[k].b];
    const Block &bi = m_blocks[m_jobs[i].b];
    int delta = 0;

    if (bk.type == 0) {
        for (int l = bk.sm; l <= bk.em; l++) {
            delta += p(ji, l) - p(jk, l);
        }
        if (bi.type == 1) {
            delta += p(jk, bi.sm) - p(ji, bi.sm);
        } else if (bi.type == 0) {
            for (int l = bi.sm; l <= bi.em; l++) {
                delta -= p(ji, l);
            }
        }
        return delta;
    }

    if (bk.type == 1) {
        if (bi.type == 1) {
            delta += p(ji, bk.sm) + p(jk, bi.sm) - p(jk, bk.sm) - p(ji, bi.sm);
        } else if (bi.type == -1) {
            delta += p(ji, bk.sm) - p(jk, bk.sm);
        } else {
            for (int l = bi.sm; l <= bi.em; l++) {
                delta += p(jk, l) - p(ji, l);
            }
            delta += p(ji, bk.sm) - p(jk, bk.sm);
        }
        return delta;
    }

    if (bk.type == -1) {
        if (bi.type == 1) {
            delta += p(jk, bi.sm) - p(ji, bi.sm);
        } else if (bi.type == 0) {
            for (int l = bi.sm; l <= bi.em; l++) {
                delta += p(jk, l) - p(ji, l);
            }
        }
        return delta;
    }

    return -1;
}

// ---------------------------------------------------------------------------
// Generalised Taillard evaluation (LocalSearch::preInsertion + the scoring
// loops of relocate/twoOpt/swap).
// ---------------------------------------------------------------------------

// Heads and tails of the sequence with the block [k, k+x) removed.
void HyprrLS::reduced_matrices(const Solution &s, size_t k, size_t x) {
    const size_t len = m_n - x;
    m_reduced.clear();
    m_reduced.insert(m_reduced.end(), s.sequence.begin(), s.sequence.begin() + (long)k);
    m_reduced.insert(m_reduced.end(), s.sequence.begin() + (long)(k + x), s.sequence.end());

    for (size_t r = 0; r < k; r++) {
        m_rh[r] = s.departure_times[r];
    }
    for (size_t r = k; r < len; r++) {
        head_row(r == 0 ? m_zero : m_rh[r - 1], m_reduced[r], m_rh[r]);
    }

    m_rt[len] = m_zero;
    for (size_t r = k; r < len; r++) {
        m_rt[r] = s.tail[r + x];
    }
    for (size_t r = k; r-- > 0;) {
        tail_row(m_rt[r + 1], m_reduced[r], m_rt[r]);
    }
}

// Makespan of inserting the block s[k, k+x) at index `pos` of the reduced
// sequence. Exact when <= bound; otherwise some value > bound (cut-off).
size_t HyprrLS::eval_block(const Solution &s, size_t k, size_t x, size_t pos, size_t bound) {
    const std::vector<size_t> *prev = pos == 0 ? &m_zero : &m_rh[pos - 1];
    for (size_t t = 0; t < x; t++) {
        head_row(*prev, s.sequence[k + t], m_buf[t]);
        prev = &m_buf[t];
    }
    const std::vector<size_t> &last = m_buf[x - 1];
    const std::vector<size_t> &tail = m_rt[pos];
    size_t value = 0;
    for (size_t c = 0; c < m_m; c++) {
        value = std::max(value, last[c] + tail[c]);
        if (value > bound) {
            break;
        }
    }
    return value;
}

// Sum of the last departure row of the candidate scored by the last
// eval_block call (block rows still in m_buf).
size_t HyprrLS::last_row_sum_block(size_t x, size_t pos) {
    const size_t len = m_n - x;
    std::vector<size_t> *prev = &m_buf[x - 1];
    size_t slot = x;
    for (size_t r = pos; r < len; r++) {
        std::vector<size_t> &out = m_buf[slot];
        head_row(*prev, m_reduced[r], out);
        prev = &out;
        slot = slot == x ? x + 1 : x;
    }
    return row_sum(*prev);
}

// Makespan of swapping positions i < j: rows i..j recomputed from the head of
// i-1, joined to the untouched tail of j+1.
size_t HyprrLS::eval_swap(const Solution &s, size_t i, size_t j, size_t bound) {
    const std::vector<size_t> *prev = i == 0 ? &m_zero : &s.departure_times[i - 1];
    for (size_t r = i; r <= j; r++) {
        const size_t job = r == i ? s.sequence[j] : (r == j ? s.sequence[i] : s.sequence[r]);
        head_row(*prev, job, m_buf[r - i]);
        prev = &m_buf[r - i];
    }
    const std::vector<size_t> &last = m_buf[j - i];
    if (last[m_m - 1] > bound) {
        return last[m_m - 1];
    }
    const std::vector<size_t> &tail = s.tail[j + 1];
    size_t value = 0;
    for (size_t c = 0; c < m_m; c++) {
        value = std::max(value, last[c] + tail[c]);
        if (value > bound) {
            break;
        }
    }
    return value;
}

size_t HyprrLS::last_row_sum_swap(const Solution &s, size_t i, size_t j) {
    const size_t base = j - i;
    std::vector<size_t> *prev = &m_buf[base];
    size_t slot = base + 1;
    for (size_t r = j + 1; r < m_n; r++) {
        std::vector<size_t> &out = m_buf[slot];
        head_row(*prev, s.sequence[r], out);
        prev = &out;
        slot = slot == base + 1 ? base + 2 : base + 1;
    }
    return row_sum(*prev);
}

// ---------------------------------------------------------------------------
// Naive evaluation: rebuild the candidate and recompute it from scratch.
// ---------------------------------------------------------------------------
void HyprrLS::build_block_move(const Solution &s, size_t k, size_t x, size_t pos) {
    auto &q = m_probe.sequence;
    q.clear();
    // reduced sequence with the block inserted at index pos
    size_t taken = 0; // jobs of the reduced sequence already copied
    auto push_reduced = [&](size_t upto) {
        while (taken < upto) {
            const size_t src = taken < k ? taken : taken + x;
            q.push_back(s.sequence[src]);
            taken++;
        }
    };
    push_reduced(pos);
    for (size_t t = 0; t < x; t++) {
        q.push_back(s.sequence[k + t]);
    }
    push_reduced(m_n - x);
}

void HyprrLS::build_swap_move(const Solution &s, size_t i, size_t j) {
    m_probe.sequence = s.sequence;
    std::swap(m_probe.sequence[i], m_probe.sequence[j]);
}

size_t HyprrLS::probe_cost() {
    core::recalculate_solution(m_instance, m_probe);
    return m_probe.cost;
}

// ---------------------------------------------------------------------------
// Neighbourhoods
// ---------------------------------------------------------------------------

// relocate (x = 1) and twoOpt(x) (x = 2..4). For each block start k the
// candidate final positions of the block are [0, k-2] U [k+1, n-x], scanned in
// increasing order (the original `!(k-1 <= i && i < k+x)` filter).
bool HyprrLS::block_sweep(Solution &s, size_t x) {
    refresh(s, 0);
    if (x == 0 || x >= m_n) {
        return false;
    }
    bool improved = false;
    const bool prune = x == 1 && (m_opt.block_skip || m_opt.insertion_lb);

    for (size_t k = 0; k + x <= m_n; k++) {
        if (m_opt.taillard) {
            reduced_matrices(s, k, x);
        }

        for (size_t pos = 0; pos + x <= m_n; pos++) {
            if (pos + 1 == k || pos == k) {
                continue;
            }

            if (prune) {
                m_work.candidates++;
                const bool pruned = (m_opt.block_skip && m_jobs[k].b == m_jobs[pos].b) ||
                                    (m_opt.insertion_lb && insertion_lb(s, (int)k, (int)pos) >= 0);
                if (pruned) {
                    if (m_work.audit) {
                        build_block_move(s, k, x, pos);
                        if (probe_cost() < s.cost) {
                            m_work.pruned_improving++;
                        }
                    }
                    continue;
                }
                m_work.evaluated++;
            }

            if (m_opt.taillard) {
                const size_t cost = eval_block(s, k, x, pos, s.cost);
                if (cost > s.cost) {
                    continue;
                }
                if (cost == s.cost && last_row_sum_block(x, pos) >= m_t_cur) {
                    continue;
                }
                build_block_move(s, k, x, pos);
            } else {
                build_block_move(s, k, x, pos);
                const size_t cost = probe_cost();
                if (cost > s.cost) {
                    continue;
                }
                if (cost == s.cost && row_sum(m_probe.departure_times[m_n - 1]) >= m_t_cur) {
                    continue;
                }
            }

            improved = true;
            s.sequence.swap(m_probe.sequence);
            refresh(s, std::min(k, pos));
            if (m_opt.taillard) {
                reduced_matrices(s, k, x);
            }
        }
    }
    return improved;
}

bool HyprrLS::insertion(Solution &s) { return block_sweep(s, 1); }

bool HyprrLS::block_insertion(Solution &s, size_t x) { return block_sweep(s, x); }

bool HyprrLS::swap(Solution &s) {
    refresh(s, 0);
    bool improved = false;
    const bool prune = m_opt.block_skip || m_opt.swap_lb;

    for (size_t i = 0; i < m_n; i++) {
        for (size_t j = i + 1; j < m_n; j++) {
            if (prune) {
                m_work.candidates++;
                const bool pruned = (m_opt.block_skip && m_jobs[i].b == m_jobs[j].b) ||
                                    (m_opt.swap_lb && swap_lb(s, (int)i, (int)j) >= 0);
                if (pruned) {
                    if (m_work.audit) {
                        build_swap_move(s, i, j);
                        if (probe_cost() < s.cost) {
                            m_work.pruned_improving++;
                        }
                    }
                    continue;
                }
                m_work.evaluated++;
            }

            if (m_opt.taillard) {
                const size_t cost = eval_swap(s, i, j, s.cost);
                if (cost > s.cost) {
                    continue;
                }
                if (cost == s.cost && last_row_sum_swap(s, i, j) >= m_t_cur) {
                    continue;
                }
            } else {
                build_swap_move(s, i, j);
                const size_t cost = probe_cost();
                if (cost > s.cost) {
                    continue;
                }
                if (cost == s.cost && row_sum(m_probe.departure_times[m_n - 1]) >= m_t_cur) {
                    continue;
                }
            }

            improved = true;
            std::swap(s.sequence[i], s.sequence[j]);
            refresh(s, i);
        }
    }
    return improved;
}

bool HyprrLS::neighborhood(Solution &s, size_t i) {
    if (i == 1) {
        return insertion(s);
    }
    if (i <= 4) {
        return block_insertion(s, i);
    }
    return swap(s);
}

void HyprrLS::vnd(Solution &s) {
    while (true) {
        bool found = false;
        for (size_t i = 1; i <= n_viz() && !found; i++) {
            found = neighborhood(s, i);
        }
        if (!found) {
            break;
        }
    }
}
