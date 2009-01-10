@* Events.

The event module implements the famous Event Queue, at the heart of
any Discrete Event Simulator. The implementation is sketched as
follow: first, we define the data-structure as well as some macros to
build them. Then, we define three operations on Event Queues:
initialization, insertion, and picking the first event.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
@*2 Data-structures.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


@ We start with some macro definition, hence the usual

@(simulator_events.h@>=
#ifndef SIMULATOR_EVENTS_H
#define SIMULATOR_EVENTS_H

#include "simulator_network.h"

@ First, we define some types of events:
\item {$\bullet$} |END_SIM| signals the end of the simulation
\item {$\bullet$} |ARRIVAL| signals an arrival at a router
\item {$\bullet$} |DEPARTURE| signals a departure from a router

@(simulator_events.h@>+=
typedef enum _eventType { END_SIM,
			  ARRIVAL,
			  DEPARTURE } EventType;

@q useless @>
@q #define TRAP		3 @>
@q #define MES		4 @>


@ Our simulator schedule packets in the form of doubly-linked
list. Therefore, an event, conveying a |packet|, is timestamped by a
|time| field. The doubly-linked list structure is defined with |prev|
and |next|. Moreover, all events are assigned a |type|, as defined
above.

@(simulator_events.h@>+=
typedef struct _event 
{
  EventType type;
  double time;
  struct _packet *packet;
  struct _event *prev;
  struct _event *next;
} Event;


@ Where a packet is defined by its source and destination, as well as
  its position in the network and its length, in bits.

@(simulator_events.h@>+=
typedef struct _packet {
	TerminalId src;
	TerminalId dst;
	int	   position;
	int	   length;
} Packet;

@ To simplify the creation of an event queue, we define the following
macro. This macro creates and allocates the memory for an empty event,
of type |c|, forecasted for time |t|.

@(simulator_events.h@>+=
#define CREATE_EV(event, c, t) {							\ 
					event = (Event *) xmalloc(sizeof(Event)) ;	\
					event->type = c ;			 	\
					event->time = t ;				\
					event->packet = NULL;				\
					event->prev = NULL;				\
					event->next = NULL;				\
				}

@ The |xmalloc| function is simply an extension of |malloc| that catches
memory exhaustion

@c
#include "simulator.h"

void *xmalloc(size_t size)
{
	void *ptr;
	if ((ptr = malloc(size)) == NULL) {
		err(1, "Simulator: memory exhausted");
	} @+ else {
		return ptr;
	}
}

@ And, as we will use it globally, we advertise its signature in the
header

@(simulator_events.h@>+=
  void *xmalloc(size_t size);


@ And we close this Include by

@(simulator_events.h@>+=
     #endif /* |SIMULATOR_EVENTS_H| */




@q%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@>
@*2 Event manipulation.
@q%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@>





As mentioned above, an event queue will be represented by a
doubly-linked list. Therefore, we have to provide the entry of the
list, |first|, and its exit, |last|.

@c
static Event *first;
static Event *last;


@q%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@>
@*3 Event queue initialization.
@q%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@>


Let us start with the event queue initialization. First, using the
macro defined above, we allocate a terminal event element, belonging
to the |END_SIM| class and scheduled for the provided time |t|. Then,
we initialize |first| and |last| as pointing to this single event.

@c
void
evlist_init(double endSimulationTime)
{
	Event *event;

	CREATE_EV(event, END_SIM, endSimulationTime);
	first = last = event;
}


@q%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@>
@*3 Pop the first element.
@q%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@>


Then, an usual task will be to pick the first element of the event
queue, so as to process it. Hence, we define the following function
that copy the content of the first event and free the corresponding
event.

@c
void
evlist_first(int *type, double *time, Packet **packet)
{
  Event *event;
  event = first;

  first = first->next;

  @<Maintain the backward pointer@>;

  @<Copy the event@>;

  free(event);
}

@ When we pick the first event, we have to null-ify its |prev| pointer
in order to maintain the (backward) list structure. Obviously, if this
first element is empty, then the list invariants trivially hold.

@<Maintain the backward pointer@>=
  if (first != NULL) {
    first->prev = NULL;
  }


@ To retrieve the content of this first element, we simply have to
copy each of the event informations

@<Copy the event@>=
  *time = event->time;
  *type = event->type;
  *packet = event->packet;


@q%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@>
@*3 Event insertion.
@q%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@>


@ Finally, the last operation we need is the insertion.

@c
void evlist_insert (Event *event) 
{ 
	Event *currentEvent,
	      *nextEvent, 
	      *previousEvent;

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
	} @+ else { @/@,
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
         @/@,
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
		
     currentEvent->next = event;
     event->prev = currentEvent;
     event->next = NULL;
     @/@,
     /* Event is at the end of the queue */
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
         @/@,
	 /* Event goes after previousEvent, before currentEvent */
	 previousEvent->next = event;
	 event->prev = previousEvent;
	 event->next = currentEvent;
	 currentEvent->prev = event;
	 return;
       }

       currentEvent = previousEvent;
       previousEvent = previousEvent->prev;
     }

     currentEvent->prev = event;
     event->next = currentEvent;
     event->prev = NULL;
     @/@,
     /* Event is at the head of the queue */
	  
@q  } @>
