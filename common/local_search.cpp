#include <algorithm>
#include "local_search.h"

using namespace std;

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
    int best_cost = tsp_calculate_costs(best_solution, dist_matrix, points);
    
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
