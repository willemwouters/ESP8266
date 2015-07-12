/*
 * list.c
 *
 *  Created on: May 10, 2015
 *      Author: wouters
 */

#include "list.h"
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "mem.h"

static char linked_lock = 0;

struct linked_node *  linked_insertnode(struct linked_list * list, void * d)
{
	struct linked_node* n = (struct linked_node*) os_malloc(sizeof(struct linked_node));
	while(linked_lock);
	linked_lock = 1;
	if(list->totalcount == 0) {
		list->first = n;
		list->last = n;
		list->totalcount = 1;
		n->list = list;
		n->prev = 0;
	} else {
		n->list = n;
		n->prev = list->last;
		list->last->next = n;
		list->last = n;
		list->totalcount++;
	}

	n->next = 0;
	n->data = d;
	linked_lock = 0;
	//puts("Insert node");
	return n;

}

struct linked_node * linked_removenode(struct linked_list * list, struct linked_node * n) {
	if(list->totalcount <= 0) {
		os_printf("Error removing node \r\n");
		return 0;
	}
	while(linked_lock);
		linked_lock = 1;

	int i;
	int found = 0;
	struct linked_node* ln = list->first;
	while(ln != NULL) {
			if(ln == n) {
				found = 1;
			}

			ln = ln->next;
	}
	if(found == 0) {
		os_printf("Couldnt find \r\n");
		linked_lock = 0;
		return 0;
	}
	list->totalcount--;
	struct linked_node * ntmp;

	if(n->prev != 0) {
		n->prev->next = n->next;
	}
	if(n->next != 0) {
		n->next->prev = n->prev;
	}
	ntmp = n->next;
	if(list->totalcount == 1) {
		if(n->prev != 0) {
			list->first = n->prev;
			list->last = n->prev;
		} else if(n->next != 0) {
			list->first = n->next;
			list->last = n->next;
		}
	}
	linked_lock = 0;
	os_free(n);
	return ntmp;
}
