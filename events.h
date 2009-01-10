#ifndef EVENTS_H
#define EVENTS_H

#include "includes.h"
#include "util.h"

/* Definition of event types */
#define END_SIM		0
#define ARR		1
#define DEP		2
#define TRAP		3
#define MES		4

struct event {
   int			 class;
   double		 time;
   struct packet	*pckt;
   struct event		*prev;
   struct event		*next;
};

void	evlist_first(int *, double *, struct packet **);
void	evlist_init(double);
void	evlist_insert(struct event *);


#define CREATE_EV(ev, c, t)	{					\
					ev = xmalloc(sizeof(*ev));	\
					ev->class = c;			\
					ev->time = t;			\
					ev->prev = NULL;		\
					ev->next = NULL;		\
				}

#endif	/* EVENTS_H */
