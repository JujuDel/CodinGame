import sys
import math

n = int(input())
G = []
for i in range(n):
    j, d = map(int, input().split())
    G += [(j, j+d-1)]

G.sort(key=lambda x:x[1])

res = 1
lim = G[0][1]

for i in range(1, n):
    if lim < G[i][0]:
        lim = G[i][1]
        res += 1

print(res)