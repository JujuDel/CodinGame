#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

//#define DEBUG

struct Coord {
    int x;
    int y;
};

bool isValid(const vector<vector<int>>& grid, const int x, const int y) {
    if (y < 0 || y >= grid.size() || x < 0 || x >= grid[0].size())
        return false;
    
    return grid[y][x] == -1;
}

void connected_component_analysis(vector<vector<int>>& grid) {
    for (int y = 0; y < grid.size(); ++y) {
        for (int x = 0; x < grid[0].size(); ++x) {
            if (grid[y][x] != -1)
                continue;
            
            int count = 0;
            queue<Coord> myQueue;
            myQueue.push({x, y});
            vector<Coord> mem;
            while (myQueue.size() > 0) {
                for (int i = 0; i < myQueue.size(); i++) {
                    const Coord c = myQueue.front();
                    myQueue.pop();

                    if (!isValid(grid, c.x, c.y))
                        continue;

                    count++;
                    grid[c.y][c.x] = 1;
                    mem.push_back(c);

                    if (isValid(grid, c.x + 1, c.y))
                        myQueue.push({c.x + 1, c.y});
                    if (isValid(grid, c.x - 1, c.y))
                        myQueue.push({c.x - 1, c.y});
                    if (isValid(grid, c.x, c.y + 1))
                        myQueue.push({c.x, c.y + 1});
                    if (isValid(grid, c.x, c.y - 1))
                        myQueue.push({c.x, c.y - 1});
                }
            }

            for (const Coord& c : mem) {
                grid[c.y][c.x] = count;
            }
        }
    }
}

int main()
{
    int L;
    cin >> L; cin.ignore();
    int H;
    cin >> H; cin.ignore();

    vector<vector<int>> grid;
    for (int i = 0; i < H; i++) {
        string row;
        getline(cin, row);
        vector<int> rowVec;
        for (const auto& c : row) {
#ifdef DEBUG
            cerr << " " << c;
#endif
            if (c == '#') {
                rowVec.push_back(0);
            }
            else {
                rowVec.push_back(-1);
            }
        }
#ifdef DEBUG
        cerr << endl;
#endif
        grid.push_back(rowVec);
    }

    connected_component_analysis(grid);

#ifdef DEBUG
    cerr << endl;
    for (const vector<int>& row : grid) {
        for (const int & c : row) {
            cerr << " " << c;
        }
        cerr << endl;
    }
    cerr << endl << endl;
#endif

    int N;
    cin >> N; cin.ignore();
    for (int i = 0; i < N; i++) {
        int X;
        int Y;
        cin >> X >> Y; cin.ignore();
        cout << grid[Y][X] << endl;
    }
}