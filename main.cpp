#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QBrush>
#include <QPen>
#include <QDebug>
#include <QMouseEvent>
#include <unordered_map>

enum class PieceType { Pawn, Rook, Knight, Bishop, Queen, King };

class ChessPiece : public QGraphicsTextItem {
public:
    static const int Type; // Declaration of the static type identifier for ChessPiece

    ChessPiece(const QString &symbol, int x, int y, PieceType type, bool isWhite)
            : QGraphicsTextItem(symbol), m_pieceType(type), m_isWhite(isWhite) {
        setPos(x, y);
        setDefaultTextColor(isWhite ? Qt::black : Qt::white);
        setFont(QFont("Arial", 24));
    }

    // Returns the chess piece type (e.g., Pawn, Rook, etc.)
    PieceType pieceType() const { return m_pieceType; }
    bool isWhitePiece() const { return m_isWhite; }

    void highlight(bool highlight = true) {
        setOpacity(highlight ? 0.5 : 1.0);
    }

    // Override the QGraphicsItem::type() method to return our custom type identifier
    int type() const override {
        return Type;
    }

private:
    PieceType m_pieceType; // Using m_ prefix for member variables for clarity
    bool m_isWhite;
};

// Definition of the static type identifier for ChessPiece
// This needs to be done in a source file (.cpp), not in the header (.h)
const int ChessPiece::Type = QGraphicsItem::UserType + 1;

class ChessBoard : public QGraphicsView {
public:
    ChessBoard(QWidget *parent = nullptr) : QGraphicsView(parent), selectedPiece(nullptr), isWhiteTurn(false) {
        scene = new QGraphicsScene(this);
        setScene(scene);
        setFixedSize(400, 400);
        setBackgroundBrush(QBrush(Qt::gray));

        drawBoard();
        setupPieces();
    }

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    void drawBoard();
    void setupPieces();
    bool isValidMove(ChessPiece *piece, int row, int col);
    void highlightValidMoves(ChessPiece *piece, bool highlight = true);
    bool isValidMoveWhitePawn(ChessPiece *piece, int newRow, int newCol);
    bool isValidMoveBlackPawn(ChessPiece *piece, int newRow, int newCol);
    bool isPawnPromotion(ChessPiece *piece, int newRow);
    void promotePawn(ChessPiece *piece, int newRow, int newCol);

    QGraphicsScene *scene;
    ChessPiece *selectedPiece;
    QPointF originalPos;
    QList<QGraphicsRectItem*> highlightedSquares;
    bool isWhiteTurn;

    bool movePiece(ChessPiece *piece, int row, int col);

    bool isPathBlocked(int row, int col, int row1, int col1);

    void clearHighlights();
};

void ChessBoard::highlightValidMoves(ChessPiece *piece, bool highlight) {
    if (!piece) return;

    for (QGraphicsRectItem *highlightedSquare : highlightedSquares) {
        scene->removeItem(highlightedSquare);
        delete highlightedSquare;
    }
    highlightedSquares.clear();

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (isValidMove(piece, row, col)) {
                QGraphicsRectItem *highlightedSquare = new QGraphicsRectItem(col * 50, row * 50, 50, 50);
                highlightedSquare->setBrush(QBrush(Qt::blue));
                highlightedSquare->setOpacity(highlight ? 0.5 : 0);
                scene->addItem(highlightedSquare);
                highlightedSquares.append(highlightedSquare);
            }
        }
    }
}

void ChessBoard::mousePressEvent(QMouseEvent *event) {
    QGraphicsView::mousePressEvent(event);

    QPoint viewportPos = event->pos();
    QPointF scenePos = mapToScene(viewportPos);
    qDebug() << "Clicked at scene position: " << scenePos;

    int row = static_cast<int>(scenePos.y() / 50);
    int col = static_cast<int>(scenePos.x() / 50);

    QGraphicsItem *item = scene->itemAt(scenePos, QTransform());
    ChessPiece *clickedPiece = dynamic_cast<ChessPiece*>(item);

    if (isWhiteTurn) {
        // White's turn
        if (selectedPiece == nullptr && clickedPiece != nullptr && clickedPiece->isWhitePiece()) {
            selectedPiece = clickedPiece;
            originalPos = clickedPiece->pos();
            highlightValidMoves(selectedPiece);
            selectedPiece->highlight();
        } else if (selectedPiece != nullptr && clickedPiece == nullptr) {
            if (movePiece(selectedPiece, row, col)) {
                selectedPiece->highlight(false);
                selectedPiece = nullptr;
                clearHighlights();
                isWhiteTurn = !isWhiteTurn; // Switch to black's turn
            } else {
                // Handle invalid move
            }
        } else if (selectedPiece != nullptr && clickedPiece == selectedPiece) {
            selectedPiece->highlight(false);
            selectedPiece = nullptr;
            clearHighlights();
        } else if (selectedPiece != nullptr && clickedPiece != selectedPiece) {
            // Handle trying to move opponent's piece
        } else {
            // Handle other cases
        }
    } else {
        // Black's turn
        if (selectedPiece == nullptr && clickedPiece != nullptr && !clickedPiece->isWhitePiece()) {
            selectedPiece = clickedPiece;
            originalPos = clickedPiece->pos();
            highlightValidMoves(selectedPiece);
            selectedPiece->highlight();
        } else if (selectedPiece != nullptr && clickedPiece == nullptr) {
            if (movePiece(selectedPiece, row, col)) {
                selectedPiece->highlight(false);
                selectedPiece = nullptr;
                clearHighlights();
                isWhiteTurn = !isWhiteTurn; // Switch to white's turn
            } else {
                // Handle invalid move
            }
        } else if (selectedPiece != nullptr && clickedPiece == selectedPiece) {
            selectedPiece->highlight(false);
            selectedPiece = nullptr;
            clearHighlights();
        } else if (selectedPiece != nullptr && clickedPiece != selectedPiece) {
            // Handle trying to move opponent's piece
        } else {
            // Handle other cases
        }
    }
}


void ChessBoard::drawBoard() {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            QColor color = (i + j) % 2 == 0 ? Qt::lightGray : Qt::darkGray;
            QGraphicsRectItem *square = new QGraphicsRectItem(j * 50, i * 50, 50, 50);
            square->setBrush(QBrush(color));
            scene->addItem(square);
        }
    }
}

void ChessBoard::setupPieces() {
    // Setup white pieces
    scene->addItem(new ChessPiece("♖", 0, 0, PieceType::Rook, true));
    scene->addItem(new ChessPiece("♖", 350, 0, PieceType::Rook, true));
    scene->addItem(new ChessPiece("♘", 50, 0, PieceType::Knight, true));
    scene->addItem(new ChessPiece("♘", 300, 0, PieceType::Knight, true));
    scene->addItem(new ChessPiece("♗", 100, 0, PieceType::Bishop, true));
    scene->addItem(new ChessPiece("♗", 250, 0, PieceType::Bishop, true));
    scene->addItem(new ChessPiece("♕", 150, 0, PieceType::Queen, true));
    scene->addItem(new ChessPiece("♔", 200, 0, PieceType::King, true));
    scene->addItem(new ChessPiece("♙", 0, 50, PieceType::Pawn, true));
    scene->addItem(new ChessPiece("♙", 50, 50, PieceType::Pawn, true));
    scene->addItem(new ChessPiece("♙", 100, 50, PieceType::Pawn, true));
    scene->addItem(new ChessPiece("♙", 150, 50, PieceType::Pawn, true));
    scene->addItem(new ChessPiece("♙", 200, 50, PieceType::Pawn, true));
    scene->addItem(new ChessPiece("♙", 250, 50, PieceType::Pawn, true));
    scene->addItem(new ChessPiece("♙", 300, 50, PieceType::Pawn, true));
    scene->addItem(new ChessPiece("♙", 350, 50, PieceType::Pawn, true));

    // Setup black pieces
    scene->addItem(new ChessPiece("♜", 0, 350, PieceType::Rook, false));
    scene->addItem(new ChessPiece("♜", 350, 350, PieceType::Rook, false));
    scene->addItem(new ChessPiece("♞", 50, 350, PieceType::Knight, false));
    scene->addItem(new ChessPiece("♞", 300, 350, PieceType::Knight, false));
    scene->addItem(new ChessPiece("♝", 100, 350, PieceType::Bishop, false));
    scene->addItem(new ChessPiece("♝", 250, 350, PieceType::Bishop, false));
    scene->addItem(new ChessPiece("♛", 150, 350, PieceType::Queen, false));
    scene->addItem(new ChessPiece("♚", 200, 350, PieceType::King, false));
    scene->addItem(new ChessPiece("♟", 0, 300, PieceType::Pawn, false));
    scene->addItem(new ChessPiece("♟", 50, 300, PieceType::Pawn, false));
    scene->addItem(new ChessPiece("♟", 100, 300, PieceType::Pawn, false));
    scene->addItem(new ChessPiece("♟", 150, 300, PieceType::Pawn, false));
    scene->addItem(new ChessPiece("♟", 200, 300, PieceType::Pawn, false));
    scene->addItem(new ChessPiece("♟", 250, 300, PieceType::Pawn, false));
    scene->addItem(new ChessPiece("♟", 300, 300, PieceType::Pawn, false));
    scene->addItem(new ChessPiece("♟", 350, 300, PieceType::Pawn, false));
}

bool ChessBoard::movePiece(ChessPiece *piece, int row, int col) {
    if (!piece) return false;

    int currentRow = piece->pos().y() / 50;
    int currentCol = piece->pos().x() / 50;

    if (isValidMove(piece, row, col)) {
        // Find all items in the square we're moving to
        QList<QGraphicsItem *> itemsAtDestination = scene->items(QRectF(col * 50, row * 50, 50, 50), Qt::IntersectsItemShape);
        for (QGraphicsItem *item : itemsAtDestination) {
            ChessPiece *potentialCapturedPiece = dynamic_cast<ChessPiece *>(item);
            if (potentialCapturedPiece && potentialCapturedPiece != piece && potentialCapturedPiece->isWhitePiece() != piece->isWhitePiece()) {
                qDebug() << "Capturing piece at " << col << ", " << row;
                scene->removeItem(potentialCapturedPiece);
                delete potentialCapturedPiece;
                break; // Assuming only one piece can occupy a square
            }
        }

        // Check for pawn promotion
        if (isPawnPromotion(piece, row)) {
            promotePawn(piece, row, col);
        } else {
            // Move the piece to the new location
            piece->setPos(col * 50, row * 50);
        }

        // The move was successful
        return true;
    }

    // The move was not successful (invalid move)
    return false;
}

bool ChessBoard::isPathBlocked(int startRow, int startCol, int endRow, int endCol) {
    if (startRow == endRow && startCol == endCol) {
        return false; // No path to check, start and end are the same.
    }

    int rowDirection = (endRow > startRow) ? 1 : (endRow < startRow) ? -1 : 0;
    int colDirection = (endCol > startCol) ? 1 : (endCol < startCol) ? -1 : 0;

    int currentRow = startRow + rowDirection;
    int currentCol = startCol + colDirection;

    while ((currentRow != endRow || currentCol != endCol)) {
        QGraphicsItem* item = scene->itemAt(currentCol * 50 + 25, currentRow * 50 + 25, QTransform());
        if (item) {
            ChessPiece* chessPiece = dynamic_cast<ChessPiece*>(item);
            if (chessPiece) {
                return true; // Found a chess piece blocking the path.
            }
        }
        currentRow += rowDirection;
        currentCol += colDirection;
    }

    return false; // Path is not blocked.
}



bool ChessBoard::isValidMoveWhitePawn(ChessPiece *piece, int newRow, int newCol) {
    if (!piece) return false;

    int currentRow = piece->pos().y() / 50;
    int currentCol = piece->pos().x() / 50;
    int rowDiff = newRow - currentRow;
    int colDiff = newCol - currentCol;

    QGraphicsItem *destinationItem = scene->itemAt(newCol * 50 + 25, newRow * 50 + 25, QTransform());
    ChessPiece *destinationPiece = dynamic_cast<ChessPiece *>(destinationItem);

    // Forward movement without capture
    if (colDiff == 0 && !destinationPiece) {
        if (rowDiff == 1) {
            return true; // Move one square forward
        }
        if (rowDiff == 2 && currentRow == 6 && !isPathBlocked(currentRow, currentCol, newRow, newCol)) {
            return true; // Initial double move
        }
    }

    // Capture move
    if (abs(colDiff) == 1 && rowDiff == -1) { // White pawn moves up the board, hence -1
        QGraphicsItem *destinationItem = scene->itemAt(newCol * 50 + 25, newRow * 50 + 25, QTransform());
        ChessPiece *destinationPiece = dynamic_cast<ChessPiece *>(destinationItem);
        if (destinationPiece && destinationPiece->isWhitePiece() != piece->isWhitePiece()) {
            return true; // There's a piece of the opposite color, capture is valid
        }
    }

    return false;
}

bool ChessBoard::isValidMoveBlackPawn(ChessPiece *piece, int newRow, int newCol) {
    if (!piece) return false;

    int currentRow = piece->pos().y() / 50;
    int currentCol = piece->pos().x() / 50;
    int rowDiff = newRow - currentRow;
    int colDiff = newCol - currentCol;

    QGraphicsItem *destinationItem = scene->itemAt(newCol * 50 + 25, newRow * 50 + 25, QTransform());
    ChessPiece *destinationPiece = dynamic_cast<ChessPiece *>(destinationItem);

    // Forward movement without capture
    if (colDiff == 0 && !destinationPiece) {
        if (rowDiff == -1) {
            return true; // Move one square forward
        }
        if (rowDiff == -2 && currentRow == 1 && !isPathBlocked(currentRow, currentCol, newRow, newCol)) {
            return true; // Initial double move
        }
    }

    // Capture move
    if (abs(colDiff) == 1 && rowDiff == 1 && destinationPiece && destinationPiece->isWhitePiece() != piece->isWhitePiece()) {
        return true; // Diagonal capture
    }

    return false;
}



bool ChessBoard::isValidMove(ChessPiece *piece, int newRow, int newCol) {
    if (!piece) return false;

    int currentRow = piece->pos().y() / 50;
    int currentCol = piece->pos().x() / 50;

    // Check if the destination is the same as the current position
    if (newRow == currentRow && newCol == currentCol) {
        // The piece is not actually moving
        return false;
    }

    int rowDiff = newRow - currentRow;
    int colDiff = newCol - currentCol;
    int absRowDiff = std::abs(rowDiff);
    int absColDiff = std::abs(colDiff);

    QGraphicsItem *destinationItem = scene->itemAt(newCol * 50 + 25, newRow * 50 + 25, QTransform());
    ChessPiece *destinationPiece = dynamic_cast<ChessPiece *>(destinationItem);
    // Ensure a piece doesn't capture one of the same color
    if (destinationPiece && destinationPiece->isWhitePiece() == piece->isWhitePiece()) {
        return false;
    }

    switch (piece->pieceType()) {
        case PieceType::Pawn:
            // Pawn move logic is handled in the respective functions
            return piece->isWhitePiece() ? isValidMoveWhitePawn(piece, newRow, newCol) : isValidMoveBlackPawn(piece, newRow, newCol);

        case PieceType::Rook: {
            // Check if moving straight in a row or column
            if (newRow != currentRow && newCol != currentCol) return false; // Not moving straight

            // Check for path blockage for moves longer than one square
            if ((rowDiff > 1 || colDiff > 1) && isPathBlocked(currentRow, currentCol, newRow, newCol)) return false;

            return true; // Rook can move if no blockage and moving straight
        }

        case PieceType::Knight:
            // Knights can jump over pieces
            if ((absRowDiff == 2 && absColDiff == 1) || (absRowDiff == 1 && absColDiff == 2)) return true;
            break;

        case PieceType::Bishop:
            // Bishops move diagonally and cannot jump over pieces
            if (absRowDiff == absColDiff && !isPathBlocked(currentRow, currentCol, newRow, newCol)) return true;
            break;

        case PieceType::Queen:
            // Queens move horizontally, vertically, or diagonally and cannot jump over pieces
            if ((absRowDiff == absColDiff || rowDiff == 0 || colDiff == 0) && !isPathBlocked(currentRow, currentCol, newRow, newCol)) return true;
            break;

        case PieceType::King:
            // Kings move one square in any direction and cannot jump over pieces (though this is implicit in the one-square move)
            if (absRowDiff <= 1 && absColDiff <= 1 && (absRowDiff + absColDiff > 0) && !isPathBlocked(currentRow, currentCol, newRow, newCol)) return true;
            break;

        default:
            return false;
    }
    return false;
}


bool ChessBoard::isPawnPromotion(ChessPiece *piece, int newRow) {
    if (piece->pieceType() == PieceType::Pawn && (newRow == 0 || newRow == 7)) {
        return true;
    }
    return false;
}

void ChessBoard::promotePawn(ChessPiece *piece, int newRow, int newCol) {
    // Replace pawn with a queen
    bool isWhite = piece->isWhitePiece();
    scene->removeItem(piece);
    delete piece;
    scene->addItem(new ChessPiece("♕", newCol * 50, newRow * 50, PieceType::Queen, isWhite));
}

void ChessBoard::clearHighlights() {
    for (auto *highlightedSquare : qAsConst(highlightedSquares)) {
        scene->removeItem(highlightedSquare);
        delete highlightedSquare;
    }
    highlightedSquares.clear();
}


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    ChessBoard chessBoard;
    chessBoard.show();

    return app.exec();
}
