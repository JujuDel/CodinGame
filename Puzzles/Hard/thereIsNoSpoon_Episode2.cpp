#include <algorithm>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

int W, H;

// #######################################################
//
//                        DATA STRUCT
//
// #######################################################

struct Coord {
  int x;
  int y;

  bool operator==(const Coord &A) const {
    return A.x == this->x && A.y == this->y;
  }
  friend ostream &operator<<(ostream &os, const Coord &C) {
    os << "{" << C.x << ", " << C.y << "}";
    return os;
  }
};

namespace std {
template <> struct hash<Coord> {
  size_t operator()(const Coord &A) const { return A.y * W + A.x; }
};
} // namespace std

struct Neighbors {
  int left;
  int powerLeft;

  int right;
  int powerRight;

  int top;
  int powerTop;

  int bottom;
  int powerBottom;

  int nbNeighbors;

  Neighbors()
      : left{-1}, powerLeft{0}, right{-1}, powerRight{0}, top{-1}, powerTop{0},
        bottom{-1}, powerBottom{0}, nbNeighbors{0} {}

  friend ostream &operator<<(ostream &os, const Neighbors &N) {
    os << "Neighbors:" << endl;
    os << "    IDs -> {L: " << N.left << ", R: " << N.right << ", T: " << N.top
       << ", B: " << N.bottom << "}" << endl;
    os << " Bridge -> {L: " << N.powerLeft << ", R: " << N.powerRight
       << ", T: " << N.powerTop << ", B: " << N.powerBottom << "}" << endl;
    return os;
  }
};

enum Orientation { toRight, toBottom, toLeft, toTop };

// #######################################################
//
//                    HELPING FUNCTIONS
//
// #######################################################

//! To check:
//!   1- Number of links connected to each node match the node's power.
//!   2- Links connect all the nodes into a single connected group.
bool isValid(const vector<Neighbors> &graph, const vector<vector<int>> &grid,
             const vector<Coord> &nodes) {
  vector<bool> visited;
  for (const auto &node : graph) {
    visited.push_back(false);
  }

  queue<int> bfs;
  bfs.push(0);
  visited[0] = true;

  // Nb of connected node counter
  int count = 0;

  // BFS traversal on the graph
  while (bfs.size() > 0) {
    int idx = bfs.front();
    bfs.pop();

    const Coord C = nodes[idx];
    // Grid != 0 -> number of links do not match the node's power
    if (grid[C.y][C.x] != 0) {
      return false;
    }

    // Nb of connected node counter
    count++;

    // Add the next nodes to BFS traversal queue
    if (graph[idx].powerLeft > 0 && !visited[graph[idx].left]) {
      bfs.push(graph[idx].left);
      visited[graph[idx].left] = true;
    }
    if (graph[idx].powerRight > 0 && !visited[graph[idx].right]) {
      bfs.push(graph[idx].right);
      visited[graph[idx].right] = true;
    }
    if (graph[idx].powerTop > 0 && !visited[graph[idx].top]) {
      bfs.push(graph[idx].top);
      visited[graph[idx].top] = true;
    }
    if (graph[idx].powerBottom > 0 && !visited[graph[idx].bottom]) {
      bfs.push(graph[idx].bottom);
      visited[graph[idx].bottom] = true;
    }
  }

  return count == visited.size();
}

//! Draw a bridge of power `power` between the nodes `i` and `j`.
//! Return the memory of the connections removed because of this bridge:
//!  "[The links] must not cross any other links or nodes."
vector<pair<Orientation, pair<int, Neighbors>>>
draw(const int i, const int j, const int power, vector<Neighbors> &graph,
     vector<vector<int>> &grid, const vector<Coord> &nodes,
     const unordered_map<Coord, int> &nodes_map, const Orientation &orientation,
     const bool updateGrid = true) {
  // Update the adjency in the grid
  if (updateGrid) {
    grid[nodes[i].y][nodes[i].x] -= power;
    grid[nodes[j].y][nodes[j].x] -= power;
  }

  // Memory of the removed connections
  vector<pair<Orientation, pair<int, Neighbors>>> memory;

  if (orientation == Orientation::toRight) {
    // Update bridge power
    graph[i].powerRight += power;
    graph[j].powerLeft += power;
    // Avoid lines intersection
    for (int x = nodes[i].x + 1; x < nodes[j].x; ++x) {
      // Looking for the first node on up dir
      for (int y = nodes[i].y - 1; y >= 0; --y) {
        if (nodes_map.find({x, y}) != nodes_map.end()) {
          const int idx = nodes_map.at({x, y});
          if (graph[idx].bottom != -1) {
            memory.push_back({Orientation::toBottom, {idx, graph[idx]}});
            graph[graph[idx].bottom].top = -1;
            graph[idx].bottom = -1;
          }
          break;
        }
      }
    }
  } else if (orientation == Orientation::toBottom) {
    // Update bridge power
    graph[i].powerBottom += power;
    graph[j].powerTop += power;
    // Avoid lines intersection
    for (int y = nodes[i].y + 1; y < nodes[j].y; ++y) {
      // Looking for the first node on left dir
      for (int x = nodes[i].x - 1; x >= 0; --x) {
        if (nodes_map.find({x, y}) != nodes_map.end()) {
          const int idx = nodes_map.at({x, y});
          if (graph[idx].right != -1) {
            memory.push_back({Orientation::toRight, {idx, graph[idx]}});
            graph[graph[idx].right].left = -1;
            graph[idx].right = -1;
          }
          break;
        }
      }
    }
  }
  return memory;
}

//! Remove a bridge of power `power` between the nodes `i` and `j`.
//! Add back the connections (in `memory`) removed by this bridge.
void undraw(const int i, const int j, const int power, vector<Neighbors> &graph,
            vector<vector<int>> &grid, const vector<Coord> &nodes,
            const Orientation &orientation,
            const vector<pair<Orientation, pair<int, Neighbors>>> &memory) {
  // Update the adjency in the grid
  grid[nodes[i].y][nodes[i].x] += power;
  grid[nodes[j].y][nodes[j].x] += power;

  if (orientation == Orientation::toRight) {
    // Update bridge power
    graph[i].powerRight -= power;
    graph[j].powerLeft -= power;
  } else if (orientation == Orientation::toBottom) {
    // Update bridge power
    graph[i].powerBottom -= power;
    graph[j].powerTop -= power;
  }

  // Add back removed connection
  for (const auto &[O, pairMem] : memory) {
    auto &[idx, N] = pairMem;
    graph[idx] = N;
    if (O == Orientation::toRight) {
      graph[graph[idx].right].left = idx;
    } else if (O == Orientation::toBottom) {
      graph[graph[idx].bottom].top = idx;
    }
  }
}

// #######################################################
//
//                    CONSTRUCTIONS
//
// #######################################################

//! From the initial grid, construct the graph: for each nodes, find:
//!   - the id of the first encountered nodes in the 4 directions
//!      (LEFT, RIGHT, TOP, BOTTOM). -1 means no connection.
//!   - the number of neighbors he has.
vector<Neighbors> constructGraph(const vector<vector<int>> &grid,
                                 const unordered_map<Coord, int> &nodes_map,
                                 const vector<Coord> &nodes) {
  // The resulting graph
  vector<Neighbors> graph;

  // For every nodes
  for (int i = 0; i < nodes.size(); ++i) {
    Neighbors neighbors;
    const Coord n = nodes[i];

    int dx = n.x + 1;
    while (dx < W && grid[n.y][dx] == 0)
      dx++;
    // RIGHT neighbor found
    if (dx < W) {
      neighbors.right = nodes_map.at({dx, n.y});
      neighbors.nbNeighbors++;
    }

    dx = n.x - 1;
    while (dx >= 0 && grid[n.y][dx] == 0)
      dx--;
    // LEFT neighbor found
    if (dx >= 0) {
      neighbors.left = nodes_map.at({dx, n.y});
      neighbors.nbNeighbors++;
    }

    int dy = n.y + 1;
    while (dy < H && grid[dy][n.x] == 0)
      dy++;
    // BOTTOM neighbor found
    if (dy < H) {
      neighbors.bottom = nodes_map.at({n.x, dy});
      neighbors.nbNeighbors++;
    }

    dy = n.y - 1;
    while (dy >= 0 && grid[dy][n.x] == 0)
      dy--;
    // TOP neighbor found
    if (dy >= 0) {
      neighbors.top = nodes_map.at({n.x, dy});
      neighbors.nbNeighbors++;
    }

    graph.push_back(neighbors);
  }

  return graph;
}

//! Fill the obvious bridges within the grid. Obvious connections are:
//!     1) Node's power = 2 * node's nbNeighbors
//!         -> e.g. power 8 and 4 neighbors / power 6 and 3 neighbors / ...
//!         -> A bridge of power 2 for every connections
//!         -> The node becomes saturated
//!     2) Node's power = 2 * node's nbNeighbors - 1
//!         -> e.g. power 7 and 4 neighbors / power 5 and 3 neighbors / ...
//!         -> A bridge of power 1 for every connections
//!         -> The node isn't saturated, node's nbNeighbors - 1 bridge remains
//!         to be drawn
void fillObvious(vector<vector<int>> &grid, vector<Neighbors> &graph,
                 const vector<Coord> &nodes,
                 const unordered_map<Coord, int> &nodes_map) {
  // For all the nodes
  for (int i = 0; i < graph.size(); ++i) {
    const Coord C = nodes[i];
    const Neighbors N = graph[i];
    // Obvious case 1
    if (grid[C.y][C.x] == 2 * N.nbNeighbors) {
      if (N.left != -1 && N.powerLeft < 2) {
        draw(N.left, i, 2 - N.powerLeft, graph, grid, nodes, nodes_map,
             Orientation::toRight, false);
      }
      if (N.right != -1 && N.powerRight < 2) {
        draw(i, N.right, 2 - N.powerRight, graph, grid, nodes, nodes_map,
             Orientation::toRight, false);
      }
      if (N.top != -1 && N.powerTop < 2) {
        draw(N.top, i, 2 - N.powerTop, graph, grid, nodes, nodes_map,
             Orientation::toBottom, false);
      }
      if (N.bottom != -1 && N.powerBottom < 2) {
        draw(i, N.bottom, 2 - N.powerBottom, graph, grid, nodes, nodes_map,
             Orientation::toBottom, false);
      }
    }
    // Obvious case 2
    else if (grid[C.y][C.x] == 2 * N.nbNeighbors - 1) {
      if (N.left != -1 && N.powerLeft == 0) {
        draw(N.left, i, 1, graph, grid, nodes, nodes_map, Orientation::toRight,
             false);
      }
      if (N.right != -1 && N.powerRight == 0) {
        draw(i, N.right, 1, graph, grid, nodes, nodes_map, Orientation::toRight,
             false);
      }
      if (N.top != -1 && N.powerTop == 0) {
        draw(N.top, i, 1, graph, grid, nodes, nodes_map, Orientation::toBottom,
             false);
      }
      if (N.bottom != -1 && N.powerBottom == 0) {
        draw(i, N.bottom, 1, graph, grid, nodes, nodes_map,
             Orientation::toBottom, false);
      }
    }
  }

  // For all the nodes, update the adjency grid
  for (int i = 0; i < graph.size(); ++i) {
    const Coord C = nodes[i];
    const Neighbors N = graph[i];
    grid[C.y][C.x] -= N.powerLeft + N.powerRight + N.powerTop + N.powerBottom;
  }
}

// #######################################################
//
//               RECURSIVITY & BACKTRACKING
//
// #######################################################

//! Main resolution function: backtracking
//! For every nodes, if it is not saturated, try to add a power-1 bridge
//! Exit possibilities:
//!   1) All the nodes have been done:
//!         -> true if the final graph is valid, else false
//!   2) The current node isn't satured but no connections can be done:
//!         -> false
bool recursiv(vector<Neighbors> &graph, vector<vector<int>> &grid,
              const vector<Coord> &nodes,
              const unordered_map<Coord, int> &nodes_map, const int idx) {
  if (idx == nodes.size())
    return isValid(graph, grid, nodes);

  const Coord C = nodes[idx];
  const Neighbors N = graph[idx];

  // Some connections are needed
  if (grid[C.y][C.x] >= 1) {
    // Left reachable
    if (N.left != -1 && N.powerLeft <= 1) {
      const Coord CN = nodes[N.left];
      // Connection needed
      if (grid[CN.y][CN.x] >= 1) {
        const auto memory = draw(N.left, idx, 1, graph, grid, nodes, nodes_map,
                                 Orientation::toRight);
        if (recursiv(graph, grid, nodes, nodes_map, idx))
          return true;
        undraw(N.left, idx, 1, graph, grid, nodes, Orientation::toRight,
               memory);
      }
    }
    // Right reachable
    if (N.right != -1 && N.powerRight <= 1) {
      const Coord CN = nodes[N.right];
      // Connection needed
      if (grid[CN.y][CN.x] >= 1) {
        const auto memory = draw(idx, N.right, 1, graph, grid, nodes, nodes_map,
                                 Orientation::toRight);
        if (recursiv(graph, grid, nodes, nodes_map, idx))
          return true;
        undraw(idx, N.right, 1, graph, grid, nodes, Orientation::toRight,
               memory);
      }
    }
    // Top reachable
    if (N.top != -1 && N.powerTop <= 1) {
      const Coord CN = nodes[N.top];
      // Connection needed
      if (grid[CN.y][CN.x] >= 1) {
        const auto memory = draw(N.top, idx, 1, graph, grid, nodes, nodes_map,
                                 Orientation::toBottom);
        if (recursiv(graph, grid, nodes, nodes_map, idx))
          return true;
        undraw(N.top, idx, 1, graph, grid, nodes, Orientation::toBottom,
               memory);
      }
    }
    // Bottom reachable
    if (N.bottom != -1 && N.powerBottom <= 1) {
      const Coord CN = nodes[N.bottom];
      // Connection needed
      if (grid[CN.y][CN.x] >= 1) {
        const auto memory = draw(idx, N.bottom, 1, graph, grid, nodes,
                                 nodes_map, Orientation::toBottom);
        if (recursiv(graph, grid, nodes, nodes_map, idx))
          return true;
        undraw(idx, N.bottom, 1, graph, grid, nodes, Orientation::toBottom,
               memory);
      }
    }
  }

  // Being here means that all the possible connections are done
  // If more needs to be done, a wrong choice was previously made
  if (grid[C.y][C.x] != 0) {
    return false;
  }
  // This node is finished, continue the resolution
  else {
    return recursiv(graph, grid, nodes, nodes_map, idx + 1);
  }
}

// #######################################################
//
//                          MAIN
//
// #######################################################

int main() {

  vector<vector<int>> grid;

  unordered_map<Coord, int> nodes_map;

  vector<Coord> nodes;
  vector<int> values;

  cin >> W;
  cin.ignore();
  cin >> H;
  cin.ignore();
  for (int y = 0; y < H; y++) {
    string line;
    getline(cin, line); // width characters, each either a number or a '.'
    grid.push_back({});
    for (int x = 0; x < W; x++) {
      if (line[x] != '.') {
        values.push_back(line[x] - '0');
        nodes.push_back({x, y});
        nodes_map[nodes[nodes.size() - 1]] = nodes.size() - 1;
        grid[y].push_back(values[values.size() - 1]);
      } else {
        grid[y].push_back(0);
      }
    }
  }

  cerr << "Initial grid:" << endl;
  for (const auto &row : grid) {
    for (const auto &v : row) {
      cerr << (v == 0 ? "." : to_string(v)) << " ";
    }
    cerr << endl;
  }

  // Construct the graph without any bridges
  vector<Neighbors> graph = constructGraph(grid, nodes_map, nodes);

  cerr << endl << "Graph with " << graph.size() << " nodes" << endl << endl;

  fillObvious(grid, graph, nodes, nodes_map);

  cerr << "After obvious filling:" << endl;
  for (const auto &row : grid) {
    for (const auto &v : row) {
      cerr << (v == 0 ? "." : to_string(v)) << " ";
    }
    cerr << endl;
  }
  cerr << endl;

  // Main resolution
  recursiv(graph, grid, nodes, nodes_map, 0);

  // The graph is done, give the answer.
  for (int i = 0; i < nodes.size(); ++i) {
    const Coord C = nodes[i];
    const Neighbors N = graph[i];
    if (N.powerRight > 0) {
      const Coord CN = nodes[N.right];
      cout << C.x << " " << C.y << " " << CN.x << " " << CN.y << " "
           << N.powerRight << endl;
    }
    if (N.powerBottom > 0) {
      const Coord CN = nodes[N.bottom];
      cout << C.x << " " << C.y << " " << CN.x << " " << CN.y << " "
           << N.powerBottom << endl;
    }
  }
}