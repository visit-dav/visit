#include <sys/time.h>
#include <sys/timeb.h>

int
main(int argc, char *argv[])
{
    struct timeb *tp; ftime(tp);
    return 0;
}
