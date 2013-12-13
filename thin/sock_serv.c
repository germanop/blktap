#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "blktap.h"
#include "payload.h"

#define BACKLOG 5

static inline int process_payload(int, struct payload *);
static inline int dummy_reply(int, struct payload *);
static int (*req_reply)(int , struct payload * );
static int handle_request(struct payload * buf);
static int handle_query(struct payload * buf);

int
main(int argc, char *argv[]) {
	struct sockaddr_un addr;
	int sfd, cfd;
	ssize_t numRead;
	struct payload buf;

	req_reply = dummy_reply;

	sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sfd == -1)
		return -errno;

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, THIN_CONTROL_SOCKET, sizeof(addr.sun_path) - 1);

	if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
		return -errno;

	if (listen(sfd, BACKLOG) == -1)
		return -errno;

	for (;;) {

		cfd = accept(sfd, NULL, NULL);
		if (cfd == -1)
			return -errno;

		while ((numRead = read(cfd, &buf, sizeof(buf))) > 0)
			process_payload(cfd, &buf);

		if (numRead == -1)
			return -errno;

		if (close(cfd) == -1)
			return -errno;
	}
}

static inline int
process_payload(int fd, struct payload * buf)
{
	int err;

	print_payload(buf);
	err = req_reply(fd, buf);

	return err;
}

static int
dummy_reply(int fd, struct payload * buf)
{
	switch (buf->reply) {
	case PAYLOAD_REQUEST:
		handle_request(buf);
		break;
	case PAYLOAD_QUERY:
		handle_query(buf);
		break;
	default:
		buf->reply = PAYLOAD_UNDEF;
		print_payload(buf);
	}
	print_payload(buf);
	printf("EOM\n\n");

	/* TBD: very basic write, need a while loop */
	if (write(fd, buf, sizeof(*buf)) != sizeof(*buf))
		return -errno;

	return 0;
}

static int
handle_request(struct payload * buf)
{
	if (buf->reply != PAYLOAD_REQUEST)
		return 1;

	printf("I promise I will do something about it..");
	buf->reply = PAYLOAD_ACCEPTED;

	return 0;
}

static int
handle_query(struct payload * buf)
{
	if (buf->reply != PAYLOAD_QUERY)
		return 1;

	printf("Working on it.. be patient");
	buf->reply = PAYLOAD_WAIT;

	return 0;
}
