#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/queue.h> /* non POSIX */
#include "blktap.h"
#include "payload.h"

#define BACKLOG 5

static inline int process_payload(int, struct payload *);
static inline int dummy_reply(int, struct payload *);
static int (*req_reply)(int , struct payload * );
static int handle_request(struct payload * buf);
static int handle_query(struct payload * buf);
static void * process_req(void *);
static int increase_size(off64_t size, const char * path);

pthread_mutex_t req_mutex;
pthread_cond_t req_cond;
SIMPLEQ_HEAD(reqhead, req_entry) req_head;
struct req_entry {
	struct payload data;
	SIMPLEQ_ENTRY(req_entry) entries;
};


int
main(int argc, char *argv[]) {

	SIMPLEQ_INIT(&req_head);
	pthread_t worker;

	struct sockaddr_un addr;
	int sfd, cfd;
	ssize_t numRead;
	struct payload buf;

	/* pthread init section */
	pthread_mutex_init(&req_mutex, NULL);
	pthread_cond_init(&req_cond, NULL);
	if (pthread_create(&worker, NULL, process_req, NULL)) {
		printf("failed worker thread creation\n");
		return 1;
	}

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
	struct req_entry * req;

	if (buf->reply != PAYLOAD_REQUEST)
		return 1;

	printf("I promise I will do something about it..\n");
	req = malloc(sizeof(struct req_entry));
	req->data = *buf;
	buf->reply = PAYLOAD_ACCEPTED;
	pthread_mutex_lock(&req_mutex);
	
	SIMPLEQ_INSERT_TAIL(&req_head, req, entries);

	pthread_cond_signal(&req_cond);
	pthread_mutex_unlock(&req_mutex);

	return 0;
}

static int
handle_query(struct payload * buf)
{
	if (buf->reply != PAYLOAD_QUERY)
		return 1;

	printf("Working on it.. be patient\n");
	buf->reply = PAYLOAD_WAIT;

	return 0;
}

static void *
process_req(void * ap)
{
	struct req_entry * req;
	struct payload * data;

	for(;;) {
		pthread_mutex_lock(&req_mutex);

		while (SIMPLEQ_EMPTY(&req_head)) {
			pthread_cond_wait(&req_cond, &req_mutex);
		}

		req = SIMPLEQ_FIRST(&req_head);
		SIMPLEQ_REMOVE_HEAD(&req_head, entries);
		pthread_mutex_unlock(&req_mutex);
		data = &req->data;
		printf("worker_thread: completed %u %s \n",
		       (unsigned)data->id, data->path);
		free(req);
	}
}

/*
 * @size: current size to increase in bytes
 * @path: device full path
 */
static int
increase_size(off64_t size, const char * path) {
#define NCHARS 16
	pid_t pid;
	int status, num_read;
	char ssize[NCHARS]; /* enough for G bytes */
	size += 104857600; /* add 100 MB */

	/* prepare size for command line */
	num_read = snprintf(ssize, NCHARS, "-L""%"PRIu64"b", size);
	if (num_read >= NCHARS)
		return -1; /* size too big */

	switch (pid = fork()) {
	case -1:
		return -1;
	case 0: /* child */
		execl("/usr/sbin/lvextend", "lvextend", ssize,
		      path, (char *)NULL);
		_exit(127); /* TBD */
	default: /* parent */
		if (waitpid(pid, &status, 0) == -1)
			return -1;
		else if (WIFEXITED(status)) /* normal exit? */
			status = WEXITSTATUS(status);
		else
			return -1;
		return status; /* actually 3 is as fine as 0.. */
	}
}
