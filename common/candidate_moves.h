#pragma once

#include "helpers.h"

// for each node find n closest neighbors (dist + cost of neighbor)
// output: [node 0: closest neighbors, node 1: closest neighbors, ...]
vector<vector<int>> candidate_moves(const vector<Point> &points, const vector<vector<int>> &dist_matrix, int n = 10);

// reverse cyclic segment solution[from..to] in place, keep pos[] in sync
void apply_reverse(vector<int> &solution, vector<int> &pos, int from, int to);

// steepest local search (2-opt edges + node exchange) restricted to candidate moves
vector<int> steepest_local_search_moves(vector<int> solution, const vector<Point> &points,
                                        const vector<vector<int>> &dist_matrix,
                                        const vector<vector<int>> &candidates);
