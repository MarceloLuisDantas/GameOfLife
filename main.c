#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <ncurses.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

// Any live cell with fewer than two live neighbours dies, as if by underpopulation.
// Any live cell with two or three live neighbours lives on to the next generation.
// Any live cell with more than three live neighbours dies, as if by overpopulation.
// Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
#define MIN_NEI 2
#define MAX_NEI 3
#define BORNS 3

#define CHANCE_TO_BORN 7 // 10% born when the game start

int WIDTH  = 0;
int HEIGHT = 0;

int last_edited_x = 1;
int last_edited_y = 1;

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
    for (size_t x = 0; x < WIDTH; x++) {
        for (size_t y = 0; y < HEIGHT; y++) {
            int will_born = (rand() % 100) + 1;
            if (will_born <= CHANCE_TO_BORN)
                b->board[x][y] = '#'; 
            else
                b->board[x][y] = '-'; 
        }
    }

    for (size_t x = 0; x < WIDTH; x++)
        for (size_t y = 0; y < HEIGHT; y++)
            b->buffer[x][y] = b->board[x][y];
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
                else 
                    b->buffer[x][y] = '#';
            } else {
                if (nei == BORNS)
                    b->buffer[x][y] = '#';
                else 
                    b->buffer[x][y] = '-';
            }
        }
    }

    for (size_t x = 0; x < WIDTH; x++)
        for (size_t y = 0; y < HEIGHT; y++)
            b->board[x][y] = b->buffer[x][y];
}

void draw_border_game() {
    // Linhas horizontais
    for (int x = 1; x <= WIDTH; x++) {
        mvaddch(0, x, ACS_HLINE);       // Topo
        mvaddch(HEIGHT + 1, x, ACS_HLINE); // Base
    }

    // Linhas verticais
    for (int y = 1; y <= HEIGHT; y++) {
        mvaddch(y, 0, ACS_VLINE);       // Esquerda
        mvaddch(y, WIDTH + 1, ACS_VLINE); // Direita
    }

    // Cantos
    mvaddch(0, 0, ACS_ULCORNER);         
    mvaddch(0, WIDTH + 1, ACS_URCORNER); 
    mvaddch(HEIGHT + 1, 0, ACS_LLCORNER);
    mvaddch(HEIGHT + 1, WIDTH + 1, ACS_LRCORNER);

    char *msg = "PRESS 'ENTER' TO PAUSE";
    mvprintw(0, 2, "%s", msg);
}

void print_board_to_buffer(Board *b) {
    for (size_t x = 0; x < WIDTH; x++) {
        for (size_t y = 0; y < HEIGHT; y++) {
            if (alive(b, x, y))
                mvprintw(y + 1, x + 1, "#");
            else 
                mvprintw(y + 1, x + 1, " ");
        }
    }
}

void paused(Board *b) {
    nodelay(stdscr, FALSE);

    char *msg = " PRESS 'ENTER' TO UNPAUSE | PRESS 'X' TO SWITCH THE STATE OF A CELL | PRESS 'N' FOR NEXT GENERATION ";
    mvprintw(0, 2, "%s", msg);

    int x = last_edited_x;
    int y = last_edited_y;
    move(last_edited_y, last_edited_x);
    while (true) {
        int ch = getch();			
        switch (ch) {
            case '\n':
                nodelay(stdscr, TRUE);
                last_edited_x = x;
                last_edited_y = y;
                return;
                break;
            case 'x' :
                if (alive(b, x - 1, y - 1))
                    b->board[x - 1][y - 1] = ' ';
                else 
                    b->board[x - 1][y - 1] = '#';
                print_board_to_buffer(b);
                break;
            case 'n' :
                next_gen(b);
                print_board_to_buffer(b);
                break;
            case 'd' :
                char n = neighbours(b, x - 1, y - 1) + '0';
                mvaddch(1, 1, n);
                break;
            case KEY_UP:
                if (y > 1) y--;
                break;
            case KEY_DOWN:
                if (y < HEIGHT) y++;
                break;
            case KEY_LEFT:
                if (x > 1) x--;
                break;
            case KEY_RIGHT:
                if (x < WIDTH) x++;
                break;
        }
        move(y, x);
        refresh();
    }
}

int main(){	
    bool pause = true;
    unsigned int speed = 70000;

    initscr();			
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    noecho();

    getmaxyx(stdscr, HEIGHT, WIDTH);
    HEIGHT = HEIGHT - 2;
    WIDTH = WIDTH - 2;
    
    Board *b = create_board();
    populate(b);  
      
    draw_border_game();

    char ch;
    while (true) {
        print_board_to_buffer(b);
        refresh();
        
        ch = getch();
        if (ch == '\n')
            pause = true;    
        else if (ch == '-') {
            if (speed <= 2000000000) 
                speed += 5000;
            else
                speed = 2000000000;
        } else if (ch == '+' || ch == '=') {
            if (speed > 5000)
                speed -= 5000;
        }

        if (pause) {
            paused(b);
            pause = false;
            char *msg = " PRESS 'ENTER' TO PAUSE | PRESS '-' TO REDUZ SPEED | PRESS '+' TO INCRESS SPEED ";
            mvprintw(0, 2, "%s", msg);
            for (int x = 82; x <= 101; x++)
                mvaddch(0, x, ACS_HLINE); 
        }
        
        usleep(speed);
        next_gen(b);
    }
    
    endwin();			
    return 0;
}
