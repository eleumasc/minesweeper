#ifndef __UI_H__
#define __UI_H__

#include <ncurses.h> /* Grafica */
#include "minesweeper.h"

/* Lo schermo viene diviso in tre finestre:
 *     1. L'intestazione (titolo);
 *     2. Il corpo (menu, input e campo);
 *     3. Il piè di pagina (informazioni).
 * Il corpo è sempre visibile, le altre due parti sono visibili solamente se
 * l'altezza dello schermo supera un'altezza minima di 16 righe.
 */

/* Costanti per il tipo di finestra. */
#define WND_HEAD 1
#define WND_BODY 2
#define WND_FOOT 3

/* Costanti per la maschera della finestra di informazioni. */
#define INFO_HARROWS 1
#define INFO_VARROWS 2
#define INFO_ENTER_SELECT 4
#define INFO_Q 8
#define INFO_W 16
#define INFO_ENTER_PAUSE 32

/* Costanti per il tipo di menu di gioco. */
#define GMENU_PAUSE 1
#define GMENU_VICTORY 2
#define GMENU_DEFEAT 3

/* Costanti per il tipo di azione scelta da menu. */
#define ACTION_NEW 1
#define ACTION_LOAD 2
#define ACTION_SAVE 3
#define ACTION_QUIT 4
#define ACTION_SELECT 5
#define ACTION_MARK 6
#define ACTION_PAUSE 7
#define ACTION_CONTINUE 8

/* Costanti per i simboli usati per il disegno del campo. */
#define SYMBOL_UNUSED ACS_CKBOARD
#define SYMBOL_VISITED_FLAG '!' | A_BOLD | COLOR_PAIR(2)
#define SYMBOL_VISITED_NO '#' | COLOR_PAIR(1)
#define SYMBOL_CONTENT_EMPTY ' '
#define SYMBOL_CONTENT_MINE '*' | A_BOLD | COLOR_PAIR(3)

/* Costanti degli attributi per i simboli. */
#define A_CONTENT_NUMBER A_BOLD
#define A_CELL_SELECTED A_STANDOUT

void ui_start();

void ui_end();

void ui_set_echo(int);

void ui_sleep(int);

WINDOW* ui_window_size(int, int*, int*);

WINDOW* ui_window(int);

int ui_input_integer(char*);

int ui_input_range(char*, int, int);

char* ui_select(char*, char**, int);

void ui_message(char*);

void ui_title();

void ui_info(int);

int ui_main_menu(int);

int ui_game_menu(int, int);

int ui_minesweeper(msw_field, int*, int*, int);

#endif /* __UI_H__ */
