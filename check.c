#include "ping.h"

bool checkRights()
{
	if (getuid() != 0) {
		printf("ft_ping: usage error: need to be run as root\n");
		return false;
	}
	return true;
}

bool parseOpts(char *arg, struct options *opts)
{
	static const char supported_opts[] = "hqv";
	char *match = NULL;
	size_t len = strlen(arg);

	for (size_t i = 1; i < len; ++i)
	{
		if ((match = strchr(supported_opts, arg[i])) != NULL)
		{
			switch (*match)
			{
			case 'h':
				opts->help = 1;
				break;
			case 'q':
				opts->quiet = 1;
				break;
			case 'v':
				opts->verb = 1;
				break;
			default:
				printf("ping: unknown option\n");
			}
		}
		else
		{
			printf("ft_ping: invalid option -- '%c'\n", arg[i]);
			printf("Try 'ft_ping -h' for more information.\n");
			return false;
		}
	}
	return true;
}

int checkArgs(int a, char **b, char **host, struct options *opts)
{
	int nbHost = 0;
	for (int i = 1; i < a; i++)
	{
		if (b[i][0] == '-' && strlen(b[i]) > 1)
		{
			if (parseOpts(b[i],opts) == false) 
				return -1;
		}
		else
		{
			nbHost++;
			*host = b[i];
		}
		if (opts->help)
		{
			printHelp();
			return 1;
		}
	}
	if (!nbHost)
	{
		printf("ft_ping: missing host operand\n");
		printf("Try 'ft_ping -h' for more information.\n");
		return ERROR;
	}
	else if (nbHost > 1)
	{
		printf("ft_ping: only one host is needed\n");
		printf("Try 'ft_ping -h' for more information.\n");
		return ERROR;
	}

	return 0;
}


