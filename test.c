#include<stdio.h>
#include<stdlib.h>

struct node
{
    char* data;
    struct node *next;
};


void addLast(struct node **head, char* val)
{
    //create a new node
    struct node *newNode = malloc(sizeof(struct node));
    newNode->data = val;
    newNode->next     = NULL;

    //if head is NULL, it is an empty list
    if(*head == NULL)
         *head = newNode;
    //Otherwise, find the last node and add the newNode
    else
    {
        struct node *lastNode = *head;

        //last node's next address will be NULL.
        while(lastNode->next != NULL)
        {
            lastNode = lastNode->next;
        }

        //add the newNode at the end of the linked list
        lastNode->next = newNode;
    }

}

void printList(struct node *head)
{
    struct node *temp = head;

    //iterate the entire linked list and print the data
    while(temp != NULL)
    {
         printf("%s->", temp->data);
         temp = temp->next;
    }
    printf("NULL\n");
}

int main()
{
     struct node *head = NULL;

     addLast(&head,"here");
     addLast(&head,"is");
     addLast(&head,"text");

     printList(head);

     return 0;
}