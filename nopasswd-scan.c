#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <pty.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#define sudo_pw_prompt "[sudo] password for "

int nopasswd(const char *file)
{
	struct pollfd pfd;
	char buf[1024];

	int master, success = 0;
	pid_t pid;
	ssize_t n;

	pid = forkpty(&master, NULL, NULL, NULL);
	if (pid == -1) {
		perror("fork() failed");
		_exit(1);
	}

	else if (pid == 0) {
		execlp("sudo", "sudo", file, NULL);
		perror("execlp() failed");
		_exit(0);
	}

	pfd.events = POLLIN;
	pfd.fd = master;

	// wait the sudo password prompt for n miliseconds
	switch (poll(&pfd, 1, 5000)) {
		case 1:
			n = read(master, buf, sizeof(buf));
			if (n == -1) {
				perror("read() failed");
			} else {
				if ((size_t)n > sizeof(sudo_pw_prompt) - 1)
					n = sizeof(sudo_pw_prompt) - 1;

				success = memcmp(buf, sudo_pw_prompt, n);
			}
			break;
		case 0: // timeout
			success = 1;
			break;
		default:
			perror("poll()");
	}

	close(master);
	kill(pid, SIGKILL);

	return success;
}

int main(int argc, char **argv)
{
	FILE *fh;
	char *line = NULL;
	size_t len, total, success;
	ssize_t n;

	if (argc != 2) {
		printf("--sudo nopasswd-scan--\n");
		printf("usage: nopasswd-scan [FILE]\n");
		return 1;
	}

	fh = fopen(argv[1], "r");
	if (fh == NULL) {
		perror("fopen()");
		return 1;
	}

	// unbuffered stdout
	setvbuf(stdout, NULL, _IONBF, 0);

	// prevents sudo prompt be translated
	setenv("LANG", "C", 1);

	line = NULL;
	len = success = total = 0;

	while ((n = getline(&line, &len, fh)) > 0) {
		if (line[n - 1] == '\n')
			line[n - 1] = 0x0;

		total++;

		if (nopasswd(line)) {
			printf("\n--> %s\n", line);
			success++;
		} else {
			printf("\r%zd files checked", total);
		}
	}

	printf("\nfinished... %zd files checked, ", total);
	printf("%zd possibly does not require password\n", success);

	return 0;
}
