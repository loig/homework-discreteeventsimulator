


This CWeb is a translation of Pierre Riteau and Houssein Wehbe
simulator, written for the 2007-2008 session of the PEV lecture. 



@* Events.

[Global view]

@** Data-structures.

@ We start with some macro definition, hence the usual

@(simulator.h@>=
#ifndef SIMULATOR_H
#define SIMULATOR_H


@ First, we define some classes of events. The first one signal the
end of the simulation:

@(simulator.h@>+=
#define END_SIM		0

@(simulator.h@>+=
#define ARR		1
#define DEP		2
#define TRAP		3
#define MES		4

@ Our simulator schedule packets in the form of doubly-linked
list. Therefore, an event, conveying a |pckt|, is timestamped by a
|time| field. The doubly-linked list structure is defined with |prev|
and |next|. Moreover, all events are assigned a |class|, as defined
above.

@(simulator.h@>+=
typedef struct _event 
{
    int Class; 
    double Time;
    packet *Pckt;
    event *Prev;
    event *Next;
} event;

@ To simplify the creation of an event queue, we define the following
macro. This macro creates and allocates the memory for an empty event,
of class |c|, forecasted for time |t|.

@(simulator.h@>+=
#define CREATE_EV(ev, c, t)	{ 					\ 
					ev = xmalloc(sizeof(*ev)) ;	\
					ev->Class = c ;			\
					ev->Time = t ;			\
					ev->Pckt = NULL;                \
					ev->Prev = NULL;		\
					ev->Next = NULL; 		\
				}

@ The |xmalloc| function is simply an extension of |malloc| that catches
memory exhaustion

@c
xmalloc(size_t size)
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
static event *first;
static event *last;

@*3 Event queue initialization.

Let us start with the event queue initialization. First, using the
macro defined above, we allocate a terminal event element, belonging
to the |END_SIM| class and scheduled for the provided time |t|. Then,
we initialize |first| and |last| as pointing to this single event.

@c
void
evlist_init(double t)
{
	event *ev;

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
evlist_first(int *c, double *t, struct packet **pckt)
{
  event *x;
  x = first;

  first = first->Next;

  @<Maintain the backward pointer@>;

  @<Copy the event@>;

  free(x);
}

@ When we pick the first event, we have to null-ify its |prev| pointer
so as to maintain the (backward) list structure. Obviously, if this
first element is empty, then the list invariants trivially hold.

@<Maintain the backward pointer@>=
  if (first != NULL) {
    first->Prev = NULL;
  }


@ To retrieve the content of this first element, we simply 

@<Copy the event@>=
  *t = x->Time;
  *c = x->Class;
  *pckt = x->Pckt;


@*3 Event insertion.

@ Finally, the last operation we need is the insertion.

@c
void
evlist_insert(struct event *event)
{

	struct event *currentEvent,
	             *nextEvent, 
	             *previousEvent;
	double time;

	timeEvent = event->Time;
	timeFirstEvent = first->Time;
	timeLastEvent = last->Time;

	if (timeEvent < timeFirstEvent) {
	  @<Event goes first@>;
	  return;	      
	}

	if (timeEvent > timeLastEvent) {
	  @<Event goes last@>;
	  return;
	}

	/* Heuristic: insert forward if closer to head, backward if not */
	if (timeEvent - timeFirstEvent < timeLastEvent - timeEvent) {
	  <@Insert forward@>;
	} else {
	  <@Insert backward@>;
	}
}

@ 

@<Event goes first@>=
  {
    event->Next = first;
    first->Prev = event;
    first = event;
  }

@ qsdqsd

@<Event goes last@>=
  event->Prev = last;
  last = event;

@ qsdqdsq

@<Insert forward@>=
  currentEvent = first;
  nextEvent = currentEvent->Next;

  while (nextEvent != NULL) {
    if (timeEvent < nextEvent->Time) {
      /* Event goes after currentEvent, before nextEvent */
      currentEvent->next = event;
      event->Prev = currentEvent;
      event->Next = nextEvent;
      nextEvent->prev = event;
      return;
    }

    currentEvent = nextEvent;
    nextEvent = nextEvent->Next;
  }
		
  /* Event is at the end of the queue */
  currentEvent->next = event;
  event->Prev = currentEvent;
  event->Next = NULL;

@ qsdqdqs

<@Insert backward@>=
  {
    currentEvent = last;
    previousEvent = currentEvent->Prev;

    while (previousEvent != NULL) {
      if (timeEvent > previousEvent->time) {
	/* Event foes after previousEvent, before currentEvent */
	previousEvent->Next = event;
	event->Prev = previousEvent;
	event->Next = currentEvent;
	currentEvent->Prev = event;
	return;
      }

      currentEvent = previousEvent;
      previousEvent = previousEvent->Prev;
    }

    /* Event is at the head of the queue */
    currentEvent->Prev = event;
    event->Next = currentEvent;
    event->Prev = NULL;
  }

@* Index. Here is a list that shows where the identifiers of this program are
defined and used.
