// NOTE - This code incomplete and is for example purposes only.
//        Do not try to compile.

#include <InvalidDBTypeException.h>

avtXXXXFileFormat::avtXXXXFileFormat(const char *filename)
    : avtSTSDFileFormat(filename)
{
    bool fileOpened = false;

    // Open the file specified by the filename argument here using
    // your file format API and set fileOpened accordingly.
    YOU MUST IMPLEMENT THIS

    // If your file format API could not open the file then throw
    // an exception.
    if (!fileOpened)
    {
        EXCEPTION1(InvalidDBTypeException,
           "The file could not be opened");
    }
}
