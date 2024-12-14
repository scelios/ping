#include "ping.h"

static _Bool isAddressedToUs(uint8_t *buf)
{
	struct icmphdr *hdr_sent;
	struct icmphdr *hdr_rep = (struct icmphdr *)buf;

	if (hdr_rep->type == ICMP_ECHO)
		return 0;

	/* If error, jumping to ICMP sent packet header stored in body */
	if (hdr_rep->type != ICMP_ECHOREPLY)
		buf += ICMP_HDR_SIZE + IP_HDR_SIZE;
	hdr_sent = (struct icmphdr *)buf;

	return hdr_sent->un.echo.id == getpid();
}

int recvPacket(int sock_fd, struct packinfo *pi, const struct options *opts)
{
	uint8_t buf[RECV_PACK_SIZE] = {};
	ssize_t nb_bytes;
	struct icmphdr *icmph;
	struct iovec iov[1] = {
		[0] = { .iov_base = buf, .iov_len = sizeof(buf)}
	};
	struct msghdr msg = { .msg_iov = iov, .msg_iovlen = 1 };

	nb_bytes = recvmsg(sock_fd,&msg,MSG_DONTWAIT);
	if (errno != EAGAIN && errno != EWOULDBLOCK && nb_bytes == -1)
	{
		printf("recvmsg err: %s\n", strerror(errno));
		return -1;
	} 
	else if (nb_bytes == -1)
	{
		return 0;
	}
	icmph = skip_iphdr(buf);
	if (!isAddressedToUs((uint8_t *)icmph))
		return 0;
	if (icmph->type == ICMP_ECHOREPLY) {
		pi->nb_ok++;
		if (rttSaveNew(pi, icmph) == NULL)
			return -1;
	}
	if (printRecvInfo(buf, nb_bytes, opts, pi) == -1)
		return -1;
	return 1;
}


