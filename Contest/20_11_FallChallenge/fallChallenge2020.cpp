#include <algorithm>
#include <chrono>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

/*
    TODO / IDEAS:
     - BFS should OK (another check, warum nicht?)
     - Make "smart" choice for the learning spells:
        o having an homogeneous inc_casts?
        o force learning of +tier-3 then other learning during BFS ?
          => Trying learning by score?
     - Choice of potion accordingly to vilainScore - heroScore
     - Do not construct current potion if vilain target the same and need more
   turn
     - Check few first turn of opponent to analyse blocker moves

    DONE:
     - BFS optimized
     - Sort cost in price order before starting the BFS
     - Pick the potion with best ratio price / cost
     - Learn first spells with their score:
   score = 1 * tier-0 + 2 * tier-1 + 3 * tier-2 + 4 * tier-3 + tax - tomeIdx
     - Smart choice on BFS return:
        o get all path with same depth and perform a selection
        o selection priority:
          1. learn with highest inventory
          2. other with highest inventory
*/

//!< Game parameters
constexpr int _NB_TIERS{4};
constexpr int _NB_BREWS{5};
constexpr int _NB_VISIBLE_TOME_SPELLS{6};
constexpr int _SIZE_INVENTORY{10};

//!< BFS parameters
constexpr int _MAX_DEPTH_BFS{12};
constexpr int _MAX_BREADTH_BFS{500};
constexpr int _MIN_INV_FOR_LEARNING{1};
constexpr int _MAX_LEARN_SPELL_BFS{3};

//!< Target number of spells to have
constexpr int _MAX_LEARN_SPELL{10};

//!< In arena or not (for some display)
constexpr bool _IN_ARENA{true};

// #######################################################
//
//                      INVENTORY
//
// #######################################################

//! @brief  Simulates an array of size 4
struct Ingredients {
  int I0; //!< Amount of tier-0 ingredient
  int I1; //!< Amount of tier-1 ingredient
  int I2; //!< Amount of tier-2 ingredient
  int I3; //!< Amount of tier-3 ingredient

  //! @brief  simulate the use of an array
  const int &operator[](const int i) const {
    if (i == 0)
      return I0;
    if (i == 1)
      return I1;
    if (i == 2)
      return I2;
    else
      return I3;
  }
  //! @brief  simulate the use of an array
  int &operator[](const int i) {
    if (i == 0)
      return I0;
    if (i == 1)
      return I1;
    if (i == 2)
      return I2;
    else
      return I3;
  }

  //! @brief  The number of tier-x ingredient
  int sum() const { return I0 + I1 + I2 + I3; }

  //! @brief  The score of the ingredient
  int score() const { return I0 + 2 * I1 + 3 * I2 + 4 * I3; }

  bool allPositiv() const { return I0 >= 0 && I1 >= 0 && I2 >= 0 && I3 >= 0; }

  Ingredients &operator+=(const Ingredients &A) {
    this->I0 += A.I0;
    this->I1 += A.I1;
    this->I2 += A.I2;
    this->I3 += A.I3;
    return *this;
  }

  friend Ingredients abs(const Ingredients &P) {
    Ingredients Q{abs(P.I0), abs(P.I1), abs(P.I2), abs(P.I3)};
    return Q;
  }
  friend Ingredients operator*(const int &a, const Ingredients &P) {
    Ingredients Q{a * P.I0, a * P.I1, a * P.I2, a * P.I3};
    return Q;
  }
  friend Ingredients operator+(const Ingredients &A, const Ingredients &B) {
    Ingredients ing{A.I0 + B.I0, A.I1 + B.I1, A.I2 + B.I2, A.I3 + B.I3};
    return ing;
  }
  friend ostream &operator<<(ostream &os, const Ingredients &P) {
    os << "Ingredients " << P.I0 << " " << P.I1 << " " << P.I2 << " " << P.I3;
    return os;
  }
};

// #######################################################
//
//                        ACTIONS
//
// #######################################################

//! @brief  Struct to represent an action
struct Action {
  string type; //!< Something among "BREW", "CAST" and "REST"
  int ID;      //!< If not "REST", the ID of the thing
  int repeat;  //!< If "CAST", the number of time to repeat the spell

  //! @brief  C'tor
  Action(const string &f_type = "REST", const int f_ID = -1,
         const int f_repeat = 1)
      : type{f_type}, ID{f_ID}, repeat{f_repeat} {}

  friend ostream &operator<<(ostream &os, const Action &P) {
    os << "Action " << P.type << (P.type == "REST" ? "" : " " + to_string(P.ID))
       << (P.type == "CAST" ? " " + to_string(P.repeat) : "");
    return os;
  }
};

//! @brief  Struct to store a spell
struct Cast {
  int ID;          //!< ID of the spell
  Ingredients ing; //!< Content of the spell
  bool repeatable; //!< Whether or not the sort is repeatable

  //! @brief  C'tor
  Cast(const int f_id, const Ingredients &f_ing, const bool f_repeatable)
      : ID{f_id}, repeatable{f_repeatable}, ing{f_ing} {}

  friend ostream &operator<<(ostream &os, const Cast &P) {
    os << " -- Cast " << P.ID << " -- " << endl;
    os << P.ing << endl;
    return os;
  }
};

//! @brief  Struct to store a learnable spell
struct Learn {
  Cast cast;   //!< The spell
  int tomeIdx; //!< Its idx in the magic tome
  int tax;     //!< Its read-ahead tax

  //! @brief  C'tor
  Learn(const Cast &f_cast, const int f_tomeIdx, const int f_tax)
      : cast{f_cast}, tomeIdx{f_tomeIdx}, tax{f_tax} {}

  friend ostream &operator<<(ostream &os, const Learn &P) {
    os << " -- Learn " << P.cast.ID << " -- " << endl;
    os << P.cast.ing << endl;
    os << "Tome index: " << P.tomeIdx << endl;
    os << "Read-ahead tax: " << P.tax << endl;
    return os;
  }

  //! @brief  Sorter method for the STL sort function
  static bool tomeIdxSorter(const Learn &a, const Learn &b) {
    return a.tomeIdx < b.tomeIdx;
  }
};

//! @brief  Struct to store the data of the magic tome
struct MagicTome {
  vector<Learn> learns; //!< All the learnable spells

  //!< Below are the learnt spells used while simulating during the BFS
  vector<Cast> casts;    //!< All the casts
  vector<int> castables; //!< Whether or not the spells or castable:
                         //!<   o -1 -> not learnt
                         //!<   o 0 -> learnt but not castable
                         //!<   o 1 -> learnt and castable

  //! @brief  C'tor
  MagicTome() {
    // Add fake learn / casts
    for (int i = 0; i < _NB_VISIBLE_TOME_SPELLS; ++i) {
      learns.push_back({{-1, {0, 0, 0, 0}, 0}, 0, 0});
      casts.push_back({-1, {0, 0, 0, 0}, 0});
      castables.push_back(-1);
    }
  }

  //! @brief  Add a learnable spell
  void addLearn(const Learn &f_learn) { learns[f_learn.tomeIdx] = f_learn; }

  //! @brief  Learn a spell by updating all the related info:
  //!           `casts`, `castables` -> the learnt spells
  //!           `learns` -> the learnable spells
  //!
  //! @param[in] idx  The idx of the spell to learn in the learns list
  //!
  //! @return  The pair {read-ahead tax, learnID} of the learnt spell
  pair<int, int> learnSpell(const int idx) {
    // Add the cast in `casts` and `castables`
    casts[learns[idx].tomeIdx] = learns[idx].cast;
    castables[learns[idx].tomeIdx] = 1;

    // Get the info before erasing the spell
    pair<int, int> learnInfo = {learns[idx].tax, learns[idx].cast.ID};

    // Erase the learnable spell
    learns.erase(learns.begin() + idx);

    // Apply the tax to the spell
    for (int i = 0; i < idx; ++i) {
      learns[idx].tax++;
    }

    return learnInfo;
  }

  friend ostream &operator<<(ostream &os, const MagicTome &P) {
    os << "  Magic castables: ";
    for (const auto &b : P.castables)
      os << b << " ";
    os << endl;
    return os;
  }
};

//! @brief  Struct to store the receip of a potion
struct Brew {
  int ID;          //!< ID of the potion
  int price;       //!< Price of the potion
  Ingredients ing; //!< Content of the potion

  //! @brief  C'tor
  Brew(const int f_id, const int f_p, const Ingredients &f_ing)
      : ID{f_id}, price{f_p}, ing{f_ing} {}

  //! @brief  Given an inventory, whether or not the potion can be brewed
  //!
  //! @param[in] inventory  The current inventory
  //!
  //! @return Wheter or not the potion is brewable
  bool isBrewable(const Ingredients &inventory) const {
    for (int i = 0; i < _NB_TIERS; ++i)
      if (inventory[i] + ing[i] < 0)
        return false;
    return true;
  }

  //! @brief  Sorter method for the STL sort function
  static bool priceSorterInc(const Brew &a, const Brew &b) {
    return a.price < b.price;
  }
  static bool priceSorterDec(const Brew &a, const Brew &b) {
    return a.price > b.price;
  }

  friend ostream &operator<<(ostream &os, const Brew &P) {
    os << " -- Brew " << P.ID << " -- " << endl;
    os << "Price " << P.price << endl;
    os << P.ing;
    return os;
  }
};

// #######################################################
//
//                    BFS OPTIMIZATION
//
// #######################################################

namespace myHash {
template <typename T> void hash_combine(size_t &seed, T const &v) {
  seed ^= hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <typename It> void hash_range(std::size_t &seed, It first, It last) {
  for (; first != last; ++first) {
    hash_combine(seed, *first);
  }
}
} // namespace myHash

//! @brief  Simulate a player state
struct NodePlayer {
  //! Previous state information
  int parent;    //!< Previous player state
  Action action; //!< Action performed to go from previous state to here

  //! Current state information
  vector<bool> castables; //!< Whether or not the spells are castable
  Ingredients inventory;  //!< Current inventory
  MagicTome magicTome;    //!< Magic tome
  int hasLearnt;          //!< How many spells learnt for now

  //! Graph information
  int depth; //!< Depth of the node in the simulated graph

  //! @brief  C'tor
  NodePlayer() {}

  void set(const int f_parent, const Action &f_action,
           const vector<bool> &f_castables, const Ingredients &f_inv,
           const MagicTome &f_magicTome, const int f_hasLearnt,
           const int f_depth) {
    parent = f_parent;
    action = f_action;
    castables = f_castables;
    inventory = f_inv;
    magicTome = f_magicTome;
    hasLearnt = f_hasLearnt;
    depth = f_depth;
  }

  friend ostream &operator<<(ostream &os, const NodePlayer &P) {
    os << "---- Fake Player ----" << endl;
    os << P.action << endl;
    os << "Magic castables: ";
    for (const auto &castable : P.magicTome.castables)
      os << castable << " ";
    os << endl;
    os << "Castables: ";
    for (const auto &castable : P.castables)
      os << castable << " ";
    os << endl;
    os << P.inventory << endl;
    os << "Parent: " << P.parent << endl;
    os << "Depth: " << P.depth << endl;
    os << "----------------------";
    return os;
  }
};

struct NodePlayer_hash {
  static size_t hash(const NodePlayer &_node) {
    size_t seed = 0;
    myHash::hash_combine(seed, _node.inventory.I0);
    myHash::hash_combine(seed, _node.inventory.I1);
    myHash::hash_combine(seed, _node.inventory.I2);
    myHash::hash_combine(seed, _node.inventory.I3);
    myHash::hash_range(seed, _node.castables.begin(), _node.castables.end());
    myHash::hash_range(seed, _node.magicTome.castables.begin(),
                       _node.magicTome.castables.end());
    return seed;
  }
};

constexpr size_t _MAX_NODES_BFS{2 * _MAX_BREADTH_BFS * _MAX_DEPTH_BFS};
static NodePlayer nodesBFS[_NB_BREWS * _MAX_NODES_BFS];
static size_t nodeIdx = 0;

vector<pair<Action, int>> unrollBFS(int idx) {
  vector<pair<Action, int>> res;
  while (nodesBFS[idx].parent > 0) {
    res.push_back({nodesBFS[idx].action, nodesBFS[idx].depth});
    idx = nodesBFS[idx].parent;
  }
  return res;
}

// #######################################################
//
//                        PLAYER
//
// #######################################################

//! @brief  Player class
struct Player {
  const string name;      //!< Name of the player
  vector<Cast> casts;     //!< All the casts
  vector<bool> castables; //!< Whether or not the spells or castable

  Ingredients inventory; //!< Inventory
  int score;             //!< Score of the player

  //! @brief  C'tor
  //!
  //! @param[in] f_name The name of the player
  Player(const string &f_name)
      : name{f_name}, score{0}, inventory{0, 0, 0, 0} {}

  //! @brief  Clear the player's casts and castables
  void clear() {
    casts.clear();
    castables.clear();
  }

  //! @brief  Add a spell and update the inc_casts
  //!
  //! @param[in] f_cast      The spell to add
  //! @param[in] f_castable  Whether or not the spell is castable
  void addCast(const Cast &f_cast, const bool f_castable) {
    casts.push_back(f_cast);
    castables.push_back(f_castable);
  }

  //! @brief  Perform a BFS to create the potion `brew` by simulating
  //!         different Player status after different choices.
  //!
  //! @param[in] brew       The potion to create
  //! @param[in] maxDepth   The maximum depth of the BFS
  //! @param[in] magicTome  The magic tome from where to learn spells
  //! @param[in] verbose    Whether or not to display some texts
  //!
  //! @return The solutions
  vector<int> bfs(const Brew &brew, const int maxDepth,
                  const MagicTome &magicTome,
                  const bool verbose = false) const {
    unordered_set<int> state_set;

    // Initial state
    nodesBFS[nodeIdx].set(
        -1, {"INIT"}, castables, inventory, magicTome,
        (inventory.sum() < _MIN_INV_FOR_LEARNING ? _MAX_LEARN_SPELL_BFS : 0),
        0);

    int graphDepth = 0;
    if (!_IN_ARENA)
      cerr << "Target max: " << maxDepth << " / " << _MAX_BREADTH_BFS << endl;

    // BFS loop
    size_t start = nodeIdx++;
    while (++graphDepth < maxDepth) {
      size_t end = nodeIdx;
      size_t size = end - start;
      if (size == 0)
        break;
      if (!_IN_ARENA && verbose) {
        cerr << graphDepth - 1 << ": ==> Graph large of " << size
             << " node(s) <==" << endl;
        cerr << "start: " << start << " / end: " << end << endl;
      }

      for (int idx = start; idx < end; ++idx) {
        // Discard this branch
        if (nodesBFS[idx].inventory.sum() > _SIZE_INVENTORY)
          continue;

        size_t hashvalue = NodePlayer_hash::hash(nodesBFS[idx]);

        // State already achieved
        if (state_set.find(hashvalue) != state_set.end())
          continue;

        // Stop condition
        if (brew.isBrewable(nodesBFS[idx].inventory)) {
          // Get all the solutions at this level
          vector<int> res = {idx++};
          for (; idx < end; ++idx) {
            if (nodesBFS[idx].inventory.sum() <= _SIZE_INVENTORY &&
                brew.isBrewable(nodesBFS[idx].inventory))
              res.push_back(idx);
          }
          if (!_IN_ARENA || verbose)
            cerr << res.size() << " res out of " << size << endl;
          return res;
        }

        // It's the last turn, don't try to create children
        if (graphDepth == maxDepth || size >= _MAX_BREADTH_BFS)
          continue;

        state_set.insert(hashvalue);

        bool doRest = false;

        // If learning is still allowed
        if (nodesBFS[idx].hasLearnt < _MAX_LEARN_SPELL_BFS) {
          // For all learnable spells
          for (int learnableIds = 0;
               learnableIds < nodesBFS[idx].magicTome.learns.size();
               learnableIds++) {
            // If we can learn the spell
            if (learnableIds <= nodesBFS[idx].inventory[0]) {
              // Create a child node in this branch,...
              nodesBFS[nodeIdx].set(
                  idx, {"TODO"}, nodesBFS[idx].castables,
                  nodesBFS[idx].inventory, nodesBFS[idx].magicTome,
                  nodesBFS[idx].hasLearnt + 1, nodesBFS[idx].depth + 1);
              // ... simulate the learning of the spell
              auto [tax, castID] =
                  nodesBFS[nodeIdx].magicTome.learnSpell(learnableIds);
              nodesBFS[nodeIdx].action = {"LEARN", castID};
              nodesBFS[nodeIdx].inventory[0] +=
                  tax - learnableIds; // get the tax, and correct it
              if (nodesBFS[nodeIdx].inventory.sum() > _SIZE_INVENTORY)
                nodesBFS[nodeIdx].inventory[0] -=
                    nodesBFS[nodeIdx].inventory.sum() - _SIZE_INVENTORY;
              nodeIdx++;
            } //! ENDIF nodesBFS[idx].magicTome.learns[learnableIds].tomeIdx
              //! <= nodesBFS[idx].inventory[0]
          }   //! ENDFOR learnableIds
        }

        // For all the new learnt spells
        for (int learntIds = 0;
             learntIds < nodesBFS[idx].magicTome.castables.size();
             learntIds++) {
          // If we can use the spell
          if (nodesBFS[idx].magicTome.castables[learntIds] == 1) {
            Ingredients simulInv = nodesBFS[idx].inventory +
                                   nodesBFS[idx].magicTome.casts[learntIds].ing;

            if (simulInv.allPositiv()) {
              // This is a potential intelligent move, set it
              nodesBFS[nodeIdx].set(
                  idx, {"CAST", nodesBFS[idx].magicTome.casts[learntIds].ID, 1},
                  nodesBFS[idx].castables, simulInv, nodesBFS[idx].magicTome,
                  nodesBFS[idx].hasLearnt, nodesBFS[idx].depth + 1);
              // (this spell can't be used anymore)
              nodesBFS[nodeIdx].magicTome.castables[learntIds] = 0;
              nodeIdx++;

              // If the spell is repeatable
              if (nodesBFS[idx].magicTome.casts[learntIds].repeatable) {
                int repeat = 2;
                simulInv += nodesBFS[idx].magicTome.casts[learntIds].ing;
                while (simulInv.allPositiv()) {
                  // This is a potential intelligent move, set it
                  nodesBFS[nodeIdx].set(
                      idx,
                      {"CAST", nodesBFS[idx].magicTome.casts[learntIds].ID,
                       repeat},
                      nodesBFS[idx].castables, simulInv,
                      nodesBFS[idx].magicTome, nodesBFS[idx].hasLearnt,
                      nodesBFS[idx].depth + 1);
                  // (this spell can't be used anymore)
                  nodesBFS[nodeIdx].magicTome.castables[learntIds] = 0;
                  nodeIdx++;

                  // Repeat one more time
                  simulInv += nodesBFS[idx].magicTome.casts[learntIds].ing;
                  repeat++;
                }
              }
            }
          } //! ENDIF nodesBFS[idx].magicTome.castables[learntIds] == 1
          else {
            doRest = true;
          }
        } //! ENDFOR learntIds

        // For all the spells
        for (int castIds = 0; castIds < casts.size(); castIds++) {
          // If we can use the spell
          if (nodesBFS[idx].castables[castIds]) {
            Ingredients simulInv = nodesBFS[idx].inventory + casts[castIds].ing;
            if (simulInv.allPositiv()) {
              // This is a potential intelligent move, set it
              nodesBFS[nodeIdx].set(
                  idx, {"CAST", casts[castIds].ID, 1}, nodesBFS[idx].castables,
                  simulInv, nodesBFS[idx].magicTome, nodesBFS[idx].hasLearnt,
                  nodesBFS[idx].depth + 1);
              // (this spell can't be used anymore)
              nodesBFS[nodeIdx].castables[castIds] = false;
              nodeIdx++;

              // If the spell is repeatable
              if (casts[castIds].repeatable) {
                int repeat = 2;
                simulInv += casts[castIds].ing;
                while (simulInv.allPositiv()) {
                  // This is a potential intelligent move, set it
                  nodesBFS[nodeIdx].set(
                      idx, {"CAST", casts[castIds].ID, repeat},
                      nodesBFS[idx].castables, simulInv,
                      nodesBFS[idx].magicTome, nodesBFS[idx].hasLearnt,
                      nodesBFS[idx].depth + 1);
                  // (this spell can't be used anymore)
                  nodesBFS[nodeIdx].castables[castIds] = false;
                  nodeIdx++;

                  // Repeat one more time
                  simulInv += casts[castIds].ing;
                  repeat++;
                }
              }
            }
          } //! ENDIF node->castables[castIdx]
          else {
            doRest = true;
          }
        } //! ENDFOR castIdx

        // No tier had at least one move
        if (doRest) {
          // Simulate a rest move
          nodesBFS[nodeIdx].set(
              idx, {"REST"}, nodesBFS[idx].castables, nodesBFS[idx].inventory,
              nodesBFS[idx].magicTome, nodesBFS[idx].hasLearnt,
              nodesBFS[idx].depth + 1);
          for (int i = 0; i < nodesBFS[nodeIdx].castables.size(); ++i) {
            nodesBFS[nodeIdx].castables[i] = true;
          }
          for (int i = 0; i < nodesBFS[nodeIdx].magicTome.castables.size();
               ++i) {
            nodesBFS[nodeIdx].magicTome.castables[i] = 1;
          }
          nodeIdx++;
        }
      }
      start = end;
    }
    return {};
  }

  //! @brief  Among different simulated player states, pick a preferable action
  //!
  //! Selection priority:
  //!   1. Learn with reward
  //!   2. Learn with cost
  //!   3. Cast
  //!   4. Rest
  //!
  //! @param[in]  nodes     A vector of simulated player states
  //! @param[out] safety    Keeping two actions in advance for timeout safety
  //! @param[in]  magicTome The magic tome from where to learn
  //!
  //! @return An action choosed among the given vector
  Action choiceAction(const vector<int> &initNodes,
                      vector<pair<Action, int>> &safety,
                      const MagicTome &magicTome) const {

    vector<int> nodes = initNodes;

    while (nodesBFS[nodes[0]].depth > 1) {
      for (int &n : nodes)
        n = nodesBFS[n].parent;
    }

    int a_learn = -1;
    int a_learn_safe = -1;
    int learn_final = 0;
    int b_noLearn = -1;
    int b_noLearn_safe = -1;
    int noLearn_final = 0;

    for (int i = 0; i < initNodes.size(); ++i) {
      if (nodesBFS[nodes[i]].action.type == "LEARN") {
        int score = nodesBFS[initNodes[i]].inventory.score();
        if (score > learn_final) {
          learn_final = score;
          a_learn = nodes[i];
          a_learn_safe = initNodes[i];
        }
      } else if (nodesBFS[nodes[i]].action.type == "CAST") {
        int score = nodesBFS[initNodes[i]].inventory.score();
        if (score > noLearn_final) {
          noLearn_final = score;
          b_noLearn = nodes[i];
          b_noLearn_safe = initNodes[i];
        }
      }
    }

    if (a_learn != -1) {
      safety = unrollBFS(a_learn_safe);
      return nodesBFS[a_learn].action;
    }
    if (b_noLearn != -1) {
      safety = unrollBFS(b_noLearn_safe);
      return nodesBFS[b_noLearn].action;
    }

    safety.clear();
    return {};
  }

  //! @brief  Get the next action to do given the possible potions to create
  //!
  //! @param[in]  brews       All the possible brew to create
  //! @param[out] bestBrewID  The resulting brewID
  //! @param[out] safety      Keeping whole chain of decision
  //! @param[in]  verbose     Whether or to display some texts
  //!
  //! @return A pair of {Action, int} being the action to do and its cost
  pair<Action, int> getAction(const vector<Brew> &brews, int &bestBrewID,
                              const MagicTome &magicTome, const int oppScore,
                              vector<pair<Action, int>> &safety,
                              const bool verbose = false) const {
    if (verbose)
      cerr << "Computing the costs" << endl;

    vector<int> bestNodes;
    int bestCost = 100;
    int bestPrice = 0;

    if (nodeIdx > 6 * _MAX_BREADTH_BFS * _MAX_DEPTH_BFS)
      nodeIdx = 0;

    double ratio = 0;

    auto t1 = chrono::high_resolution_clock::now();
    for (int i = 0; i < brews.size(); ++i) {
      if (verbose)
        cerr << brews[i] << endl;

      auto safetime = chrono::duration_cast<chrono::duration<double>>(
          chrono::high_resolution_clock::now() - t1);
      if (safetime.count() * 1000 > 35)
        break;

      vector<int> nodes;
      if (verbose) {
        auto start = chrono::high_resolution_clock::now();
        nodes = bfs(brews[i], _MAX_DEPTH_BFS, magicTome);
        auto end = chrono::high_resolution_clock::now();
        auto elapsed_time =
            chrono::duration_cast<chrono::duration<double>>(end - start);
        cerr << "Elapsed time for this brew: " << elapsed_time.count() * 1000
             << "ms" << endl;
      } else {
        nodes = bfs(brews[i], _MAX_DEPTH_BFS, magicTome);
      }

      if (verbose) {
        if (nodes.size() > 0)
          cerr << "Solution in " << nodesBFS[nodes[0]].depth << " steps" << endl
               << endl;
        else
          cerr << "No solution" << endl << endl;
      }

      if (nodes.size() > 0) {
        double tmpRatio;
        if (nodesBFS[nodes[0]].depth == 0)
          tmpRatio = brews[i].price;
        else
          tmpRatio = 1. * brews[i].price / nodesBFS[nodes[0]].depth;

        if (bestNodes.size() == 0 || tmpRatio > ratio) {
          ratio = tmpRatio;
          bestPrice = brews[i].price;
          bestNodes = nodes;
          bestBrewID = brews[i].ID;
        }
      }
    }

    if (bestNodes.size() == 0)
      return {{"REST", -1}, -1};
    else if (nodesBFS[bestNodes[0]].depth == 0)
      return {{"BREW", bestBrewID}, 0};
    else {
      int depth = nodesBFS[bestNodes[0]].depth;
      return {choiceAction(bestNodes, safety, magicTome), depth};
    }
  }

  Action getBestSoloAction() const {
    Action action;
    int bestScore = 0;
    for (int tierId = _NB_TIERS - 1; tierId >= 0; --tierId) {
      bool atLeastOne = false;
      // For all the spell that can increase this ingredient
      for (int castIds = 0; castIds < casts.size(); ++castIds) {
        // If we can use the spell
        if (castables[castIds]) {

          Ingredients simulInv = inventory;
          int repeat;
          if (casts[castIds].repeatable) {
            repeat = -1;
            while (simulInv.allPositiv() && simulInv.sum() <= _SIZE_INVENTORY) {
              simulInv += casts[castIds].ing;
              repeat++;
            }
          } else {
            simulInv += casts[castIds].ing;
            repeat = (int)(simulInv.allPositiv() &&
                           simulInv.sum() <= _SIZE_INVENTORY);
          }
          if (repeat == 0)
            continue;

          atLeastOne = true;
          for (int rep = repeat; rep > 0; --rep) {
            int score = (inventory + rep * casts[castIds].ing).score();
            if (score > bestScore) {
              action = {"CAST", casts[castIds].ID, rep};
              bestScore = score;
            }
          }
        } //! ENDIF node->castables[castIdx]
      }   //! ENDFOR castIdx
      if (atLeastOne)
        break;
    } //! ENDFOR tierId
    return action;
  }

  //! @brief  Prints out the status of the Player
  void debug() const {
    cerr << " --- " << name << " ---" << endl;
    cerr << inventory << endl;
    cerr << "Spells:";
    for (int i = 0; i < casts.size(); ++i)
      cerr << " " << castables[i];
    cerr << endl;
    cerr << " --------------" << endl;
  }
};

// #######################################################
//
//                        CHOICES
//
// #######################################################

//! @brief  Basic learning strategy
//!
//! @param[in]  idxRound  The current round
//! @param[in]  nbTier0   The amount of tier-0 in the inventory
//! @param[out] learnID   The resulting ID of the spell to learn
//!
//! @return whether or not to learn a spell
bool choiceLearn(const Player &player, const MagicTome &magicTome,
                 int &learnID) {
  if (player.casts.size() < _MAX_LEARN_SPELL) {
    // 1- Try to look for `positive only` spells
    for (const Learn &learn : magicTome.learns) {
      if (learn.cast.ing.allPositiv() && learn.tomeIdx <= player.inventory[0]) {
        learnID = learn.cast.ID;
        return true;
      }
    }
    // 2- Get the best spell from the scores
    int bestScore = -100;
    int bestIdx = 7;
    for (const Learn &learn : magicTome.learns) {
      if (learn.tomeIdx <= player.inventory[0]) {
        const int score = learn.cast.ing.score() + learn.tax - learn.tomeIdx;
        if (score > bestScore ||
            (score == bestScore && learn.tomeIdx < bestIdx)) {
          learnID = learn.cast.ID;
          bestIdx = learn.tomeIdx;
          bestScore = score;
        }
      }
    }
    return bestScore != -100;
  }
  return false;
}

// #######################################################
//
//                         MAIN
//
// #######################################################

int main() {
  Player hero(" HERO ");
  Player vilain("VILAIN");

  int idxTour = 0;

  vector<pair<Action, int>> safety;
  int depthSafety = 2;
  int safetyBrew = -1;

  while (1) {
    int actionCount; // the number of spells and recipes in play
    cin >> actionCount;

    hero.clear();
    vilain.clear();

    vector<Brew> brews;
    MagicTome magicTome;

    for (int i = 0; i < actionCount; i++) {
      int actionId;      // the unique ID of this spell or recipe
      string actionType; // CAST, OPPONENT_CAST, LEARN, BREW
      int delta0, delta1, delta2, delta3; // ingredients change
      int price;                 // the price in rupees if this is a potion
      int tomeIndex, taxCount;   // Tome info
      bool castable, repeatable; // Spell info
      cin >> actionId >> actionType >> delta0 >> delta1 >> delta2 >> delta3 >>
          price >> tomeIndex >> taxCount >> castable >> repeatable;

      if (actionType == "CAST") {
        hero.addCast({actionId, {delta0, delta1, delta2, delta3}, repeatable},
                     castable);
      } else if (actionType == "OPPONENT_CAST") {
        vilain.addCast({actionId, {delta0, delta1, delta2, delta3}, repeatable},
                       castable);
      } else if (actionType == "BREW") {
        brews.push_back({actionId, price, {delta0, delta1, delta2, delta3}});
      } else if (actionType == "LEARN") {
        magicTome.addLearn(
            {{actionId, {delta0, delta1, delta2, delta3}, repeatable},
             tomeIndex,
             taxCount});
      }
    }

    cin >> hero.inventory[0] >> hero.inventory[1] >> hero.inventory[2] >>
        hero.inventory[3] >> hero.score;

    cin >> vilain.inventory[0] >> vilain.inventory[1] >> vilain.inventory[2] >>
        vilain.inventory[3] >> vilain.score;

    // Try to learn a spell
    int learnID;
    if (choiceLearn(hero, magicTome, learnID)) {
      cout << "LEARN " << learnID
           << (idxTour < 2 ? " GL ! HF :) \U0001F52E" : "") << endl;
    } else {
      // Sort the potions
      //sort(brews.begin(), brews.end(), Brew::priceSorterInc);

      bool safeBrewFound = false;
      for (const Brew &brew : brews) {
        if (brew.ID == safetyBrew) {
          safeBrewFound = true;
        }
      }
      // Reset the safety
      if (!safeBrewFound) {
        safety.clear();
        safetyBrew = -1;
      }

      // Do the BFS
      auto start = chrono::high_resolution_clock::now();
      pair<Action, int> pairHero = hero.getAction(
          brews, safetyBrew, magicTome, vilain.score, safety, !_IN_ARENA);
      auto end = chrono::high_resolution_clock::now();
      auto elapsed_time =
          chrono::duration_cast<chrono::duration<double>>(end - start);
      cerr << "Elapsed time hero: " << elapsed_time.count() * 1000 << "ms"
           << endl;
      cerr << "Best cost hero: " << pairHero.second << " for brew" << safetyBrew
           << endl
           << endl;

      if (safety.size() > 0) {
        for (const auto &p : safety)
          cerr << p.first << endl;
      }

      // The BFS didn't work -> runtime safety on edge cases
      if (pairHero.second == -1) {
        cerr << "Safety:" << endl;
        cerr << "  With size " << safety.size() << endl;
        cerr << "  Depth " << max(int(safety.size()) - depthSafety, 0) << endl;
        if (safety.size() > 1) {
          int idx = max(int(safety.size()) - depthSafety, 0);
          if (safety[idx].first.type == "REST") {
            cout << "REST \U0001F62A " << elapsed_time.count() * 1000
                 << "ms SAFE" << endl;
          } else if (safety[idx].first.type == "LEARN") {
            bool safeLearnFound = false;
            for (const Learn &learn : magicTome.learns) {
              if (learn.cast.ID == safety[idx].first.ID) {
                safeLearnFound = true;
              }
            }
            if (safeLearnFound) {
              cout << "LEARN " << safety[idx].first.ID
                   << (!_IN_ARENA
                           ? " " + to_string(elapsed_time.count() * 1000) +
                                 "ms SAFE"
                           : "")
                   << endl;
            } else {
              Action action = hero.getBestSoloAction();
              if (action.type == "REST")
                cout << "REST \U0001F62A " << elapsed_time.count() * 1000
                     << "ms DOUBLE SAFE" << endl;
              else
                cout << action.type << " " << action.ID
                     << (action.type == "CAST" ? " " + to_string(action.repeat)
                                               : "")
                     << (!_IN_ARENA
                             ? " " + to_string(elapsed_time.count() * 1000) +
                                   "ms DOUBLE SAFE"
                             : "")
                     << endl;
            }
          } else {
            cout << safety[idx].first.type << " " << safety[idx].first.ID
                 << (safety[idx].first.type == "CAST"
                         ? " " + to_string(safety[idx].first.repeat)
                         : "")
                 << (!_IN_ARENA ? " " + to_string(elapsed_time.count() * 1000) +
                                      "ms SAFE"
                                : "")
                 << endl;
          }
          depthSafety++;
        } else {
          Action action = hero.getBestSoloAction();
          if (action.type == "REST")
            cout << "REST \U0001F62A " << elapsed_time.count() * 1000
                 << "ms DOUBLE SAFE" << endl;
          else
            cout << action.type << " " << action.ID
                 << (action.type == "CAST" ? " " + to_string(action.repeat)
                                           : "")
                 << (!_IN_ARENA ? " " + to_string(elapsed_time.count() * 1000) +
                                      "ms DOUBLE SAFE"
                                : "")
                 << endl;
        }
        // The BFS worked
      } else {
        // Reset the safety
        depthSafety = 2;
        if (pairHero.first.type == "BREW")
          safety.clear();

        if (pairHero.first.type == "REST")
          cout << "REST \U0001F62A"
               << (!_IN_ARENA
                       ? " " + to_string(elapsed_time.count() * 1000) + "ms"
                       : "")
               << endl;
        else
          cout << pairHero.first.type << " " << pairHero.first.ID
               << (pairHero.first.type == "CAST"
                       ? " " + to_string(pairHero.first.repeat)
                       : "")
               << (!_IN_ARENA
                       ? " " + to_string(elapsed_time.count() * 1000) + "ms"
                       : "")
               << endl;
      }
    }
    idxTour++;
  }
}