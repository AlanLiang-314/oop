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
#include <chrono>


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
    friend void swapPhyQubit(BiDict& qubitMapping, std::pair<int, int> gate);
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

    std::vector<int> getNeighbor(int node) const {
        return graph[node];
    }

    std::vector<std::vector<int>> allPairDistances(std::vector<std::vector<std::vector<int>>>& paths) const {
        std::vector<std::vector<int>> distances(paths.size(), std::vector<int>(paths.size(), 0));

        for (int i = 0; i < paths.size(); i++) {
            for (int j = 0; j < paths[i].size(); j++) {
                const auto distance =  paths[i][j].size();
                distances[i][j] = distance == 0 ? 0 : distance - 1;
            }
        }

        return distances;
    }
    

    std::vector<std::vector<std::vector<int>>> allPairShortestPaths() const {
        int n = graph.size();
        std::vector<std::vector<std::vector<int>>> result(n, std::vector<std::vector<int>>(n));

        for (int i = 0; i < n; i++) {
            std::queue<std::vector<int>> q;
            q.push({i});
            std::vector<bool> visited(n, false);
            visited[i] = true;
            int distance = 0;

            while (!q.empty()) {
                int size = q.size();
                for (int j = 0; j < size; j++) {
                    auto path = q.front();
                    q.pop();
                    int node = path.back();
                    for (int neighbor : graph[node]) {
                        if (!visited[neighbor]) {
                            visited[neighbor] = true;
                            std::vector<int> new_path = path;
                            new_path.push_back(neighbor);
                            result[i][neighbor] = new_path;
                            q.push(new_path);
                        }
                    }
                }
                distance++;
            }
        }

        return result;
    }
};

void swapQubit(BiDict& qubitMapping, std::pair<int, int> gate) {
    std::swap(qubitMapping.forward[gate.first], qubitMapping.forward[gate.second]);
    std::swap(qubitMapping.reverse[qubitMapping.forward[gate.first]], qubitMapping.reverse[qubitMapping.forward[gate.second]]);
}

void swapPhyQubit(BiDict& qubitMapping, std::pair<int, int> gate) {
    std::swap(qubitMapping.reverse[gate.first], qubitMapping.reverse[gate.second]);
    std::swap(qubitMapping.forward[qubitMapping.reverse[gate.first]], qubitMapping.forward[qubitMapping.reverse[gate.second]]);
}

// Function declarations
std::vector<std::pair<int, std::pair<int, int>>> newSwap(const std::vector<std::pair<int, int>>& gates, const std::vector<std::pair<int, int>>& dependencies, const Graph& g, BiDict& qubitMapping, int logQubits, int numGates);

int main() {
    std::srand(42);
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

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

    std::vector<std::pair<int, std::pair<int, int>>> operations = newSwap(gates, dependencies, g, qubitMapping, logQubits, numGates);

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

std::vector<std::pair<int, std::pair<int, int>>> newSwap(const std::vector<std::pair<int, int>>& gates, const std::vector<std::pair<int, int>>& dependencies, const Graph& g, BiDict& qubitMapping, int logQubits, int numGates) {
    std::vector<std::vector<int>> dependencyGraph(numGates);
    std::vector<int> inDegree(numGates, 0);
    std::vector<std::pair<int, std::pair<int, int>>> operations;
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<std::vector<int>>> paths = g.allPairShortestPaths();
    std::vector<std::vector<int>> allPairDistance = g.allPairDistances(paths);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // std::cout << "find path: " << duration.count() << " microseconds\n";
    

    // start = std::chrono::high_resolution_clock::now();
    // allocateQubit(gates, allPairDistance, g, qubitMapping, logQubits);
    // end = std::chrono::high_resolution_clock::now();
    // duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // std::cout << "allocated: " << duration.count() << " microseconds\n";

    for (int i = 0; i < logQubits; ++i) {
        qubitMapping.setItem(i, i);
        std::cout << i + 1 << ' ' << qubitMapping.getItem(i) + 1 << '\n';
    }


    for (const auto& dependency : dependencies) {
        int u = dependency.first;
        int v = dependency.second;
        dependencyGraph[u].push_back(v);
        inDegree[v]++;
    }

    std::queue<int> checkQueue;
    std::vector<int> executableQueue;
    std::vector<int> futureQueue;

    for (int idx = 0; idx < numGates; idx++) {
        if (inDegree[idx] == 0) {
            checkQueue.push(idx);
        }
    }

    start = std::chrono::high_resolution_clock::now();
    while (true) {
        while (!checkQueue.empty()) {
            int candidate = checkQueue.front();
            checkQueue.pop();
            int logSrc = gates[candidate].first;
            int logDst = gates[candidate].second;
            if (allPairDistance[qubitMapping.getItem(logSrc)][qubitMapping.getItem(logDst)] == 1) {
                operations.push_back(std::make_pair(1, std::make_pair(logSrc, logDst)));

                for (int v : dependencyGraph[candidate]) {
                    inDegree[v]--;
                    if (inDegree[v] == 0) {
                        checkQueue.push(v);
                    }
                }

            } else {
                executableQueue.push_back(candidate);
            }
        }

        if (executableQueue.empty()) {
            break;
        }

        // futureQueue.clear();
        for(const auto& candidate : executableQueue) {
            int logSrc = gates[candidate].first;
            int logDst = gates[candidate].second;
            std::vector<int> path = paths[qubitMapping.getItem(logSrc)][qubitMapping.getItem(logDst)];
            // printf("solving gate %d, (%d, %d)\n", candidate, logSrc, logDst);
            for(int i=0;i< path.size() - 2; i++) {
                int src = path[i];
                int dst = path[i + 1];
                int logSrc = qubitMapping.getReverseItem(src);
                int logDst = qubitMapping.getReverseItem(dst);
                operations.push_back(std::make_pair(0, std::make_pair(logSrc, logDst)));
                swapQubit(qubitMapping, std::make_pair(logSrc, logDst));
            }
            assert(allPairDistance[qubitMapping.getItem(logSrc)][qubitMapping.getItem(logDst)] == 1);
        }

        for (int gate : executableQueue) {
            checkQueue.push(gate);
        }
        executableQueue.clear();

    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // std::cout << "excute: " << duration.count() << " microseconds\n";
    

    return operations;
}
