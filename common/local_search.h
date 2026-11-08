#pragma once

#include "helpers.h"
#include <utility>

vector<pair<int, int>> in_neighborhood_nodes(vector<int> solution);
vector<pair<int, int>> out_neighborhood_nodes(vector<int> solution, vector<int> not_in_solution);
vector<pair<int, int>> in_neighborhood_edges(vector<int> solution);

int delta_in_nodes(const vector<int> &solution, int i, int j, const vector<vector<int>> &dist_matrix);
int delta_in_edges(const vector<int> &solution, int i, int j, const vector<vector<int>> &dist_matrix);
int delta_out(const vector<int> &solution, const vector<int> &not_in_solution, int i, int j, const vector<vector<int>> &dist_matrix, const vector<Point> &points);

pair<vector<int>, int> in_neighborhood_edges_evaluate(vector<int> solution, int cost, const vector<vector<int>> &dist_matrix);
pair<vector<int>, int> in_neighborhood_nodes_evaluate(vector<int> solution, int cost, const vector<vector<int>> &dist_matrix);
tuple<vector<int>, int, vector<int>> out_neighborhood_evaluate(vector<int> solution, vector<int> not_in_solution, int cost, const vector<vector<int>> &dist_matrix, const vector<Point> &points);

tuple<vector<int>, int> local_search_steepest(vector<int> init_solution, string in_neighborhood, const vector<vector<int>> &dist_matrix, const vector<Point> &points);
vector<int> local_search_greedy(vector<int> solution, string in_neighborhood, const vector<vector<int>> &dist_matrix, const vector<Point> &points);