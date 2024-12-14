#ifndef PING_H
#define PING_H

#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <netinet/ip.h> 
#include <netdb.h>
#define IP_TTL_VALUE 64

#define RECV_PACK_SIZE ((IP_HDR_SIZE + ICMP_HDR_SIZE) * 2 + ICMP_BODY_SIZE + 1)

#define IP_HDR_SIZE (sizeof(struct iphdr))
#define ICMP_HDR_SIZE (sizeof(struct icmphdr))
#define ICMP_BODY_SIZE 56

#define ERROR -1
/**
 * Ft_ping exit codes.
 * @E_EXIT_OK: At least one ICMP echo reply packet was received from target.
 * @E_EXIT_ERR_HOST: No ICMP echo reply packet was received from target.
 * @E_EXIT_ERR_ARGS: An error occured while parsing arguments.
 */
enum e_exitcode {
	E_EXIT_OK,
	E_EXIT_ERR_HOST,
	E_EXIT_ERR_ARGS = 64
};

struct options {
	_Bool help;
	_Bool quiet;
	_Bool verb;
};

struct rtt_node {
	struct timeval val;
	struct rtt_node *next;
};

struct packinfo {
	int nb_send;
	int nb_ok;
	struct timeval *min;
	struct timeval *max;
	struct timeval avg;
	struct timeval stddev;
	struct rtt_node *rtt_list;
	struct rtt_node *rtt_last;
};


struct sockinfo {
	char *host;
	struct sockaddr_in remote_addr;
	char str_sin_addr[INET_ADDRSTRLEN];
};

static inline void * skip_iphdr(void *buf)
{
	return (void *)((uint8_t *)buf + IP_HDR_SIZE);
}

static inline void * skip_icmphdr(void *buf)
{
	return (void *)((uint8_t *)buf + ICMP_HDR_SIZE);
}

bool checkRights();
int checkArgs(int ac, char **av, char **host, struct options *opts);

int initSock(int *sock_fd, struct sockinfo *si, char *host, int ttl);

int sendPacket(int sock_fd, const struct sockinfo *si, struct packinfo *pi);
int recvPacket(int sock_fd, struct packinfo *pi, const struct options *opts);

void printHelp();
void printStartInfo(const struct sockinfo *si, const struct options *opts);
int printRecvInfo(void *buf, ssize_t nb_bytes, const struct options *opts, const struct packinfo *pi);
void printEndInfo(const struct sockinfo *si, struct packinfo *pi);

struct rtt_node * rttSaveNew(struct packinfo *pi, struct icmphdr *icmph);
void rttClean(struct packinfo *pi);
void rttCalcStats(struct packinfo *pi);

#endif 