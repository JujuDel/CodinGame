#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdarg>


using namespace std;


template<typename T>
void debugGrid(const vector<T>& grid)
{
    cerr << "GRID: " << grid.size() << " x " << grid[0].size() << endl << endl;
    for (const auto& row : grid)
    {
        for (const auto& c : row)
        {
            cerr << c << " ";
        }
        cerr << endl;
    }
    cerr << endl;
}

template<typename T>
void solution(const vector<T>& grid)
{
    cerr << "SOLUTION:" << endl;
    for (const auto& row : grid)
    {
        for (const auto& c : row)
        {
            cout << c;
        }
        cout << endl;
    }
}

bool doIt(
    const vector<string>& field, vector<vector<char>>& grid,
    int x, int y, int icell=-1, int px=-1, int py=-1)
{
    if (icell == 0)
    {
        return false;
    }
    else if (icell > 0)
    {
        // NORTH
        if (y - icell >= 0 &&
            (field[y - icell][x] == '.' || field[y - icell][x] == 'H') &&
            (grid[y - icell][x] == '.'))
        {
            // Try to do the whole move
            int yi = y;
            grid[yi--][x] = '^';
            for (/**/; yi > y - icell; --yi)
            {
                if (grid[yi][x] != '.') break;
                if (isdigit(field[yi][x]) || field[yi][x] == 'H') break;
                grid[yi][x] = '^';
            }
            
            // If the move is not possible, cancel it
            if (yi != y - icell)
            {
                for (int yii = y; yii > yi; --yii)
                    grid[yii][x] = '.';
            }
            // If it goes into a hole
            else if (field[y - icell][x] == 'H')
            {
                // Continue looking for the next ball
                grid[y - icell][x] = 'X';
                if (doIt(field, grid, px + 1, py))
                {
                    grid[y - icell][x] = '.';
                    return true;
                }
                // If this move doesn't make it work with other balls,
                // cancel it
                else
                {
                    grid[y - icell][x] = '.';
                    for (int yii = y; yii > yi; --yii)
                        grid[yii][x] = '.';
                }
            }
            // The move must go on
            else
            {   
                // Continue the move
                if (doIt(field, grid, x, yi, icell - 1, px, py))
                {
                    return true;
                }
                // If this move doesn't make it work at the end,
                // cancel it
                else
                {
                    for (int yii = y; yii > yi; --yii)
                        grid[yii][x] = '.';
                }
            }
        }

        // SOUTH
        if (y + icell < grid.size() &&
            (field[y + icell][x] == '.' || field[y + icell][x] == 'H') &&
            (grid[y + icell][x] == '.'))
        {
            // Try to do the whole move
            int yi = y;
            grid[yi++][x] = 'v';
            for (/**/; yi < y + icell; ++yi)
            {
                if (grid[yi][x] != '.') break;
                if (isdigit(field[yi][x]) || field[yi][x] == 'H') break;
                grid[yi][x] = 'v';
            }
            
            // If the move is not possible, cancel it
            if (yi != y + icell)
            {
                for (int yii = y; yii < yi; ++yii)
                    grid[yii][x] = '.';
            }
            // If it goes into a hole
            else if (field[y + icell][x] == 'H')
            {
                // Continue looking for the next ball
                grid[y + icell][x] = 'X';
                if (doIt(field, grid, px + 1, py))
                {
                    grid[y + icell][x] = '.';
                    return true;
                }
                // If this move doesn't make it work with other balls,
                // cancel it
                else
                {
                    grid[y + icell][x] = '.';
                    for (int yii = y; yii < yi; ++yii)
                        grid[yii][x] = '.';
                }
            }
            // The move must go on
            else
            {   
                // Continue the move
                if (doIt(field, grid, x, yi, icell - 1, px, py))
                {
                    return true;
                }
                // If this move doesn't make it work at the end,
                // cancel it
                else
                {
                    for (int yii = y; yii < yi; ++yii)
                        grid[yii][x] = '.';
                }
            }
        }

        // WEST
        if (x - icell >= 0 &&
            (field[y][x - icell] == '.' || field[y][x - icell] == 'H') &&
            (grid[y][x - icell] == '.'))
        {
            // Try to do the whole move
            int xi = x;
            grid[y][xi--] = '<';
            for (/**/; xi > x - icell; --xi)
            {
                if (grid[y][xi] != '.') break;
                if (isdigit(field[y][xi]) || field[y][xi] == 'H') break;
                grid[y][xi] = '<';
            }
            
            // If the move is not possible, cancel it
            if (xi != x - icell)
            {
                for (int xii = x; xii > xi; --xii)
                    grid[y][xii] = '.';
            }
            // If it goes into a hole
            else if (field[y][x - icell] == 'H')
            {
                // Continue looking for the next ball
                grid[y][x - icell] = 'X';
                if (doIt(field, grid, px + 1, py))
                {
                    grid[y][x - icell] = '.';
                    return true;
                }
                // If this move doesn't make it work with other balls,
                // cancel it
                else
                {
                    grid[y][x - icell] = '.';
                    for (int xii = x; xii > xi; --xii)
                        grid[y][xii] = '.';
                }
            }
            // The move must go on
            else
            {   
                // Continue the move
                if (doIt(field, grid, xi, y, icell - 1, px, py))
                {
                    return true;
                }
                // If this move doesn't make it work at the end,
                // cancel it
                else
                {
                    for (int xii = x; xii > xi; --xii)
                        grid[y][xii] = '.';
                }
            }
        }

        // EAST
        if (x + icell < grid[0].size() &&
            (field[y][x + icell] == '.' || field[y][x + icell] == 'H') &&
            (grid[y][x + icell] == '.'))
        {
            // Try to do the whole move
            int xi = x;
            grid[y][xi++] = '>';
            for (/**/; xi < x + icell; ++xi)
            {
                if (grid[y][xi] != '.') break;
                if (isdigit(field[y][xi]) || field[y][xi] == 'H') break;
                grid[y][xi] = '>';
            }
            
            // If the move is not possible, cancel it
            if (xi != x + icell)
            {
                for (int xii = x; xii < xi; ++xii)
                    grid[y][xii] = '.';
            }
            // If it goes into a hole
            else if (field[y][x + icell] == 'H')
            {
                // Continue looking for the next ball
                grid[y][x + icell] = 'X';
                if (doIt(field, grid, px + 1, py))
                {
                    grid[y][x + icell] = '.';
                    return true;
                }
                // If this move doesn't make it work with other balls,
                // cancel it
                else
                {
                    grid[y][x + icell] = '.';
                    for (int xii = x; xii < xi; ++xii)
                        grid[y][xii] = '.';
                }
            }
            // The move must go on
            else
            {   
                // Continue the move
                if (doIt(field, grid, xi, y, icell - 1, px, py))
                {
                    return true;
                }
                // If this move doesn't make it work at the end,
                // cancel it
                else
                {
                    for (int xii = x; xii < xi; ++xii)
                        grid[y][xii] = '.';
                }
            }
        }
        
        return false;
    }
    else
    {
        if (x >= grid[0].size())
        {
            return doIt(field, grid, 0, y + 1);
        }
        else if (y >= grid.size())
        {
            return true;
        }
        else if (isdigit(field[y][x]))
        {
            icell = int(field[y][x]) - int('0');

            // NORTH
            if (y - icell >= 0 &&
                (field[y - icell][x] == '.' || field[y - icell][x] == 'H') &&
                (grid[y - icell][x] == '.'))
            {
                // Try to do the whole move
                int yi = y;
                grid[yi--][x] = '^';
                for (/**/; yi > y - icell; --yi)
                {
                    if (grid[yi][x] != '.') break;
                    if (isdigit(field[yi][x]) || field[yi][x] == 'H') break;
                    grid[yi][x] = '^';
                }
                
                // If the move is not possible, cancel it
                if (yi != y - icell)
                {
                    for (int yii = y; yii > yi; --yii)
                        grid[yii][x] = '.';
                }
                // If it goes into a hole
                else if (field[y - icell][x] == 'H')
                {
                    // Continue looking for the next ball
                    grid[y - icell][x] = 'X';
                    if (doIt(field, grid, x + 1, y))
                    {
                        grid[y - icell][x] = '.';
                        return true;
                    }
                    // If this move doesn't make it work with other balls,
                    // cancel it
                    else
                    {
                        grid[y - icell][x] = '.';
                        for (int yii = y; yii > yi; --yii)
                            grid[yii][x] = '.';
                    }
                }
                // The move must go on
                else
                {   
                    // Continue the move
                    if (doIt(field, grid, x, yi, icell - 1, x, y))
                    {
                        return true;
                    }
                    // If this move doesn't make it work at the end,
                    // cancel it
                    else
                    {
                        for (int yii = y; yii > yi; --yii)
                            grid[yii][x] = '.';
                    }
                }
            }

            // SOUTH
            if (y + icell < grid.size() &&
                (field[y + icell][x] == '.' || field[y + icell][x] == 'H') &&
                (grid[y + icell][x] == '.'))
            {
                // Try to do the whole move
                int yi = y;
                grid[yi++][x] = 'v';
                for (/**/; yi < y + icell; ++yi)
                {
                    if (grid[yi][x] != '.') break;
                    if (isdigit(field[yi][x]) || field[yi][x] == 'H') break;
                    grid[yi][x] = 'v';
                }
                
                // If the move is not possible, cancel it
                if (yi != y + icell)
                {
                    for (int yii = y; yii < yi; ++yii)
                        grid[yii][x] = '.';
                }
                // If it goes into a hole
                else if (field[y + icell][x] == 'H')
                {
                    // Continue looking for the next ball
                    grid[y + icell][x] = 'X';
                    if (doIt(field, grid, x + 1, y))
                    {
                        grid[y + icell][x] = '.';
                        return true;
                    }
                    // If this move doesn't make it work with other balls,
                    // cancel it
                    else
                    {
                        grid[y + icell][x] = '.';
                        for (int yii = y; yii < yi; ++yii)
                            grid[yii][x] = '.';
                    }
                }
                // The move must go on
                else
                {   
                    // Continue the move
                    if (doIt(field, grid, x, yi, icell - 1, x, y))
                    {
                        return true;
                    }
                    // If this move doesn't make it work at the end,
                    // cancel it
                    else
                    {
                        for (int yii = y; yii < yi; ++yii)
                            grid[yii][x] = '.';
                    }
                }
            }

            // WEST
            if (x - icell >= 0 &&
                (field[y][x - icell] == '.' || field[y][x - icell] == 'H') &&
                (grid[y][x - icell] == '.'))
            {
                // Try to do the whole move
                int xi = x;
                grid[y][xi--] = '<';
                for (/**/; xi > x - icell; --xi)
                {
                    if (grid[y][xi] != '.') break;
                    if (isdigit(field[y][xi]) || field[y][xi] == 'H') break;
                    grid[y][xi] = '<';
                }
                
                // If the move is not possible, cancel it
                if (xi != x - icell)
                {
                    for (int xii = x; xii > xi; --xii)
                        grid[y][xii] = '.';
                }
                // If it goes into a hole
                else if (field[y][x - icell] == 'H')
                {
                    // Continue looking for the next ball
                    grid[y][x - icell] = 'X';
                    if (doIt(field, grid, x + 1, y))
                    {
                        grid[y][x - icell] = '.';
                        return true;
                    }
                    // If this move doesn't make it work with other balls,
                    // cancel it
                    else
                    {
                        grid[y][x - icell] = '.';
                        for (int xii = x; xii > xi; --xii)
                            grid[y][xii] = '.';
                    }
                }
                // The move must go on
                else
                {   
                    // Continue the move
                    if (doIt(field, grid, xi, y, icell - 1, x, y))
                    {
                        return true;
                    }
                    // If this move doesn't make it work at the end,
                    // cancel it
                    else
                    {
                        for (int xii = x; xii > xi; --xii)
                            grid[y][xii] = '.';
                    }
                }
            }

            // EAST
            if (x + icell < grid[0].size() &&
                (field[y][x + icell] == '.' || field[y][x + icell] == 'H') &&
                (grid[y][x + icell] == '.'))
            {
                // Try to do the whole move
                int xi = x;
                grid[y][xi++] = '>';
                for (/**/; xi < x + icell; ++xi)
                {
                    if (grid[y][xi] != '.') break;
                    if (isdigit(field[y][xi]) || field[y][xi] == 'H') break;
                    grid[y][xi] = '>';
                }
                
                // If the move is not possible, cancel it
                if (xi != x + icell)
                {
                    for (int xii = x; xii < xi; ++xii)
                        grid[y][xii] = '.';
                }
                // If it goes into a hole
                else if (field[y][x + icell] == 'H')
                {
                    // Continue looking for the next ball
                    grid[y][x + icell] = 'X';
                    if (doIt(field, grid, x + 1, y))
                    {
                        grid[y][x + icell] = '.';
                        return true;
                    }
                    // If this move doesn't make it work with other balls,
                    // cancel it
                    else
                    {
                        grid[y][x + icell] = '.';
                        for (int xii = x; xii < xi; ++xii)
                            grid[y][xii] = '.';
                    }
                }
                // The move must go on
                else
                {   
                    // Continue the move
                    if (doIt(field, grid, xi, y, icell - 1, x, y))
                    {
                        return true;
                    }
                    // If this move doesn't make it work at the end,
                    // cancel it
                    else
                    {
                        for (int xii = x; xii < xi; ++xii)
                            grid[y][xii] = '.';
                    }
                }
            }
            
            return false;
        }
        else
        {
            return doIt(field, grid, x + 1, y);
        }
    }
}

int main()
{
    int width;
    int height;
    cin >> width >> height; cin.ignore();
    
    vector<string> gridI;
    vector<vector<char>> gridSol;
    
    for (int i = 0; i < height; i++) {
        string row;
        cin >> row; cin.ignore();
        gridI.push_back(row);
        
        vector<char> c;
        for (auto r : row)
        {
            c.push_back('.');
        }
        gridSol.push_back(c);
        
    }
    debugGrid(gridI);
    
    doIt(gridI, gridSol, 0, 0);
    
    solution(gridSol);
    
    return 0;
}