import networkx as nx
from collections import deque, defaultdict

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

G = nx.Graph()
for _ in range(num_phyLinks):
    _, src, dst = map(int, input().split())
    G.add_edge(src - 1, dst - 1)
#-----------------input------------------


# initialize graph and in_degree
graph = defaultdict(list)
in_degree = defaultdict(int)

for u, v in dependencies:
    graph[u].append(v)
    in_degree[v] += 1

queue = deque([u for u in graph if in_degree[u] == 0])

# topological sort
result = []
while queue:
    u = queue.popleft()
    result.append(u)

    for v in graph[u]:
        in_degree[v] -= 1
        if in_degree[v] == 0:
            queue.append(v)

assert len(result) == len(graph), "cycle detected!"

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

    def reverse_get(self, value):
        if value in self.reverse:
            return self.reverse[value]
        else:
            raise KeyError(f"Value {value} not found in the bidict")
    
# qubit_mapping = {i:i for i in range(logQubits)} # logical -> physical
qubit_mapping = BiDict()
for i in range(logQubits):
    qubit_mapping[i] = i 

for key, value in qubit_mapping.forward.items():
    print(f"{key + 1} {value + 1}")

# get all pair paths
all_pair_paths = dict(nx.all_pairs_shortest_path(G))

operations = []


for id in result:
    logical_srcbit, logical_dstbit = gates[id]
    phy_srcbit, phy_dstbit = qubit_mapping[logical_srcbit], qubit_mapping[logical_dstbit]
    path = all_pair_paths[phy_srcbit][phy_dstbit]

    if len(path) > 2:
        for a, b in zip(path[:-2], path[1:]):
            temp_a, temp_b = qubit_mapping.reverse[a], qubit_mapping.reverse[b]
            qubit_mapping[temp_a] = b
            qubit_mapping[temp_b] = a
            operations.append((0, temp_a, temp_b)) # swap

        phy_srcbit, phy_dstbit = qubit_mapping[logical_srcbit], qubit_mapping[logical_dstbit]

        path = all_pair_paths[phy_srcbit][phy_dstbit]
        assert len(path) == 2, "not a neighbor!"
    
    operations.append((1, logical_srcbit, logical_dstbit)) # cnot


for op in operations:
    op_type, src, dst = op
    print(f"{'CNOT' if op_type else 'SWAP'} q{src + 1} q{dst + 1}")

