#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>


#define BOARD_SIZE 6
#define NUM_PIECES 8
#define MAX_SOLUTIONS 5000

typedef struct {
   // char name;
    int width;
    int height;
    int empty;
} PuzzlePiece;

//enum PieceName {A, B, C, D, E, F, G, H, I};

void duplicateBoard(int from[BOARD_SIZE][BOARD_SIZE], int to[BOARD_SIZE][BOARD_SIZE]) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            to[row][col] = from[row][col];
        }
    }
}

int modifyEmpty(PuzzlePiece piece, int rt) {
    if (piece.empty < 0 || rt == 0) {
       return piece.empty;
    }
    //printf("piece empty = %d\n", piece.empty);
    int pos_x = piece.empty % piece.width;
    int pos_y = piece.empty / piece.width;
   // printf("piece x = %d\n", pos_x);
    //printf("piece y = %d\n", pos_y);

      //  printf("piece y = %d\n", pos_y);
    int store_pos;
    if (rt > 0) {
        store_pos = pos_x;
        pos_x = pos_y;
        pos_y = piece.width - 1 - store_pos;
    }
    if (rt > 1) {
        store_pos = pos_x;
        pos_x = pos_y;
        pos_y = piece.height - 1 - store_pos;
    }
    if (rt > 2) {
        store_pos = pos_x;
        pos_x = pos_y;
        pos_y = piece.width - 1 - store_pos;
    }

    int no_row = rt % 2 == 0? piece.width : piece.height;
   // printf("after x = %d\n", pos_x);
    //printf("after y = %d\n", pos_y);
    //printf("row = %d\n", no_row);
    //printf("rt 1 empty = %d\n", pos_y * no_row + pos_x);
    return pos_y * no_row + pos_x;
}

bool canPlacePiece(int board[BOARD_SIZE][BOARD_SIZE], PuzzlePiece piece, int row, int col, int rt) {
    int width = rt % 2 == 0? piece.width : piece.height;
    int height = rt % 2 == 0? piece.height : piece.width;
    if (row < 0 || row + height > BOARD_SIZE || col < 0 || col + width > BOARD_SIZE || rt < 0 || rt > 3) {
        return false;
    }
    int count = 0;
    int empty = modifyEmpty(piece, rt);
    //printf("empty = %d\n", empty);

    for (int i = row; i < row + height; i++) {
        for (int j = col; j < col + width; j++) {
            if (board[i][j] != 0 && count != empty) {
                return false;
            }
            count++;
        }
    }

    return true;
}

void placePiece(int board[BOARD_SIZE][BOARD_SIZE], PuzzlePiece piece, int row, int col, int number, int rt) {
    int count = 0;
    //printf("empty = %d\n", piece.empty);
    int empty = modifyEmpty(piece, rt);
    int width = rt % 2 == 0? piece.width : piece.height;
    int height = rt % 2 == 0? piece.height : piece.width;
    for (int i = row; i < row + height; i++) {
        for (int j = col; j < col + width; j++) {
            //printf("count = %d\n", count);
            if (count == empty) {
                count++;
                continue;
            }
            board[i][j] = number;
            count++;
        }
    }
}

void removePiece(int board[BOARD_SIZE][BOARD_SIZE], PuzzlePiece piece, int row, int col, int rt) {
    int count = 0;
    int empty = modifyEmpty(piece, rt);
    int width = rt % 2 == 0? piece.width : piece.height;
    int height = rt % 2 == 0? piece.height : piece.width;
    for (int i = row; i < row + height; i++) {
        for (int j = col; j < col + width; j++) {
            if (count == empty) {
                count++;
                continue;
            }
            board[i][j] = 0;
            count++;
        }
    }
}

void removePieceSec(int board[BOARD_SIZE][BOARD_SIZE], int piece) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == piece) {
                board[i][j] = 0;
            }
        }
    }
}

char switchToChar(int index) {
   if (index <= NUM_PIECES && index >= 1) {
        return index + 'A' - 1;
   }
   return ' ';
}

void printBoard(int board[BOARD_SIZE][BOARD_SIZE]) {
    printf("  0 1 2 3 4 5\n");
    for (int row = 0; row < BOARD_SIZE; row++) {
        printf("%d ", row);
        for (int col = 0; col < BOARD_SIZE; col++) {
            printf("%c ", switchToChar(board[row][col]));
        }
        printf("\n");
    }
    printf("\n");
}

void printPiece(PuzzlePiece piece, char name) {
    //printf("Piece %c ", name);
    int count = 0;
    for (int i = 0; i < piece.height; i++) {
        for (int j = 0; j < piece.width; j++) {
            if (count == piece.empty) {
                printf("  ");
                count++;
                continue;
            }
            printf("%c ", name);
            count++;
        }
        //printf("\n      ");
        printf("\n");
    }
    printf("\n");
}

bool isBoardFull(int board[BOARD_SIZE][BOARD_SIZE]) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if (board[row][col] == 0)
                return false;
        }
    }
    return true;
}

bool solvePuzzle(int board[BOARD_SIZE][BOARD_SIZE], PuzzlePiece pieces[], int index, int* solutionCount, int possible_board[MAX_SOLUTIONS][BOARD_SIZE][BOARD_SIZE]) {
    if (index == NUM_PIECES) {
      //  printf("all pieces index %d \n", index);
        //printBoard(board);
        if (isBoardFull(board)) {
            if (*solutionCount < MAX_SOLUTIONS) {
                duplicateBoard(board, possible_board[*solutionCount]);
                (*solutionCount)++;
            }

            if (*solutionCount == MAX_SOLUTIONS){
                return true;
            }
        }
        return false;
    }



    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            for (int rt = 0; rt < 4; rt++) {
                if (canPlacePiece(board, pieces[index], row, col, rt)) {
                    placePiece(board, pieces[index], row, col, index + 1, rt);
                    //if (index == 7) {
                    //     printBoard(board);
                    //}
                   // return true;
                    if (solvePuzzle(board, pieces, index + 1, solutionCount, possible_board)) {
                        return true;
                    }
                    removePiece(board, pieces[index], row, col, rt);
                }
                if (pieces[index].height == pieces[index]. width && pieces[index].empty < 0) {
                    break;
                }
                if (pieces[index].empty < 0 && rt == 1) {
                    break;
                }

            }

        }
    }

    return false;
}


void randomlyPickBoard(int possible_board[MAX_SOLUTIONS][BOARD_SIZE][BOARD_SIZE], int question[BOARD_SIZE][BOARD_SIZE], int noOfSolution) {
    int r = rand() % noOfSolution;
    printf("The %d board is:  \n", r);
    duplicateBoard(possible_board[r], question);
}

bool foundEleChar(char array[], char letter, int size) {
    for (int i = 0; i < size; i++) {
        //printf("%c", array[i]);
        if (array[i] == letter) {
            return true;
        }
    }
    return false;
}


void removeEle(char array[], char letter, int size) {
    bool replace = false;
    for (int i = 0; i < size; i++) {
        if (array[i] == letter) {
            replace = true;
        }
        if (replace) {
            array[i] = array[i + 1];
        }
    }
}

void setUpGame(int question[BOARD_SIZE][BOARD_SIZE], char missing[],  int noOfRemove) {
    int count = 0;
    while (count != noOfRemove) {
        int r = rand() % NUM_PIECES + 1;
        while (foundEleChar(missing, switchToChar(r), count)) {
            r = rand() % NUM_PIECES + 1;
            //printf("%d ", r);
        }
        removePieceSec(question, r);
        missing[count] = switchToChar(r);
        //printf("Piece %c [%d, %d] \n", switchToChar(r), pieces[r - 1].height, pieces[r - 1].width);
        //r = rand() % NUM_PIECES + 1;
        //removePieceSec(question, r);
        //missing[1] = switchToChar(r);
        count++;
    }

}

int main() {
    srand(time(NULL));
    int board[BOARD_SIZE][BOARD_SIZE] = {0};
    PuzzlePiece pieces[NUM_PIECES] = {{2, 3, 4}, {1, 4, -1}, {3, 2, -1}, {1, 1, -1}, {2, 2, -1}, {5, 1, -1}, {4, 2, -1}, {2, 2, 1}};

    int solutionCount = 0;
    int possible_board[MAX_SOLUTIONS][BOARD_SIZE][BOARD_SIZE];

    if (solvePuzzle(board, pieces, 0, &solutionCount, possible_board)) {
        printf("Maximum number of solutions reached.\n");
    } else {
        printf("Only %d solutions found.\n", solutionCount);
    }


    int question[BOARD_SIZE][BOARD_SIZE];
    randomlyPickBoard(possible_board, question, solutionCount);

    int hp = 7;

    printBoard(question);

    printf("You have to choose how many pieces you want to remove from the board.\nThe pieces to be removed are random.\n");

    int no_missing;
    printf("How many pieces do you want to remove from the board?  \n");
    scanf("%d", &no_missing);

    char missing[NUM_PIECES];

    setUpGame(question, missing, no_missing);

    //printf("Piece %c [%d, %d] \n", switchToChar(r), pieces[r - 1].height, pieces[r - 1].width);
    //int no_missing = 2;


    char added[NUM_PIECES + 1];
    int no_added = 0;

    printBoard(question);


    while (!isBoardFull(question)) {

        printf("The following pieces are missing: \n\n");
        for(int i = 0; i < no_missing; i++) {
            printPiece(pieces[missing[i] - 'A'], missing[i]);
            //printf("Piece %c [%d, %d] with empty %d\n", missing[i], pieces[missing[i] - 'A'].height, pieces[missing[i] - 'A'].width, pieces[missing[i] - 'A'].empty);
        }

        int inputRow;
        int inputCol;
        char inputPiece;
        int inputRt;


         printf("Type a the row and the col and piece and press enter: \n");

         scanf("%d %d %c %d", &inputRow, &inputCol, &inputPiece, &inputRt);


        //enum PieceName inputName = A;

        //printf("The piece is %d\n", inputName);
        if (inputRow == -1 && foundEleChar(added, inputPiece, no_added)) {
            removePieceSec(question, inputPiece - 'A' + 1);
            printf("The piece has been removed\n");
            printBoard(question);
            missing[no_missing] = inputPiece;
            no_missing++;
            removeEle(added, inputPiece, no_added);
            no_added--;
            hp -= 1;
            printf("Your HP is now %d\n", hp);
        } else if (canPlacePiece(question, pieces[inputPiece - 'A'], inputRow, inputCol, inputRt) && foundEleChar(missing, inputPiece, no_missing)) {
           printf("Valid\n");
           placePiece(question, pieces[inputPiece - 'A'], inputRow, inputCol, inputPiece - 'A' + 1, inputRt);
           removeEle(missing, inputPiece, no_missing);
           //printf("%c %5c\n", missing[0], missing[1]);
           no_missing--;
           added[no_added] = inputPiece;
           no_added++;

           printf("The new board is: \n");
           printBoard(question);
        } else {
           hp -= 2;
           printf("Invalid input. Your HP is now %d\n", hp);
           printf("The board\n");
           printBoard(question);
        }
        if (hp == 0) {
           break;
        }
    }
    if (hp == 0) {
        printf("You enter invalid input so many times. Game Over");
        return 0;
    }
    printf("You fill in all the spaces!\n");



    return 0;
}
