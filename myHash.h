#ifndef MYHASH_H  
#define MYHASH_H_

struct node{
    int64_t key;
    int val[2];
    struct node *next;
};

struct table{
    int size;
    struct node **list;
};

struct table *createTable(int size);
int hashCode(struct table *t,int64_t key);
void insert(struct table *t,int64_t key,int secr);
void printTable(struct table *t, int redirect);
void freeTable(struct table *t);

#endif 
