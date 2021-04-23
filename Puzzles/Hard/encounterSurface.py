import sys
import math
import numpy as np
from scipy.spatial import ConvexHull

# ################################################# #
# #####                 UTILS                 ##### #
# ################################################# #
epsilon = 1E-9

def value(x):
    """Returns 0 if x is 'sufficiently close' to zero, +/- 1E-9"""
    if x >= 0 and x <= epsilon:
        return 0
    if x < 0 and -x <= epsilon:
        return 0
    return x

def intersect(x1, y1, x2, y2, x3, y3, x4, y4):
    """
    Return the point of intersection (or None) between edges:
      e1: (x1,y1) - (x2,y2)
      e2: (x3,y3) - (x4,y4)
    Might include end-points.
    """
    # common denominator
    da = (y4 - y3)*(x2 - x1)
    db = (x4 - x3)*(y2 - y1)
    denom = da - db
                
    if value(denom) == 0:
        return None    # PARALLEL OR COINCIDENT
                
    # numerators
    ux = (x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)
    uy = (x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)
                
    ux = ux / denom
    uy = uy / denom
                
    # line segment intersections are between 0 and 1. Both must be true
    # Special care on both boundaries w/ floating point issues.
    if value(ux) >= 0 and value(ux-1) <= 0 and value(uy) >= 0 and value(uy-1) <= 0:
        ix = x1 + ux * (x2-x1)
        iy = y1 + ux * (y2-y1)
        return (ix, iy)
        
    return None     # no intersection

def computeAngleSign(x1, y1, x2, y2, x3, y3):
    """
    Determine if angle (p1,p2,p3) is right or left turn by computing
    3x3 determinant. If sign is + if p1-p2-p3 forms counterclockwise
    triangle. So if positive, then left turn. If zero then colinear.
    If negative, then right turn.
    """
    val1 = (x2 - x1)*(y3 - y1)
    val2 = (y2 - y1)*(x3 - x1)
    diff = value(val1 - val2)
    if diff > 0:
        return +1
    elif diff < 0:
        return -1
    else:
        return 0

def inhalfplane(pt, q):
    """Return True if point pt is in half-plane defined by q."""
    signTail = computeAngleSign(pt.x(), pt.y(),
                                q.head().x(), q.head().y(),
                                q.tail().x(), q.tail().y())
    return signTail >= 0

def dist(p, q):
    """Compute Euclidean distance between two points."""
    return math.sqrt((p.x()-q.x())**2 + (p.y()-q.y())**2)

def aim (p, q):
    """Return true if p is "aiming towards" q's half-plane edge."""
    # First check if p.tail is in the half-plane of q
    inside = inhalfplane(p.tail(), q)

    # compute cross product of q x p to determine orientation
    # en.wikipedia.org/wiki/Cross_product#Computational_geometry
    # normalize p and q
    pnorm = Point(p.tail().x() - p.head().x(), 
                  p.tail().y() - p.head().y())
    qnorm = Point(q.tail().x() - q.head().x(), 
                  q.tail().y() - q.head().y())

    cross = qnorm.x()*pnorm.y() - qnorm.y()*pnorm.x()
    if inside:
        # in half-plane, so now check orientation
        return cross < 0
    else:
        # not in half-plane.
        return cross >= 0

def containedWithin(pt, p):
    """
    Determine if pt is fully contained within p. Do so by 
    summing angles with each edge in the convex polygon p.
    """
    sum = 0
    for e in p.edges():
        C = dist(e.head(), e.tail())
        A = dist(pt, e.head())
        B = dist(pt, e.tail())
        sum += math.degrees(math.acos((A*A+B*B-C*C)/(2*A*B)))
    return value(sum-360) == 0

def convexIntersect(p, q):
    """
    Compute and return polygon resulting from the intersection of
    two convext polygons, p and q.
    """
    intersection = Polygon()
    pn = p.numEdges()
    qn = q.numEdges()
    k = 1
    inside = None              # can't know inside until intersection
    first = None               # remember 1st intersection to know when to stop
    firstp = pe = p.edges()[0] # get first edge of p and q
    firstq = qe = q.edges()[0]
    while k < 2 * (pn + qn):
        pt = pe.intersect(qe)
        if pt is not None:
            if first == None:
                first = pt
            elif pt == first:
                # stop when find first intersection again
                break

            intersection.add(pt.x(), pt.y())
            if inhalfplane(pe.tail(), qe):
                inside = p
            else:
                inside = q

        # Identify relationship between edges; either we advance
        # p or we advance q, based on whether p's current edge
        # is aiming at qe (or vice versa).
        advancep = advanceq = False

        if (aim(pe,qe) and aim(qe,pe)) or (not aim(pe,qe) and not aim(qe,pe)):
            if inside is p:
                advanceq = True
            elif inside is q:
                advancep = True
            else:
                # no intersection yet. Choose based on
                # which one is "outside"
                if inhalfplane(pe.tail(), qe):
                    advanceq = True
                else:
                    advancep = True
        elif aim(pe, qe):
            advancep = True
        elif aim(qe, pe):
            advanceq = True

        if advancep:
            if inside is p:
                intersection.add(pe.tail().x(), pe.tail().y())
            pe = pe.next()
        elif advanceq:
            if inside is q:
                intersection.add(qe.tail().x(), qe.tail().y())
            qe = qe.next()

        k += 1
            
    if intersection.numPoints() == 0:
        if containedWithin(firstp.tail(), q):
            return p
        elif containedWithin(firstq.tail(), p):
            return q
        else:
            return None

    # Return computed intersection
    return intersection

# ################################################# #
# #####                 POINT                 ##### #
# ################################################# #

class Point:
    """Represents a point in Cartesian space."""

    def __init__(self, x, y):
        """Creates a point (x,y) in Cartesian space."""
        self._x = x
        self._y = y

    def copy(self):
        """Return copy of a point."""
        return Point(self._x, self._y)

    def x(self):
        """Return x value of point."""
        return self._x

    def y(self):
        """Return y value of point."""
        return self._y

# ################################################ #
# #####                 EDGE                 ##### #
# ################################################ #

class Edge:
    """Represents an edge in Cartesian space."""

    def __init__(self, head, tail):
        """
        Creates an edge for consecutive points head and tail.
        It is assumed that head != tail
        """
        if head == tail:
            raise ValueError("Can't create edge from two identical points")
        self._head = head
        self._tail = tail
        self._next = None

    def head(self):
        """Return head value of edge."""
        return self._head

    def tail(self):
        """Return tail value of edge."""
        return self._tail

    def next(self):
        """Return next edge in polygon."""
        return self._next
        
    def setNext(self, e):
        """Make 'e' the next edge in polygon after self."""
        self._next = e

    def intersect(self, e):
        """Return intersection between two edges (aside from end-points)."""
        if self.head() == e.head() or self.head() == e.tail():
            return None
        if self.tail() == e.head() or self.tail() == e.tail():
            return None

        # compute intersection of two line segments using x,y coords
        pt = intersect(self.head().x(),
                       self.head().y(),
                       self.tail().x(),
                       self.tail().y(),
                       e.head().x(),
                       e.head().y(),
                       e.tail().x(),
                       e.tail().y())
        if pt is None:
            return None
        return Point (pt[0], pt[1])

# ################################################# #
# #####                POLYGON                ##### #
# ################################################# #

class Polygon:
    """Represents polygon of points in Cartesian space."""

    def __init__(self, pts=[]):
        """
        Creates polygon from list of points. If omitted, polygon is empty.
        """
        self.points = []
        for pt in pts:
            self.points.append(pt.copy())

    def valid(self):
        """A polygon becomes valid with three or more points."""
        return len(self.points) >= 3

    def numEdges(self):
        """Return the number of edges in polygon."""
        if len(self.points) < 1:
            return 0
        elif len(self.points) == 2:
            return 1
        else:
            return len(self.points)

    def edges(self):
        """Return edges in the polygon, in order."""
        order = []
        for i in range(0, len(self.points)-1):
            order.append(Edge(self.points[i], self.points[i+1]))

        if self.valid():
            n = len(self.points)
            order.append(Edge(self.points[n-1], self.points[0]))

        # Now link edges to next one in the chain. Make sure to
        # link back to start
        for i in range(len(order)-1):
            order[i].setNext(order[i+1])
        order[-1].setNext(order[0])
        return order

    def numPoints(self):
        """Return the number of points in polygon."""
        return len(self.points)

    def add(self, x, y):
        """Extend polygon with additional (x,y) point."""
        self.points.append(Point(x,y))
        n = len(self.points)

    def make_convex(self):
        pts = []
        for pt in self.points:
            pts.append((pt.x(), pt.y()))
        pts = np.array(pts)
        pts = pts[ConvexHull(pts).vertices]
        print(*pts, file=sys.stderr)

        self.points = [Point(x, y) for (x,y) in pts]

    def area(self):  
        """Return the area of the polygon."""
        area = 0
        q = self.points[-1]
        for p in self.points:
            area += p.x() * q.y() - p.y() * q.x()
            q = p
        return area / 2

# ################################################ #
# #####                 MAIN                 ##### #
# ################################################ #

if __name__ == '__main__':
    n = int(input())
    m = int(input())

    p = []
    for i in range(n):
        x, y = map(int, input().split())
        p.append((x, y))
    p = np.array(p)
    p = p[ConvexHull(p).vertices]
    print("Pol1:", *p, file=sys.stderr)

    q = []
    for i in range(m):
        x, y = map(int, input().split())
        q.append((x, y))
    q = np.array(q)
    q = q[ConvexHull(q).vertices]
    print("Pol2:", *q, file=sys.stderr)

    inter = convexIntersect(Polygon([Point(x, y) for (x,y) in p]),
                            Polygon([Point(x, y) for (x,y) in q]))

    if inter:
        inter.make_convex()
        area_f = abs(inter.area())
        print(area_f, file=sys.stderr)

        area_i = int(area_f)
        print(area_i + (area_i != area_f))
    else:
        print(0)