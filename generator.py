import random
import networkx as nx
import os

seeds: int = 1
testset_type: str = "large_v2"

if not os.path.exists(os.path.join("testset", testset_type)):
    os.makedirs(os.path.join("testset", testset_type))

for seeds in range(1, 21):
    random.seed(seeds)

    # large
    logQubits = random.randint(5000, 7000)
    gates = random.randint(8000, 10000)

    # medium
    # logQubits = random.randint(1000, 2000)
    # gates = random.randint(2000, 4000)

    # small
    # logQubits = random.randint(100, 500) # 8
    # gates = random.randint(1000, 2000)

    # xsmall
    # logQubits = random.randint(100, 300)
    # gates = random.randint(250, 600)

    # tiny
    # logQubits = random.randint(4, 10) # 8
    # gates = random.randint(8, 16)


    phyLinks = (logQubits - 1) + random.randint(0, 2 * logQubits)
    phyLinks = min(phyLinks, (logQubits * (logQubits - 1)) // 2)
    phyQubits = logQubits

    gates_id = []
    last_gate_using_bit = [None] * logQubits
    dependencies = []

    for i in range(gates):
        src_bit, dst_bit = random.sample(range(logQubits), 2)
        if src_bit > dst_bit:
            temp = src_bit
            src_bit = dst_bit
            dst_bit = temp

        gates_id.append((src_bit, dst_bit))

        if last_gate_using_bit[src_bit] is not None:
            dependencies.append((last_gate_using_bit[src_bit], i))
        if last_gate_using_bit[dst_bit] is not None and last_gate_using_bit[dst_bit] != last_gate_using_bit[src_bit]:
            dependencies.append((last_gate_using_bit[dst_bit], i))

        last_gate_using_bit[src_bit] = i
        last_gate_using_bit[dst_bit] = i


    def generate_graph(n, m):
        G = nx.Graph()
        G.add_nodes_from(range(n))
        
        for i in range(n):
            G.add_edge(i, (i+1)%n)
        
        nodes = list(range(n))
        while G.number_of_edges() < m:
            # print(G.number_of_edges(), m, n)
            u, v = random.sample(nodes, 2)
            if not G.has_edge(u, v):
                G.add_edge(u, v)
        
        return G

    G = generate_graph(phyQubits, phyLinks)

    with open(f"testset/{testset_type}/{str(seeds).zfill(3)}.txt", 'w', encoding="utf-8") as f:
        f.write(f"{logQubits} {gates} {len(dependencies)} {phyQubits} {phyLinks}\n")
        for i, gate in enumerate(gates_id):
            f.write(f"{i + 1} {gate[0] + 1} {gate[1] + 1}\n")

        for i, item in enumerate(dependencies):
            f.write(f"{i + 1} {item[0] + 1} {item[1] + 1}\n")

        for i, item in enumerate(G.edges):
            f.write(f"{i + 1} {item[0] + 1} {item[1] + 1}\n")


# for plot generation
# from qiskit import QuantumCircuit
# nx.draw(G, with_labels=True)
# plt.show()
# qc = QuantumCircuit(logQbits)

# for src, dst in gates_id:
#     qc.cx(src - 1, dst - 1)  # Controlled-X gate

# qc.draw(output='mpl')

# plt.show()
