#pragma once

// INCLUDES
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

// VARIABLES
Task* tasks;
int taskCount;
char* curFileName;


int c;
int curIndex;
int highlightedID;
int visibleTasks;

// FUNCTION DECLARATIONS

void removeChildFromParent(int childId);

void recurseDelete(int id);

void printTaskRecursive(int id, int highlight, int x, int *y, int depth, int *currentIndex);

int printTasks(int highlight);

void editTask(int id, int parent, char* pretext, int mode);

int getTaskIDByHighlight(int targetHighlight, int *currentIndex, int id);

void saveFile(char* name);

void loadFile(char* name);
