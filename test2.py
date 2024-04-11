# 定義NAND閘函數
def NAND(a, b):
    return not (a and b)

# 實現2線到4線解碼器
def decoder(a, b):
    # 使用最少NAND閘實現
    x = NAND(a, b)
    y = NAND(a, x)
    z = NAND(x, b)
    w = NAND(y, z)
    
    # 輸出4個解碼線
    d0 = NAND(x, x)
    d1 = NAND(y, y)
    d2 = NAND(z, z)
    d3 = NAND(w, w)
    
    return d0, d1, d2, d3

# 測試
print("a b | d0 d1 d2 d3")
print("-----------------")
for a in [False, True]:
    for b in [False, True]:
        d0, d1, d2, d3 = decoder(a, b)
        print(f"{int(a)} {int(b)} | {int(d0)} {int(d1)} {int(d2)} {int(d3)}")