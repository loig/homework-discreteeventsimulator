#include "parser.h"

void	allocation();
int	flow(Network *, int);
void	getcomments(FILE *);

void
allocation(Network *net)
{
	int i;

	net->terminals = (Terminal *) xmalloc(net->noTerminals * sizeof(Terminal));
	net->routers = (Router *) xmalloc(net->noRouters * sizeof(Router));

	for (i = 0; i < net->noRouters; i++){
	  net->routers[i].routingTable = (int *) xmalloc(net->noTerminals * sizeof(int));
	}

	net->flows = (int **) xmalloc(net->noTerminals * sizeof(int *));
	for (i = 0; i < net->noTerminals; i++){
	  net->flows[i] = (int *) xmalloc(net->noTerminals * sizeof(int));
	}
}

/*
 * Returns the whole flow starting from <terminal>, i.e. the sum of the
 * line <terminal> in the flow matrix.
 */
int
flow(Network *net, int terminal)
{
	int j, ret;

	ret = 0;
	for (j = 0; j < net->noTerminals; j++){
		ret += net->flows[terminal][j];
	}

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
parse_flows(char *path, Network *net)
{
	FILE *flows;
	int i, j;

	if ((flows = fopen(path, "r")) == NULL)
		err(1, NULL);

	for (i = 0; i < net->noTerminals; i++)
		for (j = 0; j < net->noTerminals; j++) {
			get_comments(flows);
			if (fscanf(flows, "%d", &net->flows[i][j]) != 1) {
				fprintf(stderr, "flows file: wrong format\n");
				exit(1);
			}
		}

	for (i = 0; i < net->noRouters; i++) {
		get_comments(flows);
		if (fscanf(flows, "%d", &net->routers[i].bufferSize) != 1) {
			fprintf(stderr, "flows file: wrong format\n");
			exit(1);
		}
	}

	get_comments(flows);
	if (fscanf(flows, "%d", &averagePacketSize) != 1) {
		fprintf(stderr, "flows file: wrong format\n");
		exit(1);
	}

	get_comments(flows);
	for (i = 0; i < net->noRouters; i++) {
		get_comments(flows);
		if (fscanf(flows, "%d", &net->routers[i].bandwidth) != 1) {
			fprintf(stderr, "flows file: wrong format\n");
			exit(1);
		}
	}

	/* Fill the flow field of the terminal structure */
	for (i = 0; i < net->noTerminals; i++)
		net->terminals[i].traffic = flow(net, i);

	return 0;
}

int
parse_network(char *path, Network *net)
{
	FILE *network;
	int i, j;

	if ((network = fopen(path, "r")) == NULL)
		err(1, NULL);

	get_comments(network);
	if (fscanf(network, "%d", &net->noTerminals) != 1) {
		fprintf(stderr, "network file: wrong format\n");
		exit(1);
	}

	get_comments(network);
	if (fscanf(network, "%d", &net->noRouters) != 1) {
		fprintf(stderr, "network file: wrong format\n");
		exit(1);
	}

	allocation(net);

	for (i = 0; i < net->noTerminals; i++) {
		get_comments(network);
		if (fscanf(network, "%d", &net->terminals[i].gateway) != 1) {
			fprintf(stderr, "network file: wrong format\n");
			exit(1);
		}
	}

	for (i = 0; i < net->noRouters; i++)
		for (j = 0; j < net->noTerminals; j++) {
			get_comments(network);
			if (fscanf(network, "%d", &net->routers[i].routingTable[j]) != 1) {
				fprintf(stderr, "network file: wrong format\n");
				exit(1);
			}
		}

	return 0;
}
