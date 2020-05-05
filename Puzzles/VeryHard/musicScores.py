import numpy as np


def decodeDWE(w, h, code):
    grid = np.zeros((h, w), dtype=np.uint8)

    x = 0
    y = 0
    for c in range(0, len(code), 2):
        value = 255 if code[c] == 'W' else 0

        for _ in range(int(code[c + 1])):

            grid[y][x] = value
            x += 1

            if x == w:
                x = 0
                y += 1
    return grid


def findLinesIdx(w, h, image):
    lines = []
    start = 0
    i = 0
    while len(lines) == 0:
        for j in range(1, h):
            if image[j][i] == 255:
                if image[j - 1][i] == 0:
                    lines.append((start, j - 1))
            else:
                if image[j - 1][i] == 255:
                    start = j
        i += 1

    startLastLine = lines[-1][1] + lines[1][0] - lines[0][1]
    lines.append(
            (startLastLine,
             startLastLine + lines[0][1] - lines[0][0]))

    return lines, i


def isInLine(j, lines):
    for start, end in lines:
        if start <= j <= end:
            return True
    if j > end:
        return False


def jsOfNote(i, j, image):
    start = end = j
    if image[j][i] == 255:
        while image[start][i] == 255:
            start -= 1
        while image[end][i] == 255:
            end +=1

    while image[start][i] == 0:
        start -= 1
    start += 1
    while image[end][i] == 0:
        end += 1
    end -= 1

    return start, end


def colorNote(Is, Js, image):
    midJ = (Js[1] + Js[0]) // 2
    midI = (Is[1] + Is[0]) // 2

    if image[midJ][midI] == 255:
        return "WHITE"
    else:
        return "BLACK"


def endOfNote(i, j, w, image, sizeMin):
    size = 0
    while image[j][i] == 0:
        i += 1
        size += 1
    if size > sizeMin:
        return i, size // 2

    while image[j][i] == 255:
        i += 1
        size += 1
    while image[j][i] == 0:
        i += 1
        size += 1
    return i, size // 2


def isNote(i, w, h, lines, image, sizeMin, sizeLine):
    for j in range(h):
        if image[j][i] == 0 and not isInLine(j, lines):
            end, sizeMid = endOfNote(i, j, w, image, sizeMin)
            js = jsOfNote(i + sizeMid, j, image)
            if js[1] - js[0] < sizeLine - 5:
                break
            return True, end, js, colorNote((i, end), js, image)
    return False, i + 1, (-1, -1), "NOPE"


def noteFromJ(js, lines):
    s, e = js
    if s < lines[0][0] and e <= lines[0][1]:
        return "G"

    if s < lines[0][0] and e > lines[0][1]:
        return "F"

    if s >= lines[0][0] and e <= lines[1][1]:
        return "E"

    if s < lines[1][0] and e > lines[1][1]:
        return "D"

    if s >= lines[1][0] and e <= lines[2][1]:
        return "C"

    if s < lines[2][0] and e > lines[2][1]:
        return "B"

    if s >= lines[2][0] and e <= lines[3][1]:
        return "A"

    if s < lines[3][0] and e > lines[3][1]:
        return "G"

    if s >= lines[3][0] and e <= lines[4][1]:
        return "F"

    if s < lines[4][0] and e > lines[4][1]:
        return "E"

    if s >= lines[4][0] and e <= lines[5][1]:
        return "D"

    if s < lines[5][0] and e > lines[5][1]:
        return "C"

    return "X"


w, h = [int(i) for i in input().split()]
image = input().split()

grid = decodeDWE(w, h, image)

lines, start = findLinesIdx(w, h, grid)
sizeMin = lines[0][1] - lines[0][0] + 4
sizeLine = lines[1][0] - lines[0][1]

start += 1
res = ""
while start < w:
    ret, start, js, color = isNote(start, w, h, lines, grid, sizeMin, sizeLine)
    if ret:
        res += " " + noteFromJ(js, lines)
        res += "H" if color == "WHITE" else "Q"

print(res[1:])