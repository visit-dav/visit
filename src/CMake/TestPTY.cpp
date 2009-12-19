#include <stdlib.h>
#include <stropts.h>

int
main(int argc, char *arg[])
{
    grantpty(0);
    return 0;
}
