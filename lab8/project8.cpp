#include "../common/helpers.h"
#include "../common/greedy_heuristics.h"
#include "../common/local_search.h"
#include "../common/ils.h"
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;

static void write_row(ostream &out, const string &type, int cost, const vector<int> &sol)
{
    out << type << ',' << cost << ',';
    for (size_t i = 0; i < sol.size(); ++i)
        out << (i ? " " : "") << sol[i];
    out << '\n';
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cerr << "Usage: " << argv[0] << " <instance.csv> <output.csv> [n_optima] [ils_runs] [ils_time_ms]" << endl;
        return 1;
    }
    vector<Point> points = load_points(argv[1]);
    if (points.empty())
    {
        cerr << "No points loaded" << endl;
        return 1;
    }
    vector<vector<int>> dist_matrix = calculate_distance_matrix(points);
    int n_optima = argc >= 4 ? atoi(argv[3]) : 1000;
    int ils_runs = argc >= 5 ? atoi(argv[4]) : 10;
    double ils_time_ms = argc >= 6 ? atof(argv[5]) : 840;

    filesystem::path out_path(argv[2]);
    if (out_path.has_parent_path())
        filesystem::create_directories(out_path.parent_path());
    ofstream out(out_path);
    if (!out)
    {
        cerr << "Cannot open output file: " << argv[2] << endl;
        return 1;
    }
    out << "type,cost,nodes\n";

    // 1000 random local optima from greedy local search (edge neighborhood)
    for (int i = 0; i < n_optima; ++i)
    {
        vector<int> sol = local_search_greedy(tsp_random_heuristic(points), "edge", dist_matrix, points);
        write_row(out, "local_optimum", tsp_calculate_costs(sol, dist_matrix, points), sol);
    }

    // very good reference solution: best of several ILS runs
    vector<int> best;
    int best_cost = INT_INFINITY;
    for (int i = 0; i < ils_runs; ++i)
    {
        auto [sol, calls, evals] = ils_steepest_list_moves(points, dist_matrix, ils_time_ms);
        int cost = tsp_calculate_costs(sol, dist_matrix, points);
        cerr << "ILS run " << i << ": " << cost << endl;
        if (cost < best_cost)
        {
            best_cost = cost;
            best = sol;
        }
    }
    write_row(out, "ils_best", best_cost, best);
    return 0;
}
