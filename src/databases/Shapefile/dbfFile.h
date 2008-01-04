/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#ifndef DBF_FILE_H
#define DBF_FILE_H
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
    
// ****************************************************************************
// Purpose: This header provides functions and types that can be used to
//          read fields from a DBASE file used by ESRI shape files.
//   
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Sun Mar 27 23:51:03 PST 2005
//
// Modifications:
//   Brad Whitlock, Wed Apr 6 10:30:23 PDT 2005
//   I removed common storage support.
//
// ****************************************************************************

typedef struct
{
    unsigned char month;
    unsigned char day;
    unsigned int  year;
} dbfDate_t;

typedef enum
{
    dbfFieldChar,
    dbfFieldDate,
    dbfFieldFloatingPointNumber,
    dbfFieldFixedPointNumber,
    dbfFieldLogical,
    dbfFieldMemo,
    dbfFieldVariable,
    dbfFieldPicture,
    dbfFieldBinary,
    dbfFieldGeneral,
    dbfFieldShortInt,
    dbfFieldInt,
    dbfFieldDouble,
} dbfFieldType_t;

typedef struct
{
    char           fieldName[11];
    dbfFieldType_t fieldType;
    unsigned int   fieldOffset;
    unsigned char  fieldLength;
    unsigned char  decimalCount;
    unsigned char  workAreaId;
    unsigned char  multiUser[2];
    unsigned char  setFields;
    unsigned char  fieldIsIndexed;
} dbfFieldDescriptor_t;

typedef struct
{
    unsigned char         format;
    dbfDate_t             lastUpdate;
    unsigned long         numRecords;
    unsigned short        headerSize;
    unsigned short        recordSize;
    unsigned char         transaction;
    unsigned char         encrypted;
    unsigned char         multiUser[12];
    unsigned char         indexExists;
    unsigned char         languageDriver;
    unsigned int          numFieldDescriptors;
    dbfFieldDescriptor_t *fieldDescriptors;
} dbfFileHeader_t;

typedef struct
{
    FILE            *fp;
    char            *filename;
    dbfFileHeader_t  header;
} dbfFile_t;

typedef enum
{
    dbfFileErrorSuccess,
    dbfFileErrorInvalidFile,
    dbfFileErrorNotFound
} dbfFileError_t;

typedef enum
{
    dbfReadErrorSuccess,
    dbfReadErrorFailure,
    dbfReadErrorInvalidRequest
} dbfReadError_t;

/* Memory allocation, deallocation routines. */
#define dbfMalloc(S)      dbfMallocEx(__FILE__,__LINE__, #S,S)
#define dbfFree(P)        dbfFreeEx(__FILE__,__LINE__, #P, P)
void *dbfMallocEx(const char *f, const int line, const char *src, size_t s);
void dbfFreeEx(const char *f, const int line, const char *src, void *ptr);

void dbfInitialize(void*(*user_malloc)(size_t),
                   void (*user_free)(void *));
void dbfFinalize();

dbfFile_t *dbfFileOpen(const char *filename, dbfFileError_t *);
void dbfFileClose(dbfFile_t *);

int dbfGetFieldStorageSize(dbfFieldDescriptor_t *);
dbfFieldDescriptor_t *dbfGetFieldDescriptor(dbfFile_t *, const char *field,
                                            unsigned int *offset);
void dbfSetForceFloat(int);
void *dbfFileReadField(dbfFile_t *, const char *field, dbfReadError_t *rcError);
void *dbfFileReadField2(dbfFile_t *, const char *field, void *data,
                        dbfReadError_t *rcError);

#ifdef __cplusplus
}
#endif

#endif
