#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <linux/if.h>
#include <string.h>

static void usage(const int status)
{
	fprintf(status ? stderr : stdout,"Usage arp_ping: -i interface\n\
options:\n\
	-i, --iface	which interface to use\n\
	-h, --help	display this help and exit\n");

	exit(status);
}

/* get_iface_info: get ip and mac address from the given interface. */
void get_iface_info(const char *iface_name)
{
	struct ifaddrs *addrs, *next;
	unsigned int iface_found;

	getifaddrs(&addrs);
	iface_found = 0;

	next = addrs;
	while (next) {
		if (strncmp(next->ifa_name, iface_name, IFNAMSIZ) == 0) {
			iface_found = 1;	
			break;
		}

		next = next->ifa_next;
	}

	freeifaddrs(addrs);

	if (!iface_found) {
		fprintf(stderr, "error: interface %s not found\n", iface_name);
		exit(EXIT_FAILURE);
	}

	return NULL;
}

int main(int argc, char **argv)
{
	int opt;

	static struct option long_options[] = {
		{"iface", required_argument, NULL, 'i'},
		{"help", no_argument, NULL, 'h'},
		{0, 0, 0, 0}
	};

	while ((opt = 
		getopt_long(argc, argv, "i:", long_options, NULL)) != -1) {
		switch (opt) {
		case 'i':
			get_iface_info(optarg);
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		default:
			usage(EXIT_FAILURE);
		}
	}

	return EXIT_SUCCESS;
}
