#include "ActionStack.h"
ActionStack::ActionStack() {
    top = -1;
}
void ActionStack::push(string action) {
    if (top < 99)
        actions[++top] = action;
}
void ActionStack::displayRecentActions() {
    for (int i = top; i >= 0; i--)
        cout << actions[i] << endl;
}