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

// FUNCTION DECLARATIONS

void printTaskRecursive(int id, int highlight, int x, int *y, int depth, int *currentIndex);

int printTasks(int highlight);

void editTask(int id, int parent, char* pretext);

int getTaskIDByHighlight(int targetHighlight, int *currentIndex, int id);
