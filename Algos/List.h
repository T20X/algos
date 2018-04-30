#pragma once

#include <iostream>
using namespace std;

 struct ListNode {
    int val;
    ListNode *next;
    ListNode(int x) : val(x), next(0) {}
    
};


     ListNode* reverseList(ListNode* head) {

         ListNode* next = head;
         ListNode* prev = NULL;

         while (next != NULL)
         {
             head = next;
             ListNode* tmp = head->next;
             head->next = prev;
             prev = head;
             next = tmp;
             /*if (tmp)
                 next = tmp;
             else
                 break;*/
         }

         return head;

     }


 void printList(ListNode* l)
 {
     ListNode* p = l;
     while (p != 0)
     {
         cout << p->val << " -> ";
         p = p->next;
     }

     cout << endl;
 }

 void testList()
 {
     ListNode* l1 = new ListNode(1);
    // ListNode* l2 = new ListNode(2);
    // l2->next = l1;
     ListNode* l3 = new ListNode(3);
     l3->next = l1;

     printList(l3);
     ListNode* reversed = reverseList(l3);
     printList(reversed);

 }
