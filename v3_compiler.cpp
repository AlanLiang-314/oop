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
#include <algorithm>


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
    std::vector<int> inDegree;

public:
    Graph() = default;
    Graph(int n) : graph(n), inDegree(n, 0) {}

    void addEdge(int node1, int node2) {
        graph[node1].push_back(node2);
        graph[node2].push_back(node1);
        inDegree[node1]++;
        inDegree[node2]++;
    }

    std::vector<int> getInDegree() const {
        return inDegree;
    }

    int maxInDegree() const {
        int maxInDegree = INT_MIN;
        int maxInDegreeIdx = -1;
        for(int i=0; i< inDegree.size(); i++) {
            if(inDegree[i] > maxInDegree) {
                maxInDegree = inDegree[i];
                maxInDegreeIdx = i;
            }
        }
        assert(maxInDegreeIdx != -1);
        return maxInDegreeIdx;
    }

    const auto& getNeighbor(int node) const {
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
std::vector<std::pair<int, std::pair<int, int>>> sabresSwap(const std::vector<std::pair<int, int>>& gates, const std::vector<std::pair<int, int>>& dependencies, const Graph& g, BiDict& qubitMapping, int logQubits, int numGates);

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

    std::vector<std::pair<int, std::pair<int, int>>> operations = sabresSwap(gates, dependencies, g, qubitMapping, logQubits, numGates);

    // for (int i = 0; i < logQubits; ++i) {
    //     std::cout << i + 1 << " " << qubitMapping.getItem(i) + 1 << std::endl;
    // }

    // printf("%d\n", operations.size());

    for (const auto& op : operations) {
        if (op.first == 1) {
            std::cout << "CNOT q" << op.second.first + 1 << " q" << op.second.second + 1 << '\n';
        } else {
            std::cout << "SWAP q" << op.second.first + 1 << " q" << op.second.second + 1 << '\n';
        }
    }

    return 0;
}

std::vector<std::vector<int>> getFrequencyMatrix(const std::vector<std::pair<int, int>>& gates, int n) {
    std::vector<std::vector<int>> freqeuncy(n, std::vector<int>(n, 0));
    for(const auto& gate : gates) {
        freqeuncy[gate.first][gate.second]++;
        freqeuncy[gate.second][gate.first]++;
    }
    return freqeuncy;
}


std::vector<int> sortQubits(const std::vector<std::vector<int>>& F, int logQubits) {
    std::vector<int> queue;
    std::vector<int> Fsum(F.size(), 0);
    std::unordered_set<int> qubits(logQubits);
    for (int i = 0; i < logQubits; i++) {
        qubits.insert(i);
    }

    for(int i=0;i<F.size();i++) {
        for(const auto& freqeuncy : F[i]) {
            // printf("%d ", freqeuncy);
            Fsum[i] += freqeuncy;
        }
        // printf("\n");
    }

    while(!qubits.empty()) {
        std::pair<int, int> best_qubit = std::make_pair(-1, INT_MAX);
        for(const auto& qubit : qubits) {
            if(Fsum[qubit] < best_qubit.second) {
                best_qubit.second = Fsum[qubit];
                best_qubit.first = qubit;
            }
        }
        assert(best_qubit.second < INT_MAX);
        queue.push_back(best_qubit.first);
        qubits.erase(best_qubit.first);
        for(int j=0;j<Fsum.size();j++) {
            Fsum[j] -= F[j][best_qubit.first];
        }
    }

    std::reverse(queue.begin(), queue.end());

    // for(auto i : queue) {
    //     printf("%d ", i);
    // }
    // printf("\n");

    return queue;
}

void allocateQubit(const std::vector<std::pair<int, int>>& gates, const std::vector<std::vector<int>>& allPairDistance, const Graph& g, BiDict& qubitMapping, int logQubits) {
    std::vector<std::vector<int>> F = getFrequencyMatrix(gates, logQubits);
    std::vector<int> queue = sortQubits(F, logQubits);
    int maxInDegree = g.maxInDegree();
    std::unordered_set<int> phyQubits(logQubits);
    for (int i = 0; i < logQubits; i++) {
        phyQubits.insert(i);
    }

    qubitMapping.setItem(queue[0], maxInDegree);
    // printf("%d->%d\n", queue[0], qubitMapping.getItem(queue[0]));
    phyQubits.erase(maxInDegree);

    for(int i=1; i < queue.size(); i++) {
        int bestScore = INT_MAX, bestQubit = -1;
        // printf("add qubit %d\n", queue[i]);
        for(const auto& phyQubit : phyQubits) {
            int score = 0;
            // assign (phyqubit, queue[i])
            // check the add score queue[0] ... queue[i -1], queue[i]
            for(int j=0;j<i;j++) {
                score += F[j][i] * allPairDistance[qubitMapping.getItem(queue[j])][phyQubit];
            }
            // printf("%d->%d, score %d\n", queue[i], phyQubit, score);
            if(score < bestScore) {
                bestScore = score;
                bestQubit = phyQubit;
            }

        }
        // printf("assign %d->%d\n", queue[i], bestQubit);
        qubitMapping.setItem(queue[i], bestQubit);
        phyQubits.erase(bestQubit);
    }
}

std::vector<std::pair<int, std::pair<int, int>>> sabresSwap(const std::vector<std::pair<int, int>>& gates, const std::vector<std::pair<int, int>>& dependencies, const Graph& g, BiDict& qubitMapping, int logQubits, int numGates) {
    std::vector<std::vector<int>> dependencyGraph(numGates);
    std::vector<int> inDegree(numGates, 0);

    std::vector<std::vector<int>> allPairDistance = g.allPairDistances();
    
    if(false) {
        allocateQubit(gates, allPairDistance, g, qubitMapping, logQubits);
    } else {
        // random assign qubit mapping
        for (int i = 0; i < logQubits; ++i) {
            qubitMapping.setItem(i, i);
        }
    }

    for (int i = 0; i < logQubits; ++i) {
        std::cout << i + 1 << ' ' << qubitMapping.getItem(i) + 1 << '\n';
    }

    for (const auto& dependency : dependencies) {
        int u = dependency.first;
        int v = dependency.second;
        dependencyGraph[u].push_back(v);
        inDegree[v]++;
    }

    std::vector<std::pair<int, std::pair<int, int>>> operations;

    std::queue<int> checkQueue;
    // std::unordered_set<int> executableQueue;
    std::vector<int> executableQueue;
    std::vector<int> futureQueue;

    // find all gate id with no dependency
    for (int idx = 0; idx < numGates; idx++) {
        if (inDegree[idx] == 0) {
            checkQueue.push(idx);
        }
    }
    

    int targetGate, remainGate = numGates;
    // printf("target gate: %d, numgate: %d\n", targetGate, numGates);
    while(remainGate) {
    // printf("remain gate: %d\n", remainGate);
    targetGate = numGates / 20;
    while (executableQueue.size() < (numGates / 100)) {
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
                targetGate--;
                remainGate--;

                for (int v : dependencyGraph[candidate]) {
                    inDegree[v]--;
                    if (inDegree[v] == 0) {
                        // printf("new candidate %d\n", v + 1);
                        checkQueue.push(v);
                    }
                }

            } else {
                // printf("new executable %d\n", candidate + 1);
                // executableQueue.insert(candidate);
                executableQueue.push_back(candidate);
            }

            // printf("check queue size %d\n", checkQueue.size());
        }

        if (executableQueue.empty()) {
            break;
        }



        futureQueue.clear();
        std::vector<std::pair<int, int>> bestSwaps;
        std::pair<int, int> bestSwap;
        std::pair<int, int> punishSwap = std::make_pair(-1, -1);

        std::unordered_map<int, int> swapMap;
        std::unordered_map<int, int> futureSwapMap;
        std::vector<std::pair<std::pair<int, int>, std::vector<int>>> candidateSwaps(executableQueue.size() * 2);

        int index = 0;
        for (const int& candidate : executableQueue) {
            int logSrc = gates[candidate].first;
            int logDst = gates[candidate].second;
            int phySrc = qubitMapping.getItem(logSrc);
            int phyDst = qubitMapping.getItem(logDst);
            swapMap[logSrc] = candidate;
            swapMap[logDst] = candidate;
            // printf("%d %d ", logSrc, logDst);
            
            for (int neighbor : dependencyGraph[candidate]) {
                if (inDegree[neighbor] == 1) {
                    futureQueue.push_back(neighbor);
                    int logSrc = gates[neighbor].first;
                    int logDst = gates[neighbor].second;
                    futureSwapMap[logSrc] = neighbor;
                    futureSwapMap[logDst] = neighbor;
                }
            }

            candidateSwaps[index].first = std::make_pair(logSrc, logDst);
            for (int neighbor : g.getNeighbor(phySrc)) {
                candidateSwaps[index].second.push_back(qubitMapping.getReverseItem(neighbor));
            }
            index++;

            candidateSwaps[index].first = std::make_pair(logDst, logSrc);
            for (int neighbor : g.getNeighbor(phyDst)) {
                candidateSwaps[index].second.push_back(qubitMapping.getReverseItem(neighbor));
            }
            index++;
        }
        // printf("\n");

        // for (int candidate : futureQueue) {
            // printf("%d %d ", gates[candidate].first, gates[candidate].second);
        // }
        // printf("\n");

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

                currentScore = (2 * distance + futureDistance);
                if (currentScore < bestScore) {
                    bestScore = currentScore;
                    bestSwaps.clear();
                    bestSwaps.push_back(swap);
                    // printf("clear, (%d, %d) push into best swap\n", swap.first, swap.second);
                } else if (currentScore == bestScore) {
                    bestSwaps.push_back(swap);
                    // printf("(%d, %d) push into best swap\n", swap.first, swap.second);
                }
                
            }
        }

        int idx = std::rand() % bestSwaps.size();
        bestSwap = bestSwaps[idx];
        punishSwap = bestSwap;

        // printf("best swap (%d, %d), score %d\n", bestSwap.first, bestSwap.second, bestScore);
        swapQubit(qubitMapping, bestSwap);
        operations.push_back(std::make_pair(0, bestSwap));

        // if(executableQueue.size() > 30) {
        //     continue;
        // }

        for (int gate : executableQueue) {
            checkQueue.push(gate);
        }
        executableQueue.clear();
    }

    targetGate = numGates / 100;
    if(targetGate == 0) {
        targetGate = remainGate;
    }
    // printf("remain gate: %d, target gate: %d\n", remainGate, targetGate);

    while (targetGate) {
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
                targetGate--;
                remainGate--;

                for (int v : dependencyGraph[candidate]) {
                    inDegree[v]--;
                    if (inDegree[v] == 0) {
                        // printf("new candidate %d\n", v + 1);
                        checkQueue.push(v);
                    }
                }

            } else {
                // printf("new executable %d\n", candidate + 1);
                // executableQueue.insert(candidate);
                executableQueue.push_back(candidate);
            }

            // printf("check queue size %d\n", checkQueue.size());
        }

        // break;

        if (executableQueue.empty()) {
            break;
        }


        std::pair<int, int> bestSwap;

        for(const auto& gateId : executableQueue) {
            const auto& gate = gates[gateId];
            const auto a = qubitMapping.getItem(gate.first), b = qubitMapping.getItem(gate.second);
            const auto distance = allPairDistance[a][b];
            const auto& neighbors = g.getNeighbor(a);

            for(const auto& neighbor : neighbors) {
                if(distance - allPairDistance[b][neighbor] > 0) {
                    bestSwap = std::make_pair(a, neighbor);
                    break;
                }
            }
        }

        // printf("best swap (%d, %d)\n", bestSwap.first, bestSwap.second);
        bestSwap.first = qubitMapping.getReverseItem(bestSwap.first), bestSwap.second = qubitMapping.getReverseItem(bestSwap.second);
        swapQubit(qubitMapping, bestSwap);
        operations.push_back(std::make_pair(0, bestSwap));

        for (int gate : executableQueue) {
            checkQueue.push(gate);
        }
        executableQueue.clear();
    }
    }



    return operations;
}