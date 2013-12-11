#ifndef _PAYLOAD_H
#define _PAYLOAD_H    1
#include <sys/types.h>
#include <inttypes.h>

typedef enum {
	/* server */
	PAYLOAD_ACCEPTED = 0,
	PAYLOAD_REJECTED,
	PAYLOAD_DONE,
	PAYLOAD_WAIT,
	/* client */
	PAYLOAD_QUERY,
	PAYLOAD_REQUEST,
	/* generic */
	PAYLOAD_UNDEF
} payload_message_t;

struct payload {
	pid_t id;
	uint64_t curr;
	uint64_t req;
	off64_t vhd_size;
	payload_message_t reply;
};

int init_payload(struct payload *);
void print_payload(struct payload *);

/* Temporary location */
int thin_sock_comm(struct payload *);

#endif /* payload.h */
