#include "metro.h"

#include <cstdlib>

using namespace std;

static int heightOf(const AvlNode *node)
{
    if (node == nullptr)
        return 0;
    return node->height;
}

static int bigger(int a, int b)
{
    if (a > b)
        return a;
    return b;
}

static void updateHeight(AvlNode *node)
{
    node->height = 1 + bigger(heightOf(node->left), heightOf(node->right));
}

static int balanceFactor(const AvlNode *node)
{
    return heightOf(node->left) - heightOf(node->right);
}

static AvlNode *rotateRight(AvlNode *y)
{
    AvlNode *x = y->left;
    AvlNode *moved = x->right;

    x->right = y;
    y->left = moved;

    updateHeight(y);
    updateHeight(x);
    return x;
}

static AvlNode *rotateLeft(AvlNode *x)
{
    AvlNode *y = x->right;
    AvlNode *moved = y->left;

    y->left = x;
    x->right = moved;

    updateHeight(x);
    updateHeight(y);
    return y;
}

static AvlNode *rebalance(AvlNode *node, int key)
{
    int balance;

    updateHeight(node);
    balance = balanceFactor(node);

    if (balance > 1 && key < node->left->key)
        return rotateRight(node);

    if (balance < -1 && key > node->right->key)
        return rotateLeft(node);

    if (balance > 1 && key > node->left->key) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }

    if (balance < -1 && key < node->right->key) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

static AvlNode *insertNode(AvlNode *node, int key, bool *added)
{
    if (node == nullptr) {
        AvlNode *fresh = (AvlNode *)malloc(sizeof(AvlNode));
        if (fresh == nullptr)
            return nullptr;
        fresh->key = key;
        fresh->height = 1;
        fresh->left = nullptr;
        fresh->right = nullptr;
        *added = true;
        return fresh;
    }

    if (key < node->key) {
        AvlNode *child = insertNode(node->left, key, added);
        if (child == nullptr)
            return node;
        node->left = child;
    } else if (key > node->key) {
        AvlNode *child = insertNode(node->right, key, added);
        if (child == nullptr)
            return node;
        node->right = child;
    } else {
        return node;
    }

    if (!*added)
        return node;
    return rebalance(node, key);
}

Avl *newAvl(void)
{
    Avl *tree = (Avl *)malloc(sizeof(Avl));
    if (tree == nullptr)
        return nullptr;
    tree->root = nullptr;
    tree->size = 0;
    return tree;
}

bool avlInsert(Avl *tree, int key)
{
    bool added = false;

    if (tree == nullptr)
        return false;

    tree->root = insertNode(tree->root, key, &added);
    if (added)
        tree->size++;
    return added;
}

bool avlContains(const Avl *tree, int key)
{
    const AvlNode *node;

    if (tree == nullptr)
        return false;

    node = tree->root;
    while (node != nullptr) {
        if (key == node->key)
            return true;
        if (key < node->key)
            node = node->left;
        else
            node = node->right;
    }
    return false;
}

size_t avlSize(const Avl *tree)
{
    if (tree == nullptr)
        return 0;
    return tree->size;
}

int avlHeight(const Avl *tree)
{
    if (tree == nullptr)
        return 0;
    return heightOf(tree->root);
}

static void collect(const AvlNode *node, int low, int high, int *out, size_t maxOut, size_t *count)
{
    if (node == nullptr || *count >= maxOut)
        return;

    if (node->key > low)
        collect(node->left, low, high, out, maxOut, count);

    if (node->key >= low && node->key <= high && *count < maxOut) {
        out[*count] = node->key;
        (*count)++;
    }

    if (node->key < high)
        collect(node->right, low, high, out, maxOut, count);
}

size_t avlInorder(const Avl *tree, int *out, size_t maxOut)
{
    size_t count = 0;

    if (tree == nullptr || out == nullptr)
        return 0;
    collect(tree->root, -2147483647 - 1, 2147483647, out, maxOut, &count);
    return count;
}

size_t avlRange(const Avl *tree, int low, int high, int *out, size_t maxOut)
{
    size_t count = 0;

    if (tree == nullptr || out == nullptr || low > high)
        return 0;
    collect(tree->root, low, high, out, maxOut, &count);
    return count;
}

static void freeNodes(AvlNode *node)
{
    if (node == nullptr)
        return;
    freeNodes(node->left);
    freeNodes(node->right);
    free(node);
}

void freeAvl(Avl *tree)
{
    if (tree == nullptr)
        return;
    freeNodes(tree->root);
    free(tree);
}
