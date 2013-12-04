#ifndef _PAYLOAD_H
#define _PAYLOAD_H    1
#include <sys/types.h>
#include <inttypes.h>

struct payload {
	pid_t id;
	uint64_t curr;
	uint64_t req;
};

int init_payload(struct payload *);
void print_payload(struct payload *);

#endif /* payload.h */
