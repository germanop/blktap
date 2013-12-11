#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "blktap.h"
#include "payload.h"

#define BACKLOG 5

static inline int process_request(int, struct payload *);
static inline int dummy_reply(int, struct payload *);

int
main(int argc, char *argv[]) {
	struct sockaddr_un addr;
	int sfd, cfd;
	ssize_t numRead;
	struct payload buf;

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
			process_request(cfd, &buf);

		if (numRead == -1)
			return -errno;

		if (close(cfd) == -1)
			return -errno;
	}
}

static inline int
process_request(int fd, struct payload * buf)
{
	print_payload(buf);
	return dummy_reply(fd, buf);
}

static inline int
dummy_reply(int fd, struct payload * buf)
{
	buf->reply = PAYLOAD_ACCEPTED;

	/* TBD: very basic write, need a while loop */
	if (write(fd, buf, sizeof(*buf)) != sizeof(*buf))
		return -errno;

	return 0;
}
