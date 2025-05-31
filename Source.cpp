#include <iostream>
#include <vector>
#include <memory>
#include <sstream>
#include <unordered_map>


#define WHITE_COLOR "\033[37m"
#define BLACK_COLOR "\033[30m"
#define PURPLE "\033[35m"
#define RESET "\033[0m"
#define RED "\033[31m"

using namespace std;

enum class GameMode { SINGLE_PLAYER, MULTI_PLAYER };

enum class GameStatus { IN_PROGRESS, WHITE_WINS, BLACK_WINS, DRAW };


unordered_map<string, string> figures = {
    {"whiteChecker", "⚪ "},
    {"blackChecker", "⚫ "},
	{"whiteKing", string(WHITE_COLOR) + "👑" + RESET},
    {"blackKing", string(BLACK_COLOR) + "👑" + RESET},
    {"lightCell", string(WHITE_COLOR) + " □ " + RESET},
    {"darkCell", string(PURPLE) + " ■ " + RESET},
	{"whitePlayer", "👤"},
	{"blackPlayer", "👤"}
};


enum class PlayerType { USER, BOT };

enum class PieceColor { WHITE, BLACK };

enum class PieceType { MAN, KING };

class Piece {
public:
    PieceColor color;
    PieceType type;

    Piece(PieceColor color) : color(color), type(PieceType::MAN) {}
    void promote()
    {
		type = PieceType::KING;
    };
};

class Cell {
public:
    int row;
    int col;
    shared_ptr<Piece> piece;

    Cell(int r, int c) : row(r), col(c), piece(nullptr) {};

	bool isEmpty() const
    {
		return piece == nullptr;
    };
};

class Board {
public:
    static const int SIZE = 8;
    vector<vector<Cell>> cells;

    Board() {
        for (int r = 0; r < SIZE; ++r) {
            vector<Cell> row;
            for (int c = 0; c < SIZE; ++c) {
                row.emplace_back(r, c);
            }
            cells.push_back(row);
        }
        initialize();
    }

    void initialize() {
    cells.resize(SIZE, vector<Cell>(SIZE, Cell(0, 0)));
    for (int row = 0; row < SIZE; ++row) {
        for (int col = 0; col < SIZE; ++col) {
            cells[row][col] = Cell(row, col);
            // Шашки тільки на чорних клітинках
            if ((row + col) % 2 == 1) {
                if (row < 3) {
                    cells[row][col].piece = make_shared<Piece>(PieceColor::BLACK);
                }
                else if (row > 4) {
                    cells[row][col].piece = make_shared<Piece>(PieceColor::WHITE);
                }
            }
        }
    }
}

    void print() const {
        cout << "   ";
        for (int col = 0; col < SIZE; ++col)
            cout << col << "  ";
        cout << endl;

        for (int row = 0; row < SIZE; ++row) {
            cout << row << " ";
            for (int col = 0; col < SIZE; ++col) {
                const Cell& cell = cells[row][col];

                bool isLightCell = ((row + col) % 2 == 0);

                if (!cell.isEmpty()) {
					if (cell.piece->type == PieceType::KING) {
						if (cell.piece->color == PieceColor::WHITE)
							cout << figures["whiteKing"];
						else
							cout << figures["blackKing"];
					}
					else
					{
                        if (cell.piece->color == PieceColor::WHITE)
                            cout << figures["whiteChecker"];
                        else
                            cout << figures["blackChecker"];
					}
                }
                else {
                    if (isLightCell)
                        cout << figures["lightCell"];
                    else
                        cout << figures["darkCell"];
                }
            }
            cout << endl;
        }
    }

    bool isMoveValid(int fromRow, int fromCol, int toRow, int toCol) const {
        if (fromRow < 0 || fromRow >= SIZE || toRow < 0 || toRow >= SIZE ||
            fromCol < 0 || fromCol >= SIZE || toCol < 0 || toCol >= SIZE)
            return false;

        const Cell& from = cells[fromRow][fromCol];
        const Cell& to = cells[toRow][toCol];

        if (from.isEmpty() || !to.isEmpty()) return false;

        int dr = toRow - fromRow;
        int dc = abs(toCol - fromCol);

        if (dc != 1 || abs(dr) != 1) return false;

        return true;
    }

    bool ableToMakeMove(int fromRow, int fromCol, int toRow, int toCol) {
        if (!isMoveValid(fromRow, fromCol, toRow, toCol)) {
            return false;
        }

        cells[toRow][toCol].piece = cells[fromRow][fromCol].piece;
        cells[fromRow][fromCol].piece = nullptr;

        if ((toRow == 0 && cells[toRow][toCol].piece->color == PieceColor::WHITE) ||
            (toRow == SIZE - 1 && cells[toRow][toCol].piece->color == PieceColor::BLACK)) {
            cells[toRow][toCol].piece->promote();
        }
        return true;
    }
};

class Player {
public:
    PlayerType type;
    PieceColor color;

    Player(PlayerType type, PieceColor color) : type(type), color(color) {};

    void makeMove(Board& board) {
        int fromRow = 0, fromCol = 0, toRow= 0, toCol= 0;
        while (!board.ableToMakeMove(fromRow, fromCol, toRow, toCol))
        {
            cout << "Enter move (fromRow fromCol toRow toCol): ";
            string input;
            getline(cin, input);
            stringstream iss(input);
            iss >> fromRow >> fromCol >> toRow >> toCol;
            string word;
            vector<string> words;

            while (iss >> word) {
                words.push_back(word);
            }
            for (const auto& w : words) {
                cout << w << " ";
            }
            if (!board.isMoveValid(fromRow, fromCol, toRow, toCol)) {
                cout << RED << "Invalid move!" << RESET << endl;
            }
        };
    }
};

class Game {
private:
    Board board;
    Player player1;
    Player player2;
    Player* currentPlayer;

public:
    Game() :
        player1(PlayerType::USER, PieceColor::WHITE),
        player2(PlayerType::USER, PieceColor::BLACK),
        currentPlayer(&player1) {
    }
    void start() {
        while (!isGameOver()) {
            board.print();
            cout << ((currentPlayer->color == PieceColor::WHITE) ? "White" : "Black") << "'s turn." << endl;
            currentPlayer->makeMove(board);
            switchPlayer();
        }
        showWinner();
    }
    void switchPlayer() {
        currentPlayer = (currentPlayer == &player1) ? &player2 : &player1;
    }
    bool isGameOver() const {
        // Просте завершення: не залишилось шашок (можна покращити)
        bool whiteExists = false, blackExists = false;
        for (const auto& row : board.cells) {
            for (const auto& cell : row) {
                if (!cell.isEmpty()) {
                    if (cell.piece->color == PieceColor::WHITE) whiteExists = true;
                    if (cell.piece->color == PieceColor::BLACK) blackExists = true;
                }
            }
        }
        return !(whiteExists && blackExists);
    }
    void showWinner() const {
        cout << "Game over! ";
        if (!isGameOver()) return;
        cout << "Winner: ";
        for (const auto& row : board.cells) {
            for (const auto& cell : row) {
                if (!cell.isEmpty()) {
                    cout << ((cell.piece->color == PieceColor::WHITE) ? "White" : "Black") << endl;
                    return;
                }
            }
        }
    }
};

int main() {
    system("chcp 65001 > nul");

    Game game;
    game.start();
    return 0;
}

