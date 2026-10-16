#ifndef ACTIONSTACK_H
#define ACTIONSTACK_H

#include "common.h"

class ActionStack {
private:
    string actions[100];
    int top;

public:
    ActionStack();
    void push(string action);
    void displayRecentActions();
};

