#pragma once

#include <vector>
#include <exception>
#include <iostream>
#include <memory>
#include <stack>
#include <unordered_set>
#include <assert.h>

using namespace std;

struct TreeNode
{
    int val;
    TreeNode* left;
    TreeNode* right;
};

struct InOrderIterative
{
    void go(TreeNode* node)
    {
        cout << "\n";
        unordered_set<TreeNode*> visited;
        stack<TreeNode*> s;
        s.emplace(node);
        while (!s.empty())
        {
            auto v = s.top();
            bool print = true;
            if (v->left)
            {
                auto it = visited.find(v->left);
                if (it == visited.end())
                {
                    s.emplace(v->left);
                    print = false;
                }
            }      

            if (print)
            {
                cout << "->" << v->val;
                visited.emplace(v);
                s.pop();

                if (v->right)
                {
                    auto it = visited.find(v->right);
                    if (it == visited.end())
                        s.emplace(v->right);
                }
            }
        }
    }

     void go2(TreeNode* node)
        {
            cout << "\n";
            //unordered_set<TreeNode*> visited;
            stack<TreeNode*> s;
            //s.emplace(node);
            TreeNode* curr = node;
            while (curr != nullptr || !s.empty())
            {
                //TreeNode* curr = s.top();
                while (curr)
                {
                    s.emplace(curr);
                    curr = curr->left;
                }

                curr = s.top();
                cout << "->" << curr->val;
                s.pop();
                curr = curr->right;

            }
        }
    
};

TreeNode* findNode(TreeNode* root, int val)
{
    TreeNode* current = root;
    while (current)
    {
        if (current->val == val)
            return current;
        else if (val < current->val) current = current->left;
        else current = current->right;
    }

    return nullptr;
}

TreeNode* findNode2(TreeNode* root, int val)
{
    while (root && root->val != val)    
        root = (val < root->val) ?
            root->left : root->right;    
    return root;
}

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

    cout << n->val << "->";

    printPreOrder(n->left);
    printPreOrder(n->right);
}

void printInOrder(TreeNode* n)
{
    if (n == nullptr)
        return;

    printInOrder(n->left);
    cout << n->val << "->";
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
        root = add(root, 7);
        add(root, 4);
        add(root, 3);
        add(root, 2);
        add(root, 6);
        add(root, 1);
        add(root, 5);
        add(root, 17);
        add(root, 9);
        add(root, 8);
        printInOrder(root);
        Solution s;
        //cout << endl << s.kthSmallest(root, 8) << endl;

        InOrderIterative p;
        p.go(root);
        p.go2(root);

        assert(findNode(root, 8)->val == 8);
        assert(findNode(root, 17)->val == 17);
        assert(findNode(root, 4)->val == 4);
        assert(findNode(root, 1)->val == 1);
        assert(findNode(root, 9)->val == 9);
        assert(findNode(root, 5)->val == 5);
        assert(findNode(root, 500) == nullptr);
        assert(findNode(root, 0) == nullptr);

        assert(findNode2(root, 8)->val == 8);
        assert(findNode2(root, 17)->val == 17);
        assert(findNode2(root, 4)->val == 4);
        assert(findNode2(root, 1)->val == 1);
        assert(findNode2(root, 9)->val == 9);
        assert(findNode2(root, 5)->val == 5);
        assert(findNode2(root, 500) == nullptr);
        assert(findNode2(root, 0) == nullptr);
    }
        
        

}