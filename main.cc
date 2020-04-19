#include <cstdio>
#include <cstring>
#include <vector>
#include <random>

#define VERSION "0.0.1"

static char USAGE[400];
static int len = sprintf(USAGE, """Minesweeper (%s)\n\
\n\
Usage:\n\
    minesweeper [flags]\n\
\n\
Flags:\n\
    -h, --help          print usage\n\
    -d, --difficulty    set difficulty (0: beginner, 1: intermediate, 2: advanced)\n\
\n\
Gameplay:\n\
    Enter the coordinates separated by commas to reveal a cell.\n\
    Enter the coordinates followed by 'm' separated by commas to mark a mine\n\
""", VERSION);

#define BEGINNER 0
#define INTERMEDIATE 1
#define ADVANCED  2

using std::vector;

class Board {
public:
    Board(int difficulty) {
        // Get difficulty and init side length and num mines
        switch (difficulty) {
            case BEGINNER:
                _side = 9;
                _mines = 10;
                break;
            case INTERMEDIATE:
                _side = 16;
                _mines = 40;
                break;
            case ADVANCED:
                _side = 24;
                _mines = 99;
                break;
        }

        // Construct boards
        _mine_board = vector<vector<short>>(_side, vector<short>(_side, 0));
        _board = vector<vector<char>>(_side, vector<char>(_side, '-'));

        //// init random number generator
        std::random_device rd;
        std::default_random_engine seed(rd());
        std::uniform_int_distribution<int> dis(0, _side-1);

        // Generate mines in random locations
        // Mine is marked by -1
        // Surrounding are positive integer mine counts
        // Default is 0
        for (int i=0; i<_mines; ++i) {
            int row = dis(seed), col = dis(seed);
            // This location already has a mine, move on
            if (_mine_board[row][col] == -1) {
                --i;
            } else {
                // There isn't a mine already placed here
                placeMineHere(row, col);
            }
        }
    }

    void playerMarkMine(int row, int col) {
        if (!isValid(row, col))
            return;
        if (_board[row][col] == '#') {
            _board[row][col] = '-';
            if (_mine_board[row][col] == -1) {
                _mines++;
            }
        } else {
            _board[row][col] = '#';
            if (_mine_board[row][col] == -1) {
                _mines--;
            }
        }
    }

    /*
     * Handle player input
     * Returns 0 on success
     * Returns !0 on failure
     */
    int playerInput(int row, int col) {
        // Check if input is out of bounds
        if (!isValid(row, col)) {
            printf("Input row and col out of bounds. Try again.\n");
            return 0;
        }
        // Check if the input is marked
        if (_board[row][col] == '#') {
            printf(" Input has been marked.\n");
            return 0;
        }

        // Check if there's a mine at the input
        if (_mine_board[row][col] == -1) {
            printf("You trigger a mine, game over.\n");
            return 1;
        }

        // Reveal the mine counts
        revealMineCountNearInput(row, col);
        return 0;
    }

    void revealMineCountNearInput(int row, int col) {
        // Return if
        //    1. input is out of bounds
        //    2. input is a mine
        //    3. board already has this piece revealed
        //
        if (!isValid(row, col) ||
                _mine_board[row][col] == -1 ||
                _board[row][col] != '-')
            return;

        if (_mine_board[row][col] != 0) {
            _board[row][col] = '0' + _mine_board[row][col];
        } else {
            _board[row][col] = '0';
            revealMineCountNearInput(row + 1, col);
            revealMineCountNearInput(row, col + 1);
            revealMineCountNearInput(row - 1, col);
            revealMineCountNearInput(row, col - 1);
        }
    }

    void printMineBoard() {
        _printBoard(_mine_board, 'i');
    }

    void printPlayerBoard() {
        _printBoard(_board, 'c');
    }

    bool isValid(int row, int col) {
        return row >= 0 && col >= 0 && row < _side && col < _side;
    }

    int getMines() {
        return _mines;
    }


private:
    int _side;
    int _mines;
    vector<vector<short>> _mine_board;
    vector<vector<char>> _board;

    /*
     * Helper function to place a mine at the given row, col
     * while incrementing mine counts at all surrounding cells
     */
    void placeMineHere(int row, int col) {
        _mine_board[row][col] = -1;
        for (int nrow = row-1; nrow <= row+1; ++nrow) {
            for (int ncol = col-1; ncol <= col+1; ++ncol) {
                incrementCount(nrow, ncol);
            }
        }
    }

    /*
     * Helper function for building the minefield
     * Whenever we generate a new random location for a mine
     * call this function on the surrounding coordinates
     * to increment their count
     */
    void incrementCount(int row, int col) {
        if (isValid(row, col) && _mine_board[row][col] != -1) {
            _mine_board[row][col]++;
        }
    }

    /*
     * Helper function to print a board
     */
    template<class T>
    void _printBoard(vector<vector<T>> board, char type) {
        // Print col numbers
        printf("    ");
        for (int i=0; i<_side; ++i)
            printf("  %d", i);
        printf("\n    ");

        // Print separator
        for (int i=0; i<_side; ++i)
            printf("---");
        printf("\n");

        // Print row numbers and rows
        for (int row=0; row<_side; ++row) {
            printf("%d  | ", row);

            if (type == 'c') {
                for (T& c : board[row]) {
                    printf("%2c ", c);
                }
            } else {
                for (T& c : board[row]) {
                    printf("%2d ", c);
                }
            }
            printf("\n");
        }
    }


};

int main(int argc, char** argv) {

    int difficulty = BEGINNER;

    for (int i=1; i < argc; ++i) {
        if (strcmp(argv[i], "-h")==0 || strcmp(argv[i], "--help")==0) {
            printf("%s", USAGE);
            return 0;
        }
        if (strcmp(argv[i], "-d")==0 || strcmp(argv[i], "--difficulty")==0) {
            i++;
            if (i>=argc) return 1;
            difficulty = atoi(argv[i]);
        }
    }

    printf("Welcome to MineSweeper (%s).\n\n", VERSION);
    auto b = Board(difficulty);

    //b.printMineBoard();
    b.printPlayerBoard();

    int row, col;
    char c;
    while (1) {
        c = ' ';
        row = -1, col = -1;
        printf("Enter (row, col, [m]): ");
        if (scanf("%d, %d, %c%*c", &row, &col, &c) == 0) {
            getchar();
            printf("Invalid input format.\n");
            continue;
        }
        if (!b.isValid(row, col)) {
            printf("Invalid values.\n");
            continue;
        }
        printf("\nYou entered row: %d, col: %d ", row, col);
        if (c == 'm') {
            printf("to mark as a mine.\n");
            b.playerMarkMine(row, col);
        } else {
            printf("to reveal.\n");
            if (b.playerInput(row, col) != 0) break;
        }
        b.printPlayerBoard();

        if (b.getMines() == 0) {
            printf("You won!!!\n");
            return 0;
        }
    }

    return 0;
}
