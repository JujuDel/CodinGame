import sys
import math
import itertools

n = int(input())
L = []
for i in range(n):
    L += [input()]

res = sum(len(l) for l in L)
perms = list(itertools.permutations(L))
for perm in perms:
    seq, perm = perm[0], perm[1:]
    for i in range(len(perm)):
        j = 0
        while j < len(seq):
            subseq = perm[i]
            if perm[i] in seq:
                break
            end = len(seq) - j
            if seq[j:] == perm[i][:end]:
                seq += perm[i][end:]
            j += 1
        if j == len(seq):
            seq += perm[i]
    res = min(res, len(seq))

print(res)