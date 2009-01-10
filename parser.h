#ifndef PARSER_H
#define PARSER_H

#include "simulator.h"
#include "simulator_network.h"
#include "simulator_events.h"

int	parse_flows(char *, Network *);
int	parse_network(char *, Network *);

#endif	/* PARSER_H */
