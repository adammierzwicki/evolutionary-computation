from dataclasses import dataclass
import matplotlib.pyplot as plt
import math
import random
import sys


@dataclass
class Point:
    x: int
    y: int
    cost: int


def load_points(file_name) -> list[Point]:
    points = []
    with open(file_name, "r") as f:
        for line in f:
            x, y, cost = map(int, line.strip().split(";"))
            points.append(Point(x, y, cost))
    return points


def euclidean_dist(p1: Point, p2: Point) -> int:
    return int(((p1.x - p2.x) ** 2 + (p1.y - p2.y) ** 2) ** 0.5 + 0.5)


def calculate_dist_matrix(points):
    m = [[0 for _ in range(len(points))] for _ in range(len(points))]
    for i in range(len(points)):
        for j in range(len(points)):
            m[i][j] = euclidean_dist(points[i], points[j])

    return m


def tsp_calculate_costs(solution, dist_matrix, points):
    dist_cost = 0
    node_cost = 0
    for i in range(len(solution)):
        node_cost += points[solution[i]].cost
        dist_cost += dist_matrix[solution[i]][solution[(i + 1) % len(solution)]]

    return node_cost, dist_cost, node_cost + dist_cost


def tsp_verify(solution, dist_matrix, points):
    # print("Solution len:", len(solution))
    # print("Points count:", len(points))
    # print("Solution", solution)

    node_cost, dist_cost, total_cost = tsp_calculate_costs(
        solution, dist_matrix, points
    )
    # print("Node cost", node_cost)
    # print("Distance cost", dist_cost)
    # print("Total cost", total_cost)
    return total_cost



def find_nn(point_idx, dist_matrix, points, idxs_left):
    min_idx = None
    min_val = float("inf")
    for i in idxs_left:
        dist = dist_matrix[point_idx][i]
        total = dist + points[i].cost
        if total < min_val:
            min_val = total
            min_idx = i

    return min_idx, min_val


def find_nn_any_pos(p1_idx, p2_idx, dist_matrix, points, idxs_left):
    min_idx = None
    min_val = float("inf")
    for i in idxs_left:
        candidate = points[i]
        dist = dist_matrix[p1_idx][i] + dist_matrix[p2_idx][i]
        total = dist + candidate.cost - dist_matrix[p1_idx][p2_idx]
        if total < min_val:
            min_val = total
            min_idx = i
    return min_idx, min_val


def tsp_nn_heuristic(points, dist_matrix, start_idx=None) -> list[int]:
    idxs = list(range(len(points)))
    if start_idx is None:
        start_idx = random.choice(idxs)

    solution = [start_idx]
    idxs.remove(start_idx)
    while len(solution) < math.ceil(len(points) / 2):
        next_idx, _ = find_nn(solution[-1], dist_matrix, points, idxs)
        assert next_idx is not None
        solution.append(next_idx)
        idxs.remove(next_idx)

    return solution


def tsp_random_heuristic(points, dist_matrix) -> list[int]:
    solution = []
    idxs = list(range(len(points)))
    while len(solution) < math.ceil(len(points) / 2):
        i = random.choice(idxs)
        solution.append(i)
        idxs.remove(i)
    return solution


def tsp_nn_any_pos_heuristic(points, dist_matrix, start_idx=None) -> list[int]:
    idxs = list(range(len(points)))
    if start_idx is None:
        start_idx = random.choice(idxs)

    solution = [start_idx]
    idxs.remove(start_idx)
    while len(solution) < math.ceil(len(points) / 2):
        min_idx = None
        min_val = float("inf")
        min_insert_pos = None
        for insert_pos in range(len(solution) + 1):
            if insert_pos == 0:
                next_idx, val = find_nn(solution[0], dist_matrix, points, idxs)
            elif insert_pos == len(solution):
                next_idx, val = find_nn(solution[-1], dist_matrix, points, idxs)
            else:
                next_idx, val = find_nn_any_pos(
                    solution[insert_pos - 1],
                    solution[insert_pos],
                    dist_matrix,
                    points,
                    idxs,
                )

            if val < min_val:
                min_val = val
                min_idx = next_idx
                min_insert_pos = insert_pos
        assert min_idx is not None and min_insert_pos is not None
        solution.insert(min_insert_pos, min_idx)
        idxs.remove(min_idx)

    return solution


def tsp_cycle_heuristic(points, dist_matrix, start_idx=None) -> list[int]:
    idxs = list(range(len(points)))
    if start_idx is None:
        start_idx = random.choice(idxs)

    solution = [start_idx]
    idxs.remove(start_idx)

    next_idx, _ = find_nn(solution[-1], dist_matrix, points, idxs)
    assert next_idx is not None
    solution.append(next_idx)
    idxs.remove(next_idx)
    while len(solution) < math.ceil(len(points) / 2):
        min_idx = None
        min_val = float("inf")
        min_insert_pos = None
        for insert_pos in range(len(solution)):
            next_idx, val = find_nn_any_pos(
                solution[insert_pos - 1],
                solution[insert_pos],
                dist_matrix,
                points,
                idxs,
            )

            if val < min_val:
                min_val = val
                min_idx = next_idx
                min_insert_pos = insert_pos
        assert min_idx is not None and min_insert_pos is not None
        solution.insert(min_insert_pos, min_idx)
        idxs.remove(min_idx)

    return solution


def visualize_solution(points, solution, cost, title="Solution"):
    if not points or not solution:
        return

    solution_set = set(solution)
    remaining = [i for i in range(len(points)) if i not in solution_set]

    max_cost = max(p.cost for p in points) if points else 1

    def marker_size(node_cost: int, min_size: int = 25, max_size: int = 1200) -> float:
        if max_cost <= 0:
            return max_size
        relative = node_cost / max_cost
        return min_size + (relative ** 0.5) * (max_size - min_size)

    x_solution = [points[i].x for i in solution] + [points[solution[0]].x]
    y_solution = [points[i].y for i in solution] + [points[solution[0]].y]

    x_remaining = [points[i].x for i in remaining]
    y_remaining = [points[i].y for i in remaining]

    plt.figure(figsize=(8, 6))

    if remaining:
        plt.scatter(
            x_remaining,
            y_remaining,
            s=[marker_size(points[i].cost) for i in remaining],
            color="lightgray",
            alpha=0.5,
            zorder=0,
        )

    plt.plot(
        x_solution,
        y_solution,
        marker="o",
        color="blue",
        linewidth=2,
        markersize=4,
        zorder=2,
    )

    plt.scatter(
        [points[i].x for i in solution],
        [points[i].y for i in solution],
        s=[marker_size(points[i].cost) for i in solution],
        color="royalblue",
        edgecolors="black",
        linewidths=0.5,
        alpha=0.7,
        zorder=3,
    )

    plt.title(f"{title} (Cost: {cost})")
    plt.xlabel("X")
    plt.ylabel("Y")
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(f"{title.replace(' ', '_')}.png", dpi=200)
    plt.close()


def main():
    if len(sys.argv) < 2:
        print("Missing input file")
        return

    points: list[Point] = load_points(sys.argv[1])
    dist_matrix = calculate_dist_matrix(points)

    # random solutions
    random_costs = []
    random_best_cost = float("inf")
    random_best_solution = None
    for i in range(200):
        solution = tsp_random_heuristic(points, dist_matrix)
        cost = tsp_verify(solution, dist_matrix, points)
        random_costs.append(cost)
        if cost < random_best_cost:
            random_best_cost = cost
            random_best_solution = solution
    print("Random method costs: min:", min(random_costs), "max:", max(random_costs), "avg:", sum(random_costs) / len(random_costs))
    print("Random best solution:", random_best_solution, "cost:", random_best_cost)
    visualize_solution(points, random_best_solution, random_best_cost, "Random Best Solution")

    # nn end node solutions
    nn_costs = []
    nn_best_cost = float("inf")
    nn_best_solution = None
    for i in range(200):
        solution = tsp_nn_heuristic(points, dist_matrix, i)
        cost = tsp_verify(solution, dist_matrix, points)
        nn_costs.append(cost)
        if cost < nn_best_cost:
            nn_best_cost = cost
            nn_best_solution = solution
    print("NN end node method costs: min:", min(nn_costs), "max:", max(nn_costs), "avg:", sum(nn_costs) / len(nn_costs))
    print("NN best solution:", nn_best_solution, "cost:", nn_best_cost)
    visualize_solution(points, nn_best_solution, nn_best_cost, "NN Best Solution")

    # nn any position solutions
    nn_any_pos_costs = []
    nn_any_best_cost = float("inf")
    nn_any_best_solution = None
    for i in range(200):
        solution = tsp_nn_any_pos_heuristic(points, dist_matrix, i)
        cost = tsp_verify(solution, dist_matrix, points)
        nn_any_pos_costs.append(cost)
        if cost < nn_any_best_cost:
            nn_any_best_cost = cost
            nn_any_best_solution = solution
    print("NN any position method costs: min:", min(nn_any_pos_costs), "max:", max(nn_any_pos_costs), "avg:", sum(nn_any_pos_costs) / len(nn_any_pos_costs))
    print("NN any position best solution:", nn_any_best_solution, "cost:", nn_any_best_cost)
    visualize_solution(points, nn_any_best_solution, nn_any_best_cost, "NN Any Position Best Solution")

    # greedy cycle solutions
    cycle_costs = []
    cycle_best_cost = float("inf")
    cycle_best_solution = None
    for i in range(200):
        solution = tsp_cycle_heuristic(points, dist_matrix, i)
        cost = tsp_verify(solution, dist_matrix, points)
        cycle_costs.append(cost)
        if cost < cycle_best_cost:
            cycle_best_cost = cost
            cycle_best_solution = solution
    print("Greedy cycle method costs: min:", min(cycle_costs), "max:", max(cycle_costs), "avg:", sum(cycle_costs) / len(cycle_costs))
    print("Greedy cycle best solution:", cycle_best_solution, "cost:", cycle_best_cost)
    visualize_solution(points, cycle_best_solution, cycle_best_cost, "Greedy Cycle Best Solution")


if __name__ == "__main__":
    main()