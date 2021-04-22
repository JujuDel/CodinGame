import sys
import math
import time

current_ms_time = lambda: int(round(time.time() * 1000))

"""
Ongoing:
 - Use dead end for ennemies

TODO:
 - Use dead end for pellets
 - Add retro-tracking

Suggestion:
 - Move accordingly to ennemy's pos
    -> don't go in cell if cell is next to killing enemy and cooldown > 0

Need correction:
 - Avoid two pacs on same pellet when diagonally close

Done:
 - Avoid trying to kill ennemy if its cooldown == 0
 - Request shift form if close (d < 5) to ennemy's pac
 - Real distance and not euclidean distance
 - Remove unseen pellets
 - Save pellets info with '.'/'?' for unknown, ' ' for empty and 'o'/'O' for seen or visible
 - When closest found, check if in marked list and unmark it if distance is shorter
"""

###########################
#          DEBUG          #
###########################


def debug(*args):
    print(*args, file=sys.stderr)

def debugGrid(grid, knownPellets, unknownPellets):
    debug("GRID:")
    for j in range(len(grid)):
        line = ''
        for i in range(len(grid[j])):
            if (i,j) in knownPellets:
                if grid[j][i] == 'X':
                    line += 'O'
                else:
                    line += 'o'
            elif (i,j) in unknownPellets:
                if grid[j][i] == 'X':
                    line += '?'
                else:
                    line += '.'
            else:
                line += grid[j][i]
        debug(line)


###########################
#       PATH FINDING      #
###########################


def getNeighbors(node, prevs, grid, w, h):
    x, y = node
    nextNodes = []
    
    if x == 0:
        if not (w - 1, y) in prevs: nextNodes.append((w - 1, y))
    elif grid[y][x - 1] != '#':
        if not (x - 1, y) in prevs: nextNodes.append((x - 1, y))

    if x + 1 == w:
        if not (0, y) in prevs: nextNodes.append((0, y))
    elif grid[y][x + 1] != '#':
        if not (x + 1, y) in prevs: nextNodes.append((x + 1, y))

    if y > 0 and grid[y - 1][x] != '#':
        if not (x, y - 1) in prevs: nextNodes.append((x, y - 1))

    if y + 1 < h and grid[y + 1][x] != '#':
        if not (x, y + 1) in prevs: nextNodes.append((x, y + 1))
    
    return nextNodes

def filterFromAllies(pacman, ID, nodes):
    res = []
    for n in nodes:
        toAdd = True
        for i, pac in pacman.items():
            if pac.activated and i != ID and n[0] == pac.X and n[1] == pac.Y:
                toAdd = False
                break

        if toAdd:
            res.append(n)

    return res

def filterFromEnnemies(nodes, RPS_win_loose, myType, seenEnnemies, enemies):
    res = []
    for n in nodes:
        toAdd = True
        for eID in seenEnnemies:
            x, y, eType, speed, cooldown = ennemies[eID].getInfo()
            dist = distanceEuc(n, (x, y))
            if RPS_win_loose[eType] == myType and dist < 2:
                toAdd = False
                break
            elif dist < 2 and cooldown == 0:
                toAdd = False
                break
        if toAdd:
            res.append(n)
    return res

def findClosestPellet(pacman, ID, seenEnnemies, enemies, RPS_win_loose, pellets, markedPellets, grid, w, h):
    pac = pacman[ID]
    nodes = [(pac.X, pac.Y)]
    done = []

    oneMore = []

    count = 0
    while len(oneMore) == 0 and len(nodes) > 0:
        #debug(nodes)
        toReturn = []
        for i in range(len(nodes) - 1, -1, -1):
            node = nodes[i]
            if node in pellets:
                if node in markedPellets:
                    otherDist, otherID = markedPellets[node]
                    if otherDist <= count:
                        done.append(node)
                        del(nodes[i])
                    else:
                        toReturn.append( [[node], count, False] )

                elif pac.speedLeft > 0 and count < 2:
                    oneMore.append(node)
                    done += nodes[i+1:]
                    nodes = []
                    break

                else:
                    toReturn.append( [[node], count, True] )
            else:
                done.append(node)
    
        if len(toReturn) > 0:
            return toReturn

        nNodes = []
        for node in nodes:
            tmpNodes = getNeighbors(node, done + nNodes, grid, w, h)
            tmpNodes = filterFromAllies(pacman, ID, tmpNodes)
            if pac.cooldown > 0:
                nNodes +=  filterFromEnnemies(tmpNodes, RPS_win_loose, pac.type, seenEnnemies, enemies)
            else:
                nNodes += tmpNodes
        nodes = nNodes
        count += 1
    
    if len(oneMore) > 0:
        toReturn = []
        
        node = oneMore[0]
        tmpNodes = getNeighbors(node, done, grid, w, h)
        tmpNodes = filterFromAllies(pacman, ID, tmpNodes)
        nNodes = []
        if pac.cooldown > 0:
            nNodes +=  filterFromEnnemies(tmpNodes, RPS_win_loose, pac.type, seenEnnemies, enemies)
        else:
            nNodes += tmpNodes
        
        for nn in nNodes:
            if nn in pellets:
                toReturn.append( [[node, nn], count, not nn in markedPellets] )

        if len(toReturn) > 0:
            return toReturn

        if len(nNodes) > 0:
            return [[[node, nNodes[0]], count, True]]
        else:
            return [[[node], count - 1, True]]
    
    return [[[], -1, False]]

def bestChoices(fromNode, patterns, pellets, superPellets, grid, w, h):
    debug(patterns)
    if len(patterns[0][0]) == 0:
        return [], -1, False
    elif len(patterns) == 1:
        return patterns[0]

    bestI = 0
    maxPellets = -1
    for i, value in enumerate(patterns):
        coords, dist, firstOnIt = value

        marked = {}
        marked[fromNode] = 1
        if len(coords) > 1:
            marked[coords[0]] = 1

        nodes = [coords[-1]]
        count = 0  
        while len(nodes) > 0:
            nNodes = []
            
            for (x, y) in nodes: 
                for nNode in getNeigbhors(grid, w, h, x, y):
                    if not nNode in marked:
                        if nNode in pellets:
                            count += 1
                            nNodes.append(nNode)
                        elif nNode in superPellets:
                            count += 10
                            nNodes.append(nNode)
                        
                        marked[nNode] = 1
            nodes = nNodes
        
        if count > maxPellets:
            maxPellets = count
            bestI = i
    
    return patterns[bestI]


##### Ennemies retro-tracking ####

# Implementation not finished yet
def findAndUpdateEnnPacSuperPelet(superPellet, ennemies, grid, w, h):
    tX, tY = superPellet

    candidatesID = []

    for ID, ePac in ennemies.item():
        eucDist = abs(ePac.X - tX) + abs(ePac.Y - tY)
        if eucDist > ePac.count:
            continue

        node = (ePac.X, ePac.Y)
        nodes = {}
        nodes[node] = []
        while count > 0:
            for (x, y), v in nodes.item():
                for node in getNeigbhors(grid, w, h, x, y, meIncluded=True):
                    nNodes[node] = 1
            nodes = nNodes
            count -= 1
        
        if superPellet in nodes:
            candidatesID.append(ID)
        
        if len(candidatesID) > 1:
            break
    
    if len(candidatesID) == 1:
        ePac = ennemies[candidatesID[0]]


###########################
#          PACMAN         #
###########################


class PacMan:
    def __init__(self):
        self.X = self.Y = 0
        self.tX = self.tY = self.tV = -1
        
        self.available = False
        self.activated = False

        self.cooldown = 10
        self.speedLeft = 0
        self.type = ""
        self.nextType = ""
        self.doSpeed = False

    def reinit(self):
        self.tX = self.tY = self.tV = -1
        self.available = True
        self.activated = True
    
    def triggerSpeed(self):
        if self.cooldown == 0:
            self.available = False
            self.doSpeed = True

    def setTarget(self, x, y, v):
        self.tX = x
        self.tY = y
        self.tV = v
        self.available = False

    def generateAction(self, ID):
        if not self.activated or self.available:
            return ""
        
        if self.speedLeft > 0:
            uniS = "\U0001F47A "
        else:
            uniS = ""
        
        if self.type == "ROCK":
            uni = "\U0001F9F1"
        elif self.type == "PAPER":
            uni = "\U0001F9FB"
        elif self.type == "SCISSORS":
            uni = "\U00002702"
        else:
            uni = "\U00002749"

        debug("Action", self.cooldown, self.nextType)
        if self.cooldown == 0:
            if self.nextType != "":
                nextType = self.nextType
                self.nextType = ""
                return "SWITCH {} {}".format(ID, nextType)
            elif self.doSpeed:
                self.doSpeed = False
                return "SPEED {}".format(ID)
        return "MOVE {} {} {} {}".format(ID, self.tX, self.tY, uniS + uni)

def findClosestPac(x, y, pacman):
    res = []
    for ID, pac in pacman.items():
        if pac.activated and pac.available:
            d = abs(y - pac.Y) + abs(x - pac.X)
            res.append((d, ID, (x, y, v)))
    res.sort(key=lambda tup:tup[0])
    return res

def assignClosests(pacman, closestPerPellet):
    debug("SUPER PELLET:")

    if len(closestPerPellet) == 0:
        return
    
    elif len(closestPerPellet) == 1:
        closest = closestPerPellet[0]
        x, y, v = closest[0][2]
        pacman[closest[0][1]].setTarget(x, y, v)
        debug(closest[0][1], "on", (x, y))
    
    else:
        IDsDone = []
        for closest in closestPerPellet:
            for d, ID, (x, y, v) in closest:
                if not ID in IDsDone:
                    pacman[ID].setTarget(x, y, v)
                    debug(ID, "on", (x, y))
                    IDsDone.append(ID)
                    break

def findClosestPacFromEnnemy(grid, posEnnemy, pacman):
    eX, eY = posEnnemy
    w, h = len(grid[0]), len(grid)
    D = 6
    resID = 5

    for ID, pac in pacman.items():
        if not pac.activated:
            continue

        x = pac.X
        y = pac.Y

        if x == eX:
            if y == eY or abs(y - eY) > 5:
                continue

            dirY = int(abs(eY - y) / (eY - y))
            d = 0

            while 0 <= y < h and y != eY:
                if grid[y][x] == '#':
                    break
                y += dirY
                d += 1

            if y == eY:
                if d < D:
                    D = d
                    resID = ID

        elif y == eY:
            if eX == x:
                continue
    
            if grid[y][0] == '#' and grid[y][-1] == '#':
                if abs(eX - x) > 5:
                    continue
                dirX = int(abs(eX - x) / (eX - x))

            else:
                if eX > x:
                    if eX - x > 5 and x + w - 1 - eX > 5:
                        continue
                    elif eX - x <= 5:
                        dirX = 1
                    else:
                        dirX = -1
                else:
                    if x - eX > 5 and w - 1 - x + eX > 5:
                        continue
                    elif x - eX <= 5:
                        dirX = -1
                    else:
                        dirX = 1

            d = 0
            while x != eX:
                if grid[y][x] == '#':
                    break
                x += dirX
                d += 1
                if x < 0: x = w - 1
                elif x > w - 1: x = 0

            if x == eX:
                if d < D:
                    D = d
                    resID = ID
            
    return resID, D

pacman = {}
for ID in '01234':
    pacman[ID] = PacMan()

def deactivateAll(pacman):
    for ID, pac in pacman.items():
        pac.activated = False

def killHim(pac, eX, eY, pellets, grid, w, h):
    if pac.speedLeft == 0:
        return eX, eY
    
    toRet = None

    x, y = pac.X, pac.Y

    if y == eY: dirY = 0
    else: dirY = int((eY - y) / abs(eY - y))

    # Special cases
    if eX == 0 or eX == w - 1:
        if x == eX:
            if 0 <= y + 2 * dirY < h and grid[y + 2 * dirY][eX] != '#':
                if (eX, y + 2 * dirY) in pellets:
                    return eX, y + 2 * dirY
                else:
                    toRet = (eX, y + 2 * dirY)
            elif x == 0:
                if (w - 1, eY) in pellets:
                    return w - 1, eY
                elif toRet == None:
                    toRet = (w - 1, eY)
            
            if (0, eY) in pellets:
                return 0, eY
            else:
                if toRet == None:
                    return 0, eY
                return toRet
        
        # eY == y, x == w-1 and eX = 0
        elif x == w - 1:
            if grid[eY][1] != '#':
                if (1, y) in pellets:
                    return 1, y
                else:
                    toRet = (1, y)
            elif y - 1 >= 0 and grid[y - 1][eX] != '#':
                if (eX, y - 1) in pellets:
                    return eX, y - 1
                elif toRet == None:
                    toRet = (eX, y - 1)
            elif y + 1 < h and grid[y + 1][eX] != '#':
                if (eX, y + 1) in pellets:
                    return eX, y + 1
                elif toRet == None:
                    toRet = (eX, y + 1)
            
            if toRet == None:
                return eX, eY
            else:
                return toRet

        # eY == y, x == 0 and eX = w-1
        else:
            if grid[eY][w - 2] != '#':
                if (w - 2, y) in pellets:
                    return w - 2, y
                else:
                    toRet = (w - 2, y)
            elif y - 1 >= 0 and grid[y - 1][eX] != '#':
                if (eX, y - 1) in pellets:
                    return eX, y - 1
                elif toRet == None:
                    toRet = (eX, y - 1)
            elif y + 1 < h and grid[y + 1][eX] != '#':
                if (eX, y + 1) in pellets:
                    return eX, y + 1
                elif toRet == None:
                    toRet = eX, y + 1
            
            if toRet == None:
                return eX, eY
            else:
                return toRet

    if x == eX: dirX = 0
    else: dirX = int((eX - x) / abs(eX - x))

    if dirX == 0:
        if 0 <= y + 2 * dirY < h and grid[y + 2 * dirY][eX] != '#':
            if (eX, y + 2 * dirY) in pellets:
                return eX, y + 2 * dirY
            else:
                toRet = (eX, y + 2 * dirY)
        elif eX - 1 >= 0 and grid[eY][eX - 1] != '#':
            if (eX - 1, eY) in pellets:
                return eX - 1, eY
            elif toRet == None:
                toRet = (eX - 1, eY)
        elif eX + 1 < w and grid[eY][eX + 1] != '#':
            if (eX + 1, eY) in pellets:
                return eX + 1, eY
            elif toRet == None:
                toRet = (eX + 1, eY)
        
        if toRet == None:
            return eX, eY
        else:
            return toRet
    
    # dirY == 0
    else:
        if 0 <= x + 2 * dirX < w and grid[eY][x + 2 * dirX] != '#':
            if (x + 2 * dirX, eY) in pellets:
                return x + 2 * dirX, eY
            else:
                toRet = (x + 2 * dirX, eY)
        elif eY - 1 >= 0 and grid[eY - 1][eX] != '#':
            if (eX, eY - 1) in pellets:
                return eX, eY - 1
            elif toRet == None:
                toRet = (eX, eY - 1)
        elif eY + 1 < h and grid[eY + 1][eX] != '#':
            if (eX, eY + 1) in pellets:
                return eX, eY + 1
            elif toRet == None:
                toRet = (eX, eY + 1)
        
        if toRet == None:
            return eX, eY
        else:
            return toRet


###########################
#          ENNEMY         #
###########################

class EnnemyPacMan:
    def __init__(self, ID):
        self.presence = False
        self.activated = False
        self.ID = ID

        self.X = self.Y = -1
        self.speedTurnsLeft = 0
        self.abilityCooldown = 10
        self.typeID = ""

        self.count = 0

    def setInfo(self, x, y, typeID, speedTurnsLeft, abilityCooldown, activate = True):
        self.presence = True
        self.activated = activate

        self.X = x
        self.Y = y
        self.typeID = typeID
        self.speedTurnsLeft = speedTurnsLeft
        self.abilityCooldown = abilityCooldown

        self.count = 0
    
    def getInfo(self):
        return self.X, self.Y, self.typeID, self.speedTurnsLeft, self.abilityCooldown

def deactivateEnnemies(ennemies):
    for ID in '01234':
        ennemies[ID].activated = False

ennemies = {}
for ID in '01234':
    ennemies[ID] = EnnemyPacMan(ID)

###########################
#   ROCK-PAPER-SCISSORS   #
###########################


RPS_win_loose = {
    "ROCK": "SCISSORS",
    "PAPER": "ROCK",
    "SCISSORS":"PAPER"
}

RPS_loose_win = {
    "ROCK": "PAPER",
    "PAPER": "SCISSORS",
    "SCISSORS":"ROCK"
}


###########################
#           GRID          #
###########################


def safeMove(fromPos, avoid, pellets, superPellets, grid, w, h):
    x, y = fromPos
    eX, eY = avoid

    # Try to get a superPellet
    if x == 0 and (w - 1, y) in superPellets and (w - 1, y) != (eX, eY):
        return (w - 1, y), True
    if x == w - 1 and (0, y) in superPellets and (0, y) != (eX, eY):
        return (0, y), True

    if x - 1 >= 0 and (x - 1, y) in superPellets and (x - 1, y) != (eX, eY):
        return (x - 1, y), True
    if x + 1 < w and (x + 1, y) in superPellets and (x + 1, y) != (eX, eY):
        return (x + 1, y), True
    if y - 1 >= 0 and (x, y - 1) in superPellets and (x, y - 1) != (eX, eY):
        return (x, y - 1), True
    if y + 1 < h and (x, y + 1) in superPellets and (x, y + 1) != (eX, eY):
        return (x, y + 1), True
    
    # Try to get a pellet
    if x == 0 and (w - 1, y) in pellets and (w - 1, y) != (eX, eY):
        return (w - 1, y), True
    if x == w - 1 and (0, y) in pellets and (0, y) != (eX, eY):
        return (0, y), True

    if x - 1 >= 0 and (x - 1, y) in pellets and (x - 1, y) != (eX, eY):
        return (x - 1, y), True
    if x + 1 < w and (x + 1, y) in pellets and (x + 1, y) != (eX, eY):
        return (x + 1, y), True
    if y - 1 >= 0 and (x, y - 1) in pellets and (x, y - 1) != (eX, eY):
        return (x, y - 1), True
    if y + 1 < h and (x, y + 1) in pellets and (x, y + 1) != (eX, eY):
        return (x, y + 1), True
    
    # Just avoid being in eX, eY and if possible hide
    if x == 0 and grid[y][w - 1] != '#' and (w - 1, y) != (eX, eY):
        return (w - 1, y), False
    if x == w - 1 and grid[y][0] != '#' and (0, y) != (eX, eY):
        return (0, y), False

    if x - 1 >= 0 and grid[y][x - 1] != '#' and (x - 1, y) != (eX, eY):
        return (x - 1, y), False
    if x + 1 < w and grid[y][x + 1] != '#' and (x + 1, y) != (eX, eY):
        return (x + 1, y), False
    if y - 1 >= 0 and grid[y - 1][x] != '#' and (x, y - 1) != (eX, eY):
        return (x, y - 1), False
    if y + 1 < h and grid[y + 1][x] != '#' and (x, y + 1) != (eX, eY):
        return (x, y + 1), False
    return (x, y), False

def nbNeigbhors(grid, w, h, x, y):
    res = 0

    if y + 1 < h and grid[y + 1][x] == ' ':
        res += 1
    if y - 1 >= 0 and grid[y - 1][x] == ' ':
        res += 1
    
    if x == 0:
        if grid[y][w - 1] == ' ':
            res += 1
    elif grid[y][x - 1] == ' ':
        res += 1

    if x == w - 1:
        if grid[y][0] == ' ':
            res += 1
    elif grid[y][x + 1] == ' ':
        res += 1

    return res

def getNeigbhors(grid, w, h, x, y, meIncluded = False):
    res = [(x, y)] if meIncluded else []

    if y + 1 < h and grid[y + 1][x] == ' ':
        res += [(x, y + 1)]
    if y - 1 >= 0 and grid[y - 1][x] == ' ':
        res += [(x, y - 1)]
    
    if x == 0:
        if grid[y][w - 1] == ' ':
            res += [(w - 1, y)]
    elif grid[y][x - 1] == ' ':
        res += [(x - 1, y)]

    if x == w - 1:
        if grid[y][0] == ' ':
            res += [(0, y)]
    elif grid[y][x + 1] == ' ':
        res += [(x + 1, y)]

    return res

def markDeadEnd(grid, w, h):
    for j in range(h):
        for i in range(w):
            if grid[j][i] == ' ' and nbNeigbhors(grid, w, h, i, j) == 1:
                grid[j][i] = 'X'
    for j in range(h):
        for i in range(w):
            if grid[j][i] == ' ' and nbNeigbhors(grid, w, h, i, j) == 1:
                grid[j][i] = 'X'

    for j in range(h - 1, -1, -1):
        for i in range(w - 1, -1, -1):
            if grid[j][i] == ' ' and nbNeigbhors(grid, w, h, i, j) == 1:
                grid[j][i] = 'X'
    for j in range(h - 1, -1, -1):
        for i in range(w - 1, -1, -1):
            if grid[j][i] == ' ' and nbNeigbhors(grid, w, h, i, j) == 1:
                grid[j][i] = 'X'

def distanceEuc(n1, n2):
    return abs(n1[1] - n2[1]) + abs(n1[0] - n2[0])

def removeUnseenPellets(knownPellets, unknownPellets, emptyPellets, visiblePellet, pacman, grid, w, h):
    for ID, pac in pacman.items():
        if not pac.activated: continue
        x, y = pac.X, pac.Y
        
        knownPellets.pop((x, y), None)
        unknownPellets.pop((x, y), None)
        emptyPellets[(x, y)] = 1

        # WEST VISION
        nX, nY = x - 1, y
        if nX < 0: nX = w - 1
        while grid[nY][nX] != '#' and nX != x:
            if not (nX, nY) in visiblePellet:
                knownPellets.pop((nX, nY), None)
                unknownPellets.pop((nX, nY), None)
                emptyPellets[(nX, nY)] = 1
            
            if nX == 0: nX = w - 1
            else: nX -= 1
        
        # EAST VISION
        if nX != x:
            nX = x + 1
            if nX == w: nX = 0
            while grid[nY][nX] != '#':
                if not (nX, nY) in visiblePellet:
                    knownPellets.pop((nX, nY), None)
                    unknownPellets.pop((nX, nY), None)
                    emptyPellets[(nX, nY)] = 1
                
                if nX == w - 1: nX = 0
                else: nX += 1
        
        # NORTH VISION
        nX, nY = x, y - 1
        while nY >= 0 and grid[nY][nX] != '#':
            if not (nX, nY) in visiblePellet:
                knownPellets.pop((nX, nY), None)
                unknownPellets.pop((nX, nY), None)
                emptyPellets[(nX, nY)] = 1
            nY -= 1

        # SOUTH VISION
        nY = y + 1
        while nY < h and grid[nY][nX] != '#':
            if not (nX, nY) in visiblePellet:
                knownPellets.pop((nX, nY), None)
                unknownPellets.pop((nX, nY), None)
                emptyPellets[(nX, nY)] = 1
            nY += 1

width, height = map(int, input().split())
grid = []
for i in range(height):
    row = input()  # one line of the grid: space " " is floor, pound "#" is wall
    line = [c for c in row]
    grid.append(line)

markDeadEnd(grid, width, height)


###########################
#         PELLETS         #
###########################

pSuperPellets = {}

emptyPellets = {}
knownPellets = {}
unknownPellets = {}

for j in range(height):
    for i in range(width):
        if grid[j][i] != '#':
            knownPellets[(i, j)] = '?'

def getNextDeadEnds(node, avoid, grid, w, h):
    res = []
    x, y = node

    # Special cases
    if x == 0 and grid[y][w - 1] == 'X' and not (w - 1, y) in avoid:
        res.append((w - 1, y))
    if x == w - 1 and grid[y][0] == 'X' and not (0, y) in avoid:
        res.append((0, y))

    if x - 1 >= 0 and grid[y][x - 1] == 'X' and not (x - 1, y) in avoid:
        res.append((x - 1, y))
    if x + 1 < w and grid[y][x + 1] == 'X' and not (x + 1, y) in avoid:
        res.append((x + 1, y))
    if y - 1 >= 0 and grid[y - 1][x] == 'X' and not (x, y - 1) in avoid:
        res.append((x, y - 1))
    if y + 1 < h and grid[y + 1][x] == 'X' and not (x, y + 1) in avoid:
        res.append((x, y + 1))
    
    return res

def updateVisibleInDeadEnd(x, y, knownPellets, unknownPellets, emptyPellets, grid, w, h):
    nodes = [(x, y)]
    done = {}

    while len(nodes) > 0:
        nNodes = []
        for n in nodes:
            if n in emptyPellets:
                knownPellets.pop(n, None)
                unknownPellets.pop(n, None)
            else:
                knownPellets[n] = 1
                unknownPellets.pop(n, None)

                tmpNodes = getNextDeadEnds(n, done, grid, w, h)
                if len(tmpNodes) > 0:
                    nNodes += tmpNodes
                    for tn in tmpNodes:
                        done[tn] = 1
        
        nodes = nNodes


###########################
#           MAIN          #
###########################

debugGrid(grid, knownPellets, unknownPellets)
debug("")

start = True
while True:
    start_time = current_ms_time()

    deactivateAll(pacman)
    deactivateEnnemies(ennemies)

    my_score, opponent_score = map(int, input().split())

    visible_pac_count = int(input())
    seenEnnemies = []
    for i in range(visible_pac_count):
        ID, me, x, y, typeID, speedTurnsLeft, abilityCooldown = input().split()
        if typeID == 'DEAD': continue
        me = (me != "0")
        x, y = int(x), int(y)
        speedTurnsLeft = int(speedTurnsLeft)
        abilityCooldown = int(abilityCooldown)
        if me:
            pacman[ID].reinit()
            pacman[ID].X = x
            pacman[ID].Y = y
            pacman[ID].type = typeID
            pacman[ID].speedLeft = speedTurnsLeft
            pacman[ID].cooldown = abilityCooldown
            debug("ME:")
            if start:
                debug(ID, x, y, typeID, speedTurnsLeft, abilityCooldown)
                debug("OPPONENT by symetry:")
                debug(ID, width - 1 - x, y, typeID, 0, 0, "\n")
                ennemies[ID].setInfo(width - 1 - x, y, typeID, 0, 0, False)
                unknownPellets.pop((width - 1 - x, y), None)
                knownPellets.pop((width - 1 - x, y), None)
            else:
                debug(ID, x, y, typeID, speedTurnsLeft, abilityCooldown, "\n")
        else:
            debug("OPPONENT:")
            debug(ID, x, y, typeID, speedTurnsLeft, abilityCooldown, "\n")
            ennemies[ID].setInfo(x, y, typeID, speedTurnsLeft, abilityCooldown)
            seenEnnemies.append(ID)

        # No pelet here as a pac is on it. No matter the pac
        unknownPellets.pop((x, y), None)
        knownPellets.pop((x, y), None)
        emptyPellets[(x, y)] = 1
    start = False

    # Update non visible ennemies count
    for ID, ePac in ennemies.items():
        if not ID in seenEnnemies:
            ePac.count += 1

    visible_pellet_count = int(input())
    debug(visible_pellet_count, "visible pellets")

    superPellet = {}
    visiblePellet = {}
    for i in range(visible_pellet_count):
        x, y, v = map(int, input().split())
        if v == 1:
            visiblePellet[(x, y)] = v
            knownPellets[(x, y)] = v
            if grid[y][x] == 'X':
                updateVisibleInDeadEnd(x, y, knownPellets, unknownPellets, emptyPellets, grid, width, height)
        else:
            superPellet[(x, y)] = v
        unknownPellets.pop((x, y), None)
    removeUnseenPellets(knownPellets, unknownPellets, emptyPellets, visiblePellet, pacman, grid, width, height)




    # A super pellet is missing ! Find the Ennemie who did it!
    # Not implemented yet
    if False and 0 < len(pSuperPellets) and len(pSuperPellets) > len(superPellet):
        for (x, y), v in pSuperPellets.items():
            if not (x, y) in superPellet:
                isMyPacOnIt = False
                for ID, pac in pacman.items():
                    if pac.activated:
                        if (x, y) == (pax.X, pac.Y):
                            isMyPacOnIt = True
                            break
                if isMyPacOnIt:
                    continue
                
                isVisiblePacOnIt = False
                for ID in seenEnnemies:
                    if (x, y) == (ennemies[ID].X, ennemies[ID].Y):
                        isVisiblePacOnIt = True
                        break
                if isVisiblePacOnIt:
                    continue
                
                findAndUpdateEnnPacSuperPelet((x, y), ennemies, grid, width, height)
                



    debug(len(knownPellets), "known pellets:\n", knownPellets)
    debug(len(unknownPellets), "unknown pellets")
    debug(len(superPellet), "super pellets:\n", superPellet, "\n")
    debugGrid(grid, knownPellets, unknownPellets)

    killed = {}
    if len(seenEnnemies) == 0:
        for ID, pac in pacman.items():
            if pac.activated and pac.available:
                pac.triggerSpeed()
    else:
        # Trigger Speed if no enenmy seen
        for ID, pac in pacman.items():
            if pac.activated and pac.available:
                x, y = pac.X, pac.Y
                eSeen = False
                for eID in seenEnnemies:
                    eX, eY, eType, eSpeed, eAbility = ennemies[eID].getInfo()
                    if (x == eX and abs(y - eY) < 3) or (y == eY and abs(x - eX) < 3):
                        eSeen = True
                        break
                if not eSeen:
                    pac.triggerSpeed()

        # Try to kill
        for ID, pac in pacman.items():
            if pac.activated and pac.available:
                x, y = pac.X, pac.Y
                for eID in seenEnnemies:
                    eX, eY, eType, eSpeed, eAbility = ennemies[eID].getInfo()

                    deadEndCase = False
                    if grid[eY][eX] == '#':
                        a =0

                    if deadEndCase:
                        continue

                    d = abs(x - eX) + abs(y - eY)
                    if y == eY and d >= width - 2: d = width - d

                    if d < 2 or (d == 2 and eSpeed > 0):
                        debug(ID, "and", (eX, eY), "are FUCKING close")

                        if RPS_win_loose[pac.type] == eType and eAbility > 0:
                            if (eX, eY) in killed:
                                continue
                            eX, eY = killHim(pac, eX, eY, knownPellets, grid, width, height)
                            pac.setTarget(eX, eY, 5)
                            killed[(eX, eY)] = True
                            debug("Kill him")
                            break

                        elif RPS_loose_win[eType] != pac.type and pac.cooldown == 0:
                            pac.nextType = RPS_loose_win[eType]
                            pac.available = False
                            debug("Eheh, I'm smart")
                            break

                        elif x == eX or y == eY:
                            if d == 2:
                                eX = (x + eX) // 2
                                eY = (y + eY) // 2
                            debug("Avoiding", (eX, eY), "from", (x, y))
                            (tX, tY), isPellet = safeMove((x, y), (eX, eY), knownPellets, superPellet, grid, width, height)
                            if pac.speedLeft > 0:
                                debug("Avoiding", (x, y), "from", (tX, tY))
                                (fX, fY), _ = safeMove((tX, tY), (x, y), knownPellets, superPellet, grid, width, height)
                                pac.setTarget(fX, fY, 5)
                                superPellet.pop((fX, fY), None)
                            else:
                                pac.setTarget(tX, tY, 5)
                            debug("Cowardness power!")
                            if isPellet:
                                knownPellets.pop((tX, tY), None)
                                superPellet.pop((tX, tY), None)
                            break

                    elif d < 5 and eSpeed > 0 and pac.speedLeft > 0 and RPS_win_loose[eType] == pac.type:
                        debug(ID, "and", (eX, eY), "are 'FAR' but both 'FAST' closed")
                        eX = (eX + x) // 2
                        eY = (eY + y) // 2
                        (tX, tY), isPellet = safeMove((x, y), (eX, eY), knownPellets, superPellet, grid, width, height)
                        pac.setTarget(tX, tY, 5)
                        debug("Super cowardness power!")
                        if isPellet:
                            knownPellets.pop((tX, tY), None)
                            superPellet.pop((tX, tY), None)
                        break

                    elif d == 2 and (eSpeed == 0 or (eSpeed > 0 and eSpeed < pac.speedLeft)):
                        debug(ID, "and", (eX, eY), "are 'OK' closed")
                        if RPS_win_loose[pac.type] == eType:
                            if (eX, eY) in killed:
                                continue
                            pac.setTarget(eX, eY, 5)
                            debug("Chasing!")
                            killed[(eX, eY)] = True
                            break
                        elif RPS_win_loose[eType] == pac.type:
                            eX = (eX + x) // 2
                            eY = (eY + y) // 2
                            (tX, tY), isPellet = safeMove((x, y), (eX, eY), knownPellets, superPellet, grid, width, height)
                            pac.setTarget(tX, tY, 5)
                            debug("Cowardness power!")
                            if isPellet:
                                knownPellets.pop((tX, tY), None)
                                superPellet.pop((tX, tY), None)
                            break

    # Try to look for superPellets shortest path in 30 moves
    markedPellets = {}
    # Do it twice
    for _ in [0] * 2:
        for ID, pac in pacman.items():
            if pac.activated and pac.available and len(superPellet) > 0:
                patterns = findClosestPellet(pacman, ID, seenEnnemies, ennemies, RPS_win_loose, superPellet, markedPellets, grid, width, height)
                coords, dist, firstOnIt = bestChoices((pac.X, pac.Y), patterns, knownPellets, superPellet, grid, width, height)
                
                if len(coords) > 0:
                    tX, tY = coords[-1]
                    pac.setTarget(tX, tY, 1)
                    if not firstOnIt:
                        _, otherID = markedPellets[(tX, tY)]
                        pacman[otherID].available = True
                        debug("Remove", otherID, "from", (tX,tY), "and put", ID, "on it")
                    else:
                        debug(ID, "first PAC on SUPER PELLETS", (tX,tY))
                    for (tX, tY) in coords:
                        markedPellets[(tX, tY)] = dist, ID


    markedPellets = {}
    # Do it twice
    for _ in [0] * 2:
        for ID, pac in pacman.items():
            if pac.activated and pac.available and len(knownPellets) - len(markedPellets) > 0:
                patterns = findClosestPellet(pacman, ID, seenEnnemies, ennemies, RPS_win_loose, knownPellets, markedPellets, grid, width, height)
                coords, dist, firstOnIt = bestChoices((pac.X, pac.Y), patterns, knownPellets, {}, grid, width, height)
                
                if len(coords) > 0:
                    tX, tY = coords[-1]
                    pac.setTarget(tX, tY, 1)
                    if not firstOnIt:
                        _, otherID = markedPellets[(tX, tY)]
                        pacman[otherID].available = True
                        debug("Remove", otherID, "from", (tX,tY), "and put", ID, "on it")
                    else:
                        debug(ID, "first PAC on PELLET", (tX,tY))
                    for (tX, tY) in coords:
                        markedPellets[(tX, tY)] = dist, ID


    # For unknown pellets
    for ID, pac in pacman.items():
        if pac.activated and pac.available and len(unknownPellets) > 0:
            patterns = findClosestPellet(pacman, ID, seenEnnemies, ennemies, RPS_win_loose, unknownPellets, {}, grid, width, height)
            coords, _, _ = bestChoices((pac.X, pac.Y), patterns, knownPellets, superPellet, grid, width, height)
            
            if len(coords) > 0:
                tX, tY = coords[-1]
                pac.setTarget(tX, tY, 1)

    action = '|'.join([pac.generateAction(ID) for ID, pac in pacman.items()])

    debug(current_ms_time() - start_time, "ms")
    
    print(action)

    pSuperPellets = superPellet
