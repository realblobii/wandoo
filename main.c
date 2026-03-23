#include "wandoo.h"
#include "meta.h"

int highlight = 1; 

int main(int argc, char* argv[])
{
  
  if (argc == 2)
  {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    loadFile(argv[1]);
  }
  else
  {
    printf("Incorrect usage. \nUsage: wandoo <filename>\n");
    return 1; 
  }

  while (1)
  {
    clear();
    int width = COLS;           
    attron(A_REVERSE);         
    mvhline(0, 0, ' ', width); 
    mvprintw(0, 0, " wandoo %s - editing '%s'", WANDOO_VER, curFileName);
    attroff(A_REVERSE);        
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
        editTask(taskCount, highlightedID, "new");
        taskCount++;
        break;
      case 10: 
        editTask(highlightedID, tasks[highlightedID].parent, tasks[highlightedID].task);
        break;
      case 'w':
        saveFile(curFileName);
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


int printTasks(int highlight)
{
  int x = 2, y = 2;
  int currentIndex = 0;
  printTaskRecursive(0, highlight, x, &y, 0, &currentIndex);
  return currentIndex;
}

void editTask(int id, int parent, char* pretext)
{
  uint8_t cmp = tasks[id].complete;
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
  tasks[id].complete = cmp;
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


void saveFile(char* name)
{
  if (!tasks || taskCount <= 0) return;
  FILE *file = fopen(name, "wb");
  if (!file) return;

  for (int i = 0; i < taskCount; i++)
  {

    int namelen = tasks[i].task ? (int)strlen(tasks[i].task) : 0;
    fwrite(&namelen, sizeof(int), 1, file);
    if (namelen > 0) fwrite(tasks[i].task, sizeof(char), namelen, file);


    fwrite(&tasks[i].complete, sizeof(tasks[i].complete), 1, file);


    fwrite(&tasks[i].parent, sizeof(int), 1, file);
    fwrite(&tasks[i].childCount, sizeof(int), 1, file);


    if (tasks[i].childCount > 0 && tasks[i].children)
      fwrite(tasks[i].children, sizeof(int), tasks[i].childCount, file);
  }

  fclose(file);
}

void loadFile(char* name)
{
  curFileName = name;
  FILE *file = fopen(name, "rb");
  if (!file)
  {
    taskCount = 1;
    tasks = calloc(1, sizeof(Task));
    tasks[0] = (Task){.task = strdup("root"), .complete = 0, .parent = -1, .children = NULL, .childCount = 0};
    highlight = 1;
    return;
  }

  if (tasks)
  {
    for (int i = 0; i < taskCount; i++)
    {
      free(tasks[i].task);
      free(tasks[i].children);
    }
    free(tasks);
    tasks = NULL;
    taskCount = 0;
  }

  while (1)
  {
    int namelen;
    if (fread(&namelen, sizeof(int), 1, file) != 1) break;
    if (namelen < 0 || namelen > 10000) break;

    char *namebuf = malloc(namelen + 1);
    if (!namebuf) break;
    if (fread(namebuf, sizeof(char), namelen, file) != (size_t)namelen)
    {
      free(namebuf);
      break;
    }
    namebuf[namelen] = '\0';

    uint8_t complete;
    int parent, childCount;

    if (fread(&complete, sizeof(uint8_t), 1, file) != 1 ||
        fread(&parent, sizeof(int), 1, file) != 1 ||
        fread(&childCount, sizeof(int), 1, file) != 1)
    {
      free(namebuf);
      break;
    }

    int *children = NULL;
    if (childCount > 0)
    {
      children = malloc(sizeof(int) * childCount);
      if (!children || fread(children, sizeof(int), childCount, file) != (size_t)childCount)
      {
        free(namebuf);
        if (children) free(children);
        break;
      }
    }

    Task *newTasks = realloc(tasks, sizeof(Task) * (taskCount + 1));
    if (!newTasks)
    {
      free(namebuf);
      if (children) free(children);
      break;
    }
    tasks = newTasks;

    tasks[taskCount].task = namebuf;
    tasks[taskCount].complete = complete;
    tasks[taskCount].parent = parent;
    tasks[taskCount].childCount = childCount;
    tasks[taskCount].children = children;

    taskCount++;
  }

  if (taskCount == 0)
  {
    taskCount = 1;
    tasks = calloc(1, sizeof(Task));
    tasks[0] = (Task){.task = strdup("root"), .complete = 0, .parent = -1, .children = NULL, .childCount = 0};
  }

  highlight = 1;
  fclose(file);
}
