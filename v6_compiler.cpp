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


// struct pairHash {
//     template <class T1, class T2>
//     std::size_t operator () (const std::pair<T1,T2> &p) const {
//         auto h1 = std::hash<T1>{}(p.first);
//         auto h2 = std::hash<T2>{}(p.second); 

//         return h1 * 31 ^ h2;
//     }
// };

class BiDict {
private:
    std::vector<int> forward;
    std::vector<int> reverse;

public:
    std::vector<bool> lock;
    std::vector<bool> lockReverse;
    BiDict(int size) : forward(size), reverse(size), lock(size, false), lockReverse(size, false) {}

    void lockUp(int logBit, int phyBit) {
        lock[phyBit] = true, lockReverse[logBit] = true; 
    }

    void set(int key, int value) {
        forward[key] = value;
    }

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

    friend void swapQubit(BiDict& qubitMapping, std::pair<int, int>& gate);
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

    const auto getSize() const {
        return graph.size();
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

            while (!q.empty()) {
                int node = q.front();
                q.pop();
                for (const auto& neighbor : graph[node]) {
                    if (!visited[neighbor]) {
                        visited[neighbor] = true;
                        distances[i][neighbor] = distances[neighbor][i] = distances[i][node] + 1;
                        q.push(neighbor);
                    }
                }
            }
        }

        return distances;
    }

    friend int getNearestLock(int key, BiDict& qubitMapping, const Graph& g);
};

int getNearestLock(int key, BiDict& qubitMapping, const Graph& g) {
    int n = g.graph.size();
    std::queue<int> q;
    q.push(key);
    qubitMapping.lock[key] = true;
    std::vector<bool> visited(n, false);
    visited[key] = true;

    // for(auto i : qubitMapping.lock) {
    //     printf("%d ", i ? 1 : 0);
    // }
    // printf("\n");

    while (!q.empty()) {
        int node = q.front();
        // printf("check phyqubit %d\n", node);
        q.pop();
        if(!qubitMapping.lock[node]) {
            qubitMapping.lock[node] = true;
            // for(auto i : qubitMapping.lock) {
            //     printf("%d ", i ? 1 : 0);
            // }
            // printf("\n");
            return node;
        } else {
            for (const auto& neighbor : g.graph[node]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }
    }

    return -1;
}



void swapQubit(BiDict& qubitMapping, std::pair<int, int>& gate) {
    std::swap(qubitMapping.forward[gate.first], qubitMapping.forward[gate.second]);
    std::swap(qubitMapping.reverse[qubitMapping.forward[gate.first]], qubitMapping.reverse[qubitMapping.forward[gate.second]]);
}

// Function declarations
std::vector<std::pair<int, std::pair<int, int>>> sabresSwap(const std::vector<std::pair<int, int>>& gates, const std::vector<std::pair<int, int>>& dependencies, const Graph& g, BiDict& qubitMapping, int logQubits, int numGates);

int main() {
    std::srand(42);
    // std::ios::sync_with_stdio(false);
    // std::cin.tie(nullptr);
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
    // for (int i = 0; i < logQubits; ++i) {
    //     qubitMapping.setItem(i, i);
    // }

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


std::vector<std::pair<int, std::pair<int, int>>> sabresSwap(const std::vector<std::pair<int, int>>& gates, const std::vector<std::pair<int, int>>& dependencies, const Graph& g, BiDict& qubitMapping, int logQubits, int numGates) {
    
    std::vector<std::vector<int>> dependencyGraph(numGates);
    std::vector<int> inDegree(numGates, 0);

    std::vector<std::vector<int>> allPairDistance = g.allPairDistances();

    BiDict originQubitMap(logQubits);
    
    // random assign qubit mapping
    for (int i = 0; i < logQubits; ++i) {
        qubitMapping.set(i, -1);
    }

    // for (int i = 0; i < logQubits; ++i) {
    //     std::cout << i + 1 << ' ' << qubitMapping.getItem(i) + 1 << '\n';
    // }

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
            int logSrc = gates[idx].first;
            int logDst = gates[idx].second;
            int nearestNeighbor;

            if(qubitMapping.lock[logSrc]) {
                // find the first phybit without lock
                for(int i=0; i<qubitMapping.size(); i++) {
                    if(!qubitMapping.lock[i]) {
                        nearestNeighbor = i;
                        break;
                    }
                }
                qubitMapping.lockUp(logSrc, nearestNeighbor);
                qubitMapping.setItem(logSrc, nearestNeighbor);
                originQubitMap.setItem(logSrc, nearestNeighbor);
            } else {
                
                qubitMapping.lockUp(logSrc, logSrc);
                qubitMapping.setItem(logSrc, logSrc);
                originQubitMap.setItem(logSrc, logSrc);
            }

            nearestNeighbor = getNearestLock(qubitMapping.getItem(logSrc), qubitMapping, g);
            qubitMapping.lockUp(logDst, nearestNeighbor);
            qubitMapping.setItem(logDst, nearestNeighbor);
            originQubitMap.setItem(logSrc, nearestNeighbor);
            // operations.push_back(std::make_pair(1, std::make_pair(logSrc, logDst)));

            // printf("set mapping %d->%d, %d-> %d\n", logSrc, logSrc, logDst, nearestNeighbor);
        }
    }
    

    while (true) {
        // printf("check queue size %d\n", checkQueue.size());

        while (!checkQueue.empty()) {
            int candidate = checkQueue.front();
            checkQueue.pop();
            int logSrc = gates[candidate].first;
            int logDst = gates[candidate].second;
            // printf("check candidate %d, (%d, %d)\n", candidate + 1, logSrc, logDst);

            if(qubitMapping.lockReverse[logSrc] && qubitMapping.lockReverse[logDst]) {
                // printf("direct push candidate %d\n", candidate + 1);
            } else {
                // printf("locked? %d %d\n", qubitMapping.lockReverse[logSrc] ? 1 : 0, qubitMapping.lockReverse[logDst] ? 1 : 0);
                int lockedQubit = qubitMapping.lockReverse[logSrc] ? logSrc : logDst;
                int movingQubit = qubitMapping.lockReverse[logSrc] ? logDst : logSrc;
                // printf("locke phybit %d\n", qubitMapping.getItem(lockedQubit));
                int neighbor = getNearestLock(qubitMapping.getItem(lockedQubit), qubitMapping, g);
                qubitMapping.lockUp(movingQubit, neighbor);
                qubitMapping.setItem(movingQubit, neighbor);
                originQubitMap.setItem(movingQubit, neighbor);
                // printf("set mapping %d->%d\n", movingQubit, neighbor);
            }

            if(allPairDistance[qubitMapping.getItem(logSrc)][qubitMapping.getItem(logDst)] == 1) {
                operations.push_back(std::make_pair(1, std::make_pair(logSrc, logDst)));

                for (int v : dependencyGraph[candidate]) {
                    inDegree[v]--;
                    if (inDegree[v] == 0) {
                        // printf("new candidate %d\n", v + 1);
                        checkQueue.push(v);
                    }
                }
            } else {
                executableQueue.push_back(candidate);
            }
        }

        // break;

        if (executableQueue.empty()) {
            break;
        }


        std::pair<int, int> bestSwap = std::make_pair(-1, -1);
        std::unordered_map<int, int> swapMap;

        for(const auto& gateId : executableQueue) {
            const auto& gate = gates[gateId];
            const auto a = qubitMapping.getItem(gate.first), b = qubitMapping.getItem(gate.second);
            const auto distance = allPairDistance[a][b];
            // printf("(%d, %d), distance %d\n", gate.first, gate.second, distance);

            const auto& neighbors = g.getNeighbor(a);

            for(const auto& neighbor : neighbors) {
                if(distance - allPairDistance[b][neighbor] > 0) {
                    bestSwap = std::make_pair(a, neighbor);
                    break;
                }
            }

            if(bestSwap.first != -1) break;
        }

        bestSwap.first = qubitMapping.getReverseItem(bestSwap.first), bestSwap.second = qubitMapping.getReverseItem(bestSwap.second);
        // printf("best swap (%d, %d)\n", bestSwap.first, bestSwap.second);
        swapQubit(qubitMapping, bestSwap);
        operations.push_back(std::make_pair(0, bestSwap));

        for (int gate : executableQueue) {
            checkQueue.push(gate);
        }
        executableQueue.clear();
    }

    for (int i = 0; i < logQubits; ++i) {
        std::cout << i + 1 << ' ' << originQubitMap.getItem(i) + 1 << '\n';
    }


    return operations;
}