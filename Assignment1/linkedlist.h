#include <stdio.h>
#include <stdlib.h>
typedef struct p_node{
  char* name;
  pid_t pid;
  int active;
  struct p_node* next;
}p_node;
p_node* newNode(char* name, pid_t pid, int active){
  p_node* p=(p_node*)malloc(sizeof(p_node));
  p->name=name;
  p->pid=pid;
  p->active=active;
  p->next=NULL;
  return p;
}
p_node* append(p_node* p,p_node* root){
  if(root == NULL){
      root=p;
  }
  else {
    p_node* temp=root;
    while(temp->next!=NULL){
      temp=temp->next;
    }
    temp->next=p;
  }
  return root;
}
// void printList(p_node* root){
//   if(root==NULL)perror("Fail to print list, list empty.\n");
//   p_node* temp=root;
//   while(temp!=NULL){
//     printf("name=%s, id=%d, active=%d\n",temp->name,temp->pid,temp->active);
//     temp=temp->next;
//   }
// }
p_node* findById(pid_t pid,p_node* root){
  p_node* p=root;
  while(p!=NULL){
    if (p->pid==pid){
      return p;
    }
    p=p->next;
  }
  printf("Fail, process not in list.\n");
  return NULL;
}
p_node* deleteById(pid_t pid, p_node* root){
  p_node* p=root;
  if(p==NULL){
    printf("%s\n","No process existing.");
    return root;
  }else if(p->pid==pid){
    root=p->next;
    free(p);
    return root;
  }
  while(p->next!=NULL){
    if (p->next->pid==pid){
      p_node* temp = p->next;
      p->next=p->next->next;
      free(temp);
      return root;
    }
    p=p->next;
  }
  printf("%s\n","Fail to delete, process not in list.\n" );
  return root;
}
