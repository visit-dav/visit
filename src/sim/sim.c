#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

int consoleinputdescriptor = 0;
int engineinputdescriptor = 10;

int runflag = 0;
int quitflag = 0;

int cycle = 0;

void ControlCHandler(int sig)
{
    if (runflag)
    {
        fprintf(stderr, "\n   pausing simulation.\n");
    }
    runflag = 0;
    signal(SIGINT, ControlCHandler);
}

void RunSingleCycle()
{
     time_t starttime = time(NULL);
     while (time(NULL) == starttime)
     {
         // Wait
     }

     printf(" ... Finished cycle %d\n", cycle);
     cycle++;
}

void RunSimulation()
{
    runflag = 1;
    while (runflag)
    {
        RunSingleCycle();
    }
}

void ProcessConsoleCommand()
{
    // Read A Command
    char buff[10000];
    int iseof = (fgets(buff, 10000, stdin) == NULL);

    if (buff[strlen(buff)-1] == '\n')
        buff[strlen(buff)-1] = '\0';

    if (iseof)
    {
        sprintf(buff, "quit");
        printf("quit\n");
    }

    if (!strcmp(buff, "quit"))
    {
        quitflag = 1;
    }
    else if (!strcmp(buff, "step"))
    {
        RunSingleCycle();
    }
    else if (!strcmp(buff, "run"))
    {
        RunSimulation();
    }
    else if (!strcmp(buff, "visit"))
    {
        printf("connecting to visit unimplemented so far\n");
    }
    else if (!strcmp(buff, ""))
    {
        // Do nothing on blank input.
    }
    else
    {
        fprintf(stderr, "Error: unknown command '%s'\n", buff);
    }
}

void ProcessEngineCommand()
{
    printf("Received engine command\n");
}

void MainLoop()
{
    signal(SIGINT, ControlCHandler);

    fprintf(stderr, "command> ");
    fflush(stderr);

    while (!quitflag)
    {
        fd_set readSet;

        int maxdescriptor = (consoleinputdescriptor > engineinputdescriptor)
                           ? consoleinputdescriptor : engineinputdescriptor;

        int ret = 0;

        FD_ZERO(&readSet);

        FD_SET(consoleinputdescriptor, &readSet);
        //FD_SET(engineinputdescriptor, &readSet);
        ret = select(maxdescriptor+1, &readSet, (fd_set *)NULL, (fd_set *)NULL,
                     NULL);

        if (ret <= 0)
        {
            if (errno == EINTR)
            {
                fprintf(stderr, "\nInterrupted...... quitting\n");
                quitflag = 1;
            }
            else
            {
                fprintf(stderr, "Error in select at line %d\n",__LINE__);
                exit(1);
            }
        }
        else
        {
            if (FD_ISSET(consoleinputdescriptor, &readSet))
            {
                ProcessConsoleCommand();
                if (!quitflag)
                {
                    fprintf(stderr, "command> ");
                    fflush(stderr);
                }

            }
            else if (FD_ISSET(engineinputdescriptor, &readSet))
            {
                ProcessEngineCommand();
            }
            else
            {
                fprintf(stderr, "Unknown FD set by select at line %d\n",__LINE__);
                exit(1);
            }
        }
    }
}


int main(int argc, char *argv[])
{
    printf("\n          >>> STARTING SIMULATION PROTOTYPE <<<\n\n\n");

    printf("Known Commands:\n"
           "     quit  :        exit code\n"
           "     step  :        run for one cycle\n"
           "     run   :        run continuously\n"
           "     visit :        start visit engine\n\n");

    MainLoop();

    return 0;
}
