#pragma once
#include <vector>
#include <string>
#include <tuple>
#include <random>
#include <limits>
#include <chrono>
#include <ostream>

using namespace std;

const int INT_INFINITY = numeric_limits<int>::max();
const double DOUBLE_INFINITY = numeric_limits<double>::infinity();

struct Point
{
    int x;
    int y;
    int cost;
};

mt19937 &rng();

ostream &operator<<(ostream &os, const vector<int> &v);

vector<Point> load_points(const string &filename);
int euclidean_distance(const Point &p1, const Point &p2);
vector<vector<int>> calculate_distance_matrix(const vector<Point> &points);

int tsp_calculate_costs(const vector<int> &solution, const vector<vector<int>> &dist_matrix, const vector<Point> &points);

int random_start_idx(const vector<Point> &points);
double elapsed_ms(chrono::steady_clock::time_point start);
void print_stats(const string &name, const vector<int> &costs, const vector<int> &best_solution, int best_cost, const vector<double> &times_ms);
