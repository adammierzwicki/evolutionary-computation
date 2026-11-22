#include "ils.h"
#include "greedy_heuristics.h"
#include "ls_prev_deltas.h"
#include <algorithm>
#include <chrono>
#include <random>

using namespace std;

tuple<vector<int>, vector<int>> msls_steepest_list_moves(vector<Point>& points, vector<vector<int>>& dist_matrix, int iterations) {
    vector<int> best_solution;
    vector<int> evaluations;
    int best_cost = INT_INFINITY;
    for (int i = 0; i < iterations; ++i) {
        vector<int> init_solution = tsp_random_heuristic(points);
        auto [solution, evals] = steepest_ls_with_prev_deltas(init_solution, dist_matrix, points);
        evaluations.push_back(evals);
        int cost = tsp_calculate_costs(solution, dist_matrix, points);
        if (cost < best_cost) {
            best_cost = cost;
            best_solution = solution;
        }
    }
    return {best_solution, evaluations};
}

// intra-route perturbation: double bridge exchange
// cycle A|B|C|D -> A|C|B|D (3 random cut points).
// not reachable by a single edge exchange - LS won't immediately undo it.
vector<int> double_bridge(const vector<int>& solution)
{
    int n = solution.size();
    if (n < 8) return solution;

    uniform_int_distribution<int> dist(1, n - 1);
    int p1, p2, p3;
    do {
        p1 = dist(rng()); p2 = dist(rng()); p3 = dist(rng());
    } while (p1 == p2 || p2 == p3 || p1 == p3);
    if (p1 > p2) swap(p1, p2);
    if (p2 > p3) swap(p2, p3);
    if (p1 > p2) swap(p1, p2);

    vector<int> result;
    result.reserve(n);
    result.insert(result.end(), solution.begin(), solution.begin() + p1);          // A
    result.insert(result.end(), solution.begin() + p2, solution.begin() + p3);     // C
    result.insert(result.end(), solution.begin() + p1, solution.begin() + p2);     // B
    result.insert(result.end(), solution.begin() + p3, solution.end());            // D
    return result;
}

// inter-route perturbation
// 1. remove k random nodes from the cycle
// 2. score every outside node by insertion cost (omit removed nodes), pick a random one
//    from the top `pool` and insert it at its cheapest position.
// smallest change - less probable to be immediately undone by LS
vector<int> exchange_nodes(const vector<int>& solution, const vector<Point>& points,
                           const vector<vector<int>>& dist_matrix, int k, int pool)
{
    vector<int> sol = solution;
    vector<char> in_cycle(points.size(), 0), removed(points.size(), 0);
    for (int v : sol) in_cycle[v] = 1;

    k = min(k, (int)sol.size() - 3);
    for (int i = 0; i < k; ++i) {
        int idx = uniform_int_distribution<int>(0, sol.size() - 1)(rng());
        in_cycle[sol[idx]] = 0;
        removed[sol[idx]] = 1;
        sol.erase(sol.begin() + idx);
    }

    for (int i = 0; i < k; ++i) {
        vector<tuple<int, int, int>> options; // (insertion delta, node, insert after idx)
        int sol_size = sol.size();
        for (int u = 0; u < (int)points.size(); ++u) {
            if (in_cycle[u] || removed[u]) continue; // skip just removed nodes
            int best = INT_INFINITY, best_pos = 0;
            for (int j = 0; j < sol_size; ++j) {
                int a = sol[j], b = sol[(j + 1) % sol_size];
                int d = dist_matrix[a][u] + dist_matrix[u][b] - dist_matrix[a][b];
                if (d < best) { best = d; best_pos = j; }
            }
            options.emplace_back(best + points[u].cost, u, best_pos);
        }

        int top = min(pool, (int)options.size());
        partial_sort(options.begin(), options.begin() + top, options.end()); // top 'pool' best exchanges
        auto [delta, u, pos] = options[uniform_int_distribution<int>(0, top - 1)(rng())]; // pick one at random
        sol.insert(sol.begin() + pos + 1, u);
        in_cycle[u] = 1;
    }
    return sol;
}

vector<int> perturb_solution(const vector<int>& solution, const vector<Point>& points,
                             const vector<vector<int>>& dist_matrix)
{
    if (uniform_int_distribution<int>(0, 1)(rng()) == 0)
        return double_bridge(solution);
    return exchange_nodes(solution, points, dist_matrix);
}

tuple<vector<int>, int, vector<int>> ils_steepest_list_moves(vector<Point>& points, vector<vector<int>>& dist_matrix, double max_time_ms) {
    auto start_time = chrono::steady_clock::now();
    int ls_calls = 1;
    vector<int> evaluations;
    auto [best_solution, first_evals] = steepest_ls_with_prev_deltas(tsp_random_heuristic(points), dist_matrix, points);
    evaluations.push_back(first_evals);
    int best_cost = tsp_calculate_costs(best_solution, dist_matrix, points);
    while (chrono::duration<double, milli>(chrono::steady_clock::now() - start_time).count() < max_time_ms) {
        auto [solution, evals] = steepest_ls_with_prev_deltas(perturb_solution(best_solution, points, dist_matrix), dist_matrix, points);
        evaluations.push_back(evals);
        ls_calls++;
        int cost = tsp_calculate_costs(solution, dist_matrix, points);
        if (cost < best_cost) {
            best_cost = cost;
            best_solution = solution;
        }
    }
    return {best_solution, ls_calls, evaluations};
}