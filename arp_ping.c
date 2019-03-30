#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>

static void usage(const int status)
{
	fprintf(status ? stderr : stdout,"Usage arp_ping: -i interface\n\
options:\n\
	-i, --iface	which interface to use\n\
	-h, --help	display this help and exit\n");

	exit(status);
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
			/* TODO: get ip and hardware address of given iface. */
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
