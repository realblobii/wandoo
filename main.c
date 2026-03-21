#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>

char **tasks;
int taskCount = 1;

void printTasks(int highlight);
void newTask();

int main()
{
    tasks = calloc(1, sizeof(char*));
    tasks[0] = strdup("New Task");

    int highlight = 1;
    int c;

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    curs_set(0);

    while (1)
    {
        clear();
        printTasks(highlight);
        refresh();

        c = getch();
        switch(c)
        {
            case KEY_UP:
                highlight--;
                if (highlight < 1) highlight = taskCount;
                break;
            case KEY_DOWN:
                highlight++;
                if (highlight > taskCount) highlight = 1;
                break;
            case '+':                  
		newTask(); 
		break;
            case 'q':
                goto cleanup;
        }
    }

cleanup:
    endwin();
    for (int i = 0; i < taskCount; i++) free(tasks[i]);
    free(tasks);
    return 0;
}

void printTasks(int highlight)
{
    int x = 2, y = 2;
    for (int i = 0; i < taskCount; ++i)
    {
        if (highlight == i + 1)
            attron(A_REVERSE);

        mvprintw(y, x, "[ ] %s", tasks[i]);

        if (highlight == i + 1)
            attroff(A_REVERSE);

        y++;
    }
}

void newTask()
{
    int h = 7, w = 40;
    int y = (LINES - h) / 2;
    int x = (COLS - w) / 2;

    WINDOW *popup = newwin(h, w, y, x);
    box(popup, 0, 0);
    mvwprintw(popup, 1, 2, "Enter new task:");
    mvwprintw(popup, h-2, 2, "Press Enter to save");
    wrefresh(popup);

    echo();
    char buffer[100];
    mvwgetnstr(popup, 3, 2, buffer, sizeof(buffer)-1); // user types here
    noecho();


    tasks = realloc(tasks, sizeof(char*) * (taskCount + 1));
    tasks[taskCount] = strdup(buffer);
    taskCount++;


    delwin(popup);
}
