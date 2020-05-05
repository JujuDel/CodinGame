# Length Of Interest
class LOI:
    def __init__(self, mini, maxi):
        self.mini = mini
        self.maxi = maxi

    def updateROI(self, roi):
        self.mini = roi.mini
        self.maxi = roi.maxi

    def updateXY(self, mini, maxi):
        self.mini = mini
        self.maxi = maxi


def final():
    global width, height
    global x, y
    global isXfound, isFirstTry
    global current, warm, cold

    tmpX, tmpY = x, y
    if isXfound:
        y = int((current.maxi + current.mini) / 2)
    else:
        x = int((current.maxi + current.mini) / 2)
        isXfound = True
        current.updateXY(0, height - 1)
        warm.updateROI(current)
        cold.updateROI(current)

    isFirstTry = True
    return (x == tmpX and y == tmpY)


def get(v, lim):
    global width, height
    global x, y
    global current, warm, cold
    global isOutside

    mini = current.mini
    maxi = current.maxi

    give = mini + maxi - v

    if isOutside:
        if v == 0:
            give = int(give / 2)
        elif v == lim:
            give = int((give + lim) / 2)
    isOutside = False

    if give == v: give = v + 1

    if give < 0:
        give = 0
        isOutside = True
    elif give > lim:
        give = lim
        isOutside = True

    lower  = int((give + v - 1) / 2)
    higher = int((give + v + 1) / 2)

    if give > v:
        warm.updateXY(higher, maxi)
        cold.updateXY(mini, lower)
    else:
        warm.updateXY(mini, lower)
        cold.updateXY(higher, maxi)

    return give


def doIt():
    global width, height
    global lastX, lastY, x, y
    global bomb_dir
    global current, warm, cold
    global isXfound, isFirstTry

    tmpX, tmpY = x, y

    if bomb_dir == 'WARMER':
        current.updateROI(warm)
    if bomb_dir == 'COLDER':
        current.updateROI(cold)
    if bomb_dir == 'SAME':
        if not isFirstTry:
            if not final(): return

    if current.mini >= current.maxi:
        if not final(): return

    isFirstTry = False

    if isXfound:
        y = get(y, height - 1)
    else:
        x = get(x, width - 1)
    lastX = tmpX
    lastY = tmpY


width, height = map(int, input().split())
current = LOI(0, width - 1)
warm    = LOI(0, width - 1)
cold    = LOI(0, width - 1)

nb_round = int(input())

x, y = map(int, input().split())
lastX = lastY = 0

isFirstTry = True
isXfound   = False
isOutside  = False


# game loop
while True:
    bomb_dir = input() # COLDER, WARMER, SAME or UNKNOWN

    doIt()

    print(x, y)
