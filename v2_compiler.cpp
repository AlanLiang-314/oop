#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <climits>
#include <cmath>
#include <cassert>
#include <limits>
#include <cstdlib>


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
    std::vector<int> forward;
    std::vector<int> reverse;

public:
    BiDict(int size) : forward(size), reverse(size) {}

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
std::vector<std::pair<int, std::pair<int, int>>> sabresSwap(const std::vector<std::pair<int, int>>& gates, const std::vector<std::pair<int, int>>& dependencies, const Graph& g, int logQubits, int numGates);

int main() {
    std::srand(42);
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

    BiDict qubitMapping(logQubits);
    for (int i = 0; i < logQubits; ++i) {
        qubitMapping.setItem(i, i);
    }

    std::vector<std::pair<int, std::pair<int, int>>> operations = sabresSwap(gates, dependencies, g, logQubits, numGates);

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

std::vector<std::pair<int, std::pair<int, int>>> sabresSwap(const std::vector<std::pair<int, int>>& gates, const std::vector<std::pair<int, int>>& dependencies, const Graph& g, int logQubits, int numGates) {
    std::vector<std::vector<int>> dependencyGraph(numGates);
    std::vector<int> inDegree(numGates, 0);

    // printf("numgates %d\n", numGates);

    for (const auto& dependency : dependencies) {
        int u = dependency.first;
        int v = dependency.second;
        dependencyGraph[u].push_back(v);
        inDegree[v]++;
    }

    std::queue<int> checkQueue;
    std::unordered_set<int> executableQueue;

    // find all gate id with no dependency
    for (int idx = 0; idx < numGates; idx++) {
        if (inDegree[idx] == 0) {
            checkQueue.push(idx);
        }
    }
    
    // random assign qubit mapping
    std::vector<std::pair<int, std::pair<int, int>>> operations;
    BiDict qubitMapping(logQubits);
    for (int i = 0; i < logQubits; ++i) {
        qubitMapping.setItem(i, i);
    }

    std::vector<std::vector<int>> allPairDistance = g.allPairDistances();

    // for (int i=0;i<numGates;i++) {
    //     printf("%d: ", i + 1);
    //     for (int tgtnode : dependencyGraph[i]) {
    //         printf("%d ", tgtnode + 1);
    //     }
    //     printf(" | %d\n", inDegree[i]);
    // }

    int a_cnt = 2;
    while (1) {
        // printf("check queue size %d\n", checkQueue.size());

        while (!checkQueue.empty()) {
            int candidate = checkQueue.front();
            checkQueue.pop();
            // printf("check candidate %d\n", candidate + 1);
            int logSrc = gates[candidate].first;
            int logDst = gates[candidate].second;

            if (allPairDistance[qubitMapping.getItem(logSrc)][qubitMapping.getItem(logDst)] == 1) {
                // printf("done candidate %d, (%d, %d)\n", candidate + 1, logSrc + 1, logDst + 1);
                operations.push_back(std::make_pair(1, std::make_pair(logSrc, logDst)));

                for (int v : dependencyGraph[candidate]) {
                    inDegree[v]--;
                    if (inDegree[v] == 0) {
                        // printf("new candidate %d\n", v + 1);
                        checkQueue.push(v);
                    }
                }

            } else {
                // printf("new executable %d\n", candidate + 1);
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

        std::vector<std::vector<int>> bestSolvedGates;
        std::vector<int> bestSolvedGate;
        std::vector<std::pair<int, int>> bestSwaps;
        std::pair<int, int> bestSwap;

        // int cnt = 3;
        while (bestSolvedGates.empty()) {
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
                    bestSwaps.clear();
                    bestSwaps.push_back(swap);
                    bestSolvedGates.clear();
                    bestSolvedGates.push_back(solvedGate);
                } else if (currentScore == bestScore) {
                    bestSwaps.push_back(swap);
                    bestSolvedGates.push_back(solvedGate);
                }
                // printf("swap (%d, %d), score %d\n", swap.first, swap.second, currentScore);
                swapQubit(qubitMapping, swap);
            }

            assert(bestScore < INT_MAX);

            int idx = std::rand() % bestSwaps.size();
            std::pair<int, int> bestSwap = bestSwaps[idx];
            bestSolvedGate = bestSolvedGates[idx];

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
