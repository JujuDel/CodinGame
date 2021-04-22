import sys
import math

m = int(input())  # the amount of motorbikes to control
v = int(input())  # the minimum amount of motorbikes that must survive
l0 = input()  # L0 to L3 are lanes of the road. A dot character . represents a safe space, a zero 0 represents a hole in the road.
l1 = input()
l2 = input()
l3 = input()

print(l0, file=sys.stderr)
print(l1, file=sys.stderr)
print(l2, file=sys.stderr)
print(l3, file=sys.stderr)

goal = len(l1)

l0 += '.' * 50
l1 += '.' * 50
l2 += '.' * 50
l3 += '.' * 50
grid = [l0, l1, l2, l3]


def trySpeed(grid, M, v, x, speed, res, idx):
    removed = []

    res[idx] = "SPEED"
    for j in range(len(M) - 1, -1, -1):
        y = M[j]
        for i in range(1, speed + 2):
            if grid[y][x + i] == '0':
                del(M[j])
                removed.append(y)
                break
    if recursif(grid, M, v, x + speed + 1, speed + 1, res, idx + 1):
        return True
    for y in removed:
        M.append(y)
    
    return False

def tryJump(grid, M, v, x, speed, res, idx):
    removed = []

    res[idx] = "JUMP"
    for j in range(len(M) - 1, -1, -1):
        y = M[j]
        if grid[y][x + speed] == '0':
            del(M[j])
            removed.append(y)
    if recursif(grid, M, v, x + speed, speed, res, idx + 1):
        return True
    for y in removed:
        M.append(y)
    
    return False

def tryWait(grid, M, v, x, speed, res, idx):
    removed = []

    res[idx] = "WAIT"
    for j in range(len(M) - 1, -1, -1):
        y = M[j]
        for i in range(1, speed + 1):
            if grid[y][x + i] == '0':
                del(M[j])
                removed.append(y)
                break
    if recursif(grid, M, v, x + speed, speed, res, idx + 1):
        return True
    for y in removed:
        M.append(y)
    
    return False

def tryUp(grid, M, v, x, speed, res, idx):
    removed = []

    res[idx] = "UP"
    for j in range(len(M) - 1, -1, -1):
        y = M[j]
        rem = False
        for i in range(1, speed):
            if grid[y][x + i] == '0':
                del(M[j])
                removed.append(y)
                rem = True
                break
        if rem: continue
        for i in range(1, speed + 1):
            if grid[y - 1][x + i] == '0':
                del(M[j])
                removed.append(y)
                rem = True
                break
        if not rem:
            M[j] = y - 1
    if recursif(grid, M, v, x + speed, speed, res, idx + 1):
        return True

    for j, y in enumerate(M):
        M[j] += 1
    for y in removed:
        M.append(y)

    return False

def tryDown(grid, M, v, x, speed, res, idx):
    removed = []
    
    res[idx] = "DOWN"
    for j in range(len(M) - 1, -1, -1):
        y = M[j]
        rem = False
        for i in range(1, speed):
            if grid[y][x + i] == '0':
                del(M[j])
                removed.append(y)
                rem = True
                break
        if rem: continue
        for i in range(1, speed + 1):
            if grid[y + 1][x + i] == '0':
                del(M[j])
                removed.append(y)
                rem = True
                break
        if not rem:
            M[j] = y + 1
    if recursif(grid, M, v, x + speed, speed, res, idx + 1):
        return True
    for j, y in enumerate(M):
        M[j] -= 1
    for y in removed:
        M.append(y)
    
    return False

def trySlow(grid, M, v, x, speed, res, idx):
    removed = []

    res[idx] = "SLOW"
    for j in range(len(M) - 1, -1, -1):
        y = M[j]
        for i in range(1, speed):
            if grid[y][x + i] == '0':
                del(M[j])
                removed.append(y)
                break
    if recursif(grid, M, v, x + speed - 1, speed - 1, res, idx + 1):
        return True
    for y in removed:
        M.append(y)

    return False

def recursif(grid, M, v, x, speed, res, idx):
    global goal

    #print(res[idx-1], file=sys.stderr)
    #print(M, file=sys.stderr)
    #print(v, file=sys.stderr)
    #print((x, speed), file=sys.stderr)

    if len(M) < v or idx > 49:
        #print("FAILURE", file=sys.stderr)
        #print("", file=sys.stderr)
        return False

    if x > goal:
        return True

    #print("", file=sys.stderr)

    if speed >= 50:
        speed = 49
    if speed < 0:
        speed = 0
    
    speedSuccess = trySpeed(grid, M, v, x, speed, res, idx)
    if speedSuccess:
        return True

    if not 0 in M:
        upSuccess = tryUp(grid, M, v, x, speed, res, idx)
        if upSuccess:
            return True
    
    if not 3 in M:
        downSuccess = tryDown(grid, M, v, x, speed, res, idx)
        if downSuccess:
            return True

    waitSuccess = tryWait(grid, M, v, x, speed, res, idx)
    if waitSuccess:
        return True

    jumpSuccess = tryJump(grid, M, v, x, speed, res, idx)
    if jumpSuccess:
        return True

    slowSuccess = trySlow(grid, M, v, x, speed, res, idx)

    return slowSuccess

def solve(grid, M, v, s):
    res = [''] * 50

    if len(M) == 4:
        if s == 0:
            res[0] = 'SPEED'
            ret = recursif(grid, M, 4, 1, 1, res, 1)
        else:
            ret = recursif(grid, M, 4, 0, s, res, 0)
    
        if not ret:
            recursif(grid, M, v, 0, s, res, 0)
    else:
        if s == 0:
            res[0] = 'SPEED'
            ret = recursif(grid, M, v, 1, 1, res, 1)
        else:
            ret = recursif(grid, M, v, 0, s, res, 0)

    return res

# game loop
idx = 0
ready = False
Motorbikes = []
while True:
    s = int(input())  # the motorbikes' speed
    for i in range(m):
        # x: x coordinate of the motorbike
        # y: y coordinate of the motorbike
        # a: indicates whether the motorbike is activated "1" or detroyed "0"
        x, y, a = [int(j) for j in input().split()]
        print((x,y,a,s), file=sys.stderr)
        Motorbikes.append(y)

    if not ready:
        orders = solve(grid, Motorbikes, v, s)
        print(orders, file=sys.stderr)
        ready = True

    print(orders[idx])
    idx += 1