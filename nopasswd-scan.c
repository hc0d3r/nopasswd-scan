#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <pty.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

#define sudo_pw_prompt "[sudo] password for "

int hexnormalize(int ch)
{
	if (ch >= '0' && ch <= '9')
		ch = ch - '0';
	else if (ch >= 'a' && ch <= 'f')
		ch = ch - 'a' + 10;
	else
		ch = ch - 'A' + 10;

	return ch;
}

char **build_argv(const char *cmd)
{
	char **argv, *buf, c, aux;
	unsigned char hex;
	int len, argc, i, start, j;

	argc = 1;

	argv = malloc(3 * sizeof(char *));
	if (argv == NULL)
		goto end;

	argv[0] = "sudo";

	len = strlen(cmd);
	buf = malloc(len + 1);
	if (buf == NULL)
		goto set_argv_null;

	start = 0;

	for (i = 0, j = 0; i < len; i++, j++) {
		c = cmd[i];

		if (c == '\\') {
			if ((i + 1) < len) {
				aux = cmd[i + 1];

				if (aux == ' ') {
					buf[j] = ' ';
					i++;
				}

				else if (aux == '\\') {
					buf[j] = '\\';
					i++;
				}

				else if ((aux == 'x' || aux == 'X') && (i + 3) < len) {
					if (isxdigit(cmd[i + 2]) && isxdigit(cmd[i + 3])) {
						hex = hexnormalize(cmd[i + 2]) * 16;
						hex += hexnormalize(cmd[i + 3]);
						buf[j] = (char) hex;
						i += 3;
					}
				}

				else {
					buf[j] = '\\';
				}
			}

			else {
				buf[j] = '\\';
			}
		}

		else if (c == ' ') {
			argv = realloc(argv, (argc + 3) * sizeof(char *));
			if (argv == NULL)
				goto end;

			argv[argc++] = buf + start;
			start = j + 1;
			buf[j] = 0x0;
		}

		else {
			buf[j] = c;
		}
	}

	buf[j] = 0x0;
	argv[argc++] = buf + start;

set_argv_null:
	argv[argc] = NULL;

end:
	return argv;
}

int nopasswd(const char *cmd)
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
		execvp("sudo", build_argv(cmd));
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
		printf("usage: nopasswd-scan [CMDLIST]\n");
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
			printf("\r%zd commands checked", total);
		}
	}

	printf("\nfinished... %zd commands checked, ", total);
	printf("%zd possibly don't require a password\n", success);

	return 0;
}
