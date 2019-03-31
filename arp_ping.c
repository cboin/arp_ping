#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <linux/if.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct iface_info_s *get_iface_info(const char *iface_name);
void free_iface_info(struct iface_info_s *iface_info);

struct iface_info_s
{
	char if_name[IFNAMSIZ];
	char if_inet_addr_str[INET_ADDRSTRLEN];
};

static void usage(const int status)
{
	fprintf(status ? stderr : stdout,"Usage arp_ping: -i interface\n\
options:\n\
	-i, --iface	which interface to use\n\
	-h, --help	display this help and exit\n");

	exit(status);
}

/* get_iface_info: get ip and mac address from the given interface. */
struct iface_info_s *get_iface_info(const char *iface_name)
{
	struct ifaddrs *addrs, *next, *iface;
	struct iface_info_s *iface_info;

	if (getifaddrs(&addrs) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	iface = NULL;

	next = addrs;
	while (next) {
		if ((strncmp(next->ifa_name, iface_name, IFNAMSIZ) == 0) 
			&& (next->ifa_addr->sa_family == AF_INET)) {
			iface = next;
			break;
		}

		next = next->ifa_next;
	}

	freeifaddrs(addrs);

	if (!iface) {
		fprintf(stderr, "error: interface %s not found\n", iface_name);
		exit(EXIT_FAILURE);
	}

	iface_info = 
		(struct iface_info_s *) malloc(sizeof(struct iface_info_s));

	if (iface_info == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	strncpy(iface_info->if_name, next->ifa_name, IFNAMSIZ);
	inet_ntop(AF_INET, &((struct sockaddr_in *) iface->ifa_addr)->sin_addr,
		iface_info->if_inet_addr_str, INET_ADDRSTRLEN);
	/* TODO: get interface mac address. */


	return iface_info;
}

/* free_iface_info: frees the memory space pointed to by iface_info */
void free_iface_info(struct iface_info_s *iface_info)
{
	free(iface_info);
}

int main(int argc, char **argv)
{
	int opt;
	struct iface_info_s *iface_info;

	static struct option long_options[] = {
		{"iface", required_argument, NULL, 'i'},
		{"help", no_argument, NULL, 'h'},
		{0, 0, 0, 0}
	};

	while ((opt = 
		getopt_long(argc, argv, "i:", long_options, NULL)) != -1) {
		switch (opt) {
		case 'i':
			iface_info = get_iface_info(optarg);
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		default:
			usage(EXIT_FAILURE);
		}
	}

	printf("iface_name: %s\n", iface_info->if_name);
	printf("iface_inet_addr_str: %s\n", iface_info->if_inet_addr_str);
	free_iface_info(iface_info);

	return EXIT_SUCCESS;
}
