#ifndef DSR_OBJECT_H
#define DSR_OBJECT_H
#include <dbh.h>

// ****************************************************************************
// Class: DSRObject
//
// Purpose:
//   Wraps the dsr struct and provides some code to read a dsr from a file
//   and make sure that the endian representation works for the current
//   machine.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 24 16:59:37 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class DSRObject
{
public:
    DSRObject();
    virtual ~DSRObject();

    bool PopulateFromFile(const char *filename);
    bool ReversedEndian() const;

    dsr data;
private:
    void ReverseEndians();
    bool reverseEndian;
};

#endif
