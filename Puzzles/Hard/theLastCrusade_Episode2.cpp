#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <sstream>

using namespace std;

struct Coord {
    int x;
    int y;

    bool operator==(const Coord &c) const
	{
		return x == c.x && y == c.y;
	}
};

enum DIRECTION {
    TOP = 0,
    RIGHT = 1,
    BOTTOM = 2,
    LEFT = 3,
    NONE = 4
};

struct Node {
    Coord coord;
    const DIRECTION from;
    int score;
    DIRECTION rotation;
    Node *parent;
    int nbMoveBefore;

    Node(const Coord& f_coord, const DIRECTION f_from, const int f_score, const DIRECTION f_rotation, Node *f_parent) :
        coord{ f_coord },
        from{ f_from },
        score{ f_score },
        rotation(f_rotation),
        parent{ f_parent },
        nbMoveBefore{ 0 }
    {}
};

DIRECTION strtodir(const string& s) {
    if (s == "BOTTOM") return BOTTOM;
    if (s == "TOP") return TOP;
    if (s == "LEFT") return LEFT;
    if (s == "RIGHT") return RIGHT;
    return NONE;
}
string dirtostr(const DIRECTION& d) {
    if (d == BOTTOM) return "BOTTOM";
    if (d == TOP) return "TOP";
    if (d == LEFT) return "LEFT";
    if (d == RIGHT) return "RIGHT";
    return "NONE";
}

// Order of the enum: TOP, RIGHT, BOTTOM, LEFT, NONE
static const DIRECTION oppositeDir[]{ BOTTOM, LEFT, TOP, RIGHT };
static const int directionX[]{ 0, 1, 0, -1 };
static const int directionY[]{ -1, 0, 1, 0 };

int nextTypeValue(const int type, const int rotation) {
    if (type == 2 || type == 3) {
        if (type + rotation == 1) {
            return 3;
        }
        if (type + rotation == 4) {
            return 2;
        }
        return type + rotation;
    }
    if (type == 4 || type == 5) {
        if (type + rotation == 3) {
            return 5;
        }
        if (type + rotation == 6) {
            return 4;
        }
        return type + rotation;
    }

    if (type >= 6 && type <= 9) {
        if (type + rotation == 5) {
            return 9;
        }
        if (type + rotation == 10) {
            return 6;
        }
        return type + rotation;
    }

    if (type >= 10 && type <= 13) {
        if (type + rotation == 9) {
            return 13;
        }
        if (type + rotation == 14) {
            return 10;
        }
        return type + rotation;
    }

    return type;
}

void setIdsInRoomsWithRotation(vector<pair<int, int>>& idsInRooms) {
    idsInRooms.push_back({0, 0});
    idsInRooms.push_back({1, 0});
    idsInRooms.push_back({2, 0});
    idsInRooms.push_back({2, 1});
    idsInRooms.push_back({3, 0});
    idsInRooms.push_back({3, 1});
    idsInRooms.push_back({4, 0});
    idsInRooms.push_back({4, 1});
    idsInRooms.push_back({4, 2});
    idsInRooms.push_back({4, 3});
    idsInRooms.push_back({5, 0});
    idsInRooms.push_back({5, 1});
    idsInRooms.push_back({5, 2});
    idsInRooms.push_back({5, 3});
}

void setRoomsWithRotation(vector<vector<unordered_map<DIRECTION, DIRECTION>>>& roomsWithRotation) {
    // Room 0 - Type 0
    roomsWithRotation.push_back({ {} });

    // Room 1 - Type 1
    roomsWithRotation.push_back({
        { {TOP, BOTTOM},
          {LEFT, BOTTOM},
          {RIGHT, BOTTOM}
        }
    });

    // Room 2 - Type 2 + 3
    roomsWithRotation.push_back({
        { {LEFT, RIGHT},
          {RIGHT, LEFT}
        },
        { {TOP, BOTTOM}
        }
    });

    // Room 3 - Type 4 + 5
    roomsWithRotation.push_back({
        { {TOP, LEFT},
          {RIGHT, BOTTOM},
          {LEFT, NONE}
        },
        { {TOP, RIGHT},
          {LEFT, BOTTOM},
          {RIGHT, NONE}
        }
    });

    // Room 4 - Type 6 + 7 + 8 + 9
    roomsWithRotation.push_back({
        { {LEFT, RIGHT},
          {RIGHT, LEFT},
          {TOP, NONE}
        },
        { {TOP, BOTTOM},
          {RIGHT, BOTTOM}
        },
        { {LEFT, BOTTOM},
          {RIGHT, BOTTOM}
        },
        { {TOP, BOTTOM},
          {LEFT, BOTTOM}
        }
    });
    
    // Room 5 - Type 10 + 11 + 12 + 13
    roomsWithRotation.push_back({
        { {TOP, LEFT},
          {LEFT, NONE}
        },
        { {TOP, RIGHT},
          {RIGHT, NONE}
        },
        { {RIGHT, BOTTOM}
        },
        { {LEFT, BOTTOM}
        }
    });
}

void moveRocks(const vector<vector<int>>& grid,
               const vector<vector<unordered_map<DIRECTION, DIRECTION>>>& roomsWithRotation,
               const vector<pair<int, int>>& idsInRooms,
               vector<vector<pair<Coord, DIRECTION>>>& rocks)
{
    vector<pair<Coord, DIRECTION>> newRound;
    for (const pair<Coord, DIRECTION>& rock : rocks[rocks.size() - 1]) {
        if (rock.second == NONE) {
            newRound.push_back(rock);
        }
        else {
            const pair<int, int> p = idsInRooms[abs(grid[rock.first.y][rock.first.x])];
            unordered_map<DIRECTION, DIRECTION> room = roomsWithRotation[p.first][p.second];
            if (room.find(rock.second) != room.end() && room.at(rock.second) != NONE) {
                const DIRECTION to = room.at(rock.second);
                const Coord nextCoor{ rock.first.x + directionX[to], rock.first.y + directionY[to] };
                
                const pair<int, int> nextp = idsInRooms[abs(grid[nextCoor.y][nextCoor.x])];
                unordered_map<DIRECTION, DIRECTION> nextroom = roomsWithRotation[nextp.first][nextp.second];
                if (nextroom.find(oppositeDir[to]) != nextroom.end())
                    newRound.push_back({ nextCoor, oppositeDir[to] });
                else
                    newRound.push_back({ nextCoor, NONE });
            }
            else {
                newRound.push_back({ rock.first, NONE });
            }
        }
    }
    for (int i = 0; i < newRound.size(); ++i) {
        if (newRound[i].second == NONE) continue;
        for (int j = i + 1; j < newRound.size(); ++j) {
            if (newRound[j].second == NONE) continue;

            if (newRound[i].first == newRound[j].first) {
                newRound[i].second = NONE;
                newRound[j].second = NONE;
            }
        }
    }
    rocks.push_back(newRound);
}

void checkRocks(const vector<vector<int>>& grid,
                const vector<vector<unordered_map<DIRECTION, DIRECTION>>>& roomsWithRotation,
                const vector<pair<int, int>>& idsInRooms,
                vector<vector<pair<Coord, DIRECTION>>>& rocksRound,
                Node* n)
{
    int round = rocksRound.size() - 1;
    // For each rock position at this round
    for (int idRock = 0; idRock < rocksRound[round].size(); ++idRock) {
        if (rocksRound[round][idRock].second == NONE)
            continue;

        // If the player is on the same position as a rock
        // Be carefull with Type 4 and Type 5 (i.e. room 3)
        if (n->coord == rocksRound[round][idRock].first && idsInRooms[abs(grid[n->coord.y][n->coord.x])].first != 3) {
            cerr << "KILLING DETECTED" << endl;
            cerr << "  " << n->coord.x << " " << n->coord.y << endl;

            Node* node = n;
            for (int i = 0; node && i <= round; ++i, node=node->parent) {
                pair<Coord, DIRECTION> rock = rocksRound[round - i][idRock];
                if (grid[rock.first.y][rock.first.x] <= 1)
                    continue;

                const pair<int, int> p = idsInRooms[abs(grid[rock.first.y][rock.first.x])];
                const int nbRotationRoom = roomsWithRotation[p.first].size();
                cerr << "      Trying with Indy at " << node->coord.x << " " << node->coord.y << endl;
                // Find the smallest killing rotation, if any
                int left = (p.second - 1 + nbRotationRoom) % nbRotationRoom;
                int right = (p.second + 1) % nbRotationRoom;
                int penalty = 1;
                while (left != right) {
                    // 1 more rotation on the left
                    unordered_map<DIRECTION, DIRECTION> roomLeft = roomsWithRotation[p.first][left];
                    if (roomLeft.find(rock.second) == roomLeft.end() || (roomLeft.find(rock.second) != roomLeft.end() && roomLeft.at(rock.second) == NONE)) {
                        penalty *= -1;
                        break;
                    }
                    if (left == right)
                        break;

                    // 1 more rotation on the right
                    unordered_map<DIRECTION, DIRECTION> roomRight = roomsWithRotation[p.first][right];
                    if (roomRight.find(rock.second) == roomRight.end() || (roomRight.find(rock.second) != roomRight.end() && roomRight.at(rock.second) == NONE)) {
                        break;
                    }

                    // Update left, right and penalty
                    left = (left - 1 + nbRotationRoom) % nbRotationRoom;
                    if (left == right)
                        break;
                    right = (right + 1) % nbRotationRoom;
                    penalty++;
                }

                cerr << "      Found a rotation " << penalty << endl;
                // A killing move to the rock has been found
                if (!node->parent) {
                    node->parent = new Node({0,0}, NONE, 1, NONE, nullptr);
                    cerr << "      Fake parent" << endl;
                }
                if (!node->parent->parent) {
                    node->parent->parent = new Node({0,0}, NONE, 1, NONE, nullptr);
                    cerr << "      Fake grand-parent" << endl;
                }

                node = node->parent;
                
                if (node->parent) {
                    Node* newNode = new Node(rock.first, rock.second, node->score, node->rotation, node->parent);
                    node->parent = newNode;
                    node->rotation = (penalty > 0 ? RIGHT : LEFT);
                    node->nbMoveBefore = round - i - 1;
                    n->score -= abs(penalty);
                    for (int R = 0; R <= round ; ++R)
                    rocksRound[R][idRock].second = NONE;
                    cerr << "KILLING ABORTED!" << endl;
                    cerr << " " << node->parent->coord.x << " " << node->parent->coord.y << " " << dirtostr(node->rotation) << " " << node->score << endl;
                    break;
                }
            }
        }
    }
}

Node* bfs(const vector<vector<int>>& grid,
          const vector<vector<unordered_map<DIRECTION, DIRECTION>>>& roomsWithRotation,
          const vector<pair<int, int>>& idsInRooms,
          const Coord& start, const DIRECTION from,
          const Coord& end,
          const vector<pair<Coord, DIRECTION>>& rocks)
{
    if (start == end) {
        return nullptr;
    }
    queue<Node*> queue;

    // Current room
    const pair<int, int> p = idsInRooms[abs(grid[start.y][start.x])];
    unordered_map<DIRECTION, DIRECTION> room = roomsWithRotation[p.first][p.second];
    if (room.find(from) != room.end()) {
        if (room.at(from) != NONE) {
            const DIRECTION to = room.at(from);
            queue.push(new Node({start.x + directionX[to], start.y + directionY[to]}, oppositeDir[to], 1, NONE, nullptr));
        }
    }
    
    vector<vector<pair<Coord, DIRECTION>>> rocksRound;
    rocksRound.push_back(rocks);

    moveRocks(grid, roomsWithRotation, idsInRooms, rocksRound);

    while (queue.size() > 0) {
        cerr << endl << "Rocks at " << rocksRound.size() << endl;
        for (const auto& rock : rocksRound[rocksRound.size() - 1])
        {
            if (rock.second != NONE)
                cerr << " " << rock.first.x << " " << rock.first.y << endl;
        }
        // BFS -> all node at the same level
        const int size = queue.size();
        for (int i = 0; i < size; ++i) {
            Node *n = queue.front();
            queue.pop();

            checkRocks(grid, roomsWithRotation, idsInRooms, rocksRound, n);

            if (n->coord == end) {
                return n;
            }

            int x = n->coord.x;
            int y = n->coord.y;
            if (y < 0 || y >= grid.size() || x < 0 || x >= grid[0].size() || grid[y][x] == 0) {
                continue;
            }

            // Current room
            const pair<int, int> p = idsInRooms[abs(grid[y][x])];
            unordered_map<DIRECTION, DIRECTION> room = roomsWithRotation[p.first][p.second];
            if (room.find(n->from) != room.end()) {
                if (room.at(n->from) != NONE) {
                    const DIRECTION to = room.at(n->from);
                    queue.push(new Node({x + directionX[to], y + directionY[to]}, oppositeDir[to], n->score + 1, NONE, n));
                }
            }
            if (grid[y][x] < 0)
                continue;

            // Rotated rooms
            const int nbRotationRoom = roomsWithRotation[p.first].size();
            int left = (p.second - 1 + nbRotationRoom) % nbRotationRoom;
            int right = (p.second + 1) % nbRotationRoom;
            int penalty = 1;

            while (penalty <= n->score) {
                // 1 more rotation on the left
                unordered_map<DIRECTION, DIRECTION> roomLeft = roomsWithRotation[p.first][left];
                if (roomLeft.find(n->from) != roomLeft.end() && roomLeft.at(n->from) != NONE) {
                    const DIRECTION to = roomLeft.at(n->from);
                    queue.push(new Node({x + directionX[to], y + directionY[to]}, oppositeDir[to], n->score - penalty + 1, LEFT, n));
                }
                if (left == right)
                    break;

                // 1 more rotation on the right
                unordered_map<DIRECTION, DIRECTION> roomRight = roomsWithRotation[p.first][right];
                if (roomRight.find(n->from) != roomRight.end() && roomRight.at(n->from) != NONE) {
                    const DIRECTION to = roomRight.at(n->from);
                    queue.push(new Node({x + directionX[to], y + directionY[to]}, oppositeDir[to], n->score - penalty + 1, RIGHT, n));
                }

                // Update left, right and penalty
                left = (left - 1 + nbRotationRoom) % nbRotationRoom;
                if (left == right)
                    break;
                right = (right + 1) % nbRotationRoom;
                penalty++;
            }
        }
        moveRocks(grid, roomsWithRotation, idsInRooms, rocksRound);
    }
    return nullptr;
}

int main()
{
    vector<vector<unordered_map<DIRECTION, DIRECTION>>> roomsWithRotation;
    vector<pair<int,int>> idsInRooms;
    setRoomsWithRotation(roomsWithRotation);
    setIdsInRoomsWithRotation(idsInRooms);

    int W; // number of columns.
    int H; // number of rows.
    cin >> W >> H; cin.ignore();
    
    vector<vector<int>> grid;
    for (int i = 0; i < H; i++) {
        string LINE;
        getline(cin, LINE); // each line represents a line in the grid and contains W integers T. The absolute value of T specifies the type of the room. If T is negative, the room cannot be rotated.
    
        stringstream ss(LINE);
        int temp;
        vector<int> array;
        while (ss >> temp) {
            array.push_back(temp);
        }
        grid.push_back(array);
    }
    int EX; // the coordinate along the X axis of the exit.
    cin >> EX; cin.ignore();
    const Coord goal{ EX, H - 1 };

    cerr << "EXIT: " << EX << endl << endl;

    int numberMove = 0; 

    // game loop
    while (1) {
        int XI;
        int YI;
        string POSI;
        cin >> XI >> YI >> POSI; cin.ignore();

        const DIRECTION from = strtodir(POSI);

        cerr << "BEFORE" << endl;
        for (const vector<int>& row : grid) {
            for (const int& ID : row) {
                cerr << " "  << ID;
            }
            cerr << endl;
        }

        int R; // the number of rocks currently in the grid.
        cin >> R; cin.ignore();
        vector<pair<Coord, DIRECTION>> rocks;
        for (int i = 0; i < R; i++) {
            int XR;
            int YR;
            string POSR;
            cin >> XR >> YR >> POSR; cin.ignore();
            rocks.push_back({ {XR, YR}, strtodir(POSR) });
        }

   
        auto startTime = chrono::high_resolution_clock::now();
        Node* node = bfs(grid, roomsWithRotation, idsInRooms, {XI, YI}, from, goal, rocks);
        auto endTime = chrono::high_resolution_clock::now();
        cerr << endl << "Elapsed time: " << chrono::duration_cast<chrono::duration<double>>(endTime - startTime).count() * 1000 << "ms" << endl;

        cerr << endl << "INSTRUCTIONS in memory:" << endl;
        int x, y;
        DIRECTION rot = NONE;
        while (node && node->parent) {
            if (node->rotation != NONE && node->nbMoveBefore == 0) {
                //cerr << "nbMoveBefore " << node->nbMoveBefore << endl;
                x = node->parent->coord.x;
                y = node->parent->coord.y;
                rot = node->rotation;
                cerr << "  " << x << " " << y << " " << dirtostr(rot) << endl;
            }
            else {
                cerr << "  " << node->parent->coord.x << " " << node->parent->coord.y << endl;
            }
            node = node->parent;
        }

        // One line containing on of three commands: 'X Y LEFT', 'X Y RIGHT' or 'WAIT'
        if (rot == NONE) {
            cout << "WAIT" << endl;
        }
        else {
            cout << x << " " << y << " " << dirtostr(rot) << endl;
            grid[y][x] = nextTypeValue(grid[y][x], rot == RIGHT ? 1 : -1);
        }
    
        cerr << endl << "AFTER" << endl;
        for (const vector<int>& row : grid) {
            for (const int& ID : row) {
                cerr << " "  << ID;
            }
            cerr << endl;
        }
    }
}