#include "greedy_heuristics.h"
#include <cmath>
#include <numeric>
#include <algorithm>

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

tuple<int, int> find_nn_any_pos(int p1_idx, int p2_idx, const vector<vector<int>> &dist_matrix, const vector<Point> &points, const vector<bool> &visited)
{
    int min_idx = -1;
    int min_val = INT_INFINITY;
    for (size_t i = 0; i < points.size(); ++i)
    {
        if (visited[i])
        {
            continue;
        }
        int dist = dist_matrix[p1_idx][i] + dist_matrix[p2_idx][i];
        int total = points[i].cost + dist - dist_matrix[p1_idx][p2_idx];
        if (total < min_val)
        {
            min_val = total;
            min_idx = i;
        }
    }
    return {min_idx, min_val};
}

vector<int> tsp_random_heuristic(const vector<Point> &points)
{
    vector<int> idxs(points.size());
    iota(idxs.begin(), idxs.end(), 0);
    shuffle(idxs.begin(), idxs.end(), rng());
    idxs.resize(ceil(points.size() / 2.0));
    return idxs;
}

vector<int> tsp_nn_heuristic(const vector<Point> &points, const vector<vector<int>> &dist_matrix, int start_idx)
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
        auto [next_idx, next_val] = find_nn(solution.back(), dist_matrix, points, visited);
        if (next_idx == -1)
        {
            break;
        }
        solution.push_back(next_idx);
        visited[next_idx] = true;
    }
    return solution;
}

vector<int> tsp_nn_any_pos_heuristic(const vector<Point> &points, const vector<vector<int>> &dist_matrix, int start_idx)
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
        int min_idx = -1;
        int min_val = INT_INFINITY;
        int min_insert_pos = -1;
        int next_idx, next_val;
        for (size_t insert_pos = 0; insert_pos <= solution.size(); ++insert_pos)
        {
            if (insert_pos == 0)
            {
                tie(next_idx, next_val) = find_nn(solution.front(), dist_matrix, points, visited);
            }
            else if (insert_pos == solution.size())
            {
                tie(next_idx, next_val) = find_nn(solution.back(), dist_matrix, points, visited);
            }
            else
            {
                tie(next_idx, next_val) = find_nn_any_pos(solution[insert_pos - 1], solution[insert_pos], dist_matrix, points, visited);
            }

            if (next_val < min_val)
            {
                min_val = next_val;
                min_idx = next_idx;
                min_insert_pos = insert_pos;
            }
        }
        if (min_idx == -1)
        {
            break;
        }
        solution.insert(solution.begin() + min_insert_pos, min_idx);
        visited[min_idx] = true;
    }
    return solution;
}

vector<int> tsp_cycle_heuristic(const vector<Point> &points, const vector<vector<int>> &dist_matrix, int start_idx)
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
        int min_idx = -1;
        int min_val = INT_INFINITY;
        int min_insert_pos = -1;
        for (size_t insert_pos = 0; insert_pos < solution.size(); ++insert_pos)
        {
            size_t prev_pos = (insert_pos + solution.size() - 1) % solution.size();
            auto [next_idx, next_val] = find_nn_any_pos(solution[prev_pos], solution[insert_pos], dist_matrix, points, visited);

            if (next_val < min_val)
            {
                min_val = next_val;
                min_idx = next_idx;
                min_insert_pos = insert_pos;
            }
        }
        if (min_idx == -1)
        {
            break;
        }
        solution.insert(solution.begin() + min_insert_pos, min_idx);
        visited[min_idx] = true;
    }
    return solution;
}