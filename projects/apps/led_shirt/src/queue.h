/*
 * queue.h
 *
 *  Created on: Jan 25, 2015
 *      Author: wouters
 */

#ifndef QUEUE_H_
#define QUEUE_H_


/*	queue.h

	Header file for queue implementation

	by: Steven Skiena
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


#define QUEUESIZE       50

typedef struct {
        void* q[QUEUESIZE+1];		/* body of queue */
        int first;                      /* position of first element */
        int last;                       /* position of last element */
        int count;                      /* number of queue elements */
} queue;

void init_queue(queue *q);
void enqueue(queue *q, void* x);
void* dequeue(queue *q);
int isfull(queue *q);
int getqueuesize(queue * q);

#endif /* QUEUE_H_ */
