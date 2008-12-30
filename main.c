/*
 * Network simulator for the PEV course
 * by Houssein Hwebe and Pierre Riteau, 2008
 */

#include <err.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "events.h"
#include "includes.h"
#include "parser.h"
#include "regress.h"
#include "util.h"

#define UNIF()		drand48()
#define EXPO(M)		(-M * log(UNIF()))
#define MAX(a,b)	(a > b) ? a : b

void		print_stats(double t_warm);
void		sched_termpckt(int);
void		simulation(double);
double		stddev(double, double, int);
void		usage();

struct network	net;
double		t_cur;
double		t_simu;
int		plm, blmc, blmf;
int		regress;

int
main(int argc, char **argv)
{
	int	 ch, i, j;
	double	 t_warm;
	char	*fnetwork, *fflows;

	/* PLM is the default */
	plm = 1;
	blmc = blmf = 0;
	regress = 0;
	fnetwork = fflows = NULL;
	while ((ch = getopt(argc, argv, "cfpr")) != -1) {
		switch (ch) {
		case 'c':
			blmc = 1;
			blmf = plm = 0;
			break;
		case 'f':
			blmf = 1;
			blmc = plm = 0;
			break;
		case 'p':
			plm = 1;
			blmc = blmf = 0;
			break;
		case 'r':
			regress = 1;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 5)
		usage();

	/* Read configuration files */
	parse_network(argv[0], &net);
	parse_flows(argv[1], &net);

	/* Seed the pseudo random number generator */
	srand48(atol(argv[2]));

	/* Get simulation times */
	t_warm = atof(argv[3]);
	t_simu = atof(argv[4]);
	if (t_warm >= t_simu) {
		fprintf(stderr, "error: warmup time is too big\n");
		exit(1);
	}

	/* Initialize statistics */
	for (i = 0; i < net.nb_terminals; i++)
		for (j = 0; j < net.nb_terminals; j++) {
			net.terminals[i].latency[j] = 0.0;
			net.terminals[i].nbpckt_loss[j] = 0;
			net.terminals[i].tot_pckt[j] = 0;
			net.terminals[i].tot_pckt_ok[j] = 0;
		}

	for (i = 0; i < net.nb_routers; i++) {
		net.routers[i].backlog_bits = 0.0;
		net.routers[i].backlog_pckts = 0;
		net.routers[i].mbacklog.area = 0.0;
		net.routers[i].mbacklog.last_time = 0.0;
		net.routers[i].mbacklog.nb_obs = 0;
		net.routers[i].mbacklog.raw = 0.0;
		net.routers[i].mbacklog.square = 0.0;
		net.routers[i].t_prec = 0.0;
		net.routers[i].latency = 0.0;
		net.routers[i].latency2 = 0.0;
		net.routers[i].lossrate = 0.0;
		net.routers[i].pckt_in = 0;
		net.routers[i].pckt_out = 0;
		net.routers[i].throughput_in = 0.0;
		net.routers[i].throughput_out = 0.0;
		net.routers[i].workrate = 0.0;
	}

	/* Initialize the event list */
	evlist_init(t_simu);

	/* Now start the simulation */
	simulation(t_warm);

	return 0;
}



/*
 * Schedule a new packet outgoing of <terminal> with data from the flow matrix.
 */
void
sched_termpckt(int terminal)
{
	int		 flowsum, j, termflow;
	double		 date, unif;
	struct event	*ev;

	flowsum = 0;
	termflow = net.terminals[terminal].traffic;

	/* If no flow is going out of this terminal, just drop the scheduling */
	if (termflow == 0)
		return;

	date = t_cur + EXPO(1 / (double) termflow);

	/*
	 * If the packet will be out after the simulation is finished,
	 * just drop the scheduling.
	 */
	if (date > t_simu)
		return;

	/* Create packet with our terminal as source */
	CREATE_EV(ev, ARR, date);
	ev->pckt = xmalloc(sizeof(struct packet));
	ev->pckt->src = terminal;

	/* Find out who is the destination */
	unif = UNIF();
	for (j = 0; j < net.nb_terminals; j++) {
		flowsum += net.flows[terminal][j];
		if (unif < (flowsum / (double) termflow)) {
			ev->pckt->dst = j;
			break;
		}
	}

	/* The packet starts its life in the terminal's gateway */
	ev->pckt->where = net.terminals[terminal].gateway;
	ev->pckt->t_dep = ev->time;

	/* Packet lengths follow an exponential law */
	ev->pckt->length = EXPO(net.mean_packet_size);
	net.terminals[terminal].tot_pckt[ev->pckt->dst]++;

	/* Add event to the event list */
	evlist_insert(ev);
}

void
simulation(double t_warm)
{
	struct event	*ev;
	struct packet	*pckt;
	double		 period;
	int		 class, first, i;

	first = 1;
	period = 1.0;

	CREATE_EV(ev, MES, period);
	ev->pckt = NULL;
	evlist_insert(ev);

	/* Initialize end-of-work time for each router */
	for (i = 0; i < net.nb_routers; i++)
		net.routers[i].t_free = 0.0;

	/* Generate a packet from each terminal */
	if (!regress)
		for (i = 0; i < net.nb_terminals; i++)
			sched_termpckt(i);
	else
		load_events(&net);

	/* Main simulation loop */
	do {
		struct router	 *router;
		int		 id_router, nexthop;

		/* Get the first evenement in the list */
		evlist_first(&class, &t_cur, &pckt);

		if (t_cur >= t_warm)
			if (first) {
				for (i = 0; i < net.nb_routers; i++)
					net.routers[i].t_prec = t_warm;
				first = 0;
			}

		if (pckt != NULL) {
			double prev_backlog;
			id_router = pckt->where;
			router = &net.routers[id_router];
			prev_backlog = router->backlog_bits;

			/* For BLMC, update the backlog utilisation, in any case */
			if (blmc && router->backlog_pckts != 0)
				router->backlog_bits -= router->bandwidth * (t_cur - router->t_prec);

			if (t_cur >= t_warm) {
				/* Compute intregral of backlog from the last event */
				if (plm)
					router->mbacklog.area += router->backlog_pckts * (t_cur - router->t_prec);
				else if (blmf)
					router->mbacklog.area += router->backlog_bits * (t_cur - router->t_prec);
				else
					router->mbacklog.area += (router->backlog_bits + (prev_backlog - router->backlog_bits) / 2) * (t_cur - router->t_prec);
			}
		}

		/* This is just for debugging and regress tests */
		if (class == TRAP) {
			for (i = 0; i < net.nb_routers; i++) {
				id_router = i;
				router = &net.routers[id_router];
				if (t_cur >= t_warm) {
					double prev_backlog = router->backlog_bits;

					/* For BLMC, update the backlog utilisation, in any case */
					if (blmc && router->backlog_pckts != 0)
						router->backlog_bits -= router->bandwidth * (t_cur - router->t_prec);

					if (plm)
						router->mbacklog.area += router->backlog_pckts * (t_cur - router->t_prec);
					else if (blmf)
						router->mbacklog.area += router->backlog_bits * (t_cur - router->t_prec);
					else
						router->mbacklog.area += (router->backlog_bits + (prev_backlog - router->backlog_bits) / 2) * (t_cur - router->t_prec);
				}
				router->t_prec = t_cur;
			}
		}

		switch (class) {

		/* Arrival in a router */
		case ARR:
			/*
			 * If the packet comes from a terminal
			 * we must schedule its next packet
			 */
			if (net.terminals[pckt->src].gateway == id_router)
				if (!regress)
					sched_termpckt(pckt->src);

			/* Statistics */
			if (t_cur >= t_warm) {
				router->throughput_in += pckt->length;
				router->pckt_in++;
				if (router->backlog_pckts != 0)
					router->workrate += t_cur - router->t_prec;
			}

			/* Is the buffer full? */
			if (plm) {
				if (router->bufsize == router->backlog_pckts) {
					if (t_cur >= t_warm) {
						/* Buffer is full! */
						router->lossrate++;
						net.terminals[pckt->src].nbpckt_loss[pckt->dst]++;
					}
					break;
				}
			}
			else if ((router->bufsize != -1) && (router->bufsize < router->backlog_bits + pckt->length)) {
				if (t_cur >= t_warm) {
					router->lossrate++;
					net.terminals[pckt->src].nbpckt_loss[pckt->dst]++;
				}
				break;
			}

			/* Increase backlog */
			if (blmf || blmc)
				router->backlog_bits += pckt->length;
			router->backlog_pckts++;

			/* Update t_free */
			router->t_free = MAX(t_cur, router->t_free);

			/* Latency statistics */
			if (t_cur >= t_warm) {
				router->latency += (router->t_free + pckt->length / (double) router->bandwidth) - t_cur;
				router->latency2 += pow((router->t_free + pckt->length / (double) router->bandwidth) - t_cur, 2);
			}

			struct event *ev;
			CREATE_EV(ev, DEP,
			    router->t_free + pckt->length /
			    (double) router->bandwidth);
			router->t_free += pckt->length /
			    (double) router->bandwidth;
			ev->pckt = pckt;
			evlist_insert(ev);
			break;

		/* Departure from a router */
		case DEP:
			if (blmf)
				router->backlog_bits -= pckt->length;

			/* One packet less in the router */
			router->backlog_pckts--;

			/* Where shall it go? */
			nexthop = net.routers[id_router].rt[pckt->dst];

			/*
			 * We are arrived at the destination terminal.
			 * You can now unfasten your seat belt.
			 */
			if (nexthop == -1) {
				if (t_cur >= t_warm) {
					net.terminals[pckt->src].latency[pckt->dst] += t_cur - pckt->t_dep;
					net.terminals[pckt->dst].tot_pckt_ok[pckt->src]++;
				}
				free(pckt);
			}
			else {
				/* Nexthop is a router, give it the packet */
				struct event *ev;
				CREATE_EV(ev, ARR, t_cur);
				ev->pckt = pckt;
				pckt->where = nexthop;
				evlist_insert(ev);
			}

			/* Statistics */
			if (t_cur >= t_warm) {
				router->throughput_out += pckt->length;
				router->pckt_out++;
				router->workrate += t_cur - router->t_prec;
			}
			break;
		case TRAP:
			printf("TRAP: %f s\n", t_cur);
			for (i = 0; i < net.nb_routers; i++) {
				printf("Router %d: %d p, %f b\n", i,
				    net.routers[i].backlog_pckts,
				    net.routers[i].backlog_bits);
			}
			break;
		case MES:
			/* To calculte the backlog periodically and keep track of variance */
			for (i = 0; i < net.nb_routers; i++) {
				router = &net.routers[i];
				if (t_cur >= t_warm) {
					router->mbacklog.nb_obs++;
					if (plm) {
						router->mbacklog.raw += net.mean_packet_size * router->mbacklog.area / (t_cur - router->mbacklog.last_time);
						router->mbacklog.square += net.mean_packet_size * pow(router->mbacklog.area / (t_cur - router->mbacklog.last_time), 2);
					}
					else {
						router->mbacklog.raw += router->mbacklog.area / (t_cur - router->mbacklog.last_time);
						router->mbacklog.square += pow(router->mbacklog.area / (t_cur - router->mbacklog.last_time), 2);
					}
				}
				router->mbacklog.last_time = t_cur;
				router->mbacklog.area = 0;
			}
			CREATE_EV(ev, MES, t_cur + period);
			ev->pckt = NULL;
			evlist_insert(ev);
			break;
		case END_SIM:
			break;
		default:
			fprintf(stderr, "Should not happen!\n");
			exit(1);
			break;
		}
		if (class != TRAP && class != MES)
			router->t_prec = t_cur;
	} while (class != END_SIM);

	print_stats(t_warm);
}

void
print_stats(double t_warm)
{
	int i, j;

	printf("Simulation time: %f s\n", t_simu);
	printf("Statistics time: %f s\n\n", t_cur - t_warm);
	printf("\t\tUtil (%%)\tBcklg ");
	printf("%s", plm ? "(bits)" : "(bits)");
	printf("\tDelay (s)\tInput (bps)\tOutput (bps)\tLoss (%%)\n");
	for (i = 0; i < net.nb_routers; i++) {
		struct router	*router;
		double		 backlog_stddev, latency_stddev, m2;

		router = &net.routers[i];

		/* Mean backlog statistics */
		router->mbacklog.raw /= router->mbacklog.nb_obs;
		m2 = router->mbacklog.square / (router->mbacklog.nb_obs - 1);
		backlog_stddev = sqrt(fabs(m2 - pow(router->mbacklog.raw, 2) * router->mbacklog.nb_obs / (router->mbacklog.nb_obs - 1)));

		/* Latency */
		latency_stddev = stddev(router->latency, router->latency2, router->pckt_out);
		router->latency /= (double) router->pckt_out;

		printf("Router %d\t", i);
		printf("%f\t", 100 * router->workrate / (t_cur - t_warm));
		printf("%f\t", router->mbacklog.raw);
		printf("%f\t", router->latency);
		printf("%f\t", router->throughput_in / (t_cur - t_warm));
		printf("%f\t", router->throughput_out / (t_cur - t_warm));
		printf("%f\t", 100 * router->lossrate / (double) router->pckt_in);
		printf("\n");
		printf("95%% confidence interval for backlog: [%f,%f], ",
				router->mbacklog.raw - (1.96 * backlog_stddev / sqrt(router->mbacklog.nb_obs)),
				router->mbacklog.raw + (1.96 * backlog_stddev / sqrt(router->mbacklog.nb_obs)));
		printf("relative error: %f %%\n", 100 * (1.96 * backlog_stddev / sqrt(router->mbacklog.nb_obs) / router->mbacklog.raw));
		printf("95%% confidence interval for latency: [%f,%f], ",
		    router->latency - (1.96 * latency_stddev / sqrt((double) router->pckt_out)),
		    router->latency + (1.96 * latency_stddev / sqrt((double) router->pckt_out)));
		printf("relative error: %f %%\n", 100 * (1.96 * latency_stddev / sqrt((double)router->pckt_out) / router->latency));
		printf("\n");
	}
	printf("\n");

	printf("\t\t");
	for (i = 0; i < net.nb_terminals; i++) {
		printf("Delay to T%d\t", i);
		printf("Loss to T%d (%%)\t", i);
	}
	printf("\n");

	for (i = 0; i < net.nb_terminals; i++) {
		printf("Terminal %d\t", i);
		for (j = 0; j < net.nb_terminals; j++) {
			if (net.terminals[j].tot_pckt_ok[i] != 0)
				printf("%f\t", net.terminals[i].latency[j] / (double) net.terminals[j].tot_pckt_ok[i]);
			else
				printf("0\t\t");

			if (net.terminals[j].tot_pckt_ok[i] + net.terminals[i].nbpckt_loss[j] == 0)
				printf("%f\t", 0.0);
			else
				printf("%f\t", 100.0 * (double) net.terminals[i].nbpckt_loss[j] / (net.terminals[j].tot_pckt_ok[i] + net.terminals[i].nbpckt_loss[j]));

		}
		printf("\n");
	}
}

double
stddev(double val, double squareval, int nb_obs)
{
	val /= (double) nb_obs;
	squareval /= (double) nb_obs;
	return sqrt(fabs(squareval - (val * val)));
}

void
usage()
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-cfp] network flows seed warmup_time "
	    "simu_time\n", __progname);
	exit(1);
}
