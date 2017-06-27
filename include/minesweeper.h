#ifndef __MINESWEEPER_H__
#define __MINESWEEPER_H__

#include <stdio.h> /* Gestione di files */

/* Costanti assegnabili a msw_cell_struct.content. */
#define CONTENT_EMPTY 0
#define CONTENT_MINE -1

/* Costanti assegnabili a msw_cell_struct.visited. */
#define VISITED_NO 0
#define VISITED_FLAG -1

/* Costanti per il risultato della selezione di una cella. */
#define RESULT_VISITED 1
#define RESULT_DEFEAT 2
#define RESULT_VICTORY 3

/* La struttura che rappresenta una cella.
 *
 * content
 *     Il contenuto della cella. Sono assegnabili i seguenti valori:
 *      * CONTENT_MINE se la cella contiene una mina;
 *      * un numero n = [1..8] se la cella non contiene una mina ed è
 *        circordata da n mine, oppure CONTENT_EMPTY se non è circondata da
 *        alcuna mina).
 *
 * visited
 *     Il valore che indica se la cella è stata:
 *      * visitata, dunque l'istanza in cui è avvenuta la visita;
 *      * non visitata, dunque il valore VISITED_NO;
 *      * non visitata e marcata con una bandiera, dunque il valore VISITED_FLAG.
 */
struct msw_cell_struct {
    int content;
    int visited;
};

typedef struct msw_cell_struct *msw_cell, **msw_grid;

/* La struttura che rappresenta un campo.
 *
 * grid
 *     La griglia del campo, la cui dimensione è width * height.
 *
 * width
 *     La lunghezza della griglia.
 *
 * height
 *     L'altezza della griglia.
 *
 * mine_cnt
 *     Il numero di celle contenenti una mina presenti nel campo.
 *
 * flag_cnt
 *     Il numero di celle segnate con una bandiera presenti nel campo.
 *
 * nmnv_cnt
 *     Il numero di celle senza mine e non visitate (Not Mine and Not Visited).
 *     Dopo la selezione di una cella, se il valore di nmnv_cnt è uguale a
 *     zero, la partita termina con la vittoria.
 *
 * instance
 *     L'istanza corrente del campo (comincia da 1 e viene incrementata ad ogni
 *     selezione).
 *
 * undo_cnt
 *     Il numero di annullamenti effettuati.
 */
struct msw_field_struct {
    msw_grid grid;
    int width, height, mine_cnt, flag_cnt, nmnv_cnt, instance, undo_cnt;
};

typedef struct msw_field_struct *msw_field;

int msw_create(msw_field*, int, int);

void msw_destroy(msw_field*);

int msw_cell_exists(msw_field, int, int);

msw_cell msw_get_cell(msw_field, int, int);

int msw_mine_cell(msw_field, int, int);

int msw_create_random(msw_field*, int, int, int);

int msw_create_from_file(msw_field*, FILE*);

int msw_write_to_file(msw_field, FILE*);

int msw_mark_cell(msw_field, int, int);

void msw_mark_mine_cells(msw_field);

int msw_visit_adjacent_cells(msw_field, int, int);

int msw_select_cell(msw_field, int, int);

int msw_undo(msw_field, int);

int msw_undo_incremental(msw_field);

#endif /* __MINESWEEPER_H__ */
