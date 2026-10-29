#pragma once
#include "helpers.h"
#include "regret_heuristics.h"

using namespace std;

vector<int> tsp_nn_any_pos_regret_heuristic(const vector<Point> &points, const vector<vector<int>> &dist_matrix, int start_idx);
vector<int> tsp_cycle_regret_heuristic(const vector<Point> &points, const vector<vector<int>> &dist_matrix, int start_idx);
vector<int> tsp_nn_any_pos_weighted_regret_heuristic(const vector<Point> &points, const vector<vector<int>> &dist_matrix, int start_idx, double w_regret, double w_obj);
vector<int> tsp_cycle_weighted_regret_heuristic(const vector<Point> &points, const vector<vector<int>> &dist_matrix, int start_idx, double w_regret, double w_obj);