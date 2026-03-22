#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ncurses.h>
#include <string.h>


typedef struct {
    char *task;
    uint8_t complete;
} Task;

Task* tasks;
int taskCount = 1;

void printTasks(int highlight);
void newTask();

int main()
{
    tasks = calloc(1, sizeof(Task));
    tasks[0] = (Task){.task = strdup("Task 1"), .complete=0x00};
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
            case 10:
                tasks[highlight-1].complete ^= 0x01; 
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

    for (int i = 0; i < taskCount; i++) {
        free(tasks[i].task); 
    }

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
        switch (tasks[i].complete){ 
            case 0x01:
                mvprintw(y, x, "[x] %s", tasks[i].task);
                break;
            default:
                mvprintw(y, x, "[-] %s", tasks[i].task);
                break;
        }

        if (highlight == i + 1)
            attroff(A_REVERSE);

        y++;
    }
}

void newTask()
{
    int h = 7, w = 75;
    int y = (LINES - h) / 2;
    int x = (COLS - w) / 2;

    WINDOW *popup = newwin(h, w, y, x);
    box(popup, 0, 0);
    mvwprintw(popup, 1, 2, "Enter new task:");
   
    int tbw = w-5;
    char buf[tbw + 1];
    memset(buf, '-', tbw);
    buf[tbw] = '\0';

    mvwprintw(popup, 2, 2, "%s", buf);
    mvwprintw(popup, 3, 2, "|");
    mvwprintw(popup, 3, w-4, "|");
    mvwprintw(popup, 4, 2, "%s", buf);
    mvwprintw(popup, h-2, 2, "Press Enter to save");
    wrefresh(popup);

    echo();
    char buffer[tbw-3];
    mvwgetnstr(popup, 3, 4, buffer, tbw-4); // user types here
    noecho();


    tasks = realloc(tasks, sizeof(Task) * (taskCount + 1));
    tasks[taskCount] = (Task){.task = strdup(buffer), .complete=0x00};
    taskCount++;


    delwin(popup);
}
