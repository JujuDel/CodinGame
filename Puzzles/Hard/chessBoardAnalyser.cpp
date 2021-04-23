#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>

using namespace std;

enum Color {
    WHITE = 0,
    BLACK = 1
};

struct Cell {
    bool targetBy[2];
    char piece;

    Cell(const char _p) : piece{ _p }, targetBy{ false, false }
    {}
};

bool isValid(const int x, const int y) {
    return x >= 0 && x < 8 && y >= 0 && y < 8;
}

void updateRook(vector<vector<Cell>>& board, const int x, const int y, const Color color) {
    char king{ color == WHITE ? 'k' : 'K' };
    // North
    int j = 1;
    while (isValid(x, y - j)) {
        board[y - j][x].targetBy[color] = true;
        if (board[y - j][x].piece == '.' || board[y - j][x].piece == king)
            j++;
        else
            break;
    }

    // South
    j = 1;
    while (isValid(x, y + j)) {
        board[y + j][x].targetBy[color] = true;
        if (board[y + j][x].piece == '.' || board[y + j][x].piece == king)
            j++;
        else
            break;
    }

    // East
    int i = 1;
    while (isValid(x + i, y)) {
        board[y][x + i].targetBy[color] = true;
        if (board[y][x + i].piece == '.' || board[y][x + i].piece == king)
            i++;
        else
            break;
    }

    // West
    i = 1;
    while (isValid(x - i, y)) {
        board[y][x - i].targetBy[color] = true;
        if (board[y][x - i].piece == '.' || board[y][x - i].piece == king)
            i++;
        else
            break;
    }
}

void updateKnight(vector<vector<Cell>>& board, const int x, const int y, const Color color) {
    if (isValid(x - 1, y + 2)) {
        board[y + 2][x - 1].targetBy[color] = true;
    }
    if (isValid(x - 2, y + 1)) {
        board[y + 1][x - 2].targetBy[color] = true;
    }

    if (isValid(x - 1, y - 2)) {
        board[y - 2][x - 1].targetBy[color] = true;
    }
    if (isValid(x - 2, y - 1)) {
        board[y - 1][x - 2].targetBy[color] = true;
    }

    if (isValid(x + 1, y - 2)) {
        board[y - 2][x + 1].targetBy[color] = true;
    }
    if (isValid(x + 2, y - 1)) {
        board[y - 1][x + 2].targetBy[color] = true;
    }

    if (isValid(x + 1, y + 2)) {
        board[y + 2][x + 1].targetBy[color] = true;
    }
    if (isValid(x + 2, y + 1)) {
        board[y + 1][x + 2].targetBy[color] = true;
    }
}

void updateBishop(vector<vector<Cell>>& board, const int x, const int y, const Color color) {
    const char king{ color == WHITE ? 'k' : 'K' };
    
    // North-East
    int i = 1;
    while (isValid(x + i, y - i)) {
        board[y - i][x + i].targetBy[color] = true;
        if (board[y - i][x + i].piece == '.' || board[y - i][x + i].piece == king)
            i++;
        else
            break;
    }

    // North-West
    i = 1;
    while (isValid(x - i, y - i)) {
        board[y - i][x - i].targetBy[color] = true;
        if (board[y - i][x - i].piece == '.' || board[y - i][x - i].piece == king)
            i++;
        else
            break;
    }

    // South-West
    i = 1;
    while (isValid(x - i, y + i)) {
        board[y + i][x - i].targetBy[color] = true;
        if (board[y + i][x - i].piece == '.' || board[y + i][x - i].piece == king)
            i++;
        else
            break;
    }

    // South-East
    i = 1;
    while (isValid(x + i, y + i)) {
        board[y + i][x + i].targetBy[color] = true;
        if (board[y + i][x + i].piece == '.' || board[y + i][x + i].piece == king)
            i++;
        else
            break;
    }
}

void updateQueen(vector<vector<Cell>>& board, const int x, const int y, const Color color) {
    updateBishop(board, x, y, color);
    updateRook(board, x, y, color);
}

void updateKing(vector<vector<Cell>>& board, const int x, const int y, const Color color) {
    for (int j = -1; j < 2; ++j) {
        for (int i = -1; i < 2; ++i) {
            if (isValid(x + i, y + j)) {
                board[y + j][x + i].targetBy[color] = true;
            }
        }
    }
}

void updatePawn(vector<vector<Cell>>& board, const int x, const int y, const Color color) {
    int j{ color == WHITE ? -1 : 1 };
    
    if (isValid(x + 1, y + j))
        board[y + j][x + 1].targetBy[color] = true;
    if (isValid(x - 1, y + j))
        board[y + j][x - 1].targetBy[color] = true;
}


int main()
{
    vector<vector<Cell>> board;

    int kx, ky, Kx, Ky;

    for (int j = 0; j < 8; j++) {
        string boardRow;
        getline(cin, boardRow);
        vector<Cell> row;
        for (int i = 0; i < 8; i++) {
            row.push_back({ boardRow[i] });
            if (boardRow[i] == 'k') {
                kx = i;
                ky = j;
            }
            if (boardRow[i] == 'K') {
                Kx = i;
                Ky = j;
            }
        }
        board.push_back(row);
    }

    for (const auto& row : board) {
        for (const auto& cell : row) {
            std::cerr << cell.piece << " ";
        }
        std::cerr << endl;
    }
    std::cerr << endl;

    for (int j = 0; j < 8; ++j) {
        for (int i = 0; i < 8; ++i) {
            if (board[j][i].piece == 'R') {
                updateRook(board, i, j, WHITE);
            }
            else if (board[j][i].piece == 'N') {
                updateKnight(board, i, j, WHITE);
            }
            else if (board[j][i].piece == 'B') {
                updateBishop(board, i, j, WHITE);
            }
            else if (board[j][i].piece == 'Q') {
                updateQueen(board, i, j, WHITE);
            }
            else if (board[j][i].piece == 'K') {
                updateKing(board, i, j, WHITE);
            }
            else if (board[j][i].piece == 'P') {
                updatePawn(board, i, j, WHITE);
            }
            else if (board[j][i].piece == 'r') {
                updateRook(board, i, j, BLACK);
            }
            else if (board[j][i].piece == 'n') {
                updateKnight(board, i, j, BLACK);
            }
            else if (board[j][i].piece == 'b') {
                updateBishop(board, i, j, BLACK);
            }
            else if (board[j][i].piece == 'q') {
                updateQueen(board, i, j, BLACK);
            }
            else if (board[j][i].piece == 'k') {
                updateKing(board, i, j, BLACK);
            }
            else if (board[j][i].piece == 'p') {
                updatePawn(board, i, j, BLACK);
            }
        }
    }

    std::cerr << "TargetBy: BLACK" << std::endl;
    for (const auto& row : board) {
        for (const auto& cell : row) {
            if (cell.targetBy[BLACK])
                std::cerr << 1 << " ";
            else
                std::cerr << ". ";
        }
        std::cerr << endl;
    }
    std::cerr << endl;

    std::cerr << "TargetBy: WHITE" << std::endl;
    for (const auto& row : board) {
        for (const auto& cell : row) {
            if (cell.targetBy[WHITE])
                std::cerr << 1 << " ";
            else
                std::cerr << ". ";
        }
        std::cerr << endl;
    }
    std::cerr << endl;

    bool win = false;
    if (board[Ky][Kx].targetBy[BLACK]) {
        win = true;
        for (int j = -1; j < 2; ++j) {
            for (int i = -1; i < 2; ++i) {
                if ((i == 1 && j == 1) || !isValid(Kx + i, Ky + j))
                    continue;
                
                if (board[Ky + j][Kx + i].piece == '.') {
                    if (!board[Ky + j][Kx + i].targetBy[BLACK]) {
                        win = false;
                        break;
                    }
                }
                else {
                    if (islower(board[Ky + j][Kx + i].piece) && !board[Ky + j][Kx + i].targetBy[BLACK]) {
                        win = false;
                        break;
                    }
                }
            }
        }
    }

    if (win)
    {
        cout << "B" << endl;
    }
    else
    {
        if (board[ky][kx].targetBy[WHITE]) {
            win = true;
            for (int j = -1; j < 2; ++j) {
                for (int i = -1; i < 2; ++i) {
                    if ((i == 1 && j == 1) || !isValid(kx + i, ky + j))
                        continue;
                    
                    if (board[ky + j][kx + i].piece == '.') {
                        if (!board[ky + j][kx + i].targetBy[WHITE]) {
                            win = false;
                            break;
                        }
                    }
                    else {
                        if (isupper(board[ky + j][kx + i].piece) && !board[ky + j][kx + i].targetBy[WHITE]) {
                            win = false;
                            break;
                        }
                    }
                }
            }
        }
        if (win)
        {
            cout << "W" << endl;
        }
        else
        {
            cout << "N" << endl;
        }
    }
}