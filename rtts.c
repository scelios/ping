#include "ping.h"

static int calcPacketRtt(struct icmphdr *icmph, struct rtt_node *new_rtt)
{
	struct timeval *t_send;
	struct timeval t_recv;

	t_send = ((struct timeval *)skip_icmphdr(icmph));
	if (gettimeofday(&t_recv, NULL) == -1) {
		printf("gettimeofday err: %s\n", strerror(errno));
		return -1;
	}
	timersub(&t_recv, t_send, &new_rtt->val);
	return 0;
}

struct rtt_node * rttSaveNew(struct packinfo *pi, struct icmphdr *icmph)
{
	struct rtt_node *elem = pi->rtt_list;
	struct rtt_node *new_rtt = NULL;

	if ((new_rtt = malloc(sizeof(*new_rtt))) == NULL)
		return NULL;
	if (calcPacketRtt(icmph, new_rtt) == -1)
		return NULL;
	new_rtt->next = NULL;
	if (elem != NULL) {
		while (elem->next)
			elem = elem->next;
		elem->next = new_rtt;
	} else {
		pi->rtt_list = new_rtt;
	}
	pi->rtt_last = new_rtt;
	return new_rtt;
}

void rttClean(struct packinfo *pi)
{
	struct rtt_node *elem = pi->rtt_list;
	struct rtt_node *tmp;

	while (elem) {
		tmp = elem;
		elem = elem->next;
		free(tmp);
	}
}

void calcStddev(struct packinfo *pi, long nb_elem)
{
	struct rtt_node *elem = pi->rtt_list;
	struct timeval *avg = &pi->avg;
	long sec_dev = 0;
	long usec_dev = 0;
	long total_sec_dev = 0;
	long total_usec_dev = 0;

	while (elem) {
		sec_dev = elem->val.tv_sec - avg->tv_sec;
		sec_dev *= sec_dev;
		total_sec_dev += sec_dev;
		usec_dev = elem->val.tv_usec - avg->tv_usec;
		usec_dev *= usec_dev;
		total_usec_dev += usec_dev;
		elem = elem->next;
	}
	if (nb_elem - 1 > 0) {
		total_sec_dev /= nb_elem - 1;
		total_usec_dev /= nb_elem - 1;
		pi->stddev.tv_sec = (long)sqrt(total_sec_dev);
		pi->stddev.tv_usec = (long)sqrt(total_usec_dev);
	} else {
		pi->stddev.tv_sec = 0;
		pi->stddev.tv_usec = 0;
	}
}

/**
Calculate:
- Minimum round-trip time value.
- Maximum round-trip time value.
- Average round-trip time value.
- Standard deviation round-trip time value.
 */
void rttCalcStats(struct packinfo *pi)
{
	struct rtt_node *elem = pi->rtt_list;
	long nb_elem = 0;
	long total_sec = 0;
	long total_usec = 0;

	pi->min = &elem->val;
	pi->max = &elem->val;
	while (elem) {
		if (timercmp(pi->min, &elem->val, >))
			pi->min = &elem->val;
		else if (timercmp(pi->max, &elem->val, <))
			pi->max = &elem->val;

		total_sec += elem->val.tv_sec;
		total_usec += elem->val.tv_usec;
		if (total_usec > 100000) {
			total_usec -= 100000;
			++total_sec;
		}
		++nb_elem;
		elem = elem->next;
	}
	pi->avg.tv_sec = total_sec / nb_elem;
	pi->avg.tv_usec = total_usec / nb_elem;
	calcStddev(pi, nb_elem);
}