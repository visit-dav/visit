#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]){
  char myHostName[1000]; 
  char junk[INET_ADDRSTRLEN]; 
  struct hostent *myHEP; 
  char **haddrp = NULL; 
  int addrno=0; 
  if (gethostname(myHostName, 1000) != 0) {
    printf("Cannot get host name\n"); 
    return 1;
  } else {
    printf("Hostname is %s\n", myHostName); 
  }

  myHEP = gethostbyname(myHostName); 
  if (!myHEP) {
    printf("Cannot get host entry from name %s (errno is %d)\n", myHostName, errno); 
    return 2; 
  }

  printf("Result of gethostbyname:  canonical h_name is %s\n", myHEP->h_name); 
  printf("Host addresses associated to name: \n"); 
  haddrp = myHEP->h_addr_list; 
  while (*haddrp) {
    printf("h_addr_list[%d] = %s\n", addrno++, 
           inet_ntop(myHEP->h_addrtype, *(haddrp++), junk, sizeof(junk))); 
  }
  return 0; 
}
