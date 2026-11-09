#include <iostream>
#include <string>
#include <sstream>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif
using namespace std;
// ==========================================
// TERMINAL COLOR MANAGEMENT
// ==========================================
class Colors {
public:
    static constexpr const char* RESET   = "\033[0m";
    static constexpr const char* RED     = "\033[1;31m";
    static constexpr const char* BLUE    = "\033[1;34m";
    static constexpr const char* GREEN   = "\033[1;32m";
    static constexpr const char* YELLOW  = "\033[1;33m";
    static constexpr const char* CYAN    = "\033[1;36m";
    static constexpr const char* WHITE   = "\033[1;37m";
    static constexpr const char* GREY    = "\033[90m";
    static constexpr const char* MAGENTA = "\033[1;35m";
    static void enable() {
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD mode = 0;
            if (GetConsoleMode(hOut, &mode)) {
                SetConsoleMode(
                    hOut,
                    mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING
                );
            }
        }
#endif
    }
    static const char* marker(char value) {
        if (value == 'X') return RED;
        if (value == 'O') return BLUE;
        return WHITE;
    }
};
// ==========================================
// TIC-TAC-TOE GAME CLASS
// ==========================================
class TicTacToe {
private:
    char board[3][3];
    int currentPlayer;
    char currentMarker;
    int player1Wins;
    int player2Wins;
    int draws;
    // These show the marker assignments from the most recent match.
    char lastMarkerP1;
    char lastMarkerP2;
    bool hintsUsed[3];
    // Preferred order makes equal-score hint choices predictable.
    const int preferredMoves[9] = {5, 1, 3, 7, 9, 2, 4, 6, 8};
    static char upperChar(char value) {
        return static_cast<char>(
            toupper(static_cast<unsigned char>(value))
        );
    }
    // Reset board and per-match hint allowances.
    void resetBoard() {
        char value = '1';
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                board[i][j] = value++;
            }
        }
        currentPlayer = 1;
        hintsUsed[1] = false;
        hintsUsed[2] = false;
    }
    // Display the board: X is red, O is blue, empty slots are white.
    void displayBoard() const {
        cout << "\n";
        for (int i = 0; i < 3; i++) {
            cout << " ";
            for (int j = 0; j < 3; j++) {
                char value = board[i][j];
                cout << Colors::marker(value)
                     << value
                     << Colors::RESET;
                if (j < 2) {
                    cout << Colors::GREY
                         << " | "
                         << Colors::RESET;
                }
            }
            cout << "\n";
            if (i < 2) {
                cout << Colors::GREY
                     << "---|---|---"
                     << Colors::RESET << "\n";
            }
        }
        cout << "\n";
    }
    // Check whether the specified marker has won.
    bool checkWinner(char marker) const {
        for (int i = 0; i < 3; i++) {
            if (board[i][0] == marker &&
                board[i][1] == marker &&
                board[i][2] == marker) {
                return true;
            }
            if (board[0][i] == marker &&
                board[1][i] == marker &&
                board[2][i] == marker) {
                return true;
            }
        }
        if (board[0][0] == marker &&
            board[1][1] == marker &&
            board[2][2] == marker) {
            return true;
        }
        if (board[0][2] == marker &&
            board[1][1] == marker &&
            board[2][0] == marker) {
            return true;
        }
        return false;
    }
    bool hasEmptySlot() const {
        for (int slot = 1; slot <= 9; slot++) {
            int row = (slot - 1) / 3;
            int col = (slot - 1) % 3;
            if (board[row][col] != 'X' && board[row][col] != 'O') {
                return true;
            }
        }
        return false;
    }
    // Validate a slot before accessing the board.
    bool isValidMove(int slot) const {
        if (slot < 1 || slot > 9) {
            return false;
        }
        int row = (slot - 1) / 3;
        int col = (slot - 1) % 3;
        return board[row][col] != 'X' &&
               board[row][col] != 'O';
    }
    bool placeMarker(int slot) {
        if (!isValidMove(slot)) {
            return false;
        }
        int row = (slot - 1) / 3;
        int col = (slot - 1) % 3;
        board[row][col] = currentMarker;
        return true;
    }
    // Check whether placing marker in slot would complete a win.
    bool isWinningMove(int slot, char marker) {
        if (!isValidMove(slot)) {
            return false;
        }
        int row = (slot - 1) / 3;
        int col = (slot - 1) % 3;
        char original = board[row][col];
        board[row][col] = marker;
        bool result = checkWinner(marker);
        board[row][col] = original;
        return result;
    }
    // ======================================
    // MINIMAX STRATEGY ENGINE
    // ======================================
    // Returns a score from the hint user's perspective:
    // positive = winning position, zero = draw, negative = losing position.
    // Depth makes the engine prefer quicker wins and slower losses.
    int minimax(char turn, char aiMarker, char opponent, int depth) {
        if (checkWinner(aiMarker)) {
            return 10 - depth;
        }
        if (checkWinner(opponent)) {
            return depth - 10;
        }
        if (!hasEmptySlot()) {
            return 0;
        }
        bool maximizing = (turn == aiMarker);
        int bestScore = maximizing ? -1000 : 1000;
        for (int i = 0; i < 9; i++) {
            int slot = preferredMoves[i];
            if (!isValidMove(slot)) {
                continue;
            }
            int row = (slot - 1) / 3;
            int col = (slot - 1) % 3;
            char original = board[row][col];
            board[row][col] = turn;
            char nextTurn = (turn == aiMarker) ? opponent : aiMarker;
            int score = minimax(nextTurn, aiMarker, opponent, depth + 1);
            board[row][col] = original;
            if (maximizing) {
                if (score > bestScore) {
                    bestScore = score;
                }
            } else {
                if (score < bestScore) {
                    bestScore = score;
                }
            }
        }
        return bestScore;
    }
    // Find the best move assuming both players choose optimally.
    int getBestMove(string &reason) {
        char opponent = (currentMarker == 'X') ? 'O' : 'X';
        int bestSlot = -1;
        int bestScore = -1000;
        bool bestMoveBlocksThreat = false;
        for (int i = 0; i < 9; i++) {
            int slot = preferredMoves[i];
            if (!isValidMove(slot)) {
                continue;
            }
            // An immediate win always takes priority.
            if (isWinningMove(slot, currentMarker)) {
                reason = "This move completes three markers and wins immediately.";
                return slot;
            }
            // Check whether the opponent could win in this square next turn.
            bool blocksThreat = isWinningMove(slot, opponent);
            int row = (slot - 1) / 3;
            int col = (slot - 1) % 3;
            char original = board[row][col];
            board[row][col] = currentMarker;
            int score = minimax(opponent, currentMarker, opponent, 1);
            board[row][col] = original;
            if (score > bestScore) {
                bestScore = score;
                bestSlot = slot;
                bestMoveBlocksThreat = blocksThreat;
            }
        }
        if (bestSlot == -1) {
            reason = "No moves are available.";
            return -1;
        }
        if (bestMoveBlocksThreat) {
            reason = "This move blocks an immediate winning threat.";
        } else if (bestScore > 0) {
            reason = "With optimal play, this move leads to a forced win.";
        } else if (bestScore == 0) {
            reason = "This move can secure at least a draw against optimal play.";
        } else {
            reason = "This is the strongest available move, although a loss cannot be avoided against perfect play.";
        }
        return bestSlot;
    }
    void switchPlayer() {
        currentPlayer = (currentPlayer == 1) ? 2 : 1;
        currentMarker = (currentMarker == 'X') ? 'O' : 'X';
    }
    // The marker labels refer to the most recently played match.
    void displayScoreboard() const {
        cout << "\n"
             << Colors::YELLOW
             << "========== MATCH SCOREBOARD =========="
             << Colors::RESET << "\n";
        cout << Colors::GREEN << "Player 1 (" 
             << Colors::marker(lastMarkerP1) << lastMarkerP1
             << Colors::GREEN << ") Wins: "
             << player1Wins << Colors::RESET << "\n";
        cout << Colors::GREEN << "Player 2 ("
             << Colors::marker(lastMarkerP2) << lastMarkerP2
             << Colors::GREEN << ") Wins: "
             << player2Wins << Colors::RESET << "\n";
        cout << Colors::CYAN
             << "Draws: " << draws
             << Colors::RESET << "\n";
        cout << Colors::YELLOW
             << "======================================="
             << Colors::RESET << "\n";
        cout << Colors::GREY
             << "(Marker labels show the assignments from the latest match.)"
             << Colors::RESET << "\n";
    }
    // ======================================
    // PLAY ONE MATCH
    // ======================================
    // Returns false if input closes, so the program can exit cleanly.
    bool playMatch() {
        resetBoard();
        int moves = 0;
        char markerP1 = 'X';
        cout << "\n"
             << Colors::YELLOW
             << "Player 1, choose your marker (X/O): "
             << Colors::RESET;
        while (true) {
            string input;
            if (!getline(cin, input)) {
                cout << "\nInput stream closed. Exiting game.\n";
                return false;
            }
            if (input.size() == 1) {
                markerP1 = upperChar(input[0]);
                if (markerP1 == 'X' || markerP1 == 'O') {
                    break;
                }
            }
            cout << Colors::RED
                 << "Invalid marker! Enter X or O: "
                 << Colors::RESET;
        }
        currentMarker = markerP1;
        char markerP2 = (markerP1 == 'X') ? 'O' : 'X';
        lastMarkerP1 = markerP1;
        lastMarkerP2 = markerP2;
        cout << "\n"
             << Colors::GREEN << "Player 1: "
             << Colors::RESET
             << Colors::marker(markerP1) << markerP1
             << Colors::RESET << "\n";
        cout << Colors::GREEN << "Player 2: "
             << Colors::RESET
             << Colors::marker(markerP2) << markerP2
             << Colors::RESET << "\n";
        cout << "\n"
             << Colors::CYAN
             << "Enter slot number (1-9) to play."
             << "\nEnter H for a minimax strategic hint."
             << Colors::RESET << "\n";
        while (moves < 9) {
            displayBoard();
            cout << Colors::YELLOW
                 << "Player " << currentPlayer << " ("
                 << Colors::marker(currentMarker) << currentMarker
                 << Colors::YELLOW << "), enter your move: "
                 << Colors::RESET;
            string input;
            if (!getline(cin, input)) {
                cout << "\nInput stream closed. Exiting game.\n";
                return false;
            }
            // Strategic hint: one hint per player per match.
            if (input.size() == 1 && upperChar(input[0]) == 'H') {
                if (hintsUsed[currentPlayer]) {
                    cout << Colors::RED
                         << "You have already used your hint!"
                         << Colors::RESET << "\n";
                    continue;
                }
                string reason;
                int suggestion = getBestMove(reason);
                cout << "\n"
                     << Colors::MAGENTA
                     << "===== SMART STRATEGY ASSISTANT ====="
                     << Colors::RESET << "\n";
                if (suggestion != -1) {
                    cout << Colors::CYAN
                         << "Suggested position: " << suggestion
                         << "\nReason: " << reason
                         << Colors::RESET << "\n";
                    cout << Colors::GREY
                         << "The hint does not make the move automatically."
                         << Colors::RESET << "\n";
                }
                hintsUsed[currentPlayer] = true;
                continue;
            }
            // Accept exactly one integer and reject extra characters.
            stringstream ss(input);
            int slot;
            char extra;
            if (!(ss >> slot) || (ss >> extra)) {
                cout << Colors::RED
                     << "Invalid input! Enter a number from 1 to 9 or H."
                     << Colors::RESET << "\n";
                continue;
            }
            if (!placeMarker(slot)) {
                cout << Colors::RED
                     << "Invalid or occupied position! Try again."
                     << Colors::RESET << "\n";
                continue;
            }
            moves++;
            if (checkWinner(currentMarker)) {
                displayBoard();
                cout << "\n"
                     << Colors::GREEN
                     << "====================================\n"
                     << "       CONGRATULATIONS!\n"
                     << "====================================\n"
                     << "       PLAYER " << currentPlayer << " WINS!\n"
                     << Colors::RESET;
                if (currentPlayer == 1) {
                    player1Wins++;
                } else {
                    player2Wins++;
                }
                break;
            }
            if (moves == 9) {
                displayBoard();
                cout << Colors::CYAN
                     << "The match is a DRAW!"
                     << Colors::RESET << "\n";
                draws++;
                break;
            }
            switchPlayer();
        }
        displayScoreboard();
        return true;
    }
public:
    TicTacToe() {
        player1Wins = 0;
        player2Wins = 0;
        draws = 0;
        lastMarkerP1 = 'X';
        lastMarkerP2 = 'O';
        resetBoard();
    }
    void startGame() {
        cout << "\n"
             << Colors::YELLOW
             << "========================================\n"
             << "          SMART TIC-TAC-TOE             \n"
             << "       MINIMAX STRATEGY ASSISTANT       \n"
             << "========================================\n"
             << Colors::RESET;
        while (true) {
            if (!playMatch()) {
                break;
            }
            cout << "\n"
                 << Colors::YELLOW
                 << "Do you want to play another match? (Y/N): "
                 << Colors::RESET;
            string input;
            char playAgain = ' ';
            while (true) {
                if (!getline(cin, input)) {
                    cout << "\nInput stream closed. Exiting game.\n";
                    playAgain = 'N';
                    break;
                }
                if (input.size() == 1) {
                    playAgain = upperChar(input[0]);
                    if (playAgain == 'Y' || playAgain == 'N') {
                        break;
                    }
                }
                cout << Colors::RED
                     << "Please enter Y or N: "
                     << Colors::RESET;
            }
            if (playAgain != 'Y') {
                break;
            }
        }
        cout << "\n"
             << Colors::GREEN
             << "Thank you for playing!\n"
             << Colors::RESET;
        cout << Colors::YELLOW
             << "FINAL GAME STATISTICS\n"
             << Colors::RESET;
        displayScoreboard();
    }
};
// ==========================================
// MAIN FUNCTION
// ==========================================
int main() {
    Colors::enable();
    TicTacToe game;
    game.startGame();
    return 0;
}
