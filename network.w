@* Network Topology.

In this module, we mostly describe memory structure only. These
structures will hold the representation of the simulated network,
along with its terminals and routers.

@ So, first, the usual

@(simulator_network.h@>=
#ifndef SIMULATOR_NETWORK_H
#define SIMULATOR_NETWORK_H


@ A network is characterized by: a set of terminals, of cardinal
|noTerminals| stored in array pointed by |terminals|; a set of
routers, of cardinal |noRouters| stored in an array pointed by
|routers|; and a flow graph (in packets per second), stored in a matrix
pointed by |flows|.

@(simulator_network.h@>+=
@q { @>
     typedef struct _network {
       int noTerminals;
       struct _terminal *terminals;
       @;
       int noRouters;
       struct _router *routers;
       @;
       int **flows;
     } Network;
@q } @>

@ A terminal is characterized by the address of its entry-point in the
network, the so-called |gateway|, as well its |traffic|, ie. the
number of packets per second it can send on the network.

@(simulator_network.h@>+=
@q { @>
     typedef struct _terminal {
       int      gateway;
       int      traffic;
     } Terminal;
@q } @>

@ And instead of IP addresses, we use |TerminalId| to represent
network addresses:

@(simulator_network.h@>+=
typedef int TerminalId;

  @ A router is a more complex beast, as it features:
\item {$\bullet$} a routing table |routingTable|
\item {$\bullet$} the size of its buffer |bufferSize|, 
      with ``$-1$'' corresponding to an infinite buffer
\item {$\bullet$} its |bandwidth|, in bits per second
\item {$\bullet$} its backlog, in bits with |backlogBits| 
                  and in packets with |backlogPackets|
\item {$\bullet$} the previous time the simulator has updated its backlog
\item {$\bullet$} the date at which the current sent packet will be released

@(simulator_network.h@>+=
@q { @>
   typedef struct _router {
       int *routingTable;
       int bufferSize;
       int bandwidth;
       double backlogBits;
       int backlogPackets;
       double previousTime;
       double dateFree;
} Router;

@q } @>

@ As for terminals, we use |RouterId| to represent the network
addresses of routers:

@(simulator_network.h@>+=
typedef int RouterId;

@ We can close our header then

@(simulator_network.h@>+=
#endif /* |SIMULATOR_NETWORK_H| */


@ And we are done with network data-structures. Hence, we can declare
a static |network| data-structure:

@c
  static Network network;

