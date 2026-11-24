#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]){
  char myHostName[1000]; 
  char junk[NI_MAXHOST];
  struct addrinfo hints = {0};
  struct addrinfo *hostInfo = NULL;
  struct addrinfo *address = NULL;
  int addrno=0; 
  if (gethostname(myHostName, 1000) != 0) {
    printf("Cannot get host name\n"); 
    return 1;
  } else {
    printf("Hostname is %s\n", myHostName); 
  }

  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_CANONNAME;
  if (getaddrinfo(myHostName, NULL, &hints, &hostInfo) != 0) {
    printf("Cannot get host entry from name %s (errno is %d)\n", myHostName, errno); 
    return 2; 
  }

  printf("Result of getaddrinfo: canonical name is %s\n",
         hostInfo->ai_canonname != NULL ? hostInfo->ai_canonname : myHostName);
  printf("Host addresses associated to name: \n"); 
  for (address = hostInfo; address != NULL; address = address->ai_next) {
    if (getnameinfo(address->ai_addr, address->ai_addrlen, junk, sizeof(junk),
                    NULL, 0, NI_NUMERICHOST) == 0)
      printf("address[%d] = %s\n", addrno++, junk);
  }
  freeaddrinfo(hostInfo);
  return 0; 
}
