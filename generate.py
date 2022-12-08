import random
import sys


def generate(nfamilies, maxsets, maxelements):
    families = []
    for i in range(nfamilies):
        nsets = random.randint(0, maxsets - 1)
        family = []
        for j in range(nsets):
            nelements = random.randint(0, maxelements - 1)
            s = set()
            for k in range(nelements):
                s.add(random.randint(0, maxelements - 1))
            family.append(list(s))
        families.append(family)
    return families


def generate_and_write(nfamilies, maxsets, maxelements, filename):
    families = generate(nfamilies, maxsets, maxelements)
    with open(filename, 'w+') as f:
        f.write(f'%d\n' % len(families))
        for family in families:
            f.write(f'%d\n' % len(family))
            for s in family:
                if len(s) == 0:
                    continue
                for elem in s[:-1]:
                    f.write(f'%d ' % elem)
                f.write(f'%d\n' % s[-1])


if len(sys.argv) < 5:
    print("Użycie: python3 generate.py ILE_RODZIN MAX_ZBIOROW MAX_ELEMENTOW SCIEZKA")

generate_and_write(int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3]), sys.argv[4])
