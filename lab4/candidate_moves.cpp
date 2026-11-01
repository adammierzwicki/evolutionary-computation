#include "../common/helpers.h"
#include "../common/greedy_heuristics.h"
#include "../common/local_search.h"
#include <iostream>

using namespace std;


// for each node find n closest neighbors (dist+cost)
// output vector: [node 0: closest neighbors, node 1: closest neighbors, ...]
vector<vector<int>> candidate_moves(const vector<Point>& points, const vector<vector<int>>& dist_matrix, int n = 10) {
    vector<vector<pair<int, int>>> temp_candidates(points.size());

    for (size_t i = 0; i < points.size(); ++i) {
        for (size_t j = i + 1; j < points.size(); ++j) {
            temp_candidates[i].emplace_back(j, dist_matrix[i][j]+points[j].cost);
            temp_candidates[j].emplace_back(i, dist_matrix[i][j]+points[i].cost);
        }
    }
    vector<vector<int>> candidates(points.size());
    for (size_t i = 0; i < points.size(); ++i) {
        auto& neighbors = temp_candidates[i];
        sort(neighbors.begin(), neighbors.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
            if (a.second == b.second) return a.first < b.first;
            return a.second < b.second;
        });
        if (neighbors.size() > n) {
            neighbors.resize(n);
        }
        for (const auto& neighbor : neighbors) {
            candidates[i].push_back(neighbor.first);
        }
    }

    return candidates;
}

// reverse cyclic segment solution[from..to] in place, keep pos[] in sync
void apply_reverse(vector<int>& solution, vector<int>& pos, int from, int to) {
    int n = solution.size();
    int len = ((to - from + n) % n) + 1; // number of elements in the segment to reverse
    for (int k = 0; k < len / 2; ++k) {
        int a = (from + k) % n;
        int b = (to - k + n) % n;
        swap(solution[a], solution[b]);
        pos[solution[a]] = a;
        pos[solution[b]] = b;
    }
}

struct InMove  { int from, to, delta; };   // reverse segment from..to
struct OutMove { int idx, delta; };        // replace solution[idx]

/*
przypadek 1
    .
    v +1    v +1
1 - 2 - 3 - 4 - 5 - 6
2-3  i 4-5

delta = -edge(v, v+1) - edge(vc, vc+1) + edge(v, vc) + edge(v+1, vc+1)

przypadek 2
    .
-1 v    -1 v
1 - 2 - 3 - 4 - 5 - 6
1-2 i 3-4

delta = -edge(v, v-1) - edge(vc, vc-1) + edge(v, vc) + edge(v-1, vc-1)
*/
InMove delta_in(int node_idx, int candidate_idx, const vector<vector<int>>& dist_matrix, const vector<int>& solution) {
    int n = solution.size();
    int before_node_idx = (node_idx > 0) ? node_idx - 1 : n - 1;
    int after_node_idx = (node_idx + 1) % n;
    int before_candidate_idx = (candidate_idx > 0) ? candidate_idx - 1 : n - 1;
    int after_candidate_idx = (candidate_idx + 1) % n;

    if (before_node_idx == candidate_idx || after_node_idx == candidate_idx) {
        return {0, 0, INT_INFINITY};   // already adjacent
    }

    int node = solution[node_idx], candidate = solution[candidate_idx];
    int before_node = solution[before_node_idx], after_node = solution[after_node_idx];
    int before_candidate = solution[before_candidate_idx], after_candidate = solution[after_candidate_idx];

    int delta1 = -dist_matrix[node][after_node] - dist_matrix[candidate][after_candidate]
                 + dist_matrix[node][candidate] + dist_matrix[after_node][after_candidate];
    int delta2 = -dist_matrix[node][before_node] - dist_matrix[candidate][before_candidate]
                 + dist_matrix[node][candidate] + dist_matrix[before_node][before_candidate];

    if (delta1 < delta2) return {after_node_idx, candidate_idx, delta1};
    return {node_idx, before_candidate_idx, delta2};
}

/*
case not in cycle

    przypadek 1

    delta = +edge(v, vc) -edge(v, v-1) -cost(v-1) - edge(v-1, v-2) + edge(vc, v-2) + cost(vc)


    przypadek 2
    
    delta = +edge(v, vc) -edge(v, v+1) -cost(v+1) - edge(v+1, v+2) + edge(vc, v+2) + cost(vc)

returns (new solution, delta, removed node)
*/
OutMove delta_out(int node_idx, int candidate, const vector<vector<int>>& dist_matrix, const vector<int>& solution, const vector<Point>& points) {
    int n = solution.size();
    int before_node_idx = (node_idx > 0) ? node_idx - 1 : n - 1;
    int before_before_node_idx = (before_node_idx > 0) ? before_node_idx - 1 : n - 1;
    int after_node_idx = (node_idx + 1) % n;
    int after_after_node_idx = (after_node_idx + 1) % n;
    int node = solution[node_idx];
    int before_node = solution[before_node_idx];
    int before_before_node = solution[before_before_node_idx];
    int after_node = solution[after_node_idx];
    int after_after_node = solution[after_after_node_idx];

    int delta1 = dist_matrix[node][candidate] - dist_matrix[node][before_node] - points[before_node].cost
                 - dist_matrix[before_node][before_before_node] + dist_matrix[candidate][before_before_node] + points[candidate].cost;
    int delta2 = dist_matrix[node][candidate] - dist_matrix[node][after_node] - points[after_node].cost
                 - dist_matrix[after_node][after_after_node] + dist_matrix[candidate][after_after_node] + points[candidate].cost;

    if (delta1 < delta2) return {before_node_idx, delta1};
    return {after_node_idx, delta2};
}

vector<int> steepest_local_search_moves(vector<int> solution, const vector<Point>& points,
                                        const vector<vector<int>>& dist_matrix,
                                        const vector<vector<int>>& candidates) {
    int n = solution.size();
    vector<int> pos(points.size(), -1);          // pos[node] = index in solution, -1 if not selected
    for (int i = 0; i < n; ++i) pos[solution[i]] = i;

    while (true) {
        int best_delta = 0;                      // only strictly improving moves
        int best_type = -1;                      // 0 = in (2-opt), 1 = out (replace)
        int best_a = 0, best_b = 0;              // in: from,to | out: idx,new node

        for (int i = 0; i < n; ++i) {
            for (int neighbor : candidates[solution[i]]) {
                if (pos[neighbor] == -1) {
                    OutMove m = delta_out(i, neighbor, dist_matrix, solution, points);
                    if (m.delta < best_delta) {
                        best_delta = m.delta; best_type = 1; best_a = m.idx; best_b = neighbor;
                    }
                } else {
                    InMove m = delta_in(i, pos[neighbor], dist_matrix, solution);
                    if (m.delta < best_delta) {
                        best_delta = m.delta; best_type = 0; best_a = m.from; best_b = m.to;
                    }
                }
            }
        }

        if (best_type == -1) break;              // local optimum

        if (best_type == 0) {
            apply_reverse(solution, pos, best_a, best_b);
        } else {
            pos[solution[best_a]] = -1;
            solution[best_a] = best_b;
            pos[best_b] = best_a;
        }
    }
    return solution;
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
    
    
    // for (int c_size = 3; c_size < 21; ++c_size) {
    //     vector<int> costs;
    //     vector<double> times;
    //     vector<int> best_solution;
    //     int best_cost = INT_INFINITY;
    //     for (int i = 0; i < 200; ++i) {
    //         vector<int> initial_solution = tsp_random_heuristic(points);
    //         auto t = chrono::steady_clock::now();
    //         vector<int> solution = steepest_local_search_moves(initial_solution, points, dist_matrix, candidate_moves(points, dist_matrix, c_size));
    //         times.push_back(elapsed_ms(t));
            
    //         int cost = tsp_calculate_costs(solution, dist_matrix, points);
    //         costs.push_back(cost);
    //         if (cost < best_cost) {
    //             best_cost = cost;
    //             best_solution = solution;
    //         }
    //     }
    //     print_stats("Steepest Local Search Candidate Moves c_size=" + to_string(c_size), costs, best_solution, best_cost, times);
    // }
    
    vector<int> costs_candidate, costs_steepest;
    vector<double> times_candiadate, times_steepest;
    vector<int> best_solution_candidate, best_solution_steepest;
    int best_cost_candidate = INT_INFINITY;
    int best_cost_steepest = INT_INFINITY;
    for (int i = 0; i < 200; ++i) {
        vector<int> initial_solution = tsp_random_heuristic(points);
        auto t = chrono::steady_clock::now();
        vector<int> solution_candidate = steepest_local_search_moves(initial_solution, points, dist_matrix, candidate_moves(points, dist_matrix, 10));
        times_candiadate.push_back(elapsed_ms(t));

        auto t_steepest = chrono::steady_clock::now();
        vector<int> solution_steepest = local_search_steepest(initial_solution, "edge", dist_matrix, points);
        times_steepest.push_back(elapsed_ms(t_steepest));
        
        int cost = tsp_calculate_costs(solution_candidate, dist_matrix, points);
        costs_candidate.push_back(cost);
        if (cost < best_cost_candidate) {
            best_cost_candidate = cost;
            best_solution_candidate = solution_candidate;
        }

        int cost_steepest = tsp_calculate_costs(solution_steepest, dist_matrix, points);
        costs_steepest.push_back(cost_steepest);
        if (cost_steepest < best_cost_steepest) {
            best_cost_steepest = cost_steepest;
            best_solution_steepest = solution_steepest;
        }
    }
    print_stats("Steepest Local Search with Candidate Moves", costs_candidate, best_solution_candidate, best_cost_candidate, times_candiadate);
    print_stats("Steepest Local Search w/o Candidate Moves", costs_steepest, best_solution_steepest, best_cost_steepest, times_steepest);
    return 0;
}

/*
\
 . (N0)
  \
   . (N1) -- . (N2)  // candidate
            /
           /
                       . (N3)
      /
. (n4)

0 - 1 - 3 - 4
0 - 1 - 2 - 4

case in cycle
    przypadek 1
        .
        v+1     v+1
    1 - 2 - 3 - 4 - 5 - 6
    2-3  i 4-5

    delta = -edge(v, v+1) - edge(vc, vc+1) + edge(v, vc) + edge(v+1, vc+1)

    przypadek 2
        .
     -1 v    -1 v
    1 - 2 - 3 - 4 - 5 - 6
    1-2 i 3-4

    delta = -edge(v, v-1) - edge(vc, vc-1) + edge(v, vc) + edge(v-1, vc-1)

case not in cycle

    przypadek 1

    delta = +edge(v, vc) -edge(v, v-1) -cost(v-1) - edge(v-1, v-2) + edge(vc, v-2) + cost(vc)


    przypadek 2
    
    delta = +edge(v, vc) -edge(v, v+1) -cost(v+1) - edge(v+1, v+2) + edge(vc, v+2) + cost(vc)
    

        .
        v   
1 - 2 - 3 - 4 - 5 - 6
|       .       |
|       .       |
|       .       | 
 ------ 8 ------
        ^
*/