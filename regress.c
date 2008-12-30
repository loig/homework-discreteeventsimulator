#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "events.h"
#include "includes.h"
#include "regress.h"

void
load_events(struct network *net)
{
	struct event    *ev;
	char		*s;
	char		 buf[BUFSIZ];
	int		 type;
	double		 time;

	while (1) {
		s = fgets(buf, sizeof(buf), stdin);
		if (s == NULL) {
			if (ferror(stdin))
				err(1, NULL);
			return;
		}
		else if (*s == '\0') {
			fprintf(stderr, "Error in regress input\n");
			exit(1);
		}
			

		if (buf[0] == 'a')
			type = ARR;
		else if (buf[0] == 't')
			type = TRAP;
		else {
			fprintf(stderr, "Error in regress input\n");
			exit(1);
		}

		s++;
		while (isspace(*s))
			s++;
		if (sscanf(s, "%lf", &time) != 1)
			err(1, NULL);
		while (!isspace(*s))
			s++;
		
		if (type == ARR) {
			int	dst, length, src;

			CREATE_EV(ev, ARR, time);
			if (sscanf(s, "%d %d %d", &src, &dst, &length) != 3)
				exit(1);
			ev->pckt = xmalloc(sizeof(struct packet));
			ev->pckt->src = src;
			ev->pckt->dst = dst;
			ev->pckt->where = net->terminals[src].gateway;
			ev->pckt->length = length;
			ev->pckt->t_dep = time;
			ev->time = time;
			net->terminals[src].tot_pckt[dst]++;
			evlist_insert(ev);
		}
		else {
			CREATE_EV(ev, TRAP, time);
			ev->pckt = NULL;
			evlist_insert(ev);
		}
	}
}
