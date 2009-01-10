@* Prelude.

Before getting started, we have to layout our code a little. Hence, we
start with a header file for the whole simulator.

@ Hence, we import the usual libraries

@(simulator.h@>=
#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <assert.h>
#include <math.h>

@ We also include our own libraries, featuring the event queue, the
network representation, and the parser

@(simulator.h@>+=
#include "simulator_events.h"
#include "simulator_network.h"
#include "parser.h"

@ And define the usual macros to generate random numbers

@(simulator.h@>+=
#define UNIF()  @, drand48()
#define EXPO(M)  @, (-M * log(UNIF()))
#define MAX(a,b) @, (a > b) ? a : b

@ The simulation can be executed in several modes: either
Packet-Level, or Bit-Level with Memory Continuously Freed, or
Bit-Level with Memory freed when transmission is Finished.

@(simulator.h@>+=
typedef enum _simulationMode { PLM, BLMC, BLMF } SimulationMode;


@ The following constants represent:
\item {$\bullet$} the current time
\item {$\bullet$} the final simulation time, as chosen by the user
\item {$\bullet$} the mean size of packets, as chosen by the user
\item {$\bullet$} the simulation mode, as chosen by the user

@(simulator.h@>+=
double currentTime;
double endSimulationTime;
int averagePacketSize;
SimulationMode simulationMode;

@ In order to be able to change the distribution law of random
numbers, we use the following functions, which allow us to choose the
distribution of packet sizes and the emission rate.

@(simulator.h@>+=
double (*randSendDate)(double);
double (*randPacketSize)(double);


@ And we close this header by

@(simulator.h@>+=
#endif /* |SIMULATOR_H| */