
/*	queue.c

	Implementation of a FIFO queue abstract data type.

	by: Steven Skiena
	begun: March 27, 2002
*/


/*
Copyright 2003 by Steven S. Skiena; all rights reserved.

Permission is granted for use in non-commerical applications
provided this copyright notice remains intact and unchanged.

This program appears in my book:

"Programming Challenges: The Programming Contest Training Manual"
by Steven Skiena and Miguel Revilla, Springer-Verlag, New York 2003.

See our website www.programming-challenges.com for additional information.

This book can be ordered from Amazon.com at

http://www.amazon.com/exec/obidos/ASIN/0387001638/thealgorithmrepo/

*/


#include "queue.h"

static char queue_lock = 0;
void init_queue(queue *q)
{
        q->first = 0;
        q->last = QUEUESIZE-1;
        q->count = 0;
}

void enqueue(queue *q, void* x)
{
        if (q->count >= QUEUESIZE) {
        	//printf("Warning: queue overflow enqueue x=%d\n",x);
        }
        else {
        		while(queue_lock);
        		queue_lock = 1;
                q->last = (q->last+1) % QUEUESIZE;
                q->q[ q->last ] = x;
                q->count = q->count + 1;
                queue_lock = 0;
        }
}

void* dequeue(queue *q)
{
	void* x = 0;

        if (q->count <= 0) {
        	//printf("Warning: empty queue dequeue.\n");
        }
        else {
			while(queue_lock);
			queue_lock = 1;
			x = q->q[ q->first ];
			q->first = (q->first+1) % QUEUESIZE;
			q->count = q->count - 1;
			queue_lock = 0;
        }

        return(x);
}
int isfull(queue *q) {
	 if (q->count >= QUEUESIZE) {
		 return 1;
	}
	return 0;
}
int getqueuesize(queue *q) {
	return q->count;
}

int empty(queue *q)
{
        if (q->count <= 0) return (1);
        else return (0);
}


