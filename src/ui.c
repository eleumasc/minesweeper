#include <stdio.h> /* sprintf */
#include <ncurses.h> /* Grafica */
#include "minesweeper.h"
#include "ui.h"

/* ui_start inizializza ncurses e l'interfaccia utente. */
void ui_start() {
    initscr();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    ui_set_echo(0);
}

/* ui_end termina l'interfaccia utente. */
void ui_end() {
    erase();
    refresh();
    endwin();
}

/* ui_set_echo imposta la visibilità del cursore e dell'input utente. */
void ui_set_echo(int enable) {
    curs_set(enable ? 1 : 0);
    if (enable)
        echo();
    else
        noecho();
}

/* ui_sleep sospende il processo per ms millisecondi. */
void ui_sleep(int ms) {
    napms(ms);
}

/* ui_window_size crea la finestra del tipo indicato e ne restituisce il
 * puntatore, se compatibile con le dimensioni dello schermo, altrimenti NULL.
 * Se width_ptr oppure height_ptr sono puntatori non nulli, allora le
 * dimensioni della finestra creata vengono salvate rispettivamente in
 * *width_ptr e *height_ptr.
 */
WINDOW* ui_window_size(int wnd_type, int *width_ptr, int *height_ptr) {
    int fill, x, y, width, height;
    WINDOW *wnd;

    fill = (LINES < 16);

    /* Con meno di 16 righe di altezza, solo la finestra WND_BODY è
     * visibile e deve riempire lo schermo.
     */
    if (wnd_type != WND_BODY && fill)
        return NULL;

    /* Calcolo delle dimensioni della finestra WND_BODY, le dimensioni
     * delle altre finestre sono basate sulle sue dimensioni.
     */
    x = 0;
    y = (fill ? 0 : 4);
    width = COLS;
    height = (fill ? LINES : LINES - 8);

    switch (wnd_type) {
        case WND_HEAD:
            y = 0;
            height = 4;
        break;
        case WND_FOOT:
            y = y + height;
            height = 4;
    }

    /* Finestra per disegnare il bordo. */
    wnd = newwin(height, width, y, x);
    box(wnd, ACS_VLINE, ACS_HLINE);
    wrefresh(wnd);
    delwin(wnd);

    /* Calcolo delle dimensioni per la finestra interna destinata al
     * contenuto.
     */
    x++;
    y++;
    width -= 2;
    height -= 2;

    /* Finestra destinata al contenuto. */
    wnd = newwin(height, width, y, x);
    werase(wnd);

    /* Salvataggio di width e height, se richiesto attraverso il passaggio di
     * puntatori non nulli width_ptr e height_ptr.
     */
    if (width_ptr != NULL)
        *width_ptr = width;
    if (height_ptr != NULL)
        *height_ptr = height;

    return wnd;
}

/* ui_window crea la finestra del tipo indicato con le stesse modalità di
 * ui_window_size, ma non viene effettuato il salvataggio delle dimensioni
 * della finestra creata.
 */
WINDOW* ui_window(int wnd_type) {
    return ui_window_size(wnd_type, NULL, NULL);
}

/* ui_input_integer visualizza una finestra che richiede la digitazione di un
 * numero intero e restituisce tale numero.
 */
int ui_input_integer(char *caption) {
    int i, r;
    WINDOW *body = ui_window(WND_BODY);

    ui_info(INFO_ENTER_SELECT);

    ui_set_echo(1);

    do {
        werase(body);
        wprintw(body, "%s\n", caption);
        wrefresh(body);
        r = wscanw(body, "%d", &i);
    } while (r != 1);

    ui_set_echo(0);

    delwin(body);

    return i;
}

/* ui_input_range visualizza una finestra che richiede la digitazione di un
 * intero compreso tra from e to e restituisce tale numero.
 */
int ui_input_range(char *caption, int from, int to) {
    int i;

    do {
        i = ui_input_integer(caption);
    } while (i < from || i > to);

    return i;
}

/* ui_select visualizza una finestra che richiede la selezione di un'opzione
 * tra una lista di opzioni e restituisce il puntatore alla stringa
 * dell'opzione selezionata.
 */
char* ui_select(char *caption, char *options[], int n) {
    int select = 0, confirm = 0;
    WINDOW *body = ui_window(WND_BODY);

    /* Abilitazione dell'ascolto della pressione di tasti speciali (es. tasti
     * direzionali).
     */
    keypad(body, 1);

    ui_info(INFO_VARROWS | INFO_ENTER_SELECT);

    do {
        int i, refresh = 0;

        werase(body);
        wprintw(body, "%s\n\n", caption);

        /* Stampa della lista di opzioni. */
        for (i = 0; i < n; i++)
            wprintw(body, "(%c) %s\n", (i == select ? '*' : ' '), options[i]);

        wrefresh(body);

        do {
            /* Ascolto e gestione della pressione di un tasto. */
            int key = wgetch(body);

            switch (key) {
                case KEY_UP:
                    if (select > 0) {
                        select--;
                        refresh = 1;
                    }
                break;
                case KEY_DOWN:
                    if (select < n - 1) {
                        select++;
                        refresh = 1;
                    }
                break;
                case '\n':
                    confirm = 1;
                    break;
            }
        } while (!confirm && !refresh);
    } while (!confirm);

    delwin(body);

    return options[select];
}

/* ui_message visualizza una finestra di messaggio. */
void ui_message(char *message) {
    char *options[] = { "OK" };
    ui_select(message, options, 1);
}

/* ui_title visualizza la finestra del titolo. */
void ui_title() {
    WINDOW *head = ui_window(WND_HEAD);

    wattron(head, A_BOLD);
    wprintw(head, "MINESWEEPER\n");
    wattroff(head, A_BOLD);
    wprintw(head, "Campo minato di Samuele Casarin");

    wrefresh(head);

    delwin(head);
}

/* ui_info visualizza la finestra delle informazioni. */
void ui_info(int info_mask) {
    WINDOW *foot = ui_window(WND_FOOT);

    wprintw(foot, "| ");

    if ((info_mask & INFO_HARROWS) || (info_mask & INFO_VARROWS)) {
        if (info_mask & INFO_VARROWS)
            wprintw(foot, "Su/Giu");
        if ((info_mask & INFO_HARROWS) && (info_mask & INFO_VARROWS))
            wprintw(foot, "/");
        if (info_mask & INFO_HARROWS)
            wprintw(foot, "Sin./Des.");
        wprintw(foot, ": Muovi | ");
    }

    if (info_mask & INFO_ENTER_SELECT)
        wprintw(foot, "INVIO: Seleziona | ");

    if (info_mask & INFO_Q)
        wprintw(foot, "Q: Seleziona | ");

    if (info_mask & INFO_W)
        wprintw(foot, "W: Marca | ");

    if (info_mask & INFO_ENTER_PAUSE)
        wprintw(foot, "INVIO: Pausa | ");

    wrefresh(foot);

    delwin(foot);
}

/* ui_main_menu visualizza la finestra del menu principale e restituisce una
 * costante che rappresenta l'opzione del menu selezionata.
 */
int ui_main_menu(int save) {
    char *caption = "Cosa vuoi fare?";
    char *options[] = {
        "Nuovo",
        "Carica",
        "Salva",
        "Esci"
    };
    char *option;

    if (save)
        option = ui_select(caption, options, 4);
    else {
        /* L'opzione di salvataggio non deve essere resa disponibile, dunque basta
         * copiare il puntatore alla stringa "Esci" al posto del puntatore alla stringa
         * "Salva" e considerare solamente i primi tre elementi dell'array.
         */
        options[2] = options[3];
        option = ui_select(caption, options, 3);
    }

    if (option == options[0])
        return ACTION_NEW;
    else if (option == options[1])
        return ACTION_LOAD;
    else if (option == options[2] && save)
        return ACTION_SAVE;
    else
        return ACTION_QUIT;
}

/* ui_main_menu visualizza la finestra del menu di gioco del tipo dato e
 * restituisce una costante che rappresenta l'opzione del menu selezionata.
 */
int ui_game_menu(int gmenu_type, int lives) {
    char *options[] = {
        "Continua",
        "Esci"
    };
    char *option;

    switch (gmenu_type) {
        case GMENU_PAUSE:
            option = ui_select("PAUSA", options, 2);
        break;
        case GMENU_VICTORY:
            option = ui_select("HAI VINTO!", options + 1, 1);
        break;
        case GMENU_DEFEAT: {
            if (lives > 0) {
                char caption[32];
                sprintf(caption, "HAI PERSO!\nHai ancora %d vite.", lives);
                option = ui_select(caption, options, 2);
            } else
                option = ui_select("HAI PERSO!\nNon hai più vite...", options + 1, 1);
        }
    }

    if (option == options[0])
        return ACTION_CONTINUE;
    else
        return ACTION_QUIT;
}

/* ui_minesweeper visualizza la finestra del campo e gestisce il controllo
 * dello spostamento del cursore delle celle e restituisce una costante che
 * rappresenta l'azione scelta da input (modifica dello stato del campo oppure
 * pausa).
 * Se draw_only è vero, ui_minesweeper non attende l'input dell'azione, si
 * limita a disegnare il campo e restituisce -1.
 */
int ui_minesweeper(msw_field field, int *x, int *y, int draw_only) {
    int action = 0, w_width, w_height, vb_x = 0, vb_y = 0, vp_x = 0, vp_y = 0, vp_width, vp_height, x0, y0;
    WINDOW *body = ui_window_size(WND_BODY, &w_width, &w_height);

    /* Abilitazione dell'ascolto della pressione di tasti speciali (es. tasti
     * direzionali).
     */
    keypad(body, 1);

    ui_info(INFO_HARROWS | INFO_VARROWS | INFO_Q | INFO_W | INFO_ENTER_PAUSE);

    /* Riempimento della finestra con simboli che rappresentano l'area della
     * finestra non utilizzata. Ad ogni refresh della finestra, verranno
     * riscritti solamente i caratteri all'interno della viewbox.
     */
    for (y0 = 0; y0 < w_height; y0++)
        for (x0 = 0; x0 < w_width; x0++) {
            mvwaddch(body, y0, x0, SYMBOL_UNUSED);
        }

    /* Regolazione dell'area della finestra destinata alla visuale del campo
     * (viewbox) e, in parte, dell'area visibile del campo (viewport).
     */
    if (field->width <= w_width) {
        vb_x = w_width / 2 - field->width / 2;
        vp_width = field->width;
    } else
        vp_width = w_width;

    if (field->height <= w_height) {
        vb_y = w_height / 2 - field->height / 2;
        vp_height = field->height;
    } else
        vp_height = w_height;

    do {
        int refresh = 0;

        /* Regolazione dell'area visibile del campo (viewport). */
        if (vp_width < field->width) {
            vp_x = *x - vp_width / 2;
            if (vp_x < 0)
                vp_x = 0;
            else if (vp_x + vp_width > field->width)
                vp_x = field->width - vp_width;
        }

        if (vp_height < field->height) {
            vp_y = *y - vp_height / 2;
            if (vp_y < 0)
                vp_y = 0;
            else if (vp_y + vp_height > field->height)
                vp_y = field->height - vp_height;
        }

        /* Disegno della parte di campo all'interno dell'area visibile. */
        for (y0 = 0; y0 < vp_height; y0++)
            for (x0 = 0; x0 < vp_width; x0++) {
                msw_cell cell = msw_get_cell(field, vp_x + x0, vp_y + y0);
                int symbol;

                if (cell->visited == VISITED_NO)
                    symbol = SYMBOL_VISITED_NO;
                else if (cell->visited == VISITED_FLAG)
                    symbol = SYMBOL_VISITED_FLAG;
                else if (cell->content == CONTENT_EMPTY)
                    symbol = SYMBOL_CONTENT_EMPTY;
                else if (cell->content == CONTENT_MINE)
                    symbol = SYMBOL_CONTENT_MINE;
                else
                    symbol = ((cell->content + '0') | A_CONTENT_NUMBER);

                mvwaddch(body, vb_y + y0, vb_x + x0, symbol | (vp_x + x0 == *x && vp_y + y0 == *y ? A_CELL_SELECTED : 0));
            }

        wrefresh(body);

        /* Se draw_only è vero, salto dell'input dell'azione. */
        if (!draw_only)
            do {
                /* Ascolto e gestione della pressione di un tasto. */
                int key = wgetch(body);

                switch (key) {
                    case KEY_LEFT:
                        if (*x > 0) {
                            (*x)--;
                            refresh = 1;
                        }
                    break;
                    case KEY_RIGHT:
                        if (*x < field->width - 1) {
                            (*x)++;
                            refresh = 1;
                        }
                    break;
                    case KEY_UP:
                        if (*y > 0) {
                            (*y)--;
                            refresh = 1;
                        }
                    break;
                    case KEY_DOWN:
                        if (*y < field->height - 1) {
                            (*y)++;
                            refresh = 1;
                        }
                    break;
                    case 'q':
                    case 'Q':
                        action = ACTION_SELECT;
                    break;
                    case 'w':
                    case 'W':
                        action = ACTION_MARK;
                    break;
                    case '\n':
                        action = ACTION_PAUSE;
                }
            } while (!action && !refresh);
        else
            action = -1;
    } while (!action);

    delwin(body);

    return action;
}
