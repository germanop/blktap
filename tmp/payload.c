#include <stdio.h>
#include "payload.h"

int init_payload(struct payload *pload)
{
	pload->id = -1;
	pload->curr = 0;
	pload->req = 0;
	return 0;
}

void print_payload(struct payload *pload)
{
	printf("payload data:\n");
	printf("id = %d\n", pload->id);
	printf("current size = %"PRIu64"\n", pload->curr);
	printf("requested size = %"PRIu64"\n", pload->req);
	return;
}
