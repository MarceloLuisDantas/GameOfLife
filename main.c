#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <stdbool.h>
#include <unistd.h>

#define WIDTH 200
#define HEIGHT 50

// Any live cell with fewer than two live neighbours dies, as if by underpopulation.
// Any live cell with two or three live neighbours lives on to the next generation.
// Any live cell with more than three live neighbours dies, as if by overpopulation.
// Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
#define MIN_NEI 2
#define MAX_NEI 3
#define BORNS 3

#define CHANCE_TO_BORN 7 // 10% born when the game start

typedef struct Board {
    char **board;
    char **buffer; // Buffer para gravar a proxima geração
} Board;

Board *create_board() {
    Board *b = malloc(sizeof(Board));
    b->board = malloc(sizeof(char*) * WIDTH);
    for (size_t i = 0; i < WIDTH; i++)
        b->board[i] = malloc(sizeof(char*) * HEIGHT);

    b->buffer = malloc(sizeof(char*) * WIDTH);
    for (size_t i = 0; i < WIDTH; i++)
        b->buffer[i] = malloc(sizeof(char*) * HEIGHT);
    return b;
}

void populate(Board *b) {
    for (size_t i = 0; i < WIDTH; i++) {
        for (size_t j = 0; j < HEIGHT; j++) {
            int will_born = (rand() % 100) + 1;
            if (will_born <= CHANCE_TO_BORN)
                b->board[i][j] = '#'; 
            else
                b->board[i][j] = ' '; 
        }
    }
}

bool alive(Board *b, int x, int y) {
    if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT)
        return 0;
    return (b->board[x][y] == '#');
}

//       |    (x-1)   |   ( x )  |    (x+1)
// (y-1) | (y-1)(x-1) | (y-1)(x) | (y-1)(x+1)
// ( y ) |   (y)(x-1) |   (x)(y) |   (y)(x+1)
// (y+1) | (y+1)(x-1) | (y+1)(x) | (y+1)(x+1)
int neighbours(Board *b, int x, int y) {
    return (
        alive(b, x-1, y-1) + alive(b, x, y-1) + alive(b, x+1, y-1) + 
        alive(b, x-1, y)                      + alive(b, x+1, y) + 
        alive(b, x-1, y+1) + alive(b, x, y+1) + alive(b, x+1, y+1)
    );
}

void next_gen(Board *b) {
    for (size_t x = 0; x < WIDTH; x++) {
        for (size_t y = 0; y < HEIGHT; y++) {
            int nei = neighbours(b, x, y);
            if (alive(b, x, y)) {
                if (nei < MIN_NEI || nei > MAX_NEI)
                    b->buffer[x][y] = '-';
            } else {
                if (nei == BORNS)
                    b->buffer[x][y] = '#';
            }
        }
    }

    for (size_t x = 0; x < WIDTH; x++)
        for (size_t y = 0; y < HEIGHT; y++)
            b->board[x][y] = b->buffer[x][y];
}

void draw_border() {
    // Linhas horizontais
    for (int x = 0; x <= WIDTH + 1; x++) {
        mvaddch(0, x, ACS_HLINE);       // Topo
        mvaddch(HEIGHT + 1, x, ACS_HLINE); // Base
    }

    // Linhas verticais
    for (int y = 0; y <= HEIGHT + 1; y++) {
        mvaddch(y, 0, ACS_VLINE);       // Esquerda
        mvaddch(y, WIDTH + 1, ACS_VLINE); // Direita
    }

    // Cantos
    mvaddch(0, 0, ACS_ULCORNER);          // ┌
    mvaddch(0, WIDTH + 1, ACS_URCORNER);  // ┐
    mvaddch(HEIGHT + 1, 0, ACS_LLCORNER);  // └
    mvaddch(HEIGHT + 1, WIDTH + 1, ACS_LRCORNER); // ┘
    
}

void print_board_to_buffer(Board *b) {
    for (size_t i = 0; i < WIDTH; i++) {
        for (size_t j = 0; j < HEIGHT; j++) {
            mvaddch(j + 1, i + 1, b->board[i][j]);
        }
    }
}

int main(){	
    Board *b = create_board();
    populate(b);    
    
    initscr();			
    
    resizeterm(HEIGHT + 2, WIDTH + 2);
    draw_border();
    for (;;) {
        print_board_to_buffer(b);
        next_gen(b);
        sleep(3);
        refresh();			
    }
    
    endwin();			
    return 0;
}
