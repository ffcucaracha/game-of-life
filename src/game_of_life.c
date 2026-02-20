#include <ncurses.h>
#include <stdio.h>
#include <string.h>

#define WIDTH 80
#define HEIGHT 25
#define MIN_DELAY 20
#define MAX_DELAY 1000
#define DELAY_STEP 20

typedef struct {
    unsigned char cells[HEIGHT][WIDTH];
    unsigned char next[HEIGHT][WIDTH];
    int delay_ms;
    unsigned long generation;
} Game;

static int is_alive_char(int ch) {
    return ch == '1' || ch == 'O' || ch == 'o' || ch == 'X' || ch == '*' || ch == '#';
}

static void clear_field(unsigned char field[HEIGHT][WIDTH]) {
    memset(field, 0, HEIGHT * WIDTH * sizeof(unsigned char));
}

static void init_game(Game *game) {
    clear_field(game->cells);
    clear_field(game->next);
    game->delay_ms = 200;
    game->generation = 0;
}

static void load_from_stdin(Game *game) {
    int row = 0;
    int col = 0;
    int ch = 0;

    while (row < HEIGHT && (ch = getchar()) != EOF) {
        if (ch == '\n' || ch == '\r') {
            continue;
        }
        game->cells[row][col] = (unsigned char)is_alive_char(ch);
        col++;
        if (col == WIDTH) {
            col = 0;
            row++;
        }
    }
}

static int wrap_index(int value, int bound) {
    if (value < 0) {
        return bound - 1;
    }
    if (value >= bound) {
        return 0;
    }
    return value;
}

static int count_neighbors(unsigned char field[HEIGHT][WIDTH], int row, int col) {
    int dr = 0;
    int dc = 0;
    int count = 0;

    for (dr = -1; dr <= 1; dr++) {
        for (dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) {
                continue;
            }
            count += field[wrap_index(row + dr, HEIGHT)][wrap_index(col + dc, WIDTH)];
        }
    }
    return count;
}

static unsigned char next_cell_state(unsigned char current, int neighbors) {
    if (current) {
        return (unsigned char)(neighbors == 2 || neighbors == 3);
    }
    return (unsigned char)(neighbors == 3);
}

static void step_game(Game *game) {
    int row = 0;
    int col = 0;

    for (row = 0; row < HEIGHT; row++) {
        for (col = 0; col < WIDTH; col++) {
            game->next[row][col] =
                next_cell_state(game->cells[row][col], count_neighbors(game->cells, row, col));
        }
    }
    memcpy(game->cells, game->next, HEIGHT * WIDTH * sizeof(unsigned char));
    game->generation++;
}

static void draw_cell(int row, int col, unsigned char alive) {
    mvaddch(row, col, alive ? 'O' : ' ');
}

static void draw_board(Game *game) {
    int row = 0;
    int col = 0;

    for (row = 0; row < HEIGHT; row++) {
        for (col = 0; col < WIDTH; col++) {
            draw_cell(row, col, game->cells[row][col]);
        }
    }
    mvprintw(HEIGHT, 0,
             "A/Z speed +/- | Space exit | delay: %d ms | generation: %lu",
             game->delay_ms,
             game->generation);
    refresh();
}

static void increase_speed(Game *game) {
    if (game->delay_ms > MIN_DELAY) {
        game->delay_ms -= DELAY_STEP;
    }
}

static void decrease_speed(Game *game) {
    if (game->delay_ms < MAX_DELAY) {
        game->delay_ms += DELAY_STEP;
    }
}

static int process_input(Game *game) {
    int key = getch();

    if (key == 'a' || key == 'A') {
        increase_speed(game);
    } else if (key == 'z' || key == 'Z') {
        decrease_speed(game);
    } else if (key == ' ') {
        return 0;
    }
    return 1;
}

static void run_loop(Game *game) {
    int running = 1;

    while (running) {
        timeout(game->delay_ms);
        draw_board(game);
        running = process_input(game);
        if (!running) {
            break;
        }
        step_game(game);
    }
}

int main(void) {
    Game game;

    init_game(&game);
    load_from_stdin(&game);

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, 1);

    run_loop(&game);

    endwin();
    return 0;
}
