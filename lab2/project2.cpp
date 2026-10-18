// g++ -O2 project2.cpp -o project2

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

int random_start_idx(const vector<Point> &points)
{
    uniform_int_distribution<> dis(0, points.size() - 1);
    return dis(rng());
}

vector<int> tsp_nn_any_pos_regret_heuristic(const vector<Point> &points, const vector<vector<int>> &dist_matrix, int start_idx = -1)
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
        int max_regret = -1;
        int max_best_val = INT_INFINITY;
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
            if (regret > max_regret || (regret == max_regret && best_val < max_best_val))
            {
                max_regret = regret;
                max_best_val = best_val;
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

vector<int> tsp_cycle_regret_heuristic(const vector<Point> &points, const vector<vector<int>> &dist_matrix, int start_idx = -1)
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
        int max_regret = -1;
        int max_best_val = INT_INFINITY;
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
            if (regret > max_regret || (regret == max_regret && best_val < max_best_val))
            {
                max_regret = regret;
                max_best_val = best_val;
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

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }
    vector<Point> points = load_points(argv[1]);
    if (points.empty())
    {
        cerr << "No points loaded" << endl;
        return 1;
    }
    vector<vector<int>> dist_matrix = calculate_distance_matrix(points);

    // NN any pos 2-regret heuristic
    vector<int> nn_any_pos_costs;
    int nn_any_pos_best_cost = INT_INFINITY;
    vector<int> nn_any_pos_best_solution;
    vector<double> nn_any_pos_times;
    for (int i = 0; i < 200; ++i)
    {
        auto start = chrono::steady_clock::now();
        vector<int> nn_any_pos_solution = tsp_nn_any_pos_regret_heuristic(points, dist_matrix, i);
        nn_any_pos_times.push_back(elapsed_ms(start));
        int cost = tsp_verify(nn_any_pos_solution, dist_matrix, points);
        nn_any_pos_costs.push_back(cost);
        if (cost < nn_any_pos_best_cost)
        {
            nn_any_pos_best_cost = cost;
            nn_any_pos_best_solution = nn_any_pos_solution;
        }
    }
    print_stats("NN any position 2-regret", nn_any_pos_costs, nn_any_pos_best_solution, nn_any_pos_best_cost, nn_any_pos_times);

    // Cycle 2-regret heuristic
    vector<int> cycle_costs;
    int cycle_best_cost = INT_INFINITY;
    vector<int> cycle_best_solution;
    vector<double> cycle_times;
    for (int i = 0; i < 200; ++i)
    {
        auto start = chrono::steady_clock::now();
        vector<int> cycle_solution = tsp_cycle_regret_heuristic(points, dist_matrix, i);
        cycle_times.push_back(elapsed_ms(start));
        int cost = tsp_verify(cycle_solution, dist_matrix, points);
        cycle_costs.push_back(cost);
        if (cost < cycle_best_cost)
        {
            cycle_best_cost = cost;
            cycle_best_solution = cycle_solution;
        }
    }
    print_stats("Greedy cycle 2-regret", cycle_costs, cycle_best_solution, cycle_best_cost, cycle_times);

    // NN any pos weighted 2-regret heuristic
    vector<int> nn_any_pos_weighted_costs;
    int nn_any_pos_weighted_best_cost = INT_INFINITY;
    vector<int> nn_any_pos_weighted_best_solution;
    vector<double> nn_any_pos_weighted_times;
    for (int i = 0; i < 200; ++i)
    {
        auto start = chrono::steady_clock::now();
        vector<int> nn_any_pos_weighted_solution = tsp_nn_any_pos_weighted_regret_heuristic(points, dist_matrix, i);
        nn_any_pos_weighted_times.push_back(elapsed_ms(start));
        int cost = tsp_verify(nn_any_pos_weighted_solution, dist_matrix, points);
        nn_any_pos_weighted_costs.push_back(cost);
        if (cost < nn_any_pos_weighted_best_cost)
        {
            nn_any_pos_weighted_best_cost = cost;
            nn_any_pos_weighted_best_solution = nn_any_pos_weighted_solution;
        }
    }
    print_stats("NN any position weighted 2-regret", nn_any_pos_weighted_costs, nn_any_pos_weighted_best_solution, nn_any_pos_weighted_best_cost, nn_any_pos_weighted_times);

    // Cycle weighted 2-regret heuristic
    vector<int> cycle_weighted_costs;
    int cycle_weighted_best_cost = INT_INFINITY;
    vector<int> cycle_weighted_best_solution;
    vector<double> cycle_weighted_times;
    for (int i = 0; i < 200; ++i)
    {
        auto start = chrono::steady_clock::now();
        vector<int> cycle_weighted_solution = tsp_cycle_weighted_regret_heuristic(points, dist_matrix, i);
        cycle_weighted_times.push_back(elapsed_ms(start));
        int cost = tsp_verify(cycle_weighted_solution, dist_matrix, points);
        cycle_weighted_costs.push_back(cost);
        if (cost < cycle_weighted_best_cost)
        {
            cycle_weighted_best_cost = cost;
            cycle_weighted_best_solution = cycle_weighted_solution;
        }
    }
    print_stats("Greedy cycle weighted 2-regret", cycle_weighted_costs, cycle_weighted_best_solution, cycle_weighted_best_cost, cycle_weighted_times);

    return 0;
}