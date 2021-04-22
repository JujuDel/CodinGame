#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

struct Coord {
    int x;
    int y;

    friend std::ostream& operator<<(std::ostream& os, const Coord& obj);
};
std::ostream& operator<<(std::ostream& os, const Coord& obj) {
    os << obj.x << " " << obj.y;
    return os;
}


typedef vector<vector<char>> Grid;
typedef vector<vector<int>> Heatmap;

bool sortByFirst(const pair<int, Coord> &a, const pair<int, Coord> &b) {
    return (a.first > b.first);
}

void printGrid(const Grid& grid) {
    cerr << "GRID:" << endl;
    for (const auto& row : grid) {
        for (const auto& c : row) {
            cerr << c << " ";
        }
        cerr << endl;
    }
    cerr << endl;
}
void printHeatmap(const Heatmap& grid) {
    cerr << "HEATMAP:" << endl;
    for (const auto& row : grid) {
        for (const auto& c : row) {
            cerr << (c==-1 ? "" : " ") << c << " ";
        }
        cerr << endl;
    }
    cerr << endl;
}

Heatmap constructHeatmap(const Grid& grid, Heatmap& res) {
    for (int j = 0; j < grid.size(); ++j) {
        for (int i = 0; i < grid[j].size(); ++i) {
            if (grid[j][i] == '#')
                res[j][i] = -1;
            if (grid[j][i] == '@') {
                // NORTH
                int d = 0;
                while (j + d >= 0 && d >= -3 && grid[j + d][i] != '#') {
                    res[j + d][i]++;
                    d--;
                }
                // SOUTH
                d = 1;
                while (j + d < grid.size() && d <= 3 && grid[j + d][i] != '#') {
                    res[j + d][i]++;
                    d++;
                }
                // EAST
                d = 1;
                while (i + d < grid[0].size() && d <= 3 && grid[j][i + d] != '#') {
                    res[j][i + d]++;
                    d++;
                }
                // WEST
                d = -1;
                while (i + d >= 0 && d >= -3 && grid[j][i + d] != '#') {
                    res[j][i + d]++;
                    d--;
                }
            }
        }
    }

    return res;
}

vector<pair<int, Coord>> heatmapToSortedVec(const Grid& grid, const Heatmap& heatmap) {
    vector<pair<int, Coord>> res;

    for (int j = 0; j < heatmap.size(); ++j) {
        for (int i = 0; i < heatmap[j].size(); ++i) {
            if (heatmap[j][i] > 0 && grid[j][i] != '@' && grid[j][i] != '2' && grid[j][i] != '1')
                res.push_back({heatmap[j][i], {i, j}});
        }
    }
    sort(res.begin(), res.end(), sortByFirst);
    return res;
}

vector<Coord> placeBomb(Grid& grid, const Coord& pos) {
    vector<Coord> savings;

    //cerr << "  Bomb at " << pos << endl;

    // NORTH
    int d = -1;
    while (pos.y + d >= 0 && d >= -3 && grid[pos.y + d][pos.x] != '#') {
        if (grid[pos.y + d][pos.x] == '@') {
            savings.push_back({pos.x, pos.y + d});
            grid[pos.y + d][pos.x] = '3';
        }
        d--;
    }
    // SOUTH
    d = 1;
    while (pos.y + d < grid.size() && d <= 3 && grid[pos.y + d][pos.x] != '#') {
        if (grid[pos.y + d][pos.x] == '@') {
            savings.push_back({pos.x, pos.y + d});
            grid[pos.y + d][pos.x] = '3';
        }
        d++;
    }
    // EAST
    d = 1;
    while (pos.x + d < grid[0].size() && d <= 3 && grid[pos.y][pos.x + d] != '#') {
        if (grid[pos.y][pos.x + d] == '@') {
            savings.push_back({pos.x + d, pos.y});
            grid[pos.y][pos.x + d] = '3';
        }
        d++;
    }
    // WEST
    d = -1;
    while (pos.x + d >= 0 && d >= -3 && grid[pos.y][pos.x + d] != '#') {
        if (grid[pos.y][pos.x + d] == '@') {
            savings.push_back({pos.x + d, pos.y});
            grid[pos.y][pos.x + d] = '3';
        }
        d--;
    }

    return savings;
}

bool solve(Grid& grid, const int nbBomb, const int nbRound, const int nbFirewall, int round, vector<string>& instructions) {
    if (round >= nbRound || nbBomb <= 0)
        return nbFirewall == 0;
    if (nbFirewall <= 0)
        return true;

    Heatmap heatmap(grid.size(), vector<int>(grid[0].size(), 0));
    constructHeatmap(grid, heatmap);

    int count = 0;
    for (int j = 0; j < heatmap.size(); ++j) {
        for (int i = 0; i < heatmap[j].size(); ++i) {
            if (isdigit(grid[j][i]) && grid[j][i] != '0') {
                if (grid[j][i] != '1')
                    count++;
                grid[j][i] = char(int(grid[j][i])-1);
            }
        }
    }

    //cerr << "Round " << round << endl;
    //printHeatmap(heatmap);
    //printGrid(grid);

    for (const pair<int, Coord>& pair : heatmapToSortedVec(grid, heatmap)) {
        vector<Coord> savings = placeBomb(grid, pair.second);

        instructions[round] = to_string(pair.second.x) + " " + to_string(pair.second.y);
        if (solve(grid, nbBomb - 1, nbRound, nbFirewall - savings.size(), round + 1, instructions))
            return true;
        
        for (const Coord& pos : savings) {
            grid[pos.y][pos.x] = '@';
        }
    }
    if (count > 0) {
        instructions[round] = "WAIT";
        return solve(grid, nbBomb, nbRound, nbFirewall, round + 1, instructions);
    }
    return false;
}

int main()
{
    int width; // width of the firewall grid
    int height; // height of the firewall grid
    cin >> width >> height; cin.ignore();

    int nbFirewall = 0;
    Grid grid;

    for (int i = 0; i < height; i++) {
        string mapRow;
        getline(cin, mapRow); // one line of the firewall grid
        vector<char> row;
        for (const char& c : mapRow) {
            row.push_back(c);
            if (c == '@') nbFirewall++;
        }
        grid.push_back(row);
    }

    vector<string> instructions;
    int idx = 0;

    // game loop
    while (1) {
        int rounds; // number of rounds left before the end of the game
        int bombs; // number of bombs left
        cin >> rounds >> bombs; cin.ignore();

        if (instructions.size() == 0) {
            instructions = vector<string>(rounds, "WAIT");
            solve(grid, bombs, rounds, nbFirewall, 0, instructions);
        }

        cout << instructions[idx++] << endl;
    }
}