#include "../common/helpers.h"
#include "../common/greedy_heuristics.h"
#include "../common/ls_prev_deltas.h"
#include <iostream>
#include <algorithm>
#include <numeric>

using namespace std;

// returns (best solution, number of neighborhood evaluations of each LS call)
tuple<vector<int>, vector<int>> msls_steepest_list_moves(vector<Point>& points, vector<vector<int>>& dist_matrix, int iterations=400) {
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
// not reachable by a single edge exchange - LS wont immediately undo it.
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
// 2. score every outside node by insertion cost (omit removed nodes), pick random one from top `pool` and insert it at its cheapest position.
// smallest change - less probable to be immediately undone by LS
vector<int> exchange_nodes(const vector<int>& solution, const vector<Point>& points,
                           const vector<vector<int>>& dist_matrix, int k = 5, int pool = 10)
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
        partial_sort(options.begin(), options.begin() + top, options.end()); // choose top 'pool' best exchanges
        auto [delta, u, pos] = options[uniform_int_distribution<int>(0, top - 1)(rng())]; // pick one random to insert
        sol.insert(sol.begin() + pos + 1, u);
        in_cycle[u] = 1;
    }
    return sol;
}


// 50/50 chance to apply intra- or inter-route perturbation
vector<int> perturb_solution(const vector<int>& solution, const vector<Point>& points,
                             const vector<vector<int>>& dist_matrix)
{
    if (uniform_int_distribution<int>(0, 1)(rng()) == 0)
        return double_bridge(solution);
    return exchange_nodes(solution, points, dist_matrix);
}

// returns (best solution, LS calls, number of neighborhood evaluations of each LS call)
tuple<vector<int>, int, vector<int>> ils_steepest_list_moves(vector<Point>& points, vector<vector<int>>& dist_matrix, int max_time_ms) {
    auto start_time = chrono::steady_clock::now();
    int ls_calls = 1;
    vector<int> evaluations;
    auto [best_solution, first_evals] = steepest_ls_with_prev_deltas(tsp_random_heuristic(points), dist_matrix, points);
    evaluations.push_back(first_evals);
    int best_cost = tsp_calculate_costs(best_solution, dist_matrix, points);
    while (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start_time).count() < max_time_ms) {
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }
    string filename = argv[1];
    vector<Point> points = load_points(filename);
    if (points.empty()) {
        cerr << "No points loaded" << endl;
        return 1;
    }
    vector<vector<int>> dist_matrix = calculate_distance_matrix(points);
    vector<int> costs_msls, costs_ils;
    vector<double> times_msls, times_ils;
    vector<int> best_solution_msls, best_solution_ils;
    vector<int> ls_calls_list;
    vector<int> evals_msls, evals_ils; // evaluations of every LS call across all runs
    int best_cost_msls = INT_INFINITY;
    int best_cost_ils = INT_INFINITY;
    for (int i = 0; i < 20; ++i) {
        auto t = chrono::steady_clock::now();
        auto [solution_msls, run_evals_msls] = msls_steepest_list_moves(points, dist_matrix);
        times_msls.push_back(elapsed_ms(t));
        evals_msls.insert(evals_msls.end(), run_evals_msls.begin(), run_evals_msls.end());

        auto t_ils = chrono::steady_clock::now();
        auto [solution_ils, ls_calls, run_evals_ils] = ils_steepest_list_moves(points, dist_matrix, 840); //t - avg time of MSLS in ms
        times_ils.push_back(elapsed_ms(t_ils));
        ls_calls_list.push_back(ls_calls);
        evals_ils.insert(evals_ils.end(), run_evals_ils.begin(), run_evals_ils.end());
        
        int cost = tsp_calculate_costs(solution_msls, dist_matrix, points);
        costs_msls.push_back(cost);
        if (cost < best_cost_msls) {
            best_cost_msls = cost;
            best_solution_msls = solution_msls;
        }

        int cost_ils = tsp_calculate_costs(solution_ils, dist_matrix, points);
        costs_ils.push_back(cost_ils);
        if (cost_ils < best_cost_ils) {
            best_cost_ils = cost_ils;
            best_solution_ils = solution_ils;
        }
    }
    print_stats("Multiple start local search", costs_msls, best_solution_msls, best_cost_msls, times_msls);
    print_stats("Iterated local search", costs_ils, best_solution_ils, best_cost_ils, times_ils);
    cout << "ILS local search calls (max, avg, min): " 
         << *max_element(ls_calls_list.begin(), ls_calls_list.end()) << ", "
         << accumulate(ls_calls_list.begin(), ls_calls_list.end(), 0) / static_cast<double>(ls_calls_list.size()) << ", "
         << *min_element(ls_calls_list.begin(), ls_calls_list.end()) << endl;
    cout << "Neighborhood evaluations per LS call in MSLS (max, avg, min): "
         << *max_element(evals_msls.begin(), evals_msls.end()) << ", "
         << accumulate(evals_msls.begin(), evals_msls.end(), 0.0) / evals_msls.size() << ", "
         << *min_element(evals_msls.begin(), evals_msls.end()) << endl;
    cout << "Neighborhood evaluations per LS call in ILS (max, avg, min): "
         << *max_element(evals_ils.begin(), evals_ils.end()) << ", "
         << accumulate(evals_ils.begin(), evals_ils.end(), 0.0) / evals_ils.size() << ", "
         << *min_element(evals_ils.begin(), evals_ils.end()) << endl;
    return 0;
}