#include "ping.h"

static unsigned short checksum(unsigned short *ptr, int nbytes) {
	unsigned long sum;
	unsigned short oddbyte;

	sum = 0;
	while (nbytes > 1) {
		sum += *ptr++;
		nbytes -= 2;
	}
	if (nbytes == 1) {
		oddbyte = 0;
		*((unsigned char *)&oddbyte) = *(unsigned char *)ptr;
		sum += oddbyte;
	}
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return (unsigned short) ~sum;
}

static int fill_icmp_echo_packet(uint8_t *buf, int packet_len)
{
	static int seq = 0;
	struct icmphdr *hdr = (struct icmphdr *)buf;
	struct timeval *timestamp = skip_icmphdr(buf);

	if (gettimeofday(timestamp, NULL) == -1) {
		printf("gettimeofday err: %s\n", strerror(errno));
		return -1;
	}
	hdr->type = ICMP_ECHO;
	hdr->un.echo.id = getpid();
	hdr->un.echo.sequence = seq++;
	hdr->checksum = checksum((unsigned short *)buf, packet_len);
	return 0;
}

int sendPacket(int sock_fd, const struct sockinfo *si, struct packinfo *pi)
{
	ssize_t nb_bytes;
	uint8_t buf[sizeof(struct icmphdr) + ICMP_BODY_SIZE] = {};

	if (fill_icmp_echo_packet(buf, sizeof(buf)) == -1)
		return -1;
	nb_bytes = sendto(sock_fd, buf, sizeof(buf), 0, (const struct sockaddr *)&si->remote_addr, sizeof(si->remote_addr));
	if (nb_bytes == -1)
	{
		if (errno == EACCES) {
			printf("ft_ping: socket access error. Are you trying to ping broadcast ?\n");
		} else {
			printf("sendto err: %s\n", strerror(errno));
		}
		return -1;
	}
	pi->nb_send++;
	return 0;
}