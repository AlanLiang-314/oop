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

#define printf // 

class BiDict {
private:
    std::vector<int> forward;
    std::vector<int> reverse;

public:
    BiDict(int size) : forward(size), reverse(size) {}

    void set(int key, int value) {
        forward[key] = value;
        reverse[key] = value;
    }

    void setItem(int key, int value) {
        forward[key] = value;
        reverse[value] = key;
    }

    int getItem(int key) {
        // printf("touch %d->%d, locked %d\n", key, forward[key], lockReverse[key] ? 1: 0);
        return forward[key];
    }

    int getReverseItem(int key) {
        return reverse[key];
    }

    int size() {
        return forward.size();
    }

    void printMapping() {
        int i = 0;
        for(auto item : forward) {
            printf("%d: %d, ", i, item);
            i++;
        }
        printf("\n");
        i = 0;
        for(auto item : reverse) {
            printf("%d: %d, ", i, item);
            i++;
        }
        printf("\n");
    }

    const auto& getForward() {
        return forward;
    }

    friend void swapQubit(BiDict& qubitMapping, std::pair<int, int>& gate);
    friend void swapTmpQubit(BiDict& qubitMapping, std::pair<int, int>& gate);
    friend void swapPhyQubit(BiDict& qubitMapping, std::pair<int, int>& gate);
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
        for(size_t i=0; i< inDegree.size(); i++) {
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

    friend int getNearestQubit(int key, BiDict& qubitMapping, const Graph& g);
};

int getNearestQubit(int key, BiDict& qubitMapping, const Graph& g) {
    int n = g.graph.size();
    std::queue<int> q;
    q.push(key);
    std::vector<bool> visited(n, false);
    visited[key] = true;

    while (!q.empty()) {
        int node = q.front();
        q.pop();
        if(qubitMapping.getReverseItem(node) == -1) {
            return node;
        } else {
            for (const auto& neighbor : g.graph[node]) {
                // printf("%d neighbor %d\n", node, neighbor);
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

void swapPhyQubit(BiDict& qubitMapping, std::pair<int, int>& gate) {
    std::swap(qubitMapping.reverse[gate.first], qubitMapping.reverse[gate.second]);
    std::swap(qubitMapping.forward[qubitMapping.reverse[gate.first]], qubitMapping.forward[qubitMapping.reverse[gate.second]]);
}


void swapTmpQubit(BiDict& qubitMapping, std::pair<int, int>& gate) {
    int unknown, known;
    if(qubitMapping.reverse[gate.first] == -1) {
        unknown = gate.first;
        known = gate.second;
    } else {
        unknown = gate.second;
        known = gate.first;
    }
    // printf("known: %d, unknown %d\n", known, unknown);
    qubitMapping.forward[qubitMapping.reverse[known]] = unknown;
    std::swap(qubitMapping.reverse[gate.first], qubitMapping.reverse[gate.second]);
    // qubitMapping.printMapping();
}

std::vector<std::pair<int, std::pair<int, int>>> sabresSwap(const std::vector<std::pair<int, int>>& gates, const std::vector<std::pair<int, int>>& dependencies, const Graph& g, BiDict& qubitMapping, int logQubits, int numGates) {
    
    std::vector<std::vector<int>> dependencyGraph(numGates);
    std::vector<int> inDegree(numGates, 0);

    std::vector<std::vector<int>> allPairDistance = g.allPairDistances();
    
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

    // find all gate id without dependency
    for (int idx = 0; idx < numGates; idx++) {
        // printf("idx %d indegree %d\n", idx, inDegree[idx]);
        if (inDegree[idx] == 0) {
            checkQueue.push(idx);
            int logSrc = gates[idx].first;
            int logDst = gates[idx].second;
            int nearestNeighbor;
            // printf("log src %d -> %d\n", logSrc, qubitMapping.getReverseItem(logSrc));
            if(qubitMapping.getReverseItem(logSrc) != -1) {
                // find the first phybit without lock
                for(int i=0; i<qubitMapping.size(); i++) {
                    if(qubitMapping.getReverseItem(i) == -1) {
                        nearestNeighbor = i;
                        // printf("neighbor %d\n", nearestNeighbor);
                        break;
                    }
                }
                qubitMapping.setItem(logSrc, nearestNeighbor);
            } else {
                qubitMapping.setItem(logSrc, logSrc);
            }

            nearestNeighbor = getNearestQubit(qubitMapping.getItem(logSrc), qubitMapping, g);
            // qubitMapping.lockUp(logDst, nearestNeighbor);
            qubitMapping.setItem(logDst, nearestNeighbor);
            // qubitMapping.printMapping();

            printf("set mapping %d->%d, %d->%d\n", logSrc, qubitMapping.getItem(logSrc), logDst, nearestNeighbor);
            qubitMapping.printMapping();
            // printLock(qubitMapping);
        }
    }
    

    while (true) {
        printf("check queue size %d\n", checkQueue.size());

        while (!checkQueue.empty()) {
            int candidate = checkQueue.front();
            checkQueue.pop();
            int logSrc = gates[candidate].first;
            int logDst = gates[candidate].second;
            int phySrc = qubitMapping.getItem(logSrc);
            int phyDst = qubitMapping.getItem(logDst);
            printf("check candidate %d, (%d, %d)\n", candidate, logSrc, logDst);
            // printf("phy src %d phy dst %d\n", phySrc, phyDst);

            if(phySrc != -1 && phyDst != -1) {
                // printf("direct push candidate %d\n", candidate + 1);
            } else if(phySrc == -1 && phyDst == -1) {
                // place first qubit at corresponding place and find neighbor
                int neighbor;
                for(int i=0; i<qubitMapping.size(); i++) {
                    if(qubitMapping.getReverseItem(i) == -1) {
                        neighbor = i;
                        break;
                    }
                }
                qubitMapping.setItem(logSrc, neighbor);
                neighbor = getNearestQubit(qubitMapping.getItem(logSrc), qubitMapping, g);
                assert(neighbor != -1);
                qubitMapping.setItem(logDst, neighbor);
                printf("set mapping %d->%d, %d->%d\n", logSrc, qubitMapping.getItem(logSrc), logDst, neighbor);

            } else {
                int lockedQubit = (phyDst == -1) ? logSrc : logDst;
                int movingQubit = (phyDst == -1) ? logDst : logSrc;
                // printf("locked phybit %d, move phybit %d, lock qubit %d, move qubit %d\n", qubitMapping.getItem(lockedQubit), qubitMapping.getItem(movingQubit), lockedQubit, movingQubit);
                int neighbor = getNearestQubit(qubitMapping.getItem(lockedQubit), qubitMapping, g);
                assert(neighbor != -1);
                qubitMapping.setItem(movingQubit, neighbor);
                // qubitMapping.printMapping();
                printf("set mapping %d->%d, distance %d\n", movingQubit, neighbor, allPairDistance[qubitMapping.getItem(lockedQubit)][neighbor]);
                qubitMapping.printMapping();
            }

            if(allPairDistance[qubitMapping.getItem(logSrc)][qubitMapping.getItem(logDst)] == 1) {
                operations.push_back(std::make_pair(0, std::make_pair(logSrc, logDst)));

                for (int v : dependencyGraph[candidate]) {
                    inDegree[v]--;
                    if (inDegree[v] == 0) {
                        printf("new candidate %d\n", v);
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


        std::pair<int, int> bestSwap = std::make_pair(-2, -2), bestLogSwap;
        // std::unordered_map<int, int> swapMap;

        for(const auto& gateId : executableQueue) {
            const auto& gate = gates[gateId];
            const auto a = qubitMapping.getItem(gate.first), b = qubitMapping.getItem(gate.second);
            const auto distance = allPairDistance[a][b];
            // printf("(%d, %d), distance %d\n", gate.first, gate.second, distance);

            // {
            // const auto& neighbors = g.getNeighbor(b);

            // for(const auto& neighbor : neighbors) {
            //     // printf("candidate swap (%d, %d)\n", a, neighbor);
            //     if(distance - allPairDistance[a][neighbor] > 0) {
            //         bestSwap = std::make_pair(b, neighbor);
            //         break;
            //     }
            // }
            // }

            {
            const auto& neighbors = g.getNeighbor(a);

            for(const auto& neighbor : neighbors) {
                // printf("candidate swap (%d, %d)\n", a, neighbor);
                if(distance - allPairDistance[b][neighbor] > 0) {
                    bestSwap = std::make_pair(a, neighbor);
                    break;
                }
            }
            }

            // if(bestSwap.first != -2) break;
        }

        assert(bestSwap.first != -2);

        // printf("best swap (%d, %d)\n", bestSwap.first, bestSwap.second);
        bestLogSwap = std::make_pair(qubitMapping.getReverseItem(bestSwap.first), qubitMapping.getReverseItem(bestSwap.second));
        // printf("best swap (%d, %d)\n", bestLogSwap.first, bestLogSwap.second);
        if(bestLogSwap.first == -1 || bestLogSwap.second == -1) {
            swapTmpQubit(qubitMapping, bestSwap);
            operations.push_back(std::make_pair(2, bestSwap));
        } else {
            swapQubit(qubitMapping, bestLogSwap);
            operations.push_back(std::make_pair(1, bestLogSwap));
        }
        // qubitMapping.printMapping();

        for (int gate : executableQueue) {
            checkQueue.push(gate);
        }
        executableQueue.clear();
    }

    // for (int i = 0; i < logQubits; ++i) {
    //     std::cout << i + 1 << ' ' << originQubitMap.getItem(i) + 1 << '\n';
    // }


    return operations;
}


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

    // fill logQubit that does not have correspond phyQubit
    const auto& forward = qubitMapping.getForward();
    std::unordered_set<int> unused(forward.size());
    std::vector<int> unmatch;
    for(size_t i=0;i< forward.size();i++) unused.insert(i); 
    for(size_t i = 0;i<forward.size();i++) {
        if(forward[i] == -1) {
            unmatch.push_back(i);
        } else {
            unused.erase(forward[i]);
        }
    }
    assert(unmatch.size() == unused.size());

    int idx = 0;
    for(const auto& phyQubit : unused) {
        qubitMapping.setItem(unmatch[idx], phyQubit);
        idx++;
    }

    // swap back to get origin
    for(auto op = operations.rbegin(); op != operations.rend(); op++) {
        if(op->first == 1) {
            // printf("swap (%d, %d)\n", op->second.first, op->second.second);
            swapQubit(qubitMapping, op->second);
            // qubitMapping.printMapping();
        } else if(op->first == 2) {
            // printf("swap (%d, %d)\n", op->second.first, op->second.second);
            swapPhyQubit(qubitMapping, op->second);
            op->second.first = qubitMapping.getReverseItem(op->second.first);
            op->second.second = qubitMapping.getReverseItem(op->second.second);
            // qubitMapping.printMapping();
        }
    }

    for (int i = 0; i < logQubits; ++i) {
        // std::cout << i << " " << qubitMapping.getItem(i) << std::endl;
        std::cout << i + 1 << " " << qubitMapping.getItem(i) + 1 << std::endl;
    }

    for (const auto& op : operations) {
        if (op.first == 0) {
            std::cout << "CNOT q" << op.second.first + 1 << " q" << op.second.second + 1 << '\n';
        } else {
            std::cout << "SWAP q" << op.second.first + 1 << " q" << op.second.second + 1 << '\n';
        }
    }

    return 0;
}