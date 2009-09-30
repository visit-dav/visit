/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                                XDMFParser.C                               //
// ************************************************************************* //

#include <XDMFParser.h>

#include <DebugStream.h>

#include <ctype.h>
#include <stdio.h>

#include <cstring>

#if 0
#define READ_SIZE 65536
#else
#define READ_SIZE 128
#endif
#define BUFFER_SIZE READ_SIZE+24

#define OPEN (*chr == '<')
#define CLOSE (*chr == '>')
#define EQUAL (*chr == '=')
#define QUESTION (*chr == '?')
#define SPACE isspace(*chr)
#define EMPTY_CLOSE (*chr == '/' && *(chr+1) == '>')
#define NAME_START (isalpha(*chr) || *chr == '_' || *chr == ':')
#define NAME_CHAR (isalnum(*chr) || *chr == '.' || *chr == '-' || *chr == '_' || *chr == ':')
#define END_OF_FILE (chr == &(nullChar[0]))

// ****************************************************************************
//  Method: XDMFParser constructor
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 10:32:54 PDT 2007
//
// ****************************************************************************

XDMFParser::XDMFParser()
{
    fname = NULL;
    file = NULL;

    nullChar[0] = '\0'; nullChar[1] = '\0'; nullChar[2] = '\0';
    nullChar[3] = '\0'; nullChar[4] = '\0'; nullChar[5] = '\0';
    nullChar[6] = '\0'; nullChar[7] = '\0';

    offset = 0;
    ibuf = 0;
    lbuf = 0;
    endOfFile = true;
    buffer = new char[BUFFER_SIZE];
}


// ****************************************************************************
//  Method: XDMFParser destructor
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 10:32:54 PDT 2007
//
// ****************************************************************************

XDMFParser::~XDMFParser()
{
    delete [] fname;
    delete [] buffer;

    if (file != NULL)
        fclose(file);
}


// ****************************************************************************
//  Method: XDMFParser::SetInputFileName
//
//  Purpose:
//      Set the name of the file to parse.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 10:32:54 PDT 2007
//
//  Modifications:
//    Eric Brugger, Tue Sep 29 12:38:08 PDT 2009
//    Added initialization of offset, ibuf and lbuf.
//
// ****************************************************************************

void
XDMFParser::SetInputFileName(const char *filename)
{
    fname = new char[strlen(filename)+1];
    strcpy(fname, filename);

    file = fopen(filename, "r");
 
    offset = 0;
    ibuf = 0;
    lbuf = 0;
    endOfFile = false;
}


// ****************************************************************************
//  Method: XDMFParser::GetNextElement
//
//  Purpose:
//      Parse the next element in the file, returning its type.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 10:32:54 PDT 2007
//
//  Modifications:
//    Eric Brugger, Fri Apr 24 08:20:39 PDT 2009
//    I modified the routine to store the elementName in all upper case
//    letters.
//
// ****************************************************************************

XDMFParser::ElementType
XDMFParser::GetNextElement()
{
    //
    // Skip over white space.
    //
    char *chr = GetChar();
    while (SPACE) chr = GetChar();
    if (END_OF_FILE)
        return TYPE_EOF;

    if (OPEN)
    {
        chr = GetChar();
        if (QUESTION)
        {
            //
            // Processing Instructions
            //
            chr = GetChar();
            while (!END_OF_FILE && (*chr != '?' || *(chr+1) != '>'))
                chr = GetChar();
            chr = GetChar(); chr = GetChar();

            return TYPE_PI;
        }
        else if (*chr == '!')
        {
            chr = GetChar();
            if (*(chr) == '-' && *(chr+1) == '-')
            {
                //
                // Comment
                //
                chr = GetChar(); chr = GetChar();
                while (!END_OF_FILE &&
                       (*chr != '-' || *(chr+1) != '-' || *(chr+2) != '>'))
                    chr = GetChar();
                chr = GetChar(); chr = GetChar(); chr = GetChar();

                return TYPE_COMMENT;
            }
            else if (*(chr)   == '[' && *(chr+1) == 'C' && *(chr+2) == 'D' &&
                     *(chr+3) == 'A' && *(chr+4) == 'T' && *(chr+5) == 'A' &&
                     *(chr+6) == '[')
            {
                //
                // We have CData.
                //
                chr = GetChar(); chr = GetChar(); chr = GetChar();
                chr = GetChar(); chr = GetChar(); chr = GetChar();
                chr = GetChar();

                //
                // Skip any leading whitespace.
                //
                while (SPACE) chr = GetChar();

                cdataOffset = offset + ibuf;
                int iCdataValue = 0;
                while (!END_OF_FILE &&
                       (*chr != ']' || *(chr+1) != ']' || *(chr+2) != '>'))
                {
                    chr = GetChar();
                    if (iCdataValue < 1023)
                    {
                        cdataValue[iCdataValue++] = *chr;
                    }
                }
                cdataLength = offset + ibuf - cdataOffset;
                cdataValue[iCdataValue] = '\0';
                chr = GetChar(); chr = GetChar(); chr = GetChar();

                return TYPE_CDATA;
            }
            else
            {
                //
                // Special elements that start with "<!", such as ATTLIST,
                // DOCTYPE, and ELEMENT.
                //
                while (!END_OF_FILE && *chr != '>')
                    chr = GetChar();
                chr = GetChar();

                return TYPE_SPECIAL;
            }
        }
        else if (*chr == '/')
        {
            //
            // We have an end tag.
            //
            chr = GetChar();

            //
            // If we have a close character, then we have an empty close
            // tag. This may not be legal, but is required for the parser
            // to work properly, since these are generated internally when
            // an open tag is terminated with a "/>".
            //
            if (CLOSE)
            {
                elementName[0] = '\0';
                return TYPE_END_TAG;
            }

            //
            // If it doesn't start with a letter, underscore or colon it
            // is an error, so skip over the tag looking for the next open
            // character.
            //
            if (!NAME_START)
            {
                while (!END_OF_FILE && !OPEN)
                    chr = GetChar();
                return TYPE_ERROR;
            }

            //
            // Process the name.
            //
            char *cptr = &elementName[0];
            *cptr++ = toupper(*chr);
            chr = GetChar();

            while (NAME_CHAR)
            {
                *cptr++ = toupper(*chr);
                chr = GetChar();
            }
            *cptr = '\0';

            //
            // Skip any trailing whitespace after the name.
            //
            while (SPACE) chr = GetChar();

            //
            // If the first non-white space character isn't a closing
            // character, it is an error.
            //
            if (!CLOSE)
            {
                while (!END_OF_FILE && !OPEN)
                    chr = GetChar();
                return TYPE_ERROR;
            }

            //
            // Skip the closing character.
            //
            chr = GetChar();

            return TYPE_END_TAG;
        }
        else
        {
            //
            // We have a start tag.
            //

            //
            // If it doesn't start with a letter, underscore or colon it
            // is an error, so skip over the tag looking for the next open
            // character.
            //
            if (!NAME_START)
            {
                while (!END_OF_FILE && !OPEN)
                    chr = GetChar();
                return TYPE_ERROR;
            }

            //
            // Process the name.
            //
            char *cptr = &elementName[0];
            *cptr++ = toupper(*chr);
            chr = GetChar();

            while (NAME_CHAR)
            {
                *cptr++ = toupper(*chr);
                chr = GetChar();
            }
            *cptr = '\0';
            UnGetChar();

            return TYPE_START_TAG;
        }
    }
    else
    {
        //
        // We have CData.
        //

        //
        // Skip any leading whitespace.
        //
        while (SPACE) chr = GetChar();
        if (END_OF_FILE)
            return TYPE_EOF;

        cdataOffset = offset + ibuf - 1;
        int iCdataValue = 0;
        while (!END_OF_FILE && !OPEN)
        {
            if (iCdataValue < 1023)
            {
                cdataValue[iCdataValue++] = *chr;
            }
            chr = GetChar();
        }
        cdataLength = offset + ibuf - cdataOffset - 1;
        cdataValue[iCdataValue] = '\0';
        UnGetChar();

        return TYPE_CDATA;
    }
}


// ****************************************************************************
//  Method: XDMFParser::SkipToEndTag
//
//  Purpose:
//      Skip to the next end tag. This routine assumes that we are still
//      within the start tag.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 10:32:54 PDT 2007
//
// ****************************************************************************

void
XDMFParser::SkipToEndTag()
{
    ElementType elementType;

    //
    // Read the rest of the start tag and get the next element.
    //
    while (GetNextAttribute()) /* do nothing */;

    elementType = GetNextElement();

    //
    // Keep reading tags until we get a zero nesting level.
    //
    int nestingLevel = 1;
    while (elementType != TYPE_EOF)
    {
        switch (elementType)
        {
          case TYPE_START_TAG:
            nestingLevel++;
            //
            // Read the rest of the start tag.
            //
            while (GetNextAttribute()) /* do nothing */;

            break;

          case TYPE_END_TAG:
            nestingLevel--;
            break;

          default:
            break;
        }
        if (nestingLevel == 0)
            break;

        elementType = GetNextElement();
    }
}


// ****************************************************************************
//  Method: XDMFParser::GetElementName
//
//  Purpose:
//      Return the name of the current element.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 10:32:54 PDT 2007
//
// ****************************************************************************

const char *
XDMFParser::GetElementName() const
{
    return elementName;
}


// ****************************************************************************
//  Method: XDMFParser::GetNextAttribute
//
//  Purpose:
//      Parse the next attribute in the current start tag. Returns true if
//      another attribute was found, false otherwise.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 10:32:54 PDT 2007
//
//  Modifications:
//    Eric Brugger, Fri Apr 24 08:20:39 PDT 2009
//    I modified the routine to store the attributeName in all upper case
//    letters.
//
// ****************************************************************************

bool
XDMFParser::GetNextAttribute()
{
    char *chr = GetChar();
    while (SPACE) chr = GetChar();
    if (END_OF_FILE)
        return false;

    bool notFound = true;
    while (notFound)
    {
        //
        // If we hit a closing symbol, then return.
        //
        if (CLOSE)
        {
            return false;
        }
        else if (EMPTY_CLOSE)
        {
            UnGetChar();
            UnGetChar('<');
            return false;
        }
    
        //
        // If the name doesn't start with a letter, underscore or colon,
        // it is an error, so skip over the remaining text until reaching
        // a closing symbol or some white space.
        //
        if (!NAME_START)
        {
            while (!END_OF_FILE && !SPACE && !CLOSE && !EMPTY_CLOSE)
                chr = GetChar();
            if (CLOSE)
            {
                return false;
            }
            else if (EMPTY_CLOSE)
            {
                UnGetChar('<');
                return false;
            }
            break;
        }

        //
        // Process the attribute name.
        //
        char *cptr = &attributeName[0];
        *cptr++ = toupper(*chr);
        chr = GetChar();

        while (NAME_CHAR)
        {
            *cptr++ = toupper(*chr);
            chr = GetChar();
        }
        *cptr = '\0';

        //
        // Skip over white space.
        //
        while (SPACE) chr = GetChar();

        //
        // If there isn't an equal sign, it is an error, so skip over the
        // remaining text until reaching a closing symbol or some white
        // space.
        //
        if (!EQUAL)
        {
            while (!END_OF_FILE && !SPACE && !CLOSE && !EMPTY_CLOSE)
                chr = GetChar();
            if (CLOSE)
            {
                return false;
            }
            else if (EMPTY_CLOSE)
            {
                UnGetChar('<');
                return false;
            }
            break;
        }

        //
        // Skip the equal sign.
        //
        chr = GetChar();

        //
        // Skip over white space.
        //
        while (SPACE) chr = GetChar();

        //
        // If there isn't a single or double quote, it is an error, so skip
        // over the remaining text until reaching a closing symbol or some
        // white space.
        //
        if (*chr != '"' && *chr != '\'')
        {
            while (!END_OF_FILE && !SPACE && !CLOSE && !EMPTY_CLOSE)
                chr = GetChar();
            if (CLOSE)
            {
                return false;
            }
            else if (EMPTY_CLOSE)
            {
                UnGetChar('<');
                return false;
            }
            break;
        }
        char delimeter = *chr;
   
        //
        // Skip the delimeter.
        //
        chr = GetChar();

        //
        // Process the attribute value.
        //
        cptr = &attributeValue[0];

        while (!END_OF_FILE && *chr != delimeter && !OPEN)
        {
            *cptr++ = *chr;
            chr = GetChar();
        }
        *cptr = '\0';

        //
        // If the closing delimeter is missing, it is an error, so return.
        //
        if (OPEN)
        {
            UnGetChar();
            return false;
        }

        notFound = false;
    }

    return true;
}


// ****************************************************************************
//  Method: XDMFParser::GetAttributeName
//
//  Purpose:
//      Return the name of the current attribute.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 10:32:54 PDT 2007
//
// ****************************************************************************

const char *
XDMFParser::GetAttributeName() const
{
    return attributeName;
}


// ****************************************************************************
//  Method: XDMFParser::GetAttributeValue
//
//  Purpose:
//      Return the value of the current attribute.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 10:32:54 PDT 2007
//
// ****************************************************************************

const char *
XDMFParser::GetAttributeValue() const
{
    return attributeValue;
}


// ****************************************************************************
//  Method: XDMFParser::GetAttributeValueAsUpper
//
//  Purpose:
//      Return the value of the current attribute in all upper case letters.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Apr 24 08:20:39 PDT 2009
//
// ****************************************************************************

const char *
XDMFParser::GetAttributeValueAsUpper()
{
    for (int i = 0; attributeValue[i] != 0; i++)
        attributeValue[i] = toupper(attributeValue[i]);
    return attributeValue;
}


// ****************************************************************************
//  Method: XDMFParser::GetCDataOffset
//
//  Purpose:
//      Return the file offset of the current CData.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 10:32:54 PDT 2007
//
// ****************************************************************************

int
XDMFParser::GetCDataOffset() const
{
    return cdataOffset;
}


// ****************************************************************************
//  Method: XDMFParser::GetCDataLength
//
//  Purpose:
//      Return the length of the current CData.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 10:32:54 PDT 2007
//
// ****************************************************************************

int
XDMFParser::GetCDataLength() const
{
    return cdataLength;
}


// ****************************************************************************
//  Method: XDMFParser::GetCDataValue
//
//  Purpose:
//      Return the value of the current CData.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 10:32:54 PDT 2007
//
// ****************************************************************************

const char *
XDMFParser::GetCDataValue() const
{
    return cdataValue;
}


// ****************************************************************************
//  Method: XDMFParser::PrintFile
//
//  Purpose:
//      Print the current file. This is primarily used for debugging.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 10:32:54 PDT 2007
//
// ****************************************************************************

void
XDMFParser::PrintFile()
{
    int j;
    int indent = 0;

    //
    // Process the next 100000 elements. This is a precaution against
    // getting into an infinite loop. It could just as well be a while
    // true loop.
    //
    for (int i = 0; i < 100000; i++)
    {
        //
        // We are getting the element type at the beginning of the
        // loop so that we can handle the END_TAG now to get the
        // indentation correct.
        //
        ElementType elementType = GetNextElement();
        if (elementType == TYPE_END_TAG) indent--;

        for (int k = 0; k < indent ; k++)
            cerr << " ";

        switch (elementType)
        {
          case TYPE_PI:
            cerr << "Processing Information." << endl;
            break;
          case TYPE_COMMENT:
            cerr << "Comment." << endl;
            break;
          case TYPE_SPECIAL:
            cerr << "Special." << endl;
            break;
          case TYPE_START_TAG:
            indent++;
            cerr << "<" << GetElementName();
            j = 0;
            while (j < 100 && GetNextAttribute())
            {
                cerr << " " << GetAttributeName() << "=\""
                     << GetAttributeValue() << "\"";
                j++;
            }
            cerr << ">" << endl;
            break;
          case TYPE_END_TAG:
            cerr << "</" << GetElementName() << ">" << endl;
            break;
          case TYPE_CDATA:
            if (GetCDataLength() <= 1023)
                cerr << GetCDataValue() << endl;
            else
            {
                FILE *file = fopen(fname, "r");
                fseek(file, GetCDataOffset(), SEEK_SET);
                char *buf = new char[GetCDataLength()+1];
                fread(buf, 1, GetCDataLength(), file);
                buf[GetCDataLength()] = '\0';
                fclose(file);
                cerr << buf << endl;
                delete [] buf;
            }
            break;
          case TYPE_EOF:
            return;
            break;
          case TYPE_ERROR:
            cerr << "Error." << endl;
            break;
        }
    }
}


// ****************************************************************************
//  Method: XDMFParser::UnGetChar
//
//  Purpose:
//      Return the last character read to the input buffer. We can return
//      up to 8 characters to the input buffer.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 10:32:54 PDT 2007
//
// ****************************************************************************

void
XDMFParser::UnGetChar()
{
    if (ibuf > 0)
        ibuf--;
}


// ****************************************************************************
//  Method: XDMFParser::UnGetChar
//
//  Purpose:
//      Return the specified character to the input buffer. We can return
//      up to 8 characters to the input buffer.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 10:32:54 PDT 2007
//
// ****************************************************************************

void
XDMFParser::UnGetChar(const char &chr)
{
    if (ibuf > 0)
        buffer[--ibuf] = chr;
}


// ****************************************************************************
//  Method: XDMFParser::GetChar
//
//  Purpose:
//      Return the next character to the input buffer. We can peek ahead
//      up to 8 characters to the input buffer by indexing off of the
//      returned pointer.
//
//  Programmer: Eric Brugger
//  Creation:   Wed Nov 14 10:32:54 PDT 2007
//
// ****************************************************************************

char *
XDMFParser::GetChar()
{
    if (ibuf < lbuf)
        return &buffer[ibuf++];

    //
    // If we are at the end of the file, return a pointer to the
    // null character buffer.
    //
    if (endOfFile)
        return &(nullChar[0]);

    if (lbuf > 0)
    {
        //
        // We have data in the buffer, so copy 8 characters before and
        // after the current position to the front of the buffer. Set
        // ibuf to take into account that the first 8 characters have
        // already been processed. Increment the offset appropriatly.
        //
        for (int i = 0; i < 16; i++)
            buffer[i] = buffer[lbuf-8+i];
        ibuf = 8;
        offset += READ_SIZE;
    }
    else
    {
        //
        // This is the first time we have been called, so copy 16 null
        // characters to the front of the buffer. Set ibuf to take into
        // account that the first 16 characters in the buffer are null.
        // Set the offset appropriatly.
        //
        for (int i = 0; i < 16; i++)
            buffer[i] = '\0';
        ibuf = 16;
        offset = -16;
    }

    //
    // Fill the buffer.
    //
    int nread;
    if((nread = fread(&buffer[16], 1, READ_SIZE, file)) < READ_SIZE)
    {
        lbuf = nread + 16;
        for (int i = 0; i < 8; i++)
            buffer[lbuf+i] = '\0';
        endOfFile = true;
    }
    else
    {
        lbuf = nread + 16 - 8;
    }

    return &buffer[ibuf++];
}
