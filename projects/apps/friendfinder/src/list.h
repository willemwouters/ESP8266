/*
 * list.h
 *
 *  Created on: May 10, 2015
 *      Author: wouters
 */

#ifndef LIST_H_
#define LIST_H_

struct linked_node {
	void * data;
	struct linked_node * prev;
	struct linked_node * next;
	void * list;
};


struct linked_list {
	int totalcount;
	struct linked_node * first;
	struct linked_node * last;
};

struct linked_node * linked_insertnode(struct linked_list * list, void * d);
struct linked_node * linked_removenode(struct linked_list * list, struct linked_node * n);
#endif /* LIST_H_ */
