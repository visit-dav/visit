#ifndef TYPEREPRESENTATION_H
#define TYPEREPRESENTATION_H
#include <comm_exports.h>

// *******************************************************************
// Class: TypeRepresentation
//
// Purpose:
//   This class contains information about a machine's data type
//   sizes and endianess.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 12:36:08 PDT 2000
//
// Modifications:
//   
// *******************************************************************

class COMM_API TypeRepresentation
{
public:
    TypeRepresentation();
    TypeRepresentation(const TypeRepresentation &);
    ~TypeRepresentation();
    void operator = (const TypeRepresentation &);
    bool operator == (const TypeRepresentation &);

    int CharSize();
    int IntSize();
    int LongSize();
    int FloatSize();
    int DoubleSize();

    unsigned char IntFormat;
    unsigned char LongFormat;
    unsigned char FloatFormat;
    unsigned char DoubleFormat;
};

#endif
