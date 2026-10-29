#include "helpers.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>
#include <algorithm>

using namespace std;

mt19937 &rng()
{
    static mt19937 gen(random_device{}());
    return gen;
}

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
