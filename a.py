import os

algs = ['SVNS_S', 'DIWO', 'MFFO', 'IG_VND2', 'HVNS', 'IG_IJ', 'HDDE', 'IG', 'DE_PLS', 'MA', 'IG_RIS', 'hmgHS', 'SVNS_D', 'SaDIWO', 'IG_VND1', 'TPA', 'P_EDA', 'RAIS', 'DE_ABC']
constructors = {}
for alg in algs:
    try:
        alg_constructors = os.listdir(f"{alg}/src/constructions")
    except:
        continue

    alg_constructors.remove("meson.build")
    constructors[alg] = alg_constructors

unique_constructors = set()
for alg, cs in constructors.items():
    for c in cs:
        with open(f"{alg}/src/constructions/{c}", "r") as file:
            print(file.read())
            unique_constructors.add(c + "\n" + file.read())

a = []
for c in unique_constructors:
    a.append(c.split("\n")[0])

print(sorted(a))
print(len(a))