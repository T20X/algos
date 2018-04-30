#pragma once

#include <vector>
#include <exception>
#include <iostream>

using namespace std;

struct TreeNode
{
    int val;
    TreeNode* left;
    TreeNode* right;
};

TreeNode* add(TreeNode* n, int v)
{
    if (n == nullptr)
    {
        return new TreeNode{ v, nullptr, nullptr };        
    }

    if (n->val > v)
    {
        TreeNode* r = add(n->left, v);
        if (r != nullptr)
        {
            n->left = r;
            return nullptr;
        }
    }
    else
    {
        TreeNode* r = add(n->right, v);
        if (r != nullptr)
        {
            n->right = r;
            return nullptr;
        }
    }

    return nullptr;
}

void printPreOrder(TreeNode* n)
{
    if (n == nullptr)
        return;

    cout << n->val << " -> ";

    printPreOrder(n->left);
    printPreOrder(n->right);
}

void printInOrder(TreeNode* n)
{
    if (n == nullptr)
        return;

    printInOrder(n->left);
    cout << n->val << " -> ";
    printInOrder(n->right);
}

class Solution {
public:
    Solution() :N(0)
    {
    }

    int kthSmallest(TreeNode* root, int k)
    {
        K = k;     
        if (preOrder(root))
            return solution;
        else
            throw runtime_error("asdasd!");
    }

    bool preOrder(TreeNode* node)
    {
        if (node == nullptr)     
            return false;    

        if (preOrder(node->left))
            return true;

        N++;
        if (N >= K)
        {
            solution = node->val;
            return true; //terminate condition
        }
        
        if (preOrder(node->right))
            return true;

        return false;
    }

private:
    int N;
    int solution;
    int K;
};

void kthSmallest()
{
    {
        TreeNode* root = nullptr;
        root = add(root, 5);
        add(root, 4);
        add(root, 3);
        add(root, 2);
        add(root, 6);
        add(root, 1);
        add(root, 7);
        add(root, 9);
        add(root, 8);
        printInOrder(root);
        Solution s;
        cout << endl << s.kthSmallest(root, 8) << endl;
    }
        
        

}