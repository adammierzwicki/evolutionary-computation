
#include "greedy_heuristics.h"

using namespace std;

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
