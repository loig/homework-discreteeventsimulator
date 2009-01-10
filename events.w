
@* Events.

[Global view]

@** Data-structures.

@ We start with some macro definition, hence the usual

@(simulator.h@>=
#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdio.h>
#include <stdlib.h>


@ First, we define some types of events. The first one signal the
end of the simulation:

@(simulator.h@>+=
#define END_SIM		0

@ And things we don't know yet

@(simulator.h@>+=
#define ARR		1
#define DEP		2
#define TRAP		3
#define MES		4

@ Our simulator schedule packets in the form of doubly-linked
list. Therefore, an event, conveying a |pckt|, is timestamped by a
|time| field. The doubly-linked list structure is defined with |prev|
and |next|. Moreover, all events are assigned a |type|, as defined
above.

@(simulator.h@>+=
typedef struct _event 
{
  int type;
  double time;
  struct _packet *pckt;
  struct _event *prev;
  struct _event *next;
} Event;


@ Where a packet is defined by its source and destination, as well as
  its position in the network and its length, in bits. For the
  measurements, we also add a |departureTime| field that indicates the
  time at which the packet was sent.

@(simulator.h@>+=
typedef struct _packet {
	int	src;
	int	dst;
	int	position;
	int	length;
        @;
	double	departureTime;
} Packet;

@ To simplify the creation of an event queue, we define the following
macro. This macro creates and allocates the memory for an empty event,
of class |c|, forecasted for time |t|.

@(simulator.h@>+=
#define CREATE_EV(ev, c, t)	{ 					\ 
					ev = xmalloc(sizeof(*ev)) ;	\
					ev->type = c ;			\
					ev->time = t ;			\
					ev->pckt = NULL;                \
					ev->prev = NULL;		\
					ev->next = NULL; 		\
				}

@ The |xmalloc| function is simply an extension of |malloc| that catches
memory exhaustion

@c
#include "simulator.h"

void *xmalloc(size_t size)
{
	void *ptr;
	if ((ptr = malloc(size)) == NULL) {
		err(1, "simulator: exhausted memory");
	} @+ else {
		return ptr;
	}
}


@ And we close this include by

@(simulator.h@>+=
     #endif

@** Event manipulation.

As mentioned above, an event queue will be represented by a
doubly-linked list. Therefore, we have to provide the entry of the
list, |first|, and its exit, |last|.

@c
static Event *first;
static Event *last;

@*3 Event queue initialization.

Let us start with the event queue initialization. First, using the
macro defined above, we allocate a terminal event element, belonging
to the |END_SIM| class and scheduled for the provided time |t|. Then,
we initialize |first| and |last| as pointing to this single event.

@c
void
evlist_init(double t)
{
	Event *ev;

	CREATE_EV(ev, END_SIM, t);
	first = last = ev;
}

@*3 Pop the first element.

Then, an usual task will be to pick the first element of the event
queue, so as to process it. Hence, we define the following function
that copy the content of the first event and free the corresponding
event.

@c
void
evlist_first(int *c, double *t, Packet **pckt)
{
  Event *x;
  x = first;

  first = first->next;

  @<Maintain the backward pointer@>;

  @<Copy the event@>;

  free(x);
}

@ When we pick the first event, we have to null-ify its |prev| pointer
so as to maintain the (backward) list structure. Obviously, if this
first element is empty, then the list invariants trivially hold.

@<Maintain the backward pointer@>=
  if (first != NULL) {
    first->prev = NULL;
  }


@ To retrieve the content of this first element, we simply 

@<Copy the event@>=
  *t = x->time;
  *c = x->type;
  *pckt = x->pckt;


@*3 Event insertion.

@ Finally, the last operation we need is the insertion.

@c
void evlist_insert (Event *event) 
{ 
	Event *currentEvent,
	      *nextEvent, 
	      *previousEvent;
	double time; @;

	double timeEvent = event->time;
	double timeFirstEvent = first->time;
	double timeLastEvent = last->time;

	if (timeEvent < timeFirstEvent) {
	  @<Event goes first@>;
	  return;	      
	}

	if (timeEvent > timeLastEvent) {
	  @<Event goes last@>;
	  return;
	} 

	if (timeEvent - timeFirstEvent < timeLastEvent - timeEvent) { @;
          // Heuristics: insert forward if closer to head
	  @<Insert forward@>;
	} @+ else { 
	  // Heuristics: insert backward if not
	  @<Insert backward@>;
	} 
}

@ When the |event| goes first, it has to next-point to the previous
|first| and the previous |first| has to previous-point to it. And we
signal who is the |first|.

@<Event goes first@>=
@q { @>
    event->next = first;
    first->prev = event;
    first = event;
@q } @>

@ The same goes when the event is the last one.

@<Event goes last@>=
@q { @>
    event->prev = last;
    last->next = event;
    last = event;
@q } @>

@ The forward insertion consists in the trivial list traversal to find
a place to insert, or reaching the end of the list.

@<Insert forward@>=
@q { @>
     currentEvent = first;
     nextEvent = currentEvent->next; @;

     while (nextEvent != NULL) { 
       if (timeEvent < nextEvent->time) { 
	 /* Event goes after currentEvent, before nextEvent */
	 currentEvent->next = event;
	 event->prev = currentEvent;
	 event->next = nextEvent;
	 nextEvent->prev = event;
	 return;
       }
       
       currentEvent = nextEvent;
       nextEvent = nextEvent->next;
     }
		
     /* Event is at the end of the queue */
     currentEvent->next = event;
     event->prev = currentEvent;
     event->next = NULL;
@q } @>

@ Conversely, the backward insertion traverses the list with the
|prev| pointers, until it reaches a good place or the head of the
queue.

@<Insert backward@>=
@q { @>
     currentEvent = last;
     previousEvent = currentEvent->prev;

     while (previousEvent != NULL) { 
       if (timeEvent > previousEvent->time) {
	 /* Event foes after previousEvent, before currentEvent */
	 previousEvent->next = event;
	 event->prev = previousEvent;
	 event->next = currentEvent;
	 currentEvent->prev = event;
	 return;
       }

       currentEvent = previousEvent;
       previousEvent = previousEvent->prev;
     }

     /* Event is at the head of the queue */
     currentEvent->prev = event;
     event->next = currentEvent;
     event->prev = NULL;
@q  } @>
