#include "metro.h"

#include <cstdlib>

using namespace std;

Bst *newBst(void)
{
    return (Bst *)calloc(1, sizeof(Bst));
}

bool bstInsert(Bst *tree, int key)
{
    BstNode *node;
    BstNode *parent = nullptr;
    BstNode *cur;

    if (tree == nullptr)
        return false;

    cur = tree->root;
    while (cur != nullptr) {
        if (key == cur->key)
            return false;
        parent = cur;
        if (key < cur->key)
            cur = cur->left;
        else
            cur = cur->right;
    }

    node = (BstNode *)malloc(sizeof(BstNode));
    if (node == nullptr)
        return false;
    node->key = key;
    node->left = nullptr;
    node->right = nullptr;

    if (parent == nullptr)
        tree->root = node;
    else if (key < parent->key)
        parent->left = node;
    else
        parent->right = node;
    tree->size++;
    return true;
}

bool bstContains(const Bst *tree, int key)
{
    const BstNode *node = tree == nullptr ? nullptr : tree->root;
    while (node != nullptr) {
        if (key == node->key)
            return true;
        node = key < node->key ? node->left : node->right;
    }
    return false;
}

static bool containsFrom(const BstNode *node, int key)
{
    if (node == nullptr)
        return false;
    if (key == node->key)
        return true;
    return key < node->key ? containsFrom(node->left, key)
                           : containsFrom(node->right, key);
}

bool bstContainsRecursive(const Bst *tree, int key)
{
    return tree != nullptr && containsFrom(tree->root, key);
}

size_t bstSize(const Bst *tree)
{
    return tree == nullptr ? 0 : tree->size;
}

struct NodeDepth {
    BstNode *node;
    int depth;
};

int bstHeight(const Bst *tree)
{
    Stack *stack;
    NodeDepth item;
    int height = 0;

    if (tree == nullptr || tree->root == nullptr)
        return 0;
    stack = newStack(sizeof(NodeDepth));
    if (stack == nullptr)
        return -1;

    item.node = tree->root;
    item.depth = 1;
    stackPush(stack, &item);
    while (stackPop(stack, &item)) {
        NodeDepth child;
        if (item.depth > height)
            height = item.depth;
        child.depth = item.depth + 1;
        if (item.node->left != nullptr) {
            child.node = item.node->left;
            stackPush(stack, &child);
        }
        if (item.node->right != nullptr) {
            child.node = item.node->right;
            stackPush(stack, &child);
        }
    }
    freeStack(stack);
    return height;
}

static size_t walkRange(const Bst *tree, int low, int high, int *out, size_t maxOut)
{
    Stack *stack;
    BstNode *cur;
    size_t found = 0;

    if (tree == nullptr || out == nullptr || maxOut == 0 || low > high)
        return 0;
    stack = newStack(sizeof(BstNode *));
    if (stack == nullptr)
        return 0;

    cur = tree->root;
    while (cur != nullptr || !stackIsEmpty(stack)) {
        while (cur != nullptr) {
            if (cur->key >= low) {
                stackPush(stack, &cur);
                cur = cur->left;
            } else {
                cur = cur->right;
            }
        }
        if (!stackPop(stack, &cur))
            break;
        if (cur->key > high)
            break;
        out[found++] = cur->key;
        if (found == maxOut)
            break;
        cur = cur->right;
    }
    freeStack(stack);
    return found;
}

size_t bstInorder(const Bst *tree, int *out, size_t maxOut)
{
    return walkRange(tree, -2147483647 - 1, 2147483647, out, maxOut);
}

size_t bstRange(const Bst *tree, int low, int high, int *out, size_t maxOut)
{
    return walkRange(tree, low, high, out, maxOut);
}

void freeBst(Bst *tree)
{
    Stack *stack;
    BstNode *node;

    if (tree == nullptr)
        return;

    stack = newStack(sizeof(BstNode *));
    if (stack != nullptr && tree->root != nullptr) {
        stackPush(stack, &tree->root);
        while (stackPop(stack, &node)) {
            if (node->left != nullptr)
                stackPush(stack, &node->left);
            if (node->right != nullptr)
                stackPush(stack, &node->right);
            free(node);
        }
    }
    freeStack(stack);
    free(tree);
}
