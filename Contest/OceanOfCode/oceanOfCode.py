import sys
import copy
import random
import time
import gc

gc.disable()

# @TODO:
#    - Make detection more robust:
#        o by taking multiple damages detection -> Done
#        o by computing paths with timeout safety -> Done
#        o by removing path on trigger -> Done
#    - Make survival more robust:
#        o by avoiding mines -> Ongoing
#        o by avoiding conflicts and hide in my own mines -> testing
#    - Bonus - Make shooting more robust:
#        o think about shooting after opp did silence: I don't have the info sometimes

cells = []

"""
    DEBUG
"""

def debug(message, title=""):
    if title == "":
        print(message, file=sys.stderr)
    else:
        print(title, message, file=sys.stderr)

def debugGrid(grid=cells, title="GRID:"):
    debug(title)

    lY = 0
    for l in grid:
        line = " "
        lX = 0
        for c in l:
            line += " " + c
            lX += 1
            if lX == 5:
                line += " "
                lX = 0
        print(line, file=sys.stderr)
        lY += 1
        if lY == 5:
            print("", file=sys.stderr)
            lY = 0

def debugEnnemy(oppLife, oppOrder, sonarResult):
    print("Ennemy's input:", file=sys.stderr)
    print("  Life:", oppLife, file=sys.stderr)
    print("  Order:", oppOrder, file=sys.stderr)
    print("  Sonar:", sonarResult, file=sys.stderr)

"""
    CHOICES
"""

def chooseStart(my_id, minX=5, minY=5, size=5):
    global cells

    possiblePos = []
    for j in range(minY, minY + size):
        for i in range(minX, minX + size):
            if cells[j][i] == '.':
                possiblePos.append((i, j))

    if len(possiblePos) > 0:
        return possiblePos[random.randint(0, len(possiblePos) - 1)]
    else:
        return chooseStart(my_id, 0, 0, 15)

def choosePower(ennemy, fakeMe, triggerEscape, torpedo_cooldown, silence_cooldown, mine_cooldown, sonar_cooldown):
    if len(ennemy.pos) == 1 and torpedo_cooldown > 0:
        power = " TORPEDO"
        torpedo_cooldown -= 1

    elif len(ennemy.pos) < 15 and triggerEscape and 0 < silence_cooldown < 2:
        power = " SILENCE"
        silence_cooldown -= 1

    elif len(ennemy.pos) < 15 and torpedo_cooldown > 0:
        power = " TORPEDO"
        torpedo_cooldown -= 1

    elif len(fakeMe.pos) < 20 and silence_cooldown > 0:
        power = " SILENCE"
        silence_cooldown -= 1

    elif mine_cooldown > 0:
        power = " MINE"
        mine_cooldown -= 1

    elif silence_cooldown > 0:
        power = " SILENCE"
        silence_cooldown -= 1

    elif sonar_cooldown > 0:
        power = " SONAR"
        sonar_cooldown -= 1
    else:

        power = " TORPEDO"
        torpedo_cooldown -= 1

    return torpedo_cooldown, silence_cooldown, mine_cooldown, sonar_cooldown, power

def chooseMove(x, y, possibleChars):
    global cells

    res = []

    if y + 1 < 15 and cells[y + 1][x] in possibleChars:
        res.append( (('S'), x, y + 1) )

    if x - 1 > -1 and cells[y][x - 1] in possibleChars:
        res.append( (('W'), x - 1, y) )

    if x + 1 < 15 and cells[y][x + 1] in possibleChars:
        res.append( (('E'), x + 1, y) )

    if y - 1 > -1 and cells[y - 1][x] in possibleChars:
        res.append( (('N'), x, y - 1) )

    return res

def chooseSonar(ennemy):
    res = [0] * 9

    for (x, y) in ennemy.pos:
        caseX = x // 5
        caseY = y // 5
        res[3 * caseY + caseX] += 1

    return res

def chooseSilence(x, y):
    global cells

    res = []

    bestFound = True
    for d in range(1, 5):  # SOUTH
        if y + d < 15:
            if cells[y + d][x] != '.':
                nbMove = d - 1
                bestFound = False
                break
        else:
            nbMove = d - 1
            bestFound = False
            break
    if bestFound:
        res.append(('S', 4))
    elif nbMove > 0:
        res.append(('S', nbMove))

    bestFound = True
    for d in range(1, 5):  # WEST
        if x - d > -1:
            if cells[y][x - d] != '.':
                nbMove = d - 1
                bestFound = False
                break
        else:
            nbMove = d - 1
            bestFound = False
            break
    if bestFound:
        res.append(('W', 4))
    elif nbMove > 0:
        res.append(('W', nbMove))

    bestFound = True
    for d in range(1, 5):  # East
        if x + d < 15:
            if cells[y][x + d] != '.':
                nbMove = d - 1
                bestFound = False
                break
        else:
            nbMove = d - 1
            bestFound = False
            break
    if bestFound:
        res.append(('E', 4))
    elif nbMove > 0:
        res.append(('E', nbMove))

    bestFound = True
    for d in range(1, 5):  # North
        if y - d > -1:
            if cells[y - d][x] != '.':
                nbMove = d - 1
                bestFound = False
                break
        else:
            nbMove = d - 1
            bestFound = False
            break
    if bestFound:
        res.append(('N', 4))
    elif nbMove > 0:
        res.append(('N', nbMove))

    return res

def isPathExists(x, y, targetX, targetY):
    global cells

    if x < 0 or x > 14 or y < 0 or y > 14:
        return False

    if cells[y][x] == 'x':
        return False

    if x == targetX and y == targetY:
        return True

    if x == targetX:
        dirY = int((targetY - y) / abs(targetY - y))
        return isPathExists(x, y + dirY, targetX, targetY)

    elif y == targetY:
        dirX = int((targetX - x) / abs(targetX - x))
        return isPathExists(x + dirX, y, targetX, targetY)

    else:
        dirX = int((targetX - x) / abs(targetX - x))
        dirY = int((targetY - y) / abs(targetY - y))
        return isPathExists(x + dirX, y, targetX, targetY) or isPathExists(x, y + dirY, targetX, targetY)

def isShootPossible(x, y, targetX, targetY):
    global cells

    if abs(x - targetX) + abs(y - targetY) > 4:
        return False
    else:
        return isPathExists(x, y, targetX, targetY)

def shootAroundWithoutTouching(fromPos, toPos):
    x, y = fromPos
    tX, tY = toPos

    for j in [-1, 0, 1]:
        for i in [-1, 0, 1]:
            nX = tX + i
            nY = tY + j
            if not ( abs(x - nX) + abs(y - nY) < 2 or (abs(x - nX) == 1 and abs(y - nY) == 1) ):
                if isShootPossible(x, y, nX, nY):
                    debug("Shoot:")
                    debug("  Aiming: (" + str(nX) + ", " + str(nY) + ")")

                    if nX == tX and nY == tY:
                        return "TORPEDO " + str(nX) + " " + str(nY), nX, nY, 2
                    else:
                        return "TORPEDO " + str(nX) + " " + str(nY), nX, nY, 1

    return "NOPE", -1, -1, -1

def chooseTarget(x, y, ennemy, opp_life, my_life, triggerSonar, trig):
    #if True: return "NOPE", -1, -1, -1

    trigX, trigY = trig

    if len(ennemy.pos) == 1:
        tX, tY = ennemy.pos[0]

        if x == tX and y == tY:
            if opp_life < 3 or my_life - opp_life > 2:
                debug("Shoot:")
                debug("  Aiming: (" + str(tX) + ", " + str(tY) + ")")
                return "TORPEDO " + str(tX) + " " + str(tY), tX, tY, 2
            else:
                return "NOPE", -1, -1, -1

        elif abs(x - tX) + abs(y - tY) < 2 or (abs(x - tX) == 1 and abs(y - tY) == 1):
            if opp_life == 1:
                return shootAroundWithoutTouching((x, y), (tX, tY))
            elif opp_life == 2 or my_life - opp_life > 1:
                debug("Shoot:")
                debug("  Aiming: (" + str(tX) + ", " + str(tY) + ")")
                return "TORPEDO " + str(tX) + " " + str(tY), tX, tY, 2
            else:
                return shootAroundWithoutTouching((x, y), (tX, tY))

        elif isShootPossible(x, y, tX, tY):
            debug("Shoot:")
            debug("  Aiming: (" + str(tX) + ", " + str(tY) + ")")
            return "TORPEDO " + str(tX) + " " + str(tY), tX, tY, 2

        else:
            return shootAroundWithoutTouching((x, y), (tX, tY))

    maxEnnemy = max(ennemy.countPos)
    confidence = maxEnnemy / sum(ennemy.countPos)
    nbMax = len([0 for j in ennemy.countPos if j == maxEnnemy])

    if confidence > 0.10 and nbMax == 1:
        tX, tY = ennemy.pos[ ennemy.countPos.index(maxEnnemy) ]
        if not (abs(x - tX) <= 1 and abs(y - tY) <= 1):
            if isShootPossible(x, y, tX, tY):
                debug("Shoot:")
                debug("  Aiming: (" + str(tX) + ", " + str(tY) + ")")
                return "TORPEDO " + str(tX) + " " + str(tY), tX, tY, 0

    if not triggerSonar and len(ennemy.pos) < 7:
        targetI = 0
        maxTouched = 0
        for i, p in enumerate(ennemy.pos):
            tX, tY = p
            if isShootPossible(x, y, tX, tY):
                touching = 0
                for j in [-1, 0, 1]:
                    for k in [-1, 0, 1]:
                        if len(ennemy.pos) > 1 and tX + j == x and tY + k == y:
                            touching = -20
                        elif [tX + j, tY + k] in ennemy.pos:
                            touching += 1
                if touching > maxTouched:
                    maxTouched = touching
                    targetI = i

        if maxTouched > 0:
            tX, tY = ennemy.pos[targetI]
            print("Shoot:", file=sys.stderr)
            print("  Aiming: (" + str(tX) + ", " + str(tY) + ")", file=sys.stderr)

            return "TORPEDO " + str(tX) + " " + str(tY), tX, tY, 0

    if len(ennemy.pos) < 3:
        targetI = 0
        maxTouched = 0
        for i, p in enumerate(ennemy.pos):
            tX, tY = p
            if isShootPossible(x, y, tX, tY):
                touching = 0
                for j in [-1, 0, 1]:
                    for k in [-1, 0, 1]:
                        if tX + j == x and tY + k == y:
                            touching = -20
                        if [tX + j, tY + k] in ennemy.pos:
                            touching += 1
                if touching > maxTouched:
                    maxTouched = touching
                    targetI = i

        if maxTouched > 0:
            tX, tY = ennemy.pos[targetI]
            print("Shoot:", file=sys.stderr)
            print("  Aiming: (" + str(tX) + ", " + str(tY) + ")", file=sys.stderr)

            return "TORPEDO " + str(tX) + " " + str(tY), tX, tY, 0

    return "NOPE", -1, -1, -1

"""
    GRID
"""

def markWater(grid, c):
    for y in range(len(grid)):
        for x in range(len(grid[y])):
            if grid[y][x] == '.':
                grid[y][x] = c

def markRegion(positions, c):
    global cells
    for (x, y) in positions:
        cells[y][x] = c

def updateGridForSilence(x, y, direction, value):
    global cells

    if direction == 'N':
        for yi in range(y - 1, y - value - 1, -1):
            cells[yi][x] = 'w'
        return x, y - value

    elif direction == 'S':
        for yi in range(y + 1, y + value + 1):
            cells[yi][x] = 'w'
        return x, y + value

    elif direction == 'W':
        for xi in range(x - 1, x - value - 1, -1):
            cells[y][xi] = 'w'
        return x - value, y

    elif direction == 'E':
        for xi in range(x + 1, x + value + 1):
            cells[y][xi] = 'w'
        return x + value, y

def initDeadEnd(c='X'):
    global cells

    for j in range(15):
        for i in range(15):
            if cells[j][i] == '.':
                if len(chooseMove(i, j, '.s')) < 2:
                    cells[j][i] = c

    for j in range(14, -1, -1):
        for i in range(14, -1, -1):
            if cells[j][i] == '.':
                if len(chooseMove(i, j, '.s')) < 2:
                    cells[j][i] = c

def markCreatedDeadEnd(x, y, c = 'o'):
    global cells

    memory = []
    nextPosI = chooseMove(x, y, '.s')

    for pos in nextPosI:
        nextPos = [pos]
        while len(nextPos) == 1:
            moveDir, nX, nY = nextPos[0]
            if cells[nY][nX] == 's':
                break
            memory.append((nX, nY))
            cells[nY][nX] = 's'
            nextPos = chooseMove(nX, nY, '.s')

    for j in range(15):
        for i in range(15):
            if cells[j][i] == '.':
                if len(chooseMove(i, j, '.s')) < 2:
                    cells[j][i] = c
    for j in range(14, -1, -1):
        for i in range(14, -1, -1):
            if cells[j][i] == '.':
                if len(chooseMove(i, j, '.s')) < 2:
                    cells[j][i] = c

    markRegion(memory, '.')

def clear(cells, charToKeep=''):
    for i in range(15):
        for j in range(15):
            if cells[j][i] != charToKeep and cells[j][i] != 'x':
                cells[j][i] = '.'

def clearPath(c):
    global cells
    for i in range(15):
        for j in range(15):
            if cells[j][i] == c:
                cells[j][i] = '.'

def updateXY(x, y, c):
    if c == 'N':
        return x, y - 1

    elif c == 'S':
        return x, y + 1

    elif c == 'W':
        return x - 1, y

    elif c == 'E':
        return x + 1, y

def connectedComponentUtil(grid, x, y, idComponent):
    if x < 0 or x > 14 or y < 0 or y > 14:
        return
    if grid[y][x] == -1 or grid[y][x] == idComponent:
        return

    grid[y][x] = idComponent

    connectedComponentUtil(grid, x + 1, y, idComponent)
    connectedComponentUtil(grid, x - 1, y, idComponent)
    connectedComponentUtil(grid, x, y + 1, idComponent)
    connectedComponentUtil(grid, x, y - 1, idComponent)

def connectedComponentCountFromPosUtil(grid, x, y, idComp):
    if x < 0 or x > 14 or y < 0 or y > 14:
        return 0
    if grid[y][x] != idComp:
        return 0

    grid[y][x] = -1

    return 1 + \
        connectedComponentCountFromPosUtil(grid, x+1, y, idComp) + \
        connectedComponentCountFromPosUtil(grid, x-1, y, idComp) + \
        connectedComponentCountFromPosUtil(grid, x, y+1, idComp) + \
        connectedComponentCountFromPosUtil(grid, x, y-1, idComp)

def connectedComponentCount(grid):
    # connectedComponentGrid init
    connectedComponentGrid = []
    for j in range(15):
        connectedComponentGrid.append([])
        for i in range(15):
            if grid[j][i] == '.':
                connectedComponentGrid[j].append(0)
            else:
                connectedComponentGrid[j].append(-1)

    # For each water, assign id to every connected water
    idComponent = 0
    for j in range(15):
        for i in range(15):
            if connectedComponentGrid[j][i] == 0:
                idComponent += 1
                connectedComponentUtil(connectedComponentGrid, i, j, idComponent)

    return idComponent, connectedComponentGrid

def decideMoveLvl2(extendedChoices, idxs, pMove):
    global cells

    res = -1

    for i in idxs:
        (moveDir, nX, nY, nNbCase, nNbZone) = extendedChoices[i]
        cells[nY][nX] = '-'
        nChoice = chooseMove(nX, nY, '.')

        for (nMoveDir, nnX, nnY) in nChoice:
            cells[nnY][nnX] = '-'
            nnNbZone, _ = connectedComponentCount(cells)
            cells[nnY][nnX] = '.'
            if nnNbZone == nNbZone:
                if moveDir == pMove:
                    return i
                    break
                else:
                    return i

        cells[nY][nX] = '.'

    return res

def decideMove(x, y, choice, pMove, positionInMove=0):
    global cells

    clearPath('o')

    # Grid for connected component counting
    nbZone, grid = connectedComponentCount(cells)

    resWithEqNbZones = []
    resOthers = []
    for (moveDir, nextX, nextY) in choice:
        extendedChoice = [moveDir, nextX, nextY]

        # Nb of achievable case if following this dir
        _, grid = connectedComponentCount(cells)
        idComp = grid[nextY][nextX]  # Redo the calculus as grid is modified during calculus
        extendedChoice.append(connectedComponentCountFromPosUtil(grid, nextX, nextY, idComp))

        # Nb of zones after moving in nextX, nextY
        cells[nextY][nextX] = '-'
        nextNbZone, _ = connectedComponentCount(cells)
        cells[nextY][nextX] = '.'

        extendedChoice.append(nextNbZone)

        if nextNbZone == nbZone:
            resWithEqNbZones.append(extendedChoice)
        else:
            resOthers.append(extendedChoice)


    maxIdx = [-1]
    maxCount = -1
    if len(resWithEqNbZones) > 0:
        for i, (moveDir, nX, nY, nNbCases, nNbZone) in enumerate(resWithEqNbZones):
            if nNbCases > maxCount:
                maxCount = nNbCases
                maxIdx = [i]
            elif nNbCases == maxCount:
                maxIdx.append(i)

        if len(maxIdx) == 1 and maxIdx[0] > -1:
            return resWithEqNbZones[maxIdx[0]]

        elif len(maxIdx) > 0:
            idxOfTheGoodOne = decideMoveLvl2(resWithEqNbZones, maxIdx, pMove)
            if idxOfTheGoodOne != -1:
                return resWithEqNbZones[idxOfTheGoodOne]

    maxIdx = [-1]
    if len(resOthers) > 0:
        for i, (moveDir, nX, nY, nNbCases, nNbZone) in enumerate(resOthers):
            if nNbCases > maxCount:
                maxCount = nNbCases
                maxIdx = [i]

    if maxIdx[0] > -1:
        return resOthers[maxIdx[0]]

    a, b, c = choice[positionInMove]
    return a, b, c, -1, -1

"""
    ENNEMY
"""

class Ennemy:

    dmgOrder = [
        'TORPEDO',
        'TRIGGER'
        ]

    def __init__(self):
        global cells

        self.filterTrigger = True
        self.mineStep = []

        self.pos = []
        self.countPos = []

        self.cellsMoves = []
        self.posX = 14
        self.posY = 14

        self.awaitingOrders = []

        self.grid = []
        self.pMoves = []
        self.lastKnown = (-1, -1)
        self.pMove = ''

        self.moves = [[]]

        for j in range(29):
            self.cellsMoves.append([])
            for i in range(29):
                self.cellsMoves[j].append('.')
        self.cellsMoves[self.posY][self.posX] = 'o'

        for j in range(len(cells)):
            self.grid.append([])
            for i in range(len(cells[j])):
                if cells[j][i] != 'x':
                    self.pos.append([i, j])
                    self.grid[j].append('.')
                else:
                    self.grid[j].append('x')

        self.countPos = [1] * len(self.pos)
        self.dmgPerPos = [[0] for _ in range(len(self.pos))]
        self.paths = [ [[x, y]] for x, y in self.pos]

    """
        DEBUG
    """

    def debugPos(self):
        grid = []
        for y in range(15):
            grid.append([])
            for c in self.grid[y]:
                grid[y].append(c)
        for p in self.paths:
            (x, y) = p[-1]
            grid[y][x] = 'o'
        #for (x, y) in self.pos:
        #    grid[y][x] = 'o'
        debugGrid(grid, "Ennemy's grid")
        debug([p[-1] for p in self.paths])

    def debugCases(self, title=""):
        if title == "":
            print("  Ennemy Position:", file=sys.stderr)
            print("  Nb Pos:", len(self.pos), '/', sum(self.countPos), file=sys.stderr)
            if len(self.pos) < 9:
                print("    Pos:", self.pos, file=sys.stderr)
                #debug([p[-1] for p in self.paths],"    Paths:")
            print("    selfGrid: (" + str(self.posX) + ", " + str(self.posY) + ")", file=sys.stderr)
        else:
            debug(title)
            print("    Ennemy Position:", file=sys.stderr)
            print("      Nb Pos:", len(self.pos), '/', sum(self.countPos), file=sys.stderr)
            if len(self.pos) < 9:
                print("      Pos:", self.pos, file=sys.stderr)
                #debug([p[-1] for p in self.paths],"      Paths:")
            print("      selfGrid: (" + str(self.posX) + ", " + str(self.posY) + ")", file=sys.stderr)

    """
        CLEAN
    """

    def clearMoves(self):
        for j in range(29):
            for i in range(29):
                self.cellsMoves[j][i] = '.'
        self.posX = 14
        self.posY = 14
        self.cellsMoves[self.posY][self.posX] = 'o'

        self.pMoves = []
        self.pMove = ''

    def clearGrid(self):
        for j in range(15):
            for i in range(15):
                if self.grid[j][i] != 'x':
                    self.grid[j][i] = '.'

    """
        SELF GRID
    """

    def markMine(self, x, y):
        global cells

        for j in [-2, 2]:
            if y + j < 0 or y + j > 14: continue
            for i in [-1, 0, 1]:
                if x + i < 0 or x + i > 14: continue

                if cells[y + j][x + i] == '.':
                    cells[y + j][x + i] = 'M'

        for j in [-1, 0, 1]:
            if y + j < 0 or y + j > 14: continue
            for i in [-2, -1, 0, 1, 2]:
                if x + i < 0 or x + i > 14: continue

                if cells[y + j][x + i] == '.':
                    cells[y + j][x + i] = 'M'

    def checkReconstruction(self, isEnnemy):
        if len(self.pos) != 1: return

        x, y = self.pos[0]
        self.grid[y][x] = 'w'

        if isEnnemy:
            step = len(self.moves) - 1 + sum([len(m) for m in self.moves])

        for order, moveDir in reversed(self.pMoves):
            if order == 'MOVE':
                if isEnnemy:
                    if step in self.mineStep:
                        self.markMine(x, y)
                        del(self.mineStep[self.mineStep.index(step)])
                    step -= 1

                if moveDir == 'N':
                    y += 1
                elif moveDir == 'S':
                    y -= 1
                elif moveDir == 'W':
                    x += 1
                elif moveDir == 'E':
                    x -= 1

                self.grid[y][x] = 'w'

            if order == 'SILENCE':
                if isEnnemy:
                    if step in self.mineStep:
                        self.markMine(x, y)
                        del(self.mineStep[self.mineStep.index(step)])
                    step -= 1

                lX, lY = self.lastKnown
                if x == lX and y != lY:
                    dY = int((lY - y) / abs(lY - y))
                    for j in range(1, abs(lY - y)):
                        self.grid[y + dY * j][x] = 'j'
                elif y == lY and x != lX:
                    dX = int((lX - x) / abs(lX - x))
                    for i in range(1, abs(lX - x)):
                        self.grid[y][x + dX * i] = 'j'

        self.pMoves = []
        self.lastKnown = self.pos[0]

    """
        UPDATE POS
    """

    def updateDamages(self, diffLife, isDebug):

        if isDebug:
            debug(diffLife, "    Life lost:")
            debug(self.dmgPerPos, "    List damages:")
        idxs = [i for i in range(len(self.pos)) if diffLife in self.dmgPerPos[i]]

        LPos = [self.pos[i] for i in idxs]
        LCountPos = [self.countPos[i] for i in idxs]

        self.pos = LPos
        self.countPos = LCountPos
        self.dmgPerPos = [[0] for _ in range(len(idxs))]
        self.paths = [path for path in self.paths if path[-1] in self.pos]

    def updateDoubleShoot(self, shoot, trigger):
        sX, sY = shoot
        tX, tY = trigger

        for j in [-1, 0, 1]:
            for i in [-1, 0, 1]:
                p = [sX + i, sY + j]
                if p in self.pos:
                    idx = self.pos.index(p)
                    if i == 0 and j == 0:
                        for k in range(len(self.dmgPerPos[idx])):
                            self.dmgPerPos[idx][k] += 2
                    else:
                        for k in range(len(self.dmgPerPos[idx])):
                            self.dmgPerPos[idx][k] += 1
                p = [tX + i, tY + j]
                if p in self.pos:
                    idx = self.pos.index(p)
                    if i == 0 and j == 0:
                        for k in range(len(self.dmgPerPos[idx])):
                            self.dmgPerPos[idx][k] += 2
                    else:
                        for k in range(len(self.dmgPerPos[idx])):
                            self.dmgPerPos[idx][k] += 1

    def updateShoot(self, shoot):
        x, y = shoot

        for j in [-1, 0, 1]:
            for i in [-1, 0, 1]:
                p = [x + i, y + j]
                if p in self.pos:
                    idx = self.pos.index(p)
                    if i == 0 and j == 0:
                        for k in range(len(self.dmgPerPos[idx])):
                            self.dmgPerPos[idx][k] += 2
                    else:
                        for k in range(len(self.dmgPerPos[idx])):
                            self.dmgPerPos[idx][k] += 1

    def oldSilence(self, moves):

        LPos = []
        LCountPos = []
        LDmgPerPos = []

        for i, p in enumerate(self.pos):
            x, y = p

            dmg = self.dmgPerPos[i]
            if p not in LPos:
                LPos.append(p)
                LCountPos.append( self.countPos[i] )
                LDmgPerPos.append(copy.deepcopy(dmg))
            else:
                LCountPos[ LPos.index(p) ] += self.countPos[i]
                LDmgPerPos[ LPos.index(p) ] += copy.deepcopy(dmg)

            if not 'S' in moves:
                for i in range(1, 5):
                    if y - i > -1 and self.grid[y - i][x] == '.' and self.cellsMoves[self.posY - i][self.posX] != 'o':
                        if [x, y - i] not in LPos:
                            LPos.append([x, y - i])
                            LCountPos.append(1)
                            LDmgPerPos.append(copy.deepcopy(dmg))
                        else:
                            LCountPos[LPos.index( [x, y - i] )] += 1
                            LDmgPerPos[LPos.index( [x, y - i] )] += copy.deepcopy(dmg)
                    else:
                        break
            if not 'N' in moves:
                for i in range(1, 5):
                    if y + i < 15 and self.grid[y + i][x] == '.' and self.cellsMoves[self.posY + i][self.posX] != 'o':
                        if [x, y + i] not in LPos:
                            LPos.append([x, y + i])
                            LCountPos.append(1)
                            LDmgPerPos.append(copy.deepcopy(dmg))
                        else:
                            LCountPos[LPos.index( [x, y + i] )] += 1
                            LDmgPerPos[LPos.index( [x, y + i] )] += copy.deepcopy(dmg)
                    else:
                        break
            if not 'E' in moves:
                for i in range(1, 5):
                    if x - i > -1 and self.grid[y][x - i] == '.' and self.cellsMoves[self.posY][self.posX - i] != 'o':
                        if [x - i, y] not in LPos:
                            LPos.append([x - i, y])
                            LCountPos.append(1)
                            LDmgPerPos.append(copy.deepcopy(dmg))
                        else:
                            LCountPos[LPos.index( [x - i, y] )] += 1
                            LDmgPerPos[LPos.index( [x - i, y] )] += copy.deepcopy(dmg)
                    else:
                        break
            if not 'W' in moves:
                for i in range(1, 5):
                    if x + i < 15 and self.grid[y][x + i] == '.' and self.cellsMoves[self.posY][self.posX + i] != 'o':
                        if [x + i, y] not in LPos:
                            LPos.append([x + i, y])
                            LCountPos.append(1)
                            LDmgPerPos.append(copy.deepcopy(dmg))
                        else:
                            LCountPos[LPos.index( [x + i, y] )] += 1
                            LDmgPerPos[LPos.index( [x + i, y] )] += copy.deepcopy(dmg)
                    else:
                        break

        self.pos = LPos
        self.countPos = LCountPos
        self.dmgPerPos = LDmgPerPos

    def updatePosFromPaths(self):
        LPos = []
        LCountPos = []
        LDmgPerPos = []
        for path in self.paths:
            x, y = path[-1]
            if not path[-1] in LPos:
                idx = self.pos.index(path[-1])
                LPos.append([x, y])
                LCountPos.append(self.countPos[idx])
                LDmgPerPos.append(self.dmgPerPos[idx])
        self.pos = LPos
        self.dmgPerPos = LDmgPerPos
        self.countPos = LCountPos

    def updateSilence(self, moves):
        global cells

        self.oldSilence(moves)

        if len(self.pos) > 70 or len(self.paths) > 100:
            self.paths = [ [[x, y]] for x, y in self.pos]
            self.filterTrigger = False
        else:
            for i, path in enumerate(copy.deepcopy(self.paths)):
                x, y = path[-1]
                nPath = copy.deepcopy(path)
                if not 'S' in moves:
                    for i in range(1, 5):
                        if y - i > -1 and self.grid[y - i][x] == '.' and not [x, y - i] in nPath:
                            nPath.append( [x, y - i] )
                            self.paths.append(nPath)
                            nPath = copy.deepcopy(nPath)
                        else:
                            break
                nPath = copy.deepcopy(path)
                if not 'N' in moves:
                    for i in range(1, 5):
                        if y + i < 15 and self.grid[y + i][x] == '.' and not [x, y + i] in nPath:
                            nPath.append( [x, y + i] )
                            self.paths.append(nPath)
                            nPath = copy.deepcopy(nPath)
                        else:
                            break
                nPath = copy.deepcopy(path)
                if not 'E' in moves:
                    for i in range(1, 5):
                        if x - i > -1 and self.grid[y][x - i] == '.' and not [x - i, y] in nPath:
                            nPath.append( [x - i, y] )
                            self.paths.append(nPath)
                            nPath = copy.deepcopy(nPath)
                        else:
                            break
                nPath = copy.deepcopy(path)
                if not 'W' in moves:
                    for i in range(1, 5):
                        if x + i < 15 and self.grid[y][x + i] == '.' and not [x + i, y] in nPath:
                            nPath.append( [x + i, y] )
                            self.paths.append(nPath)
                            nPath = copy.deepcopy(nPath)
                        else:
                            break

        self.updatePosFromPaths()

        self.clearMoves()

        self.pMoves.append(('SILENCE', ''))
        self.moves.append([])

    def updateTorpedo(self, x, y):
        LPos = []
        LCountPos = []
        LDmgPerPos = []
        LPaths = []

        for i, p in enumerate(self.pos):
            if isShootPossible(x, y, p[0], p[1]):
                LPos.append(p)
                LCountPos.append(self.countPos[i])
                LDmgPerPos.append(self.dmgPerPos[i])
                for j, path in enumerate(self.paths):
                    if path[-1] == p:
                        LPaths.append(self.paths[j])

        self.pos = LPos
        self.countPos = LCountPos
        self.dmgPerPos = LDmgPerPos
        self.paths = LPaths

    def updateSurface(self, c, isEnnemy, isDebug):
        self.updateAwatingOders("", isEnnemy, isDebug)

        caseX = (c - 1) % 3
        caseY = (c - 1) // 3
        xmin = 5 * caseX
        xmax = xmin + 4
        ymin = 5 * caseY
        ymax = ymin + 4

        LPos = []
        LCountPos = []
        LDmgPerPos = []

        for i, p in enumerate(self.pos):
            if ymin <= p[1] <= ymax and xmin <= p[0] <= xmax:
                LPos.append(p)
                LCountPos.append(self.countPos[i])
                LDmgPerPos.append(self.dmgPerPos[i])

        self.pos = LPos
        self.countPos = LCountPos
        self.dmgPerPos = LDmgPerPos
        self.paths = [ [[x, y]] for x, y in self.pos]

        self.clearMoves()
        self.clearGrid()
        self.filterTrigger = False

    def updateSonar(self, sonar_result, case):
        if sonar_result == "NA":
            return

        xmin = ((case - 1) % 3) * 5
        xmax = xmin + 4
        ymin = ((case - 1) // 3) * 5
        ymax = ymin + 4

        self.debugCases("Before Sonar:")
        LPos = []
        LCountPos = []
        LDmgPerPos = []
        LPaths = []

        if sonar_result == 'Y':
            for i, p in enumerate(self.pos):
                if ymin <= p[1] <= ymax and xmin <= p[0] <= xmax:
                    LPos.append(p)
                    LCountPos.append(self.countPos[i])
                    LDmgPerPos.append(self.dmgPerPos[i])
                    for j, path in enumerate(self.paths):
                        if path[-1] == p:
                            LPaths.append(self.paths[j])
        else:
            for i, p in enumerate(self.pos):
                if not(ymin <= p[1] <= ymax and xmin <= p[0] <= xmax):
                    LPos.append(p)
                    LCountPos.append(self.countPos[i])
                    LDmgPerPos.append(self.dmgPerPos[i])
                    for j, path in enumerate(self.paths):
                        if path[-1] == p:
                            LPaths.append(self.paths[j])

        self.pos = LPos
        self.countPos = LCountPos
        self.dmgPerPos = LDmgPerPos
        self.paths = LPaths

        self.debugCases("After Sonar:")

    def updateMove(self, c, isEnnemy, isDebug):
        global cells

        self.updateAwatingOders(c, isEnnemy, isDebug)

        for p in range(len(self.pos) - 1, -1, -1):
            x, y = self.pos[p]
            if c == 'N':
                if y > 0 and self.grid[y - 1][x] == '.':
                    self.pos[p][1] -= 1
                else:
                    del(self.countPos[p])
                    del(self.dmgPerPos[p])
                    del(self.pos[p])
            elif c == 'S':
                if y < 14 and self.grid[y + 1][x] == '.':
                    self.pos[p][1] += 1
                else:
                    del(self.countPos[p])
                    del(self.dmgPerPos[p])
                    del(self.pos[p])
            elif c == 'W':
                if x > 0 and self.grid[y][x - 1] == '.':
                    self.pos[p][0] -= 1
                else:
                    del(self.countPos[p])
                    del(self.dmgPerPos[p])
                    del(self.pos[p])
            elif c == 'E':
                if x < 14 and self.grid[y][x + 1] == '.':
                    self.pos[p][0] += 1
                else:
                    del(self.countPos[p])
                    del(self.dmgPerPos[p])
                    del(self.pos[p])

        for p in range(len(self.paths) - 1, -1, -1):
            x, y = self.paths[p][-1]
            if c == 'N':
                if y > 0 and self.grid[y - 1][x] == '.' and not [x, y - 1] in self.paths[p]:
                    self.paths[p].append([x, y - 1])
                else:
                    del(self.paths[p])
            elif c == 'S':
                if y < 14 and self.grid[y + 1][x] == '.' and not [x, y + 1] in self.paths[p]:
                    self.paths[p].append([x, y + 1])
                else:
                    del(self.paths[p])
            elif c == 'W':
                if x > 0 and self.grid[y][x - 1] == '.' and not [x - 1, y] in self.paths[p]:
                    self.paths[p].append([x - 1, y])
                else:
                    del(self.paths[p])
            elif c == 'E':
                if x < 14 and self.grid[y][x + 1] == '.' and not [x + 1, y] in self.paths[p]:
                    self.paths[p].append([x + 1, y])
                else:
                    del(self.paths[p])

        if c == 'N':
            self.posY -= 1
        elif c == 'S':
            self.posY += 1
        elif c == 'W':
            self.posX -= 1
        elif c == 'E':
            self.posX += 1
        self.cellsMoves[self.posY][self.posX] = 'o'

        self.pMoves.append(('MOVE',c))
        self.pMove = c
        self.moves[-1].append(c)

    def updateMine(self, isDebug):
        step = len(self.moves) - 1 + sum([len(m) for m in self.moves])
        self.mineStep.append(step)
        if isDebug: debug(step, "Add mine at step:")

    def updateTrigger(self, x, y, isDebug):
        if not self.filterTrigger: return

        if isDebug: debug(len(self.paths), "  Before trigger filtering:")
        for i in range(len(self.paths)-1, -1, -1):
            path = self.paths[i]
            found = False

            for pX, pY in path:
                if (abs(x - pX) == 1 and y == pY) or (x == pX and abs(y - pY) == 1):
                    found = True
                    break

            if not found:
                del(self.paths[i])

        if isDebug: debug(len(self.paths), "  After trigger filtering:")

    """
        UPDATE ORDERS
    """

    def updateOrderShoot(self, shootInfo, triggInfo, isDebug):
        iShoot, shoot = shootInfo
        iTrigg, trigg = triggInfo

        if iShoot and iTrigg:
            if isDebug: debug("  iShoot and iTrigger: True")
            self.updateDoubleShoot(shoot, trigg)

        elif iShoot:
            if isDebug: debug("  iShoot: True")
            self.updateShoot(shoot)

        elif iTrigg:
            if isDebug: debug("  iTrigger: True")
            self.updateShoot(trigg)

    def updateAwatingOders(self, move, isEnnemy, isDebug):
        if len(self.awaitingOrders) == 0: return

        if isDebug: debug("    Awaiting orders:")

        for order in self.awaitingOrders:
            if isDebug: debug(order, "      Order:")

            if order[0] == 'SILENCE':
                if isDebug: self.debugCases("        Before silence:")
                self.updateSilence( (move, order[1]) )
                if isDebug: self.debugCases("        After silence:")
            elif order[0] == 'TORPEDO':
                self.updateTorpedo(int(order[1]), int(order[2]))
                self.updateShoot( (int(order[1]), int(order[2])) )
            elif order[0] == 'SHOOT':
                self.updateOrderShoot(order[1], order[2], isDebug)
            elif order[0] == 'DAMAGE':
                if isDebug: self.debugCases("        Before damages:")
                self.updateDamages(order[1], isDebug)
                if isDebug: self.debugCases("        After damages:")
            elif order[0] == 'MOVE': print("DAFUQ, you made me bug")
            elif order[0] == 'SURFACE': print("DAFUQ, you made me bug")
            elif order[0] == 'SONAR': debug("Not implemented")
            elif order[0] == 'MINE':
                self.updateMine(isDebug)
            elif order[0] == 'TRIGGER':
                self.updateShoot( (int(order[1]), int(order[2])) )
                self.updateTrigger(int(order[1]), int(order[2]), isDebug)

            self.checkReconstruction(isEnnemy)

        self.awaitingOrders = []

    def updateOrders(self, orders, shootInfo, triggInfo, lifeLost, isEnnemy, myCase=-1, isDebug=True):
        orders = orders.split('|')
        orders = [o.split() for o in orders if len(o.split()) > 0]

        ennShoot = False
        ennShootX = -1
        ennShootY = -1
        ennTrigg = False
        ennTriggX = -1
        ennTriggY = -1

        if len(self.awaitingOrders) > 0:
            self.awaitingOrders.append(['SHOOT', shootInfo, triggInfo])
        else:
            self.updateOrderShoot(shootInfo, triggInfo, isDebug)

        for i, order in enumerate(orders):
            if isDebug: debug(order, "Order:")

            if order[0] == 'MOVE':
                if isDebug: self.debugCases("  Before move:")
                self.updateMove(order[1], isEnnemy, isDebug)
                if isDebug: self.debugCases("  After move:")

            elif order[0] == 'SURFACE':
                if isDebug: self.debugCases("  Before surface:")
                if len(order) > 1:
                    self.updateSurface(int(order[1]), isEnnemy, isDebug)
                else:
                    self.updateSurface(myCase, isEnnemy, isDebug)
                if isDebug: self.debugCases("  After surface:")
                lifeLost -= 1

            elif order[0] == 'SILENCE':
                if isDebug: debug("  Added in awainting orders")
                self.awaitingOrders.append(('SILENCE', self.pMove))

            elif order[0] == 'TORPEDO':
                ennShoot = True
                ennShootX = int(order[1])
                ennShootY = int(order[2])
                if len(self.awaitingOrders) == 0:
                    if isDebug: self.debugCases("  Before torpedo:")
                    self.updateTorpedo(ennShootX, ennShootY)
                    if isDebug: self.debugCases("  After torpedo:")
                    self.updateShoot( (ennShootX, ennShootY) )
                else:
                    self.awaitingOrders.append(order)

            elif order[0] == 'SONAR':
                if isDebug: debug("  Not implemented")
            elif order[0] == 'MINE':
                if len(self.awaitingOrders) == 0:
                    self.updateMine(isDebug)
                else:
                    self.awaitingOrders.append(order)
            elif order[0] == 'TRIGGER':
                if len(self.awaitingOrders) == 0:
                    ennTrigg = True
                    ennTriggX = int(order[1])
                    ennTriggY = int(order[2])
                    self.updateShoot( (ennTriggX, ennTriggY) )
                    self.updateTrigger(ennTriggX, ennTriggY, isDebug)
                else:
                    self.awaitingOrders.append(order)

            self.checkReconstruction(isEnnemy)

        if len(self.awaitingOrders) > 0:
            self.awaitingOrders.append(('DAMAGE', lifeLost))
        else:
            if isDebug: self.debugCases("  Before damages:")
            self.updateDamages(lifeLost, isDebug)
            if isDebug: self.debugCases("  After damages:")

        return (ennShoot, (ennShootX, ennShootY)), (ennTrigg, (ennTriggX, ennTriggY))

"""
    MINES
"""

mines = []

def distanceClosest(x, y):
    global mines

    if len(mines) == 0: return 3

    minDist = 1000
    for mX, mY in mines:
        nDist = abs(x - mX) + abs(y - mY)
        if nDist < minDist:
            minDist = nDist

    return minDist

def chooseDirMine(x, y):
    global cells

    res = ''
    resX = -1
    resY = -1

    choices = []

    if y - 1 >= 0 and cells[y - 1][x] != 'x':
        choices.append(('N', x, y - 1))
    if y + 1 < 15 and cells[y + 1][x] != 'x':
        choices.append(('S', x, y + 1))
    if x - 1 >= 0 and cells[y][x - 1] != 'x':
        choices.append(('W', x - 1, y))
    if x + 1 < 15 and cells[y][x + 1] != 'x':
        choices.append(('E', x + 1, y))

    distCenter = 15
    for mDir, mx, my in choices:
        if distanceClosest(mx, my) > 2:
            nDistCenter = abs(mx - 7) + abs(my - 7)
            if nDistCenter < distCenter:
                distCenter = nDistCenter
                res = mDir
                resX = mx
                resY = my

    return res, resX, resY

def chooseTrigger(x, y, ennemy, opp_life, my_life):
    global mines

    confidence = max(ennemy.countPos) / sum(ennemy.countPos)

    if len(ennemy.pos) == 1:
        tX, tY = ennemy.pos[0]

        for mX, mY in mines:
            if x == mX and y == mY: continue

            elif mX == tX and mY == tY:
                if abs(x - mX) <= 1 and abs(y - mY) <= 1:
                    if opp_life <= 2:
                        if my_life > 1:
                            return True, mX, mY, False
                    if my_life < 4: continue

                return True, mX, mY, False

            elif abs(mX - tX) <= 1 and abs(mY - tY) <= 1:
                if opp_life == 1:
                        return True, mX, mY, False

                if abs(x - mX) <= 1 and abs(y - mY) <= 1: continue

                return True, mX, mY, False

    if confidence > 0.20:
        tX, tY = ennemy.pos[ ennemy.countPos.index(max(ennemy.countPos)) ]
        for mX, mY in mines:
            if abs(x - mX) <= 1 and abs(y - mY) <= 1: continue

            elif abs(mX - tX) <= 1 and abs(mY - tY) <= 1:
                return True, mX, mY, False

    if len(ennemy.pos) < 7:
        targetI = 0
        maxTouched = 0
        for i, mine in enumerate(mines):
            mX, mY = mine
            touching = 0
            for j in [-1, 0, 1]:
                for k in [-1, 0, 1]:
                    if mX + j == x and mY + k == y:
                        touching = -20
                    elif [mX + j, mY + k] in ennemy.pos:
                        touching +=1
            if touching > maxTouched:
                maxTouched = touching
                targetI = i

        if maxTouched > 0:
            mX, mY = mines[targetI]
            return True, mX, mY, True
    else:
        maxTouched = 0
        maxProb = 0
        targetP = 0
        for i, mine in enumerate(mines):
            mX, mY = mine
            prob = 0
            for j in [-1, 0, 1]:
                for k in [-1, 0, 1]:
                    if mX + j == x and mY + k == y:
                        prob -= sum(ennemy.countPos)
                    elif [mX + j, mY + k] in ennemy.pos:
                        prob += ennemy.countPos[ennemy.pos.index([mX + j, mY + k])]
            if prob > maxProb:
                maxProb = prob
                targetP = i

        if maxProb / sum(ennemy.countPos) > 0.5:
            mX, mY = mines[targetP]
            return True, mX, mY, True

    return False, -1, -1, False

"""
    FINISH HIM
"""

def silencePlusMoveThenShoot(cells, x, y, ennemy, opp_life, my_life):

    resTarget = 'NOPE'
    resTargetX = -1
    resTargetY = -1
    resNbDmg = -1

    resSilenceDir = ''
    resNbMove = -1
    resMoveDir = ''
    resMoveY = -1
    resMoveX = -1

    silences = chooseSilence(x, y)
    for silenceDir, nbMove in silences:
        if silenceDir == 'N':
            dX = 0
            dY = -1
        elif silenceDir == 'S':
            dX = 0
            dY = 1
        elif silenceDir == 'E':
            dX = 1
            dY = 0
        elif silenceDir == 'W':
            dX = -1
            dY = 0

        for i in range(1, nbMove + 1):
            nX = x + i * dX
            nY = y + i * dY
            cells[nY][nX] = 'w'

            target, xShoot, yShoot, nbDmg = chooseTarget(nX, nY, ennemy, opp_life, my_life, False, (-1, -1))
            if target != 'NOPE':
                if nbDmg > resNbDmg:
                    resTarget = target
                    resTargetX = xShoot
                    resTargetY = yShoot
                    resNbDmg = nbDmg

                    resSilenceDir = silenceDir
                    resNbMove = i

                    resMoveDir = ''
                    if resNbDmg >= opp_life:
                        cells[nY][nX] = '.'
                        break


            choices = chooseMove(nX, nY, '.')
            for moveDir, nnX, nnY in choices:
                target, xShoot, yShoot, nbDmg = chooseTarget(nnX, nnY, ennemy, opp_life, my_life, False, (-1, -1))
                if target != 'NOPE':
                    if nbDmg > resNbDmg:
                        resTarget = target
                        resTargetX = xShoot
                        resTargetY = yShoot
                        resNbDmg = nbDmg

                        resSilenceDir = silenceDir
                        resNbMove = i

                        resMoveDir = moveDir
                        resMoveX = nnX
                        resMoveY = nnY

                        if resNbDmg >= opp_life:
                            cells[nY][nX] = '.'
                            break

            cells[nY][nX] = '.'

            if resNbDmg >= opp_life: break

        if resNbDmg >= opp_life: break

    if resTarget == 'NOPE':
        return 'NOPE', -1, -1, -1, '', -1, '', -1, -1
    else:
        return resTarget, resTargetX, resTargetY, resNbDmg, resSilenceDir, resNbMove, resMoveDir, resMoveX, resMoveY

def silenceThenShoot(cells, x, y, ennemy, opp_life, my_life):

    resTarget = 'NOPE'
    resTargetX = -1
    resTargetY = -1

    resMoveDir = ''
    resNbMove = -1
    resNbDmg = -1

    silences = chooseSilence(x, y)
    for moveDir, nbMove in silences:
        if moveDir == 'N':
            dX = 0
            dY = -1
        elif moveDir == 'S':
            dX = 0
            dY = 1
        elif moveDir == 'E':
            dX = 1
            dY = 0
        elif moveDir == 'W':
            dX = -1
            dY = 0

        for i in range(1, nbMove + 1):
            nX = x + i * dX
            nY = y + i * dY
            target, xShoot, yShoot, nbDmg = chooseTarget(nX, nY, ennemy, opp_life, my_life, False, (-1, -1))
            if target != 'NOPE':
                if nbDmg > resNbDmg:
                    resTarget = target
                    resTargetX = xShoot
                    resTargetY = yShoot
                    resNbDmg = nbDmg

                    resMoveDir = moveDir
                    resNbMove = i
                    if resNbDmg >= opp_life: break

        if resNbDmg >= opp_life: break

    if resTarget == 'NOPE':
        return 'NOPE', -1, -1, -1, '', -1
    else:
        return resTarget, resTargetX, resTargetY, resNbDmg, resMoveDir, resNbMove

def movePlusSilenceThenShoot(cells, x, y, ennemy, torpedo_cooldown, silence_cooldow, opp_life, my_life):

    resTarget = 'NOPE'
    resTargetX = -1
    resTargetY = -1

    resMoveDir = ''
    resMoveX = -1
    resMoveY = -1
    resSilenceDir = ''
    resNbMove = -1
    resNbDmg = -1

    choice = chooseMove(x, y, '.')
    for moveDir, nX, nY in choice:

        target, xShoot, yShoot, nbDmg = chooseTarget(nX, nY, ennemy, opp_life, my_life, False, (-1, -1))
        if target != 'NOPE':
            if nbDmg > resNbDmg:
                resTarget = target
                resTargetX = xShoot
                resTargetY = yShoot
                resNbDmg = nbDmg

                resMoveDir = moveDir
                resMoveX = nX
                resMoveY = nY

                resSilenceDir = ''
                if resNbDmg >= opp_life: break

        if silence_cooldown == 0 or (silence_cooldown == 1 and torpedo_cooldown == 0):
            target, xShoot, yShoot, nbDmg, silenceDir, nbMove = silenceThenShoot(cells, nX, nY, ennemy, opp_life, my_life)
            if target != 'NOPE':
                if nbDmg > resNbDmg:
                    resTarget = target
                    resTargetX = xShoot
                    resTargetY = yShoot
                    resNbDmg = nbDmg

                    resSilenceDir = silenceDir
                    resNbMove = nbMove

                    resMoveDir = moveDir
                    resMoveX = nX
                    resMoveY = nY

                    if resNbDmg >= opp_life: break

    if resTarget == 'NOPE':
        return 'NOPE', -1, -1, -1, '', -1, -1, '', -1
    else:
        return resTarget, resTargetX, resTargetY, resNbDmg, resMoveDir, resMoveX, resMoveY, resSilenceDir, resNbMove

def finishHim(cells, x, y, ennemy, torpedo_cooldown, silence_cooldown, opp_life, my_life, redo=True):
    order = []

    if torpedo_cooldown > 1: return order, -1, -1

    nbDmg = -1

    resTargetX = -1
    resTargetY = -1

    resMoveSilence = ''
    resNbMove = -1

    resMoveDir = ''
    resMoveX = -1
    resMoveY = -1

    silenceShoot = False
    moveShoot = False

    if torpedo_cooldown == 0:
        # Try shoot
        target, xShoot, yShoot, nbDmg = chooseTarget(x, y, ennemy, opp_life, my_life, False, (-1, -1))

        if nbDmg >= opp_life:
            resTargetX = xShoot
            resTargetY = yShoot
            maxDmgs = nbDmg

    if nbDmg < opp_life and silence_cooldown == 0:
        # Try silence->shoot + silence->move->shoot
        target, xShoot, yShoot, nbDmg, moveSilence, nbMove, moveDir, moveX, moveY = silencePlusMoveThenShoot(cells, x, y, ennemy, opp_life, my_life)

        if nbDmg >= opp_life:
            resTargetX = xShoot
            resTargetY = yShoot

            resMoveSilence = moveSilence
            resNbMove = nbMove

            resMoveDir = moveDir
            resMoveX = moveX
            resMoveY = moveY

            silenceShoot = True

    if nbDmg < opp_life:
        # Try move->shoot + move->silence->shoot
        target, xShoot, yShoot, nbDmg, moveDir, moveX, moveY, moveSilence, nbMove = movePlusSilenceThenShoot(cells, x, y, ennemy, torpedo_cooldown, silence_cooldown, opp_life, my_life)

        if nbDmg >= opp_life:
            resTargetX = xShoot
            resTargetY = yShoot

            resMoveSilence = moveSilence
            resNbMove = nbMove

            resMoveDir = moveDir
            resMoveX = moveX
            resMoveY = moveY

            silenceShoot = False
            moveShoot = True

    if nbDmg < opp_life:
        if redo and my_life > 1:
            newGrid = copy.deepcopy(cells)
            clear(newGrid)
            return finishHim(newGrid, x, y, ennemy, torpedo_cooldown, silence_cooldown, opp_life, my_life - 1, redo=False)
        else:
            return order, -1, -1

    if not redo:
        order.append('SURFACE')

    if silenceShoot:
        if resMoveSilence != '':
            # Case Try silence->shoot or silence->move->shoot succeed
            order.append('SILENCE ' + resMoveSilence + ' ' + str(resNbMove))
            updateGridForSilence(x, y, resMoveSilence, resNbMove)
            if resMoveDir != '':
                if torpedo_cooldown == 1:
                    order.append('MOVE ' + resMoveDir + ' TORPEDO')
                else:
                    order.append('MOVE ' + resMoveDir + ' SILENCE')
                cells[resMoveY][resMoveX] = '-'
    elif moveShoot:
        # Case Try move->shoot or move->silence->shoot succeed
        if torpedo_cooldown == 1:
            order.append('MOVE ' + resMoveDir + ' TORPEDO')
        elif silence_cooldown == 1:
            order.append('MOVE ' + resMoveDir + ' SILENCE')
        else:
            order.append('MOVE ' + resMoveDir + ' MINE')
        cells[resMoveY][resMoveX] = '-'

        if resMoveSilence != '':
            order.append('SILENCE ' + resMoveSilence + ' ' + str(resNbMove))
            debug((resMoveX, resMoveY))
            updateGridForSilence(resMoveX, resMoveY, resMoveSilence, resNbMove)

    order.append('TORPEDO ' + str(resTargetX) + ' ' + str(resTargetY))
    return order, resTargetX, resTargetY

"""
    MAIN
"""

width, height, my_id = [int(i) for i in input().split()]
for i in range(height):
    line = input()
    cells.append([])
    for c in line:
        cells[i].append(c)

initDeadEnd()

debugGrid()

x, y = chooseStart(my_id, 5, 5)
print(x, y)

ennemy = Ennemy()
fakeMe = Ennemy()

nbWater = sum([sum([1 for c in row if c == '.']) for row in cells])
debug(nbWater, "Number case with water:")

pLife = 6
pOppLife = 6
triggerEscape = False
positionInMove = 0

nbMovesWithoutSilence = 0
maxMovesBeforeSilence = 30
wTorpedo = 3
wMove = 1
wSonar = 2

iShoot = False
shoot = (-1, -1)

iTrigger = False
trigX = -1
trigY = -1

pMove = ''

caseSonar = -1

while True:
    x, y, my_life, opp_life, torpedo_cooldown, sonar_cooldown, silence_cooldown, mine_cooldown = [int(i) for i in input().split()]
    sonar_result = input()
    opponent_orders = input()
    start = time.time()

    nbDealtCell = sum([sum([1 for c in row if c == '-' or c == 'o' or c == 'w']) for row in cells])

    #if 2 * nbDealtCell > nbWater:
    #    markWater(cells, 'M')


    cells[y][x] = '-'

    if my_life < pLife:
        triggerEscape = True

    debug("Update Ennemy's info:")
    ennemy.updateSonar(sonar_result, caseSonar)
    ennShootInfo, ennTriggInfo = ennemy.updateOrders(
        opponent_orders, (iShoot, shoot), (iTrigger, (trigX, trigY)), pOppLife - opp_life, True, isDebug=True)

    iTrigger = False
    iShoot = False

    ennemy.debugPos()

    markCreatedDeadEnd(x, y)
    debug(mines, "MINES:")
    debugGrid()

    iTrigger, trigX, trigY, triggerSonar = chooseTrigger(x, y, ennemy, opp_life, my_life)
    if iTrigger:
        action = "TRIGGER " + str(trigX) + " " + str(trigY)
        mines.remove((trigX, trigY))
    else:
        action = ""

    ###############
    # FINISH HIM

    orders = []
    if len(ennemy.pos) == 1:
        orders, finX, finY = finishHim(cells, x, y, ennemy, torpedo_cooldown, silence_cooldown, opp_life, my_life)

    if len(orders) == 0:
        ###############
        # SILENCES

        silences = []
        if silence_cooldown == 0:
            if triggerEscape:
                silences = chooseSilence(x, y)
                print("Silence:", silences, file=sys.stderr)
            elif nbMovesWithoutSilence > maxMovesBeforeSilence:
                silences = [('N', 0)]

        ###############
        # REST

        actionMove = ""

        choice = chooseMove(x, y, '.')
        debug(choice, "Choice Moves:")

        if len(choice) == 0:
            clearPath('o')
            choice = chooseMove(x, y, '.')
            debug(choice, "Choice Moves:")

        if len(choice) == 0:
            if positionInMove == 0:
                positionInMove = -1
            else:
                positionInMove = 0
            clear(cells, 'M')

            action += " | SURFACE"
        else:
            target = "NOPE"

            if torpedo_cooldown == 0:
                target, xShoot, yShoot, _ = chooseTarget(x, y, ennemy, opp_life, my_life, triggerSonar, (trigX, trigY))
                if target != "NOPE":
                    action += " | " + target
                    iShoot = True
                    triggerEscape = True
                    torpedo_cooldown = 3
                    shoot = (xShoot, yShoot)
                    nbMovesWithoutSilence = maxMovesBeforeSilence + 1

            if sonar_cooldown == 0:
                scoreSonar = chooseSonar(ennemy)
                value = max(scoreSonar)
                idxCase = scoreSonar.index(value)
                scoreSonar[idxCase] = -50
                if max(scoreSonar) != 0:
                    if len(ennemy.pos) > 10 or value - max(scoreSonar) > 3:
                        caseSonar = idxCase + 1
                        action += " | SONAR " + str(caseSonar)
                    sonar_cooldown = 4

            if mine_cooldown == 0:
                dirMine, mX, mY = chooseDirMine(x, y)
                if dirMine != '':
                    action += " | MINE " + dirMine
                    mine_cooldown = 3
                    mines.append((mX, mY))

            if len(silences) > 0:
                direction, value = silences[positionInMove]

                x, y = updateGridForSilence(x, y, direction, value)
                action += " | SILENCE " + direction + " " + str(value)

                cells[y][x] = '-'

                nbMovesWithoutSilence = 0
                triggerEscape = False

                if target == "NOPE" and torpedo_cooldown == 0:
                    target, xShoot, yShoot, _ = chooseTarget(x, y, ennemy, opp_life, my_life, triggerSonar, (trigX, trigY))
                    if target != "NOPE":
                        action += " | " + target
                        iShoot = True
                        triggerEscape = True
                        torpedo_cooldown = 3
                        shoot = (xShoot, yShoot)
                        nbMovesWithoutSilence = maxMovesBeforeSilence + 1

                if mine_cooldown == 0:
                    dirMine, mX, mY = chooseDirMine(x, y)
                    if dirMine != '':
                        action += " | MINE " + dirMine
                        mine_cooldown = 3
                        mines.append((mX, mY))

                choice = chooseMove(x, y, '.')
                debug(choice, "Choice Moves:")

            if len(choice) > 0:
                moveDir, x, y, _, _ = decideMove(x, y, choice, pMove, positionInMove)
                actionMove = "MOVE " + moveDir
                pMove = moveDir

                torpedo_cooldown, silence_cooldown, mine_cooldown, sonar_cooldown, power = \
                    choosePower(
                        ennemy, fakeMe, triggerEscape,
                        torpedo_cooldown, silence_cooldown, mine_cooldown, sonar_cooldown)

                actionMove += power
                nbMovesWithoutSilence += wMove

                action += " | " + actionMove

                if target == "NOPE" and torpedo_cooldown == 0:
                    target, xShoot, yShoot, _ = chooseTarget(x, y, ennemy, opp_life, my_life, triggerSonar, (trigX, trigY))
                    if target != "NOPE":
                        action += " | " + target
                        iShoot = True
                        triggerEscape = True
                        torpedo_cooldown = 3
                        shoot = (xShoot, yShoot)
                        nbMovesWithoutSilence = maxMovesBeforeSilence + 1

                if mine_cooldown == 0:
                    dirMine, mX, mY = chooseDirMine(x, y)
                    if dirMine != '':
                        action += " | MINE " + dirMine
                        mine_cooldown = 3
                        mines.append((mX, mY))

                if sonar_cooldown == 0:
                    scoreSonar = chooseSonar(ennemy)
                    value = max(scoreSonar)
                    idxCase = scoreSonar.index(value)
                    scoreSonar[idxCase] = -50
                    if max(scoreSonar) != 0:
                        if len(ennemy.pos) > 10 or value - max(scoreSonar) > 3:
                            caseSonar = idxCase + 1
                            action += " | SONAR " + str(caseSonar)
                        sonar_cooldown = 4

    else:
        iShoot = True
        shoot = (finX, finY)
        for order in orders:
            action += " | " + order


    if "SURFACE" in action:
        fakeMe.updateOrders(action, ennShootInfo, ennTriggInfo, pLife - my_life, False, 3 * (y // 5) + x // 5 + 1, False)
    else:
        fakeMe.updateOrders(action, ennShootInfo, ennTriggInfo, pLife - my_life, False, isDebug=False)

    pLife = my_life
    pOppLife = opp_life

    t = time.time() - start

    msg = " | MSG \U0001F3AF" + str(len(ennemy.pos)) + "-" + str(len(fakeMe.pos)) \
        + "\U0001F446" + str(len(ennemy.paths)) + str(ennemy.pos[ennemy.countPos.index(max(ennemy.countPos))]) \
        + str(round(t * 1000, 1)) + "ms"

    if len(orders) > 0:
        msg += " \U0001F92F"

    print(action + msg)
