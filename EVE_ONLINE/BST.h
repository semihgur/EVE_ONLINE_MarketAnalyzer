#pragma once

#include "Item.h"

struct Compare;

class BST {
public:
    struct Node {
        Item val;
        Node* left;
        Node* right;
        Node(Item v);
    };

    Node* root;
    Compare cmp;
    int count;
    double smallest_profit_per_isk;

    BST();

    void insert(Item val);
    Item* search(double profit_per_isk);
    std::priority_queue<Item*, std::vector<Item*>, Compare> getTop25();
    void printAll();

private:
    void inOrder(Node* node, std::priority_queue<Item*, std::vector<Item*>, Compare>& maxHeap);
    void printInOrder(Node* node);
};

struct Compare {
    bool operator()(Item* a, Item* b) {
        return a->profit_per_isk < b->profit_per_isk;
    }
};
