#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ncurses.h>
#include <string.h>


typedef struct {
    char *task;
    uint8_t complete;
    int parent;
    int *children;
    int childCount;
} Task;

Task* tasks;
int taskCount = 1;

void printTasks(int highlight);
void newTask(int parent);

int main()
{
    tasks = calloc(1, sizeof(Task));
    tasks[0] = (Task){.task = strdup("root"), .complete=0x00, .parent=-1, .children = NULL, .childCount = 0};
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
                newTask(highlight-1); 
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

// printTaskRecursive(int id, int highlight, int x, int *y, int depth) is a recursive function that
// recurses through each task, printing its information at indentation level depth, starting at x and y, and highlighting
// the task corresponding to highlight. 
void printTaskRecursive(int id, int highlight, int x, int *y, int depth)
{
    int indent = depth * 2;

    if (highlight == id + 1)
        attron(A_REVERSE);

    if (tasks[id].complete == 0x01)
        mvprintw(*y, x + indent, "[x] %s", tasks[id].task);
    else
        mvprintw(*y, x + indent, "[-] %s", tasks[id].task);

    if (highlight == id + 1)
        attroff(A_REVERSE);

    (*y)++;

    for (int i = 0; i < tasks[id].childCount; i++)
    {
        int childID = tasks[id].children[i];
        printTaskRecursive(childID, highlight, x, y, depth + 1);
    }
}

// printTasks(int highlight) starts the recursive print task function and highlights the task corresponding to highlight
void printTasks(int highlight)
{
    int x = 2, y = 2;
    printTaskRecursive(0, highlight, x, &y, 0);
}

// newTask(int parent) creates a dialog to create a new task with the parent parent. 
void newTask(int parent)
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
    mvwgetnstr(popup, 3, 4, buffer, tbw-4);
    noecho();

    tasks = realloc(tasks, sizeof(Task) * (taskCount + 1));
    tasks[taskCount] = (Task){
        .task = strdup(buffer),
        .complete = 0x00,
        .parent = parent,
        .children = NULL,
        .childCount = 0
    };

    Task *p = &tasks[parent];
    p->children = realloc(p->children, sizeof(int) * (p->childCount + 1));
    p->children[p->childCount] = taskCount;
    p->childCount++;

    taskCount++;
    delwin(popup);
}
