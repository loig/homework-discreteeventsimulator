#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "util.h"

void	allocation();
int	flow(struct network *, int);
void	getcomments(FILE *);

void
allocation(struct network *net)
{
	int i;

	net->terminals = xmalloc(net->nb_terminals * sizeof(struct terminal));
	for (i = 0; i < net->nb_terminals; i++) {
		net->terminals[i].latency =
		    xmalloc(net->nb_terminals * sizeof(double));
		net->terminals[i].nbpckt_loss =
		    xmalloc(net->nb_terminals * sizeof(double));
		net->terminals[i].tot_pckt =
		    xmalloc(net->nb_terminals * sizeof(double));
		net->terminals[i].tot_pckt_ok =
		    xmalloc(net->nb_terminals * sizeof(double));
	}

	net->routers = xmalloc(net->nb_routers * sizeof(struct router));
	for (i = 0; i < net->nb_routers; i++)
		net->routers[i].rt = xmalloc(net->nb_terminals * sizeof(int));

	net->flows = xmalloc(net->nb_terminals * sizeof(int *));
	for (i = 0; i < net->nb_terminals; i++)
		net->flows[i] = xmalloc(net->nb_terminals * sizeof(int));
}

/*
 * Returns the whole flow starting from <terminal>, i.e. the sum of the
 * line <terminal> in the flow matrix.
 */
int
flow(struct network *net, int terminal)
{
	int j, ret;

	ret = 0;
	for (j = 0; j < net->nb_terminals; j++)
		ret += net->flows[terminal][j];
	return ret;
}

void
get_comments(FILE * fp)
{
	char buf[1024];
	int c;

	while (1) {
		if ((c = getc(fp)) == EOF)
			return;
		if (c == '#') {
			fgets(buf, sizeof(buf), fp);
		}
		else if (c != '\n') {
			ungetc(c, fp);
			return;
		}
	}
}

int
parse_flows(char *path, struct network *net)
{
	FILE *flows;
	int i, j;

	if ((flows = fopen(path, "r")) == NULL)
		err(1, NULL);

	for (i = 0; i < net->nb_terminals; i++)
		for (j = 0; j < net->nb_terminals; j++) {
			get_comments(flows);
			if (fscanf(flows, "%d", &net->flows[i][j]) != 1) {
				fprintf(stderr, "flows file: wrong format\n");
				exit(1);
			}
		}

	for (i = 0; i < net->nb_routers; i++) {
		get_comments(flows);
		if (fscanf(flows, "%d", &net->routers[i].bufsize) != 1) {
			fprintf(stderr, "flows file: wrong format\n");
			exit(1);
		}
	}

	get_comments(flows);
	if (fscanf(flows, "%d", &net->mean_packet_size) != 1) {
		fprintf(stderr, "flows file: wrong format\n");
		exit(1);
	}

	get_comments(flows);
	for (i = 0; i < net->nb_routers; i++) {
		get_comments(flows);
		if (fscanf(flows, "%d", &net->routers[i].bandwidth) != 1) {
			fprintf(stderr, "flows file: wrong format\n");
			exit(1);
		}
	}

	/* Fill the flow field of the terminal structure */
	for (i = 0; i < net->nb_terminals; i++)
		net->terminals[i].traffic = flow(net, i);

	return 0;
}

int
parse_network(char *path, struct network *net)
{
	FILE *network;
	int i, j;

	if ((network = fopen(path, "r")) == NULL)
		err(1, NULL);

	get_comments(network);
	if (fscanf(network, "%d", &net->nb_terminals) != 1) {
		fprintf(stderr, "network file: wrong format\n");
		exit(1);
	}

	get_comments(network);
	if (fscanf(network, "%d", &net->nb_routers) != 1) {
		fprintf(stderr, "network file: wrong format\n");
		exit(1);
	}

	allocation(net);

	for (i = 0; i < net->nb_terminals; i++) {
		get_comments(network);
		if (fscanf(network, "%d", &net->terminals[i].gateway) != 1) {
			fprintf(stderr, "network file: wrong format\n");
			exit(1);
		}
	}

	for (i = 0; i < net->nb_routers; i++)
		for (j = 0; j < net->nb_terminals; j++) {
			get_comments(network);
			if (fscanf(network, "%d", &net->routers[i].rt[j]) != 1) {
				fprintf(stderr, "network file: wrong format\n");
				exit(1);
			}
		}

	return 0;
}
