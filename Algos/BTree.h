#pragma once

#include <memory>
#include <assert.h>
using namespace std;

namespace btree
{
    struct Btree
    {
        struct Node
        {   
            vector<int> keys;
            vector<unique_ptr<Node>> childs;
            bool leaf = true;
            int N = 0;
            Node(int cap)
            {
                keys.resize(cap);
                childs.resize(cap+1);
            }
        };

        int t_;
        int cap_;

        unique_ptr<Node> r_;
        Btree(int t)
           :t_(t),
            cap_(2*t_-1),
            r_(create())
        {                   
        }

        unique_ptr<Node> create()
        {
            return unique_ptr<Node>(
                new Node(cap_));
        }

        void split(Node* parent, int parentIdx)
        {            
            auto& left = parent->childs[parentIdx];
            auto right = create();

            Node* leftPtr = left.get();
            Node* rightPtr = right.get();            

            for (int i = 0; i < t_ - 1; ++i)            
                rightPtr->keys[i] = leftPtr->keys[t_ + i];                     
            
            if (!leftPtr->leaf)
            {
                for (int i = 0; i < t_; ++i)
                  rightPtr->childs[i] = move(leftPtr->childs[t_ + i]);
            }

            for (int i = parent->N; i > parentIdx; --i)
            {
                parent->keys[i] = parent->keys[i-1];
                parent->childs[i+1] = move(parent->childs[i]);
            }

            rightPtr->N = t_ - 1;
            leftPtr->N = t_ - 1; 
            rightPtr->leaf = leftPtr->leaf;
            parent->leaf = false;
            parent->keys[parentIdx] = leftPtr->keys[t_-1];
            parent->childs[parentIdx+1] = move(right);            
            ++parent->N;
            
        }

        void insertNotFull(Node* node, int key)
        {
            bool inserted = false;
            while (!inserted)
            {
                if (node->leaf)
                {
                    int i = node->N - 1;
                    while (i >= 0 && node->keys[i] > key)
                    {
                        node->keys[i + 1] = node->keys[i];
                        --i;
                    }
                                        
                    node->keys[++i] = key;
                    inserted = true;
                    ++node->N;                    
                }
                else
                {
                    int i = node->N - 1;
                    while (i >= 0 && node->keys[i] > key)
                        --i;
                    ++i;

                    if (node->childs[i].get()->N == cap_)
                    {
                        split(node, i);
                        if (key > node->keys[i])
                            ++i;
                    }

                    node = node->childs[i].get();
                }
            }
        }

        bool find(int key)
        {
            if (!r_->N) 
                return false;

            Node* node = r_.get();           
            while (node)
            {   
                int i = 0;
                while (i <= node->N-1 && node->keys[i] < key)
                    i++;                            

                if (key == node->keys[i]) return true;
                node = !node->leaf ? node->childs[i].get() : nullptr;                
            }
            return false;
        }

        void insert(int key)
        {
            if (r_->N < cap_)
            {
                insertNotFull(r_.get(), key);
            }
            else
            {
                auto node(create());
                {
                    node->N = 0;
                    node->leaf = false;
                    node->childs[0] = move(r_);                   
                    r_ = move(node);
                }

                split(r_.get(), 0);
                insertNotFull(r_.get(), key);
            }
        }

        void dump()
        {
            vector<Node*> q;
            q.emplace_back(r_.get());
            while (!q.empty())
            {
                cout << "\n";                
                for (auto level : q)
                {
                    cout << "  --- |";
                    for (int i = 0; i < level->N; ++i)
                       cout << level->keys[i] << " | ";
                }
                cout << "\n"; 

                vector<Node*> tmp;
                for (auto level : q)
                {
                    if (!level->leaf && level->N)
                        for (int i = 0; i < level->N+1; ++i)
                            if (level->childs[i].get())
                                tmp.emplace_back(level->childs[i].get());
                }

                tmp.swap(q);
            }
        }
    };
    static void test()
    {
        Btree t(2);
        t.insert(10);
        t.insert(20);
        t.insert(30);
        t.insert(8);
        t.insert(9);
        t.insert(6);
        t.insert(12);
        t.insert(14);
        t.insert(5);
        t.insert(33);
        t.insert(4);
        t.insert(15);
        t.insert(15);
        t.insert(15);
        t.insert(15);
        t.insert(15);
        t.insert(15);
        t.insert(15);
        t.insert(15);
        t.insert(15);
        t.insert(15);
        t.insert(16);
        t.insert(18);
        t.insert(19);
        t.insert(31);
        t.dump();
        assert(t.find(30));
        assert(t.find(8));
        assert(t.find(20));
        assert(t.find(12));
        assert(t.find(6));
        assert(t.find(19));
        assert(t.find(10));        
        assert(t.find(8));
        assert(t.find(15));
        
    }
}
