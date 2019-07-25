// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                XDMFParser.h                               //
// ************************************************************************* //

#ifndef XDMF_PARSER_H
#define XDMF_PARSER_H

#include <stdio.h>

// ****************************************************************************
//  Class: avtXDMFFileFormat
//
//  Purpose:
//      Reads in XDMF files as a plugin to VisIt.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 11:28:35 PDT 2007
//
//  Modifications:
//    Eric Brugger, Fri Apr 24 08:18:22 PDT 2009
//    I added the method GetAttributeValueAsUpper.
//
// ****************************************************************************

class XDMFParser
{
  public:
                       XDMFParser();
    virtual           ~XDMFParser();

    enum               ElementType {TYPE_PI, TYPE_COMMENT, TYPE_SPECIAL,
                                    TYPE_START_TAG, TYPE_END_TAG, TYPE_CDATA,
                                    TYPE_EOF, TYPE_ERROR};

    void               SetInputFileName(const char *);

    ElementType        GetNextElement();
    const char        *GetElementName() const;
    void               SkipToEndTag();

    bool               GetNextAttribute();
    const char        *GetAttributeName() const;
    const char        *GetAttributeValue() const;
    const char        *GetAttributeValueAsUpper();

    int                GetCDataOffset() const;
    int                GetCDataLength() const;
    const char        *GetCDataValue() const;

    void               PrintFile();

  private:
    void               UnGetChar();
    void               UnGetChar(const char &);
    char              *GetChar();

    char              *fname;

    FILE              *file;

    char               nullChar[8];

    int                offset;
    int                ibuf;
    int                lbuf;
    bool               endOfFile;
    char              *buffer;

    // Return value for GetElementName.
    char               elementName[1024];

    // Return values for GetAttributeName and GetAttributeValue.
    char               attributeName[1024];
    char               attributeValue[1024];

    // Return values for GetCDataOffset, GetCDataLength, and GetCDataValue.
    int                cdataOffset;
    int                cdataLength;
    char               cdataValue[1024];
};

#endif
