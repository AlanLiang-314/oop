from collections import defaultdict
import math

class BiDict:
    def __init__(self):
        self.forward = {}  # key -> value
        self.reverse = {}  # value -> key

    def __setitem__(self, key, value):
        self.forward[key] = value
        self.reverse[value] = key

    def __getitem__(self, key):
        return self.forward[key]

    def __len__(self):
        return len(self.forward)

    def __iter__(self):
        return iter(self.forward)

class Graph:
    def __init__(self):
        self.graph = {}

    def add_edge(self, node1, node2):
        if node1 not in self.graph:
            self.graph[node1] = set()
        if node2 not in self.graph:
            self.graph[node2] = set()
        self.graph[node1].add(node2)
        self.graph[node2].add(node1)

    def are_neighbors(self, node1, node2):
        if node1 in self.graph and node2 in self.graph:
            return node2 in self.graph[node1]
        return False
    
    def get_nodes(self):
        return list(self.graph.keys())

    def all_pair_distances(self):
        nodes = self.get_nodes()
        dist = {node: {other: (0 if node == other else math.inf) for other in nodes} for node in nodes}

        for node1 in nodes:
            for node2 in self.graph[node1]:
                dist[node1][node2] = 1

        for k in nodes:
            for i in nodes:
                for j in nodes:
                    dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j])

        return dist
    
def swap_qubit(qubit_mapping: BiDict, gate: tuple[int, int]):
    temp = qubit_mapping[gate[0]]
    qubit_mapping[gate[0]] = qubit_mapping[gate[1]]
    qubit_mapping[gate[1]] = temp

#-----------------input------------------
logQubits, num_gates, num_dependencies, phyQubits, num_phyLinks = map(int, input().split())
gates = []
dependencies = []

for _ in range(num_gates):
    _, srcbit, dstbit = map(int, input().split())
    gates.append((srcbit - 1, dstbit - 1))

for _ in range(num_dependencies):
    _, srcgate, tgtgate = map(int, input().split())
    dependencies.append((srcgate - 1, tgtgate - 1))

G = Graph()
for _ in range(num_phyLinks):
    _, src, dst = map(int, input().split())
    G.add_edge(src - 1, dst - 1)
#----------------------------------------

#---------------init---------------------
graph = defaultdict(list)
in_degree = defaultdict(int)

for u, v in dependencies:
    graph[u].append(v)
    in_degree[v] += 1

qubit_mapping = BiDict()
for i in range(logQubits):
    qubit_mapping[i] = i 

all_pair_distance = G.all_pair_distances()
#-----------------------------------------

for key, value in qubit_mapping.forward.items():
    print(f"{key + 1} {value + 1}")

queue = set([u for u in graph if in_degree[u] == 0])


operations = []
while queue:
    has_excutable = True

    while has_excutable:
        has_excutable = False
        for candidate in list(queue):
            log_src, log_dst = gates[candidate]
            if all_pair_distance[qubit_mapping[log_src]][qubit_mapping[log_dst]] == 2:
                has_excutable = True
                # print(f"excutable {(log_src, log_dst)}")
                operations.append((1, log_src, log_dst)) # cnot
                queue.remove(candidate)

                for v in graph[candidate]:
                    in_degree[v] -= 1
                    if in_degree[v] == 0:
                        queue.add(v)

    candidate_swap = set()

    for candidate in queue:
        log_src, log_dst = gates[candidate]
        phy_src, phy_dst = qubit_mapping[log_src], qubit_mapping[log_dst]
        candidate_swap.update({(log_src, qubit_mapping.reverse[neighbor]) for neighbor in G.graph[phy_src]})
        candidate_swap.update({(log_dst, qubit_mapping.reverse[neighbor]) for neighbor in G.graph[phy_dst]})

    

    print(candidate_swap)
    exit(0)