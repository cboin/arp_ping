#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <linux/if.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>

struct iface_info_s *get_iface_info(const char *iface_name);
void free_iface_info(struct iface_info_s *iface_info);

struct iface_info_s
{
	char if_inet_addr[INET_ADDRSTRLEN];
	char if_inet6_addr[INET6_ADDRSTRLEN];
	unsigned char if_hw_addr[IFHWADDRLEN];
};

static void usage(const int status)
{
	fprintf(status ? stderr : stdout, "Usage arp_ping: [-h] -i interface\n\
options:\n\
	-i, --iface	which interface to use\n\
	-h, --help	display this help and exit\n");

	exit(status);
}

/* get_iface_info: get ip and mac address from the given interface. */
struct iface_info_s *get_iface_info(const char *iface_name)
{
	struct ifaddrs *addrs, *next;
	struct iface_info_s *iface_info;
	unsigned char halen;
	int iface_found;

	if (getifaddrs(&addrs) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	iface_info = malloc(sizeof(struct iface_info_s));
	if (iface_info == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	iface_found = 0;
	next = addrs;
	while (next) {
		if (strncmp(next->ifa_name, iface_name, IFNAMSIZ) == 0) {
			if (!iface_found) {
				iface_found = 1;
			}

			switch (next->ifa_addr->sa_family) {
			case AF_INET:
				inet_ntop(AF_INET,
					&((struct sockaddr_in *) next->ifa_addr)->sin_addr, 
					iface_info->if_inet_addr, 
					INET_ADDRSTRLEN);
				break;

			case AF_INET6:
				inet_ntop(AF_INET6,
					&((struct sockaddr_in6 *) next->ifa_addr)->sin6_addr,
					iface_info->if_inet6_addr,
					INET6_ADDRSTRLEN);
				break;

			case AF_PACKET:
				halen = ((struct sockaddr_ll *) next->ifa_addr)->sll_halen;
				if (halen == IFHWADDRLEN) {
					memcpy(iface_info->if_hw_addr, 
							&((struct sockaddr_ll *) next->ifa_addr)->sll_addr, 
							IFHWADDRLEN);
				}
				break;
			}
		}

		next = next->ifa_next;
	}

	freeifaddrs(addrs);

	if (!iface_found) {
		free(iface_info);
		fprintf(stderr, "error: interface %s not found\n", iface_name);
		exit(EXIT_FAILURE);
	}

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
			getopt_long(argc, argv, "i:h", long_options, NULL)) != -1) {
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

	free_iface_info(iface_info);

	return EXIT_SUCCESS;
}
