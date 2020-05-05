#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <chrono>

#define DEBUG

using namespace std;


//===================================================
//! DEBUG
//===================================================

//--------------------------------------------------------------------
//! @brief  Prints out the grid content.
//!
//! @tparam     Graph   The graph type.
//! @param[in]  grid    The grid to print out.
//--------------------------------------------------------------------
template<typename Graph>
void debugGrid(const typename Graph::grid_t& grid) {
    for (int i = grid.size() - 1; i >= 0; --i) {
        const auto& g{ grid[i] };
        for (const auto& c : g) {
            cerr << int(c) << " ";
        }
        cerr << endl;
    }
    cerr << endl;
}

//--------------------------------------------------------------------
//! @brief  Mark nodePtr and all its parents on the grid with the
//!         same dimension as 'grid'.
//!
//! @tparam     Graph   The graph type.
//! @param[in]  grid    The grid to take the dimension from.
//! @param[in]  nodePtr The last node of the graph to mark.
//--------------------------------------------------------------------
template<typename Graph>
void debugGrid(
    const typename Graph::grid_t& grid,
    const typename Graph::Node* nodePtr)
{
    typedef typename Graph::Node Node;

    vector<vector<char>> nGrid;
    for (const auto& v : grid) {
        nGrid.push_back( vector<char>(v.size(), '.') );
    }

    while (nodePtr) {
        nGrid[nodePtr->pos.y][nodePtr->pos.x] = 'X';
        nodePtr = nodePtr->parent;
    }

    for (int i = nGrid.size() - 1; i >= 0; --i) {
        const vector<char>& v{ nGrid[i] };
        for (const char& c : v) {
            cerr << c << " ";
        }
        cerr << endl;
    }
}


//===================================================
//! CUSTOM PRIORITY QUEUE
//===================================================

//--------------------------------------------------------------------
//! @brief  Struct used in our priority queue with pairs.
//!
//! With this struct, the pair with the smallest first member will be
//! on the top of the priority queue.
//!
//! @tparam     T           A type.
//! @tparam     priority_t  The type of the values to sort with.
//--------------------------------------------------------------------
template<typename T, typename priority_t>
struct CompareByGreatestPriority {
    //--------------------------------------------------------------------
    //! @brief  Overload the operator()
    //!
    //! @param[in]  a   One pair.
    //! @param[in]  b   Another pair.
    //!
    //! @return a.first > b.first.
    //--------------------------------------------------------------------
    constexpr bool operator()(const pair<priority_t, T>& a,
                              const pair<priority_t, T>& b) const noexcept
    {
        return a.first > b.first;
    }
};

//--------------------------------------------------------------------
//! @brief  Struct to use priority queue with pairs.
//!
//! The pair with the smallest first member will be on the top of the
//! priority queue.
//!
//! @tparam     T           A type.
//! @tparam     priority_t  The type of the values to sort with.
//--------------------------------------------------------------------
template<typename T, typename priority_t>
struct PriorityQueue {
    typedef pair<priority_t, T> PQElement;  //!< The type of the pair in the priority_queue
    priority_queue<PQElement, vector<PQElement>, CompareByGreatestPriority<T, priority_t>> elements;  //!< The priority_queue

    //--------------------------------------------------------------------
    //! @brief  Check if the priority_queue is empty.
    //!
    //! @return True if empty, False otherwise.
    //--------------------------------------------------------------------
    inline bool empty() const {
        return elements.empty();
    }

    //--------------------------------------------------------------------
    //! @brief  Add an element in the priority_queu
    //!
    //! @param[in]  item   The item of the pair.
    //! @param[in]  cost   The cost of the item.
    //--------------------------------------------------------------------
    inline void put(T item, priority_t cost) {
        elements.emplace( make_pair(cost, item) );
    }

    //--------------------------------------------------------------------
    //! @brief  Remove the element with the smallest cost from the
    //!         priority_queue and return its item.
    //!
    //! @return The item with the smallest cost.
    //--------------------------------------------------------------------
    T get() {
        T best_item = elements.top().second;
        elements.pop();
        return best_item;
    }
};


//===================================================
//! GRAPH STRUCT AND INNER STRUCT
//===================================================

//--------------------------------------------------------------------
//! @brief  Main struct
//!
//! Define the graph, the grid, the node and how to access and use them.
//--------------------------------------------------------------------
struct DontPanicGraph {

    //! Enum for cells definition in the grid
    enum Cell {
        EMPTY    = 0,   //!< Empty cell
        ELEVATOR = 1,   //!< Elevator on the cell
        GOAL     = 2,   //!< Goal
        TRAP     = 3    //!< This cell is trap or leads to a trap
    };

    typedef int64_t              cost_t;    //!< Cost type
    typedef vector<vector<Cell>> grid_t;    //!< Grid type

    //--------------------------------------------------------------------
    //! @brief  Location struct with operator definition
    //--------------------------------------------------------------------
    struct Location {
        int64_t x;
        int64_t y;

        constexpr bool operator == (const Location& a) const {
            return a.x == x && a.y == y;
        }

        constexpr bool operator != (const Location& a) const {
            return !(a == *this);
        }

        friend ostream &operator<<(ostream &out, const Location &L) {
            out << "(" << L.x << ", " << L.y << ")";
            return out;
        }
    };

    //--------------------------------------------------------------------
    //! @brief  Node definition for inner graph
    //--------------------------------------------------------------------
    struct Node {
        const   Location        pos;        //!< Current position
        const   cost_t          cost;       //!< Cost to arrive here
        const   int8_t          direction;  //!< Last direction taken to arrive here: either -1 or 1
        const   u_int32_t       nbElev;     //!< Nb of additional optim elevators left
        const   Node*   const   parent;     //!< Parent's node
                vector<Node*>   children;   //!< Next nodes

        //--------------------------------------------------------------------
        //! @brief  Delete all the children and clear the vector
        //--------------------------------------------------------------------
        void clearChildren() {
            for (Node* child : children)
            {
                delete child;
            } 
            children.clear();
        }

        //--------------------------------------------------------------------
        //! @brief  C'tor of Node
        //!
        //! @param[in]  f_pos       The location of the node in the grid.
        //! @param[in]  f_cost      The cost of the node.
        //! @param[in]  f_dir       The direction of the move (-1 or 1)
        //! @param[in]  f_nbElev    The number of elevator creation allowed for
        //!                         the subgraph starting with this node.
        //! @param[in]  f_parent    The node who created this node.
        //--------------------------------------------------------------------
        Node(const Location& f_pos, const cost_t f_cost, const int8_t& f_dir, const u_int32_t f_nbElev, const Node* const f_parent) :
            pos{ f_pos },
            cost{ f_cost },
            direction{ f_dir },
            nbElev{ f_nbElev },
            parent{ f_parent }
        {}

        //--------------------------------------------------------------------
        //! @brief  D'tor of Node to avoid memory leak
        //--------------------------------------------------------------------
        ~Node() {
            clearChildren();
        }

        //--------------------------------------------------------------------
        //! @brief  Create the next node
        //!
        //! A next node can be:
        //!
        //!  - a node representing the UPPER CELL if the current cell is an
        //! elevator ;
        //!
        //!  - a node representing the UPPER CELL if the current cell is not an
        //! elevator and if there are no elevator on the current line ;
        //!
        //!  - a node representing the UPPER CELL if the current cell is not an
        //! elevator, if there is at least 1 elevator on the current line, the
        //! subgraph can create an elevator and if the upper cell is either
        //! an elevator or the goal ;
        //!
        //!  - a node going in the other direction if the parent was on an
        //! elevator ;
        //!
        //!  - a node going in the same direction ;
        //!
        //! For all those cells, if the next node won't represent a TRAP or a
        //! cell out of the grid.
        //!
        //! @param[in]  grid            The mapped grid.
        //! @param[in]  goal            The goal point. A node should never
        //!                             be above it.
        //! @param[in]  elevatorOnLine  A vector maping if there is at least
        //!                             one elevator on each line of the grid.
        //--------------------------------------------------------------------
        void giveBirth(const grid_t& grid, const Location& goal, const vector<bool>& elevatorOnLine) {
            clearChildren();
            if (grid[pos.y][pos.x] == Cell::ELEVATOR) {
                if (pos.y + 1 <= goal.y && grid[pos.y + 1][pos.x] != Cell::TRAP) {
                    // It takes an elevator
                    children.push_back( new Node({pos.x, pos.y + 1}, cost + 1, direction, nbElev, this) );
                }
            }
            else {
                if (pos.y + 1 <= goal.y && grid[pos.y + 1][pos.x] != Cell::TRAP) {
                    // It creates a mandatory elevator
                    if (!elevatorOnLine[pos.y]) {
                        children.push_back( new Node({pos.x, pos.y + 1}, cost + 4, direction, nbElev, this) );
                    }
                    // It creates an elevator while one is already on the line
                    else if (nbElev > 0 && (grid[pos.y + 1][pos.x] == Cell::ELEVATOR || grid[pos.y + 1][pos.x] == Cell::GOAL)) {
                        children.push_back( new Node({pos.x, pos.y + 1}, cost + 4, direction, nbElev - 1, this) );
                    }
                    
                }

                // Straight forward
                if (0 <= pos.x + direction < grid[0].size() && grid[pos.y][pos.x + direction] != Cell::TRAP) {
                    children.push_back( new Node({pos.x + direction, pos.y}, cost + 1, direction, nbElev, this) );
                }

                // Change direction
                if (0 <= pos.x - direction < grid[0].size() && grid[pos.y][pos.x - direction] != Cell::TRAP) {
                    if (!parent || parent->pos.y < this->pos.y) {
                        children.push_back( new Node({pos.x - direction, pos.y}, cost + 4, -direction, nbElev, this) );
                    }
                }
            }
        }
    };


    Node root;  //<! Root node of the inner graph.

    //--------------------------------------------------------------------
    //! @brief  C'tor of DontPanicGraph
    //!
    //! Initialize the root node with:
    //!     Location in the grid............: f_start
    //!     Cost............................: 1
    //!     Direction.......................: 1
    //!     Number of additional elevator...: f_nbElev
    //!     Parent..........................: nullptr
    //!
    //! @param[in]  f_start     The location of the root node in the grid.
    //! @param[in]  f_nbElev    The number of elevator creation allowed for
    //!                         in the graph. 
    //--------------------------------------------------------------------
    DontPanicGraph(const Location& f_start, const u_int32_t f_nbElev) :
        root{ f_start, 1, 1, f_nbElev, nullptr }
    {
    }

    //--------------------------------------------------------------------
    //! @brief  Basic heuristic for A* search - Euclidean distance
    //!
    //! @param[in]  from    A location in the grid.
    //! @param[in]  to      A location in the grid.
    //!
    //! @return The euclidean distance between from and to.
    //--------------------------------------------------------------------
    inline static const cost_t heuristic(const Location& from, const Location& to) {
        return abs(from.x - to.x) + abs(from.y - to.y);
    }
};


//===================================================
//! A* PATH FINDING ALGORITHM
//===================================================

//--------------------------------------------------------------------
//! @brief  A* path finding.
//!
//! @tparam     Graph           The type of the graph.
//! @param[in]  root            The root node of the graph to perform
//!                             the A* search on. 
//! @param[in]  grid            The mapped grid.
//! @param[in]  goal            The goal location.
//! @param[in]  maxRound        The maximum number of round allowed.
//! @param[in]  elevatorOnLine  A vector maping if there is at least
//!                             one elevator on each line of the grid. 
//! @param[in]  mandatoryElev   The number of line in the grid that
//!                             doesn't have any elevators.
//!
//! @return A node with the location on the goal.
//--------------------------------------------------------------------
template<typename Graph>
const typename Graph::Node* a_star_search(
    typename Graph::Node& root,
    const typename Graph::grid_t& grid,
    const typename Graph::Location& goal,
    const u_int32_t maxRound,
    const vector<bool>& elevatorOnLine,
    const u_int32_t mandatoryElev)
{
    typedef typename Graph::Location Location;
    typedef typename Graph::cost_t cost_t;
    typedef typename Graph::Node Node;

    // Initialize the priority_queue
    PriorityQueue<Node*, cost_t> priorityQueue;
    priorityQueue.put(&root, 0);

    while (!priorityQueue.empty()) {
        // Get the Node with the smallest cost
        Node* current{ priorityQueue.get() };

        // Exit condition
        if (current->pos == goal) {
            return current;
        }

        // Create the children
        current->giveBirth(grid, goal, elevatorOnLine);
        for (Node* child : current->children) {
            // If the safetyCost is not too high, add the child in the priority queue
            cost_t safetyCost{ child->cost + Graph::heuristic(child->pos, goal) };
            if (safetyCost <= maxRound)
            {
                priorityQueue.put(child, child->cost);
            }
        }
    }

    throw string("No solution found with\n") +
          "  - A maximum of " + to_string(maxRound) + " turn(s) ;\n" +
          "  - " + to_string(root.nbElev + mandatoryElev) + " additional elevator(s) splited as:\n" +
          "     o " + to_string(root.nbElev) + " elevator(s) for an optimized path ;\n" +
          "     o " + to_string(mandatoryElev) + " mandatory creation.";
}

//--------------------------------------------------------------------
//! @brief  Reconstruct the best series of action from the A* search.
//!
//! @tparam     Graph   The type of the graph.
//! @param[in]  grid    The mapped grid.
//! @param[in]  node    The result node of the A* search.
//!
//! @return The best series of actions to follow.
//--------------------------------------------------------------------
template<typename Graph>
const vector<string> reconstruct_path(const typename Graph::grid_t grid, const typename Graph::Node* node)
{
    typename Graph::Location Location;

    vector<string> actions;

    // For each node, compare it with its parent
    while (node->parent) {

        string action;
        if (node->pos.y != node->parent->pos.y) {
            if (grid[node->parent->pos.y][node->parent->pos.x]) {
                // Just taking an elevator
                action = "WAIT";
            }
            else {
                // Creation of an elevator
                action = "ELEVATOR";
            }
        }
        else if (node->direction != node->parent->direction) {
            // Block the way
            action = "BLOCK";
        }
        else {
            // Continue straigt forward
            action = "WAIT";
        }

        // The leading clone is destroyed
        //   add the latency for the next one the reach its pos
        if (action != "WAIT") {
            actions.push_back("WAIT");
            actions.push_back("WAIT");
            actions.push_back("WAIT");
        }
        actions.push_back(action);

        node = node->parent;
    }

    // We went from goal to start, so reverse it
    reverse(actions.begin(), actions.end());
    return actions;
}

//===================================================
//! GRID PREPARATION
//===================================================

//--------------------------------------------------------------------
//! @brief  Mark the traps on the grid and map the lines with/without
//!         an elevator.
//!
//! A cell is a TRAP if from it is no longer possible to reach the goal.
//! Therefore, an elevator considered as trap is no longer considered as
//! an elevator.
//!
//! @tparam     Graph   The type of the graph.
//! @param[in]  grid    The mapped grid.
//! @param[in]  eX      The x location of the goal.
//! @param[in]  eY      The y location of the goal.
//!
//! @return A pair composed of a mapping of the line with/without an
//!         elevator and total number of line with at least one.
//--------------------------------------------------------------------
template<typename Graph>
const pair<vector<bool>, u_int32_t> filterTraps(typename Graph::grid_t& grid, const u_int32_t eX, const u_int32_t eY) {
    const u_int32_t w{ grid[0].size() };

    // Start indexes of the traps left and right wisely
    int idxPrevTrapL{ -1 };
    int idxPrevTrapR{ w };

    // Find the start indexes
    for (int i = eX - 1; i > 0; --i) {
        if (grid[eY][i] == Graph::ELEVATOR) {
            idxPrevTrapL = i;
            break;
        }
    }
    for (size_t i = eX + 1; i < w; ++i) {
        if (grid[eY][i] == Graph::ELEVATOR) {
            idxPrevTrapR = i;
            break;
        }
    }
    
    // Update the eY-th line
    for (int i = idxPrevTrapL; i >= 0; --i) {
        grid[eY][i] = Graph::TRAP;
    }
    for (size_t i = idxPrevTrapR; i < w; ++i) {
        grid[eY][i] = Graph::TRAP;
    }

    for (int y = eY - 1; y >= 0; --y) {
        int idxTrapL{ -1 };
        int idxTrapR{ w };

        // Find the start indexes of the y-th line
        for (int i = idxPrevTrapL; i >= 0; --i) {
            if (grid[y][i] == Graph::ELEVATOR) {
                idxTrapL = i;
                break;
            }
        }
        for (size_t i = idxPrevTrapR; i < w; ++i) {
            if (grid[y][i] == Graph::ELEVATOR) {
                idxTrapR = i;
                break;
            }
        }

        // No trap on the y-th line
        if (idxTrapL < 0 && idxTrapR >= w) {
            break;
        }

        // Update the y-th line
        for (int i = idxTrapL; i >= 0; --i) {
            grid[y][i] = Graph::TRAP;
        }
        for (size_t i = idxTrapR; i < w; ++i) {
            grid[y][i] = Graph::TRAP;
        }

        idxPrevTrapL = idxTrapL;
        idxPrevTrapR = idxTrapR;
    }
    u_int32_t count{ 0 };
    vector<bool> elevatorOnLine(eY, false);
    for (size_t y = 0; y < eY; ++y) {
        for (size_t x = 0; x < w; ++x) {
            if (grid[y][x] == Graph::ELEVATOR) {
                elevatorOnLine[y] = true;
                count++;
                break;
            }
        }
    }
    return {elevatorOnLine, count};
}


//===================================================
//! MAIN
//===================================================

int main()
{
    int nbFloors;               // number of floors
    int width;                  // width of the area
    int nbRounds;               // maximum number of rounds
    int exitFloor;              // floor on which the exit is found
    int exitPos;                // position of the exit on its floor
    int nbTotalClones;          // number of generated clones
    int nbAdditionalElevators;  // number of additional elevators
    int nbElevators;            // number of elevators
    cin >> nbFloors >> width >> nbRounds >> exitFloor >> exitPos >> nbTotalClones >> nbAdditionalElevators >> nbElevators; cin.ignore();

#ifdef DEBUG
    chrono::high_resolution_clock::time_point time = chrono::high_resolution_clock::now(); 
#endif

    // Grid which contains all the map and for each x, y:
    //   - True if there is an elevator
    //   - False otherwise
    DontPanicGraph::grid_t grid;
    for (size_t i = 0; i < nbFloors; ++i) {
        grid.push_back( vector<DontPanicGraph::Cell>(width, DontPanicGraph::EMPTY) );
    }
    grid[exitFloor][exitPos] = DontPanicGraph::GOAL;

    for (int i = 0; i < nbElevators; ++i) {
        int elevatorFloor;  // floor on which this elevator is found
        int elevatorPos;    // position of the elevator on its floor
        cin >> elevatorFloor >> elevatorPos; cin.ignore();

        grid[elevatorFloor][elevatorPos] = DontPanicGraph::ELEVATOR;
    }

    // Find the potential traps, mark them and find the line with/without an elevator
    const pair<vector<bool>, u_int32_t> elevatorsMissing{ filterTraps<DontPanicGraph>(grid, exitPos, exitFloor) };
    const vector<bool> elevatorOnLine{ elevatorsMissing.first };
    const u_int32_t nbMandatoryCreation{ elevatorOnLine.size() - elevatorsMissing.second };

#ifdef DEBUG
    debugGrid<DontPanicGraph>(grid);
#endif

    // vector of actions to perform
    vector<string> actions;
    // idx-th action to perform
    size_t idx{ 0 };

    // Game loop
    while (1) {
        int cloneFloor;     // floor of the leading clone
        int clonePos;       // position of the leading clone on its floor
        string direction;   // direction of the leading clone: LEFT or RIGHT
        cin >> cloneFloor >> clonePos >> direction; cin.ignore();

        // First execution, we now have the start position
        if (idx == 0) {
            try {
                if (nbAdditionalElevators < nbMandatoryCreation) {
                    throw "Impossible puzzle\n" + to_string(nbAdditionalElevators) + " additional elevators allowed" +
                            " but " + to_string(nbMandatoryCreation) + " are mandatory to complete it!";
                }

                // Create a graph used for a* path finding
                DontPanicGraph dontPanic({clonePos, cloneFloor}, nbAdditionalElevators - nbMandatoryCreation);
#ifdef DEBUG
                std::chrono::duration<float, std::milli> dur1 = chrono::high_resolution_clock::now() - time;
                time = chrono::high_resolution_clock::now();
#endif
                // Execute a* path finding
                const DontPanicGraph::Node* const goalNode{ a_star_search<DontPanicGraph>(
                    dontPanic.root, grid, {exitPos, exitFloor}, nbRounds, elevatorOnLine, nbMandatoryCreation) };
#ifdef DEBUG
                std::chrono::duration<float, std::milli> dur2 = chrono::high_resolution_clock::now() - time;
                time = chrono::high_resolution_clock::now();
                debugGrid<DontPanicGraph>(grid, goalNode);
#endif
                // Construct the actions from the best path
                actions = reconstruct_path<DontPanicGraph>(grid, goalNode);
                // Path is ready, do wait until the end
                actions.insert(actions.end(), nbRounds - actions.size() + 1, "WAIT");

#ifdef DEBUG
                std::chrono::duration<float, std::milli> dur3 = chrono::high_resolution_clock::now() - time;
                cerr << endl;
                cerr << "Init, Traps & Elevator preparation:... " << dur1.count() << " ms" << endl;
                cerr << "A* search:............................ " << dur2.count() << " ms" << endl; 
                cerr << "Path reconstruction:.................. " << dur3.count() << " ms" << endl;
#endif
            } catch (const string& msg) {
                cerr << "ERROR: " << msg << endl;
            }
        }

        // Execute the idx-th action
        cout << actions[idx++] << endl;
    }
}