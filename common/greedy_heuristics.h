#pragma once
#include "helpers.h"

tuple<int, int> find_nn(const int point_idx, const vector<vector<int>> &dist_matrix, const vector<Point> &points, const vector<bool> &visited);
tuple<int, int> find_nn_any_pos(int p1_idx, int p2_idx, const vector<vector<int>> &dist_matrix, const vector<Point> &points, const vector<bool> &visited);

vector<int> tsp_random_heuristic(const vector<Point> &points);
vector<int> tsp_nn_heuristic(const vector<Point> &points, const vector<vector<int>> &dist_matrix, int start_idx = -1);
vector<int> tsp_nn_any_pos_heuristic(const vector<Point> &points, const vector<vector<int>> &dist_matrix, int start_idx = -1);
vector<int> tsp_cycle_heuristic(const vector<Point> &points, const vector<vector<int>> &dist_matrix, int start_idx = -1);
