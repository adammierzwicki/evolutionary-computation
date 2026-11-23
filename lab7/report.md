# Large neighborhood search
Adam Mierzwicki 160318, Marcin Perek 160298
## Table of contents
1. Problem description
2. Pseudocode 
3. Results
4. Best solutions
5. Best solutions - node indices
6. Information whether the best solutions have been checked with the solution checker.
7. (Link to) the source code
8. Conclusions

## 1. Problem description
We are given three columns of integers with a row for each node. The first two columns contain x and y coordinates of the node positions in a plane. The third column contains node costs. The goal is to select exactly 50% of the nodes (if the number of nodes is odd we round the number of nodes to be selected up) and form a Hamiltonian cycle (closed path) through this set of nodes such that the sum of the total length of the path plus the total cost of the selected nodes is minimized. The distances between nodes are calculated as Euclidean distances rounded mathematically to integer values. 

We perform 8 experiments on large neighborhood search algorithms testing 4 different destroy operators and the need for local search after repairing.

### Destroy operators
- removal of randomly selected nodes - shuffle indices of nodes in the solution and remove nodes corresponding to the first 30% of indices
- removal of single random subpath - calculate the length of the subpath to remove so that 30% of the nodes are removed, pick a random starting point and remove that subpath
- removal of 3-5 randomly selected subpaths - randomly choose `k` subpaths to be removed (between 3 and 5), for each subpath calculate its length so that in summary 30% of the nodes are removed, pick a random starting point and remove that subpath, repeat `k` times
- removal of nodes with probability proportional to objective function - for each node in the cycle calculate its weight as the increase of objective function when rerouting through that node:
  $$w_i=dist(v_{i-1},v_i)+dist(v_i,v_{i+1})-dist(v_{i-1},v_{i+1})+cost(v_i)+1$$
  then select random nodes with probability $w_i/\sum w$ until 30% of the nodes are removed.

<div style="page-break-after: always;"></div>

### Repair operator
For repair operation we use weighted greedy cycle regret heuristic, which performed the best among greedy heuristics from previous laboratories. It iteratively expands the solution by inserting nodes into the current cycle. At each step, it evaluates the cost of inserting each candidate node at every possible position in the cycle and selects the node with the highest regret value, which is the difference between the best and second-best insertion costs.

### Improving with local search
After applying destroy and repair operators we can optionally perform a local search run to further improve the solution. We perform 2 tests for each destroy operator:
- using local search after repair
- without using local search after repair

For the local search algorithm we use the steepest local search with move evaluations from previous iterations (list of moves).

The initial solution for the LNS is a random solution.


<div style="page-break-after: always;"></div>

## 2. Pseudocode

### Large neighborhood search
```
init_solution = random_solution()
best_solution = steepest_ls_with_prev_deltas(init_solution)
best_cost = cost(best_solution)
target_size = len(best_solution)
iterations = 0

while running_time < average_time_of_one_msls_run  // 840ms
    solution = destroy(best_solution)
    solution = repair(solution, target_size)
    if use_ls
        solution = steepest_ls_with_prev_deltas(solution)
    if cost(solution) < best_cost
        update best_cost and best_solution
    iterations += 1

return best_solution
```

### Destroy: random nodes
```
n = len(solution)
m = round(0.3 * n)
choose m distinct random positions in solution
return solution without the nodes at chosen positions
```

<div style="page-break-after: always;"></div>

### Destroy: single subpath / multiple subpaths
```
n = len(solution)
m = round(0.3 * n)
if single subpath
    k = 1
else
    k = random integer from {3, 4, 5}

split m into k random positive subpath lengths l_1, ..., l_k so that l_1 + ... + l_k = m

partial_solution = copy of solution
for each length l in l_1, ..., l_k
    start = random position in partial_solution
    remove l consecutive nodes from partial_solution, beginning at start

return partial_solution
```

### Destroy: weighted nodes
```
n = len(solution)
m = round(0.3 * n)
for each position i in solution
    v = solution[i], prev = node before v, next = node after v
    w[i] = 1 + dist[prev][v] + dist[v][next] - dist[prev][next] + cost[v]

repeat m times
    choose a random position i with probability w[i] / sum(w)
    mark position i as removed
    w[i] = 0

return solution without the marked nodes
```

<div style="page-break-after: always;"></div>

### Repair: greedy cycle weighted 2-regret
```
remaining nodes = all nodes not in the partial solution

repeat
    for node in remaining nodes
        initialize best cost and second best cost to infinity
        for position in insert positions in solution
            distance = distance from node before position to node + distance from node to node after position
                       - distance from node before position to node after position
            total cost = distance + node cost
            record and update best cost, second best cost and best position for this node
        regret = second best cost - best cost
        score = w_regret * regret - w_obj * best cost
    choose node with the highest score
    insert chosen node into its best position in solution
    delete chosen node from remaining nodes
until solution contains target_size nodes
```

<div style="page-break-after: always;"></div>

## 3. Results

### Comparison with the best earlier methods
<table>
    <thead>
        <tr>
            <th style="text-align:center">Method</th>
            <th style="text-align:center">Instance A</th>
            <th style="text-align:center">Instance B</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <th>Greedy cycle weighted 2-regret</th>
            <td>73349.6 (72011 - 75532)</td>
            <td>57751.4 (52906 - 63326)</td>
        </tr>
        <tr>
            <th>Steepest LS with previous deltas, random start</th>
            <td>76082.7 (73618 - 79726)</td>
            <td>52570.2 (49632 - 5570a1)</td>
        </tr>
        <tr>
            <th>MSLS</th>
            <td>73569.1 (73187 - 74095)</td>
            <td>50207.2 (49651 - 50547)</td>
        </tr>
        <tr>
            <th>ILS</th>
            <td><b>71683.4 (71497 - 72033)</b></td>
            <td><b>48455.1 (48157 - 49195)</b></td>
        </tr>
        <tr>
            <th>LNS, random nodes, with LS</th>
            <td>72182.9 (71641 - 73582)</td>
            <td>49153.3 (48260 - 50085)</td>
        </tr>
        <tr>
            <th>LNS, random nodes, w/o LS</th>
            <td>72381.3 (71790 - 73169)</td>
            <td>49117.6 (48311 - 50025)</td>
        </tr>
        <tr>
            <th>LNS, single subpath, with LS</th>
            <td>72224.9 (71835 - 72950)</td>
            <td>49455.5 (48304 - 51067)</td>
        </tr>
        <tr>
            <th>LNS, single subpath, w/o LS</th>
            <td>72310.1 (71870 - 73245)</td>
            <td>49695.9 (48972 - 50721)</td>
        </tr>
        <tr>
            <th>LNS, multiple subpaths, with LS</th>
            <td>71755.6 (71649 - 72357)</td>
            <td>48889.6 (48253 - 49995)</td>
        </tr>
        <tr>
            <th>LNS, multiple subpaths, w/o LS</th>
            <td>71790.8 (71683 - 71950)</td>
            <td>49016.6 (48412 - 50230)</td>
        </tr>
        <tr>
            <th>LNS, weighted nodes, with LS</th>
            <td>72413.8 (71760 - 73163)</td>
            <td>49028.4 (48214 - 49713)</td>
        </tr>
        <tr>
            <th>LNS, weighted nodes, w/o LS</th>
            <td>72494.9 (71802 - 73232)</td>
            <td>49469.3 (48790 - 50169)</td>
        </tr>
    </tbody>
</table>

<div style="page-break-after: always;"></div>

### Running time comparison [ms]
<table>
    <thead>
        <tr>
            <th style="text-align:center">Method</th>
            <th style="text-align:center">Instance A (avg)</th>
            <th style="text-align:center">Instance B (avg)</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <th>Steepest LS with previous deltas, random start</th>
            <td>2.30</td>
            <td>2.47</td>
        </tr>
        <tr>
            <th>MSLS</th>
            <td>831.52</td>
            <td>812.60</td>
        </tr>
        <tr>
            <th>ILS</th>
            <td>840.19</td>
            <td>840.19</td>
        </tr>
        <tr>
            <th>LNS, random nodes, with LS</th>
            <td>840.362</td>
            <td>840.307</td>
        </tr>
        <tr>
            <th>LNS, random nodes, w/o LS</th>
            <td>840.34</td>
            <td>840.359</td>
        </tr>
        <tr>
            <th>LNS, single subpath, with LS</th>
            <td>840.462</td>
            <td>840.434</td>
        </tr>
        <tr>
            <th>LNS, single subpath, w/o LS</th>
            <td>840.332</td>
            <td>840.348</td>
        </tr>
        <tr>
            <th>LNS, multiple subpaths, with LS</th>
            <td>840.496</td>
            <td>840.408</td>
        </tr>
        <tr>
            <th>LNS, multiple subpaths, w/o LS</th>
            <td>840.26</td>
            <td>840.285</td>
        </tr>
        <tr>
            <th>LNS, weighted nodes, with LS</th>
            <td>840.263</td>
            <td>840.331</td>
        </tr>
        <tr>
            <th>LNS, weighted nodes, w/o LS</th>
            <td>840.311</td>
            <td>840.331</td>
        </tr>
    </tbody>
</table>

### Number of iterations
<table>
    <thead>
        <tr>
            <th rowspan="2" style="text-align:center">Method</th>
            <th colspan="3" style="text-align:center">Instance A</th>
            <th colspan="3" style="text-align:center">Instance B</th>
        </tr>
        <tr>
            <th style="text-align:center">min</th>
            <th style="text-align:center">avg</th>
            <th style="text-align:center">max</th>
            <th style="text-align:center">min</th>
            <th style="text-align:center">avg</th>
            <th style="text-align:center">max</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <th>LNS, random nodes, with LS</th>
            <td>1070</td><td>1139.9</td><td>1185</td>
            <td>987</td><td>1064.9</td><td>1146</td>
        </tr>
        <tr>
            <th>LNS, random nodes, w/o LS</th>
            <td>1297</td><td>1349.4</td><td>1378</td>
            <td>1279</td><td>1337.7</td><td>1377</td>
        </tr>
        <tr>
            <th>LNS, single subpath, with LS</th>
            <td>1031</td><td>1107.1</td><td>1154</td>
            <td>939</td><td>1015.85</td><td>1064</td>
        </tr>
        <tr>
            <th>LNS, single subpath, w/o LS</th>
            <td>1357</td><td>1367.15</td><td>1381</td>
            <td>1337</td><td>1364.5</td><td>1386</td>
        </tr>
        <tr>
            <th>LNS, multiple subpaths, with LS</th>
            <td>1089</td><td>1131</td><td>1153</td>
            <td>1023</td><td>1059.35</td><td>1096</td>
        </tr>
        <tr>
            <th>LNS, multiple subpaths, w/o LS</th>
            <td>1350</td><td>1359.4</td><td>1368</td>
            <td>1271</td><td>1320.95</td><td>1355</td>
        </tr>
        <tr>
            <th>LNS, weighted nodes, with LS</th>
            <td>1102</td><td>1142.15</td><td>1181</td>
            <td>952</td><td>1022.05</td><td>1077</td>
        </tr>
        <tr>
            <th>LNS, weighted nodes, w/o LS</th>
            <td>1307</td><td>1337.5</td><td>1363</td>
            <td>1252</td><td>1293</td><td>1335</td>
        </tr>
    </tbody>
</table>

<div style="page-break-after: always;"></div>

### Comparison of all methods
<table>
    <thead>
        <tr>
            <th style="text-align:center">Method</th>
            <th style="text-align:center">Instance A</th>
            <th style="text-align:center">Instance B</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <th>Random</th>
            <td>261052.4 (236589 - 292354)</td>
            <td>206729.0 (182493 - 234208)</td>
        </tr>
        <tr>
            <th>NN</th>
            <td>87476.6 (84362 - 90075)</td>
            <td>59397.3 (56007 - 63166)</td>
        </tr>
        <tr>
            <th>NN (any position)</th>
            <td>74306.5 (72328 - 77022)</td>
            <td>59476.2 (52915 - 66164)</td>
        </tr>
        <tr>
            <th>Greedy cycle</th>
            <td>73758.4 (72590 - 75345)</td>
            <td>59832.5 (53016 - 63736)</td>
        </tr>
        <tr>
            <th>NN (any position) 2-regret</th>
            <td>121360.0 (110950 - 135771)</td>
            <td>73649.8 (68761 - 79756)</td>
        </tr>
        <tr>
            <th>Greedy cycle 2-regret</th>
            <td>120368.0 (110303 - 128089)</td>
            <td>72368.0 (67078 - 76382)</td>
        </tr>
        <tr>
            <th>NN (any position) weighted 2-regret</th>
            <td>75561.2 (72419 - 78504)</td>
            <td>55698.3 (50485 - 61406)</td>
        </tr>
        <tr>
            <th>Greedy cycle weighted 2-regret</th>
            <td>73349.6 (72011 - 75532)</td>
            <td>57751.4 (52906 - 63326)</td>
        </tr>
        <tr>
            <th>Steepest LS, nodes, random start</th>
            <td>90206.2 (82646 - 99841)</td>
            <td>66119.5 (59051 - 73991)</td>
        </tr>
        <tr>
            <th>Steepest LS, nodes, greedy start</th>
            <td>73017.8 (71955 - 74682)</td>
            <td>54292.1 (49978 - 61265)</td>
        </tr>
        <tr>
            <th>Steepest LS, edges, random start</th>
            <td>76116.6 (73687 - 80203)</td>
            <td>52428.8 (50436 - 55344)</td>
        </tr>
        <tr>
            <th>Steepest LS, edges, greedy start</th>
            <td>72996.7 (71955 - 74682)</td>
            <td>53091.5 (49346 - 60354)</td>
        </tr>
        <tr>
            <th>Greedy LS, nodes, random start</th>
            <td>87950.4 (81247 - 95195)</td>
            <td>63794.9 (57198 - 71026)</td>
        </tr>
        <tr>
            <th>Greedy LS, nodes, greedy start</th>
            <td>73016.5 (71955 - 74682)</td>
            <td>54265.4 (49978 - 61265)</td>
        </tr>
        <tr>
            <th>Greedy LS, edges, random start</th>
            <td>76177.0 (74163 - 79378)</td>
            <td>52844.8 (50612 - 56161)</td>
        </tr>
        <tr>
            <th>Greedy LS, edges, greedy start</th>
            <td>72978.5 (71955 - 74664)</td>
            <td>53223.3 (49661 - 60506)</td>
        </tr> 
        <tr>
            <th>Steepest LS with candidate edges</th>
            <td>80135.4 (76405 - 87003)</td>
            <td>52554.9 (49573 - 56602)</td>
        </tr>
        <tr>
            <th>Steepest LS with previous deltas</th>
            <td>76082.7 (73618 - 79726)</td>
            <td>52570.2 (49632 - 55701)</td>
        </tr>
        <tr>
            <th>MSLS</th>
            <td>73569.1 (73187 - 74095)</td>
            <td>50207.2 (49651 - 50547)</td>
        </tr>
        <tr>
            <th>ILS</th>
            <td><b>71683.4</b> (<b>71497</b> - <b>72033</b>)</td>
            <td><b>48455.1</b> (<b>48157</b> - <b>49195</b>)</td>
        </tr>
        <tr>
            <th>LNS, random nodes, with LS</th>
            <td>72182.9 (71641 - 73582)</td>
            <td>49153.3 (48260 - 50085)</td>
        </tr>
        <tr>
            <th>LNS, random nodes, w/o LS</th>
            <td>72381.3 (71790 - 73169)</td>
            <td>49117.6 (48311 - 50025)</td>
        </tr>
        <tr>
            <th>LNS, single subpath, with LS</th>
            <td>72224.9 (71835 - 72950)</td>
            <td>49455.5 (48304 - 51067)</td>
        </tr>
        <tr>
            <th>LNS, single subpath, w/o LS</th>
            <td>72310.1 (71870 - 73245)</td>
            <td>49695.9 (48972 - 50721)</td>
        </tr>
        <tr>
            <th>LNS, multiple subpaths, with LS</th>
            <td>71755.6 (71649 - 72357)</td>
            <td>48889.6 (48253 - 49995)</td>
        </tr>
        <tr>
            <th>LNS, multiple subpaths, w/o LS</th>
            <td>71790.8 (71683 - 71950)</td>
            <td>49016.6 (48412 - 50230)</td>
        </tr>
        <tr>
            <th>LNS, weighted nodes, with LS</th>
            <td>72413.8 (71760 - 73163)</td>
            <td>49028.4 (48214 - 49713)</td>
        </tr>
        <tr>
            <th>LNS, weighted nodes, w/o LS</th>
            <td>72494.9 (71802 - 73232)</td>
            <td>49469.3 (48790 - 50169)</td>
        </tr>
    </tbody>
</table>

<div style="page-break-after: always;"></div>

### Running time [ms] of all methods
<table>
    <thead>
        <tr>
            <th rowspan="2" style="text-align:center">Method</th>
            <th colspan="2" style="text-align:center">Instance</th>
        </tr>
        <tr>
            <th style="text-align:center">A (avg)</th>
            <th style="text-align:center">B (avg)</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <th>Steepest LS, nodes, random start</th>
            <td>17.79</td>
            <td>17.33</td>
        </tr>
        <tr>
            <th>Steepest LS, nodes, greedy start</th>
            <td>0.64</td>
            <td>1.36</td>
        </tr>
        <tr>
            <th>Steepest LS, edges, random start</th>
            <td>13.90</td>
            <td>13.91</td>
        </tr>
        <tr>
            <th>Steepest LS, edges, greedy start</th>
            <td><b>0.46</b></td>
            <td>1.66</td>
        </tr>
        <tr>
            <th>Greedy LS, nodes, random start</th>
            <td>2.44</td>
            <td>2.36</td>
        </tr>
        <tr>
            <th>Greedy LS, nodes, greedy start</th>
            <td>0.81</td>
            <td><b>1.12</b></td>
        </tr>
        <tr>
            <th>Greedy LS, edges, random start</th>
            <td>1.79</td>
            <td>1.78</td>
        </tr>
        <tr>
            <th>Greedy LS, edges, greedy start</th>
            <td>0.81</td>
            <td>1.22</td>
        </tr>
        <tr>
            <th>Steepest LS with candidate edges</th>
            <td>2.50</td>
            <td>2.61</td>
        </tr>
        <tr>
            <th>Steepest LS with previous deltas</th>
            <td>2.30</td>
            <td>2.47</td>
        </tr>
        <tr>
            <th>MSLS</th>
            <td>831.52</td>
            <td>812.60</td>
        </tr>
        <tr>
            <th>ILS</th>
            <td>840.19</td>
            <td>840.19</td>
        </tr>
        <tr>
            <th>LNS, random nodes, with LS</th>
            <td>840.362</td>
            <td>840.307</td>
        </tr>
        <tr>
            <th>LNS, random nodes, w/o LS</th>
            <td>840.34</td>
            <td>840.359</td>
        </tr>
        <tr>
            <th>LNS, single subpath, with LS</th>
            <td>840.462</td>
            <td>840.434</td>
        </tr>
        <tr>
            <th>LNS, single subpath, w/o LS</th>
            <td>840.332</td>
            <td>840.348</td>
        </tr>
        <tr>
            <th>LNS, multiple subpaths, with LS</th>
            <td>840.496</td>
            <td>840.408</td>
        </tr>
        <tr>
            <th>LNS, multiple subpaths, w/o LS</th>
            <td>840.26</td>
            <td>840.285</td>
        </tr>
        <tr>
            <th>LNS, weighted nodes, with LS</th>
            <td>840.263</td>
            <td>840.331</td>
        </tr>
        <tr>
            <th>LNS, weighted nodes, w/o LS</th>
            <td>840.311</td>
            <td>840.331</td>
        </tr>
    </tbody>
</table>


<div style="page-break-after: always;"></div>

## 4. Best solutions
### Instance A
<table>
    <tr>
        <td align="center">Random nodes, w/o LS (cost 71790)<br><img src="img/Instance_A_LNS,_random_nodes,_w_o_LS.png" width="400"></td>
        <td align="center">Random nodes, with LS (cost 71641)<br><img src="img/Instance_A_LNS,_random_nodes,_with_LS.png" width="400"></td>
    </tr>
    <tr>
        <td align="center">Single subpath, w/o LS (cost 71870)<br><img src="img/Instance_A_LNS,_single_subpath,_w_o_LS.png" width="400"></td>
        <td align="center">Single subpath, with LS (cost 71835)<br><img src="img/Instance_A_LNS,_single_subpath,_with_LS.png" width="400"></td>
    </tr>
    <tr>
        <td align="center">Multiple subpaths, w/o LS (cost 71683)<br><img src="img/Instance_A_LNS,_multiple_subpaths,_w_o_LS.png" width="400"></td>
        <td align="center">Multiple subpaths, with LS (cost 71649)<br><img src="img/Instance_A_LNS,_multiple_subpaths,_with_LS.png" width="400"></td>
    </tr>
    <tr>
        <td align="center">Weighted nodes, w/o LS (cost 71802)<br><img src="img/Instance_A_LNS,_weighted_nodes,_w_o_LS.png" width="400"></td>
        <td align="center">Weighted nodes, with LS (cost 71760)<br><img src="img/Instance_A_LNS,_weighted_nodes,_with_LS.png" width="400"></td>
    </tr>
</table>

<div style="page-break-after: always;"></div>

### Instance B
<table>
    <tr>
        <td align="center">Random nodes, w/o LS (cost 48311)<br><img src="img/Instance_B_LNS,_random_nodes,_w_o_LS.png" width="400"></td>
        <td align="center">Random nodes, with LS (cost 48260)<br><img src="img/Instance_B_LNS,_random_nodes,_with_LS.png" width="400"></td>
    </tr>
    <tr>
        <td align="center">Single subpath, w/o LS (cost 48972)<br><img src="img/Instance_B_LNS,_single_subpath,_w_o_LS.png" width="400"></td>
        <td align="center">Single subpath, with LS (cost 48304)<br><img src="img/Instance_B_LNS,_single_subpath,_with_LS.png" width="400"></td>
    </tr>
    <tr>
        <td align="center">Multiple subpaths, w/o LS (cost 48412)<br><img src="img/Instance_B_LNS,_multiple_subpaths,_w_o_LS.png" width="400"></td>
        <td align="center">Multiple subpaths, with LS (cost 48253)<br><img src="img/Instance_B_LNS,_multiple_subpaths,_with_LS.png" width="400"></td>
    </tr>
    <tr>
        <td align="center">Weighted nodes, w/o LS (cost 48790)<br><img src="img/Instance_B_LNS,_weighted_nodes,_w_o_LS.png" width="400"></td>
        <td align="center">Weighted nodes, with LS (cost 48214)<br><img src="img/Instance_B_LNS,_weighted_nodes,_with_LS.png" width="400"></td>
    </tr>
</table>

<div style="page-break-after: always;"></div>

## 5. Best solutions - node indices
### Instance A
LNS, random nodes, w/o LS (cost 71790)

`[187, 189, 2, 171, 92, 69, 127, 97, 168, 116, 149, 63, 11, 49, 160, 83, 123, 100, 152, 108, 60, 31, 48, 80, 91, 157, 71, 4, 76, 132, 180, 28, 102, 185, 35, 51, 16, 67, 144, 140, 53, 179, 52, 177, 36, 131, 33, 9, 54, 8, 164, 186, 136, 184, 178, 29, 101, 147, 37, 47, 130, 159, 146, 170, 153, 193, 78, 88, 107, 64, 57, 182, 138, 151, 150, 106, 42, 89, 113, 93, 50, 3, 1, 85, 174, 176, 72, 134, 84, 58, 158, 192, 181, 114, 135, 73, 45, 87, 141, 30]`

LNS, random nodes, with LS (cost 71641)

`[131, 36, 177, 52, 179, 53, 140, 144, 67, 16, 51, 35, 185, 102, 28, 180, 132, 76, 4, 71, 157, 91, 80, 48, 31, 60, 108, 152, 100, 123, 160, 49, 11, 63, 149, 116, 168, 97, 127, 69, 129, 104, 92, 171, 2, 189, 187, 30, 141, 87, 45, 73, 135, 114, 181, 192, 158, 58, 84, 134, 72, 176, 174, 150, 106, 113, 89, 42, 151, 182, 57, 64, 107, 88, 78, 193, 153, 170, 146, 159, 130, 93, 50, 3, 169, 1, 47, 147, 101, 29, 178, 184, 136, 186, 164, 142, 8, 54, 9, 33]`

LNS, single subpath, w/o LS (cost 71870)

`[60, 31, 48, 80, 91, 157, 71, 4, 76, 132, 180, 28, 102, 185, 35, 51, 16, 67, 144, 140, 53, 179, 52, 177, 36, 131, 33, 9, 54, 8, 142, 164, 186, 136, 163, 29, 101, 178, 1, 169, 3, 50, 93, 47, 130, 159, 146, 170, 75, 153, 193, 78, 88, 107, 64, 57, 182, 138, 151, 42, 89, 113, 106, 150, 174, 176, 72, 134, 84, 58, 158, 192, 181, 114, 135, 73, 45, 87, 141, 30, 187, 189, 2, 171, 92, 69, 127, 97, 168, 116, 149, 63, 11, 49, 160, 83, 123, 100, 152, 108]`

LNS, single subpath, with LS (cost 71835)

`[47, 130, 159, 146, 170, 153, 193, 78, 88, 107, 64, 57, 182, 111, 192, 158, 134, 84, 58, 181, 114, 135, 73, 45, 87, 141, 189, 2, 171, 187, 30, 161, 127, 97, 168, 116, 149, 63, 11, 49, 160, 83, 123, 100, 152, 108, 60, 31, 48, 80, 91, 157, 71, 4, 76, 132, 180, 28, 102, 185, 35, 51, 16, 67, 144, 140, 53, 179, 52, 177, 36, 131, 33, 9, 54, 8, 174, 176, 151, 150, 106, 42, 89, 113, 93, 50, 3, 169, 1, 85, 142, 164, 186, 136, 184, 178, 29, 101, 147, 37]`

LNS, multiple subpaths, w/o LS (cost 71683)

`[150, 151, 138, 182, 57, 64, 107, 88, 78, 193, 153, 170, 146, 159, 130, 47, 147, 101, 29, 178, 184, 136, 186, 164, 8, 54, 9, 33, 131, 36, 177, 52, 179, 53, 140, 144, 67, 16, 51, 35, 185, 102, 28, 180, 132, 76, 4, 71, 157, 91, 80, 48, 31, 60, 116, 149, 63, 168, 11, 49, 108, 152, 100, 123, 83, 160, 97, 127, 69, 92, 171, 2, 189, 187, 30, 141, 87, 45, 73, 135, 114, 181, 192, 158, 58, 84, 134, 72, 176, 174, 85, 1, 169, 3, 50, 93, 113, 89, 42, 106]`

LNS, multiple subpaths, with LS (cost 71649)

`[187, 30, 141, 87, 102, 185, 45, 73, 135, 114, 181, 192, 158, 58, 84, 134, 72, 176, 174, 150, 106, 113, 89, 42, 151, 182, 57, 64, 107, 88, 78, 193, 153, 170, 146, 159, 130, 93, 50, 3, 169, 1, 47, 147, 101, 29, 178, 184, 136, 186, 164, 142, 8, 54, 9, 33, 131, 36, 177, 52, 179, 53, 140, 144, 67, 16, 51, 35, 156, 28, 180, 132, 76, 4, 71, 157, 91, 80, 48, 31, 60, 108, 152, 100, 123, 83, 160, 49, 11, 63, 149, 116, 168, 97, 127, 69, 92, 171, 2, 189]`

LNS, weighted nodes, w/o LS (cost 71802)

`[73, 45, 87, 141, 30, 187, 189, 2, 171, 92, 104, 129, 69, 127, 97, 168, 116, 149, 63, 11, 49, 160, 83, 123, 152, 108, 60, 31, 48, 80, 91, 157, 71, 4, 76, 132, 180, 28, 102, 185, 35, 51, 16, 67, 144, 140, 53, 179, 52, 177, 36, 131, 33, 9, 54, 8, 164, 186, 136, 184, 178, 29, 101, 147, 47, 93, 50, 3, 169, 1, 85, 174, 176, 150, 106, 151, 42, 89, 113, 130, 159, 146, 170, 153, 193, 78, 88, 107, 64, 57, 182, 111, 192, 158, 134, 84, 58, 181, 114, 135]`

LNS, weighted nodes, with LS (cost 71760)

`[51, 16, 67, 144, 140, 53, 179, 52, 177, 36, 131, 33, 9, 54, 8, 142, 164, 186, 136, 184, 178, 29, 101, 147, 47, 1, 169, 3, 50, 93, 130, 159, 146, 170, 107, 153, 193, 78, 88, 64, 57, 182, 138, 151, 42, 89, 113, 106, 150, 174, 176, 72, 134, 84, 58, 158, 192, 181, 114, 135, 73, 45, 87, 141, 30, 187, 189, 2, 171, 92, 69, 127, 97, 168, 116, 149, 63, 11, 49, 160, 83, 123, 100, 152, 108, 60, 31, 48, 80, 91, 157, 71, 4, 76, 132, 180, 28, 102, 185, 35]`

### Instance B
LNS, random nodes, w/o LS (cost 48311)

`[155, 71, 40, 60, 157, 45, 134, 175, 62, 36, 197, 132, 29, 6, 180, 115, 170, 125, 177, 158, 145, 28, 149, 13, 146, 37, 116, 147, 42, 121, 198, 96, 48, 156, 89, 57, 185, 76, 104, 190, 70, 187, 169, 81, 94, 105, 61, 196, 114, 10, 78, 54, 162, 192, 80, 127, 129, 90, 153, 14, 2, 20, 74, 17, 171, 99, 67, 24, 34, 152, 181, 179, 87, 82, 51, 85, 59, 73, 167, 98, 151, 3, 184, 46, 72, 120, 133, 136, 172, 160, 69, 189, 183, 30, 21, 38, 47, 140, 154, 97]`

LNS, random nodes, with LS (cost 48260)

`[81, 70, 190, 104, 76, 185, 57, 89, 156, 48, 96, 198, 121, 42, 147, 116, 37, 146, 13, 149, 28, 145, 65, 178, 103, 132, 29, 6, 197, 36, 62, 175, 134, 45, 157, 60, 40, 71, 155, 115, 170, 125, 26, 154, 140, 47, 38, 21, 30, 183, 69, 160, 172, 136, 133, 120, 117, 72, 46, 184, 3, 151, 27, 66, 67, 24, 34, 152, 181, 167, 73, 59, 85, 51, 82, 87, 179, 88, 99, 171, 17, 74, 20, 2, 14, 153, 90, 129, 127, 80, 192, 162, 54, 78, 10, 114, 196, 61, 105, 94]`

LNS, single subpath, w/o LS (cost 48972)

`[170, 125, 26, 154, 140, 47, 64, 69, 160, 172, 136, 133, 120, 117, 72, 46, 184, 3, 151, 98, 24, 34, 152, 181, 167, 73, 59, 85, 51, 82, 87, 179, 88, 99, 171, 17, 74, 20, 2, 14, 153, 90, 129, 80, 127, 10, 114, 196, 61, 105, 94, 81, 169, 190, 70, 187, 54, 192, 162, 76, 185, 57, 89, 156, 48, 96, 198, 121, 63, 183, 21, 30, 130, 50, 147, 116, 37, 146, 13, 149, 28, 145, 65, 178, 103, 132, 29, 6, 197, 36, 62, 175, 134, 45, 157, 60, 40, 71, 155, 115]`

LNS, single subpath, with LS (cost 48304)

`[80, 192, 162, 54, 78, 10, 114, 196, 61, 105, 94, 81, 70, 190, 104, 76, 185, 57, 89, 156, 48, 96, 198, 121, 42, 147, 116, 37, 146, 13, 149, 28, 145, 65, 178, 103, 132, 29, 6, 197, 36, 62, 175, 134, 45, 157, 60, 40, 71, 155, 115, 170, 177, 125, 26, 154, 140, 47, 38, 21, 30, 183, 189, 69, 160, 172, 136, 133, 120, 72, 46, 184, 3, 151, 98, 34, 24, 67, 152, 181, 167, 73, 59, 85, 51, 179, 82, 87, 88, 99, 171, 17, 74, 20, 2, 14, 153, 90, 129, 127]`

LNS, multiple subpaths, w/o LS (cost 48412)

`[40, 60, 157, 45, 134, 175, 62, 36, 197, 6, 29, 132, 103, 178, 65, 145, 28, 149, 13, 146, 37, 116, 147, 42, 121, 198, 96, 48, 156, 89, 57, 185, 76, 104, 190, 70, 187, 196, 114, 10, 78, 54, 162, 192, 80, 127, 129, 90, 153, 14, 2, 20, 74, 17, 171, 99, 88, 179, 87, 82, 51, 85, 59, 73, 167, 181, 152, 34, 24, 67, 66, 27, 151, 3, 184, 46, 72, 117, 8, 120, 133, 136, 172, 160, 69, 168, 183, 30, 21, 38, 47, 140, 154, 26, 125, 177, 170, 115, 155, 71]`

LNS, multiple subpaths, with LS (cost 48253)

`[94, 105, 61, 196, 114, 10, 78, 54, 162, 192, 80, 127, 129, 90, 153, 14, 2, 20, 74, 17, 171, 99, 88, 179, 87, 82, 51, 85, 59, 73, 167, 181, 152, 67, 24, 34, 98, 151, 3, 184, 46, 72, 117, 8, 120, 133, 136, 172, 160, 69, 183, 30, 21, 38, 47, 140, 154, 26, 125, 170, 115, 155, 71, 40, 60, 157, 45, 134, 175, 62, 36, 197, 6, 29, 132, 103, 178, 65, 145, 28, 149, 13, 146, 37, 116, 147, 42, 121, 198, 96, 48, 156, 89, 57, 185, 76, 104, 190, 70, 81]`

LNS, weighted nodes, w/o LS (cost 48790)

`[192, 162, 76, 185, 57, 89, 86, 46, 72, 117, 8, 172, 136, 133, 120, 96, 198, 121, 42, 147, 116, 37, 146, 13, 149, 28, 145, 65, 178, 103, 132, 29, 6, 180, 115, 170, 177, 125, 26, 154, 140, 97, 155, 71, 175, 134, 45, 157, 60, 40, 64, 47, 38, 21, 30, 183, 168, 69, 160, 184, 3, 151, 98, 167, 73, 59, 85, 51, 82, 87, 179, 181, 152, 34, 24, 67, 99, 171, 17, 74, 20, 2, 14, 153, 182, 80, 127, 129, 90, 10, 114, 196, 61, 105, 94, 81, 169, 70, 187, 54]`

LNS, weighted nodes, with LS (cost 48214)

`[179, 181, 152, 34, 24, 67, 99, 171, 17, 74, 20, 2, 14, 153, 90, 129, 80, 127, 10, 114, 196, 61, 105, 94, 81, 190, 70, 187, 54, 192, 162, 76, 185, 57, 89, 156, 48, 96, 198, 121, 42, 147, 116, 37, 146, 13, 149, 28, 145, 65, 178, 103, 132, 29, 6, 197, 36, 62, 175, 134, 45, 157, 60, 40, 71, 155, 115, 170, 177, 125, 26, 154, 140, 47, 38, 21, 30, 183, 189, 69, 160, 172, 136, 133, 120, 8, 117, 72, 46, 184, 3, 151, 98, 167, 73, 59, 85, 51, 82, 87]`

<div style="page-break-after: always;"></div>

## 6. Solution verification
All solutions have been verified with the solution checker and the experiment results match the results from the checker.


## 7. Source code
Source code is available on 
[Github](https://github.com/adammierzwicki/evolutionary-computation/tree/main/lab6).

## 8. Conclusions
Removing multiple subpaths in the destroy operator achieved the best average objective function value in both instances.
Using local search in the main loop improves the quality of the solutions in almost all cases, only in the case of LNS with random node removal on instance B it resulted in slightly worse solutions. This shows that it is beneficial to optimize the greedy solutions with use of local search algorithms.
Because all runs had the same time limit the time differences between the runs are negligible but we can see that the experiments with local search performed less iterations of the main loop and still achieved better solution quality.
LNS achieved comparable results to ILS but it got outperformed in both instances both on average and in the best solutions found.