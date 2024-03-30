from collections import defaultdict, deque
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


def select_swap(candidate_swap, queue, future_queue, all_pair_distance, gates, score_strategy: str = "basic"):
    best_score, best_swap, best_solved_gate = 1e9, None, None

    for swap in candidate_swap:
        swap_qubit(qubit_mapping, swap)

        score = 0
        solved_gate = []
        for candidate in queue:
            src, dst = gates[candidate]
            score += all_pair_distance[qubit_mapping[src]][qubit_mapping[dst]]
            if all_pair_distance[qubit_mapping[src]][qubit_mapping[dst]] == 1:
                solved_gate.append(candidate)
        
        future_score = 0
        for candidate in future_queue:
            src, dst = gates[candidate]
            future_score += all_pair_distance[qubit_mapping[src]][qubit_mapping[dst]]

        # score = max(qubit_score[qubit_mapping[swap[0]]], qubit_score[qubit_mapping[swap[1]]]) * (score + 0.5 * future_score)
        score = (score + 0.5 * future_score)

        # print(f"swap {swap} score {score}")
        
        if score <= best_score:
            best_score = score
            best_swap = swap
            best_solved_gate = solved_gate
        swap_qubit(qubit_mapping, swap)

    if best_score != 1e9:
        return best_swap, best_solved_gate, best_score
    else:
        print("no best swap")
        exit(1)



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
    # in_degree[v] += 1

qubit_mapping = BiDict()
qubit_score = [1] * logQubits
for i in range(logQubits):
    qubit_mapping[i] = i

all_pair_distance = G.all_pair_distances()
#-----------------------------------------

# for key, value in qubit_mapping.forward.items():
#     print(f"{key + 1} {value + 1}")

def sabre_swap():
    in_degree = defaultdict(int)
    for u, v in dependencies:
        in_degree[v] += 1

    check_queue = deque([u for u in graph if in_degree[u] == 0])
    executable_queue = []

    operations = []
    # N = 10
    while True:
        
        while check_queue:
            # print(f"check queue {check_queue}")
            candidate = check_queue.popleft()
            log_src, log_dst = gates[candidate]
            if all_pair_distance[qubit_mapping[log_src]][qubit_mapping[log_dst]] == 1:
                operations.append((1, log_src, log_dst))

                for v in graph[candidate]:
                    in_degree[v] -= 1
                    if in_degree[v] == 0:
                        check_queue.append(v)
            
            else:
                executable_queue.append(candidate)

        # print(f"excuteable queue {executable_queue}")

        if not executable_queue:
            break
        
        future_queue = []
        for candidate in executable_queue:
            future_queue.extend(neighbor for neighbor in graph[candidate] if in_degree[neighbor] == 1)

        # print(f"future gate {future_queue}")

        best_solved_gate = []
        num_search_step = 0

        while not best_solved_gate:
            candidate_swap = set()

            for candidate in executable_queue:
                log_src, log_dst = gates[candidate]
                phy_src, phy_dst = qubit_mapping[log_src], qubit_mapping[log_dst]
                candidate_swap.update({(log_src, qubit_mapping.reverse[neighbor]) for neighbor in G.graph[phy_src]})
                candidate_swap.update({(log_dst, qubit_mapping.reverse[neighbor]) for neighbor in G.graph[phy_dst]})


            best_swap, best_solved_gate, best_score = select_swap(candidate_swap, executable_queue, future_queue,
                                                                all_pair_distance, gates)

            # if num_search_step > 5:
            #     qubit_score = [1] * logQubits
            #     num_search_step = 0
            # else:
            #     # use logical or physical?
            #     qubit_score[qubit_mapping[best_swap[0]]] += 0.01
            #     qubit_score[qubit_mapping[best_swap[1]]] += 0.01
            
            # num_search_step += 1
            swap_qubit(qubit_mapping, best_swap)
            operations.append((0, best_swap[0], best_swap[1]))
        
        executable_queue = [item for item in executable_queue if item not in best_solved_gate]
        check_queue.extend(best_solved_gate)

        # print(f"best solved gate: {best_solved_gate}")
        # N -= 1

    return operations

for _ in range(1):
    operations = sabre_swap()
    for op in operations[::-1]:
        op_type, src, dst = op
        if op_type:
            swap_qubit(qubit_mapping, (src, dst))



for key, value in qubit_mapping.forward.items():
    print(f"{key + 1} {value + 1}")

operations = sabre_swap()

for op in operations:
    op_type, src, dst = op
    print(f"{'CNOT' if op_type else 'SWAP'} q{src + 1} q{dst + 1}")
