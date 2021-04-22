import sys
import math

_AOE = 4
_SAFETY = 1
_w = 40
_h = 18

class Thor:
    x = y = 0
    strikes_left = 0
    giants_left = 0
    giants = []
    action = "STRIKE"

    def __init__(self):
        self.x, self.y = map(int, input().split())
    
    def nextTurn(self):
        self.strikes_left, self.giants_left = map(int, input().split())
        self.giants.clear()
        for i in range(self.giants_left):
            x, y = map(int, input().split())
            self.giants += [[x, y]]

    def think(self):
        nbKillable = nbFckingClose = cX = cY = 0
        for x, y in self.giants:
            cX += x
            cY += y

            dX = abs(x - self.x)
            dY = abs(y - self.y)

            nbKillable += dX <= _AOE and dY <= _AOE
            nbFckingClose += dX <= _SAFETY and dY <= _SAFETY

        # Kill them all and finish the game
        if nbKillable == self.giants_left:
            print("END", file=sys.stderr)
            self.action = "STRIKE"
            return

        cX = int(cX / self.giants_left)
        cY = int(cY / self.giants_left)

        # Go in the middle of the giants
        if nbFckingClose == 0:
            print("Easy walk", file=sys.stderr)
            self.goto(cX, cY)
            return

        # Survive
    
        potentialMove = []
        

        dx, dy = -1, -1
        isValid, nbKillable = self.tryMove(dx, dy)
        if isValid:
            potentialMove += [("NW", nbKillable, (self.x+dx, self.y+dy))]
        dx, dy = -1, 1
        isValid, nbKillable = self.tryMove(dx, dy)
        if isValid:
            potentialMove += [("SW", nbKillable, (self.x+dx, self.y+dy))]
        dx, dy = -1, 0
        isValid, nbKillable = self.tryMove(dx, dy)
        if isValid:
            potentialMove += [("W", nbKillable, (self.x+dx, self.y+dy))]


        dx, dy = 1, -1
        isValid, nbKillable = self.tryMove(dx, dy)
        if isValid:
            potentialMove += [("NE", nbKillable, (self.x+dx, self.y+dy))]
        dx, dy = 1, 1
        isValid, nbKillable = self.tryMove(dx, dy)
        if isValid:
            potentialMove += [("SE", nbKillable, (self.x+dx, self.y+dy))]
        dx, dy = 1, 0
        isValid, nbKillable = self.tryMove(dx, dy)
        if isValid:
            potentialMove += [("E", nbKillable, (self.x+dx, self.y+dy))]


        dx, dy = 0, -1
        isValid, nbKillable = self.tryMove(dx, dy)
        if isValid:
            potentialMove += [("N", nbKillable, (self.x+dx, self.y+dy))]
        dx, dy = 0, 1
        isValid, nbKillable = self.tryMove(dx, dy)
        if isValid:
            potentialMove += [("S", nbKillable, (self.x+dx, self.y+dy))]


        print(potentialMove, file=sys.stderr)


        if len(potentialMove) == 0:
            self.action = "STRIKE"
        else:
            bestMove = ["", -1, (-1, -1)]
            furthest_g = -1
            furthest_m = -1
            for potential in potentialMove:
                print(potential, file=sys.stderr)
                dist_g = int(abs(potential[2][0] - cX) + abs(potential[2][1] - cY))
                dist_m = int(abs(potential[2][0] - _w/2) + abs(potential[2][1] - _h/2))
                if potential[1] > bestMove[1] or \
                   (potential[1] == bestMove[0] and dist_g > furthest_g or \
                                                (dist_g == furthest_g and dist_m < furthest_m)):
                    bestMove = potential
                    furthest_g = dist_g
                    furthest_m = dist_m

            self.action = bestMove[0]
            self.x = bestMove[2][0]
            self.y = bestMove[2][1]

    def tryMove(self, dx, dy):
        x = self.x + dx
        y = self.y + dy
        if y < 0 or y > _h or x < 0 or x > _w:
            return False, 0

        nbKillable = nbFckingClose = 0
        for gx, gy in self.giants:
            dX = abs(gx - x)
            dY = abs(gy - y)

            nbKillable += dX <= _AOE and dY <= _AOE
            nbFckingClose += dX <= _SAFETY and dY <= _SAFETY

        return nbFckingClose == 0, nbKillable

    def goto(self, x, y):
        if x < self.x:
            if y < self.y:
                self.x -= 1
                self.y -= 1
                self.action = "NW"
            elif y > self.y:
                self.x -= 1
                self.y += 1
                self.action = "SW"
            else:
                self.x -= 1
                self.action = "W"
        elif x > self.x:
            if y < self.y:
                self.x += 1
                self.y -= 1
                self.action = "NE"
            elif y > self.y:
                self.x += 1
                self.y += 1
                self.action = "SE"
            else:
                self.x += 1
                self.action = "E"
        else:
            if y < self.y:
                self.y -= 1
                self.action = "N"
            elif y > self.y:
                self.y += 1
                self.action = "S"
            else:
                self.action = "WAIT"

    def act(self):
        print(self.x, self.y, file=sys.stderr)
        print(self.action)

if __name__ == '__main__':
    thor = Thor()

    while True:
        thor.nextTurn()

        thor.think()

        thor.act()