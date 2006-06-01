/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <dbfFile.h>
#include <stdlib.h>
#include <string.h>

#define DBF_LITTLE_ENDIAN 0
#define DBF_BIG_ENDIAN    1

#define DBF_HEADER_SIZE   33
#define DBF_FIELD_DESCRIPTOR_SIZE 32

// ****************************************************************************
// Purpose: This library is for reading DBF files.
//   
// Programmer: Brad Whitlock
// Creation:   Fri Mar 25 16:41:09 PST 2005
//
// Modifications:
//   
// ****************************************************************************

/************************* INTERNAL LIBRARY VARIABLES ************************/
static int    dbfInitialized = 0;
static int    dbfIndentAmount = 0;

void*(*dbf_MallocCB)(size_t) = 0;
void (*dbf_FreeCB)(void *) = 0;

static int    dbfForceFloat = 0;

#ifdef DEBUG
/* Logging on by default. */
static int dbfLog = 1;
#else
/* Logging off by default. */
static int dbfLog = 0;
#endif
static int dbfLocalEndian = DBF_LITTLE_ENDIAN;

/*****************************************************************************/

short
dbfReadShortLE(const unsigned char *ptr)
{
    short retval = 0;

    if(dbfLocalEndian==DBF_LITTLE_ENDIAN)
    {
        retval = ((ptr[1] << 8)  & 0x0000ff00) | 
                  (ptr[0] & 0x000000ff);
    }
    else
    {
        retval = ((ptr[1] << 8)  & 0x0000ff00) | 
                  (ptr[0] & 0x000000ff);
    }

#if 0
    // byte trace
    fprintf(stdout, "dbfReadShortLE: bytes=[%d,%d], retval=%d\n",
           (int)ptr[0],
           (int)ptr[1]
           (int)retval);
#endif

    return retval;
}

unsigned short
dbfReadUShortLE(const unsigned char *ptr)
{
    unsigned short retval = 0;

    if(dbfLocalEndian==DBF_LITTLE_ENDIAN)
    {
        retval = ((ptr[1] << 8)  & 0x0000ff00) | 
                  (ptr[0] & 0x000000ff);
    }
    else
    {
        retval = ((ptr[1] << 8)  & 0x0000ff00) | 
                  (ptr[0] & 0x000000ff);
    }

#if 0
    // byte trace
    fprintf(stdout, "dbfReadShortLE: bytes=[%d,%d], retval=%d\n",
           (int)ptr[0],
           (int)ptr[1]
           (int)retval);
#endif

    return retval;
}

int
dbfReadIntLE(const unsigned char *ptr)
{
    int retval = 0;

    if(dbfLocalEndian==DBF_LITTLE_ENDIAN)
    {
        retval = ((ptr[3] << 24) & 0xff000000) | 
                 ((ptr[2] << 16) & 0x00ff0000) | 
                 ((ptr[1] << 8)  & 0x0000ff00) | 
                  (ptr[0] & 0x000000ff);
    }
    else
    {
        retval = ((ptr[3] << 24) & 0xff000000) | 
                 ((ptr[2] << 16) & 0x00ff0000) | 
                 ((ptr[1] << 8)  & 0x0000ff00) | 
                  (ptr[0] & 0x000000ff);
    }

#if 0
    // byte trace
    fprintf(stdout, "dbfReadIntLE: bytes=[%d,%d,%d,%d], retval=%d\n",
           (int)ptr[0],
           (int)ptr[1],
           (int)ptr[2],
           (int)ptr[3],
           retval);
#endif

    return retval;
}

unsigned int
dbfReadUIntLE(const unsigned char *ptr)
{
    unsigned int retval = 0;

    if(dbfLocalEndian==DBF_LITTLE_ENDIAN)
    {
        retval = ((ptr[3] << 24) & 0xff000000) | 
                 ((ptr[2] << 16) & 0x00ff0000) | 
                 ((ptr[1] << 8)  & 0x0000ff00) | 
                  (ptr[0] & 0x000000ff);
    }
    else
    {
        retval = ((ptr[3] << 24) & 0xff000000) | 
                 ((ptr[2] << 16) & 0x00ff0000) | 
                 ((ptr[1] << 8)  & 0x0000ff00) | 
                  (ptr[0] & 0x000000ff);
    }

#if 0
    // byte trace
    fprintf(stdout, "dbfReadIntLE: bytes=[%d,%d,%d,%d], retval=%d\n",
           (int)ptr[0],
           (int)ptr[1],
           (int)ptr[2],
           (int)ptr[3],
           retval);
#endif

    return retval;
}

double dbfReadDoubleLE(const unsigned char *ptr)
{
    double d;
    unsigned char *dest = (unsigned char *)&d;
    const unsigned char *src = ptr;
    if(dbfLocalEndian==DBF_LITTLE_ENDIAN)
    {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
        dest[3] = src[3];
        dest[4] = src[4];
        dest[5] = src[5];
        dest[6] = src[6];
        dest[7] = src[7];
    }
    else
    {
        dest[0] = src[7];
        dest[1] = src[6];
        dest[2] = src[5];
        dest[3] = src[4];
        dest[4] = src[3];
        dest[5] = src[2];
        dest[6] = src[1];
        dest[7] = src[0];
    }

    return d;
}

#define DBF_USHORT_LE(ptr) dbfReadUShortLE(ptr)
#define DBF_SHORT_LE(ptr)  dbfReadShortLE(ptr)
#define DBF_INT_LE(ptr)    dbfReadIntLE(ptr)
#define DBF_UINT_LE(ptr)   dbfReadUIntLE(ptr)
#define DBF_DOUBLE_LE(ptr) dbfReadDoubleLE((const unsigned char*)ptr)

/*****************************************************************************/

#define DBF_LOGFILE stdout

static void
dbfIndent(FILE *fp)
{
    int i;
    for(i = 0; i < dbfIndentAmount; ++i)
        fprintf(fp, "    ");
}

static void
dbfEnter(const char *name)
{
    if(dbfLog)
    {
        ++dbfIndentAmount;
        dbfIndent(DBF_LOGFILE);
        fprintf(DBF_LOGFILE, "%s {\n", name);
    }
}

static void
dbfLeave(const char *name)
{
    if(dbfLog)
    {
        dbfIndent(DBF_LOGFILE);
        fprintf(DBF_LOGFILE, "} %s\n", name);
        --dbfIndentAmount;
    }
}

static void
dbfErrorEx(const char *f, const int line, const char *msg)
{
    if(dbfLog)
    {
        dbfIndent(DBF_LOGFILE);
        fprintf(DBF_LOGFILE, "%s:%d %s", f, line, msg);
    }
}

#define DBF_API_ENTER(name) dbfEnter(#name);
#define DBF_API_LEAVE(name) dbfLeave(#name);
#define dbfError(msg) dbfErrorEx(__FILE__, __LINE__, msg)

/*****************************************************************************/

#define DBF_MEMLOG   DBF_LOGFILE

void *
dbfMallocEx(const char *f, const int line, const char *src, size_t s)
{
    void *retval = 0;
    DBF_API_ENTER(dbfMalloc)
    retval = dbf_MallocCB(s);

    /* Track the allocation. */
    if(dbfLog)
    {
        dbfIndent(DBF_MEMLOG);
        fprintf(DBF_MEMLOG, "dbfMalloc: %s:%d: %s: size=%d, ptr=%p\n",
            f, line, src, s, retval);
    }

    DBF_API_LEAVE(dbfMalloc);
    return retval;
}

void
dbfFreeEx(const char *f, const int line, const char *src, void *ptr)
{
    DBF_API_ENTER(dbfFree);
    if(ptr != 0)
        dbf_FreeCB(ptr);

    /* Track the free. */
    if(dbfLog)
    {
        dbfIndent(DBF_MEMLOG);
        fprintf(DBF_MEMLOG, "dbfFree: %s:%d: %s: ptr=%p\n", f, line, src, ptr);
    }
    DBF_API_LEAVE(dbfFree);
}

/*****************************************************************************/

// ****************************************************************************
// Function: dbfInitialize
//
// Purpose: 
//   Initializes the dbf library.
//
// Arguments:
//   commonStorage : When this value is set to 1, we enable the use of a common
//                   buffer into which the file can be read.
//   user_malloc   : User malloc function.
//   user_free     : User free function.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 28 01:45:32 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Apr 6 10:23:03 PDT 2005
//   I removed common storage from this library since everything that we read
//   is small and we don't do many allocations.
//
// ****************************************************************************

void
dbfInitialize(void* (*user_malloc)(size_t), void (*user_free)(void *))
{
    const int one = 1;
    const char *logging;
      
    if((logging = getenv("DBF_LOGGING")) != 0)
        dbfLog = (strcmp(logging, "1") == 0) ? 1 : 0;
    dbfInitialized = 1;
    dbfIndentAmount = 0;
    dbfLocalEndian = (*((unsigned char *)&one) == 1) ?
        DBF_LITTLE_ENDIAN : DBF_BIG_ENDIAN;

    /* Set up memory allocation, deallocation callbacks. */
    if(user_malloc == 0)
        dbf_MallocCB = malloc;
    else
        dbf_MallocCB = user_malloc;
    if(user_free == 0)
        dbf_FreeCB = free;
    else
        dbf_FreeCB = user_free;

    if(dbfLog)
        fprintf(DBF_LOGFILE, "dbfInitialize\n");
}

// ****************************************************************************
// Function: dbfFinalize
//
// Purpose: 
//   Finalizes the user's interaction with the library.
//
// Note:       Frees the common storage buffer. Could be used to check for
//             memory leaks if the dbfMallocEx, dbfFreeEx functions tracked
//             their memory usage. Right now it could be done by turning on
//             logging and making sure that each alloced pointer is also freed
//             in the logging output.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 28 01:47:35 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Apr 6 10:24:04 PDT 2005
//   I removed common storage.
//
// ****************************************************************************

void
dbfFinalize(void)
{
    if(dbfLog)
        fprintf(DBF_LOGFILE, "dbfFinalize\n");
}

// ****************************************************************************
// Function: dbfSetForceFloat
//
// Purpose: 
//   Sets whether values should be converted to float when they are read in.
//
// Arguments:
//   val : Non zero values force float conversion.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 28 01:50:20 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
dbfSetForceFloat(int val)
{
    dbfForceFloat = (val > 0) ? 1 : 0;
}

// ****************************************************************************
// Function: dbfFieldDescriptorRead
//
// Purpose: 
//   Populates a file descriptor from the bytes in fieldInfo.
//
// Arguments:
//   f         : The field descriptor to populate.
//   fieldInfo : The bytes to use to populate the field descriptor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 28 01:51:10 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

static void
dbfFieldDescriptorRead(dbfFieldDescriptor_t *f, const unsigned char *fieldInfo)
{
    DBF_API_ENTER(dbfFieldDescriptorRead);

    /* Take the bytes from fieldInfo and populate the field descriptor. */
    strncpy(f->fieldName, (const char *)fieldInfo, 10);
    if(fieldInfo[11] == 'C')
        f->fieldType = dbfFieldChar;
    else if(fieldInfo[11] == 'D')
        f->fieldType = dbfFieldDate;
    else if(fieldInfo[11] == 'F')
        f->fieldType = dbfFieldFloatingPointNumber;
    else if(fieldInfo[11] == 'N')
        f->fieldType = dbfFieldFixedPointNumber;
    else if(fieldInfo[11] == 'L')
        f->fieldType = dbfFieldLogical;
    else if(fieldInfo[11] == 'M')
        f->fieldType = dbfFieldMemo;
    else if(fieldInfo[11] == 'V')
        f->fieldType = dbfFieldVariable;
    else if(fieldInfo[11] == 'P')
        f->fieldType = dbfFieldPicture;
    else if(fieldInfo[11] == 'B')
        f->fieldType = dbfFieldBinary;
    else if(fieldInfo[11] == 'G')
        f->fieldType = dbfFieldGeneral;
    else if(fieldInfo[11] == '2')
        f->fieldType = dbfFieldShortInt;
    else if(fieldInfo[11] == '4')
        f->fieldType = dbfFieldInt;
    else if(fieldInfo[11] == '8')
        f->fieldType = dbfFieldDouble;
    else
        dbfError("Invalid field type.\n");

    f->fieldOffset     = DBF_UINT_LE(fieldInfo + 12);
    f->fieldLength    = fieldInfo[16];
    f->decimalCount   = fieldInfo[17];
    f->workAreaId     = fieldInfo[20];
    f->multiUser[0]   = fieldInfo[21];
    f->multiUser[1]   = fieldInfo[22];
    f->setFields      = fieldInfo[23];
    f->fieldIsIndexed = fieldInfo[31];

    DBF_API_LEAVE(dbfFieldDescriptorRead);
}

// ****************************************************************************
// Function: dbfFileOpen
//
// Purpose: 
//   Opens a DBF file for reading and reads the file header.
//
// Arguments:
//   filename : The name of the file to open.
//   code     : A return value indicating the success of the operation.
//
// Returns:    A pointer to a dbfFile_t object that you must free; otherwise 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 28 01:52:30 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Apr 6 10:19:17 PDT 2005
//   I removed common storage.
//
// ****************************************************************************

dbfFile_t *
dbfFileOpen(const char *filename, dbfFileError_t *code)
{
    dbfFile_t *fileObj = 0;
    FILE *fp = 0;
    DBF_API_ENTER(dbfFileOpen);

    if((fp = fopen(filename, "rb")) != 0)
    {
        unsigned char staticHeader[32];

        if(fread((void *)staticHeader, 1, 32, fp) == 32)
        {
            if(staticHeader[0] == 0x03 || /* plain .dbf */
               staticHeader[0] == 0x04 || /* plain .dbf */
               staticHeader[0] == 0x05 || /* plain .dbf */
               staticHeader[0] == 0x43 || /* with .dbv memo var size */
               staticHeader[0] == 0xB3 || /* with .dbv memo and .dbt memo */
               staticHeader[0] == 0x83 || /* with .dbt memo */
               staticHeader[0] == 0x8B || /* with .dbt memo in D4 format */
               staticHeader[0] == 0x8E || /* with SQL table */
               staticHeader[0] == 0xF5    /* with .fmp memo */
              )
            {
                size_t remainingHeaderSize;
                unsigned int i;
                unsigned char *header = 0;

                /* Allocate the file object. */
                fileObj = (dbfFile_t *)dbfMalloc(sizeof(dbfFile_t));
                memset(fileObj, 0, sizeof(dbfFile_t));
                fileObj->fp = fp;

                fileObj->filename = (char *)dbfMalloc(strlen(filename) + 1);
                strcpy(fileObj->filename, filename);

                /* Fill in the rest of the header. */
                fileObj->header.format           = staticHeader[0];
                fileObj->header.lastUpdate.year  = 1900+(unsigned int)staticHeader[1];
                fileObj->header.lastUpdate.month = staticHeader[2];
                fileObj->header.lastUpdate.day   = staticHeader[3];
                fileObj->header.numRecords       = (unsigned long)DBF_UINT_LE(staticHeader+4);
                fileObj->header.headerSize       = DBF_USHORT_LE(staticHeader+8);
                fileObj->header.recordSize       = DBF_USHORT_LE(staticHeader+10);
                fileObj->header.transaction      = staticHeader[14];
                fileObj->header.encrypted        = staticHeader[15];
                for(i = 0; i < 12; ++i)
                    fileObj->header.multiUser[i] = staticHeader[16+i];
                fileObj->header.indexExists      = staticHeader[28];
                fileObj->header.languageDriver   = staticHeader[29];

                /* Now try and read all of the field descriptors. */
                remainingHeaderSize = fileObj->header.headerSize - 32;
                header = (unsigned char *)dbfMalloc(remainingHeaderSize);
                if(fread((void *)header, 1, remainingHeaderSize, fileObj->fp) ==
                   remainingHeaderSize)
                {
                    size_t m = 0;
                    unsigned char *fieldInfo = header;

                    fileObj->header.numFieldDescriptors = (remainingHeaderSize - 1) / 32;
                    m = fileObj->header.numFieldDescriptors *
                        sizeof(dbfFieldDescriptor_t);
                    fileObj->header.fieldDescriptors = (dbfFieldDescriptor_t *)dbfMalloc(m);
                    memset((void *)fileObj->header.fieldDescriptors, 0, m);

                    /* Read an individual field descriptor. */
                    for(i = 0; i < fileObj->header.numFieldDescriptors; ++i)
                    {
                        dbfFieldDescriptor_t *f = fileObj->header.fieldDescriptors + i;
                        dbfFieldDescriptorRead(f, fieldInfo);
                        fieldInfo += 32;
                    }

                    *code = dbfFileErrorSuccess;
                }
                else
                    *code = dbfFileErrorInvalidFile;

                dbfFree(header);
            }
            else
                *code = dbfFileErrorInvalidFile;
        }
        else
            *code = dbfFileErrorInvalidFile;
    }
    else
        *code = dbfFileErrorNotFound;
    DBF_API_LEAVE(dbfFileOpen);
    
    return fileObj;
}

// ****************************************************************************
// Function: dbfFileClose
//
// Purpose:
//   Closes the file object's file and deletes the file object.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 28 01:54:22 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
dbfFileClose(dbfFile_t *fileObj)
{
    DBF_API_ENTER(dbfFileClose);
    if(fileObj != 0 && fileObj->fp != 0)
    {
        fclose(fileObj->fp);
        dbfFree(fileObj->filename);
        dbfFree(fileObj->header.fieldDescriptors);
        dbfFree(fileObj);
    }
    DBF_API_LEAVE(dbfFileClose);
}

// ****************************************************************************
// Function: dbfGetFieldStorageSize
//
// Purpose: 
//   Returns how many bytes it takes to store a specific field's field data
//   from 1 record.
//
// Arguments:
//   fieldDescriptor : The field whose storage requirements we're querying.
//
// Returns:    The number of bytes.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 28 02:00:10 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

int
dbfGetFieldStorageSize(dbfFieldDescriptor_t *fieldDescriptor)
{
    size_t s = 0;

    DBF_API_ENTER(dbfGetFieldStorageSize);

    switch(fieldDescriptor->fieldType)
    {
    case dbfFieldChar:
    case dbfFieldDate:
        s = (fieldDescriptor->fieldLength + 1);
        break;
    case dbfFieldFloatingPointNumber:
    case dbfFieldFixedPointNumber:
        s = dbfForceFloat ? sizeof(float) : sizeof(double);
        break;
    case dbfFieldLogical:
        s = 1;
        break;
    case dbfFieldShortInt:
        s = dbfForceFloat ? sizeof(float) : sizeof(short);
        break;
    case dbfFieldInt:
        s = dbfForceFloat ? sizeof(float) : sizeof(int);
        break;
    case dbfFieldDouble:
        s = dbfForceFloat ? sizeof(float) : sizeof(double);
        break;
    default:
        break;
    }

    DBF_API_LEAVE(dbfGetFieldStorageSize);

    return s;
}

// ****************************************************************************
// Method: dbfGetFieldDescriptor
//
// Purpose: 
//   Returns a pointer to the specified field's field descriptor.
//
// Arguments:
//    fileObj   : The DBf file object.
//    fieldName : The name of the field.
//    offset    : The offset of the field within a record.
//
// Returns:    A pointer to the field descriptor or 0 if it is not found.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 28 01:58:43 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

dbfFieldDescriptor_t *
dbfGetFieldDescriptor(dbfFile_t *fileObj, const char *fieldName, unsigned int *offset)
{
    dbfFieldDescriptor_t *fieldDescriptor = 0;
    unsigned int f, fieldOffset = 1;

    DBF_API_ENTER(dbfGetFieldDescriptor);

    /* Look for the field index of the specified field and calculate the
     * field's offset into the record.
     */
    for(f = 0; f < fileObj->header.numFieldDescriptors; ++f)
    {
        if(strcmp(fileObj->header.fieldDescriptors[f].fieldName, fieldName) == 0)
        {
            fieldDescriptor = fileObj->header.fieldDescriptors + f;
            break;
        }
        else
            fieldOffset += fileObj->header.fieldDescriptors[f].fieldLength;
    }

    *offset = (fieldDescriptor != 0) ? fieldOffset : 0;

    DBF_API_LEAVE(dbfGetFieldDescriptor);

    return fieldDescriptor;
}

// ****************************************************************************
// Method: dbfFileReadField
//
// Purpose: 
//   Reads a specified field into preallocated memory.
//
// Arguments:
//   fileObj   : The file object from which to read.
//   fieldName : The name of the field to read.
//   code      : The error code.
//
// Returns:    code is set to dbfFileErrorSuccess if it works.
//             The memory containing the field is returned.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 28 01:55:28 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void *
dbfFileReadField(dbfFile_t *fileObj, const char *fieldName, dbfReadError_t *code)
{
    void *data = 0;
    unsigned int fieldOffset;
    dbfFieldDescriptor_t *fieldDescriptor = 0;

    DBF_API_ENTER(dbfFileReadField);
    
    /* Get the field descriptor for the specified field. */
    fieldDescriptor = dbfGetFieldDescriptor(fileObj, fieldName, &fieldOffset);
    if(fieldDescriptor != 0)
    {
        /* Allocate memory for the field. */
        size_t fieldStorageSize = dbfGetFieldStorageSize(fieldDescriptor);
        data = dbfMalloc(fileObj->header.numRecords * fieldStorageSize);

        /* Read the field */
        dbfFileReadField2(fileObj, fieldName, data, code);

        /* If the field could not be read, free the data array. */
        if(*code != dbfReadErrorSuccess)
        {
            dbfFree(data);
            data = 0;
        }
    }
    else
        *code = dbfReadErrorInvalidRequest;

    DBF_API_LEAVE(dbfFileReadField);

    return data;
}

// ****************************************************************************
// Method: dbfFileReadField2
//
// Purpose: 
//   Reads a specified field into preallocated memory.
//
// Arguments:
//   fileObj   : The file object from which to read.
//   fieldName : The name of the field to read.
//   data      : The data array into which we'll read data.
//   code      : The error code.
//
// Returns:    code is set to dbfFileErrorSuccess if it works.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 28 01:55:28 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Apr 6 10:26:02 PDT 2005
//   I removed common storage since it was used so little.
//
// ****************************************************************************

void *
dbfFileReadField2(dbfFile_t *fileObj, const char *fieldName, void *data,
    dbfReadError_t *code)
{
    unsigned int fieldOffset = 1;
    dbfFieldDescriptor_t *fieldDescriptor = 0;

    DBF_API_ENTER(dbfFileReadField2);

    if((fieldDescriptor = dbfGetFieldDescriptor(fileObj, fieldName, &fieldOffset)) != 0)
    {
        unsigned int rec, noError = 1;
        unsigned int offset = fileObj->header.recordSize - fieldDescriptor->fieldLength;
        size_t s = fileObj->header.numRecords * dbfGetFieldStorageSize(fieldDescriptor);

        /* Seek to the start of the file and then skip to the first record. */            
        if(fseek(fileObj->fp, DBF_HEADER_SIZE + 
            fileObj->header.numFieldDescriptors *
            DBF_FIELD_DESCRIPTOR_SIZE, SEEK_SET) != 0)
        {
            *code = dbfReadErrorFailure;
            goto dbfFileReadFieldLeave;
        }

        /* Seek to the start of the field that we want. */
        if(fseek(fileObj->fp, fieldOffset, SEEK_CUR) != 0)
        {
            *code = dbfReadErrorFailure;
            goto dbfFileReadFieldLeave;
        }

        /* We found a matching field name. Prepare the return data array. */
        switch(fieldDescriptor->fieldType)
        {
        case dbfFieldChar:
        case dbfFieldDate:
            { // new scope
            // Note that we allocate fieldLength + 1 so we can insert a NULL terminator.
            unsigned char *cptr = (unsigned char *)data;
            memset(data, 0, s);

            /* Read the character field from each record. */
            for(rec = 0; rec < fileObj->header.numRecords && noError; ++rec)
            {
                /* Read the field from the file. */
                if(fread((void *)cptr, 1, fieldDescriptor->fieldLength, fileObj->fp) ==
                   fieldDescriptor->fieldLength)
                {
                    /* Get rid of trailing spaces. */
                    unsigned char *endptr = cptr + fieldDescriptor->fieldLength - 1;
                    while(*endptr == ' ' && endptr > cptr)
                        *endptr-- = '\0';

                    /* Seek to the start of the field in the next record. */
                    if(fseek(fileObj->fp, offset, SEEK_CUR) != 0)
                        noError = 0;

                    cptr += (fieldDescriptor->fieldLength + 1);
                }
                else
                    noError = 0;
            }

            } // end new scope
            break;
       case dbfFieldFloatingPointNumber:
       case dbfFieldFixedPointNumber:
            if(dbfForceFloat)
            { // new scope
                char *tmp = 0;
                float *fptr = (float *)data;
                memset(data, 0, s);

                s = fieldDescriptor->fieldLength + 1;
                tmp = (char *)dbfMalloc(s);
                memset(tmp, 0, s);

                /* Read the floating/fixed point number field from each record. */
                for(rec = 0; rec < fileObj->header.numRecords && noError; ++rec)
                {
                    /* Read the field from the file. */
                    if(fread((void *)tmp, 1, fieldDescriptor->fieldLength, fileObj->fp) ==
                       fieldDescriptor->fieldLength)
                    {
                        char *endptr = 0;
                        *fptr = (float)strtod(tmp, &endptr);
                        ++fptr;
                        /* Seek to the start of the field in the next record. */
                        if(fseek(fileObj->fp, offset, SEEK_CUR) != 0)
                            noError = 0;
                    }
                    else
                        noError = 0;
                }

                dbfFree(tmp);

            } // end new scope
            else
            { // new scope
                char *tmp = 0;
                double *dptr = (double *)data;
                memset(data, 0, s);

                s = fieldDescriptor->fieldLength + 1;
                tmp = (char *)dbfMalloc(s);
                memset(tmp, 0, s);

                /* Read the floating/fixed point number field from each record. */
                for(rec = 0; rec < fileObj->header.numRecords && noError; ++rec)
                {
                    /* Read the field from the file. */
                    if(fread((void *)tmp, 1, fieldDescriptor->fieldLength, fileObj->fp) ==
                       fieldDescriptor->fieldLength)
                    {
                        char *endptr = 0;
                        *dptr = strtod(tmp, &endptr);
                        ++dptr;
                        /* Seek to the start of the field in the next record. */
                        if(fseek(fileObj->fp, offset, SEEK_CUR) != 0)
                            noError = 0;
                    }
                    else
                        noError = 0;
                }

                dbfFree(tmp);
            } // end new scope
            break;
        case dbfFieldLogical:
            { // new scope
            unsigned char *cptr = (unsigned char *)data;
            memset(data, 'y', s);

            /* Read the logical field from each record. */
            for(rec = 0; rec < fileObj->header.numRecords && noError; ++rec)
            {
                /* Read the field from the file. */
                if(fread((void *)cptr, 1, fieldDescriptor->fieldLength, fileObj->fp) ==
                   fieldDescriptor->fieldLength)
                {
                    /* Seek to the start of the field in the next record. */
                    if(fseek(fileObj->fp, offset, SEEK_CUR) != 0)
                        noError = 0;
                    ++cptr;
                }
                else
                    noError = 0;
            }

            } // end new scope
            break;
        case dbfFieldMemo:
            dbfError("dbfFileReadField: dbfFieldMemo is not supported.\n");
            *code = dbfReadErrorFailure;
            break;
        case dbfFieldVariable:
            dbfError("dbfFileReadField: dbfFieldVariable is not supported.\n");
            *code = dbfReadErrorFailure;
            break;
        case dbfFieldPicture:
            dbfError("dbfFileReadField: dbfFieldPicture is not supported.\n");
            *code = dbfReadErrorFailure;
            break;
        case dbfFieldBinary:
            dbfError("dbfFileReadField: dbfFieldBinary is not supported.\n");
            *code = dbfReadErrorFailure;
            break;
        case dbfFieldGeneral:
            dbfError("dbfFileReadField: dbfFieldGeneral is not supported.\n");
            *code = dbfReadErrorFailure;
            break;
        case dbfFieldShortInt:
            if(dbfForceFloat)
            { // new scope
            float *fptr = (float *)data;
            memset(data, 0, s);

            /* Read the logical field from each record. */
            for(rec = 0; rec < fileObj->header.numRecords && noError; ++rec)
            {
                unsigned char tmp[2];
                /* Read the field from the file. */
                if(fread((void *)tmp, 1, fieldDescriptor->fieldLength, fileObj->fp) ==
                   fieldDescriptor->fieldLength)
                {
                    *fptr = (float)DBF_SHORT_LE(tmp);
                    ++fptr;

                    /* Seek to the start of the field in the next record. */
                    if(fseek(fileObj->fp, offset, SEEK_CUR) != 0)
                        noError = 0;
                }
                else
                    noError = 0;
            }

            } // end new scope
            else
            { // new scope
            short *sptr = (short *)data;
            memset(data, 0, s);

            /* Read the logical field from each record. */
            for(rec = 0; rec < fileObj->header.numRecords && noError; ++rec)
            {
                unsigned char tmp[2];
                /* Read the field from the file. */
                if(fread((void *)tmp, 1, fieldDescriptor->fieldLength, fileObj->fp) ==
                   fieldDescriptor->fieldLength)
                {
                    *sptr = DBF_SHORT_LE(tmp);
                    ++sptr;

                    /* Seek to the start of the field in the next record. */
                    if(fseek(fileObj->fp, offset, SEEK_CUR) != 0)
                        noError = 0;
                }
                else
                    noError = 0;
            }

            } // end new scope
            break;
        case dbfFieldInt:
            if(dbfForceFloat)
            { // new scope
                float *fptr = (float *)data;
                memset(data, 0, s);

                /* Read the logical field from each record. */
                for(rec = 0; rec < fileObj->header.numRecords && noError; ++rec)
                {
                    unsigned char tmp[4];
                    /* Read the field from the file. */
                    if(fread((void *)tmp, 1, fieldDescriptor->fieldLength, fileObj->fp) ==
                       fieldDescriptor->fieldLength)
                    {
                        *fptr = (float)DBF_INT_LE(tmp);
                        ++fptr;

                        /* Seek to the start of the field in the next record. */
                        if(fseek(fileObj->fp, offset, SEEK_CUR) != 0)
                            noError = 0;
                    }
                    else
                        noError = 0;
                }
            } // end new scope
            else
            { // new scope
                int *iptr = (int *)data;
                memset(data, 0, s);

                /* Read the logical field from each record. */
                for(rec = 0; rec < fileObj->header.numRecords && noError; ++rec)
                {
                    unsigned char tmp[4];
                    /* Read the field from the file. */
                    if(fread((void *)tmp, 1, fieldDescriptor->fieldLength, fileObj->fp) ==
                       fieldDescriptor->fieldLength)
                    {
                        *iptr = DBF_INT_LE(tmp);
                        ++iptr;

                        /* Seek to the start of the field in the next record. */
                        if(fseek(fileObj->fp, offset, SEEK_CUR) != 0)
                            noError = 0;
                    }
                    else
                        noError = 0;
                }
            } // end new scope
            break;
        case dbfFieldDouble:
            if(dbfForceFloat)
            { // new scope
                float *fptr = (float *)data;
                memset(data, 0, s);

                /* Read the logical field from each record. */
                for(rec = 0; rec < fileObj->header.numRecords && noError; ++rec)
                {
                    unsigned char tmp[8];
                    /* Read the field from the file. */
                    if(fread((void *)tmp, 1, fieldDescriptor->fieldLength, fileObj->fp) ==
                       fieldDescriptor->fieldLength)
                    {
                        *fptr = (float)DBF_DOUBLE_LE(tmp);
                        ++fptr;

                        /* Seek to the start of the field in the next record. */
                        if(fseek(fileObj->fp, offset, SEEK_CUR) != 0)
                            noError = 0;
                    }
                    else
                        noError = 0;
                }
            } // end new scope
            else
            { // new scope
                double *dptr = (double *)data;
                memset(data, 0, s);

                /* Read the logical field from each record. */
                for(rec = 0; rec < fileObj->header.numRecords && noError; ++rec)
                {
                    unsigned char tmp[8];
                    /* Read the field from the file. */
                    if(fread((void *)tmp, 1, fieldDescriptor->fieldLength, fileObj->fp) ==
                       fieldDescriptor->fieldLength)
                    {
                        *dptr = DBF_DOUBLE_LE(tmp);
                        ++dptr;

                        /* Seek to the start of the field in the next record. */
                        if(fseek(fileObj->fp, offset, SEEK_CUR) != 0)
                            noError = 0;
                    }
                    else
                        noError = 0;
                }
            } // end new scope
            break;
        }

        /* Set the function's return code. */
        if(noError && data != 0)
            *code = dbfReadErrorSuccess;
        else
            *code = dbfReadErrorFailure;
    }
    else
        *code = dbfReadErrorInvalidRequest;

dbfFileReadFieldLeave:
    DBF_API_LEAVE(dbfFileReadField2);
 
    return data;
}
