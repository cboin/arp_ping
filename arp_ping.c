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
	struct in_addr *if_in_addr;
	unsigned char *if_hw_addr;
};

static void usage(const int status)
{
	fprintf(status ? stderr : stdout, "Usage arp_ping: [-h] -i interface\n\
options:\n\
	-i, --iface	which interface to use\n\
	-h, --help	display this help and exit\n");

	exit(status);
}

static struct iface_info_s *create_iface_info(const struct in_addr *in_addr, 
	const struct sockaddr_ll *sll)
{
	struct iface_info_s *info;

	info = malloc(sizeof(struct iface_info_s));
	if (info == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	info->if_in_addr = malloc(sizeof(struct in_addr));
	if (info->if_in_addr == NULL) {
		free(info);
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	memcpy(info->if_in_addr, in_addr, sizeof(struct in_addr));

	info->if_hw_addr = malloc(sll->sll_halen * sizeof(unsigned char));
	if (info->if_hw_addr == NULL) {
		free(info->if_in_addr);
		free(info);
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	memcpy(info->if_hw_addr, sll->sll_addr, 
		sll->sll_halen * sizeof(unsigned char));

	return info;
}

/* get_iface_info: get ip and mac address from the given interface. */
struct iface_info_s *get_iface_info(const char *ifname)
{
	struct ifaddrs *addrs, *next;
	struct in_addr sa_in;
	struct sockaddr_ll sll;

	if (getifaddrs(&addrs) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	explicit_bzero(&sa_in, sizeof(struct in_addr));
	explicit_bzero(&sll, sizeof(struct sockaddr_ll));

	next = addrs;
	while (next) {
		if (strncmp(next->ifa_name, ifname, IFNAMSIZ) == 0) {
			switch (next->ifa_addr->sa_family) {
			case AF_INET:
				memcpy(&sa_in,
					&((struct sockaddr_in *) next->ifa_addr)->sin_addr,
					sizeof(struct in_addr));
				break;

			case AF_PACKET:
				memcpy(&sll, 
					next->ifa_addr, 
					sizeof(struct sockaddr_ll));
				break;
			}
		}

		next = next->ifa_next;
	}

	freeifaddrs(addrs);

	if (!sa_in.s_addr || !sll.sll_addr) {
		fprintf(stderr, "ERROR: interface %s not found\n", ifname);
		exit(EXIT_FAILURE);
	}

	return create_iface_info(&sa_in, &sll);	
}

/* free_iface_info: frees the memory space pointed to by info */
void free_iface_info(struct iface_info_s *info)
{
	free(info->if_in_addr);
	free(info->if_hw_addr);
	free(info);
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
