import csv
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

ROOT = Path(__file__).parent
DATA, IMG = ROOT / "data", ROOT / "img"
IMG.mkdir(exist_ok=True)


def load(path):
    optima, ils = [], None
    with open(path) as f:
        for row in csv.DictReader(f):
            sol = [int(x) for x in row["nodes"].split()]
            if row["type"] == "local_optimum":
                optima.append((int(row["cost"]), sol))
            else:
                ils = (int(row["cost"]), sol)
    return optima, ils


def node_set(sol):
    return set(sol)

def edge_set(sol):
    return {frozenset((sol[i], sol[(i + 1) % len(sol)])) for i in range(len(sol))}

def similarity_matrix(sets):
    k = len(sets)
    sim = np.zeros((k, k))
    for i in range(k):
        for j in range(k):
            sim[i, j] = len(sets[i] & sets[j])
    return sim

def plot(x, y, title, ylabel, r, path):
    fig, ax = plt.subplots(figsize=(6.5, 4.5))
    ax.scatter(x, y, s=8, alpha=0.5)
    ax.set_xlabel("Objective function value")
    ax.set_ylabel(ylabel)
    ax.set_title(f"{title}\nr = {r:.3f}")
    ax.grid(alpha=0.3)
    fig.tight_layout()
    fig.savefig(path, dpi=150)
    plt.close(fig)


rows = []
for inst in ("A", "B"):
    optima, ils = load(DATA / f"TSP{inst}_optima.csv")
    costs = np.array([c for c, _ in optima], dtype=float)
    sols = [s for _, s in optima]
    best_idx = int(np.argmin(costs))

    for measure, to_set in (("common edges", edge_set), ("common nodes", node_set)):
        sets = [to_set(s) for s in sols]
        sim = similarity_matrix(sets)
        ils_set = to_set(ils[1])
        sim_ils = np.array([len(s & ils_set) for s in sets])

        # average similarity to all other local optima
        avg = (sim.sum(axis=1) - np.diag(sim)) / (len(sols) - 1)
        # similarity to best local optimum, excluding the best itself
        mask = np.arange(len(sols)) != best_idx
        versions = [
            ("average similarity to other local optima", "avg", costs, avg),
            (f"similarity to best of {len(sols)} local optima", "best", costs[mask], sim[best_idx][mask]),
            (f"similarity to ILS solution (cost {ils[0]})", "ils", costs, sim_ils),
        ]
        for label, key, x, y in versions:
            r = float(np.corrcoef(x, y)[0, 1])
            tag = measure.replace(" ", "_")
            plot(x, y, f"Instance {inst}: {label}", f"{measure} ({'avg ' if key == 'avg' else ''}count)", r, IMG / f"{inst}_{key}_{tag}.png")
            rows.append((inst, measure, label, r))

with open(ROOT / "correlations.csv", "w", newline="") as f:
    w = csv.writer(f)
    w.writerow(["instance", "measure", "reference", "pearson_r"])
    w.writerows((a, b, c, f"{r:.4f}") for a, b, c, r in rows)
for a, b, c, r in rows:
    print(f"{a} | {b:13} | {c:48} | r = {r:+.3f}")
