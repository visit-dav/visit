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

void dbfInitialize(int commonStorage,
                   void*(*user_malloc)(size_t),
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
