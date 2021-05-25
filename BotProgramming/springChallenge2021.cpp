#include <algorithm>
#include <chrono>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

#define DIR(x) x % 6

#define NUMBER_CELLS 37
#define NUMBER_DIRECTION 6

#define MAX_SIZE_TREE 3

#define BEAM_SIZE_HERO 5
#define BEAM_SIZE_VILAIN 1
#define BEAM_DEPTH 1

#define LAST_DAY 23

// ############################################## //
//                                                //
//                      CELL                      //
//                                                //
// ############################################## //

struct Cell {
  int index;    // Those 8 are provided before the beginning
  int richness; // of the game and the code setting them is in
  int neigh0;   // the function `inputBoard`
  int neigh1;
  int neigh2;
  int neigh3;
  int neigh4;
  int neigh5;

  int size;       // Those 3 are provided at each turn
  bool isMine;    // and the code setting them is in the function
  bool isDormant; // `inputTrees`

  bool isSpookyShadow;

  Cell() {}

  Cell(const int f_index, const int f_richness, const int f_neigh0,
       const int f_neigh1, const int f_neigh2, const int f_neigh3,
       const int f_neigh4, const int f_neigh5)
      : index{f_index}, richness{f_richness}, neigh0{f_neigh0},
        neigh1{f_neigh1}, neigh2{f_neigh2}, neigh3{f_neigh3}, neigh4{f_neigh4},
        neigh5{f_neigh5}, size{-1}, isSpookyShadow{false} {}

  void reset() {
    size = -1;
    isSpookyShadow = false;
  }

  bool isNeighbor(const int i) const {
    return i == neigh0 || i == neigh1 || i == neigh2 || i == neigh3 ||
           i == neigh4 || i == neigh5;
  }

  int getNeighbor(const int i) const {
    if (i < 0 || i > 5)
      return -1;

    if (i == 0)
      return neigh0;
    else if (i == 1)
      return neigh1;
    else if (i == 2)
      return neigh2;
    else if (i == 3)
      return neigh3;
    else if (i == 4)
      return neigh4;
    else
      return neigh5;
  }

  friend ostream &operator<<(ostream &os, const Cell &C) {
    if (C.size == -1) {
      os << "...{" << C.isSpookyShadow << ",R" << C.richness << "}...";
    } else {
      os << "{" << C.isSpookyShadow << ",R" << C.richness << ",S" << C.size
         << ",P" << C.isMine << "}";
    }
    return os;
  }
  const static int sizeStr{12};
};

// ############################################## //
//                                                //
//                 GRID  PRINTERS                 //
//                                                //
// ############################################## //

const vector<vector<int>> _indexHexagonalGrid{
    {25, 24, 23, 22},         // Line0
    {26, 11, 10, 9, 21},      // Line1
    {27, 12, 3, 2, 8, 20},    // Line2
    {28, 13, 4, 0, 1, 7, 19}, // Line3
    {29, 14, 5, 6, 18, 36},   // Line4
    {30, 15, 16, 17, 35},     // Line5
    {31, 32, 33, 34}          // Line6
};
const vector<int> _offsets{3, 2, 1, 0, 1, 2, 3};

enum EPrintType { ALL, RICHNESS, SIZE, SHADOW };

string printType2str(const EPrintType f_printType) {
  switch (f_printType) {
  case ALL:
    return "ALL";
  case RICHNESS:
    return "RICHNESS";
  case SIZE:
    return "SIZE";
  case SHADOW:
    return "SHADOW";
  }
};

void print(const Cell *f_board, const EPrintType f_printType) {
  cerr << printType2str(f_printType) << endl;

  string offsetChar{f_printType == ALL ? string(Cell::sizeStr / 2, ' ') : " "};

  for (int line = 0; line < _indexHexagonalGrid.size(); ++line) {
    for (int o = 0; o < _offsets[line]; ++o) {
      cerr << offsetChar;
    }
    for (const int cell : _indexHexagonalGrid[line]) {
      switch (f_printType) {
      case ALL:
        cerr << f_board[cell] << " ";
        break;
      case RICHNESS:
        cerr << (f_board[cell].size == -1 ? "."
                                          : to_string(f_board[cell].richness))
             << " ";
        break;
      case SIZE:
        cerr << (f_board[cell].size == -1 ? "." : to_string(f_board[cell].size))
             << " ";
        break;
      case SHADOW:
        cerr << (f_board[cell].isSpookyShadow > 0 ? "X" : ".") << " ";
        break;
      }
    }
    cerr << endl;
  }
}

void print(const int *f_arr, const string &f_title) {
  cerr << f_title << endl;
  for (int line = 0; line < _indexHexagonalGrid.size(); ++line) {
    for (int o = 0; o < _offsets[line]; ++o) {
      cerr << " ";
    }
    for (const int cell : _indexHexagonalGrid[line]) {
      cerr << f_arr[cell] << " ";
    }
    cerr << endl;
  }
}

void print(const vector<bool> &f_vec, const string &f_title) {
  cerr << f_title << endl;
  for (int line = 0; line < _indexHexagonalGrid.size(); ++line) {
    for (int o = 0; o < _offsets[line]; ++o) {
      cerr << " ";
    }
    for (const int cell : _indexHexagonalGrid[line]) {
      cerr << (f_vec[cell] ? "X" : ".") << " ";
    }
    cerr << endl;
  }
}

// ############################################## //
//                                                //
//                     ACTION                     //
//                                                //
// ############################################## //

struct Action {
  string actionType;
  int cost;
  int idx_1;
  int idx_2;

  Action(const std::string &f_actionType = "WAIT", const int f_cost = 0,
         const int f_idx1 = 0, const int f_idx2 = 0)
      : actionType{f_actionType}, cost{f_cost}, idx_1{f_idx1}, idx_2{f_idx2} {}

  string toOutput() const {
    if (actionType == "GROW") {
      return "GROW " + to_string(idx_1);
    } else if (actionType == "COMPLETE") {
      return "COMPLETE " + to_string(idx_1);
    } else if (actionType == "SEED") {
      return "SEED " + to_string(idx_1) + " " + to_string(idx_2);
    } else {
      return "WAIT";
    }
  }

  friend bool operator==(const Action &A, const Action &B) {
    return A.toOutput() == B.toOutput();
  }

  friend ostream &operator<<(ostream &os, const Action &A) {
    os << "Action {" << A.actionType << ", C" << to_string(A.cost);
    if (A.actionType == "SEED")
      os << ", " << to_string(A.idx_1) << ", " << to_string(A.idx_2);
    else if (A.actionType == "COMPLETE" || A.actionType == "GROW")
      os << ", " << to_string(A.idx_1);
    os << "}";
    return os;
  }
};

inline int computeScoreComplete(const Cell *f_board, const int f_nutrients,
                                const int f_idx) {
  return f_nutrients + 2 * (f_board[f_idx].richness - 1);
}

// ############################################## //
//                                                //
//                     PLAYER                     //
//                                                //
// ############################################## //

const vector<int> _costGrow{1, 3, 7};
const vector<int> _cumCostGrow{15, 14, 11, 4};

struct Player {
  int sun;
  int score;
  bool isWaiting;

  vector<int> trees[MAX_SIZE_TREE + 1]; // trees[i] contains the indexes of the
                                        // cells with a tree of size i
  vector<bool> toSeed;

  // Assignment operator
  Player &operator=(const Player &P) {
    sun = P.sun;
    score = P.score;
    score = P.score;
    isWaiting = P.isWaiting;
    for (int s = 0; s <= MAX_SIZE_TREE; ++s) {
      trees[s] = P.trees[s];
    }
    toSeed = P.toSeed;
    return *this;
  }

  Player() { toSeed = vector<bool>(NUMBER_CELLS, true); }

  void clearToSeed(const Cell *f_board) {
    for (int i = 0; i < NUMBER_CELLS; ++i) {
      toSeed[i] = f_board[i].richness != 0;
    }
  }

  void updateToSeed(const Cell *f_board, const Player &f_opp) {
    clearToSeed(f_board);
    // Update self shadowing
    for (int size = 0; size <= MAX_SIZE_TREE; size++) {
      for (const int &tree : trees[size]) {
        toSeed[tree] = false;
        // Try all direction, for MAX_SIZE_TREE neighbors
        for (int dir = 0; dir < NUMBER_DIRECTION; ++dir) {
          int currIdx{tree};
          for (int j = 0; j < MAX_SIZE_TREE; ++j) {
            currIdx = f_board[currIdx].getNeighbor(dir);
            if (currIdx == -1)
              break;
            toSeed[currIdx] = false;
          }
        }
      }
    }
    // Update opponent trees
    for (int size = 0; size <= MAX_SIZE_TREE; size++) {
      for (const int &tree : f_opp.trees[size]) {
        toSeed[tree] = false;
      }
    }
  }

  int sunToCompleteAll() const { return 4 * trees[MAX_SIZE_TREE].size(); }

  int sunUntilDay(const int f_dayCurr, const int f_dayEnd, const Cell *f_board,
                  const int f_recVal = 0) const {
    if (f_dayCurr >= f_dayEnd)
      return f_recVal + sun;

    vector<bool> isSpookyShadow(NUMBER_CELLS, false);

    const int idShadow{DIR(f_dayCurr + 1)};
    for (int c = 0; c < NUMBER_CELLS; ++c) {
      if (f_board[c].size > 0) {
        int currId{f_board[c].getNeighbor(idShadow)};
        for (int i = 0; i < f_board[c].size; ++i) {
          if (currId == -1)
            break;
          if (f_board[currId].size != -1) {
            isSpookyShadow[currId] = f_board[c].size >= f_board[currId].size;
          }
          currId = f_board[currId].getNeighbor(idShadow);
        }
      }
    }

    int nbSun{0};
    for (int size = 1; size <= MAX_SIZE_TREE; ++size) {
      for (const int &tree : trees[size]) {
        if (!isSpookyShadow[tree])
          nbSun += size;
      }
    }

    return sunUntilDay(f_dayCurr + 1, f_dayEnd, f_board, f_recVal + nbSun);
  }

  int computeCostAction(const string &f_action, const Cell *f_board,
                        const int f_idx = 0) const {
    if (f_action == "SEED") {
      // Cost is number of seed on the field
      return trees[0].size();
    }

    if (f_action == "GROW") {
      int size{f_board[f_idx].size};
      // {0->1:1, 1->2:3, 2->3:7} + number of tree on the field of size + 1
      return 1 + size * (size + 1) + trees[size + 1].size();
    }

    if (f_action == "COMPLETE") {
      // Cost is always 4
      return 4;
    }

    return 0;
  }

  //! @brief Recursive method which update the vector of Actions with all legal
  //!        seeds from f_start and with a distance f_dist
  //!
  //! @param[in]  f_board    Array of 37 cells
  //! @param[in]  f_day      The current day
  //! @param[in]  f_start    Idx of the initial tree
  //! @param[in]  f_currIdx  Current cell idx
  //! @param[in]  f_dist     Current distance from the initial tree
  //! @param[in]  f_size     Size of the initial tree
  //! @param[in]  f_cost     Cost of the action
  //! @param[out] f_actions  Resulting vector of actions
  //! @param[out] f_mem      Memory of already planted cells
  void recSeed(const Cell *f_board, const int f_day, const int f_start,
               const int f_currIdx, const int f_dist, const int f_size,
               const int f_cost, const int *f_attack, vector<Action> &f_actions,
               unordered_set<int> &f_mem) const {
    // Too far or out of the grid
    if (f_currIdx == -1 || f_dist > f_size)
      return;

    // Never visited cell
    if (f_board[f_currIdx].richness != 0 && f_board[f_currIdx].size == -1 &&
        f_mem.find(f_currIdx) == f_mem.end()) {
      if (keepSeed(f_board, f_day, f_start, f_currIdx, f_attack)) {
        // Add the action
        f_actions.push_back({"SEED", f_cost, f_start, f_currIdx});
        // Memorize this cell
        f_mem.insert(f_currIdx);
      }
    }
    // Continue to go further
    for (int dir = 0; dir < NUMBER_DIRECTION; dir++) {
      recSeed(f_board, f_day, f_start, f_board[f_currIdx].getNeighbor(dir),
              f_dist + 1, f_size, f_cost, f_attack, f_actions, f_mem);
    }
  }

  //! @brief Get all the legal moves of the player
  //!
  //! @param[in] f_board  Array of 37 cells
  //!
  //! @return A vector containing all the legal actions
  vector<Action> getAllLegalMoves(Cell *f_board, const int f_day,
                                  const int f_nutrients,
                                  const int *f_attack) const {
    if (isWaiting)
      return {{"WAIT"}};

    vector<Action> actions;

    // 1 - SEED
    const int costSeed{computeCostAction("SEED", f_board)};
    if (costSeed <= sun) {
      // For trees of size 1 to trees of maximum size
      for (int size = 1; size <= MAX_SIZE_TREE; ++size) {
        for (const int &tree : trees[size]) {
          if (f_board[tree].isDormant)
            continue;

          // Memory of cell already noted
          unordered_set<int> mem;
          for (int dir = 0; dir < NUMBER_DIRECTION; dir++) {
            recSeed(f_board, f_day, tree, f_board[tree].getNeighbor(dir), 1,
                    size, costSeed, f_attack, actions, mem);
          }
        }
      }
    }

    // 2 - GROW
    for (int size = 0; size < MAX_SIZE_TREE; ++size) {
      if (trees[size].size() > 0) {
        const int costGrow{computeCostAction("GROW", f_board, trees[size][0])};
        if (costGrow <= sun) {
          for (const int &tree : trees[size]) {
            if (!f_board[tree].isDormant &&
                keepGrow(f_board, f_day, f_nutrients, tree))
              actions.push_back({"GROW", costGrow, tree});
          }
        }
      }
    }

    // 3 - COMPLETE
    if (sun >= 4) {
      for (const int &tree : trees[MAX_SIZE_TREE]) {
        if (!f_board[tree].isDormant &&
            keepComplete(f_board, f_day, f_nutrients, tree))
          actions.push_back({"COMPLETE", 4, tree});
      }
    }

    // 4 - WAIT
    actions.push_back({"WAIT"});

    return actions;
  }

  // --------------------------------------------- //
  //                    KEEPERS                    //
  // --------------------------------------------- //

  bool keepGrow(Cell *f_board, const int f_day, const int f_nutrients,
                const int f_idx) const {
    if (f_day < 19)
      return true;

    // If not enough day to make to tree to size max
    if (MAX_SIZE_TREE - f_board[f_idx].size > LAST_DAY - f_day)
      return false;

    const int size = f_board[f_idx].size;
    int cost = 4;
    while (f_board[f_idx].size < 3) {
      cost += computeCostAction("GROW", f_board, f_idx);
      f_board[f_idx].size++;
    }
    f_board[f_idx].size = size;

    return computeScoreComplete(f_board, f_nutrients, f_idx) > cost;
  }

  bool keepComplete(const Cell *f_board, const int f_day, const int f_nutrients,
                    const int f_idx) const {
    return f_day >= 12 &&
           computeScoreComplete(f_board, f_nutrients, f_idx) >= 2;
  }

  bool keepSeed(const Cell *f_board, const int f_day, const int from,
                const int to, const int *f_attack = nullptr) const {
    if (trees[0].size() > 0)
      return false;

    bool toAdd = true;

    for (int d = 0; d < 6; ++d) {
      int idx = to;
      for (int r = 0; r < 2; ++r) {
        idx = f_board[idx].getNeighbor(d);
        if (idx == -1)
          break;

        if (f_board[idx].size >= 0 && f_board[idx].isMine) {
          toAdd = false;
          break;
        }
      }
    }

    if (!f_attack)
      return f_day < 19 && (toAdd || f_day > 12);
    else
      return f_day < 19 && f_attack[to] > 0;
  }

  // --------------------------------------------- //
  //                    __str__                    //
  // --------------------------------------------- //

  friend ostream &operator<<(ostream &os, const Player &P) {
    os << "Player {Su: " << P.sun << ", Sc: " << P.score << "}";
    return os;
  }
};

// ############################################## //
//                                                //
//                   SIMULATION                   //
//                                                //
// ############################################## //

namespace myHash {
template <typename T> void hash_combine(size_t &seed, T const &v) {
  seed ^= hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
template <typename It> void hash_range(std::size_t &seed, It first, It last) {
  for (; first != last; ++first) {
    hash_combine(seed, *first);
  }
}

void hash_player(size_t &seed, Player const &P) {
  hash_combine(seed, P.score);
  hash_combine(seed, P.sun);
  for (int s = 0; s <= MAX_SIZE_TREE; ++s) {
    hash_range(seed, P.trees[s].begin(), P.trees[s].end());
  }
}

void hash_cell(size_t &seed, Cell const &C) {
  hash_combine(seed, C.index);
  hash_combine(seed, C.isDormant);
  hash_combine(seed, C.size);
}
} // namespace myHash

//! @brief  Simulate a Game state
struct Game {
  //! Game state information
  int day;
  double nutrients;

  //! Previous state information
  int parent;          //!< Previous player state
  Action actionHero;   //!< Actions performed by me and my opponent to go from
  Action actionVilain; //!< previous state to here

  //! Current state information
  Cell board[NUMBER_CELLS]; //!< The board of the game
  int shadow[NUMBER_CELLS]; //!< The number of tree doing a shadow per cell
  Player hero;              //!< Me at this game state
  Player vilain;            //!< My opponent at this game state

  int attack[NUMBER_CELLS];
  int shadow_next[NUMBER_CELLS];

  int opp_heatmap[NUMBER_CELLS];
  int seed_heatmap[NUMBER_CELLS];
  int max_heatmap;

  //! @brief  C'tor
  Game() {}

  // Compute the shadows at f_day
  void computeShadowsAndHeatmap() {
    for (int c = 0; c < NUMBER_CELLS; ++c) {
      shadow_next[c] = 0;
      attack[c] = 0;
      seed_heatmap[c] = 0;
      opp_heatmap[c] = 0;
    }
    max_heatmap = 0;

    int dir_next{(day + 1) % 6};

    for (int c = 0; c < NUMBER_CELLS; ++c) {
      if (board[c].size > 0) {
        int idx = c;
        for (int s = 0; s < board[c].size; ++s) {
          idx = board[idx].getNeighbor(dir_next);
          if (idx == -1)
            break;

          shadow_next[idx] = max(shadow_next[idx], board[c].size);
        }
      }
    }

    for (int c = 0; c < NUMBER_CELLS; ++c) {
      if (board[c].size > 0) {
        int idx = c;
        for (int s = 0; s < board[c].size; ++s) {
          idx = board[idx].getNeighbor(dir_next);
          if (idx == -1)
            break;

          if (board[idx].richness > 0) {
            if (board[c].isMine)
              attack[idx] -= 1;
            else
              attack[idx] += 1;
          }
        }
      }
    }

    for (int c = 0; c < NUMBER_CELLS; ++c) {
      if (board[c].size > 0) {
        if (board[c].isMine)
          seed_heatmap[c]++;
        else
          opp_heatmap[c]++;

        for (int dir = 0; dir < 6; ++dir) {
          int idx = c;
          for (int s = 0; s < 3; ++s) {
            idx = board[idx].getNeighbor(dir);
            if (idx == -1)
              break;
            if (board[c].isMine)
              seed_heatmap[idx]++;
            else
              opp_heatmap[idx]++;
          }
        }
      }
    }
    for (int c = 0; c < NUMBER_CELLS; ++c) {
      max_heatmap = max(max_heatmap, seed_heatmap[c]);
    }
  }

  pair<int, int> countSunPenalty(const int f_idx) const {
    if (board[f_idx].size <= 0)
      return {0, 0};

    pair<int, int> res = {0, 0};
    int idx = f_idx;
    int dir_next{(day + 1) % 6};

    for (int s = 0; s < board[f_idx].size; ++s) {
      idx = board[idx].getNeighbor(dir_next);
      if (idx == -1)
        break;

      if (board[idx].size > 0 && board[f_idx].size >= board[idx].size) {
        if (board[idx].isMine)
          res.first += board[idx].size;
        else
          res.second += board[idx].size;
      }
    }
    return res;
  }

  // --------------------------------------------- //
  //                     UTILS                     //
  // --------------------------------------------- //

  void set(const int f_parent, const Action &f_actionHero,
           const Action &f_actionVilain, const Cell *f_board,
           const int *f_shadow, const Player &f_hero, const Player &f_vilain,
           const int f_day, const int f_nutrients) {
    parent = f_parent;
    actionHero = f_actionHero;
    actionVilain = f_actionVilain;

    if (f_shadow) {
      for (int c = 0; c < NUMBER_CELLS; ++c) {
        board[c] = f_board[c];
        shadow[c] = f_shadow[c];
      }
    } else {
      for (int c = 0; c < NUMBER_CELLS; ++c) {
        board[c] = f_board[c];
      }
    }

    hero = f_hero;
    vilain = f_vilain;

    day = f_day;
    nutrients = f_nutrients;

    computeShadowsAndHeatmap();
  }

  void resetShadows() {
    // Put back the shadow array to false
    for (int c = 0; c < NUMBER_CELLS; ++c) {
      shadow[c] = 0;
    }

    // Direction of the shadows
    const int dir{DIR(day)};

    // Apply spooky shadows from hero
    for (int size = 1; size <= MAX_SIZE_TREE; ++size) {
      for (const int &tree : hero.trees[size]) {
        int idx = tree;
        for (int d = 0; d < size; ++d) {
          idx = board[idx].getNeighbor(dir);
          if (idx == -1)
            break;
          if (board[idx].size >= 0 && board[idx].size <= size) {
            shadow[idx]++;
          }
        }
      }
    }

    // Apply spooky shadows from vilain
    for (int size = 1; size <= MAX_SIZE_TREE; ++size) {
      for (const int &tree : vilain.trees[size]) {
        int idx = tree;
        for (int d = 0; d < size; ++d) {
          idx = board[idx].getNeighbor(dir);
          if (idx == -1)
            break;
          if (board[idx].size >= 0 && board[idx].size <= size) {
            shadow[idx]++;
          }
        }
      }
    }
  }

  // --------------------------------------------- //
  //                 MAKE  ACTIONS                 //
  // --------------------------------------------- //

  void startDay() {
    for (int size = 0; size <= MAX_SIZE_TREE; ++size) {
      // Give hero his suns and wake up the dormant trees
      for (const int &tree : hero.trees[size]) {
        board[tree].isDormant = false;
        if (!shadow[tree])
          hero.sun += size;
      }
      // Give vilain his suns and wake up the dormant trees
      for (const int &tree : vilain.trees[size]) {
        board[tree].isDormant = false;
        if (!shadow[tree])
          vilain.sun += size;
      }
    }

    // The players aren't waiting anymore
    hero.isWaiting = false;
    vilain.isWaiting = false;

    computeShadowsAndHeatmap();
  }

  void makeWait(Player *f_p_player) { f_p_player->isWaiting = true; }

  void makeComplete(Player *f_p_player, const Action *f_p_action,
                    const bool f_bothComplete) {
    // Apply cost
    f_p_player->sun -= 4;
    // Get points
    f_p_player->score +=
        computeScoreComplete(board, nutrients, f_p_action->idx_1);
    if (!f_bothComplete)
      nutrients = max(0., nutrients - 1);
    // Remove the tree's shadow
    const int dir{DIR(day)};
    int idx = f_p_action->idx_1;
    for (int d = 0; d < board[f_p_action->idx_1].size; ++d) {
      idx = board[idx].getNeighbor(dir);
      if (idx == -1)
        break;
      if (board[idx].size > 0 &&
          board[idx].size <= board[f_p_action->idx_1].size) {
        shadow[idx]--;
      }
    }
    // Remove tree from f_p_player->trees[MAX_SIZE_TREE]
    f_p_player->trees[MAX_SIZE_TREE].erase(
        remove(f_p_player->trees[MAX_SIZE_TREE].begin(),
               f_p_player->trees[MAX_SIZE_TREE].end(), f_p_action->idx_1),
        f_p_player->trees[MAX_SIZE_TREE].end());
    // Remove tree from board
    board[f_p_action->idx_1].size = -1;
    shadow[f_p_action->idx_1] = 0;
  }

  void makeGrow(Player *f_p_player, const Action *f_p_action,
                const bool f_bothGrow) {
    // Apply cost
    f_p_player->sun -= f_p_action->cost;
    // Remove tree from f_p_player->trees[size]
    f_p_player->trees[board[f_p_action->idx_1].size].erase(
        remove(f_p_player->trees[board[f_p_action->idx_1].size].begin(),
               f_p_player->trees[board[f_p_action->idx_1].size].end(),
               f_p_action->idx_1),
        f_p_player->trees[board[f_p_action->idx_1].size].end());
    // Add tree to hero.trees[size + 1]
    hero.trees[board[f_p_action->idx_1].size + 1].push_back(f_p_action->idx_1);
    // Grow the tree
    board[f_p_action->idx_1].size++;
    // Make the tree dormant
    board[f_p_action->idx_1].isDormant = true;
    // Extend the tree's shadow effect
    if (!f_bothGrow) {
      int idx = f_p_action->idx_1;
      int dir = DIR(day);
      for (int d = 0; d < board[f_p_action->idx_1].size; ++d) {
        idx = board[idx].getNeighbor(dir);
        if (idx == -1)
          break;
        if ((d == board[f_p_action->idx_1].size - 1 && board[idx].size > 0 &&
             board[idx].size <= board[f_p_action->idx_1].size) ||
            (d < board[f_p_action->idx_1].size - 1 &&
             board[idx].size == board[f_p_action->idx_1].size)) {
          shadow[idx]++;
        }
      }
    }
  }

  void makeSeed(Player *f_p_player, const Action *f_p_action) {
    // Apply cost
    f_p_player->sun -= f_p_action->cost;
    // Add seed to hero.trees[0]
    f_p_player->trees[0].push_back(f_p_action->idx_2);
    // Add seed to board
    board[f_p_action->idx_2].size = 0;
    board[f_p_action->idx_2].isMine = f_p_player == &hero;
    // Make the trees dormant
    board[f_p_action->idx_1].isDormant = true;
    board[f_p_action->idx_2].isDormant = true;
  }

  void next(const int f_parentIdx, const Game &f_parent,
            const Action &f_actionHero, const Action &f_actionVilain) {
    // Initialize the node
    set(f_parentIdx, f_actionHero, f_actionVilain, f_parent.board,
        f_parent.shadow, f_parent.hero, f_parent.vilain, f_parent.day,
        f_parent.nutrients);

    if (day > 23) {
      makeWait(&hero);
      makeWait(&vilain);
      return;
    }

    const bool doSame{actionHero.actionType == actionVilain.actionType};

    vector<pair<Player *, Action *>> pairs;
    pairs.push_back({&hero, &actionHero});
    pairs.push_back({&vilain, &actionVilain});
    for (pair<Player *, Action *> &pair : pairs) {
      Player *p_player{pair.first};
      Action *p_actionPlayer{pair.second};
      if (p_actionPlayer->actionType == "WAIT") {
        makeWait(p_player);
      } else if (p_actionPlayer->actionType == "COMPLETE") {
        makeComplete(p_player, p_actionPlayer, doSame);
      } else if (p_actionPlayer->actionType == "GROW") {
        makeGrow(p_player, p_actionPlayer, doSame);
      } else if (p_actionPlayer->actionType == "SEED") {
        makeSeed(p_player, p_actionPlayer);
      }
    }

    if (doSame) {
      if (actionHero.actionType == "COMPLETE")
        nutrients -= 2;
      if (actionHero.actionType == "GROW")
        resetShadows();
    }

    // Next day is starting
    if (hero.isWaiting && vilain.isWaiting) {
      day += 1;
      if (day <= LAST_DAY) {
        resetShadows();
        startDay();
      }
    }
  }

  // --------------------------------------------- //
  //                     EVALS                     //
  // --------------------------------------------- //

  void reward(double &eval, const Player *player, const Player *player_opp,
              const Action *action) const {
    // Reward on seed
    if (action->actionType == "SEED") {
      // eval += 10;
      eval += (max_heatmap - seed_heatmap[action->idx_2]) * 5;
    }

    // Penalise on wait to prefer to do actions in late games
    if (action->actionType == "WAIT") {
      eval -= 15 * day;
    }

    const vector<double> cte_grow{15, 20, 25};
    if (action->actionType == "GROW") {
      // Reward the growing action
      eval += cte_grow[board[action->idx_1].size - 1];
      eval += opp_heatmap[action->idx_1] * 5;

      const bool shouldBeMine = (player == &hero ? false : true);

      // Reward when blocking an opponent tree
      const int dir = (day + 1) % 6;
      int idx = action->idx_1;
      for (int s = 0; s < board[action->idx_1].size; ++s) {
        idx = board[idx].getNeighbor(dir);
        if (idx == -1)
          break;

        if (board[idx].size > 0 && board[idx].isMine == shouldBeMine &&
            board[idx].size <= board[action->idx_1].size) {
          eval += board[idx].size * 10;
        }
      }

      // Penalise if growing a tree blocked at next turn
      const int dir_reverse = (day + 4) % 6;
      idx = action->idx_1;
      for (int s = 0; s < board[action->idx_1].size; ++s) {
        idx = board[idx].getNeighbor(dir_reverse);
        if (idx == -1)
          break;

        if (board[idx].size > 0 && board[idx].isMine == shouldBeMine &&
            board[idx].size >= board[action->idx_1].size) {
          eval -= board[action->idx_1].size * 10;
          break;
        }
      }
    }

    if (action->actionType == "COMPLETE") {
      if (day < 23) {
        // Complete on late game with more reward
        eval += 20 * ((int)player->trees[MAX_SIZE_TREE].size() + 1 - 5);
        // Complete on late game with more reward
        eval += day / 2;
        // Penalise/Reward if tree will give sun on next turn
        if (shadow_next[action->idx_1] != MAX_SIZE_TREE)
          eval -= 30;
        else
          eval += 30;
        // Reward if oppenent is ahead
        eval += 30 * (player_opp->score - player->score > 20);
      } else
        eval += 100;
    }
  }

  double eval_solo(const string &playerstr) const {
    if (day > 23)
      return 0;

    double eval = 0;

    const Player *player{playerstr == "HERO" ? &hero : &vilain};
    const Player *player_opp{playerstr == "HERO" ? &vilain : &hero};
    const Action *action{playerstr == "HERO" ? &actionHero : &actionVilain};
    const bool shouldBeMine{playerstr == "HERO"};

    int tree[]{0, 0, 0, 0};
    int nbClose = 0;
    for (const int s : {0, 1, 2, 3}) {
      for (const int t : player->trees[s]) {
        tree[s] += board[t].richness;
        for (int dir = 0; dir < 6; ++dir) {
          int neighbor = board[t].getNeighbor(dir);
          if (neighbor == -1)
            continue;
          if (board[neighbor].size > 0 &&
              board[neighbor].isMine == shouldBeMine) {
            nbClose++;
            break;
          }
        }
      }
    }

    eval += (player->score - player_opp->score) * 3;
    eval += player->sun - player_opp->sun;

    reward(eval, player, player_opp, action);

    if (day < 19)
      eval -= nbClose * 5;

    double cte_tree[]{10, 5, 10, 20};

    if (day < 19)
      eval += tree[0] * cte_tree[0];
    if (day < 20)
      eval += tree[1] * cte_tree[1];
    if (day < 21)
      eval += tree[2] * cte_tree[2];
    if (day < 22)
      eval += tree[3] * cte_tree[3];

    return eval;
  }

  double evaluate(const string &playerstr) const {
    if (day > 23)
      return 0;

    double eval = 0;

    const Player *player{playerstr == "HERO" ? &hero : &vilain};
    const Player *player_opp{playerstr == "HERO" ? &vilain : &hero};
    const Action *action{playerstr == "HERO" ? &actionHero : &actionVilain};
    const bool shouldBeMine{playerstr == "HERO"};

    int tree[]{0, 0, 0, 0};
    int nbClose = 0;
    for (const int s : {0, 1, 2, 3}) {
      for (const int t : player->trees[s]) {
        tree[s] += board[t].richness;
        for (int dir = 0; dir < 6; ++dir) {
          int neighbor = board[t].getNeighbor(dir);
          if (neighbor == -1)
            continue;
          if (board[neighbor].size > 0 &&
              board[neighbor].isMine == shouldBeMine) {
            nbClose++;
            break;
          }
        }
      }
    }

    eval += player->score * 3;
    eval += player->sun / 3;

    if (day < 19)
      eval -= nbClose * 5;

    double cte_tree[]{1, 5, 10, 20};

    if (day < 19)
      eval += tree[0] * cte_tree[0];
    if (day < 20)
      eval += tree[1] * cte_tree[1];
    if (day < 21)
      eval += tree[2] * cte_tree[2];
    if (day < 22)
      eval += tree[3] * cte_tree[3];

    return eval;
  }

  // --------------------------------------------- //
  //                    __str__                    //
  // --------------------------------------------- //

  friend ostream &operator<<(ostream &os, const Game &G) {
    os << "---- Game State ----" << endl;
    os << "      Day: " << G.day << endl;
    os << "Nutrients: " << G.nutrients << endl;
    os << endl;
    os << "Parent: " << G.parent << endl;
    os << "  Hero: " << G.actionHero << endl;
    os << "      : Sun " << G.hero.sun << " / Score " << G.hero.score << endl;
    os << " Vilan: " << G.actionVilain << endl;
    os << "      : Sun " << G.vilain.sun << " / Score " << G.vilain.score
       << endl;
    os << endl;
    print(G.board, EPrintType::SIZE);
    print(G.shadow, "SHADOWS");
    os << "----------------------";
    return os;
  }
};

struct Game_hash {
  static size_t hash(const Game &_node) {
    size_t seed = 0;

    for (int c = 0; c < NUMBER_CELLS; ++c) {
      myHash::hash_cell(seed, _node.board[c]);
    }
    myHash::hash_player(seed, _node.hero);
    myHash::hash_player(seed, _node.vilain);

    myHash::hash_combine(seed, _node.day);
    myHash::hash_combine(seed, _node.nutrients);

    return seed;
  }
};

#define MAX_GRAPH 100000
static Game s_gamesGraph[MAX_GRAPH];
static size_t s_nodeIdx = 0;

// Compute the shadows at f_day
vector<int> computeShadow(const Cell *f_board, const int f_day) {
  vector<int> shadows(NUMBER_CELLS, 0);

  int dir{DIR(f_day)};

  for (int c = 0; c < NUMBER_CELLS; ++c) {
    if (f_board[c].size > 0) {
      int idx = c;
      for (int s = 0; s < f_board[c].size; ++s) {
        idx = f_board[idx].getNeighbor(dir);
        if (idx == -1)
          break;

        shadows[idx] = max(shadows[idx], f_board[c].size);
      }
    }
  }

  return shadows;
}

// ############################################## //
//                                                //
//                     CHOICE                     //
//                                                //
// ############################################## //

string chooseAction(vector<Action> &f_actions, const Cell *f_board,
                    const Player &hero, const Player &Vilain, const int f_day) {
  vector<pair<int, pair<int, int>>> orders_seed;
  vector<pair<int, int>> orders_grow;
  vector<pair<int, int>> orders_complete;

  const int sunToCompleteAll{hero.sunToCompleteAll()};
  const int sunAtEnd{hero.sunUntilDay(f_day, LAST_DAY, f_board)};

  // Split the actions into 3 categories, ignoring the "WAIT" order
  for (const Action &a : f_actions) {
    if (a.actionType == "SEED") {
      orders_seed.push_back({a.cost, {a.idx_1, a.idx_2}});
    } else if (a.actionType == "GROW") {
      orders_grow.push_back({a.cost, a.idx_1});
    } else if (a.actionType == "COMPLETE") {
      orders_complete.push_back({a.cost, a.idx_1});
    }
  }

  // 1 - Try to seed
  int bestIdx = -1;
  if (f_day < 19) {
    for (int idx = 0; idx < orders_seed.size(); idx++) {
      const pair<int, pair<int, int>> pair_seed{orders_seed[idx]};
      const int cost{pair_seed.first};
      const pair<int, int> seed{pair_seed.second};

      if (sunAtEnd - cost < sunToCompleteAll) {
        continue;
      }

      if (hero.toSeed[seed.second]) {
        if (bestIdx == -1) {
          bestIdx = idx;
        } else if (seed.second < orders_seed[bestIdx].second.second ||
                   (seed.second == orders_seed[bestIdx].second.second &&
                    f_board[seed.first].size <
                        f_board[orders_seed[bestIdx].first].size)) {
          bestIdx = idx;
        }
      }
    }
  }
  if (bestIdx != -1) {
    return "SEED " + to_string(orders_seed[bestIdx].second.first) + " " +
           to_string(orders_seed[bestIdx].second.second);
  }

  // 2 - Try to grow
  if (f_day != LAST_DAY) {
    for (const pair<int, int> &pair_grow : orders_grow) {
      const int cost{pair_grow.first};
      const int grow{pair_grow.second};

      if (sunAtEnd - cost < sunToCompleteAll) {
        continue;
      }
      if (bestIdx == -1) {
        bestIdx = grow;
      } else if (f_board[grow].size > f_board[bestIdx].size ||
                 (f_board[grow].size == f_board[bestIdx].size &&
                  grow < bestIdx)) {
        bestIdx = grow;
      }
    }
  }
  if (bestIdx != -1) {
    return "GROW " + to_string(bestIdx);
  }

  // 3 - Try to complete
  for (const pair<int, int> &pair_complete : orders_complete) {
    const int cost{pair_complete.first};
    const int complete{pair_complete.second};
    if (bestIdx == -1) {
      bestIdx = complete;
    } else if ((f_day < 20 &&
                f_board[complete].richness > f_board[bestIdx].richness) ||
               (f_day >= 20 &&
                f_board[complete].richness > f_board[bestIdx].richness)) {
      bestIdx = complete;
    }
  }
  if (bestIdx != -1 && f_day > 10) {
    return "COMPLETE " + to_string(bestIdx);
  }

  return "WAIT";
}

// ############################################## //
//                                                //
//                  BEAM  SEARCH                  //
//                                                //
// ############################################## //

Action chooseBetweenActions(const Game &f_game, const Cell *f_board,
                            const Action &A, const Action &B) {
  if (A.actionType == "SEED") {
    // Same target
    if (A.idx_2 == B.idx_2) {
      // Take the new if will give sun next turn
      if (f_game.shadow_next[B.idx_1] != MAX_SIZE_TREE) {
        return B;
      }
    }
    // Different target
    else {
      // Put the seed where attack is higher
      if (f_game.attack[B.idx_2] < f_game.attack[A.idx_2]) {
        return B;
      }
      // If equal, take the new if will give sun next turn
      else if (f_game.attack[B.idx_2] == f_game.attack[A.idx_2] &&
               f_game.shadow_next[B.idx_1] != MAX_SIZE_TREE)
        return B;
    }
  }

  else if (A.actionType == "GROW") {
    if (B.actionType == "SEED") {
      return B;
    }
    if (B.actionType == "GROW" && f_board[B.idx_1].size > f_board[A.idx_1].size)
      return B;
  }

  else if (A.actionType == "COMPLETE") {
    // Prefer SEED and GROW other COMPLETE
    if (B.actionType == "SEED" || B.actionType == "GROW") {
      return B;
    }
    // FROM 2 COMPLETE, prefer the one for which
    // hidden sun vilain - hidden sun hero is higher
    else if (B.actionType == "COMPLETE") {
      const pair<int, int> curr = f_game.countSunPenalty(A.idx_1);
      const pair<int, int> cand = f_game.countSunPenalty(B.idx_1);
      if (cand.second - cand.first > curr.second - curr.first)
        return B;
    }
  }

  else
    return B;

  return A;
}

struct NodePrio {
  size_t idx;
  double score;
};

class CompareNodeByScore {
public:
  bool operator()(const NodePrio &a, const NodePrio &b) {
    return a.score < b.score;
  }
};

Action beamSearchUntilDay(const Cell *f_board,
                          const vector<Action> &f_firstActions,
                          const Player &f_hero, const Player &f_vilain,
                          const int f_day, const int f_nutrients,
                          const bool debug = false) {
  // Initial state
  s_gamesGraph[s_nodeIdx].set(-1, {"INIT"}, {"INIT"}, f_board, nullptr, f_hero,
                              f_vilain, f_day, f_nutrients);
  s_gamesGraph[s_nodeIdx].resetShadows();
  s_gamesGraph[s_nodeIdx].computeShadowsAndHeatmap();

  // Current width of the graph
  queue<NodePrio> nodesUntilDay;
  nodesUntilDay.push({s_nodeIdx, 0});

  const chrono::time_point<chrono::high_resolution_clock> start{
      chrono::high_resolution_clock::now()};

  // inc s_nodeIdx
  s_nodeIdx = (s_nodeIdx + 1) % MAX_GRAPH;

  // Next ordered level of the graph
  priority_queue<NodePrio, vector<NodePrio>, CompareNodeByScore> prio_queue;
  int count = 0;
  while (nodesUntilDay.size() > 0 && count < 4) {
    const chrono::duration<double> duration_bs{
        chrono::high_resolution_clock::now() - start};
    const double ms_bs{duration_bs.count() * 1000};
    cerr << " Turn " << count++ << ": BS lasted " << ms_bs << "ms for now"
         << endl;

    size_t size{nodesUntilDay.size()};
    for (int i = 0; i < size; ++i) {
      // Current node's idx
      const size_t idx{nodesUntilDay.front().idx};
      const double prevScore{nodesUntilDay.front().score};
      nodesUntilDay.pop();

      const vector<Action> actionsHero{
          s_gamesGraph[idx].parent == -1
              ? f_firstActions
              : s_gamesGraph[idx].hero.getAllLegalMoves(
                    s_gamesGraph[idx].board, f_day, f_nutrients,
                    s_gamesGraph[idx].attack)};

      // Simulate each action from s_gamesGraph[idx]
      for (const Action &actionH : actionsHero) {
        // Simulate the action
        s_gamesGraph[s_nodeIdx].next(idx, s_gamesGraph[idx], actionH, {"NONE"});

        // End of the day
        if (actionH.actionType == "WAIT") {
          // Next day
          s_gamesGraph[s_nodeIdx].day += 1;
          if (s_gamesGraph[s_nodeIdx].day <= LAST_DAY) {
            s_gamesGraph[s_nodeIdx].resetShadows();
            s_gamesGraph[s_nodeIdx].startDay();
          }

          // Evaluate the board state after the simulation
          const double eval{s_gamesGraph[s_nodeIdx].evaluate("HERO")};
          // Add the new board state in the prio_queue
          prio_queue.push({s_nodeIdx, prevScore + eval});
        } else {
          double eval = prevScore;
          s_gamesGraph[s_nodeIdx].reward(eval, &s_gamesGraph[s_nodeIdx].hero,
                                         &s_gamesGraph[s_nodeIdx].vilain,
                                         &actionH);
          // Add the node in the list
          nodesUntilDay.push({s_nodeIdx, eval});
        }

        // Move forward in the graph
        s_nodeIdx = (s_nodeIdx + 1) % MAX_GRAPH;
      }
    }
  }

  bool showDebug = true;

  if (showDebug)
    cerr << "======================" << endl;

  // Unroll all the best choices and choose one
  vector<size_t> choices;
  const double best{prio_queue.top().score};
  while (!prio_queue.empty() && prio_queue.top().score == best) {
    size_t idx{prio_queue.top().idx};
    double score{prio_queue.top().score};
    prio_queue.pop();

    vector<Action> debug;

    if (s_gamesGraph[idx].parent == -1) {
      // Should NEVER happen
      if (score == best)
        choices.push_back(idx);
      if (showDebug)
        cerr << score << ": Action {WAIT}" << endl;
    } else {
      while (s_gamesGraph[s_gamesGraph[idx].parent].parent != -1) {
        if (showDebug)
          debug.push_back(s_gamesGraph[idx].actionHero);
        idx = s_gamesGraph[idx].parent;
      }
      if (score == best)
        choices.push_back(idx);

      if (showDebug) {
        debug.push_back(s_gamesGraph[idx].actionHero);
        cerr << score << ": ";
        for (int i = debug.size() - 1; i >= 0; --i) {
          cerr << debug[i] << (i != 0 ? " -> " : "");
        }
        cerr << endl;
      }
    }
  }

  Action res{"INIT"};
  for (const int &idx : choices) {
    if (res.actionType == "INIT") {
      res = s_gamesGraph[idx].actionHero;
    } else {
      cerr << "EX-AEQUO " << res << " / " << s_gamesGraph[idx].actionHero
           << endl;
      // When several actions have the same score:
      //  1 - SEED
      //  2 - GROW
      //  3 - COMPLETE
      //  4 - WAIT
      if (0) {
        res = chooseBetweenActions(s_gamesGraph[idx], s_gamesGraph[idx].board,
                                   res, s_gamesGraph[idx].actionHero);
      } else {
        if (s_gamesGraph[idx].actionHero.cost < res.cost)
          res = s_gamesGraph[idx].actionHero;
        else if (s_gamesGraph[idx].actionHero.cost == res.cost)
          res = chooseBetweenActions(s_gamesGraph[idx], s_gamesGraph[idx].board,
                                     res, s_gamesGraph[idx].actionHero);
      }
      cerr << "  " << res << endl;
    }
  }

  cerr << "======================" << endl;
  cerr << res << endl;

  return res;
}

Action beamSearch(const Cell *f_board, const vector<Action> &f_firstActions,
                  const Player &f_hero, const Player &f_vilain, const int f_day,
                  const int f_nutrients, const bool debug = false) {
  // Initial state
  s_gamesGraph[s_nodeIdx].set(-1, {"INIT"}, {"INIT"}, f_board, nullptr, f_hero,
                              f_vilain, f_day, f_nutrients);
  s_gamesGraph[s_nodeIdx].resetShadows();
  s_gamesGraph[s_nodeIdx].computeShadowsAndHeatmap();

  // cerr << s_gamesGraph[s_nodeIdx] << endl;

  // Current depth of the graph
  int graphDepth{0};

  // Current width of the graph
  queue<NodePrio> nodesLevel;
  nodesLevel.push({s_nodeIdx, 0});

  const chrono::time_point<chrono::high_resolution_clock> start{
      chrono::high_resolution_clock::now()};

  // Beam search loop
  s_nodeIdx = (s_nodeIdx + 1) % MAX_GRAPH;
  while (graphDepth++ < BEAM_DEPTH) {
    if (1 || debug) {
      cerr << "======================" << endl;
      cerr << "Turn " << graphDepth << endl;
      const chrono::duration<double> duration_bs{
          chrono::high_resolution_clock::now() - start};
      const double ms_bs{duration_bs.count() * 1000};
      cerr << "  BS lasted " << ms_bs << "ms for now" << endl;
    }

    size_t size{nodesLevel.size()};
    if (size == 0)
      return {"WAIT"};

    // Next ordered level of the graph
    priority_queue<NodePrio, vector<NodePrio>, CompareNodeByScore> prio_queue;
    for (int i = 0; i < size; ++i) {
      // Current node's idx
      const size_t idx{nodesLevel.front().idx};
      const double prevScore{nodesLevel.front().score};
      nodesLevel.pop();

      // 1 - SIMULATE VILAIN WITH SOLO EVAL

      const vector<Action> actionsVilain{
          s_gamesGraph[idx].vilain.getAllLegalMoves(
              s_gamesGraph[idx].board, f_day, f_nutrients, nullptr)};
      if (0 || debug) {
        cerr << "  idx " << idx << " - Vilain: " << actionsVilain.size()
             << " possible actions" << endl;
      }

      double bestEvalVilain{numeric_limits<double>::min()};
      Action actionVilain;
      for (const Action &actionV : actionsVilain) {
        // Simulate the action
        s_gamesGraph[s_nodeIdx].next(idx, s_gamesGraph[idx], {"NONE"}, actionV);
        // Evaluate the board state after the simulation
        const double eval{s_gamesGraph[s_nodeIdx].eval_solo("VILAIN")};

        if (eval > bestEvalVilain) {
          bestEvalVilain = eval;
          actionVilain = actionV;
        }
      }

      // 2 - SIMULATE HERO WITH SOLO EVAL, KNOWING VILAIN's ACTIONS

      const vector<Action> actionsHero{
          s_gamesGraph[idx].parent == -1
              ? f_firstActions
              : s_gamesGraph[idx].hero.getAllLegalMoves(
                    s_gamesGraph[idx].board, f_day, f_nutrients,
                    s_gamesGraph[idx].attack)};

      if (0 || debug) {
        cerr << "  idx " << idx << " - Hero: " << actionsHero.size()
             << " possible actions" << endl;
      }

      // Simulate each action from s_gamesGraph[idx]
      for (const Action &actionH : actionsHero) {
        // Simulate the action
        s_gamesGraph[s_nodeIdx].next(idx, s_gamesGraph[idx], actionH,
                                     actionVilain);
        // Evaluate the board state after the simulation
        const double eval{s_gamesGraph[s_nodeIdx].eval_solo("HERO")};
        // Add the new board state in the prio_queue
        prio_queue.push({s_nodeIdx, eval});

        if (graphDepth == BEAM_DEPTH) {
          cerr << " " << actionH << " -> " << eval << endl;
        }

        // Move forward in the graph
        s_nodeIdx = (s_nodeIdx + 1) % MAX_GRAPH;
      }

      // 3 - UNROLL BEST HERO ACTIONS, KNOWING VILAIN's ACTIONS

      for (int i = 0; !prio_queue.empty() && i < BEAM_SIZE_HERO; ++i) {
        nodesLevel.push(prio_queue.top());
        prio_queue.pop();
      }
    }
  }

  Action res{"INIT"};
  int idxRes;
  size_t sizeNodesLevel{nodesLevel.size()};
  const double best{nodesLevel.front().score};
  while (!nodesLevel.empty() &&
         (res.actionType == "INIT" || nodesLevel.front().score == best)) {
    size_t idx{nodesLevel.front().idx};
    nodesLevel.pop();

    // Unroll
    if (BEAM_DEPTH > 1) {
      while (s_gamesGraph[s_gamesGraph[idx].parent].parent != -1) {
        idx = s_gamesGraph[idx].parent;
      }
    }

    s_gamesGraph[idx].resetShadows();
    s_gamesGraph[idx].computeShadowsAndHeatmap();

    // If the res is COMPLETE
    if (s_gamesGraph[idx].actionHero.actionType == "COMPLETE") {
      // Cancel the COMPLETE when:
      // - I have less than 4 level3 trees
      // - vilain score is not to far away
      // - before day 15
      // - the tree will give sun at next turn
      if (f_hero.trees[3].size() < 5 && f_vilain.score - f_hero.score < 20 &&
          f_day < 15 &&
          s_gamesGraph[idxRes]
                  .shadow_next[s_gamesGraph[idx].actionHero.idx_1] !=
              MAX_SIZE_TREE) {
        cerr << "IGNOREv1 this " << s_gamesGraph[idx].actionHero << endl;
        continue;
      }
      // Cancel the COMPLETE when the tree will give sun and opp has less than 3
      // trees
      if (f_day < 23 &&
          s_gamesGraph[idx].shadow_next[s_gamesGraph[idx].actionHero.idx_1] !=
              MAX_SIZE_TREE &&
          f_vilain.trees[3].size() < 1) {
        cerr << "IGNOREv2 this " << s_gamesGraph[idx].actionHero << endl;
        continue;
      }
      // Cancel the COMPLETE when the tree will give sun and
      // - opp is waiting OR
      // - opp doesn't have enough sun to grow a level 3
      if (f_day < 23 &&
          s_gamesGraph[idx].shadow_next[s_gamesGraph[idx].actionHero.idx_1] !=
              MAX_SIZE_TREE &&
          (f_vilain.isWaiting || f_vilain.sun < 7 + f_vilain.trees[3].size())) {
        cerr << "IGNOREv3 this " << s_gamesGraph[idx].actionHero << endl;
        continue;
      }
    }

    if (res.actionType == "INIT") {
      res = s_gamesGraph[idx].actionHero;
      idxRes = idx;
    } else {
      cerr << "EX-AEQUO: " << res << " / " << s_gamesGraph[idx].actionHero
           << endl;
      cerr << "  " << res << " -> ";

      res = chooseBetweenActions(s_gamesGraph[idx], s_gamesGraph[idx].board,
                                 res, s_gamesGraph[idx].actionHero);
      if (res == s_gamesGraph[idx].actionHero)
        idxRes = idx;
      cerr << res << endl;
    }
  }

  cerr << "======================" << endl;
  cerr << res << endl;

  return res;
}

// ############################################## //
//                                                //
//                     INPUTS                     //
//                                                //
// ############################################## //

void inputBoard(Cell *f_board, const int f_nbCells = 37) {
  for (int i = 0; i < f_nbCells; i++) {
    int index;    // 0 is the center cell, the next cells spiral outwards
    int richness; // 0 if the cell is unusable, 1-3 for usable cells
    int neigh0;   // the index of the neighbouring cell for each direction
    int neigh1;
    int neigh2;
    int neigh3;
    int neigh4;
    int neigh5;
    cin >> index >> richness >> neigh0 >> neigh1 >> neigh2 >> neigh3 >>
        neigh4 >> neigh5;
    cin.ignore();
    f_board[index] = {index,  richness, neigh0, neigh1,
                      neigh2, neigh3,   neigh4, neigh5};
  }
}

void inputTrees(Cell *f_board, Player &hero, Player &vilain,
                const int f_nbCells = 37) {
  int numberOfTrees; // the current amount of trees
  cin >> numberOfTrees;
  cin.ignore();

  for (int size = 0; size <= MAX_SIZE_TREE; size++) {
    hero.trees[size].clear();
    vilain.trees[size].clear();
  }

  for (int t = 0; t < numberOfTrees; t++) {
    int cellIndex;  // location of this tree
    int size;       // size of this tree: 0-3
    bool isMine;    // 1 if this is your tree
    bool isDormant; // 1 if this tree is dormant
    cin >> cellIndex >> size >> isMine >> isDormant;
    cin.ignore();
    f_board[cellIndex].size = size;
    f_board[cellIndex].isMine = isMine;
    f_board[cellIndex].isDormant = isDormant;

    if (isMine) {
      hero.trees[size].push_back(cellIndex);
    } else {
      vilain.trees[size].push_back(cellIndex);
    }
  }
}

vector<Action> inputAction(const Player &hero, Cell *f_board, const int f_day,
                           const int f_nutrients) {
  vector<Action> actions;

  // All legal actions
  int numberOfPossibleActions;
  cin >> numberOfPossibleActions;
  cin.ignore();

  string space_delimiter = " ";
  for (int i = 0; i < numberOfPossibleActions; i++) {
    string possibleAction;
    getline(cin, possibleAction);

    // Split the action by space and store the words in the vector Action
    vector<string> action;
    size_t pos = 0;
    while ((pos = possibleAction.find(space_delimiter)) != string::npos) {
      action.push_back(possibleAction.substr(0, pos));
      possibleAction.erase(0, pos + space_delimiter.length());
    }
    action.push_back(possibleAction);

    if (action[0] == "WAIT")
      actions.push_back({"WAIT"});

    if (action[0] == "COMPLETE") {
      const int idx{stoi(action[1])};
      if (hero.keepComplete(f_board, f_day, f_nutrients, idx)) {
        actions.push_back({"COMPLETE",
                           hero.computeCostAction("COMPLETE", f_board, idx),
                           idx});
      }
    }

    if (action[0] == "GROW") {
      const int idx{stoi(action[1])};
      if (hero.keepGrow(f_board, f_day, f_nutrients, idx)) {
        actions.push_back(
            {"GROW", hero.computeCostAction("GROW", f_board, idx), idx});
      }
    }

    if (action[0] == "SEED") {
      int from = stoi(action[1]);
      int to = stoi(action[2]);
      if (hero.keepSeed(f_board, f_day, from, to)) {
        actions.push_back(
            {"SEED", hero.computeCostAction("SEED", f_board), from, to});
      }
    }
  }

  return actions;
}

// ############################################## //
//                                                //
//                      MAIN                      //
//                                                //
// ############################################## //

int main() {
    std::cerr << "You have enoug info to make your own BOT now \U0001F609" << std::endl;
    std::cerr << "http://www.someecards.com/usercards/nsviewcard/MjAxMi04ZWYxNjNmNTI5ODZiMjk0/" << std::endl;
}