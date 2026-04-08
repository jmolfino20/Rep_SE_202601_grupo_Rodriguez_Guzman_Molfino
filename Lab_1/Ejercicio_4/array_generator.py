sizes = [20, 70, 120, 170, 220, 270]

for size in sizes:
    values = ",".join(str(i) for i in range(1, size + 1))

    print(f"// VECTOR_SIZE = {size}")
    print(f"const int vector_flash_ext_{size}[{size}] = {{")
    print(f"    {values}")
    print("};\n")