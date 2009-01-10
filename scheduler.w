@* The Packet Scheduler.

We generate network traffic on the end-hosts, ie. terminals. 

@ Hence, the scheduling of a packet for a given terminal |terminalId|
proceeds as follow:

@c
  void schedulePacket(TerminalId terminalId, @|
       		      double (*randSendDate)(int), @|
		      int (*randPacketSize)(int))
{
  
  int maxFlow = network.terminals[terminalId].traffic;
  /* Retrieve this node traffic capability */

  
  if (maxFlow <= 0)
    return; 
  /* If it is not able to send anything, we just quit the scheduling */

  double sendDate = currentTime + (*randSendDate)(maxFlow);
  /* We randomly get a sending time */

  if (sendDate > endSimulationTime)
    return;
  /* If the packet will be out after the simulation is finished, just
     drop the scheduling.
  */

  @<Allocate an event and a packet@>;
  @<Set packet's destination@>;

  event->packet->position = network.terminals[terminalId].gateway;
  /* We put the packet in the terminal gateway's pipe */

  event->packet->length = (*randPacketSize)(averagePacketSize);
  /* Tie the coin to get its length */

  evlist_insert(event);
  /* And add it to the event queue */

}

@ To allocate the event, we simply use the macro defined in the
previous section, setting its type to |ARR| and its send time to
|sendDate|. We also allocate a packet and already set its source as
being the current terminal.

@<Allocate an event and a packet@>=
  Event *event;
  CREATE_EV(event, ARR, sendDate);
  event->packet = xmalloc(sizeof(Packet));
  event->packet->src = terminalId;

@ Then, we randomly find a destination. Or we are supposed to. In
fact, I'm not sure to understand this code$\ldots$

@<Set packet's destination@>=
  @q What is this doing ? @>
  double unif = UNIF();
  int flowSum = 0;
  for (int j = 0; j < network.noTerminals; j++) {
    flowSum += network.flows[terminalId][j];
    if (unif < (flowSum / (double) maxFlow)) {
      event->packet->dst = j;
      break;
    }
  }

