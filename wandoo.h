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

Task* tasks;
int taskCount;

void printTasks(int highlight);
void newTask(int parent);

