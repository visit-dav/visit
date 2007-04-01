#include <dbfFile.h>
    
int
main(int argc, char *argv[])
{
    dbfFile_t      *f = 0;
    dbfFileError_t  fileError;
    dbfReadError_t  rcError;
    void           *data = 0;

    dbfInitialize(0, 0);

#if 1
#define DATABASE "m195mbsp.dbf"
#define VARIABLE "EPA_NO"
#else
#define DATABASE "GebcoDepthContours.dbf"
#define VARIABLE "DEPTH"
#endif

    f = dbfFileOpen(DATABASE, &fileError);

    if(f != 0)
    {
        /* Try and read some data */
        data = dbfFileReadField(f, VARIABLE, &rcError);
        if(rcError == dbfReadErrorSuccess)
        {
            printf("We were able to read the %s field.\n", VARIABLE);
            dbfFree(data);
        }
        else if(rcError == dbfReadErrorFailure)
            printf("We were NOT able to read the %s field!\n", VARIABLE);
        else
            printf("%s is not a valid field name.\n", VARIABLE);
    }

    dbfFileClose(f);

    dbfFinalize();

    return 0;
}
