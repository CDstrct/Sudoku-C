#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

typedef struct {
    int size;
    int box_size;
    int **board;
    int **solution;
    int **fixed;
} Sudoku;

int **allocate_board(int size) {
    int **board = malloc(size * sizeof(int *));
    for (int i = 0; i < size; i++)
        board[i] = calloc(size, sizeof(int));
    return board;
}

void free_board(int **board, int size) {
    for (int i = 0; i < size; i++)
        free(board[i]);
    free(board);
}

void free_sudoku(Sudoku *sudoku) {
    free_board(sudoku->board, sudoku->size);
    free_board(sudoku->solution, sudoku->size);
    free_board(sudoku->fixed, sudoku->size);
}

int is_valid(Sudoku *sudoku, int **board, int row, int col, int num) {
    for (int i = 0; i < sudoku->size; i++) {
        if (board[row][i] == num || board[i][col] == num)
            return 0;
    }
    int start_row = row - row % sudoku->box_size;
    int start_col = col - col % sudoku->box_size;
    for (int i = 0; i < sudoku->box_size; i++)
        for (int j = 0; j < sudoku->box_size; j++)
            if (board[start_row + i][start_col + j] == num)
                return 0;
    return 1;
}

int fill_board(Sudoku *sudoku, int **board, int row, int col) {
    if (row == sudoku->size) return 1;

    int next_row = (col == sudoku->size - 1) ? row + 1 : row;
    int next_col = (col + 1) % sudoku->size;

    int *nums = malloc(sudoku->size * sizeof(int));
    for (int i = 0; i < sudoku->size; i++) nums[i] = i + 1;
    for (int i = sudoku->size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = nums[i];
        nums[i] = nums[j];
        nums[j] = temp;
    }

    for (int i = 0; i < sudoku->size; i++) {
        if (is_valid(sudoku, board, row, col, nums[i])) {
            board[row][col] = nums[i];
            if (fill_board(sudoku, board, next_row, next_col)) {
                free(nums);
                return 1;
            }
            board[row][col] = 0;
        }
    }
    free(nums);
    return 0;
}

void remove_cells(Sudoku *sudoku, int clues) {
    int total = sudoku->size * sudoku->size;
    int *positions = malloc(total * sizeof(int));
    for (int i = 0; i < total; i++) positions[i] = i;

    for (int i = total - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = positions[i];
        positions[i] = positions[j];
        positions[j] = tmp;
    }

    for (int i = 0; i < total - clues; i++) {
        int pos = positions[i];
        int r = pos / sudoku->size;
        int c = pos % sudoku->size;
        sudoku->board[r][c] = 0;
        sudoku->fixed[r][c] = 0;
    }

    free(positions);
}
//zapisuje rozwiązanie 
void copy_solution(Sudoku *sudoku) {
    for (int i = 0; i < sudoku->size; i++)
        for (int j = 0; j < sudoku->size; j++)
            sudoku->solution[i][j] = sudoku->board[i][j];
}

void display_board(Sudoku *sudoku) {
    printf("    ");
    for (int i = 0; i < sudoku->size; i++) {
        printf("%2d ", i + 1);
        if ((i + 1) % sudoku->box_size == 0 && i != sudoku->size - 1)
            printf("|");
    }
    printf("\n");

    for (int i = 0; i < sudoku->size; i++) {
        printf("%2d |", i + 1);
        for (int j = 0; j < sudoku->size; j++) {
            if (sudoku->board[i][j] == 0)
                printf(" . ");
            else
                printf(" %d ", sudoku->board[i][j]);
            if ((j + 1) % sudoku->box_size == 0 && j != sudoku->size - 1)
                printf("|");
        }
        printf("\n");
        if ((i + 1 ) % sudoku->box_size == 0 && i != sudoku->size - 1) {
            printf("   ");
            for(int k = 0; k < sudoku->size + sudoku->box_size - 1; k++)
                printf("---");
            printf("\n");
        }
    }
}


void player_move(Sudoku *sudoku) {
    int r, c, val;
    printf("Podaj wiersz, kolumnę i wartość (0 = usuwanie, np. 1 1 5): ");
    if (scanf("%d %d %d", &r, &c, &val) != 3) {
        printf("Błędne dane wejściowe!\n");
        while (getchar() != '\n');
        return;
    }
    r--; c--;
    if (r >= 0 && r < sudoku->size && c >= 0 && c < sudoku->size && sudoku->fixed[r][c] == 0) {
        if (val == 0) {
            sudoku->board[r][c] = 0;
        } else if (is_valid(sudoku, sudoku->board, r, c, val)) {
            sudoku->board[r][c] = val;
        } else {
            printf("Niepoprawny ruch!\n");
        }
    } else {
        printf("Nie można zmienić tej komórki.\n");
    }
}
//Wyszukiwanie pustych pól

int is_complete(Sudoku *sudoku) {
    for (int i = 0; i < sudoku->size; i++)
        for (int j = 0; j < sudoku->size; j++)
            if (sudoku->board[i][j] == 0)
                return 0;
    return 1;
}
//sprawdzanie poprawności wyniku

int is_correct_solution(Sudoku *sudoku) {
    for (int i = 0; i < sudoku->size; i++) {
        for (int j = 0; j < sudoku->size; j++) {
            int val = sudoku->board[i][j];
            sudoku->board[i][j] = 0;
            if (!is_valid(sudoku, sudoku->board, i, j, val)) {
                sudoku->board[i][j] = val;
                return 0;
            }
            sudoku->board[i][j] = val;
        }
    }
    return 1;
}

void start_game(int size, int clues) {
    Sudoku sudoku;
    sudoku.size = size;
    sudoku.box_size = (int)sqrt(size);

    sudoku.board = allocate_board(size);
    sudoku.solution = allocate_board(size);
    sudoku.fixed = allocate_board(size);

    srand(time(NULL));
    fill_board(&sudoku, sudoku.board, 0, 0);
    copy_solution(&sudoku);
    remove_cells(&sudoku, clues);

    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            sudoku.fixed[i][j] = (sudoku.board[i][j] != 0) ? 1 : 0;

    while (!is_complete(&sudoku)) {
        display_board(&sudoku);
        player_move(&sudoku);
    }

    display_board(&sudoku);
    if (is_correct_solution(&sudoku)) {
        printf("Gratulacje! Rozwiązałeś Sudoku!\n");
    } else {
        printf("Plansza została uzupełniona, ale zawiera błędy!\n");
    }

    free_sudoku(&sudoku);
}

void menu() {
    int choice;
    while (1) {
        printf("\n==== MENU ====\n");
        printf("1. Nowa gra\n");
        printf("2. Instrukcja\n");
        printf("3. Wyjście\n");
        printf("Wybierz opcję: ");
        if (scanf("%d", &choice) != 1) {
            printf("Niepoprawne dane!\n");
            while (getchar() != '\n');
            continue;
        }

        if (choice == 1) {
            int size_opt, diff_opt;
            printf("Wybierz rozmiar planszy:\n1. 4x4\n2. 9x9\n");
            scanf("%d", &size_opt);
            int size = (size_opt == 1) ? 4 : 9;
// Poziom trudności = liczba podpowiedzi pozostawionych na planszy
            printf("Poziom trudności:\n1. Łatwy\n2. Średni\n3. Trudny\n");
            scanf("%d", &diff_opt);
            int clues = (diff_opt == 1) ? (size * size * 0.6) :
                        (diff_opt == 2) ? (size * size * 0.45) :
                        (size * size * 0.3);

            start_game(size, clues);
        } else if (choice == 2) {
            printf("\nCelem gry jest uzupełnienie planszy liczbami tak,\naby w każdym wierszu, kolumnie i kwadracie\nwystępowały wszystkie liczby od 1 do n bez powtórzeń.\n(0 usuwa liczbę z wybranego pola)\n");
        } else if (choice == 3) {
            break;
        } else {
            printf("Niepoprawny wybór!\n");
        }
    }
}

int main() {
    menu();
    return 0;
}