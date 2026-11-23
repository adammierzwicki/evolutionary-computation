#include "../common/helpers.h"
#include "../common/greedy_heuristics.h"
#include "../common/ls_prev_deltas.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <random>
#include <functional>
#include <chrono>

using namespace std;

enum class Destroy { RandomNodes, SingleSubpath, MultiSubpath, WeightedNodes };

static const double DESTROY_FRACTION = 0.30;
static const double W_REGRET = 0.5;
static const double W_OBJ = 0.5;

static void remove_subpath(vector<int>& v, int start, int len) {
    rotate(v.begin(), v.begin() + start, v.end());
    v.erase(v.begin(), v.begin() + len);
}

vector<int> destroy_random_nodes(const vector<int>& sol, double frac = DESTROY_FRACTION) {
    int n = sol.size();
    int m = max(1, (int)round(frac * n));
    vector<int> idx(n);
    iota(idx.begin(), idx.end(), 0);
    shuffle(idx.begin(), idx.end(), rng());
    vector<char> removed(n, 0);
    for (int i = 0; i < m; ++i) removed[idx[i]] = 1;
    vector<int> result;
    result.reserve(n - m);
    for (int i = 0; i < n; ++i)
        if (!removed[i]) result.push_back(sol[i]);
    return result;
}

vector<int> destroy_subpaths(const vector<int>& sol, int num_subpaths, double frac = DESTROY_FRACTION) {
    int n = sol.size();
    int m = max(num_subpaths, (int)round(frac * n));
    num_subpaths = min(num_subpaths, m);

    // randomly generating lenghts of each subpath to remove, sum(lenghts) = m
    vector<int> cuts;
    for (int i = 1; i < m; ++i) cuts.push_back(i);
    shuffle(cuts.begin(), cuts.end(), rng());
    cuts.resize(num_subpaths - 1);
    cuts.push_back(0);
    cuts.push_back(m);
    sort(cuts.begin(), cuts.end());

    vector<int> result = sol;
    for (int s = 0; s < num_subpaths; ++s) {
        int len = cuts[s + 1] - cuts[s];
        int start = uniform_int_distribution<int>(0, (int)result.size() - 1)(rng());
        remove_subpath(result, start, len);
    }
    return result;
}

vector<int> destroy_single_subpath(const vector<int>& sol) {
    return destroy_subpaths(sol, 1);
}

vector<int> destroy_multi_subpath(const vector<int>& sol) {
    int k = uniform_int_distribution<int>(3, 5)(rng());
    return destroy_subpaths(sol, k);
}

vector<int> destroy_weighted_nodes(const vector<int>& sol, const vector<Point>& points, const vector<vector<int>>& dist_matrix, double frac = DESTROY_FRACTION) {
    int n = sol.size();
    int m = max(1, (int)round(frac * n));
    vector<double> w(n);
    for (int i = 0; i < n; ++i) {
        int prev = sol[(i + n - 1) % n], v = sol[i], next = sol[(i + 1) % n];
        w[i] = 1.0 + dist_matrix[prev][v] + dist_matrix[v][next] - dist_matrix[prev][next] + points[v].cost;
    }

    // sampling without replacement
    vector<char> removed(n, 0);
    for (int k = 0; k < m; ++k) {
        discrete_distribution<int> d(w.begin(), w.end());
        int i = d(rng());
        removed[i] = 1;
        w[i] = 0.0;
    }

    vector<int> result;
    result.reserve(n - m);
    for (int i = 0; i < n; ++i)
        if (!removed[i]) result.push_back(sol[i]);
    return result;
}


vector<int> destroy(const vector<int>& sol, Destroy type, const vector<Point>& points,
                    const vector<vector<int>>& dist_matrix) {
    switch (type) {
        case Destroy::RandomNodes: return destroy_random_nodes(sol);
        case Destroy::SingleSubpath: return destroy_single_subpath(sol);
        case Destroy::MultiSubpath: return destroy_multi_subpath(sol);
        case Destroy::WeightedNodes: return destroy_weighted_nodes(sol, points, dist_matrix);
    }
    return sol;
}

// cycle_weighted_regret heuristic but starts with incomplete cycle instead of a single node
vector<int> tsp_cycle_weighted_regret_repair(const vector<Point> &points, const vector<vector<int>> &dist_matrix, vector<int> solution, int target_size, double w_regret, double w_obj)
{
    vector<bool> visited(points.size(), false);
    for (int v : solution)
    {
        visited[v] = true;
    }

    while ((int)solution.size() < target_size)
    {
        int max_idx = -1;
        double max_score = -DOUBLE_INFINITY;
        int max_insert_pos = -1;
        for (size_t i = 0; i < points.size(); ++i)
        {
            if (visited[i])
            {
                continue;
            }

            // best and second best insertion
            int best_val = INT_INFINITY;
            int second_val = INT_INFINITY;
            int best_insert_pos = -1;
            for (size_t insert_pos = 0; insert_pos < solution.size(); ++insert_pos)
            {
                int prev = solution[(insert_pos + solution.size() - 1) % solution.size()];
                int next = solution[insert_pos];
                int dist = dist_matrix[prev][i] + dist_matrix[i][next] - dist_matrix[prev][next];
                int total = dist + points[i].cost;

                if (total < best_val)
                {
                    second_val = best_val;
                    best_val = total;
                    best_insert_pos = insert_pos;
                }
                else if (total < second_val)
                {
                    second_val = total;
                }
            }

            int regret = (second_val == INT_INFINITY) ? 0 : second_val - best_val;
            double score = w_regret * regret - w_obj * best_val;
            if (score > max_score)
            {
                max_score = score;
                max_idx = i;
                max_insert_pos = best_insert_pos;
            }
        }
        if (max_idx == -1)
        {
            break;
        }
        solution.insert(solution.begin() + max_insert_pos, max_idx);
        visited[max_idx] = true;
    }
    return solution;
}

tuple<vector<int>, int> lns(vector<Point>& points, vector<vector<int>>& dist_matrix, double max_time_ms, Destroy destroy_type, bool use_ls) {
    auto start_time = chrono::steady_clock::now();

    auto [best_solution, first_evals] = steepest_ls_with_prev_deltas(tsp_random_heuristic(points), dist_matrix, points);
    int best_cost = tsp_calculate_costs(best_solution, dist_matrix, points);
    int target = best_solution.size();
    int iterations = 0;

    while (elapsed_ms(start_time) < max_time_ms) {
        vector<int> solution = destroy(best_solution, destroy_type, points, dist_matrix);
        solution = tsp_cycle_weighted_regret_repair(points, dist_matrix, solution, target, W_REGRET, W_OBJ);
        if (use_ls) {
            auto [solution_ls, evals] = steepest_ls_with_prev_deltas(solution, dist_matrix, points);
            solution = solution_ls;
        }
        int cost = tsp_calculate_costs(solution, dist_matrix, points);
        if (cost < best_cost) {
            best_cost = cost;
            best_solution = solution;
        }
        iterations++;
    }
    return {best_solution, iterations};
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <filename> [time_limit_ms]" << endl;
        return 1;
    }
    string filename = argv[1];
    vector<Point> points = load_points(filename);
    if (points.empty()) {
        cerr << "No points loaded" << endl;
        return 1;
    }
    vector<vector<int>> dist_matrix = calculate_distance_matrix(points);

    double time_limit_ms = 840; // chosen the same as in the previous lab
    if (argc >= 3) time_limit_ms = atof(argv[2]);

    const int RUNS = 20;

    struct Variant { string name; Destroy type; };
    vector<Variant> variants = {
        {"random nodes", Destroy::RandomNodes},
        {"single subpath", Destroy::SingleSubpath},
        {"multiple subpaths", Destroy::MultiSubpath},
        {"weighted nodes", Destroy::WeightedNodes},
    };

    for (auto& v : variants) {
        for (bool use_ls : {true, false}) {
            vector<int> costs, iterations_list, best_solution;
            vector<double> times;
            int best_cost = INT_INFINITY;

            for (int i = 0; i < RUNS; ++i) {
                auto t = chrono::steady_clock::now();
                auto [solution, iterations] = lns(points, dist_matrix, time_limit_ms, v.type, use_ls);
                times.push_back(elapsed_ms(t));
                iterations_list.push_back(iterations);

                int cost = tsp_calculate_costs(solution, dist_matrix, points);
                costs.push_back(cost);
                if (cost < best_cost) {
                    best_cost = cost;
                    best_solution = solution;
                }
            }

            string name = "LNS [" + v.name + (use_ls ? "] with LS" : "] without LS");
            print_stats(name, costs, best_solution, best_cost, times);
            cout << name << " main loop iterations (max, avg, min): "
                 << *max_element(iterations_list.begin(), iterations_list.end()) << ", "
                 << accumulate(iterations_list.begin(), iterations_list.end(), 0.0) / iterations_list.size() << ", "
                 << *min_element(iterations_list.begin(), iterations_list.end()) << "\n\n";
        }
    }
    return 0;
}