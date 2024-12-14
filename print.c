#include "ping.h"

void printHelp()
{
	printf("Usage: ping [OPTION...] HOST ...\n"
	       "Send ICMP ECHO_REQUEST packets to network hosts.\n\n"
	       " Options:\n"
	       "  -h                 Show help\n"
	       "  -q                 Quiet output\n"
	       "  -v                 Verbose output\n");
}

void printStartInfo(const struct sockinfo *si, const struct options *opts)
{
	int pid;

	printf("PING %s (%s): %d data bytes", si->host, si->str_sin_addr, ICMP_BODY_SIZE);
	if (opts->verb) {
		pid = getpid();
		printf(", id 0x%04x = %d", pid, pid);
	}
	printf("\n");
}

static void printIcmpRtt(const struct timeval *rtt)
{
	long msec;
	long usec;

	msec = rtt->tv_sec * 1000 + rtt->tv_usec / 1000;
	usec = rtt->tv_usec % 1000;
	usec %= 1000;
	printf("%ld,%03ld", msec, usec);
}

int printRecvInfo(void *buf, ssize_t nb_bytes, const struct options *opts, const struct packinfo *pi)
{
	char addr[INET_ADDRSTRLEN] = {};
	struct iphdr *iph = buf;
	struct icmphdr *icmph = skip_iphdr(iph);

	inet_ntop(AF_INET, &iph->saddr, addr, INET_ADDRSTRLEN);
	if (!opts->quiet && icmph->type == ICMP_ECHOREPLY)
	{
		printf("%ld bytes from %s: ", nb_bytes - IP_HDR_SIZE, addr);
		printf("icmp_seq=%d ttl=%d time=", icmph->un.echo.sequence,iph->ttl);
		printIcmpRtt(&pi->rtt_last->val);
		printf(" ms\n");
	}
	return 0;
}

static inline float calcPacketLoss(const struct packinfo *pi)
{
	return (1.0 - (float)(pi->nb_ok) / (float)pi->nb_send) * 100.0;
}

void printEndInfo(const struct sockinfo *si, struct packinfo *pi)
{
	printf("\n--- %s ping statistics ---\n", si->host);
	printf("%d packets transmitted, %d packets received, %d%% packet loss\n",\
			 pi->nb_send, pi->nb_ok, (int)calcPacketLoss(pi));
	if (pi->nb_ok) {
		rttCalcStats(pi);
		printf("round-trip min/avg/max/stddev = ");
		printIcmpRtt(pi->min);
		printf("/");
		printIcmpRtt(&pi->avg);
		printf("/");
		printIcmpRtt(pi->max);
		printf("/");
		printIcmpRtt(&pi->stddev);
		printf(" ms\n");
	}
}