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

    friend void swapQubit(BiDict& qubitMapping, std::pair<int, int> gate);
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

    std::vector<std::vector<int>> allPairDistances() const {
        int n = graph.size();
        std::vector<std::vector<int>> distances(n, std::vector<int>(n, -1));

        for (int i = 0; i < n; i++) {
            distances[i][i] = 0;
            std::queue<int> q;
            q.push(i);
            std::vector<bool> visited(n, false);
            visited[i] = true;
            int distance = 0;

            while (!q.empty()) {
                int size = q.size();
                for (int j = 0; j < size; j++) {
                    int node = q.front();
                    q.pop();
                    for (int neighbor : getNeighbor(node)) {
                        if (!visited[neighbor]) {
                            visited[neighbor] = true;
                            distances[i][neighbor] = distance + 1;
                            q.push(neighbor);
                        }
                    }
                }
                distance++;
            }
        }

        return distances;
    }

};

void swapQubit(BiDict& qubitMapping, std::pair<int, int> gate) {
    std::swap(qubitMapping.forward[gate.first], qubitMapping.forward[gate.second]);
    std::swap(qubitMapping.reverse[qubitMapping.forward[gate.first]], qubitMapping.reverse[qubitMapping.forward[gate.second]]);
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

    std::vector<std::vector<int>> allPairDistance = g.allPairDistances();

    for (const auto& dependency : dependencies) {
        int u = dependency.first;
        int v = dependency.second;
        dependencyGraph[u].push_back(v);
        inDegree[v]++;
    }

    // random assign qubit mapping
    std::vector<std::pair<int, std::pair<int, int>>> operations;
    BiDict qubitMapping(logQubits);
    for (int i = 0; i < logQubits; ++i) {
        qubitMapping.setItem(i, i);
    }

    std::queue<int> checkQueue;
    std::unordered_set<int> executableQueue;

    // find all gate id with no dependency
    for (int idx = 0; idx < numGates; idx++) {
        if (inDegree[idx] == 0) {
            checkQueue.push(idx);
        }
    }

    // while (!checkQueue.empty())
    // {
    //     /* code */
    // }
    

    int a_cnt = 2;
    while (true) {
        // printf("check queue size %d\n", checkQueue.size());

        while (!checkQueue.empty()) {
            int candidate = checkQueue.front();
            checkQueue.pop();
            // printf("check candidate %d\n", candidate + 1);
            int logSrc = gates[candidate].first;
            int logDst = gates[candidate].second;

            if (allPairDistance[qubitMapping.getItem(logSrc)][qubitMapping.getItem(logDst)] == 1) {
                // printf("done candidate %d, (%d, %d)\n", candidate, logSrc, logDst);
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
        std::pair<int, int> punishSwap = std::make_pair(-1, -1);

        // int cnt = 3;
        // while (bestSolvedGates.empty() && cnt--) {
        // std::unordered_set<std::pair<int, int>, pairHash> candidateSwap;
        std::unordered_map<int, int> swapMap;
        std::unordered_map<int, int> futureSwapMap;
        std::vector<std::pair<std::pair<int, int>, std::vector<int>>> candidateSwaps(executableQueue.size() * 2); // each gate and their neighbor

        int index = 0;
        for (const int& candidate : executableQueue) {
            int logSrc = gates[candidate].first;
            int logDst = gates[candidate].second;
            int phySrc = qubitMapping.getItem(logSrc);
            int phyDst = qubitMapping.getItem(logDst);
            swapMap[logSrc] = candidate;
            swapMap[logDst] = candidate;
            // printf("%d %d ", logSrc, logDst);

            candidateSwaps[index].first = std::make_pair(logSrc, logDst);
            for (int neighbor : g.getNeighbor(phySrc)) {
                candidateSwaps[index].second.push_back(qubitMapping.getReverseItem(neighbor));
                // candidateSwap.insert(std::make_pair(logSrc, qubitMapping.getReverseItem(neighbor)));
            }
            index++;

            candidateSwaps[index].first = std::make_pair(logDst, logSrc);
            for (int neighbor : g.getNeighbor(phyDst)) {
                candidateSwaps[index].second.push_back(qubitMapping.getReverseItem(neighbor));
                // candidateSwap.insert(std::make_pair(logDst, qubitMapping.getReverseItem(neighbor)));
            }
            index++;
        }
        // printf("\n");

        for (int candidate : futureQueue) {
            int logSrc = gates[candidate].first;
            int logDst = gates[candidate].second;
            futureSwapMap[logSrc] = candidate;
            futureSwapMap[logDst] = candidate;
            // printf("%d %d ", gates[candidate].first, gates[candidate].second);
        }
        // printf("\n");

        // bestScore = 0;
        // std::vector<int> solvedGate;
        // for (const int& candidate : executableQueue) {
        //         int logSrc = gates[candidate].first;
        //         int logDst = gates[candidate].second;
        //         int distance = allPairDistance[qubitMapping.getItem(logSrc)][qubitMapping.getItem(logDst)];
        //         bestScore += distance;
        //         assert(distance > 1);
        // }

        // int currentFutureScore = 0;     
        // for (const int& candidate : futureQueue) {
        //     int logSrc = gates[candidate].first;
        //     int logDst = gates[candidate].second;
        //     int distance = allPairDistance[qubitMapping.getItem(logSrc)][qubitMapping.getItem(logDst)];
        //     currentFutureScore += distance;
        // }

        // printf("origin score %d, origin future %d\n", bestScore, currentFutureScore);

        // bestScore = INT_MAX;
        int currentScore, bestScore = INT_MAX;
        bestSwaps.clear();

        for(const auto& qubit : candidateSwaps) {
            auto mainGate = qubit.first;
            // we swap mainGate.first and neighbor
            for (int neighbor : qubit.second) {
                auto swap = std::make_pair(mainGate.first, neighbor);
                if(punishSwap == swap || (punishSwap.second == swap.first && punishSwap.first == swap.second)) {
                    break;
                }
                // printf("logSrc %d, neighbor %d\n", mainGate.first, neighbor);
                auto itSwap = swapMap.find(neighbor), itFSwapSrc = futureSwapMap.find(mainGate.first), itFSwapDst = futureSwapMap.find(neighbor);
                int distance = 0, futureDistance = 0;
                if (itSwap != swapMap.end()) {
                    auto gate = gates[itSwap->second];
                    distance -= allPairDistance[qubitMapping.getItem(gate.first)][qubitMapping.getItem(gate.second)];
                    // printf("has a friend (%d, %d), distance %d\n", gate.first, gate.second, allPairDistance[qubitMapping.getItem(gate.first)][qubitMapping.getItem(gate.second)]);
                }
                if (itFSwapSrc != futureSwapMap.end()) {
                    auto gate = gates[itFSwapSrc->second];
                    futureDistance -= allPairDistance[qubitMapping.getItem(gate.first)][qubitMapping.getItem(gate.second)];
                    // printf("has a future friend (%d, %d), distance %d\n", gate.first, gate.second, allPairDistance[qubitMapping.getItem(gate.first)][qubitMapping.getItem(gate.second)]);
                }
                if (itFSwapDst != futureSwapMap.end()) {
                    auto gate = gates[itFSwapDst->second];
                    futureDistance -= allPairDistance[qubitMapping.getItem(gate.first)][qubitMapping.getItem(gate.second)];
                    // printf("has a future friend (%d, %d), distance %d\n", gate.first, gate.second, allPairDistance[qubitMapping.getItem(gate.first)][qubitMapping.getItem(gate.second)]);
                }
                distance -= allPairDistance[qubitMapping.getItem(mainGate.first)][qubitMapping.getItem(mainGate.second)];
                // printf("has a friend (%d, %d) distance %d\n", mainGate.first, mainGate.second, allPairDistance[qubitMapping.getItem(mainGate.first)][qubitMapping.getItem(mainGate.second)]);

                swapQubit(qubitMapping, swap);

                if (itSwap != swapMap.end()) {
                    auto gate = gates[itSwap->second];
                    distance += allPairDistance[qubitMapping.getItem(gate.first)][qubitMapping.getItem(gate.second)];
                    // printf("has a friend (%d, %d), distance %d\n", gate.first, gate.second, allPairDistance[qubitMapping.getItem(gate.first)][qubitMapping.getItem(gate.second)]);
                }
                if (itFSwapSrc != futureSwapMap.end()) {
                    auto gate = gates[itFSwapSrc->second];
                    futureDistance += allPairDistance[qubitMapping.getItem(gate.first)][qubitMapping.getItem(gate.second)];
                    // printf("has a future friend (%d, %d), distance %d, now distance %d\n", gate.first, gate.second, allPairDistance[qubitMapping.getItem(gate.first)][qubitMapping.getItem(gate.second)], futureDistance);
                }
                if (itFSwapDst != futureSwapMap.end()) {
                    auto gate = gates[itFSwapDst->second];
                    futureDistance += allPairDistance[qubitMapping.getItem(gate.first)][qubitMapping.getItem(gate.second)];
                    // printf("has a future friend (%d, %d), distance %d, now distance %d\n", gate.first, gate.second, allPairDistance[qubitMapping.getItem(gate.first)][qubitMapping.getItem(gate.second)], futureDistance);
                }
                distance += allPairDistance[qubitMapping.getItem(mainGate.first)][qubitMapping.getItem(mainGate.second)];
                // printf("has a friend (%d, %d) distance %d\n", mainGate.first, mainGate.second, allPairDistance[qubitMapping.getItem(mainGate.first)][qubitMapping.getItem(mainGate.second)]);

                swapQubit(qubitMapping, swap);

                // printf("swap (%d, %d), score %d, future score %d\n", mainGate.first, neighbor, distance, futureDistance);

                currentScore = 2 * distance + futureDistance;
                if (currentScore < bestScore) {
                    bestScore = currentScore;
                    bestSwaps.clear();
                    bestSwaps.push_back(swap);
                    // printf("clear, (%d, %d) push into best swap\n", swap.first, swap.second);
                    // bestSolvedGates.clear();
                    // bestSolvedGates.push_back(solvedGate);
                } else if (currentScore == bestScore) {
                    bestSwaps.push_back(swap);
                    // printf("(%d, %d) push into best swap\n", swap.first, swap.second);
                    // bestSolvedGates.push_back(solvedGate);
                }
                
            }
        }

        // printf("best swap len %d\n", bestSwaps.size());

        // printf("best swap (%d, %d), score %d\n", bestSwaps[0].first, bestSwaps[0].second, bestScore);

        // auto it = executableQueue.begin();
        // while (it != executableQueue.end()) {
        //     int gateId = *it;
        //     int logSrc = gates[gateId].first, logDst = gates[gateId].second;
        //     if (allPairDistance[qubitMapping.getItem(logSrc)][qubitMapping.getItem(logDst)] == 1) {
        //         it = executableQueue.erase(it);
        //     } else {
        //         ++it;
        //     }
        // }

        // for (const auto& swap : candidateSwap) {
        //     swapQubit(qubitMapping, swap);

        //     int currentScore = 0;
        //     std::vector<int> solvedGate;
        //     for (int candidate : executableQueue) {
        //         int logSrc = gates[candidate].first;
        //         int logDst = gates[candidate].second;
        //         int distance = allPairDistance[qubitMapping.getItem(logSrc)][qubitMapping.getItem(logDst)];
        //         currentScore += distance;
        //         assert(distance > 0);
        //         if (distance == 1) {
        //             solvedGate.push_back(candidate);
        //         }
        //     }

        //     int currentFutureScore = 0;
        //     for (int candidate : futureQueue) {
        //         int logSrc = gates[candidate].first;
        //         int logDst = gates[candidate].second;
        //         int distance = allPairDistance[qubitMapping.getItem(logSrc)][qubitMapping.getItem(logDst)];
        //         currentFutureScore += distance;
        //     }
        //     printf("swap (%d, %d), score %d, future %d\n", swap.first, swap.second, currentScore, currentFutureScore);
        //     currentScore += 0.5 * currentFutureScore;

        //     if (currentScore < bestScore) {
        //         bestScore = currentScore;
        //         bestSwaps.clear();
        //         bestSwaps.push_back(swap);
        //         bestSolvedGates.clear();
        //         bestSolvedGates.push_back(solvedGate);
        //     } else if (currentScore == bestScore) {
        //         bestSwaps.push_back(swap);
        //         bestSolvedGates.push_back(solvedGate);
        //     }
        //     // printf("swap (%d, %d), score + future %d\n", swap.first, swap.second, currentScore);
        //     swapQubit(qubitMapping, swap);
        // }

        // assert(bestScore < INT_MAX);

        int idx = std::rand() % bestSwaps.size();
        bestSwap = bestSwaps[idx];
        punishSwap = bestSwap;
        // bestSolvedGate = bestSolvedGates[idx];

        // printf("best swap (%d, %d), score %d\n", bestSwap.first, bestSwap.second, bestScore);
        swapQubit(qubitMapping, bestSwap);
        operations.push_back(std::make_pair(0, bestSwap));
        // }

        for (int gate : executableQueue) {
            // executableQueue.erase(gate);
            checkQueue.push(gate);
        }
        executableQueue.clear();
    }

    return operations;
}
