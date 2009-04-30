/* $Id: signals.c,v 1.5 2005/09/14 02:30:56 rcook Exp $ */
#include <signal.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
/* set a signal handler for the given signal 
   sig == the signal to catch
   func == the handler

   useful functions are SIG_IGN, SIG_DFL
   can of course pass a pointer to your own sigfunc 
   e.g., 
   void myfunc(int sig);
   if (!setsignal(SIGHUP, myfunc))
     errexit("Can't set SIGHUP handler!\n");

  q.v. Stephenson's Advance Unix Programming, page 270
*/
typedef void sigfunc (int);
int setsignal(int sig, sigfunc *func){
  fprintf(stderr, "setting signal %d\n", sig); 
  if (signal(sig, func) == SIG_ERR) {
    fprintf(stderr, "Error: couldn't set signal %d\n", sig);
    return 0;
  }
  return 1;
}

void handler(int sig) {
  printf("test prog received signal %d\n", sig);
  /*exit(0);*/
}


/* this is code I used to see if I was getting signaled at one point, and it might be useful*/
int GettingSignaled(void)
{
  char localhost[256]; 
  struct hostent *hep = NULL;
  int i=0;
  /* see if we're getting signaled, damnit!*/
  setsignal(SIGINT, handler);
  /*setsignal(SIGKILL, handler);*/
  setsignal(SIGHUP, SIG_IGN);
  setsignal(SIGPIPE, handler);
  setsignal(SIGQUIT, handler);
  /*setsignal(SIGSTOP, handler);*/
  setsignal(SIGTERM, handler);
  setsignal(SIGTTIN, handler);
  setsignal(SIGABRT, handler);
  setsignal(SIGCHLD, handler);

  gethostname(localhost, 256);
  hep = gethostbyname(localhost);
  printf("test host is %s\n", hep->h_name);
  fflush(stdout);

  /*  switch (fork())
      {
      case -1: error
      {
      printf("failed to fork\n");
      exit (1);
      }
      case 0: child
      {
  */
  fprintf(stdout, "Child stdout.\n");
  fprintf(stderr, "Child stderr new.\n");    
  /*fclose(stdout);*/
  /*fclose(stderr);*/
  i=30;
  while (i--){
    int err = usleep (999999);
    if (err) {
      fprintf(stderr, "child sleep err %d\n", err);
      exit(1);
    }
    printf("Child sleeping... (%d) \n", i);
    fflush(stdout);
  }
  /*
    }
    default:
    {
    fprintf(stdout, "parent stdout.");
    fprintf(stderr, "parent stderr x.");
    fclose(stdout);/
    fclose(stderr);/
    return 0;
    }
    }
  */
  return 0;
}
