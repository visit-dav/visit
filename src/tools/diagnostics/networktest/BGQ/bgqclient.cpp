#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/poll.h>

#include <unistd.h>

#if defined(CLIENT_MPI)
#include "mpi.h"
#endif

char file[128];
char host[128];
char port[8];
int sckt;
struct addrinfo *serverAddrInfo;
const char *IPs[] = {
#include "vulcanio.h"
};

/*
 * Prints usage information for the program.
 */
void printUsage() 
{
    printf("client: Usage is\n mpmd_c.elf [option]\n");
    printf("client: --file arg    File holding IP address of the server\n");
    printf("client: --host arg    IP address of the server\n");
    printf("client: --port arg    Port number to use\n");
    printf("client: --help,-h     Display this help information\n");
}

/*
 * Parse the command line parameters to the program.
 */
void parseParameters(int argc, const char ** argv)
{
    // Defaults
    strcpy(port, "51777");

    // Parse the options
    int i;
    for (i = 1; i < argc; ++i)
    {
        const char *flag = argv[i];
        if (strcmp(flag, "--help") == 0 || strcmp(flag, "-h") == 0) {
            printUsage();
            exit(1);
        } else if (strcmp(flag, "--file") == 0) {
            i++;
            strcpy(file, argv[i]);
        } else if (strcmp(flag, "--host") == 0) {
            i++;
            strcpy(host, argv[i]);
        } else if (strcmp(flag, "--port") == 0) {
            i++;
            strcpy(port, argv[i]);
        } else {
           printUsage();
            exit(1);
        }
    }
}

/*
 * Setup the server addrInfo
 */
int setupAddress()
{
    printf("client: Getting address info for host %s port %s\n", host, port);
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int rc = getaddrinfo(host, port, &hints, &serverAddrInfo);
    if (rc != 0) perror("client: getaddrinfo");
    return rc;
}

/*
 * Connect to the server.
 */
int connectToServer()
{
    // Loop through all the results and connect to the first we can
    struct addrinfo *p;
    for(p = serverAddrInfo; p != NULL; p = p->ai_next)
    {
        // Create a socket
        printf("client: Creating socket\n");
        sckt = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (-1 == sckt)
        {
            perror("client: Socket");
            printf("client: Error creating socket\n");
            continue;
        }
        // Connect to the server
        printf("client: Connecting to server\n");
        int rc = connect(sckt, p->ai_addr, p->ai_addrlen);
        if (-1 == rc && EINPROGRESS == errno)
        {
             // Connection is in progress - continue on
        }
        else if (-1 == rc)
        {
            perror("client: Connect");
            printf("client: Error connecting to server\n");
            close(sckt);
            continue;
        }

        // If we get here we are done
        break;
    }

    // Done with the server info
    freeaddrinfo(serverAddrInfo);

    // Make sure we got a valid connection
    return (p == NULL);
}

/*
 * Send the specified data over the socket.
 */
int sendData(const void *buffer, int nbytes)
{
    int nbytesSent = 0;
    int rc = 0;
    // Send the data
    while (nbytesSent < nbytes)
    {
        rc = send(sckt, ((char *)buffer)+nbytesSent, (nbytes - nbytesSent), 0);
        if (-1 == rc)
        {
            if (ECONNRESET == errno)
            {
                printf("client: Sent %d bytes\n", nbytesSent);
                printf("client: Connection was reset by the server\n");
                return nbytesSent;
            }
            else
            {
                printf("client: Error sending data: %s (errno=%d)\n", strerror(errno), errno);
                exit(1);
            }
        }
        nbytesSent += rc;
    }
    printf("client: Sent %d bytes\n", nbytesSent);
    return nbytesSent;
}

/*
* main */
int main(int argc, char *argv[])
{
    int ip;
#if defined(CLIENT_MPI)
    int   rank, size;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    errno);
    if (rank == 0)
    {
#endif

    // Parse parameters
    parseParameters(argc, (const char **)argv);

    // Three ways to get IP address of server:
    // First: read it from a file, if --file <file> was used
    if (strlen(file) > 0)
    {
        FILE      *fp;
        char      *bptr[1], *p;
        size_t    n;
        while ((fp = fopen(file,"r")) == NULL);
        bptr[0] = NULL;
        if ( getline((char **) &bptr,&n,fp) > 0) {
            p = strchr(bptr[0],'\n');
            if (p != NULL) *p = '\0';
            strcpy(host,bptr[0]);
            // fall through to second method
            free(bptr[0]);
        } else {
            perror("client: getline host");
            exit(1);
        }
        bptr[0] = NULL;
        if ( getline((char **) &bptr,&n,fp) > 0) {
            p = strchr(bptr[0],'\n');
            if (p != NULL) *p = '\0';
            strcpy(port,bptr[0]);
            // fall through to second method with this port number
            free(bptr[0]);
        } else {
            perror("client: getline port");
            exit(1);
        }
        fclose(fp);
    }
// Second: Get it from --host argument
    if (strlen(host) > 0)
    {
        int rc = setupAddress();
        if (rc != 0) {
            printf("client: Error constructing server address\n");
            exit(1);
        }
        if (connectToServer()) {
            printf("client: Unable to connect to server\n");
            exit(1);
        }
    }
// Third: If no other methods provided, use brute force
//        search of all IO nodes. If an IO node is down,
//        this will have to suffer through a timeout of
//        a few seconds for each down IO node.
    else
    {
        // Connect to the server
        for (ip=0; ip<sizeof(IPs)/sizeof(char *); ip++)
        {
            strcpy(host,IPs[ip]);
            if (setupAddress()) continue;
            if (connectToServer() == 0) break;
        }
        // Make sure we got a valid connection
        if (ip == sizeof(IPs)/sizeof(char *))
        {
            printf("client: Unable to connect to server\n");
            exit(1);
        }
    }

    if (strlen(file) > 0) printf("client: File: %s\n", file);
    printf("client: Host: %s\n", host);
    printf("client: Port: %s\n", port);

    // Poll the socket
    struct pollfd ufds[1];
    ufds[0].fd = sckt;
    ufds[0].events = POLLOUT;
    ufds[0].revents = 0;
    int i;
    for (i = 0; i < 10; i++)
    {
        printf("client: Polling for ready to send\n");
        int rc = 0;
        while (0 == rc)
        {
            ufds[0].revents = 0;
            rc = poll(ufds, 1, 100);
            if (-1 == rc)
            {
                perror("client: poll");
                printf("client: Error polling socket\n");
                exit(1);
            }
            else if (0 == rc)
            {
                /* BGQ is hardcoded to use a 100ms timeout for poll so we 
                 * need to loop on this to wait longer
                 */
                continue;
            }
            else
            {
                if (ufds[0].revents & POLLOUT)
                {
                    // Ready to send data
                    char buffer[64];
                    if (i >= 9) {
                        sprintf(buffer, "Exit");
                    }
                    else {
                        sprintf(buffer, "Message %d", i);
                    }
                    sendData(buffer, sizeof(buffer));
                }
                break;
            }
        }
        sleep(1);
    }
    // Close the socket
    printf("client: Closing socket\n");
    close(sckt);

#if defined(CLIENT_MPI)
}
MPI_Finalize();
#endif

   return 0;
}
