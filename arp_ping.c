#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct arp_hdr
{
	uint16_t ah_htype;
	uint16_t ah_ptype;
	uint8_t ah_hlen;
	uint8_t ah_plen;
	uint8_t ah_oper;
	uint8_t ah_sha[6];
	uint8_t ah_spa[4];
	uint8_t ah_tha[6];
	uint8_t ah_tpa[4];
} __attribute__ ((packed));

int main(void)
{
	return EXIT_SUCCESS;
}
