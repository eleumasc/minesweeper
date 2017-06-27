#include <stdio.h> /* Gestione di files */
#include <stdlib.h> /* srand */
#include <time.h> /* time */
#include "minesweeper.h"
#include "ui.h"
#include "main.h"

/* Il campo minato corrente. */
msw_field field = NULL;

int main() {
    int quit = 0;

    ui_start();

    ui_title();

    do {
        /* Menu principale. */
        int action = ui_main_menu(field != NULL);

        switch (action) {
            case ACTION_NEW: {
                /* Input di dimensioni del campo e numero di mine e msw_create_random. */
                int width, height, mines, success;

                width = ui_input_range("Larghezza del campo", 1, 100);
                height = ui_input_range("Altezza del campo", 1, 100);
                mines = ui_input_range("Numero di mine", 1, width * height - 1);

                srand(time(NULL));
                success = msw_create_random(&field, width, height, mines);

                if (success)
                    game();
                else
                    ui_message("Non sono riuscito a creare il campo.");
            }
            break;
            case ACTION_LOAD: {
                /* Apertura del file SAVE_FILE_NAME per la lettura e msw_create_from_file. */
                FILE *fp = fopen(SAVE_FILE_NAME, "r");

                if (fp != NULL) {
                    int success = msw_create_from_file(&field, fp);
                    fclose(fp);

                    if (success)
                        game();
                    else
                        ui_message("Non sono riuscito a caricare il campo.");
                } else
                    ui_message("Non sono riuscito ad aprire il file di salvataggio per la lettura.");
            }
            break;
            case ACTION_SAVE: {
                /* Apertura del file SAVE_FILE_NAME per la scrittura e msw_write_to_file. */
                FILE *fp = fopen(SAVE_FILE_NAME, "w");

                if (fp != NULL) {
                    int success = msw_write_to_file(field, fp);
                    fclose(fp);

                    if (success)
                        ui_message("Salvataggio completato.");
                    else
                        ui_message("Non sono riuscito a salvare il campo.");
                } else
                    ui_message("Non sono riuscito ad aprire il file di salvataggio per la scrittura.");
            }
            break;
            case ACTION_QUIT:
                quit = 1;
        }
    } while (!quit);

    ui_end();

    /* Distruzione del campo. */
    msw_destroy(&field);

    return 0;
}

/* game è la procedura di gioco. */
void game() {
    int x = 0, y = 0, lives, quit = 0;

    /* Input del numero di vite (tentativi permessi). */
    do {
        lives = ui_input_range("Numero di vite [1,5]", 1, 5);
    } while (lives < 1 || lives > 5);

    do {
        /* Visualizzazione del campo e attesa dell'azione da input. */
        int action = ui_minesweeper(field, &x, &y, 0);

        switch (action) {
            case ACTION_SELECT: {
                /* Selezione della cella (x, y). */
                int result = msw_select_cell(field, x, y);

                if (result == RESULT_VICTORY || result == RESULT_DEFEAT) {
                    /* Marcatura di tutte le celle contenenti una mina se vittoria. */
                    if (result == RESULT_VICTORY)
                        msw_mark_mine_cells(field);

                    /* Visualizzazione finale del campo. */
                    ui_minesweeper(field, &x, &y, 1);
                    ui_sleep(2000);

                    /* Decremento del numero di vite se sconfitta. */
                    if (result == RESULT_DEFEAT)
                        lives--;

                    /* Menu di gioco. */
                    if (ui_game_menu(result == RESULT_VICTORY ? GMENU_VICTORY : GMENU_DEFEAT, lives) == ACTION_CONTINUE)
                        msw_undo_incremental(field);
                    else
                        quit = 1;
                }
            }
            break;
            case ACTION_MARK: {
                /* Marcatura della cella (x, y). */
                msw_mark_cell(field, x, y);
            }
            break;
            case ACTION_PAUSE: {
                /* Menu di gioco. */
                if (ui_game_menu(GMENU_PAUSE, 0) == ACTION_QUIT)
                    quit = 1;
            }
        }
    } while (!quit);
}
