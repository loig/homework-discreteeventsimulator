@ 

@(simulator.h@>=
#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <assert.h>

#define UNIF()		drand48()
#define EXPO(M)		(-M * log(UNIF()))
#define MAX(a,b)	(a > b) ? a : b

@ 

@(simulator.h@>+=
#include "simulator_events.h"

@ 

@(simulator.h@>+=
#include "simulator_network.h"

@ And we close this include by

@(simulator.h@>+=
#endif /* |SIMULATOR_H| */

@

@c
double currentTime;
double endSimulationTime;
int averagePacketSize;