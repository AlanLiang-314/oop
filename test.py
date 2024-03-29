import re

# The text you provided
text = """
testset\tiny\008.txt all OK. 13 CNOT, 0 SWAP
testset\tiny\017.txt all OK. 14 CNOT, 4 SWAP
testset\tiny\004.txt all OK. 12 CNOT, 4 SWAP
testset\tiny\001.txt all OK. 9 CNOT, 1 SWAP
testset\tiny\002.txt all OK. 8 CNOT, 8 SWAP
testset\tiny\015.txt all OK. 8 CNOT, 0 SWAP
testset\tiny\013.txt all OK. 12 CNOT, 0 SWAP
testset\tiny\018.txt all OK. 9 CNOT, 0 SWAP
testset\tiny\021.txt all OK. 14 CNOT, 0 SWAP
testset\tiny\024.txt all OK. 14 CNOT, 2 SWAP
testset\tiny\020.txt all OK. 10 CNOT, 5 SWAP
testset\tiny\003.txt all OK. 16 CNOT, 8 SWAP
testset\tiny\006.txt all OK. 9 CNOT, 4 SWAP
testset\tiny\012.txt all OK. 12 CNOT, 2 SWAP
testset\tiny\016.txt all OK. 15 CNOT, 2 SWAP
testset\tiny\007.txt all OK. 10 CNOT, 2 SWAP
testset\tiny\014.txt all OK. 16 CNOT, 0 SWAP
testset\tiny\011.txt all OK. 16 CNOT, 1 SWAP
testset\tiny\010.txt all OK. 8 CNOT, 3 SWAP
testset\tiny\009.txt all OK. 13 CNOT, 8 SWAP
testset\tiny\005.txt all OK. 12 CNOT, 3 SWAP
testset\tiny\023.txt all OK. 12 CNOT, 11 SWAP
testset\tiny\019.txt all OK. 8 CNOT, 2 SWAP
testset\tiny\033.txt all OK. 10 CNOT, 4 SWAP
testset\tiny\037.txt all OK. 9 CNOT, 1 SWAP
testset\tiny\031.txt all OK. 15 CNOT, 5 SWAP
testset\tiny\025.txt all OK. 8 CNOT, 6 SWAP
testset\tiny\026.txt all OK. 11 CNOT, 7 SWAP
testset\tiny\035.txt all OK. 13 CNOT, 5 SWAP
testset\tiny\038.txt all OK. 14 CNOT, 4 SWAP
testset\tiny\027.txt all OK. 15 CNOT, 8 SWAP
testset\tiny\029.txt all OK. 9 CNOT, 7 SWAP
testset\tiny\032.txt all OK. 11 CNOT, 2 SWAP
testset\tiny\041.txt all OK. 13 CNOT, 5 SWAP
testset\tiny\028.txt all OK. 10 CNOT, 0 SWAP
testset\tiny\036.txt all OK. 8 CNOT, 7 SWAP
testset\tiny\047.txt all OK. 9 CNOT, 3 SWAP
testset\tiny\045.txt all OK. 14 CNOT, 2 SWAP
testset\tiny\043.txt all OK. 12 CNOT, 3 SWAP
testset\tiny\052.txt all OK. 8 CNOT, 0 SWAP
testset\tiny\049.txt all OK. 13 CNOT, 0 SWAP
testset\tiny\051.txt all OK. 16 CNOT, 0 SWAP
testset\tiny\044.txt all OK. 16 CNOT, 3 SWAP
testset\tiny\046.txt all OK. 14 CNOT, 4 SWAP
testset\tiny\048.txt all OK. 13 CNOT, 10 SWAP
testset\tiny\058.txt all OK. 11 CNOT, 6 SWAP
testset\tiny\054.txt all OK. 15 CNOT, 0 SWAP
testset\tiny\059.txt all OK. 9 CNOT, 0 SWAP
testset\tiny\057.txt all OK. 13 CNOT, 4 SWAP
testset\tiny\050.txt all OK. 12 CNOT, 5 SWAP
testset\tiny\039.txt all OK. 12 CNOT, 0 SWAP
testset\tiny\060.txt all OK. 12 CNOT, 3 SWAP
testset\tiny\062.txt all OK. 10 CNOT, 8 SWAP
testset\tiny\053.txt all OK. 11 CNOT, 0 SWAP
testset\tiny\067.txt all OK. 9 CNOT, 0 SWAP
testset\tiny\063.txt all OK. 15 CNOT, 7 SWAP
testset\tiny\065.txt all OK. 12 CNOT, 6 SWAP
testset\tiny\061.txt all OK. 10 CNOT, 0 SWAP
testset\tiny\055.txt all OK. 11 CNOT, 2 SWAP
testset\tiny\069.txt all OK. 8 CNOT, 8 SWAP
testset\tiny\064.txt all OK. 9 CNOT, 2 SWAP
testset\tiny\070.txt all OK. 12 CNOT, 0 SWAP
testset\tiny\068.txt all OK. 15 CNOT, 3 SWAP
testset\tiny\072.txt all OK. 10 CNOT, 0 SWAP
testset\tiny\073.txt all OK. 9 CNOT, 1 SWAP
testset\tiny\071.txt all OK. 16 CNOT, 1 SWAP
testset\tiny\077.txt all OK. 12 CNOT, 7 SWAP
testset\tiny\066.txt all OK. 12 CNOT, 0 SWAP
testset\tiny\078.txt all OK. 10 CNOT, 13 SWAP
testset\tiny\075.txt all OK. 14 CNOT, 6 SWAP
testset\tiny\080.txt all OK. 14 CNOT, 2 SWAP
testset\tiny\079.txt all OK. 15 CNOT, 3 SWAP
testset\tiny\086.txt all OK. 8 CNOT, 2 SWAP
testset\tiny\087.txt all OK. 11 CNOT, 0 SWAP
testset\tiny\076.txt all OK. 15 CNOT, 4 SWAP
testset\tiny\081.txt all OK. 15 CNOT, 6 SWAP
testset\tiny\091.txt all OK. 10 CNOT, 4 SWAP
testset\tiny\088.txt all OK. 11 CNOT, 3 SWAP
testset\tiny\095.txt all OK. 16 CNOT, 4 SWAP
testset\tiny\082.txt all OK. 15 CNOT, 0 SWAP
testset\tiny\089.txt all OK. 12 CNOT, 1 SWAP
testset\tiny\093.txt all OK. 15 CNOT, 6 SWAP
testset\tiny\092.txt all OK. 16 CNOT, 2 SWAP
testset\tiny\096.txt all OK. 13 CNOT, 0 SWAP
testset\tiny\090.txt all OK. 9 CNOT, 0 SWAP
testset\tiny\098.txt all OK. 8 CNOT, 4 SWAP
testset\tiny\094.txt all OK. 10 CNOT, 6 SWAP
testset\tiny\085.txt all OK. 9 CNOT, 0 SWAP
testset\tiny\100.txt all OK. 15 CNOT, 0 SWAP
testset\tiny\097.txt all OK. 14 CNOT, 0 SWAP
"""

numbers = set(map(int, re.findall(r'\d+', text)))

print(numbers)# numbers_in_text = set(map(int, re.findall(r'\d+', text)))

# # Create a set of all numbers from 1 to 100
# all_numbers = set(range(1, 101))

# print(numbers_in_text)
# # Find the missing numbers
# missing_numbers = all_numbers - numbers_in_text

# print(missing_numbers)