/*
 * Inspired from pss.c by Gerardo Rubino, but implemented a doubly linked-list
 * to improve performances.
 * When inserting a new event, start from head or tail of the list, depending
 * which is closer to the event time
 * (may not be always good, but it is a simle heuristic and it works fine)
 */

#include <stdio.h>
#include <stdlib.h>

#include "events.h"

struct event	*first;
struct event	*last;

void
evlist_insert(struct event *ev)
{
	struct event *x, *y;
	double time;

	time = ev->time;
	if (time < first->time) {
		ev->next = first;
		first->prev = ev;
		first = ev;
		return;
	}
	if (time > last->time) {
		ev->prev = last;
		last->next = ev;
		last = ev;
		return;
	}

	if (time - first->time < last->time - time) {
		/* here, ev goes at least at 2nd position */
		x = first;
		y = x->next;
		while (y != NULL) {
			if (time < y->time) {
				/* --- ev goes after x end before y */
				x->next = ev;
				ev->prev = x;
				ev->next = y;
				y->prev = ev;
				return;
			}
			x = y;
			y = y->next;
		}
		/* ev goes after the whole list */
		x->next = ev;
		ev->prev = x;
		ev->next = NULL;
	}
	else {
		x = last;
		y = x->prev;
		while (y != NULL) {
			if (time > y->time) {
				y->next = ev;
				ev->prev = y;
				ev->next = x;
				x->prev = ev;
				return;
			}
			x = y;
			y = y->prev;
		}
		x->prev = ev;
		ev->next = x;
		ev->prev = NULL;
	}
}

void
evlist_first(int *c, double *t, struct packet **pckt)
{
	struct event *x;

	x = first;
	first = first->next;
	if (first != NULL)
		first->prev = NULL;
	*t = x->time;
	*c = x->class;
	*pckt = x->pckt;
	free(x);
}

void
evlist_init(double t)
{
	struct event *ev;

	CREATE_EV(ev, END_SIM, t);
	ev->pckt = NULL;
	first = last = ev;
}
