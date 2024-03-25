import os
import argparse
import subprocess
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor
from functools import partial


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


parser = argparse.ArgumentParser(description='good Program')
parser.add_argument('--exe', type=str, default="output.txt", help='Path to the solution file')
parser.add_argument('--testset_path', type=str, default="testset", help='Path to the testset folder')
parser.add_argument('--folders', type=str, nargs='+', default=["tiny"], help='List of folders to test, should be in the testset folder.')
parser.add_argument('--verbose', action='store_true', help='Produce detailed output')

# fininsh_ids = set()

def run_test_case(testcase, testcase_path, excutable_name: str = None):

    testcase = os.path.join(testcase_path, testcase)

    # print(f"testing {testcase}...", end="")

    input_file = open(testcase, 'r', encoding="utf-8")
    iterator = iter(input_file)

    logQubits, num_gates, num_dependencies, phyQubits, num_phyLinks = map(int, next(iterator).split())
    gates = []
    dependencies = []

    for _ in range(num_gates):
        _, srcbit, dstbit = map(int, next(iterator).split())
        gates.append((srcbit, dstbit))

    for _ in range(num_dependencies):
        _, srcgate, tgtgate = map(int, next(iterator).split())
        dependencies.append((srcgate, tgtgate))

    G = Graph()
    for _ in range(num_phyLinks):
        _, src, dst = map(int, next(iterator).split())
        G.add_edge(src, dst)

    input_file.close()

    graph = defaultdict(list)
    in_degree = defaultdict(int)

    for u, v in dependencies:
        graph[u].append(v)
        in_degree[v] += 1

    queue = set([u for u in graph if in_degree[u] == 0])

    if os.name == 'nt':  # Windows
        command = f"type {testcase} | {excutable_name}"
    else:  # Unix/Linux
        command = f"cat {testcase} | {excutable_name}"
    
    result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if result.returncode != 0:
        print(result.stderr)
        exit(1)
    
    output_file = result.stdout.strip().split("\n")

    iterator = iter(output_file)

    qubit_mapping = dict()
    for _ in range(logQubits):
        logbit, phybit = map(int, next(iterator).split())
        qubit_mapping[logbit] = phybit

    cnot_counter, swap_counter = 0, 0

    while queue:
        op_type, src, dst = next(iterator).split()
        src, dst = int(src[1:]), int(dst[1:])
        log_src, log_dst = (src, dst) if src < dst else (dst, src)

        if op_type == "CNOT":
            try:
                gate_id = gates.index((log_src, log_dst))
                gates[gate_id] = (-1, -1)
                gate_id += 1
                # print(f"gate_id: {gate_id}")
            except ValueError:
                print(f"gate not found: {(log_src, log_dst)}")
                # exit(1)
            try:
                queue.remove(gate_id)
            except KeyError:
                print(f"expect gate in: {queue} but found: {(log_src, log_dst)}")
                # exit(1)

            if not G.are_neighbors(qubit_mapping[log_src], qubit_mapping[log_dst]):
                print(f"{(log_src, log_dst)} are not neighbors")
                # exit(1)

            for v in graph[gate_id]:
                in_degree[v] -= 1
                if in_degree[v] == 0:
                    queue.add(v)
            if args.verbose:
                print(f"CNOT q{src} q{dst}")
            cnot_counter += 1

        elif op_type == "SWAP":
            qubit_mapping[log_src], qubit_mapping[log_dst] = qubit_mapping[log_dst], qubit_mapping[log_src]
            if args.verbose:
                print(f"SWAP q{log_src} q{log_dst}")

            swap_counter += 1

        else:
            print(f"unknown op type: {op_type}")
            exit(1)

    print(f"{testcase} all OK. {cnot_counter} CNOT, {swap_counter} SWAP")

    # global fininsh_ids
    # fininsh_ids.add(testcase)
    # print(fininsh_ids)

    return cnot_counter, swap_counter


args = parser.parse_args()

folders = args.folders
benchmark_result = []

for folder in folders:
    testcase_path = os.path.join(args.testset_path, folder)
    testcases = os.listdir(testcase_path)

    total_cnot_counter, total_swap_counter = 0, 0
    fun = partial(run_test_case, testcase_path=testcase_path, excutable_name=args.exe)

    with ThreadPoolExecutor() as executor:
        results = executor.map(fun, testcases)

    for result in results:
        cnot_counter, swap_counter = result
        total_cnot_counter += cnot_counter
        total_swap_counter += swap_counter

    print(f"{folder} completed, avg {total_cnot_counter/len(testcases):.3f} CNOT, {total_swap_counter/len(testcases):.3f} SWAP")
    benchmark_result.append((folder, total_cnot_counter/len(testcases), total_swap_counter/len(testcases)))

print("-" * 60)
print("result:")
for benchmark in benchmark_result:
    print(f"{benchmark[0]}: avg {benchmark[1]} CNOT, {benchmark[2]} SWAP")