#include "BST.h"
#include <queue>

BST::Node::Node(Item v) {
    val = v;
    left = right = nullptr;
}

BST::BST() {
    root = nullptr;
    count = 0;
    smallest_profit_per_isk = 999999.0;
}

void BST::insert(Item val) {
    // Insertion logic using cmp function
    Node* newNode = new Node(val);

    if (!root) {
        root = newNode;
        return;
    }

    Node* curr = root;
    while (curr) {
        if (curr->val.profit_per_isk > val.profit_per_isk) {
            if (curr->left)
                curr = curr->left;
            else {
                curr->left = newNode;
                break;
            }
        } else {
            if (curr->right)
                curr = curr->right;
            else {
                curr->right = newNode;
                break;
            }
        }
    }

    count++;
    if (count >= 25 && val.profit_per_isk < smallest_profit_per_isk) {
        return;
    }

    if (val.profit_per_isk < smallest_profit_per_isk) {
        smallest_profit_per_isk = val.profit_per_isk;
    }
}

Item* BST::search(double profit_per_isk) {
    Node* curr = root;

    while (curr) {
        if (curr->val.profit_per_isk == profit_per_isk) {
            return &(curr->val);
        } else if (curr->val.profit_per_isk > profit_per_isk) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }

    return nullptr;
}

std::priority_queue<Item*, std::vector<Item*>, Compare> BST::getTop25() {
    std::priority_queue<Item*, std::vector<Item*>, Compare> maxHeap;

    inOrder(root, maxHeap);
    // maxHeap now contains top 25 Items

    return maxHeap;
}

void BST::inOrder(Node* node, std::priority_queue<Item*, std::vector<Item*>, Compare>& maxHeap) {
    if (!node)
        return;

    inOrder(node->left, maxHeap);

    if (maxHeap.size() < 25) {
        maxHeap.push(&node->val);
    } else {
        if (node->val.profit_per_isk > maxHeap.top()->profit_per_isk) {
            maxHeap.pop();
            maxHeap.push(&node->val);
        }
    }

    inOrder(node->right, maxHeap);
}

void BST::printAll() {
    printInOrder(root);
}

void BST::printInOrder(Node* node) {
    if (node == nullptr)
        return;

    printInOrder(node->left);
    node->val.print();
    printInOrder(node->right);
}
