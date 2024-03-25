import networkx as nx
from collections import deque, defaultdict
from copy import deepcopy

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
#----------------------------------------


# initialize graph and in_degree
graph = defaultdict(list)
in_degree = defaultdict(int)

for u, v in dependencies:
    graph[u].append(v)
    in_degree[v] += 1

qubit_mapping = BiDict()
for i in range(logQubits):
    qubit_mapping[i] = i 

for key, value in qubit_mapping.forward.items():
    print(f"{key + 1} {value + 1}")

# get all pair paths
all_pair_paths = dict(nx.all_pairs_shortest_path(G))


queue = set([u for u in graph if in_degree[u] == 0])

def swap_qubit(qubit_mapping: BiDict, gate: tuple[int, int]):
    temp = qubit_mapping[gate[0]]
    qubit_mapping[gate[0]] = qubit_mapping[gate[1]]
    qubit_mapping[gate[1]] = temp


# perform topological sort
result = []
has_excutable = True
# N = 30
operations = []
print_flag = False
puninsh_swap = (-1, -1)
punishment_swaps = deque()

while queue:
    has_excutable = True

    while has_excutable:
        has_excutable = False
        for candidate in list(queue):
            log_src, log_dst = gates[candidate]
            if len(all_pair_paths[qubit_mapping[log_src]][qubit_mapping[log_dst]]) == 2:
                has_excutable = True
                # print(f"excutable {(log_src, log_dst)}")
                # if log_src == 73 and log_dst == 362:
                #     print_flag = True
                operations.append((1, log_src, log_dst)) # cnot
                queue.remove(candidate)

                for v in graph[candidate]:
                    in_degree[v] -= 1
                    if in_degree[v] == 0:
                        queue.add(v)
            # elif len(all_pair_paths[qubit_mapping[log_src]][qubit_mapping[log_dst]]) < 2:
            #     print("error")
                        
            # if len(queue) >= 5:
            #     has_excutable = False
            #     break

    candidate_swap = set()
    
    for candidate in queue:
        log_src, log_dst = gates[candidate]
        # if print_flag:
        #     print(f"not excutable {(log_src, log_dst)}, finding swap")
        #     print(f"distance: {len(all_pair_paths[qubit_mapping[log_src]][qubit_mapping[log_dst]])}, {all_pair_paths[qubit_mapping[log_src]][qubit_mapping[log_dst]]}")
        phy_src, phy_dst = qubit_mapping[log_src], qubit_mapping[log_dst]
        candidate_swap.update({(log_src, qubit_mapping.reverse[neighbor]) for neighbor in G[phy_src]})
        candidate_swap.update({(log_dst, qubit_mapping.reverse[neighbor]) for neighbor in G[phy_dst]})

    for puninsh_swap in punishment_swaps:
        if puninsh_swap in candidate_swap:
            candidate_swap.remove(puninsh_swap)
    
    # if print_flag:
    #     print(len(queue))
    #     print(queue)
        # exit(0)

    # deal with candidate swap
    clear_queue = deepcopy(queue)
    while len(clear_queue):
        best_score, best_swap = 1e9, None
        for swap in candidate_swap:
            swap_qubit(qubit_mapping, swap)
            score = 0
            for candidate in clear_queue:
                src, dst = gates[candidate]
                score += len(all_pair_paths[qubit_mapping[src]][qubit_mapping[dst]])
            if score <= best_score:
                best_score = score
                best_swap = swap
            # if print_flag:
            #     print(f"swap: {swap} score: {score}")
            swap_qubit(qubit_mapping, swap)

        if best_score != 1e9:
            # if print_flag:
            #     print(f"best swap: {best_swap} best score: {best_score}")
                # exit(0)
            swap_qubit(qubit_mapping, best_swap)

            punishment_swaps.append(best_swap)
            if len(punishment_swaps) >= 4:
                punishment_swaps.popleft()
            
            operations.append((0, best_swap[0], best_swap[1])) # swap

        for candidate in list(clear_queue):
            src, dst = gates[candidate]
            if len(all_pair_paths[qubit_mapping[src]][qubit_mapping[dst]]):
                clear_queue.remove(candidate)


        


# print("finish")

for op in operations:
    op_type, src, dst = op
    print(f"{'CNOT' if op_type else 'SWAP'} q{src + 1} q{dst + 1}")


