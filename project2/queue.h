#ifndef queue_h
#define queue_h

#include "tools.h"

extern pthread_cond_t CV;

struct node {
  
    struct table * table;
    struct node * next;
};

#endif /* queue_h */