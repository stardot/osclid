/* roscli.c
 * Roscli source file
 * (c) Chris Rutter 1997
 * Substantially messed about and unixified - CRJ, 7/12/97
 * Fixed to make GCC 4.4.3 on Ubuntu happy - FDdB, 16-04-2011
 * Fixed to work properly with IP addresses - Phil Pemberton, 2025-04-22
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


static void appendstr(char **sptr, char *str) {
	int l=strlen(str);
	memcpy(*sptr, str, l);
	*sptr+=l;
}


static void appendchar(char **sptr, char c) {
	*(*sptr)++=c;
}


int main (int argc, char *argv[]) {
	int sock;
	int argp = 1;
	struct hostent *hostaddr;
	struct sockaddr_in addr;
	char *arg0 = argv[0];
	char *hostname;
	char command[256], *cend=command;	/* Command buffer and end pointer */
	unsigned int address;
	in_addr_t inet_addr(const char *cp);


	/* Skip name of executable */
	argv++; argc--;


	/* Check for '-wimp' as first argument */
	if (*argv && strcmp(*argv,"-wimp") == 0) {
		appendstr(&cend, "OscliD_Task ");
		argv++; argc--;
	}


	/* Exit with syntax message if not enough arguments */
	if (argc < 2) {
		fprintf(stderr, "Syntax: %s [-wimp] hostname|address command...\n", arg0);
		exit (3);
	}


	/* Get hostname */
	hostname = *argv++; argc--;


	/* Combine all following arguments */
	while (argc) {
		appendstr(&cend, *argv);
		appendchar(&cend, ' ');
		argv++; argc--;
	}
	/* Turn the final space into a terminating null */
	cend[-1]='\0';


	/* Find IP address of recipient */
	hostaddr = gethostbyname (hostname);
	if (hostaddr == NULL) {
		herror("resolve failed");
		fprintf(stderr, "Host '%s' not found\n", hostname);
		exit (4);
	}


	/* Copy stuff */
	memset(&addr, '\0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons (8805);
	memcpy (&addr.sin_addr, hostaddr->h_addr, hostaddr->h_length);


	/* Report what's happening */
	printf("Sending '%s' to '%s'\n", command, hostname);


	/* Do it */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (char*) &argp, sizeof argp);
	sendto (sock, command, cend-command, 0, (struct sockaddr *)&addr, sizeof (addr));

	return 0;
}
