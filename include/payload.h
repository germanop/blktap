#ifndef _PAYLOAD_H
#define _PAYLOAD_H    1
#include <sys/types.h>
#include <inttypes.h>

struct payload {
	pid_t id;
	uint64_t curr;
	uint64_t req;
	off64_t vhd_size;
};

int init_payload(struct payload *);
void print_payload(struct payload *);

/* Temporary location */
int thin_sock_comm(struct payload *);

#endif /* payload.h */
