#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <assert.h>

using namespace std;

class BiDict {
private:

public:
    unordered_map<int, int> forward;
    unordered_map<int, int> reverse;

    void insert(int key, int value) {
        forward[key] = value;
        reverse[value] = key;
    }

    int getValue(int key) {
        return forward[key];
    }

    int getKey(int value) {
        return reverse[value];
    }

    bool containsKey(int key) {
        return forward.count(key) > 0;
    }

    bool containsValue(int value) {
        return reverse.count(value) > 0;
    }
};

vector<vector<int>> allPairShortestPaths(vector<vector<int>>& graph, int numNodes) {
    vector<vector<int>> dist(numNodes, vector<int>(numNodes, INT_MAX));

    for (int u = 0; u < numNodes; u++) {
        dist[u][u] = 0;
        for (int v : graph[u]) {
            dist[u][v] = 1;
        }
    }

    for (int k = 0; k < numNodes; k++) {
        for (int i = 0; i < numNodes; i++) {
            for (int j = 0; j < numNodes; j++) {
                if (dist[i][k] != INT_MAX && dist[k][j] != INT_MAX && dist[i][j] > dist[i][k] + dist[k][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                }
            }
        }
    }

    vector<vector<int>> paths(numNodes, vector<int>(numNodes));
    for (int i = 0; i < numNodes; i++) {
        for (int j = 0; j < numNodes; j++) {
            if (dist[i][j] == INT_MAX) {
                paths[i][j] = {};
            } else {
                int u = i;
                paths[i][j] = {u};
                while (u != j) {
                    for (int v = 0; v < numNodes; v++) {
                        if (dist[u][v] != INT_MAX && dist[v][j] != INT_MAX && dist[u][j] == dist[u][v] + dist[v][j]) {
                            u = v;
                            paths[i][j].push_back(u);
                            break;
                        }
                    }
                }
            }
        }
    }

    return paths;
}

int main() {
    int logQubits, num_gates, num_dependencies, phyQubits, num_phyLinks;
    cin >> logQubits >> num_gates >> num_dependencies >> phyQubits >> num_phyLinks;

    vector<pair<int, int>> gates(num_gates);
    for (int i = 0; i < num_gates; i++) {
        int _, srcbit, dstbit;
        cin >> _ >> srcbit >> dstbit;
        gates[i] = make_pair(srcbit - 1, dstbit - 1);
    }

    vector<pair<int, int>> dependencies(num_dependencies);
    for (int i = 0; i < num_dependencies; i++) {
        int _, srcgate, tgtgate;
        cin >> _ >> srcgate >> tgtgate;
        dependencies[i] = make_pair(srcgate - 1, tgtgate - 1);
    }

    vector<vector<int>> graph(phyQubits);
    for (int i = 0; i < num_phyLinks; i++) {
        int _, src, dst;
        cin >> _ >> src >> dst;
        graph[src - 1].push_back(dst - 1);
        graph[dst - 1].push_back(src - 1);
    }

    unordered_map<int, vector<int>> adjList;
    unordered_map<int, int> inDegree;
    for (auto& dep : dependencies) {
        int u = dep.first, v = dep.second;
        adjList[u].push_back(v);
        inDegree[v]++;
    }

    queue<int> q;
    for (auto& [u, degree] : inDegree) {
        if (degree == 0) {
            q.push(u);
        }
    }

    vector<int> result;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        result.push_back(u);

        for (int v : adjList[u]) {
            inDegree[v]--;
            if (inDegree[v] == 0) {
                q.push(v);
            }
        }
    }

    assert(result.size() == adjList.size());

    BiDict qubit_mapping;
    for (int i = 0; i < logQubits; i++) {
        qubit_mapping.insert(i, i);
    }

    for (auto& [key, value] : qubit_mapping.forward) {
        cout << key + 1 << " " << value + 1 << endl;
    }

    vector<vector<int>> all_pair_paths = allPairShortestPaths(graph, phyQubits);

    vector<pair<int, pair<int, int>>> operations;
    for (int id : result) {
        int logical_srcbit = gates[id].first;
        int logical_dstbit = gates[id].second;
        int phy_srcbit = qubit_mapping.getValue(logical_srcbit);
        int phy_dstbit = qubit_mapping.getValue(logical_dstbit);

        vector<int>& path = all_pair_paths[phy_srcbit][phy_dstbit];
        if (path.size() > 2) {
            for (int i = 0; i < path.size() - 2; i++) {
                int temp_a = qubit_mapping.getKey(path[i]);
                int temp_b = qubit_mapping.getKey(path[i + 1]);
                qubit_mapping.insert(temp_a, path[i + 1]);
                qubit_mapping.insert(temp_b, path[i]);
                operations.emplace_back(0, temp_a, temp_b);
            }

            phy_srcbit = qubit_mapping.getValue(logical_srcbit);
            phy_dstbit = qubit_mapping.getValue(logical_dstbit);
            path = all_pair_paths[phy_srcbit][phy_dstbit];
            assert(path.size() == 2);
        }

        operations.emplace_back(1, logical_srcbit, logical_dstbit);
    }

    for (auto& op : operations) {
        int op_type = op.first;
        int src = op.second.first;
        int dst = op.second.second;
        cout << (op_type ? "CNOT" : "SWAP") << " q" << src + 1 << " q" << dst + 1 << endl;
    }

    return 0;
}