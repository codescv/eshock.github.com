#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct NodeStruct {
    int val;
    struct NodeStruct *next;
};

typedef struct NodeStruct Node;

Node* find_and_delete(Node *head, int target)
{
    Node *prev = NULL;
    Node *entry = head;
    while (entry != NULL) {
        if (entry->val == target) {
            if (prev) {
                prev->next = entry->next;
            } else {
                head = entry->next;
            }
            break;
        }
        prev = entry;
        entry = entry->next;
    }
    return head;
}

void find_and_delete2(Node **head, int target)
{
    while (*head != NULL) {
        Node *entry = *head;
        if (entry->val == target) {
            *head = entry->next;
            break;
        }
        head = &(entry->next);
    }
}

Node *make_list(int a[], int n)
{
    Node *result = NULL;
    Node *runner;
    int i;
    if (n == 0) {
        return NULL;
    }
    result = (Node *)malloc(sizeof(Node));
    result->val = a[0];
    runner = result;
    for (i = 1; i < n; i++) {
        runner->next = (Node *)malloc(sizeof(Node));
        runner->next->val = a[i];
        runner = runner->next;
    }
    return result;
}

void print_list(Node *l)
{
    while (l) {
        printf("%d", l->val);
        if (l->next) {
            printf("->");
        } else {
            printf("\n");
        }
        l = l->next;
    }
}

int main(int argc, const char *argv[])
{
    int a[] = {1,2,3,4,5};
    Node *l = make_list(a, 5);
    print_list(l);
    l = find_and_delete(l, 3);
    print_list(l);
    l = find_and_delete(l, 1);
    print_list(l);
    Node *l2 = make_list(a,5);
    print_list(l2);
    find_and_delete2(&l2, 3);
    print_list(l2);
    find_and_delete2(&l2, 1);
    print_list(l2);
    return 0;
}
