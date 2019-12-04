#pragma once

#include <functional>
#include <vector>
#include <forward_list>
#include <assert.h>
#include <exception>

namespace hashmap
{
    template <class K, 
              class V, 
              class H=std::hash<K>,
              class Comp=std::equal_to<K>>
    class HashMapList1
    {
    private:
        struct Node
        {
            const K key_;
            V val_;
            int b_;
        };
        using ListType = std::forward_list<Node>;
        using ListIter = typename ListType::iterator;
        using ListConstIter = typename ListType::const_iterator;
        std::forward_list<Node> l_;
        std::vector<ListIter> b_;
        size_t N_;

    public:
        HashMapList1(int bucketSize)   
            :b_(bucketSize, l_.end())
        {
        }

        bool insert(const K& key, const V& val)
        {
            int b = H()(key) % b_.size();
            if (emptyBucket(b))
            {   
                auto it = l_.begin();
                auto it2 = place(key, val, b, l_.end());                
                if (it != l_.end()) b_[it->b_] = it2;
                b_[b] = it2;
            }
            else
            {
                auto it = b_[b];
                auto front = start(b);
                auto back = front;
                while (back != l_.end() &&
                       back->b_ == front->b_ && 
                       back->key_ != key)
                    front=back++;

                if (back != l_.end() && back->key_ == key) 
                    return false;

                it=place(key, val, b, front);
                if (back != l_.end())
                    b_[back->b_]=it;
            }

            return true;
        }

        V find(const K& key)
        {
            int b = H()(key) % b_.size();
            if (emptyBucket(b))
                throw std::runtime_error("not found!");
                        
            auto it = start(b);                        
            while (it != l_.end() && it->b_ == b)
            {               
                if (it->key_ == key)
                    return it->val_;
                ++it;
            }

            throw std::runtime_error("not found!");
        }



        bool contains(const K& key)
        {
            int b = H()(key) % b_.size();
            if (!emptyBucket(b))
            {
                auto it = start(b);
                while (it != l_.end() && it->b_ == b)
                {
                    if (it->key_ == key)
                        return true;
                    ++it;
                }
            }

            return false;
        }

        bool erase(const K& key)
        {
            int b = H()(key) % b_.size();
            if (!emptyBucket(b))
            {   
                auto it = start(b);
                auto prevIt = previous(b);
                while (it != l_.end() && it->b_ == b)
                {
                    if (it->key_ == key)
                    {
                        auto nextIt = std::next(it);
                        if (nextIt != l_.end())
                        {                            
                            if (nextIt->b_ != b)
                            {
                                if (prevIt != l_.end())
                                    b_[nextIt->b_] = prevIt;
                                else
                                    b_[nextIt->b_] = nextIt;
                            }
                        }

                        if (prevIt != l_.end())
                        {
                            if (b_[b] == it) cleanBucket(b);
                            l_.erase_after(prevIt);
                        }
                        else
                        {
                            if (b_[b] == l_.begin()) cleanBucket(b);
                            l_.pop_front();
                        }
                        --N_;
                        return true;
                    }                        
                    prevIt = it++;
                }
            }

            return false;
        }

        size_t size() const { return N_; }
        bool empty() const { return !size(); }

    private:
        bool emptyBucket(int bucket) const
        {
            return b_[bucket] == l_.end();
        }

        auto place(const K& key, const V& val, int bucket, ListIter it)
        {
            ++N_;
            if (it == l_.end())
            {
                l_.push_front({ key,val,bucket });
                return l_.begin();
            }
            else
            {
                return l_.insert_after(it, { key,val,bucket });
            }
        }

        auto start(int b) 
        {
            auto it = b_[b];
            return (it == l_.begin() && b == l_.begin()->b_) ?
                      l_.begin() : std::next(it);
        }

        auto previous(int b)
        {
            auto it = b_[b];
            return (it == l_.begin() && b == l_.begin()->b_) ?
                    l_.end() : it;
        }

        void cleanBucket(int bucket)
        {
            b_[bucket] = l_.end();
        }
    };

    class Solution
    {
    public:
        int result = 0;
        int pos = 0;
        int findAux(const string& str, int sum, int level)
        {
            if (pos >= str.size())
                return -1;

            while (1)
            {
                cout << " level = " << level << " pos=" << pos;
                //  << " str=" << str.substr(pos, str.size() - pos);

                auto p = parseName(str);
                if (p.first == -1) return -1;

                if (p.second) result = max(sum + p.first, result);

                auto l = parseLevel(str);
                if (l == -1)
                    return l;
                else if (l > level)
                {
                    l = findAux(str, sum + p.first, l);
                    if (l != level)
                        return l;
                }
                else if (l < level)
                {
                    return l;
                }
            }

            return level;
        }

        pair<int/*len*/, bool/*isfile*/> parseName(const string& str)
        {
            if (pos >= str.size())
                return { -1, false };

            int prevPos = pos;
            bool isFile = false;
            while (pos < str.size() && str[pos] != '\\')
            {
                if (str[pos] == '.')
                    isFile = true;
                ++pos;
            }

            return { pos - prevPos, isFile };
        }

        int/*level*/ parseLevel(const string& str)
        {
            if (pos >= str.size())
                return -1;

            int level = 0;
            while (pos < str.size() && (str[pos] == 'n' || str[pos] == '\\' || str[pos] == 't'))
            {
                if (str[pos++] == 't')
                    ++level;
            }

            return level;
        }

        int lengthLongestPath(string input)
        {
            findAux(input, 0, 0);
            return result;
        }
    };

    static void test1()
    {
        Solution s;
        int len = s.lengthLongestPath("dir\n\tsubdir1\n\tsubdir2\n\t\tfile.ext");
        string st("\n");
        cout << "\n len=" << st.size();
        HashMapList1<int, int> h(7);
        assert(h.insert(1, 1));
        assert(h.insert(2, 2));
        assert(h.insert(3, 3));
        assert(h.insert(4, 3));
        assert(!h.insert(4, 3));
        assert(!h.insert(4, 3));
        assert(h.find(4) == 3);
        std::cout << "\n como";
        assert(h.find(1) == 1);
        assert(h.find(2) == 2);
        assert(h.find(4) == 3);
        assert(h.contains(1));
        assert(h.contains(2));
        assert(h.contains(4));
        assert(!h.contains(40));
        assert(h.contains(3));
        assert(h.erase(4));
        assert(h.erase(1));
        assert(h.erase(3));
        assert(h.erase(2));
        assert(!h.contains(4));
        assert(!h.contains(2));
        assert(!h.contains(3));
        assert(h.empty());

        try { assert(h.find(14) == 3);  assert(false); }
        catch (const std::exception& e)
        { std::cout << "\n " << e.what();}
    }
}

