#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include "payload.h"

static void usage(char *);

int
main(int argc, char *argv[]) {
	struct payload message;
	int arg;
	int opt_idx = 0, flag = 1;
	int ret;

	const struct option longopts[] = {
		{ "add", required_argument, NULL, 0 },
		{ "del", required_argument, NULL, 0 },
		{ 0, 0, 0, 0 }
	};

	init_payload(&message);
	message.reply = PAYLOAD_CLI;

	/* We expect at least one valid option and, if more, the others
	   are discarded
	*/
	while ((arg = getopt_long(argc, argv, "h",
				  longopts, &opt_idx)) != -1 && flag) {
		switch(arg) {
		case 0:
			ret = snprintf(message.path, PAYLOAD_MAX_PATH_LENGTH,
				       "%s %s", longopts[opt_idx].name, optarg);
			if (ret >= PAYLOAD_MAX_PATH_LENGTH) {
				fprintf(stderr, "input too long\n");
				return 2;
			}
			flag = 0;
			break;
		case 'h':
			usage(argv[0]);
			return 0;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	/* there must be at least one valid option */
	if(flag) {
		usage(argv[0]);
		return 1;
	}

	ret = thin_sock_comm(&message);
	printf("message: %s\n", message.path);

	return 0;
}

static void
usage(char *prog_name)
{
	printf("usage: %s --add <volume group name>\n", prog_name);
	printf("usage: %s --del <volume group name>\n", prog_name);
}
