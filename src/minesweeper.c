#include <stdio.h> /* Gestione di I/O e files */
#include <stdlib.h> /* malloc, free, rand */
#include "minesweeper.h"

/* msw_create crea un nuovo campo vuoto, dati width > 1 e height > 1, assegna
 * il puntatore a *fieldptr (se *fieldptr è un puntatore non nullo, viene prima
 * distrutto il campo riferito da esso) e restituisce vero se la creazione è
 * avvenuta con successo. In caso di errore, nessuna modifica viene apportata a
 * *fieldptr e al campo puntato da esso.
 */
int msw_create(msw_field *fieldptr, int width, int height) {
    msw_field field = NULL;

    /* La dimensione minima del campo è 2x2. */
    if (width > 1 && height > 1) {
        field = (msw_field) malloc(sizeof(struct msw_field_struct));

        if (field) {
            /* Inizializzazione della struttura msw_field_struct. */
            field->grid = (msw_grid) malloc(height * sizeof(msw_cell));
            field->width = width;
            field->height = 0;
            field->mine_cnt = 0;
            field->flag_cnt = 0;
            field->nmnv_cnt = width * height;
            field->instance = 1;
            field->undo_cnt = 0;

            if (field->grid) {
                int i = 0, success = 1;

                while (success && (i < height)) {
                    field->grid[i] = (msw_cell) malloc(width * sizeof(struct msw_cell_struct));

                    if (field->grid[i]) {
                        int j;

                        for (j = 0; j < width; j++) {
                            /* Inizializzazione della struttura msw_cell_struct. */
                            field->grid[i][j].content = CONTENT_EMPTY;
                            field->grid[i][j].visited = VISITED_NO;
                        }

                        /* field->height viene incrementato ogni volta che viene allocata una nuova riga
                         * della griglia, fino a raggiungere il valore di height. In caso di errore di
                         * malloc, solo le prime field->height devono essere deallocate.
                         */
                        field->height++;
                    } else
                        success = 0;

                    i++;
                }

                if (success) {
                    /* Distruzione del precedente campo puntato da *fieldptr e sostituzione con il
                     * puntatore al campo appena creato. */
                    msw_destroy(fieldptr);
                    *fieldptr = field;

                    return 1;
                }
            }
        }
    }

    msw_destroy(&field);

    return 0;
}

/* msw_destroy distrugge un campo precedentemente creato. */
void msw_destroy(msw_field *fieldptr) {
    if (*fieldptr) {
        msw_field field = *fieldptr;

        if (field->grid) {
            int i;

            /* Solo le prime field->height righe della griglia devono essere deallocate. */
            for (i = 0; i < field->height; i++)
                free(field->grid[i]);

            free(field->grid);
        }

        free(field);

        *fieldptr = NULL;
    }
}

/* msw_cell_exists verifica se esiste la cella alla posizione (x, y). */
int msw_cell_exists(msw_field field, int x, int y) {
    return (x >= 0 && x < field->width && y >= 0 && y < field->height);
}

/* msw_get_cell restituisce il puntatore della cella alla posizione (x, y),
 * altrimenti NULL se non esiste.
 */
msw_cell msw_get_cell(msw_field field, int x, int y) {
    if (msw_cell_exists(field, x, y))
        return field->grid[y] + x;
    return NULL;
}

/* msw_mine_cell piazza una mina sulla (x, y) cella esistente e restituisce
 * vero se l'operazione è avvenuta con successo.
 */
int msw_mine_cell(msw_field field, int x, int y) {
    if (msw_cell_exists(field, x, y)) {
        if (msw_get_cell(field, x, y)->content != CONTENT_MINE) {
            int x0, y0;

            /* Incremento del numero contenuto in tutte le celle adiacenti alla cella (x, y)
             * non contenenti una mina. In realtà, è compresa anche la cella centrale
             * nell'incremento...
             */
            for (y0 = -1; y0 <= 1; y0++)
                for (x0 = -1; x0 <= 1; x0++) {
                    if (msw_cell_exists(field, x + x0, y + y0) &&
                        msw_get_cell(field, x + x0, y + y0)->content != CONTENT_MINE)

                        msw_get_cell(field, x + x0, y + y0)->content++;
                }

            /* ... ma adesso contiene una mina. */
            msw_get_cell(field, x, y)->content = CONTENT_MINE;
            field->mine_cnt++;
            field->nmnv_cnt--;
        }

        return 1;
    }

    return 0;
}

/* msw_create_random crea un nuovo campo con le stesse modalità di msw_create,
 * eccetto per il fatto che vengono piazzate le mine nel campo in modo casuale
 * con la funzione rand (il seed deve essere prima inizializzato).
 */
int msw_create_random(msw_field *fieldptr, int width, int height, int mines) {
    msw_field field = NULL;

    /* Almeno una cella del campo deve contenere una mina e almeno una cella non
     * deve contenere una mina.
     */
    if ((mines >= 1 && mines < (width * height)) && msw_create(&field, width, height)) {
        int cells = width * height;

        /* Per ogni mina da piazzare, selezionare casualmente l'indice n tra tutti gli
         * indici delle celle ancora senza mina; la mina deve quindi essere piazzata
         * all'n-esima cella senza mina.
         */
        while (mines > 0) {
            int n = rand() % cells, x = 0, y = 0;

            while (n > 0 || msw_get_cell(field, x, y)->content == CONTENT_MINE) {
                if (msw_get_cell(field, x, y)->content != CONTENT_MINE)
                    n--;
                if (++x >= field->width) {
                    x = 0;
                    y++;
                }
            }

            msw_mine_cell(field, x, y);

            mines--;
            cells--;
        }

        msw_destroy(fieldptr);
        *fieldptr = field;

        return 1;
    }

    return 0;
}

/* msw_create_from_file crea un nuovo campo con le stesse modalità di
 * msw_create, eccetto per il fatto che lo schema (dimensione di esso e
 * posizione delle mine) vengono letti dal file descritto da *fileptr, il cui
 * formato di ogni riga è "a,b".
 */
int msw_create_from_file(msw_field *fieldptr, FILE *fileptr) {
    msw_field field = NULL;
    int width, height, mines = 0;
    int success = 1;
    char *line = NULL;
    size_t bytes_alloc = 0;
    ssize_t bytes_read;

    bytes_read = getline(&line, &bytes_alloc, fileptr);
    while (success && (bytes_read != -1)) {
        if (line[0] != '\n') {
            int a, b;

            /* La riga deve rispettare il formato "a,b", con a e b numeri interi. */
            if (sscanf(line, "%d,%d", &a, &b) == 2) {
                if (!field) {
                    /* Se il campo non è stato ancora creato, è necessario crearlo. */
                    width = a;
                    height = b;
                    success = msw_create(&field, width, height);
                }  else {
                    /* Se il campo è già stato creato, piazzare una mina. */
                    success = msw_mine_cell(field, a, b);
                    mines++;
                }
            } else
                success = 0;
        }

        if (success)
            bytes_read = getline(&line, &bytes_alloc, fileptr);
    }

    free(line);

    if (success) {
        /* Almeno una cella del campo deve contenere una mina e almeno una cella non
         * deve contenere una mina.
         */
        if (mines >= 1 && mines < (width * height)) {
            msw_destroy(fieldptr);
            *fieldptr = field;

            return 1;
        }
    }

    if (field)
        msw_destroy(&field);

    return 0;
}

/* msw_write_to_file scrive lo schema (dimensione di esso e posizione delle
 * mine) sul file descritto da *fileptr, il cui formato di ogni riga è
 * "a,b" e restituisce vero se la scrittura è avvenuta con successo.
 */
int msw_write_to_file(msw_field field, FILE *fileptr) {
    int success;

    /* Scrittura della dimensione dello schema. */
    success = (fprintf(fileptr, "%d, %d\n\n", field->width, field->height) >= 0);

    if (success) {
        int y = 0;

        /* Per ogni cella del campo... */
        while (success && (y < field->height)) {
            int x = 0;

            while (success && (x < field->width)) {
                /* Scrittura della posizione della cella, se questa contiene
                 * una mina.
                 */
                if (msw_get_cell(field, x, y)->content == CONTENT_MINE)
                    success = (fprintf(fileptr, "%d, %d\n", x, y) >= 0);

                x++;
            }

            y++;
        }
    }

    return success;
}

/* msw_mark_cell marca/demarca la cella (x, y) con una bandiera, se non
 * visitata, e restituisce vero se la modifica è avvenuta con successo.
 */
int msw_mark_cell(msw_field field, int x, int y) {
    if (msw_cell_exists(field, x, y)) {
        int *visited = &(msw_get_cell(field, x, y)->visited);

        if (*visited == VISITED_NO) {
            *visited = VISITED_FLAG;
            field->flag_cnt++;

            return 1;
        } else if (*visited == VISITED_FLAG) {
            *visited = VISITED_NO;
            field->flag_cnt--;

            return 1;
        }
    }

    return 0;
}

/* msw_mark_mine_cells marca tutte le celle contenenti una mina con una
 * bandiera.
 */
void msw_mark_mine_cells(msw_field field) {
    int x, y;

    for (y = 0; y < field->height; y++)
        for (x = 0; x < field->width; x++) {
            msw_cell cell = msw_get_cell(field, x, y);

            if (cell->content == CONTENT_MINE)
                cell->visited = VISITED_FLAG;
        }
}

/* msw_visit_adjacent_cells visita le celle adiacenti alla cella (x, y), se
 * non visitate e non marcate, e restituisce una costante che indica se, dopo
 * la visita, il risultato è la sconfitta (la cella contiene una mina) oppure
 * la semplice visita di una cella non contenente una mina.
 * msw_visit_adjacent è una funzione ausiliaria di msw_select_cell, quindi non
 * dovrebbe essere richiamata altrove.
 */
int msw_visit_adjacent_cells(msw_field field, int x, int y) {
    if (msw_cell_exists(field, x, y)) {
        msw_cell cell = msw_get_cell(field, x, y);

        /* Se la cella è non visitata e non marcata... */
        if (cell->visited == VISITED_NO) {
            /* La cella è stata visitata all'istanza corrente. */
            cell->visited = field->instance;

            /* Se la cella contiene una mina, allora sconfitta. */
            if (cell->content == CONTENT_MINE)
                return RESULT_DEFEAT;

            /* Altrimenti, si tratta di un passo in più verso la vittoria. */
            field->nmnv_cnt--;

            /* Se la cella è vuota, applicazione ricorsiva a tutte le celle adiacenti. */
            if (cell->content == CONTENT_EMPTY) {
                int x0, y0;

                for (y0 = -1; y0 <= 1; y0++)
                    for (x0 = -1; x0 <= 1; x0++) {
                        if (x0 != 0 || y0 != 0)
                            msw_visit_adjacent_cells(field, x + x0, y + y0);
                    }
            }

            /* Se la cella non contiene una mina e non è vuota, allora contiene un numero e
             * la ricorsione si interrompe. Durante la ricorsione, non è possibile incontare
             * celle contenenti una mina, poiché queste sono tutte circondate da celle
             * contenenti numeri.
             */

            return RESULT_VISITED;
        }
    }

    return 0;
}

/* msw_select_cell seleziona la cella (x, y), se non visitata e non marcata, e
 * restituisce una costante che indica se, dopo la visita, il risultato è la
 * sconfitta (la cella contiene una mina), la vittoria (tutte le celle non
 * contenenti una mina sono state visitate) oppure la semplice visita di una
 * cella non contenente una mina.
 */
int msw_select_cell(msw_field field, int x, int y) {
    if (msw_cell_exists(field, x, y)) {
        /* Se la cella è non visitata e non marcata... */
        if (msw_get_cell(field, x, y)->visited == VISITED_NO) {
            /* Valutazione del risultato della visita delle celle adiacenti. */
            int result = msw_visit_adjacent_cells(field, x, y);

            /* Se tutte le celle contenenti una mina sono state visitate, allora vittoria. */
            if (field->nmnv_cnt == 0)
                result = RESULT_VICTORY;

            /* Incremento dell'istanza corrente. */
            field->instance++;

            return result;
        }
    }

    return 0;
}

/* msw_undo annulla le ultime times mosse e restituisce vero se la modifica è
 * avvenuta con successo.
 */
int msw_undo(msw_field field, int times) {
    if (times > 0) {
        int x, y;

        field->instance -= times;

        /* Non si può andare indietro rispetto l'istanza 1. */
        if (field->instance < 1)
            field->instance = 1;

        /* Per ogni cella del campo... */
        for (y = 0; y < field->height; y++)
            for (x = 0; x < field->width; x++) {
                msw_cell cell = msw_get_cell(field, x, y);

                /* Se la cella è stata visitata "nel futuro", retrocessione a
                 * cella non visitata.
                 */
                if (cell->visited >= field->instance) {
                    cell->visited = VISITED_NO;
                    if (cell->content != CONTENT_MINE)
                        field->nmnv_cnt++;
                }
            }

        field->undo_cnt++;

        return 1;
    }

    return 0;
}

/* msw_undo_incremental annulla le ultime n mosse, dove n è un numero che viene
 * incrementato ogni volta che viene richiamata msw_undo o
 * msw_undo_incremental, e restituisce vero se la modifica è avvenuta con
 * successo.
 */
int msw_undo_incremental(msw_field field) {
    return msw_undo(field, field->undo_cnt + 1);
}
