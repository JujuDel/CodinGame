graph = {}

# n: the total number of nodes in the level, including the gateways
# l: the number of links
# e: the number of exit gateways
n, l, e = [int(i) for i in input().split()]
for i in range(l):
    # n1: N1 and N2 defines a link between these nodes
    n1, n2 = [int(j) for j in input().split()]

    if not n1 in graph:
        graph[n1] = [n2]
    else:
        graph[n1].append(n2)
    if not n2 in graph:
        graph[n2] = [n1]
    else:
        graph[n2].append(n1)

outs = []

for i in range(e):
    ei = int(input())  # the index of a gateway node
    outs.append(ei)

danger = {}
for n, v in graph.items():
    if len([i for i in v if i in outs]) > 1:
        danger[n] = [n]


def updateDanger(d, n):
    global graph, danger, outs

    if n in danger[d] or n in outs: return
    tmp = [i for i in graph[n] if i in outs]
    if len(tmp) > 0:
        danger[d].append(n)
        for nn in graph[n]:
            if nn in danger[d] or nn in outs: continue
            updateDanger(d, nn)


for d in danger:
    for n in graph[d]:
        updateDanger(d, n)


# game loop
while True:
    si = int(input())  # The index of the node on which the Skynet agent is positioned this turn

    B = si
    maxN = 0
    for n in graph[si]:
        if n in outs:
            A = si
            B = n
            break
        for d, dgr in danger.items():
            if n in dgr:
                tmp = [i for i in graph[d] if i in outs]
                if len(tmp) > 0:
                    A = tmp[0]
                    B = d

    if B in danger:
        del(danger[B])

    if B == si:
        maxN = 0
        for n, v in graph.items():
            t = [i for i in graph[n] if i in outs]
            m = len(t)
            if m > maxN:
                maxN = m
                A = n
                B = t[0]

    graph[A].remove(B)
    graph[B].remove(A)

    print(A, B)
