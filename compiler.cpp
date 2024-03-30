#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <climits>
#include <cmath>
#include <cassert>
#include <limits>

struct pairHash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second); 

        return h1 * 31 ^ h2;
    }
};

class BiDict {
private:
    std::unordered_map<int, int> forward;
    std::unordered_map<int, int> reverse;

public:
    void setItem(int key, int value) {
        forward[key] = value;
        reverse[value] = key;
    }

    int getItem(int key) {
        return forward[key];
    }

    int getReverseItem(int key) {
        return reverse[key];
    }

    int size() {
        return forward.size();
    }
};

class Graph {
private:
    std::vector<std::vector<int>> graph;

public:
    Graph() = default;
    Graph(int n) : graph(n) {}

    void addEdge(int node1, int node2) {
        graph[node1].push_back(node2);
        graph[node2].push_back(node1);
    }

    // less efficient, but not used in the code
    bool areNeighbors(int node1, int node2) {
        for (int neighbor : graph[node1]) {
            if (neighbor == node2) {
                return true;
            }
        }
        return false;
    }

    // actually not use either
    std::vector<int> getNodes() {
        std::vector<int> nodes;
        for (int i = 0; i < graph.size(); i++) {
            nodes.push_back(i);
        }
        return nodes;
    }

    std::vector<int> getNeighbor(int node) const {
        return graph[node];
    }

    // floyd method
    std::vector<std::vector<int>> allPairDistances() const {
        int n = graph.size();
        const int INF = INT_MAX / 2; // Use large enough value

        std::vector<std::vector<int>> dist(n, std::vector<int>(n, INF));

        for (int i = 0; i < n; i++) {
            dist[i][i] = 0;
        }

        for (int i = 0; i < n; i++) {
            for (int j : graph[i]) {
                dist[i][j] = 1;
            }
        }

        for (int k = 0; k < n; k++) {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    if (dist[i][k] < INF && dist[k][j] < INF) {
                        dist[i][j] = std::min(dist[i][j], dist[i][k] + dist[k][j]);
                    }
                }
            }
        }

        return dist;
    }
};

void swapQubit(BiDict& qubitMapping, std::pair<int, int> gate) {
    int temp = qubitMapping.getItem(gate.first);
    qubitMapping.setItem(gate.first, qubitMapping.getItem(gate.second));
    qubitMapping.setItem(gate.second, temp);
}

// Function declarations
std::vector<std::pair<int, std::pair<int, int>>> sabresSwap(const std::vector<std::pair<int, int>>& gates, const std::vector<std::pair<int, int>>& dependencies, const Graph& g, int logQubits);
// void printMapping(const BiDict& qubitMapping, int logQubits);

int main() {
    int logQubits, numGates, numDependencies, phyQubits, numPhyLinks;
    std::cin >> logQubits >> numGates >> numDependencies >> phyQubits >> numPhyLinks;

    std::vector<std::pair<int, int>> gates(numGates);
    for (int i = 0; i < numGates; ++i) {
        int _, srcbit, dstbit;
        std::cin >> _ >> srcbit >> dstbit;
        gates[i] = std::make_pair(srcbit - 1, dstbit - 1);
    }

    std::vector<std::pair<int, int>> dependencies(numDependencies);
    for (int i = 0; i < numDependencies; ++i) {
        int _, srcgate, tgtgate;
        std::cin >> _ >> srcgate >> tgtgate;
        dependencies[i] = std::make_pair(srcgate - 1, tgtgate - 1);
    }

    Graph g(logQubits);
    for (int i = 0; i < numPhyLinks; ++i) {
        int _, src, dst;
        std::cin >> _ >> src >> dst;
        g.addEdge(src - 1, dst - 1);
    }

    BiDict qubitMapping;
    for (int i = 0; i < logQubits; ++i) {
        qubitMapping.setItem(i, i);
    }

    std::vector<std::pair<int, std::pair<int, int>>> operations = sabresSwap(gates, dependencies, g, logQubits);

    for (int i = 0; i < logQubits; ++i) {
        std::cout << i + 1 << " " << qubitMapping.getItem(i) + 1 << std::endl;
    }

    // printf("%d\n", operations.size());

    for (const auto& op : operations) {
        if (op.first == 1) {
            std::cout << "CNOT q" << op.second.first + 1 << " q" << op.second.second + 1 << std::endl;
        } else {
            std::cout << "SWAP q" << op.second.first + 1 << " q" << op.second.second + 1 << std::endl;
        }
    }

    return 0;
}

std::vector<std::pair<int, std::pair<int, int>>> sabresSwap(const std::vector<std::pair<int, int>>& gates, const std::vector<std::pair<int, int>>& dependencies, const Graph& g, int logQubits) {
    // std::unordered_map<int, std::vector<int>> graph;
    std::vector<std::vector<int>> dependencyGraph(logQubits + 5);
    // std::unordered_map<int, int> inDegree;
    std::vector<int> inDegree(logQubits + 5, 0);

    for (const auto& dependency : dependencies) {
        int u = dependency.first;
        int v = dependency.second;
        dependencyGraph[u].push_back(v);
        inDegree[v]++;
    }

    std::queue<int> checkQueue;
    std::unordered_set<int> executableQueue;

    // find all gate id with no dependency
    for (int idx = 0; idx < logQubits; idx++) {
        if (inDegree[idx] == 0) {
            checkQueue.push(idx);
        }
    }
    
    // random assign qubit mapping
    std::vector<std::pair<int, std::pair<int, int>>> operations;
    BiDict qubitMapping;
    for (int i = 0; i < logQubits; ++i) {
        qubitMapping.setItem(i, i);
    }

    std::vector<std::vector<int>> allPairDistance = g.allPairDistances();

    int a_cnt = 2;
    while (1) {
        // printf("check queue size %d\n", checkQueue.size());

        while (!checkQueue.empty()) {
            int candidate = checkQueue.front();
            checkQueue.pop();
            // printf("check candidate %d\n", candidate);
            int logSrc = gates[candidate].first;
            int logDst = gates[candidate].second;

            if (allPairDistance[qubitMapping.getItem(logSrc)][qubitMapping.getItem(logDst)] == 1) {
                // printf("done candidate %d\n", candidate);
                operations.push_back(std::make_pair(1, std::make_pair(logSrc, logDst)));

                for (int v : dependencyGraph[candidate]) {
                    inDegree[v]--;
                    if (inDegree[v] == 0) {
                        // printf("new candidate %d\n", v);
                        checkQueue.push(v);
                    }
                }

            } else {
                // printf("new executable %d\n", candidate);
                executableQueue.insert(candidate);
            }

            // printf("check queue size %d\n", checkQueue.size());
        }

        // printf("exe queue ");
        // for (auto i : executableQueue) {
        //     printf("%d ", i);
        // }
        // printf("\n");

        if (executableQueue.empty()) {
            break;
        }

        std::vector<int> futureQueue;
        for (int candidate : executableQueue) {
            for (int neighbor : dependencyGraph[candidate]) {
                // printf("%d, neighbor %d\n", candidate, neighbor);
                if (inDegree[neighbor] == 1) {
                    futureQueue.push_back(neighbor);
                }
            }
        }

        // printf("future queue ");
        // for (auto i : futureQueue) {
        //     printf("%d ", i);
        // }
        // printf("\n");

        std::vector<int> bestSolvedGate;
        std::pair<int, int> bestSwap;

        // int cnt = 10;
        while (bestSolvedGate.empty()) {
            std::unordered_set<std::pair<int, int>, pairHash> candidateSwap;

            for (int candidate : executableQueue) {
                int logSrc = gates[candidate].first;
                int logDst = gates[candidate].second;
                int phySrc = qubitMapping.getItem(logSrc);
                int phyDst = qubitMapping.getItem(logDst);

                for (int neighbor : g.getNeighbor(phySrc)) {
                    candidateSwap.insert(std::make_pair(logSrc, qubitMapping.getReverseItem(neighbor)));
                }
                for (int neighbor : g.getNeighbor(phyDst)) {
                    candidateSwap.insert(std::make_pair(logDst, qubitMapping.getReverseItem(neighbor)));
                }
            }

            // printf("candidate swap len %d\n", candidateSwap.size());

            int bestScore = INT_MAX;

            for (const auto& swap : candidateSwap) {
                swapQubit(qubitMapping, swap);

                int currentScore = 0;
                std::vector<int> solvedGate;
                for (int candidate : executableQueue) {
                    int logSrc = gates[candidate].first;
                    int logDst = gates[candidate].second;
                    int distance = allPairDistance[qubitMapping.getItem(logSrc)][qubitMapping.getItem(logDst)];
                    currentScore += distance;
                    assert(distance > 0);
                    if (distance == 1) {
                        solvedGate.push_back(candidate);
                    }
                }

                int currentFutureScore = 0;
                for (int candidate : futureQueue) {
                    int logSrc = gates[candidate].first;
                    int logDst = gates[candidate].second;
                    int distance = allPairDistance[qubitMapping.getItem(logSrc)][qubitMapping.getItem(logDst)];
                    currentFutureScore += distance;
                }

                currentScore += 0.5 * currentFutureScore;

                if (currentScore < bestScore) {
                    bestScore = currentScore;
                    bestSwap = swap;
                    bestSolvedGate = solvedGate;
                }
                // printf("swap (%d, %d), score %d\n", swap.first, swap.second, currentScore);
                swapQubit(qubitMapping, swap);
            }

            assert(bestScore < INT_MAX);

            // printf("best swap (%d, %d), score %d\n", bestSwap.first, bestSwap.second, bestScore);
            swapQubit(qubitMapping, bestSwap);
            operations.push_back(std::make_pair(0, bestSwap));

        }

        for (int gate : bestSolvedGate) {
            executableQueue.erase(gate);
            checkQueue.push(gate);
        }
    }

    return operations;
}
