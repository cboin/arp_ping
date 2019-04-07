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
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <sys/socket.h>
#include <unistd.h>

struct iface_info_s *get_iface_info(const char *iface_name);
void free_iface_info(struct iface_info_s *iface_info);
void ping_broadcast(const struct iface_info_s *info);

struct iface_info_s {
	struct in_addr	*if_inet_addr;
	unsigned char	*if_hw_addr;
};

struct arp_pkt_s {
	uint16_t	ap_htype;
	uint16_t	ap_ptype;
	uint8_t		ap_hlen;
	uint8_t		ap_plen;
	uint16_t	ap_oper;
	uint8_t		ap_sha[ETH_ALEN];
	uint32_t	ap_spa;
	uint8_t		ap_tha[ETH_ALEN];
	uint32_t	ap_tpa;
} __attribute__((packed));

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

	info->if_inet_addr = malloc(sizeof(struct in_addr));
	if (info->if_inet_addr == NULL) {
		free(info);
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	memcpy(info->if_inet_addr, in_addr, sizeof(struct in_addr));

	info->if_hw_addr = malloc(sll->sll_halen * sizeof(unsigned char));
	if (info->if_hw_addr == NULL) {
		free(info->if_inet_addr);
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
	free(info->if_inet_addr);
	free(info->if_hw_addr);
	free(info);
}

void ping_broadcast(const struct iface_info_s *info)
{
	struct arp_pkt_s *ap;

	ap = malloc(sizeof(struct arp_pkt_s));
	if (ap == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	
	ap->ap_htype = ARPHRD_ETHER;
	ap->ap_ptype = ETH_P_IP;
	ap->ap_hlen = sizeof(info->if_hw_addr);
	ap->ap_plen = sizeof(info->if_inet_addr->s_addr);
	ap->ap_oper = ARPOP_REQUEST;
	memcpy(ap->ap_sha, info->if_hw_addr, sizeof(ap->ap_sha));
	ap->ap_spa = info->if_inet_addr->s_addr;
	explicit_bzero(ap->ap_tha, sizeof(ap->ap_tha));

}

int main(int argc, char **argv)
{
	int opt;
	struct iface_info_s *info;

	static struct option long_options[] = {
		{"iface", required_argument, NULL, 'i'},
		{"help", no_argument, NULL, 'h'},
		{0, 0, 0, 0}
	};

	while ((opt = 
			getopt_long(argc, argv, "i:h", long_options, NULL)) != -1) {
		switch (opt) {
			case 'i':
				info = get_iface_info(optarg);
				break;
			case 'h':
				usage(EXIT_SUCCESS);
				break;
			default:
				usage(EXIT_FAILURE);
		}
	}

	ping_broadcast(info);

	free_iface_info(info);

	return EXIT_SUCCESS;
}
