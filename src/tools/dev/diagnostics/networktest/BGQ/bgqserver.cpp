#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <unistd.h>

#if defined(SERVER_MPI)
#include "mpi.h"
#endif

struct addrinfo *serverAddrInfo;
int sckt;
int connection;
char file[128];
char port[8];

/*
 * Prints usage information for the program.
 */
void printUsage()
{
    printf("server: Usage is\n server.elf [option]\n");
    printf("server: --file arg  File to write server IP address/port\n");
    printf("server: --port arg  Port number to use\n");
    printf("server: --help,h    Display this help information\n");
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

    for (i = 1; i < argc; ++i) {
        const char *flag = argv[i];

        if (strcmp(flag, "--help") == 0 || strcmp(flag, "-h") == 0) {
            printUsage();
            exit(1);

        } else if (strcmp(flag, "--file") == 0) {
            i++;
            strcpy(file, argv[i]);

        } else if (strcmp(flag, "--port") == 0) {
            i++;
            strcpy(port, argv[i]);

        } else {
            printUsage();
            exit(1);
        }
    }

    // Print out the options
    if (strlen(file) > 0) {
        printf("server: File: %s\n", file);
    }

    printf("server: Port: %s\n", port);
}
/*
 * Setup the server addrInfo.
 */
void setupAddress()
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    int rc = getaddrinfo(NULL, port, &hints, &serverAddrInfo);

    if (rc != 0) {
        perror("server: getaddrinfo");
        printf("server: Error getting address info for server\n");
        exit(1);
    }
}
/*
 * Create and bind the socket.
 */
void createSocket()
{
    /* Create and bind the socket */
    struct addrinfo *p;

    for (p = serverAddrInfo; p != NULL; p = p->ai_next)
    {
        printf("server: Creating socket\n");
        sckt = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (-1 == sckt) {
            perror("server: socket");
            printf("server: Error creating socket\n");
            continue;
        }

        /* Prevent "Address already in use" error message when re-running the test */
        int optVal = 1;

        if (setsockopt(sckt, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(int)) == -1) {
            perror("server: setsockopt");
            printf("server: Error setting SO_REUSEADDR socket option\n");
            continue;
        }

        /* Bind the socket */
        printf("server: Binding socket\n");
        int rc = bind(sckt, p->ai_addr, p->ai_addrlen);

        if (-1 == rc) {
            perror("server: Bind");
            printf("server: Error binding socket\n");
            continue;
        }

        /* If we get here, the socket is ready */
        break;
    }

    /* Make sure we were able to bind the socket */
    if (p == NULL)
    {
        printf("server: Error creating and binding the socket\n");
        exit(1);
    }

    /* Done with the server info */
    freeaddrinfo(serverAddrInfo);
}

/*
 * Listen on the socket and accept a connection.
 */
void waitForConnection()
{
    // Start listening on the socket
    printf("server: Listen on the socket\n");
    int rc = listen(sckt, 1);

    if (-1 == rc) {
        perror("server: Listen");
        printf("server: Error listening on socket\n");
        exit(1);
    }

    // Poll to wait for a connection on a listening socket
    printf("server: Polling for a connection\n");
    rc = 0;

    while (0 == rc)
    {
        struct pollfd ufds[1];
        ufds[0].fd = sckt;
        ufds[0].events = POLLIN;
        ufds[0].revents = 0;
        rc = poll(ufds, 1, 100);

        if (-1 == rc)
        {
            perror("server: poll");
            printf("server: Error polling socket for accept\n");
            exit(1);

        }
        else if (0 == rc)
        {
            /* BGQ is hardcoded to use a 100ms timeout for poll so we 
             * need to loop on this to wait longer */
            continue;

        }
        else
        {
            if (ufds[0].revents & POLLIN)
            {
                /* Client is ready to connect */
                struct sockaddr_storage client_addr;
                socklen_t addr_size = sizeof(client_addr);

                connection = -1;
                ufds[0].revents = 0;
                while (-1 == connection)
                {
                    connection = accept(sckt, (struct sockaddr *)&client_addr,
                                        &addr_size);
                    if (-1 == connection && errno != EAGAIN && errno != EINTR)
                    {
                        printf("server: Error accepting connection: %s (errno=%d)\n",
                               strerror(errno), errno);
                        exit(1);
                        return;
                    }
                    sleep(1);
                }
                printf("server: Accepted connection %d\n", connection);
                break;
            }
        }
    }
}

/*
 * Receive a message on the socket.
 */
int receiveData()
{
    char buffer[64];
    int bufferSize = 64;
    int rc;

    do
    {
        rc = recv(connection, buffer, bufferSize, 0);

        if (rc == -1)
        {
            if (errno == EINTR)
            {
                // The SIGALRM signal for bitrate sampling sometimes interrupts the
                printf("server: Error receiving data: %s (errno=%d)\n",
                       strerror(errno), errno);

            }
            else
            {
                printf("server: Error receiving data: %s (errno=%d)\n",
                       strerror(errno), errno);
            }
        }
    } while(rc == -1 && (errno == EAGAIN || errno == EINTR));

    printf("server: Received data: %s\n", buffer);
    return (strcmp(buffer,"Exit") == 0);
}

#define NAMELEN (32)

/*
 * main 
 */
int
main(int argc, char *argv[])
{
    char            host[NAMELEN];
    struct hostent *server;
    int             j;
#if defined(SERVER_MPI)
    int             rank, size;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
#endif
        // Parse parameters
        parseParameters(argc, (const char**)argv);

        if (gethostname(host,NAMELEN) == 0)
        {
            server = gethostbyname(host);

            if (server != NULL)
            {
                if (inet_ntop(AF_INET,server->h_addr,host,NAMELEN) != NULL)
                {
                    printf("server: Host: %s\n", host);
                }
            }
        }

        if (strlen(file) > 0)
        {
            FILE      *fp;
            int       n;

            if ((fp = fopen(file,"w")) == NULL)
            {
                perror("server: fopen");
                printf("server: Unable to write IP address to %s\n", file);
            }
            else
            {
                if ((n = fprintf(fp, "%s\n", host)) <= strlen(host))
                {
                    if (n < 0) perror("server: fprintf");

                    printf("server: Error writing IP address to file %s\n", file);
                }

                if ((n = fprintf(fp, "%s\n", port)) <= strlen(port))
                {
                    if (n < 0) perror("server: fprintf");

                    printf("server: Error writing port to file %s\n", file);
                }

                fflush(fp);
                fclose(fp);
            }
        }

        /* Open a socket and wait for a connection */
        setupAddress();
        createSocket();
        waitForConnection();

        /* Poll the socket */
        struct pollfd ufds[1];
        ufds[0].fd = connection;
        ufds[0].events = POLLIN;
        ufds[0].revents = 0;
        int i = 0;
        int done = 0;

        for (i = 0; !done; i++)
        {
            printf("server: Polling for ready to receive\n");
            int rc = 0;

            while (0 == rc)
            {
                ufds[0].revents = 0;
                rc = poll(ufds, 1, 100);

                if (-1 == rc)
                {
                    perror("server: poll");
                    printf("server: Error polling socket\n");
                    exit(1);

                }
                else if (0 == rc)
                {
                    /* BGQ is hardcoded to use a 100ms timeout for poll so 
                     * we need to loop on this to wait longer*/
                    continue;

                }
                else
                {
                    if (ufds[0].revents & POLLIN)
                    {

                        /* Ready to receive data */
                        if (receiveData()) done = 1;
                    }
                    break;
                }
            }
        }

        // Close the connection and the socket
        printf("server: Closing socket\n");
        close(connection);
        close(sckt);

        if (strlen(file) > 0)
        {
            if (unlink(file) < 0)
            {
                perror("server: unlink");
                printf("server: Unable to remove %s\n", file);
            }
        }

#if defined(SERVER_MPI)
    }

    MPI_Finalize();
#endif
    return 0;
}

