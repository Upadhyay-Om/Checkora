
#include <iostream>
#include <string>
#include <cmath>
#include <cctype>
#include <vector>
#include <climits>
#include <algorithm>
#include <bits/stdc++.h>

using namespace std;


char board[8][8];
bool W_K_CASTLE = false;
bool W_Q_CASTLE = false;
bool B_K_CASTLE = false;
bool B_Q_CASTLE = false;
int EN_PASSANT_R = -1;
int EN_PASSANT_C = -1;

void loadBoard(const string &s) {
    for (int i = 0; i < 64 && i < (int)s.length(); i++) {
        board[i / 8][i % 8] = s[i];
    }
}

void loadCastlingRights(const string &rightsStr) {
    W_K_CASTLE = W_Q_CASTLE = B_K_CASTLE = B_Q_CASTLE = false;
    for (char c : rightsStr) {
        if (c == 'K') W_K_CASTLE = true;
        else if (c == 'Q') W_Q_CASTLE = true;
        else if (c == 'k') B_K_CASTLE = true;
        else if (c == 'q') B_Q_CASTLE = true;
    }
}

string serializeBoard() {
    string s;
    s.reserve(64);
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            s += board[r][c];
    return s;
}


bool isWhite(char c)  { return c >= 'A' && c <= 'Z'; }
bool isBlack(char c)  { return c >= 'a' && c <= 'z'; }
bool isEmpty(char c)  { return c == '.'; }

string colorOf(char c) {
    if (isWhite(c)) return "white";
    if (isBlack(c)) return "black";
    return "none";
}

bool inBounds(int r, int c) {
    return r >= 0 && r < 8 && c >= 0 && c < 8;
}


bool isPromotionMove(char piece, int toRow) {
    if (piece == 'P' && toRow == 0) return true;
    if (piece == 'p' && toRow == 7) return true;
    return false;
}

char resolvePromotion(char pawn, char choice) {
    char lower = tolower(choice);
    if (lower != 'q' && lower != 'r' && lower != 'b' && lower != 'n')
        lower = 'q';
    return isWhite(pawn) ? toupper(lower) : lower;
}


bool pathClear(int fromRow, int fromCol, int toRow, int toCol) {
    int dr = (toRow > fromRow) ? 1 : (toRow < fromRow) ? -1 : 0;
    int dc = (toCol > fromCol) ? 1 : (toCol < fromCol) ? -1 : 0;
    int r = fromRow + dr, c = fromCol + dc;
    while (r != toRow || c != toCol) {
        if (!isEmpty(board[r][c])) return false;
        r += dr;
        c += dc;
    }
    return true;
}


bool isSquareAttacked(int toRow, int toCol, string attackerColor) {
    char pKnight = (attackerColor == "white") ? 'N' : 'n';
    char pRook   = (attackerColor == "white") ? 'R' : 'r';
    char pBishop = (attackerColor == "white") ? 'B' : 'b';
    char pQueen  = (attackerColor == "white") ? 'Q' : 'q';
    char pPawn   = (attackerColor == "white") ? 'P' : 'p';
    char pKing   = (attackerColor == "white") ? 'K' : 'k';

    int knightRowOffsets[] = {-2, -2, -1, -1, 1, 1, 2, 2}, knightColOffsets[] = {-1, 1, -2, 2, -2, 2, -1, 1};
    for (int i = 0; i < 8; i++) {
        int r = toRow + knightRowOffsets[i], c = toCol + knightColOffsets[i];
        if (inBounds(r, c) && board[r][c] == pKnight) return true;
    }

    int dr[] = {0, 0, 1, -1, 1, 1, -1, -1}, dc[] = {1, -1, 0, 0, 1, -1, 1, -1};
    for (int i = 0; i < 8; i++) {
        int r = toRow + dr[i], c = toCol + dc[i];
        while (inBounds(r, c)) {
            char piece = board[r][c];
            if (piece != '.') {
                if (colorOf(piece) == attackerColor) {
                    char type = tolower(piece);
                    if (i < 4 && (type == 'r' || type == 'q')) return true;
                    if (i >= 4 && (type == 'b' || type == 'q')) return true;
                }
                break;
            }
            r += dr[i]; c += dc[i];
        }
    }

    int dir = (attackerColor == "white") ? 1 : -1;
    if (inBounds(toRow + dir, toCol - 1) && board[toRow + dir][toCol - 1] == pPawn) return true;
    if (inBounds(toRow + dir, toCol + 1) && board[toRow + dir][toCol + 1] == pPawn) return true;

    for (int r = toRow - 1; r <= toRow + 1; r++)
        for (int c = toCol - 1; c <= toCol + 1; c++)
            if (inBounds(r, c) && (r != toRow || c != toCol) && board[r][c] == pKing) return true;

    return false;
}


bool validPawn(const string &color, int fromRow, int fromCol, int toRow, int toCol) {
    int dir      = (color == "white") ? -1 : 1;
    int startRow = (color == "white") ?  6 : 1;
    int dr = toRow - fromRow;
    int dc = toCol - fromCol;

    if (dc == 0 && dr == dir && isEmpty(board[toRow][toCol]))
        return true;

    if (dc == 0 && dr == 2 * dir && fromRow == startRow)
        if (isEmpty(board[fromRow + dir][fromCol]) && isEmpty(board[toRow][toCol]))
            return true;

    if (abs(dc) == 1 && dr == dir && !isEmpty(board[toRow][toCol]))
        return true;

    if (abs(dc) == 1 && dr == dir && toRow == EN_PASSANT_R && toCol == EN_PASSANT_C)
        return true;

    return false;
}

bool validRook(int fromRow, int fromCol, int toRow, int toCol) {
    return (fromRow == toRow || fromCol == toCol) && pathClear(fromRow, fromCol, toRow, toCol);
}

bool validKnight(int fromRow, int fromCol, int toRow, int toCol) {
    int rowDir = abs(toRow - fromRow), colDir = abs(toCol - fromCol);
    return (rowDir == 2 && colDir == 1) || (rowDir == 1 && colDir == 2);
}

bool validBishop(int fromRow, int fromCol, int toRow, int toCol) {
    return (abs(toRow - fromRow) == abs(toCol - fromCol)) && pathClear(fromRow, fromCol, toRow, toCol);
}

bool validQueen(int fromRow, int fromCol, int toRow, int toCol) {
    return validRook(fromRow, fromCol, toRow, toCol) || validBishop(fromRow, fromCol, toRow, toCol);
}

bool validKing(const string &color, int fromRow, int fromCol, int toRow, int toCol) {
    if (abs(toRow - fromRow) <= 1 && abs(toCol - fromCol) <= 1) return true;

    if (fromRow == toRow && abs(toCol - fromCol) == 2) {
        if (color == "white" && fromRow == 7 && fromCol == 4) {
            if (toCol == 6 && W_K_CASTLE && isEmpty(board[7][5]) && isEmpty(board[7][6])) {
                if (!isSquareAttacked(7, 4, "black") && !isSquareAttacked(7, 5, "black") && !isSquareAttacked(7, 6, "black"))
                    return true;
            }
            if (toCol == 2 && W_Q_CASTLE && isEmpty(board[7][3]) && isEmpty(board[7][2]) && isEmpty(board[7][1])) {
                if (!isSquareAttacked(7, 4, "black") && !isSquareAttacked(7, 3, "black") && !isSquareAttacked(7, 2, "black"))
                    return true;
            }
        } else if (color == "black" && fromRow == 0 && fromCol == 4) {
            if (toCol == 6 && B_K_CASTLE && isEmpty(board[0][5]) && isEmpty(board[0][6])) {
                if (!isSquareAttacked(0, 4, "white") && !isSquareAttacked(0, 5, "white") && !isSquareAttacked(0, 6, "white"))
                    return true;
            }
            if (toCol == 2 && B_Q_CASTLE && isEmpty(board[0][3]) && isEmpty(board[0][2]) && isEmpty(board[0][1])) {
                if (!isSquareAttacked(0, 4, "white") && !isSquareAttacked(0, 3, "white") && !isSquareAttacked(0, 2, "white"))
                    return true;
            }
        }
    }
    return false;
}


bool validateMove(const string &turn, int fromRow, int fromCol, int toRow, int toCol, bool silent = false) {
    char piece = board[fromRow][fromCol];
    if (isEmpty(piece)) return false;
    if (colorOf(piece) != turn) return false;
    if (fromRow == toRow && fromCol == toCol) return false;

    char target = board[toRow][toCol];
    if (!isEmpty(target) && colorOf(target) == turn) return false;

    char type = static_cast<char>(tolower(static_cast<unsigned char>(piece)));
    bool isMoveValid = false;

    switch (type) {
        case 'p': isMoveValid = validPawn(turn, fromRow, fromCol, toRow, toCol); break;
        case 'r': isMoveValid = validRook(fromRow, fromCol, toRow, toCol);       break;
        case 'n': isMoveValid = validKnight(fromRow, fromCol, toRow, toCol);     break;
        case 'b': isMoveValid = validBishop(fromRow, fromCol, toRow, toCol);     break;
        case 'q': isMoveValid = validQueen(fromRow, fromCol, toRow, toCol);      break;
        case 'k': isMoveValid = validKing(turn, fromRow, fromCol, toRow, toCol); break;
    }

    if (isMoveValid && !silent) cout << "VALID" << endl;
    else if (!isMoveValid && !silent) cout << "INVALID Illegal move" << endl;

    return isMoveValid;
}


struct Move {
    int fromRow, fromCol, toRow, toCol;
    char promoPiece;
};

pair<int,int> findKing(const string &color);
bool leavesKingInCheck(const Move &m, const string &side);


void handleMoves(const string &turn, int row, int col) {
    char piece = board[row][col];
    if (isEmpty(piece) || colorOf(piece) != turn) {
        cout << "MOVES" << endl;
        return;
    }
    cout << "MOVES";
    for (int toRow = 0; toRow < 8; toRow++) {
        for (int toCol = 0; toCol < 8; toCol++) {
            if (validateMove(turn, row, col, toRow, toCol, true)) {
                Move move;
                move.fromRow = row; move.fromCol = col;
                move.toRow = toRow;  move.toCol = toCol;
                move.promoPiece = isPromotionMove(piece, toRow)
                    ? (isWhite(piece) ? 'Q' : 'q') : '\0';
                if (leavesKingInCheck(move, turn)) continue;

                int cap   = isEmpty(board[toRow][toCol]) ? 0 : 1;
                int promo = isPromotionMove(piece, toRow) ? 1 : 0;
                cout << " " << toRow << " " << toCol << " " << cap << " " << promo;
            }
        }
    }
    cout << endl;
}


void handlePromote(const string &turn, int fromRow, int fromCol, int toRow, int toCol,
                   char promoPiece) {
    char piece = board[fromRow][fromCol];

    if (isEmpty(piece) || colorOf(piece) != turn || tolower(piece) != 'p') {
        cout << "INVALID Not a pawn" << endl;
        return;
    }

    if (!validateMove(turn, fromRow, fromCol, toRow, toCol, true)) {
        cout << "INVALID Illegal move" << endl;
        return;
    }

    if (!isPromotionMove(piece, toRow)) {
        cout << "INVALID Not a promotion square" << endl;
        return;
    }

    board[toRow][toCol] = resolvePromotion(piece, promoPiece);
    board[fromRow][fromCol] = '.';

    cout << "PROMOTE " << serializeBoard() << endl;
}


int pieceValue(char piece) {
    switch (tolower(piece)) {
        case 'p': return 100;
        case 'n': return 320;
        case 'b': return 330;
        case 'r': return 500;
        case 'q': return 900;
        case 'k': return 20000;
        default:  return 0;
    }
}


static const int pawnTable[8][8] = {
    {  0,  0,  0,  0,  0,  0,  0,  0},
    { 50, 50, 50, 50, 50, 50, 50, 50},
    { 10, 10, 20, 30, 30, 20, 10, 10},
    {  5,  5, 10, 25, 25, 10,  5,  5},
    {  0,  0,  0, 20, 20,  0,  0,  0},
    {  5, -5,-10,  0,  0,-10, -5,  5},
    {  5, 10, 10,-20,-20, 10, 10,  5},
    {  0,  0,  0,  0,  0,  0,  0,  0}
};

static const int knightTable[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

static const int bishopTable[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  5,  0,  0,  0,  0,  5,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};

static const int rookTable[8][8] = {
    {  0,  0,  0,  0,  0,  0,  0,  0},
    {  5, 10, 10, 10, 10, 10, 10,  5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    {  0,  0,  0,  5,  5,  0,  0,  0}
};

static const int queenTable[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};

static const int kingMiddleTable[8][8] = {
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    { 20, 20,  0,  0,  0,  0, 20, 20},
    { 20, 30, 10,  0,  0, 10, 30, 20}
};

int positionalBonus(char piece, int row, int col) {
    char type = static_cast<char>(tolower(static_cast<unsigned char>(piece)));
    int r = isWhite(piece) ? row : (7 - row);

    switch (type) {
        case 'p': return pawnTable[r][col];
        case 'n': return knightTable[r][col];
        case 'b': return bishopTable[r][col];
        case 'r': return rookTable[r][col];
        case 'q': return queenTable[r][col];
        case 'k': return kingMiddleTable[r][col];
        default:  return 0;
    }
}

int evaluate() {
    int score = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            char piece = board[r][c];
            if (isEmpty(piece)) continue;

            int pieceScore = pieceValue(piece) + positionalBonus(piece, r, c);
            score += isWhite(piece) ? pieceScore : -pieceScore;
        }
    }
    pair<int, int> blackKingPos = findKing("black");
    if (blackKingPos.first != -1 && isSquareAttacked(blackKingPos.first, blackKingPos.second, "white"))
        score += 50;

    pair<int, int> whiteKingPos = findKing("white");
    if (whiteKingPos.first != -1 && isSquareAttacked(whiteKingPos.first, whiteKingPos.second, "black"))
        score -= 50;

    return score;
}

bool isInsufficientMaterial() {
    int totalMinor = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            char piece = board[r][c];
            if (piece == '.') continue;
            char type = tolower(static_cast<unsigned char>(piece));
            if (type == 'k') continue;
            if (type == 'p' || type == 'r' || type == 'q') return false;
            totalMinor++;
        }
    }
    return totalMinor <= 1;
}

vector<Move> generateMoves(const string &side) {
    vector<Move> moves;
    moves.reserve(64);

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            char piece = board[r][c];
            if (isEmpty(piece) || colorOf(piece) != side) continue;

            for (int toRow = 0; toRow < 8; toRow++) {
                for (int toCol = 0; toCol < 8; toCol++) {
                    if (validateMove(side, r, c, toRow, toCol, true)) {
                        Move move;
                        move.fromRow = r; move.fromCol = c;
                        move.toRow = toRow; move.toCol = toCol;
                        move.promoPiece = isPromotionMove(piece, toRow) ? (isWhite(piece) ? 'Q' : 'q') : '\0';
                        moves.push_back(move);
                    }
                }
            }
        }
    }
    return moves;
}

void orderMoves(vector<Move> &moves) {
    sort(moves.begin(), moves.end(), [](const Move &a, const Move &b) {
        int moveScore = 0, otherScore = 0;

        if (!isEmpty(board[a.toRow][a.toCol])) moveScore += pieceValue(board[a.toRow][a.toCol]) + 1000;
        if (!isEmpty(board[b.toRow][b.toCol])) otherScore += pieceValue(board[b.toRow][b.toCol]) + 1000;

        if (a.promoPiece) moveScore += 900;
        if (b.promoPiece) otherScore += 900;

        return moveScore > otherScore;
    });
}

pair<int,int> findKing(const string &color) {
    char target = (color == "white") ? 'K' : 'k';
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if (board[r][c] == target) return {r, c};
    return {-1, -1};
}

bool leavesKingInCheck(const Move &m, const string &side) {
    char tempBoard[8][8];
    for(int i=0; i<8; i++) for(int j=0; j<8; j++) tempBoard[i][j] = board[i][j];

    char piece = board[m.fromRow][m.fromCol];
    
    if (tolower(static_cast<unsigned char>(piece)) == 'p' && m.fromCol != m.toCol && board[m.toRow][m.toCol] == '.') {
        board[m.fromRow][m.toCol] = '.'; 
    }

    board[m.toRow][m.toCol] = m.promoPiece ? m.promoPiece : piece;
    board[m.fromRow][m.fromCol] = '.';

    if (tolower(piece) == 'k' && abs(m.toCol - m.fromCol) == 2) {
        if (m.toCol == 6) { board[m.fromRow][5] = board[m.fromRow][7]; board[m.fromRow][7] = '.'; }
        else { board[m.fromRow][3] = board[m.fromRow][0]; board[m.fromRow][0] = '.'; }
    }

    pair<int, int> kingPos = findKing(side);
    bool inCheck = (kingPos.first != -1) && isSquareAttacked(kingPos.first, kingPos.second, (side == "white" ? "black" : "white"));

    for(int i=0; i<8; i++) for(int j=0; j<8; j++) board[i][j] = tempBoard[i][j];
    return inCheck;
}

int minimax(int depth, int alpha, int beta, bool maximizing) {
    if (depth == 0) return evaluate();

    string side = maximizing ? "white" : "black";
    vector<Move> moves = generateMoves(side);
    orderMoves(moves);

    vector<Move> legal;
    legal.reserve(moves.size());
    for (auto &move : moves) {
        if (!leavesKingInCheck(move, side))
            legal.push_back(move);
    }

    if (legal.empty()) {
        string opponent = maximizing ? "black" : "white";
        pair<int,int> kingPos = findKing(side);
        if (kingPos.first >= 0 && isSquareAttacked(kingPos.first, kingPos.second, opponent))
            return maximizing ? (-99999 + (100 - depth))
                              : ( 99999 - (100 - depth));
        return 0;
    }

    if (maximizing) {
        int maxEval = INT_MIN;
        for (auto &move : legal) {
            char movingPiece = board[move.fromRow][move.fromCol];
            char capturedPiece = board[move.toRow][move.toCol];
            board[move.toRow][move.toCol] = move.promoPiece ? move.promoPiece : movingPiece;
            board[move.fromRow][move.fromCol] = '.';

            int rookFromRow = -1, rookFromCol = -1, rookToRow = -1, rookToCol = -1;
            if (tolower(movingPiece) == 'k' && abs(move.toCol - move.fromCol) == 2) {
                if (move.toCol == 6) { rookFromRow = move.fromRow; rookFromCol = 7; rookToRow = move.toRow; rookToCol = 5; }
                else if (move.toCol == 2) { rookFromRow = move.fromRow; rookFromCol = 0; rookToRow = move.toRow; rookToCol = 3; }
                if (rookFromRow != -1) {
                    board[rookToRow][rookToCol] = board[rookFromRow][rookFromCol];
                    board[rookFromRow][rookFromCol] = '.';
                }
            }

            bool prevWhiteKingside = W_K_CASTLE, prevWhiteQueenside = W_Q_CASTLE, prevBlackKingside = B_K_CASTLE, prevBlackQueenside = B_Q_CASTLE;
            if (movingPiece == 'K') { W_K_CASTLE = false; W_Q_CASTLE = false; }
            if (movingPiece == 'k') { B_K_CASTLE = false; B_Q_CASTLE = false; }
            if (movingPiece == 'R') { if (move.fromRow == 7 && move.fromCol == 0) W_Q_CASTLE = false; else if (move.fromRow == 7 && move.fromCol == 7) W_K_CASTLE = false; }
            if (movingPiece == 'r') { if (move.fromRow == 0 && move.fromCol == 0) B_Q_CASTLE = false; else if (move.fromRow == 0 && move.fromCol == 7) B_K_CASTLE = false; }
            if (capturedPiece == 'R') { if (move.toRow == 7 && move.toCol == 0) W_Q_CASTLE = false; else if (move.toRow == 7 && move.toCol == 7) W_K_CASTLE = false; }
            if (capturedPiece == 'r') { if (move.toRow == 0 && move.toCol == 0) B_Q_CASTLE = false; else if (move.toRow == 0 && move.toCol == 7) B_K_CASTLE = false; }

            int eval = minimax(depth - 1, alpha, beta, false);

            W_K_CASTLE = prevWhiteKingside; W_Q_CASTLE = prevWhiteQueenside; B_K_CASTLE = prevBlackKingside; B_Q_CASTLE = prevBlackQueenside;

            board[move.fromRow][move.fromCol] = movingPiece;
            board[move.toRow][move.toCol] = capturedPiece;
            if (rookFromRow != -1) {
                board[rookFromRow][rookFromCol] = board[rookToRow][rookToCol];
                board[rookToRow][rookToCol] = '.';
            }

            maxEval = max(maxEval, eval);
            alpha = max(alpha, eval);
            if (beta <= alpha) break;
        }
        return maxEval;
    } else {
        int minEval = INT_MAX;
        for (auto &move : legal) {
            char movingPiece = board[move.fromRow][move.fromCol];
            char capturedPiece = board[move.toRow][move.toCol];
            board[move.toRow][move.toCol] = move.promoPiece ? move.promoPiece : movingPiece;
            board[move.fromRow][move.fromCol] = '.';

            int rookFromRow = -1, rookFromCol = -1, rookToRow = -1, rookToCol = -1;
            if (tolower(movingPiece) == 'k' && abs(move.toCol - move.fromCol) == 2) {
                if (move.toCol == 6) { rookFromRow = move.fromRow; rookFromCol = 7; rookToRow = move.toRow; rookToCol = 5; }
                else if (move.toCol == 2) { rookFromRow = move.fromRow; rookFromCol = 0; rookToRow = move.toRow; rookToCol = 3; }
                if (rookFromRow != -1) {
                    board[rookToRow][rookToCol] = board[rookFromRow][rookFromCol];
                    board[rookFromRow][rookFromCol] = '.';
                }
            }

            bool prevWhiteKingside = W_K_CASTLE, prevWhiteQueenside = W_Q_CASTLE, prevBlackKingside = B_K_CASTLE, prevBlackQueenside = B_Q_CASTLE;
            if (movingPiece == 'K') { W_K_CASTLE = false; W_Q_CASTLE = false; }
            if (movingPiece == 'k') { B_K_CASTLE = false; B_Q_CASTLE = false; }
            if (movingPiece == 'R') { if (move.fromRow == 7 && move.fromCol == 0) W_Q_CASTLE = false; else if (move.fromRow == 7 && move.fromCol == 7) W_K_CASTLE = false; }
            if (movingPiece == 'r') { if (move.fromRow == 0 && move.fromCol == 0) B_Q_CASTLE = false; else if (move.fromRow == 0 && move.fromCol == 7) B_K_CASTLE = false; }
            if (capturedPiece == 'R') { if (move.toRow == 7 && move.toCol == 0) W_Q_CASTLE = false; else if (move.toRow == 7 && move.toCol == 7) W_K_CASTLE = false; }
            if (capturedPiece == 'r') { if (move.toRow == 0 && move.toCol == 0) B_Q_CASTLE = false; else if (move.toRow == 0 && move.toCol == 7) B_K_CASTLE = false; }

            int eval = minimax(depth - 1, alpha, beta, true);

            W_K_CASTLE = prevWhiteKingside; W_Q_CASTLE = prevWhiteQueenside; B_K_CASTLE = prevBlackKingside; B_Q_CASTLE = prevBlackQueenside;

            board[move.fromRow][move.fromCol] = movingPiece;
            board[move.toRow][move.toCol] = capturedPiece;
            if (rookFromRow != -1) {
                board[rookFromRow][rookFromCol] = board[rookToRow][rookToCol];
                board[rookToRow][rookToCol] = '.';
            }

            minEval = min(minEval, eval);
            beta = min(beta, eval);
            if (beta <= alpha) break;
        }
        return minEval;
    }
}


void handleStatus(const string &turn) {

    string opponent = (turn == "white") ? "black" : "white";
    pair<int,int> kingPos = findKing(turn);
    bool inCheck = (kingPos.first >= 0) &&
                   isSquareAttacked(kingPos.first, kingPos.second, opponent);

    vector<Move> moves = generateMoves(turn);
    bool hasLegal = false;
    for (auto &move : moves) {
        if (!leavesKingInCheck(move, turn)) {
            hasLegal = true;
            break;
        }
    }

    if (!hasLegal) {
        if (inCheck) cout << "STATUS CHECKMATE" << endl;
        else         cout << "STATUS STALEMATE" << endl;
    } else {
        if (inCheck) cout << "STATUS CHECK" << endl;
        else if (isInsufficientMaterial()) cout << "STATUS DRAW" << endl;
        else         cout << "STATUS OK" << endl;
    }
}

void handleNotation(const string &turn, int fromRow, int fromCol, int toRow, int toCol) {
    char piece = board[fromRow][fromCol];
    if (isEmpty(piece)) {
        cout << "NOTATION ?" << endl;
        return;
    }

    char type = static_cast<char>(tolower(static_cast<unsigned char>(piece)));
    bool isCapture = !isEmpty(board[toRow][toCol]);
    string files = "abcdefgh";

    if (type == 'k') {
        if (abs(toCol - fromCol) == 2) {
            if (toCol == 6) { cout << "NOTATION O-O" << endl; return; }
            if (toCol == 2) { cout << "NOTATION O-O-O" << endl; return; }
        }
    }

    string sanNotation = "";
    if (type == 'p') {
        if (fromCol != toCol) {
            sanNotation += files[static_cast<string::size_type>(fromCol)];
            sanNotation += 'x';
        }
        sanNotation += files[static_cast<string::size_type>(toCol)];
        sanNotation += to_string(8 - toRow);

    } else {
        sanNotation += static_cast<char>(toupper(static_cast<unsigned char>(type)));

        vector<pair<int, int>> ambiguousPieces;
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                if (r == fromRow && c == fromCol) continue;
                if (board[r][c] == piece) {
                    if (validateMove(turn, r, c, toRow, toCol, true)) {
                        Move m = {r, c, toRow, toCol, '\0'};
                        if (!leavesKingInCheck(m, turn)) {
                            ambiguousPieces.push_back({r, c});
                        }
                    }
                }
            }
        }

        if (!ambiguousPieces.empty()) {
            bool sameFile = false;
            bool sameRank = false;
            for (auto &ambig : ambiguousPieces) {
                if (ambig.second == fromCol) sameFile = true;
                if (ambig.first == fromRow) sameRank = true;
            }

            if (!sameFile) {
                sanNotation += files[static_cast<string::size_type>(fromCol)];
            } else if (!sameRank) {
                sanNotation += to_string(8 - fromRow);
            } else {
                sanNotation += files[static_cast<string::size_type>(fromCol)];
                sanNotation += to_string(8 - fromRow);
            }
        }

        if (isCapture) sanNotation += 'x';
        sanNotation += files[static_cast<string::size_type>(toCol)];
        sanNotation += to_string(8 - toRow);
    }

    char movingPiece = board[fromRow][fromCol];
    char capturedPiece = board[toRow][toCol];
    board[toRow][toCol] = movingPiece;
    board[fromRow][fromCol] = '.';

    string opponent = (turn == "white") ? "black" : "white";
    pair<int, int> kingPos = findKing(opponent);
    if (kingPos.first != -1 && isSquareAttacked(kingPos.first, kingPos.second, turn)) {
        vector<Move> oppMoves = generateMoves(opponent);
        bool hasLegal = false;
        for (auto &move : oppMoves) {
            if (!leavesKingInCheck(move, opponent)) {
                hasLegal = true;
                break;
            }
        }
        sanNotation += hasLegal ? "+" : "#";
    }

    board[fromRow][fromCol] = movingPiece;
    board[toRow][toCol] = capturedPiece;

    cout << "NOTATION " << sanNotation << endl;
}

void handleBestMove(const string &turn, int depth) {
    bool maximizing = (turn == "white");
    vector<Move> moves = generateMoves(turn);
    orderMoves(moves);

    vector<Move> legal;
    legal.reserve(moves.size());
    for (auto &move : moves) {
        if (!leavesKingInCheck(move, turn))
            legal.push_back(move);
    }

    if (legal.empty()) {
        cout << "BESTMOVE NONE" << endl;
        return;
    }

    Move best = legal[0];
    int bestVal = maximizing ? INT_MIN : INT_MAX;

    for (auto &move : legal) {
        char movingPiece = board[move.fromRow][move.fromCol];
        char capturedPiece = board[move.toRow][move.toCol];
        board[move.toRow][move.toCol] = move.promoPiece ? move.promoPiece : movingPiece;
        board[move.fromRow][move.fromCol] = '.';

        int rookFromRow = -1, rookFromCol = -1, rookToRow = -1, rookToCol = -1;
        if (tolower(movingPiece) == 'k' && abs(move.toCol - move.fromCol) == 2) {
            if (move.toCol == 6) { rookFromRow = move.fromRow; rookFromCol = 7; rookToRow = move.toRow; rookToCol = 5; }
            else if (move.toCol == 2) { rookFromRow = move.fromRow; rookFromCol = 0; rookToRow = move.toRow; rookToCol = 3; }
            if (rookFromRow != -1) {
                board[rookToRow][rookToCol] = board[rookFromRow][rookFromCol];
                board[rookFromRow][rookFromCol] = '.';
            }
        }

        bool prevWhiteKingside = W_K_CASTLE, prevWhiteQueenside = W_Q_CASTLE, prevBlackKingside = B_K_CASTLE, prevBlackQueenside = B_Q_CASTLE;
        if (movingPiece == 'K') { W_K_CASTLE = false; W_Q_CASTLE = false; }
        if (movingPiece == 'k') { B_K_CASTLE = false; B_Q_CASTLE = false; }
        if (movingPiece == 'R') { if (move.fromRow == 7 && move.fromCol == 0) W_Q_CASTLE = false; else if (move.fromRow == 7 && move.fromCol == 7) W_K_CASTLE = false; }
        if (movingPiece == 'r') { if (move.fromRow == 0 && move.fromCol == 0) B_Q_CASTLE = false; else if (move.fromRow == 0 && move.fromCol == 7) B_K_CASTLE = false; }
        if (capturedPiece == 'R') { if (move.toRow == 7 && move.toCol == 0) W_Q_CASTLE = false; else if (move.toRow == 7 && move.toCol == 7) W_K_CASTLE = false; }
        if (capturedPiece == 'r') { if (move.toRow == 0 && move.toCol == 0) B_Q_CASTLE = false; else if (move.toRow == 0 && move.toCol == 7) B_K_CASTLE = false; }

        int eval = minimax(depth - 1, INT_MIN, INT_MAX, !maximizing);

        W_K_CASTLE = prevWhiteKingside; W_Q_CASTLE = prevWhiteQueenside; B_K_CASTLE = prevBlackKingside; B_Q_CASTLE = prevBlackQueenside;

        board[move.fromRow][move.fromCol] = movingPiece;
        board[move.toRow][move.toCol] = capturedPiece;
        if (rookFromRow != -1) {
            board[rookFromRow][rookFromCol] = board[rookToRow][rookToCol];
            board[rookToRow][rookToCol] = '.';
        }

        if (maximizing) {
            if (eval > bestVal) { bestVal = eval; best = move; }
        } else {
            if (eval < bestVal) { bestVal = eval; best = move; }
        }
    }

    cout << "BESTMOVE " << best.fromRow << " " << best.fromCol
         << " " << best.toRow << " " << best.toCol << endl;
}

int main() {
    string command;
    while (cin >> command) {
        if (command == "VALIDATE") {
            string b, rights, t; int epR, epC, fromRow, fromCol, toRow, toCol;
            cin >> b >> rights >> t >> epR >> epC >> fromRow >> fromCol >> toRow >> toCol;
            loadBoard(b);
            loadCastlingRights(rights);
            EN_PASSANT_R = epR; EN_PASSANT_C = epC;
            validateMove(t, fromRow, fromCol, toRow, toCol);
        } 
        else if (command == "MOVES") {
            string b, rights, t; int epR, epC, r, c;
            cin >> b >> rights >> t >> epR >> epC >> r >> c;
            loadBoard(b);
            loadCastlingRights(rights);
            EN_PASSANT_R = epR; EN_PASSANT_C = epC;
            handleMoves(t, r, c);
        } 
        else if (command == "ATTACKED") {
            string b, rights, attackerColor; int r, c;
            cin >> b >> rights >> attackerColor >> r >> c;
            loadBoard(b);
            loadCastlingRights(rights);
            if (isSquareAttacked(r, c, attackerColor)) cout << "YES" << endl;
            else cout << "NO" << endl;
        }
        else if (command == "PROMOTE") {
            string b, rights, t; int epR, epC, fromRow, fromCol, toRow, toCol; char promo;
            cin >> b >> rights >> t >> epR >> epC >> fromRow >> fromCol >> toRow >> toCol >> promo;
            loadBoard(b);
            loadCastlingRights(rights);
            EN_PASSANT_R = epR; EN_PASSANT_C = epC;
            handlePromote(t, fromRow, fromCol, toRow, toCol, promo);
        }
        else if (command == "STATUS") {
            string b, rights, t; int epR, epC;
            cin >> b >> rights >> t >> epR >> epC;
            loadBoard(b);
            loadCastlingRights(rights);
            EN_PASSANT_R = epR; EN_PASSANT_C = epC;
            handleStatus(t);
        }
        else if (command == "BESTMOVE") {
            string b, rights, t; int epR, epC, depth;
            cin >> b >> rights >> t >> epR >> epC >> depth;
            loadBoard(b);
            loadCastlingRights(rights);
            EN_PASSANT_R = epR; EN_PASSANT_C = epC;
            handleBestMove(t, depth);
        }
        else if (command == "NOTATION") {
            string b, rights, t; int epR, epC, fromRow, fromCol, toRow, toCol;
            cin >> b >> rights >> t >> epR >> epC >> fromRow >> fromCol >> toRow >> toCol;
            loadBoard(b);
            loadCastlingRights(rights);
            EN_PASSANT_R = epR; EN_PASSANT_C = epC;
            handleNotation(t, fromRow, fromCol, toRow, toCol);
        }
    }
    return 0;
}
