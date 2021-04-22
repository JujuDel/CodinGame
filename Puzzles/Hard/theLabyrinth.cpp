#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

int Tx, Ty;
int Cx, Cy;
int H; // width.
int W; // Height.
int A; // number of rounds between the time the alarm countdown is activated
       // and the time the alarm goes off.

typedef vector<vector<char>> Grid;

void debug(const Grid &grid) {
  cerr << "GRID" << endl;
  for (const auto &row : grid) {
    for (const auto &c : row) {
      cerr << c;
    }
    cerr << endl;
  }
}

// #######################################################
//
//                          ACTION
//
// #######################################################

enum class Action : unsigned char {
  RIGHT = 0,
  UP = 1,
  LEFT = 2,
  DOWN = 3,
  NONE = 4
};

static const Action _action[4] = {Action::RIGHT, Action::UP, Action::LEFT,
                                  Action::DOWN};
static const string _actionStr[4] = {"RIGHT", "UP", "LEFT", "DOWN"};
static const int _dx[4] = {1, 0, -1, 0};
static const int _dy[4] = {0, -1, 0, 1};

struct Coord {
  int x;
  int y;

  friend inline bool operator==(Coord const &lhs, Coord const &rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
  }
  friend ostream &operator<<(ostream &os, const Coord &P) {
    os << "{" << P.x << ", " << P.y << "}";
    return os;
  }
};

namespace std {
template <> struct hash<Coord> {
  std::size_t operator()(const Coord &p) const noexcept {
    size_t hash = p.x + W * p.y;
    return hash;
  }
};
} // namespace std

// #######################################################
//
//                    BFS OPTIMIZATION
//
// #######################################################

struct NodePlayer {
  //! Previous state information
  size_t parent; //!< Previous player state
  Action action; //!< Action performed to go from previous state to here
  Coord pos;

  //! Graph information
  int depth; //!< Depth of the node in the simulated graph

  //! @brief  C'tor
  NodePlayer() {}

  void set(const int f_parent, const Action &f_action, const Coord &f_pos,
           const int f_depth) {
    parent = f_parent;
    action = f_action;
    pos = f_pos;
    depth = f_depth;
  }

  friend inline bool operator==(NodePlayer const &lhs, NodePlayer const &rhs) {
    return lhs.pos == rhs.pos;
  }
};

constexpr size_t _MAX_NODES_BFS{500000};
static NodePlayer nodesBFS[_MAX_NODES_BFS];
static size_t nodeIdx = 0;

void inc(size_t &idx) { idx = (idx + 1) % _MAX_NODES_BFS; }

vector<Action> unrollBFS(int idx) {
  vector<Action> res;
  while (nodesBFS[idx].action != Action::NONE) {
    res.push_back(nodesBFS[idx].action);
    idx = nodesBFS[idx].parent;
  }
  return res;
}

// #######################################################
//
//                          BFS
//
// #######################################################

bool bfs(const Coord &start, const Coord &end, const Grid &grid,
         const bool aimEnd, size_t &idx) {
  // Set to check if a node already exists
  unordered_set<Coord> state_set;

  // Initial state
  nodesBFS[nodeIdx].set(0, Action::NONE, start, 0);

  size_t s = nodeIdx;
  inc(nodeIdx);
  size_t e = nodeIdx;
  while (s != e) {
    for (idx = s; idx != e; inc(idx)) {
      if (state_set.find(nodesBFS[idx].pos) != state_set.end())
        continue;
      else
        state_set.insert(nodesBFS[idx].pos);

      // Stop condition
      if (aimEnd) {
        if (nodesBFS[idx].pos == end)
          return true;
      } else {
        if (grid[nodesBFS[idx].pos.y][nodesBFS[idx].pos.x] == '?')
          return true;
      }

      for (const Action &a : _action) {
        int y = nodesBFS[idx].pos.y + _dy[(unsigned char)a];
        int x = nodesBFS[idx].pos.x + _dx[(unsigned char)a];
        if (0 <= x && x < W && 0 <= y && y < H && grid[y][x] != '#' && (aimEnd ? grid[y][x] != '?' : true)) {
          nodesBFS[nodeIdx].set(idx, a, {x, y}, nodesBFS[idx].depth + 1);
          inc(nodeIdx);
        }
      }
    }
    s = e;
    e = nodeIdx;
  }
  return false;
}

// #######################################################
//
//                         MAIN
//
// #######################################################

int main() {
  cin >> H >> W >> A;
  cin.ignore();

  cerr << "DIM: " << W << "x" << H << endl;
  cerr << "ALARM: " << A << endl;

  Grid grid;
  for (int i = 0; i < H; ++i) {
    vector<char> row;
    for (int j = 0; j < W; ++j) {
      row.push_back('?');
    }
    grid.push_back(row);
  }

  bool targetFound = false;
  bool alarmTriggered = false;

  // game loop
  int idxRound = 0;
  while (1) {
    int Y; // row where Kirk is located.
    int X; // column where Kirk is located.
    cin >> Y >> X;
    cin.ignore();
    if (idxRound == 0) {
      Tx = X;
      Ty = Y;
    }
    for (int j = 0; j < H; j++) {
      string ROW; // C of the characters in '#.TC?' (i.e. one line of the ASCII
                  // maze).
      cin >> ROW;
      cin.ignore();
      for (int i = 0; i < W; ++i) {
        grid[j][i] = ROW[i];
        if (ROW[i] == 'C') {
          Cx = i;
          Cy = j;
          targetFound = true;
          cerr << "Target Found!" << endl;
        }
      }
    }

    size_t idx;
    if (targetFound) {
      if (alarmTriggered || (X == Cx && Y == Cy)) {
        cerr << "Hurry Up!!" << endl;
        bfs({X, Y}, {Tx, Ty}, grid, true, idx);
        vector<Action> actions = unrollBFS(idx);
        cerr << "SIZE: " << actions.size() << endl;
        cout << _actionStr[(unsigned char)actions[actions.size() - 1]] << endl;
        alarmTriggered = true;
      } else {
        bool isPossible = bfs({Cx, Cy}, {Tx, Ty}, grid, true, idx);
        if (isPossible) {
          cerr << "DEPTH: " << nodesBFS[idx].depth << " / SIZE: " << unrollBFS(idx).size() << endl;
          cerr << "ALARM: " << A << endl;
        }
        if (isPossible && nodesBFS[idx].depth <= A) {
          cerr << "Youhou, run is possible, let's go on contol!" << endl;
          bfs({X, Y}, {Cx, Cy}, grid, true, idx);
          vector<Action> actions = unrollBFS(idx);
          cout << _actionStr[(unsigned char)actions[actions.size() - 1]]
               << endl;
        } else {
          cerr << "Hmmm, run isn't possible, let's explore!" << endl;
          bfs({X, Y}, {}, grid, false, idx);
          vector<Action> actions = unrollBFS(idx);
          cout << _actionStr[(unsigned char)actions[actions.size() - 1]]
               << endl;
        }
      }
    } else {
      cerr << "Hmmm, let's explore!" << endl;
      bfs({X, Y}, {}, grid, false, idx);
      vector<Action> actions = unrollBFS(idx);
      cout << _actionStr[(unsigned char)actions[actions.size() - 1]] << endl;
    }

    grid[Y][X] = 'X';
    debug(grid);

    idxRound++;
  }
}