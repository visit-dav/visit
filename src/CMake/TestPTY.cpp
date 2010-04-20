#include <stdlib.h>
#include <stropts.h>

int
main(int argc, char *arg[])
{
    grantpt(0);
    return 0;
}
