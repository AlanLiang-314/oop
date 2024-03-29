from tabulate import tabulate

def get_minterms(truth_table):
    minterms = []
    for i, val in enumerate(truth_table):
        if val == 1:
            minterms.append(bin(i)[2:].zfill(len(truth_table)))
    return minterms

def get_prime_implicants(minterms):
    n = len(minterms[0])
    groups = []
    for num_ones in range(n):
        groups.append({"implicants": [], "next": []})

    for minterm in minterms:
        ones_count = minterm.count('1')
        groups[ones_count]["implicants"].append(minterm)

    for i in range(n):
        if len(groups[i]["implicants"]) == 0:
            continue
        for implicant1 in groups[i]["implicants"]:
            temp = groups[i+1]["next"]
            for implicant2 in temp:
                diffs = 0
                for j in range(n):
                    if implicant1[j] != implicant2[j]:
                        diffs += 1
                        diff_pos = j
                if diffs == 1:
                    implicant = implicant1[:diff_pos] + "-" + implicant1[diff_pos+1:]
                    if implicant not in groups[i+1]["implicants"]:
                        groups[i+1]["implicants"].append(implicant)
            groups[i+1]["next"] = groups[i+1]["implicants"]

    prime_implicants = []
    for group in groups:
        prime_implicants.extend(group["implicants"])
    return prime_implicants

def get_essential_prime_implicants(prime_implicants, minterms):
    essential = []
    prime_implicants_copy = prime_implicants[:]
    while prime_implicants_copy:
        prime_implicant = prime_implicants_copy.pop(0)
        implicant_minterms = []
        for minterm in minterms:
            temp_count = 0
            for i in range(len(minterm)):
                if prime_implicant[i] == "-" or prime_implicant[i] == minterm[i]:
                    temp_count += 1
            if temp_count == len(minterm):
                implicant_minterms.append(minterm)
        if all(x in implicant_minterms for x in implicant_minterms):
            essential.append(prime_implicant)
            for x in implicant_minterms:
                minterms.remove(x)
    return essential

def minimize_functionequation(essential_implicants, prime_implicants, minterms):
    function = []
    removed = []
    for implicant in essential_implicants:
        function.append(implicant)
    while minterms:
        for prime_implicant in prime_implicants:
            temp_minterms = []
            for minterm in minterms:
                temp_count = 0
                for i in range(len(minterm)):
                    if prime_implicant[i] == "-" or prime_implicant[i] == minterm[i]:
                        temp_count += 1
                if temp_count == len(minterm):
                    temp_minterms.append(minterm)
            if set(temp_minterms).issubset(set(minterms)) and len(temp_minterms) > 0:
                if prime_implicant not in function:
                    function.append(prime_implicant)
                    removed = removed + temp_minterms
        for x in removed:
            if x in minterms:
                minterms.remove(x)
        removed = []
    return function

def convert_to_nand(functions):
    nand_expressions = []
    for function in functions:
        nand_expression = ""
        for i, char in enumerate(function):
            if char == "-":
                continue
            elif char == "0":
                nand_expression += "¬A" + str(i) + "∧"
            else:
                nand_expression += "A" + str(i) + "∧"
        nand_expression = nand_expression[:-1]
        nand_expressions.append("(" + nand_expression + ")")
    return "∧".join(nand_expressions)

def main():
    truth_table = [int(x) for x in input("請輸入真值表(用0和1表示): ").split()]
    minterms = get_minterms(truth_table)
    prime_implicants = get_prime_implicants(minterms)
    essential_implicants = get_essential_prime_implicants(prime_implicants, minterms)
    functions = minimize_functionequation(essential_implicants, prime_implicants, minterms)
    nand_expression = convert_to_nand(functions)

    print("\n真值表: ", truth_table)
    print("閥數: ", len(truth_table))
    print("極小項: ", minterms)
    print("主要臨界點: ", prime_implicants)
    print("本質主要臨界點: ", essential_implicants)
    print("最小化函數方程式: ", functions)
    print("使用NAND閘的電路表達式: ", nand_expression)

if __name__ == "__main__":
    main()