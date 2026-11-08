#include "../common/helpers.h"
#include "../common/greedy_heuristics.h"
#include "../common/local_search.h"
#include <iostream>
#include <set>

using namespace std;

struct Move
{
    bool is_in;
    int a, b; // in: idx of nodes in first edge, out: idx of nodes to be exchanged (a-in, b-out)
    int c, d; // in: idx of nodes in second edge, out: idx of nodes neighboring a (c-before, d-after)
    int delta;
};

struct CompareMoves {
    bool operator()(const Move &lhs, const Move &rhs) const {
        return lhs.delta < rhs.delta;
    }
};

void evaluate_moves(
    const vector<int> &solution,
    const vector<int> &not_in_solution,
    const vector<vector<int>> &dist_matrix,
    const vector<Point> &points,
    multiset<Move, CompareMoves> &lm) {

    int n = solution.size();
    // inter nodes exchange
    vector<pair<int, int>> out_neighbors = out_neighborhood_nodes(solution, not_in_solution);
    for (auto [v1, v2] : out_neighbors)
    {
        int delta = delta_out(solution, not_in_solution, v1, v2, dist_matrix, points);
        if (delta < 0)
            lm.insert({false, solution[v1], not_in_solution[v2], solution[(v1+n-1) % n], solution[(v1+1) % n], delta});
    }
    
    // intra edges exchange
    vector<pair<int, int>> in_neighbors = in_neighborhood_edges(solution);
    for (auto [v1, v2]: in_neighbors){
        int a = solution[(v1+n-1) % n];
        int b = solution[v1];
        int c = solution[v2 - 1];
        int d = solution[v2];
        if (a == d || b==c) continue; // skip if edges are adjacent

        int delta = delta_in_edges(solution, v1, v2, dist_matrix);
        if (delta < 0){
            lm.insert({true, a, b, c, d, delta});
        }
        
        // this move is infeasible in the current solution but may be in the future
        int inverted_delta = dist_matrix[a][d] + dist_matrix[b][c] - dist_matrix[a][b] - dist_matrix[c][d];
        if (inverted_delta < 0){
            lm.insert({true, a, b, d, c, inverted_delta});
        }
    }
        
}

// pos is -1 if not in the cycle and idx in cycle if it is in the cycle
// -1 - not applicable, delete; 0 - not applicable, skip; 1 - applicable
int is_applicable(const vector<int> &solution, const Move &m, const vector<int> &pos) {
    int n = solution.size();
    if (m.is_in == true) { // in moves
        // case1: at least one edge no longer exists - delete
        // one node is out of cycle
        int idx_a = pos[m.a], idx_b = pos[m.b], idx_c = pos[m.c], idx_d = pos[m.d];
        if (idx_a == -1 || idx_b == -1 || idx_c == -1 || idx_d == -1) {
            return -1;
        }
        int ab = abs(idx_a - idx_b);
        int cd = abs(idx_c - idx_d);
        // nodes are not adjacent in the cycle
        if ((ab != 1 && ab != n - 1) ||
            (cd != 1 && cd != n - 1)) {
            return -1;
        }

        // case2: one edge change direction - skip
        bool ab_direction = (idx_a + 1) % n == idx_b; // true if ab still points in the same direction
        bool cd_direction = (idx_c + 1) % n == idx_d; // true if cd still points in the same direction

        if (ab_direction != cd_direction) {
            return 0;
        }
        return 1;
    }
    else {
        if (pos[m.a] != -1 && pos[m.b] == -1) {
            int idx_a = pos[m.a];
            int idx_before_a = (idx_a + n - 1) % n;
            int idx_after_a = (idx_a + 1) % n;
            if ((pos[m.c] == idx_before_a && pos[m.d] == idx_after_a) ||    // exchanging the neighbors does not change the delta
               (pos[m.c] == idx_after_a && pos[m.d] == idx_before_a)) {
                return 1;
            }
        }
    }
    return -1;
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

tuple<vector<int>, int> steepest_ls_with_prev_deltas(vector<int> init_solution, const vector<vector<int>> &dist_matrix, const vector<Point> &points)
{
    multiset<Move, CompareMoves> lm;
    vector<int> solution = init_solution;
    int n = init_solution.size();
    
    vector<int> not_in_solution;
    for (size_t i = 0; i < points.size(); ++i)
    {
        if (find(init_solution.begin(), init_solution.end(), i) == init_solution.end())
        {
            not_in_solution.push_back(static_cast<int>(i));
        }
    }
    
    vector<int> pos(points.size(), -1);
    for (size_t i = 0; i < solution.size(); ++i) {
        pos[solution[i]] = static_cast<int>(i);
    }
    vector<int> pos_out(points.size(), -1);
    for (size_t i = 0; i < not_in_solution.size(); ++i) {
        pos_out[not_in_solution[i]] = static_cast<int>(i);
    }

    evaluate_moves(solution, not_in_solution, dist_matrix, points, lm);
    bool is_same_solution = true;
    
    int evaluate_counter = 1;

    while (true)
    {
        bool move_found = false;
        Move best_move;
        for (auto it = lm.begin(); it != lm.end(); ) {
            int ret = is_applicable(solution, *it, pos);

            if (ret == -1) {
                it = lm.erase(it);
            } else if (ret == 0) {
                ++it;
            } else {
                best_move = *it;
                lm.erase(it); // delete it from lm because it would be deleted in next iteration either way
                move_found = true;
                break;
            }
        }
        
        if (move_found){
            //apply the move

            if(best_move.is_in){
                int idx_a = pos[best_move.a];
                int idx_b = pos[best_move.b];
                int idx_c = pos[best_move.c];
                int idx_d = pos[best_move.d];

                if ((idx_a + 1) % n == idx_b) { // same direction as saved move
                    apply_reverse(solution, pos, idx_b, idx_c);
                } else { // reverse direction
                    apply_reverse(solution, pos, idx_a, idx_d);
                }
            }
            else{
                int idx_a = pos[best_move.a];
                int idx_b = pos_out[best_move.b];

                solution[idx_a] = best_move.b;
                not_in_solution[idx_b] = best_move.a;

                pos[best_move.a] = -1;
                pos[best_move.b] = idx_a;
                pos_out[best_move.a] = idx_b;
                pos_out[best_move.b] = -1;
            }
            is_same_solution = false;

        }
        else {
            if (is_same_solution) {
                break;
            }
            evaluate_moves(solution, not_in_solution, dist_matrix, points, lm);
            evaluate_counter++;
            is_same_solution = true;
        }
    }
    return {solution, evaluate_counter};
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
    
    vector<int> costs_deltas, costs_steepest;
    vector<double> times_candiadate, times_steepest;
    vector<int> best_solution_deltas, best_solution_steepest;
    vector<int> evaluations_deltas, evaluations_steepest;
    int best_cost_deltas = INT_INFINITY;
    int best_cost_steepest = INT_INFINITY;
    for (int i = 0; i < 200; ++i) {
        vector<int> initial_solution = tsp_random_heuristic(points);
        auto t = chrono::steady_clock::now();
        
        auto [solution_deltas, evaluate_counter] = steepest_ls_with_prev_deltas(initial_solution, dist_matrix, points);
        times_candiadate.push_back(elapsed_ms(t));
        evaluations_deltas.push_back(evaluate_counter);

        auto t_steepest = chrono::steady_clock::now();
        auto [solution_steepest, evaluate_counter_steepest] = local_search_steepest(initial_solution, "edge", dist_matrix, points);
        times_steepest.push_back(elapsed_ms(t_steepest));
        evaluations_steepest.push_back(evaluate_counter_steepest);
        
        
        int cost = tsp_calculate_costs(solution_deltas, dist_matrix, points);
        costs_deltas.push_back(cost);
        if (cost < best_cost_deltas) {
            best_cost_deltas = cost;
            best_solution_deltas = solution_deltas;
        }

        int cost_steepest = tsp_calculate_costs(solution_steepest, dist_matrix, points);
        costs_steepest.push_back(cost_steepest);
        if (cost_steepest < best_cost_steepest) {
            best_cost_steepest = cost_steepest;
            best_solution_steepest = solution_steepest;
        }
    }
    print_stats("Steepest Local Search with Previous Deltas", costs_deltas, best_solution_deltas, best_cost_deltas, times_candiadate);
    cout << "Average Evaluations for Steepest Local Search with Previous Deltas: ";
    double avg_evaluations_deltas = accumulate(evaluations_deltas.begin(), evaluations_deltas.end(), 0.0) / evaluations_deltas.size();
    cout << avg_evaluations_deltas << endl;

    print_stats("Steepest Local Search", costs_steepest, best_solution_steepest, best_cost_steepest, times_steepest);
    cout << "Average Evaluations for Steepest Local Search: ";
    double avg_evaluations_steepest = accumulate(evaluations_steepest.begin(), evaluations_steepest.end(), 0.0) / evaluations_steepest.size();
    cout << avg_evaluations_steepest << endl;
    return 0;
}