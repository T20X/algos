#pragma once

#include <vector>
#include <iostream>
#include <limits>
#include <algorithm>
#include <ostream>
#include <iterator> 

using namespace std;

void merge(vector<int>& nums1, int m, vector<int>& nums2, int n)
{
    int k = m + n - 1;
    int i = m - 1;
    int j = n - 1;

    while (j >= 0)
    { 
        if (i >= 0 && nums1[i] > nums2[j])
        {
            nums1[k--] = nums1[i--];
        }
        else
        {
            nums1[k--] = nums2[j--];
        }
    }
}

void testMerge()
{
    {
        vector<int> nums1{ 1,3,5,10,15,20 };
        nums1.resize(20);
        vector<int> nums2{ 2,7,8,9,10,15,18,19,20,21,21,23 };
        merge(nums1, 6, nums2, 12);
        std::copy(nums1.begin(), nums1.end(), ostream_iterator<int>(cout, ","));
    }
    

    {
        cout << endl;
        vector<int> nums1{ };
        nums1.resize(3);
        vector<int> nums2{ 1 };
        merge(nums1, 0, nums2, 1);
        std::copy(nums1.begin(), nums1.end(), ostream_iterator<int>(cout, ","));
    }
}