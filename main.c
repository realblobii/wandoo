#include "wandoo.h"

int main()
{
    taskCount = 1;
    tasks = calloc(1, sizeof(Task));
    tasks[0] = (Task){.task = strdup("root"), .complete = 0x00, .parent = -1, .children = NULL, .childCount = 0};

    int highlight = 1;
    int c;
    int curIndex;
    int highlightedID;
    int visibleTasks;

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);

    while (1)
    {
        clear();
        visibleTasks = printTasks(highlight);
        refresh();

        curIndex = 0;
        highlightedID = getTaskIDByHighlight(highlight, &curIndex, 0);

        c = getch();
        switch(c)
        {
            case KEY_UP:
                highlight--;
                if (highlight < 1) highlight = visibleTasks;
                break;
            case KEY_DOWN:
                highlight++;
                if (highlight > visibleTasks) highlight = 1;
                break;
            case 32: 
                tasks[highlightedID].complete ^= 0x01;
                break;
            case '+':
                editTask(taskCount, highlightedID, "New Task");
                taskCount++;
                break;
            case 10: 
                editTask(highlightedID, tasks[highlightedID].parent, tasks[highlightedID].task);
                break;
            case 'q':
                goto cleanup;
        }
    }

cleanup:
    endwin();
    for (int i = 0; i < taskCount; i++)
        free(tasks[i].task);
    free(tasks);
    return 0;
}

/* printTaskRecursive(int id, int highlight, int x, int *y, int depth, int *currentIndex)
   Recursively prints tasks with indentation for children.
   Highlights the task corresponding to the flattened index 'highlight'.
   'x' is horizontal start, '*y' is vertical cursor, 'depth' controls indentation.
   'currentIndex' tracks the flattened line number for highlighting. */
void printTaskRecursive(int id, int highlight, int x, int *y, int depth, int *currentIndex)
{
    int indent = depth * 2;
    (*currentIndex)++;

    if (*currentIndex == highlight) attron(A_REVERSE);

    if (tasks[id].complete == 0x01)
        mvprintw(*y, x + indent, "[x] %s", tasks[id].task);
    else
        mvprintw(*y, x + indent, "[-] %s", tasks[id].task);

    if (*currentIndex == highlight) attroff(A_REVERSE);

    (*y)++;

    for (int i = 0; i < tasks[id].childCount; i++)
        printTaskRecursive(tasks[id].children[i], highlight, x, y, depth + 1, currentIndex);
}

/* printTasks(int highlight)
   Starts recursive printing of tasks from the root.
   Highlights the task corresponding to 'highlight'. */
int printTasks(int highlight)
{
    int x = 2, y = 2;
    int currentIndex = 0;
    printTaskRecursive(0, highlight, x, &y, 0, &currentIndex);
    return currentIndex;
}

/* editTask(int id, int parent, char* pretext)
   Creates or edits a task with the given 'parent'.
   Prefills the textbox with 'pretext'. Cursor visible.
   Supports left/right navigation without inserting arrow key codes.
   Does not duplicate in parent's children if editing an existing task.
   If parent == -1, task is root/orphaned and not added to a parent. */
void editTask(int id, int parent, char* pretext)
{
    int h = 7, w = 75;
    int y = (LINES - h) / 2;
    int x = (COLS - w) / 2;

    WINDOW *popup = newwin(h, w, y, x);
    box(popup, 0, 0);
    keypad(popup, TRUE);
    mvwprintw(popup, 1, 2, "Enter new task:");

    int tbw = w - 5;
    int maxlen = tbw - 4;
    char buffer[maxlen + 1];
    memset(buffer, 0, sizeof(buffer));

    int cursor = 0;
    if (pretext != NULL) {
        strncpy(buffer, pretext, maxlen);
        buffer[maxlen] = '\0';
        cursor = strlen(buffer);
    }

    char line[tbw + 1];
    memset(line, '-', tbw);
    line[tbw] = '\0';

    mvwprintw(popup, 2, 2, "%s", line);
    mvwprintw(popup, 3, 2, "|");
    mvwprintw(popup, 3, w - 4, "|");
    mvwprintw(popup, 4, 2, "%s", line);
    mvwprintw(popup, h - 2, 2, "Press Enter to save");
    curs_set(1);

    int ch;
    while (1) {
        if (cursor < 0) cursor = 0;
        if (cursor > strlen(buffer)) cursor = strlen(buffer);

        mvwprintw(popup, 3, 4, "%-*s", maxlen, buffer);
        wmove(popup, 3, 4 + cursor);
        wrefresh(popup);

        ch = wgetch(popup);

        if (ch == 10 || ch == KEY_ENTER) break;
        else if (ch == KEY_LEFT && cursor > 0) cursor--;
        else if (ch == KEY_RIGHT && cursor < strlen(buffer)) cursor++;
        else if ((ch == KEY_BACKSPACE || ch == 127) && cursor > 0) {
            memmove(&buffer[cursor-1], &buffer[cursor], strlen(buffer) - cursor + 1);
            cursor--;
        }
        else if (ch >= 32 && ch <= 126 && strlen(buffer) < maxlen) {
            memmove(&buffer[cursor+1], &buffer[cursor], strlen(buffer) - cursor + 1);
            buffer[cursor] = ch;
            cursor++;
        }
    }

    noecho();
    curs_set(0);

    if (id >= taskCount) {
        Task *newTasks = realloc(tasks, sizeof(Task) * (id + 1));
        if (!newTasks) {
            delwin(popup);
            return;
        }
        tasks = newTasks;
    }

    if (tasks[id].task != NULL) free(tasks[id].task);

    tasks[id].task = strdup(buffer);
    tasks[id].complete = 0x00;
    tasks[id].parent = parent;

    if (id >= taskCount && parent >= 0) {
        Task *p = &tasks[parent];
        int *newChildren = realloc(p->children, sizeof(int) * (p->childCount + 1));
        if (newChildren) {
            p->children = newChildren;
            p->children[p->childCount] = id;
            p->childCount++;
        }
    }

    if (id >= taskCount) taskCount = id + 1;

    delwin(popup);
}

/* getTaskIDByHighlight(int targetHighlight, int *currentIndex, int id)
   Returns the task ID in tasks[] corresponding to the N-th printed task (targetHighlight).
   Uses currentIndex to track recursion order. */
int getTaskIDByHighlight(int targetHighlight, int *currentIndex, int id)
{
    (*currentIndex)++;
    if (*currentIndex == targetHighlight)
        return id;

    for (int i = 0; i < tasks[id].childCount; i++) {
        int found = getTaskIDByHighlight(targetHighlight, currentIndex, tasks[id].children[i]);
        if (found != -1) return found;
    }
    return -1;
}
