#ifndef INCLUDES_H
#define INCLUDES_H

struct stat {
	double	area;
	double	last_time;
	int	nb_obs;
	double	raw;
	double	square;
};

struct terminal {
	int	gateway;
	int	traffic;
	/* Statistics */
	double	*latency;	/* Latency from this terminal to tj */
	int	*tot_pckt;	/* Total number of packets sent to tj */
	int	*tot_pckt_ok;	/* Total number of packets arrived to tj */
	int	*nbpckt_loss;	/* Number of packets loss from ti to tj */
};

struct router {
	int	*rt;		/* Routing table */
	int	 bufsize;	/* -1 = infinite = without loss */
	int	 bandwidth;	/* bits/s */
	double	 t_free;
	double   t_prec;
	double	 backlog_bits;		/* Current utilisation of the buffer */
	int	 backlog_pckts;		/* Current utilisation of the buffer */
	int	 pckt_in;		/* Total number of packets in */
	int	 pckt_out;		/* Total number of packets out */
	/* Statistics */
	struct stat	mbacklog;
	double		latency;
	double		latency2;
	double		lossrate;
	double		throughput_in;
	double		throughput_out;
	double		workrate;
};

struct packet {
	int	src;		/* Source terminal */
	int	dst;		/* Destination terminal */
	int	where;		/* In which router is the packet right now? */
	double	t_dep;
	int	length;		/* bits */
};

struct network {
	int		  nb_terminals;
	struct terminal  *terminals;
	int		  nb_routers;
	struct router	 *routers;
	int		  mean_packet_size;	/* in bits */
	int		**flows;		/* flow from ti to tj in pps */
};

#endif /* INCLUDES_H */
