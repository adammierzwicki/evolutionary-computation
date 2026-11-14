#pragma once

#include "helpers.h"

// steepest local search (2-opt edges + node exchange) reusing deltas from previous iterations
// returns (solution, number of full neighborhood evaluations)
tuple<vector<int>, int> steepest_ls_with_prev_deltas(vector<int> init_solution, const vector<vector<int>> &dist_matrix, const vector<Point> &points);
