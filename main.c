#include "ping.h"


_Bool pingLoop = 1;
_Bool send_packet = 1;

void handler(int signum)
{
	if (signum == SIGINT)
		pingLoop = 0;
	else if (signum == SIGALRM)
		send_packet = 1;
}

int main(int a, char **b)
{
	int ret;
	int sock_fd;
	char *host = NULL;
	struct options opts = {};
	struct sockinfo si = {};
	struct packinfo pi = {};

	if (checkRights() == false)
	{
		return E_EXIT_ERR_ARGS;
	}
	if ((ret = checkArgs(a,b,&host, &opts)) != 0)
	{
		return ret == -1 ? E_EXIT_ERR_ARGS : E_EXIT_OK;
	}
	if (initSock(&sock_fd, &si, host, IP_TTL_VALUE) == -1)
		return E_EXIT_ERR_HOST;

	signal(SIGINT,&handler);
	signal(SIGALRM,&handler);
	printStartInfo(&si, &opts);
	while(pingLoop)
	{
		if (send_packet == true)
		{
			send_packet = false;
			if (sendPacket(sock_fd,&si,&pi) == -1)
			{
				close(sock_fd);
				rttClean(&pi);
				exit(E_EXIT_ERR_HOST);
			}
			alarm(1);
		}
		if (recvPacket(sock_fd, &pi, &opts) == -1)
		{
			close(sock_fd);
			rttClean(&pi);
			exit(E_EXIT_ERR_HOST);
		}
	}
	printEndInfo(&si, &pi);

	close(sock_fd);
	rttClean(&pi);
	return pi.nb_ok > 0 ? E_EXIT_OK : E_EXIT_ERR_HOST;
}
