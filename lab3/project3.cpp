#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <random>
#include <tuple>
#include <limits>
#include <algorithm>
#include <numeric>
#include <chrono>

using namespace std;

const int INT_INFINITY = numeric_limits<int>::max();
const double DOUBLE_INFINITY = numeric_limits<double>::infinity();

mt19937 &rng()
{
    static mt19937 gen(random_device{}());
    return gen;
}

struct Point
{
    int x;
    int y;
    int cost;
};

ostream &operator<<(ostream &os, const vector<int> &v)
{
    os << "[";
    for (size_t i = 0; i < v.size(); ++i)
    {
        if (i > 0)
        {
            os << ", ";
        }
        os << v[i];
    }
    os << "]";
    return os;
}

vector<Point> load_points(const string &filename)
{
    vector<Point> points;
    ifstream file(filename);
    if (!file)
    {
        cerr << "Error opening file: " << filename << endl;
        return points;
    }
    string line;
    while (getline(file, line))
    {
        Point p;
        if (sscanf(line.c_str(), "%d;%d;%d", &p.x, &p.y, &p.cost) == 3)
        {
            points.push_back(p);
        }
    }
    return points;
}

int euclidean_distance(const Point &p1, const Point &p2)
{
    return int(sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2)) + 0.5);
}

vector<vector<int>> calculate_distance_matrix(const vector<Point> &points)
{
    size_t n = points.size();
    vector<vector<int>> m(n, vector<int>(n, 0));
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 0; j < n; ++j)
        {
            m[i][j] = euclidean_distance(points[i], points[j]);
        }
    }
    return m;
}

tuple<int, int, int> tsp_calculate_costs(const vector<int> &solution, const vector<vector<int>> &dist_matrix, const vector<Point> &points)
{
    int dist_cost = 0;
    int node_cost = 0;
    for (size_t i = 0; i < solution.size(); ++i)
    {
        node_cost += points[solution[i]].cost;
        dist_cost += dist_matrix[solution[i]][solution[(i + 1) % solution.size()]];
    }
    return {node_cost, dist_cost, node_cost + dist_cost};
}

int tsp_verify(const vector<int> &solution, const vector<vector<int>> &dist_matrix, const vector<Point> &points)
{
    auto [node_cost, dist_cost, total_cost] = tsp_calculate_costs(solution, dist_matrix, points);
    return total_cost;
}


int random_start_idx(const vector<Point> &points)
{
    uniform_int_distribution<> dis(0, points.size() - 1);
    return dis(rng());
}

vector<int> tsp_random_heuristic(const vector<Point> &points)
{
    vector<int> idxs(points.size());
    iota(idxs.begin(), idxs.end(), 0);
    shuffle(idxs.begin(), idxs.end(), rng());
    idxs.resize(ceil(points.size() / 2.0));
    return idxs;
}

tuple<int, int> find_nn(const int point_idx, const vector<vector<int>> &dist_matrix, const vector<Point> &points, const vector<bool> &visited)
{
    int min_idx = -1;
    int min_val = INT_INFINITY;
    for (size_t i = 0; i < points.size(); ++i)
    {
        if (visited[i])
        {
            continue;
        }
        int dist = dist_matrix[point_idx][i];
        int total = dist + points[i].cost;
        if (total < min_val)
        {
            min_val = total;
            min_idx = i;
        }
    }
    return {min_idx, min_val};
}

vector<int> tsp_nn_any_pos_weighted_regret_heuristic(const vector<Point> &points, const vector<vector<int>> &dist_matrix, int start_idx = -1, double w_regret = 0.5, double w_obj = 0.5)
{
    if (start_idx == -1)
    {
        start_idx = random_start_idx(points);
    }

    vector<int> solution;
    vector<bool> visited(points.size(), false);
    solution.push_back(start_idx);
    visited[start_idx] = true;
    while (solution.size() < ceil(points.size() / 2.0))
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
            for (size_t insert_pos = 0; insert_pos <= solution.size(); ++insert_pos)
            {
                int dist;
                if (insert_pos == 0)
                {
                    dist = dist_matrix[i][solution.front()];
                }
                else if (insert_pos == solution.size())
                {
                    dist = dist_matrix[solution.back()][i];
                }
                else
                {
                    int prev = solution[insert_pos - 1];
                    int next = solution[insert_pos];
                    dist = dist_matrix[prev][i] + dist_matrix[i][next] - dist_matrix[prev][next];
                }
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

vector<int> tsp_cycle_weighted_regret_heuristic(const vector<Point> &points, const vector<vector<int>> &dist_matrix, int start_idx = -1, double w_regret = 0.5, double w_obj = 0.5)
{
    if (start_idx == -1)
    {
        start_idx = random_start_idx(points);
    }

    vector<int> solution;
    vector<bool> visited(points.size(), false);
    solution.push_back(start_idx);
    visited[start_idx] = true;

    auto [second_idx, second_val] = find_nn(start_idx, dist_matrix, points, visited);
    if (second_idx == -1)
    {
        return solution;
    }
    solution.push_back(second_idx);
    visited[second_idx] = true;

    while (solution.size() < ceil(points.size() / 2.0))
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

double elapsed_ms(chrono::steady_clock::time_point start)
{
    return chrono::duration<double, milli>(chrono::steady_clock::now() - start).count();
}

void print_stats(const string &name, const vector<int> &costs, const vector<int> &best_solution, int best_cost, const vector<double> &times_ms)
{
    cout << "----------------------------------------" << endl;
    cout << name << endl;
    cout << "----------------------------------------" << endl;
    cout << " Objective value function:" << endl
         << "  - min: " << *min_element(costs.begin(), costs.end()) << endl
         << "  - max: " << *max_element(costs.begin(), costs.end()) << endl
         << "  - avg: " << accumulate(costs.begin(), costs.end(), 0.0) / costs.size() << endl;
    cout << " Avg time: " << accumulate(times_ms.begin(), times_ms.end(), 0.0) / times_ms.size() << " ms" << endl;
    cout << " Best solution:" << endl
         << " Cost: " << best_cost << endl
         << " Path: " << best_solution << endl << endl;
}


vector<pair<int, int>> in_neighborhood_nodes(vector<int> solution)
{
    // generating neighborhood by exchanging 2 nodes within solution
    vector<pair<int, int>> neighbors;
    for (size_t i = 0; i < solution.size(); ++i)
    {
        for (size_t j = i + 1; j < solution.size(); ++j)
        {
            neighbors.emplace_back(i, j);
        }
    }
    return neighbors;
}

vector<pair<int, int>> out_neighborhood_nodes(vector<int> solution, vector<int> not_in_solution)
{
    // generating neighborhood by exchanging nodes in solution with nodes not in solution
    vector<pair<int, int>> neighbors;
    for(size_t i = 0; i < solution.size(); ++i)
    {
        for (size_t j = 0; j < not_in_solution.size(); ++j)
        {
            neighbors.emplace_back(i, j);
        }
    }
    return neighbors;
}

vector<pair<int, int>> in_neighborhood_edges(vector<int> solution)
{
    // generate whole neighborhood to solution. neighbor is obtained by exchanging edges.
    // pair i, j is the edge before node i exchanged with edge before node j
    // 0   1   2   3   4   5
    //   1 > 2 > 3 > 4 > 5 > 6 
    // for all pairs i < j to not go over the return path
    vector<pair<int, int>> neighborhood;
    for (size_t i = 0; i < solution.size(); ++i)
    {
        for (size_t j = i+1; j < solution.size(); ++j)
        {
            neighborhood.emplace_back(i, j);
        }
    }
    return neighborhood;
}

int delta_in_nodes(const vector<int> &solution, int i, int j, const vector<vector<int>> &dist_matrix)
{
    int n = solution.size();
    int before_i = (i > 0) ? i - 1 : n - 1;
    int delta = 0;
    if (i + 1 == j)
    {
        delta -= dist_matrix[solution[before_i]][solution[i]] + dist_matrix[solution[j]][solution[(j + 1) % n]];
        delta += dist_matrix[solution[before_i]][solution[j]] + dist_matrix[solution[i]][solution[(j + 1) % n]];
    }
    else if (before_i == j)
    {
        delta -= dist_matrix[solution[j - 1]][solution[j]] + dist_matrix[solution[i]][solution[i + 1]];
        delta += dist_matrix[solution[j - 1]][solution[i]] + dist_matrix[solution[j]][solution[i + 1]];
    }
    else
    {
        delta -= dist_matrix[solution[before_i]][solution[i]] + dist_matrix[solution[i]][solution[(i + 1) % n]];
        delta -= dist_matrix[solution[j - 1]][solution[j]] + dist_matrix[solution[j]][solution[(j + 1) % n]];
        delta += dist_matrix[solution[before_i]][solution[j]] + dist_matrix[solution[j]][solution[(i + 1) % n]];
        delta += dist_matrix[solution[j - 1]][solution[i]] + dist_matrix[solution[i]][solution[(j + 1) % n]];
    }
    return delta;
}

int delta_in_edges(const vector<int> &solution, int i, int j, const vector<vector<int>> &dist_matrix)
{
    int delta = 0;
    int before_i = (i > 0) ? i - 1 : solution.size() - 1;
    delta -= dist_matrix[solution[before_i]][solution[i]] + dist_matrix[solution[j - 1]][solution[j]];
    delta += dist_matrix[solution[before_i]][solution[j - 1]] + dist_matrix[solution[i]][solution[j]];
    return delta;
}

int delta_out(const vector<int> &solution, const vector<int> &not_in_solution, int i, int j, const vector<vector<int>> &dist_matrix, const vector<Point> &points)
{
    int delta = 0;
    int n = solution.size();
    int before_i = (i > 0) ? i - 1 : n - 1;
    int next_i = (i + 1) % n;
    // remove edges (i-1, i), (i, i+1) and node cost
    delta -= dist_matrix[solution[before_i]][solution[i]] + dist_matrix[solution[i]][solution[next_i]];
    delta -= points[solution[i]].cost;
    // add edges (i-1, j), (j, i+1) and node cost
    delta += dist_matrix[solution[before_i]][not_in_solution[j]] + dist_matrix[not_in_solution[j]][solution[next_i]];
    delta += points[not_in_solution[j]].cost;
    return delta;
}

pair<vector<int>, int> in_neighborhood_edges_evaluate(vector<int> solution, int cost, const vector<vector<int>> &dist_matrix)
{
    // subtract the cost of edge before i and edge before j (solution[i-1], solution[i]) + cost (solution[j-1], solution[j])
    // add the cost of edges (from before i to before j) and (from i to j) (solution[i-1], solution[j-1]) + (solution[i], solution[j])
    // returns best solution and its cost
    auto neighborhood = in_neighborhood_edges(solution);
    pair<int, int> best_swap = neighborhood.front();
    int min_delta = INT_INFINITY;
    for (auto &[i, j] : neighborhood)
    {
        int delta = delta_in_edges(solution, i, j, dist_matrix);
        if (delta < min_delta)
        {
            min_delta = delta;
            best_swap = {i, j};
        }
    }

    reverse(solution.begin() + best_swap.first, solution.begin() + best_swap.second);
    return {solution, cost + min_delta};
}

pair<vector<int>, int> in_neighborhood_nodes_evaluate(vector<int> solution, int cost, const vector<vector<int>> &dist_matrix)
{
    auto neighborhood = in_neighborhood_nodes(solution);
    int min_delta = INT_INFINITY;
    pair<int, int> best_swap = {0, 0};
    for (auto &[i, j] : neighborhood)
    {
        int delta = delta_in_nodes(solution, i, j, dist_matrix);
        if (delta < min_delta)
        {
            min_delta = delta;
            best_swap = {i, j};
        }
    }
    swap(solution[best_swap.first], solution[best_swap.second]);
    return {solution, cost + min_delta};
}


tuple<vector<int>, int, vector<int>> out_neighborhood_evaluate(vector<int> solution, vector<int> not_in_solution, int cost, const vector<vector<int>> &dist_matrix, const vector<Point> &points) {
    auto neighborhood = out_neighborhood_nodes(solution, not_in_solution);
    int min_delta = INT_INFINITY;
    pair<int, int> best_swap = {0, 0};
    for (auto &[i, j] : neighborhood)
    {
        int delta = delta_out(solution, not_in_solution, i, j, dist_matrix, points);
        if (delta < min_delta)
        {
            min_delta = delta;
            best_swap = {i, j};
        }
    }
    
    swap(solution[best_swap.first], not_in_solution[best_swap.second]);
    return {solution, cost + min_delta, not_in_solution};
}


// in_neighborhood: "edge", "node"
vector<int> local_search_steepest(vector<int> init_solution, string in_neighborhood, const vector<vector<int>> &dist_matrix, const vector<Point> &points) {
    vector<int> best_solution = init_solution;
    int best_cost = tsp_verify(best_solution, dist_matrix, points);
    
    vector<int> not_in_solution;
    for (int i = 0; i < points.size(); ++i) {
        if (find(init_solution.begin(), init_solution.end(), i) == init_solution.end()) {
            not_in_solution.push_back(i);
        }
    }
    
    while (true)
    {   
        vector<int> in_solution;
        int in_cost = INT_INFINITY;
        auto [out_solution, out_cost, new_not_in_solution] = out_neighborhood_evaluate(best_solution, not_in_solution, best_cost, dist_matrix, points);
        if (in_neighborhood == "edge") {
            tie(in_solution, in_cost) = in_neighborhood_edges_evaluate(best_solution, best_cost, dist_matrix);
        } else if (in_neighborhood == "node") {
            tie(in_solution, in_cost) = in_neighborhood_nodes_evaluate(best_solution, best_cost, dist_matrix);
        }
        
        if (in_cost < out_cost && in_cost < best_cost) {
            best_solution = in_solution;
            best_cost = in_cost;
        } else if (out_cost < best_cost) {
            best_solution = out_solution;
            best_cost = out_cost;
            not_in_solution = new_not_in_solution;
        } else {
            break;
        }
    }
    return best_solution;
}

struct Move 
{
    bool inter; //true = inter, false = intra
    int i;
    int j;
};

vector<int> local_search_greedy(vector<int> solution, string in_neighborhood, const vector<vector<int>> &dist_matrix, const vector<Point> &points)
{
    const bool edge_mode = (in_neighborhood == "edge");

    vector<int> not_in_solution;
    for (int i = 0; i < (int)points.size(); ++i)
        if (find(solution.begin(), solution.end(), i) == solution.end())
            not_in_solution.push_back(i);

    vector<Move> moves;
    for (auto &[i, j] : out_neighborhood_nodes(solution, not_in_solution))
        moves.push_back({true, i, j});
    auto intra = edge_mode ? in_neighborhood_edges(solution) : in_neighborhood_nodes(solution);
    for (auto &[i, j] : intra)
    {
        if (edge_mode && j == i + 1) continue;
        moves.push_back({false, i, j});
    }

    while (true)
    {
        bool improved = false;
        for (size_t k = 0; k < moves.size(); ++k)
        {
            swap(moves[k], moves[k + rng()() % (moves.size() - k)]);
            const Move m = moves[k];
            int delta;
            if (m.inter)
                delta = delta_out(solution, not_in_solution, m.i, m.j, dist_matrix, points);
            else if (edge_mode)
                delta = delta_in_edges(solution, m.i, m.j, dist_matrix);
            else
                delta = delta_in_nodes(solution, m.i, m.j, dist_matrix);

            if (delta < 0)
            {
                if (m.inter)
                    swap(solution[m.i], not_in_solution[m.j]);
                else if (edge_mode)
                    reverse(solution.begin() + m.i, solution.begin() + m.j);
                else
                    swap(solution[m.i], solution[m.j]);
                improved = true;
                break;
            }
        }
        if (!improved) break;
    }
    return solution;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }
    string filename = argv[1];
    vector<Point> points = load_points(filename);
    if (points.empty())
    {
        cerr << "No points loaded" << endl;
        return 1;
    }
    vector<vector<int>> dist_matrix = calculate_distance_matrix(points);

    // best greedy heuristic per instance, chosen once
    bool is_A = filename.find("TSPA") != string::npos;
    auto greedy_start = [&](int i) {
        if (is_A)
            return tsp_cycle_weighted_regret_heuristic(points, dist_matrix, i);
        return tsp_nn_any_pos_weighted_regret_heuristic(points, dist_matrix, i);
    };

    for (string search : {"steepest", "greedy"})
    {
        for (string in_neighborhood : {"node", "edge"})
        {
            for (string start : {"random", "greedy"})
            {
                vector<int> costs;
                vector<double> times;
                vector<int> best_solution;
                int best_cost = INT_INFINITY;

                for (int i = 0; i < 200; ++i)
                {
                    vector<int> init = (start == "random") ? tsp_random_heuristic(points) : greedy_start(i);

                    auto t = chrono::steady_clock::now();
                    vector<int> solution;
                    if (search == "steepest")
                        solution = local_search_steepest(init, in_neighborhood, dist_matrix, points);
                    else
                        solution = local_search_greedy(init, in_neighborhood, dist_matrix, points);
                    times.push_back(elapsed_ms(t));

                    int cost = tsp_verify(solution, dist_matrix, points);
                    costs.push_back(cost);
                    if (cost < best_cost)
                    {
                        best_cost = cost;
                        best_solution = solution;
                    }
                }
                print_stats(search + " LS, " + in_neighborhood + ", " + start + " start", costs, best_solution, best_cost, times);
            }
        }
    }
    return 0;
}