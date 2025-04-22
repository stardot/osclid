/* roscli.c
 * Roscli source file
 * (c) Chris Rutter 1997
 * Substantially messed about and unixified - CRJ, 7/12/97
 * Messed about some more to XP-ify it - FDdB, 04-10-2005
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <winsock2.h>


static void appendstr(char **sptr, char *str) {
    int l=strlen(str);
    memcpy(*sptr, str, l);
    *sptr+=l;
}


static void appendchar(char **sptr, char c) {
    *(*sptr)++=c;
}


int main (int argc, char *argv[])
{
    int sock;
    int argp = 1;
    HOSTENT *hostaddr;                     /* WinSock way of doing things...*/
    struct sockaddr_in addr;
    char *arg0 = argv[0];
    char *hostname;
    char command[256], *cend = command;    /* Command buffer and end pointer */
    unsigned int address;


    /* More WinSockness... */
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);


    /* Exit if no useable WinSock DLL was found */
    if (WSAStartup(wVersionRequested, &wsaData) != 0) exit (1);


    /* Exit if the WinSock DLL doesn't support 2.2 */
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        WSACleanup();
        exit (2);
    }


    /* Skip name of executable */
    argv++; argc--;


    /* Check for '-wimp' as first argument */
    if (*argv && strcmp(*argv,"-wimp") == 0) {
        appendstr(&cend, "OscliD_Task ");
        argv++; argc--;
    }


    /* Exit with syntax message if not enough arguments */
    if (argc < 2) {
        printf("Syntax: %s [-wimp] hostname command...\n", arg0);
        WSACleanup();
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
    if (isalpha(hostname[0])) {
        hostaddr = gethostbyname (hostname);
    }
    else {
        address = inet_addr(hostname);
        hostaddr = gethostbyaddr((char*) &address, 4, AF_INET);
    }


    /* Check for WinSock errors */
    if (WSAGetLastError() != 0) {
        if (WSAGetLastError() == 11001) {
            printf("Host '%s' not found\n", hostname);
        }
        else {
            printf("WinSock error #%ld\n", WSAGetLastError());
        }
        WSACleanup();
        exit (4);
    }


    /* Copy stuff */
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons (8805);
    memcpy (&addr.sin_addr, hostaddr->h_addr, hostaddr->h_length);


    /* Report what's happening */
    printf("Sending '%s' to '0x%x'\n", command, addr.sin_addr);


    /* Actually do it */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (char*) &argp, sizeof argp);
    sendto (sock, command, cend-command, 0, (SOCKADDR *) &addr, sizeof (addr));
    closesocket(sock);


    /* Clean up and exit */
    WSACleanup();
    return 0;
}
