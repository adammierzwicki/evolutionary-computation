#pragma once

#include "helpers.h"
#include <vector>
#include <tuple>

std::tuple<std::vector<int>, std::vector<int>>
msls_steepest_list_moves(std::vector<Point>& points,
                         std::vector<std::vector<int>>& dist_matrix,
                         int iterations = 400);

std::vector<int> double_bridge(const std::vector<int>& solution);

std::vector<int> exchange_nodes(const std::vector<int>& solution,
                                const std::vector<Point>& points,
                                const std::vector<std::vector<int>>& dist_matrix,
                                int k = 5, int pool = 10);

std::vector<int> perturb_solution(const std::vector<int>& solution,
                                  const std::vector<Point>& points,
                                  const std::vector<std::vector<int>>& dist_matrix);
                                  
std::tuple<std::vector<int>, int, std::vector<int>>
ils_steepest_list_moves(std::vector<Point>& points,
                        std::vector<std::vector<int>>& dist_matrix,
                        double max_time_ms);