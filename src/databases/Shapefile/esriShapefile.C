/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#include <esriShapefile.h>
#include <stdlib.h>
#include <string.h>

#define ESRI_LITTLE_ENDIAN 0
#define ESRI_BIG_ENDIAN    1

#define ESRI_BIG_NUMBER    1.e20
#define ESRI_FABS(N)       (((N) < 0.) ? -(N) : (N))

// ****************************************************************************
// Purpose: This library is for reading ESRI shape files.
//   
// Programmer: Brad Whitlock
// Creation:   Fri Mar 25 16:41:09 PST 2005
//
// Modifications:
//   
// ****************************************************************************

/************************* INTERNAL LIBRARY VARIABLES ************************/
static int    esriInitialized = 0;
static int    esriIndentAmount = 0;

static int    esriCommonStorage = 1;
static size_t esriCommonStorageBufferSize = 10000;
static void  *esriCommonStorageBuffer = 0;

#ifdef DEBUG
/* Logging on by default. */
static int esriLog = 1;
#else
/* Logging off by default. */
static int esriLog = 0;
#endif
static int esriLocalEndian = ESRI_LITTLE_ENDIAN;

/* Error handling function. */
static void (*esriErrorHandler)(const char *) = 0;

/*****************************************************************************/

int
esriReadIntLE(const unsigned char *ptr)
{
    int retval = 0;

    if(esriLocalEndian==ESRI_LITTLE_ENDIAN)
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
    fprintf(stdout, "esriReadIntLE: bytes=[%d,%d,%d,%d], retval=%d\n",
           (int)ptr[0],
           (int)ptr[1],
           (int)ptr[2],
           (int)ptr[3],
           retval);
#endif

    return retval;
}

int
esriReadIntBE(const unsigned char *ptr)
{
    int retval = 0;

    if(esriLocalEndian==ESRI_BIG_ENDIAN)
    {
        retval = ((ptr[0] << 24) & 0xff000000) | 
                 ((ptr[1] << 16) & 0x00ff0000) | 
                 ((ptr[2] << 8)  & 0x0000ff00) | 
                  (ptr[3] & 0x000000ff);
    }
    else
    {
        retval = ((ptr[0] << 24) & 0xff000000) | 
                 ((ptr[1] << 16) & 0x00ff0000) | 
                 ((ptr[2] << 8)  & 0x0000ff00) | 
                  (ptr[3] & 0x000000ff);
    }

#if 0
    // byte trace
    fprintf(stdout, "esriReadIntBE: bytes=[%d,%d,%d,%d], retval=%d\n",
           (int)ptr[0],
           (int)ptr[1],
           (int)ptr[2],
           (int)ptr[3],
           retval);
#endif

    return retval;
}

#define ESRI_INT_LE(ptr) esriReadIntLE(ptr)
#define ESRI_INT_BE(ptr) esriReadIntBE(ptr)

double esriReadDoubleLE(const unsigned char *ptr)
{
    double d;
    unsigned char *dest = (unsigned char *)&d;
    const unsigned char *src = ptr;
    if(esriLocalEndian==ESRI_LITTLE_ENDIAN)
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

double esriReadDoubleBE(const unsigned char *ptr)
{
    double d;
    unsigned char *dest = (unsigned char *)&d;
    const unsigned char *src = ptr;
    if(esriLocalEndian==ESRI_BIG_ENDIAN)
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
    else
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

    return d;
}

#define ESRI_DOUBLE_LE(ptr) esriReadDoubleLE((const unsigned char*)ptr)
#define ESRI_DOUBLE_BE(ptr) esriReadDoubleBE((const unsigned char*)ptr)

/*****************************************************************************/

#define ESRI_LOGFILE stdout

static void
esriIndent(FILE *fp)
{
    int i;
    for(i = 0; i < esriIndentAmount; ++i)
        fprintf(fp, "    ");
}

static void
esriEnter(const char *name)
{
    if(esriLog)
    {
        ++esriIndentAmount;
        esriIndent(ESRI_LOGFILE);
        fprintf(ESRI_LOGFILE, "%s {\n", name);
    }
}

static void
esriLeave(const char *name)
{
    if(esriLog)
    {
        esriIndent(ESRI_LOGFILE);
        fprintf(ESRI_LOGFILE, "} %s\n", name);
        --esriIndentAmount;
    }
}

static void
esriErrorHandler_default(const char *msg)
{
    if(esriLog)
    {
        esriIndent(ESRI_LOGFILE);
        fprintf(ESRI_LOGFILE, "ERROR *** %s\n", msg);
    }
}

#define ESRI_API_ENTER(name) esriEnter(#name);
#define ESRI_API_LEAVE(name) esriLeave(#name);
#define esriError(S)         esriErrorHandler(S)

/*****************************************************************************/

#define ESRI_MEMLOG   ESRI_LOGFILE

void *
esriMallocEx(const char *f, const int line, const char *src, size_t s)
{
    void *retval = 0;
    ESRI_API_ENTER(esriMalloc)
    retval = malloc(s);

    /* Track the allocation. */
    if(esriLog)
    {
        esriIndent(ESRI_MEMLOG);
        fprintf(ESRI_MEMLOG, "esriMalloc: %s:%d: %s: size=%d, ptr=%p\n",
            f, line, src, s, retval);
    }

    ESRI_API_LEAVE(esriMalloc);
    return retval;
}

void
esriFreeEx(const char *f, const int line, const char *src, void *ptr)
{
    ESRI_API_ENTER(esriFree);
    if(ptr != 0)
        free(ptr);

    /* Track the free. */
    if(esriLog)
    {
        esriIndent(ESRI_MEMLOG);
        fprintf(ESRI_MEMLOG, "esriFree: %s:%d: %s: ptr=%p\n", f, line, src, ptr);
    }
    ESRI_API_LEAVE(esriFree);
}

// ****************************************************************************
// Method: esriFreeShapeEx
//
// Purpose: 
//   Frees memory belonging to a specific shape type.
//
// Arguments:
//   f    : The file where esriFree was called.
//   line : The line in the file where esriFree was called.
//   src  : The source arguments to esriFree.
//   t    : The type of shape.
//   ptr  : The pointer to the shape.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:37:39 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriFreeShapeEx(const char *f, const int line, const char *src,
    esriShapeType_t t, void *ptr)
{
    ESRI_API_ENTER(esriFreeShape);
    /* Track the free. */
    if(esriLog)
    {
        esriIndent(ESRI_MEMLOG);
        fprintf(ESRI_MEMLOG, "esriFreeShape: %s:%d: %s: ptr=%p\n", f, line, src, ptr);
    }
    switch(t)
    {
    case esriNullShape:
        esriFree(ptr);
        break;
    case esriPoint:
        esriFreePoint((esriPoint_t *)ptr);
        break;
    case esriPolyLine:
        esriFreePolyLine((esriPolyLine_t *)ptr);
        break;
    case esriPolygon:
        esriFreePolygon((esriPolygon_t *)ptr);
        break;
    case esriMultiPoint:
        esriFreeMultiPoint((esriMultiPoint_t *)ptr);
        break;
    case esriPointZ:
        esriFreePointZ((esriPointZ_t *)ptr);
        break;
    case esriPolyLineZ:
        esriFreePolyLineZ((esriPolyLineZ_t *)ptr);
        break;
    case esriPolygonZ:
        esriFreePolygonZ((esriPolygonZ_t *)ptr);
        break;
    case esriMultiPointZ:
        esriFreeMultiPointZ((esriMultiPointZ_t *)ptr);
        break;
    case esriPointM:
        esriFreePointM((esriPointM_t *)ptr);
        break;
    case esriPolyLineM:
        esriFreePolyLineM((esriPolyLineM_t *)ptr);
        break;
    case esriPolygonM:
        esriFreePolygonM((esriPolygonM_t *)ptr);
        break;
    case esriMultiPointM:
        esriFreeMultiPointM((esriMultiPointM_t *)ptr);
        break;
    case esriMultiPatch:
        esriFreeMultiPatch((esriMultiPatch_t *)ptr);
        break;
    }
    ESRI_API_LEAVE(esriFreeShape);
}

/* Returns a pointer to the common storage buffer if we're doing common
   storage. Otherwise, returns new memory.
  */
void *
esriCommonStorageAlloc(size_t s)
{
    void *retval = 0;
    ESRI_API_LEAVE(esriCommonStorageAlloc);
    if(esriCommonStorage)
    {
        if(s > esriCommonStorageBufferSize)
        {
            esriFree(esriCommonStorageBuffer);
            esriCommonStorageBufferSize = (size_t)(s * 1.25);
            esriCommonStorageBuffer = esriMalloc(esriCommonStorageBufferSize);
        }
        retval = esriCommonStorageBuffer;
    }
    else
        retval = esriMalloc(s);
    ESRI_API_LEAVE(esriCommonStorageAlloc);
    return retval;
}

void
esriCommonStorageFree(void *ptr)
{
    ESRI_API_LEAVE(esriCommonStorageFree);
    if(!esriCommonStorage)
        esriFree(ptr);
    ESRI_API_LEAVE(esriCommonStorageFree);
}

/*****************************************************************************/

// ****************************************************************************
// Method: esriInitialize
//
// Purpose: 
//   Initializes the esri library.
//
// Arguments:
//   commonStorage : Enables use of common storage.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:39:18 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriInitialize(int commonStorage, void (*errFunc)(const char *))
{
    const int one = 1;
    const char *logging;
      
    if((logging = getenv("ESRI_LOGGING")) != 0)
        esriLog = (strcmp(logging, "1") == 0) ? 1 : 0;
    esriInitialized = 1;
    esriIndentAmount = 0;
    esriLocalEndian = (*((unsigned char *)&one) == 1) ?
        ESRI_LITTLE_ENDIAN : ESRI_BIG_ENDIAN;

    /* Set up an error handler. */
    if(errFunc == 0)
        esriErrorHandler = esriErrorHandler_default;
    else
        esriErrorHandler = errFunc;

    /* Set up the common storage. */
    esriCommonStorage = commonStorage;
    if(esriCommonStorage)
    {
        esriCommonStorageBufferSize = 10000;
        esriCommonStorageBuffer = esriMalloc(esriCommonStorageBufferSize);
    }

    if(esriLog)
        fprintf(ESRI_LOGFILE, "esriInitialize\n");
}

// ****************************************************************************
// Method: esriFinalize
//
// Purpose: 
//   Final interaction with the esri library.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:39:47 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriFinalize(void)
{
    if(esriCommonStorage)
    {
        esriFree(esriCommonStorageBuffer);
        esriCommonStorageBuffer = 0;
        esriCommonStorageBufferSize = 0;
    }

    if(esriLog)
        fprintf(ESRI_LOGFILE, "esriFinalize\n");
}

// ****************************************************************************
// Method: esriShapefileOpen
//
// Purpose: 
//   Opens a shape file and returns a handle to the file.
//
// Arguments:
//   filename : The name of the file to open.
//   code     : The error code to be returned.
//
// Returns:    A pointer to the file handle or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:40:12 PST 2005
//
// Modifications:
//   
// ****************************************************************************

esriShapefile_t *
esriShapefileOpen(const char *filename, esriFileError_t *code)
{
    esriShapefile_t *fileObj = 0;
    FILE *fp = 0;
    ESRI_API_ENTER(esriShapefileOpen);

    if((fp = fopen(filename, "rb")) != 0)
    {
        unsigned char header[100];

        if(fread((void *)header, 1, 100, fp) == 100)
        {
            int file_code = ESRI_INT_BE(header);
            if(file_code == 9994)
            {
                fileObj = (esriShapefile_t *)esriMalloc(sizeof(esriShapefile_t));
                memset(fileObj, 0, sizeof(esriShapefile_t));
                fileObj->fp = fp;

                fileObj->filename = (char *)esriMalloc(strlen(filename) + 1);
                strcpy(fileObj->filename, filename);

                /* Fill in the rest of the header. */
                fileObj->header.file_code   = file_code;
                fileObj->header.file_length = ESRI_INT_BE(header+24);
                fileObj->header.version     = ESRI_INT_LE(header+28);
                fileObj->header.shape_type  = (esriShapeType_t)(ESRI_INT_LE(header+32));
                fileObj->header.xmin        = ESRI_DOUBLE_LE(header+36);
                fileObj->header.xmax        = ESRI_DOUBLE_LE(header+44);
                fileObj->header.ymin        = ESRI_DOUBLE_LE(header+52);
                fileObj->header.ymax        = ESRI_DOUBLE_LE(header+60);
                fileObj->header.zmin        = ESRI_DOUBLE_LE(header+68);
                fileObj->header.zmax        = ESRI_DOUBLE_LE(header+76);
                fileObj->header.mmin        = ESRI_DOUBLE_LE(header+84);
                fileObj->header.mmax        = ESRI_DOUBLE_LE(header+92);

                *code = esriFileErrorSuccess;
            }
            else
                *code = esriFileErrorInvalidFile;
        }
        else
            *code = esriFileErrorInvalidFile;
    }
    else
        *code = esriFileErrorNotFound;
    ESRI_API_LEAVE(esriShapefileOpen);
    
    return fileObj;
}

// ****************************************************************************
// Method: esriShapefileClose
//
// Purpose: 
//   Closes the esri file.
//
// Arguments:
//   fileObj : A pointer to the file object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:41:13 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriShapefileClose(esriShapefile_t *fileObj)
{
    ESRI_API_ENTER(esriShapefileClose);
    if(fileObj != 0 && fileObj->fp != 0)
    {
        fclose(fileObj->fp);
        esriFree(fileObj->filename);
        esriFree(fileObj);
    }
    ESRI_API_LEAVE(esriShapefileClose);
}

// ****************************************************************************
// Method: esriShapefileReadRecordHeader
//
// Purpose: 
//   Reads a shape header.
//
// Arguments:
//   fileObj : A pointer to the file object.
//   rec     : A pointer to the record header to populate.
//   code    : The return code, indicating success or failure.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:41:36 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriShapefileReadRecordHeader(esriShapefile_t *fileObj,
    esriShapefileRecordHeader_t *rec, esriReadError_t *code)
{
    ESRI_API_ENTER(esriShapefileReadRecordHeader);
    if(fileObj != 0 && fileObj->fp != 0)
    {
        unsigned char header[12];

        if(fread((void *)header, 1, 12, fileObj->fp) == 12)
        {
            rec->fileObj = fileObj;
            rec->recordNumber  = ESRI_INT_BE(header);
            /* Make the contentLength contain the number of bytes in
               the record (instead of the number of 16-bit words) minus
               the number of bytes in the record's shapeType identifier.
            */
            rec->contentLength = ESRI_INT_BE(header+4) * 2 - 4;
            /* Read the record's shapeType so we know how which function
               to call to read the next record.
             */
            rec->shapeType     = (esriShapeType_t)(ESRI_INT_LE(header+8));
            *code = esriReadErrorSuccess;
        }
        else
            *code = esriReadErrorFailure;
    }
    else
        *code = esriReadErrorFailure;
    ESRI_API_LEAVE(esriShapefileReadRecordHeader);
}

// ****************************************************************************
// Method: esriShapefileReadPoint
//
// Purpose: 
//   Reads a point.
//
// Arguments:
//   rec  : The record header returned by a previous call to
//          esriShapefileReadRecordHeader.
//   obj  : The object to be populated.
//   code : The return code, indicating success, failure.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:43:02 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriShapefileReadPoint(esriShapefileRecordHeader_t *rec,
    esriPoint_t *obj, esriReadError_t *code)
{
    unsigned char data[16];
    ESRI_API_ENTER(esriShapefileReadPoint);
    if(fread((void*)data, 1, 16, rec->fileObj->fp) == 16)
    {
        obj->x = ESRI_DOUBLE_LE(data);
        obj->y = ESRI_DOUBLE_LE(data+8);
        *code = esriReadErrorSuccess;
    }
    else
        *code = esriReadErrorFailure;
    ESRI_API_LEAVE(esriShapefileReadPoint);
}

void
esriFreePoint(esriPoint_t *obj)
{
    ESRI_API_ENTER(esriFreePoint);
    if(obj != 0)
        esriFree((void*)obj);
    ESRI_API_LEAVE(esriFreePoint);
}

void
esriPrintPoint(FILE *fp, esriPoint_t *obj)
{
    ESRI_API_ENTER(esriPrintPoint);
    fprintf(fp, "Point\n");
    fprintf(fp, "\tx = %g\n", obj->x);
    fprintf(fp, "\ty = %g\n", obj->y);
    ESRI_API_LEAVE(esriPrintPoint);
}

// ****************************************************************************
// Method: esriShapefileReadMultiPoint
//
// Purpose: 
//   Reads a multipoint.
//
// Arguments:
//   rec  : The record header returned by a previous call to
//          esriShapefileReadRecordHeader.
//   obj  : The object to be populated.
//   code : The return code, indicating success, failure.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:43:02 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriShapefileReadMultiPoint(esriShapefileRecordHeader_t *rec,
    esriMultiPoint_t *obj, esriReadError_t *code)
{
    unsigned char *data = 0;
    ESRI_API_ENTER(esriShapefileReadMultiPoint);

    data = (unsigned char *)esriCommonStorageAlloc(rec->contentLength);
    memset((void*)obj, 0, sizeof(esriMultiPoint_t));

    if(fread((void*)data, 1, rec->contentLength, rec->fileObj->fp) ==
       rec->contentLength)
    {
        int i;
        unsigned char *pts = data + 36;
        obj->xmin = ESRI_DOUBLE_LE(data);
        obj->ymin = ESRI_DOUBLE_LE(data+8);
        obj->xmax = ESRI_DOUBLE_LE(data+16);
        obj->ymax = ESRI_DOUBLE_LE(data+24);
        obj->numPoints = ESRI_INT_LE(data+32);
        
        obj->points = (esriPoint_t *)esriMalloc(obj->numPoints * sizeof(esriPoint_t));
        for(i = 0; i < obj->numPoints; ++i)
        {
            obj->points[i].x = ESRI_DOUBLE_LE(pts);
            obj->points[i].y = ESRI_DOUBLE_LE(pts+8);
            pts += 16;
        }

        *code = esriReadErrorSuccess;
    }
    else
        *code = esriReadErrorFailure;

    esriCommonStorageFree(data);
    ESRI_API_LEAVE(esriShapefileReadMultiPoint);
}

void
esriFreeMultiPoint(esriMultiPoint_t *obj)
{
    ESRI_API_ENTER(esriFreeMultiPoint);
    if(obj != 0)
    {
        if(obj->points != 0) 
            esriFree(obj->points);
        esriFree(obj);
    }
    ESRI_API_LEAVE(esriFreeMultiPoint);
}

void
esriPrintMultiPoint(FILE *fp, esriMultiPoint_t *obj)
{
    int i;
    ESRI_API_ENTER(esriPrintMultiPoint);
    fprintf(fp, "MultiPoint\n");
    fprintf(fp, "\txmin = %g\n", obj->xmin);
    fprintf(fp, "\tymin = %g\n", obj->ymin);
    fprintf(fp, "\txmax = %g\n", obj->xmax);
    fprintf(fp, "\tymax = %g\n", obj->ymax);
    fprintf(fp, "\tnumPoints = %d\n", obj->numPoints);
    fprintf(fp, "\tpoints = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, "\t\t%g %g\n", obj->points[i].x, obj->points[i].y);
    fprintf(fp, "\t}\n");
    ESRI_API_LEAVE(esriPrintMultiPoint);
}

// ****************************************************************************
// Method: esriShapefileReadPolyLine
//
// Purpose: 
//   Reads a polyline.
//
// Arguments:
//   rec  : The record header returned by a previous call to
//          esriShapefileReadRecordHeader.
//   obj  : The object to be populated.
//   code : The return code, indicating success, failure.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:43:02 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriShapefileReadPolyLine(esriShapefileRecordHeader_t *rec,
    esriPolyLine_t *obj, esriReadError_t *code)
{
    unsigned char *data = (unsigned char *)esriCommonStorageAlloc(rec->contentLength);

    ESRI_API_ENTER(esriShapefileReadPolyLine);
    memset((void*)obj, 0, sizeof(esriPolyLine_t));

    if(fread((void*)data, 1, rec->contentLength, rec->fileObj->fp) ==
       rec->contentLength)
    {
        int i;
        unsigned char *partData = data + 40;
        unsigned char *pointData = 0;
        obj->xmin = ESRI_DOUBLE_LE(data);
        obj->ymin = ESRI_DOUBLE_LE(data+8);
        obj->xmax = ESRI_DOUBLE_LE(data+16);
        obj->ymax = ESRI_DOUBLE_LE(data+24);
        obj->numParts = ESRI_INT_LE(data+32);
        obj->numPoints = ESRI_INT_LE(data+36);
        
        obj->parts = (int *)esriMalloc(obj->numParts * sizeof(int));
        for(i = 0; i < obj->numParts; ++i)
        {
            obj->parts[i] = ESRI_INT_LE(partData);
            partData += 4;
        }

        /* Read the points */
        pointData = partData;
        obj->points = (esriPoint_t *)esriMalloc(obj->numPoints * sizeof(esriPoint_t));
        for(i = 0; i < obj->numPoints; ++i)
        {
            obj->points[i].x = ESRI_DOUBLE_LE(pointData);
            obj->points[i].y = ESRI_DOUBLE_LE(pointData+8);
            pointData += 16;
        }

        *code = esriReadErrorSuccess;
    }
    else
        *code = esriReadErrorFailure;

    esriCommonStorageFree(data);
    ESRI_API_LEAVE(esriShapefileReadPolyLine);
}

void
esriFreePolyLine(esriPolyLine_t *obj)
{
    ESRI_API_ENTER(esriFreePolyLine);
    if(obj != 0)
    {
        if(obj->parts != 0) 
            esriFree(obj->parts);
        if(obj->points != 0) 
            esriFree(obj->points);
        esriFree(obj);
    }
    ESRI_API_LEAVE(esriFreePolyLine);
}

void
esriPrintPolyLine(FILE *fp, esriPolyLine_t *obj)
{
    int i;
    ESRI_API_ENTER(esriPrintPolyLine);
    fprintf(fp, "Polyline\n");
    fprintf(fp, "\txmin = %g\n", obj->xmin);
    fprintf(fp, "\tymin = %g\n", obj->ymin);
    fprintf(fp, "\txmax = %g\n", obj->xmax);
    fprintf(fp, "\tymax = %g\n", obj->ymax);
    fprintf(fp, "\tnumParts = %d\n", obj->numParts);
    fprintf(fp, "\tnumPoints = %d\n", obj->numPoints);
    fprintf(fp, "\tparts = {\n\t\t");
    for(i = 0; i < obj->numParts; ++i)
        fprintf(fp, "%d ", obj->parts[i]);
    fprintf(fp, "\n\t}\n");
    fprintf(fp, "\tpoints = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, "\t\t%g %g\n", obj->points[i].x, obj->points[i].y);
    fprintf(fp, "\t}\n");
    ESRI_API_LEAVE(esriPrintPolyLine);
}

// ****************************************************************************
// Method: esriShapefileReadPolygon
//
// Purpose: 
//   Reads a polygon.
//
// Arguments:
//   rec  : The record header returned by a previous call to
//          esriShapefileReadRecordHeader.
//   obj  : The object to be populated.
//   code : The return code, indicating success, failure.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:43:02 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriShapefileReadPolygon(esriShapefileRecordHeader_t *rec,
    esriPolygon_t *obj, esriReadError_t *code)
{
    unsigned char *data = (unsigned char *)esriCommonStorageAlloc(rec->contentLength);

    ESRI_API_ENTER(esriShapefileReadPolygon);
    memset((void*)obj, 0, sizeof(esriPolygon_t));

    if(fread((void*)data, 1, rec->contentLength, rec->fileObj->fp) ==
       rec->contentLength)
    {
        int i;
        unsigned char *partData = data + 40;
        unsigned char *pointData = 0;
        obj->xmin = ESRI_DOUBLE_LE(data);
        obj->ymin = ESRI_DOUBLE_LE(data+8);
        obj->xmax = ESRI_DOUBLE_LE(data+16);
        obj->ymax = ESRI_DOUBLE_LE(data+24);
        obj->numParts = ESRI_INT_LE(data+32);
        obj->numPoints = ESRI_INT_LE(data+36);
        
        obj->parts = (int *)esriMalloc(obj->numParts * sizeof(int));
        for(i = 0; i < obj->numParts; ++i)
        {
            obj->parts[i] = ESRI_INT_LE(partData);
            partData += 4;
        }

        /* Read the points */
        pointData = partData;
        obj->points = (esriPoint_t *)esriMalloc(obj->numPoints * sizeof(esriPoint_t));
        for(i = 0; i < obj->numPoints; ++i)
        {
            obj->points[i].x = ESRI_DOUBLE_LE(pointData);
            obj->points[i].y = ESRI_DOUBLE_LE(pointData+8);
            pointData += 16;
        }

        *code = esriReadErrorSuccess;
    }
    else
        *code = esriReadErrorFailure;

    esriCommonStorageFree(data);
    ESRI_API_LEAVE(esriShapefileReadPolygon);
}

void
esriFreePolygon(esriPolygon_t *obj)
{
    ESRI_API_ENTER(esriFreePolygon);
    if(obj != 0)
    {
        if(obj->parts != 0) 
            esriFree(obj->parts);
        if(obj->points != 0) 
            esriFree(obj->points);
        esriFree(obj);
    }
    ESRI_API_LEAVE(esriFreePolygon);
}

void
esriPrintPolygon(FILE *fp, esriPolygon_t *obj)
{
    int i;
    ESRI_API_ENTER(esriPrintPolygon);
    fprintf(fp, "Polygon\n");
    fprintf(fp, "\txmin = %g\n", obj->xmin);
    fprintf(fp, "\tymin = %g\n", obj->ymin);
    fprintf(fp, "\txmax = %g\n", obj->xmax);
    fprintf(fp, "\tymax = %g\n", obj->ymax);
    fprintf(fp, "\tnumParts = %d\n", obj->numParts);
    fprintf(fp, "\tnumPoints = %d\n", obj->numPoints);
    fprintf(fp, "\tparts = {\n\t\t");
    for(i = 0; i < obj->numParts; ++i)
        fprintf(fp, "%d ", obj->parts[i]);
    fprintf(fp, "\n\t}\n");
    fprintf(fp, "\tpoints = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, "\t\t%g %g\n", obj->points[i].x, obj->points[i].y);
    fprintf(fp, "\t}\n");
    ESRI_API_LEAVE(esriPrintPolygon);
}

// ****************************************************************************
// Method: esriShapefileReadPointM
//
// Purpose: 
//   Reads a pointM.
//
// Arguments:
//   rec  : The record header returned by a previous call to
//          esriShapefileReadRecordHeader.
//   obj  : The object to be populated.
//   code : The return code, indicating success, failure.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:43:02 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriShapefileReadPointM(esriShapefileRecordHeader_t *rec,
    esriPointM_t *obj, esriReadError_t *code)
{
    unsigned char data[24];
    ESRI_API_ENTER(esriShapefileReadPointM);
    if(fread((void*)data, 1, 24, rec->fileObj->fp) == 24)
    {
        obj->x = ESRI_DOUBLE_LE(data);
        obj->y = ESRI_DOUBLE_LE(data+8);
        obj->m = ESRI_DOUBLE_LE(data+16);
        *code = esriReadErrorSuccess;
    }
    else
        *code = esriReadErrorFailure;
    ESRI_API_LEAVE(esriShapefileReadPointM);
}

void
esriFreePointM(esriPointM_t *obj)
{
    ESRI_API_ENTER(esriFreePointM);
    if(obj != 0)
        esriFree((void*)obj);
    ESRI_API_LEAVE(esriFreePointM);
}

void
esriPrintPointM(FILE *fp, esriPointM_t *obj)
{
    ESRI_API_ENTER(esriPrintPointM);
    fprintf(fp, "PointM\n");
    fprintf(fp, "\tx = %g\n", obj->x);
    fprintf(fp, "\ty = %g\n", obj->y);
    fprintf(fp, "\tm = %g\n", obj->m);
    ESRI_API_LEAVE(esriPrintPointM);
}

// ****************************************************************************
// Method: esriShapefileReadMultiPointM
//
// Purpose: 
//   Reads a multipointM
//
// Arguments:
//   rec  : The record header returned by a previous call to
//          esriShapefileReadRecordHeader.
//   obj  : The object to be populated.
//   code : The return code, indicating success, failure.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:43:02 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriShapefileReadMultiPointM(esriShapefileRecordHeader_t *rec,
    esriMultiPointM_t *obj, esriReadError_t *code)
{
    unsigned char *data = 0;
    ESRI_API_ENTER(esriShapefileReadMultiPointM);

    data = (unsigned char *)esriCommonStorageAlloc(rec->contentLength);
    memset((void*)obj, 0, sizeof(esriMultiPointM_t));

    if(fread((void*)data, 1, rec->contentLength, rec->fileObj->fp) ==
       rec->contentLength)
    {
        int i;
        unsigned char *pts = data + 36;
        unsigned char *measures = 0;
        obj->xmin = ESRI_DOUBLE_LE(data);
        obj->ymin = ESRI_DOUBLE_LE(data+8);
        obj->xmax = ESRI_DOUBLE_LE(data+16);
        obj->ymax = ESRI_DOUBLE_LE(data+24);
        obj->numPoints = ESRI_INT_LE(data+32);
        
        obj->points = (esriPoint_t *)esriMalloc(obj->numPoints * sizeof(esriPoint_t));
        for(i = 0; i < obj->numPoints; ++i)
        {
            obj->points[i].x = ESRI_DOUBLE_LE(pts);
            obj->points[i].y = ESRI_DOUBLE_LE(pts+8);
            pts += 16;
        }

        obj->mmin = ESRI_DOUBLE_LE(pts);
        obj->mmax = ESRI_DOUBLE_LE(pts+8);
        measures = pts + 16;
        obj->measures = (double *)esriMalloc(obj->numPoints * sizeof(double));
        if((obj->mmax < obj->mmin) || (ESRI_FABS(obj->mmax) > ESRI_BIG_NUMBER) || (ESRI_FABS(obj->mmin) > ESRI_BIG_NUMBER))
        {
            memset((void *)obj->measures, 0, obj->numPoints * sizeof(double));
            esriError("Invalid mmin, mmax. Using zeros for measures.");
        }
        else
        {
            for(i = 0; i < obj->numPoints; ++i)
            {
                obj->measures[i] = ESRI_DOUBLE_LE(measures);
                measures += 8;
            }
        }

        *code = esriReadErrorSuccess;
    }
    else
        *code = esriReadErrorFailure;

    esriCommonStorageFree(data);
    ESRI_API_LEAVE(esriShapefileReadMultiPointM);
}

void
esriFreeMultiPointM(esriMultiPointM_t *obj)
{
    ESRI_API_ENTER(esriFreeMultiPointM);
    if(obj != 0)
    {
        if(obj->points != 0)
            esriFree(obj->points);
        if(obj->measures != 0)
            esriFree(obj->measures);
        esriFree(obj);
    }
    ESRI_API_LEAVE(esriFreeMultiPointM);
}

void
esriPrintMultiPointM(FILE *fp, esriMultiPointM_t *obj)
{
    int i;
    ESRI_API_ENTER(esriPrintMultiPointM);
    fprintf(fp, "MultiPointM\n");
    fprintf(fp, "\txmin = %g\n", obj->xmin);
    fprintf(fp, "\tymin = %g\n", obj->ymin);
    fprintf(fp, "\txmax = %g\n", obj->xmax);
    fprintf(fp, "\tymax = %g\n", obj->ymax);
    fprintf(fp, "\tnumPoints = %d\n", obj->numPoints);
    fprintf(fp, "\tpoints = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, "\t\t%g %g\n", obj->points[i].x, obj->points[i].y);
    fprintf(fp, "\t}\n");
    fprintf(fp, "\tpoints = {\n");
    fprintf(fp, "\tmmin = %g\n", obj->mmin);
    fprintf(fp, "\tmmax = %g\n", obj->mmax);
    fprintf(fp, "\tmeasures = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, " %g", obj->measures[i]);
    fprintf(fp, "}\n");
    ESRI_API_LEAVE(esriPrintMultiPointM);
}

// ****************************************************************************
// Method: esriShapefileReadPolylineM
//
// Purpose: 
//   Reads a polylineM.
//
// Arguments:
//   rec  : The record header returned by a previous call to
//          esriShapefileReadRecordHeader.
//   obj  : The object to be populated.
//   code : The return code, indicating success, failure.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:43:02 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriShapefileReadPolyLineM(esriShapefileRecordHeader_t *rec,
    esriPolyLineM_t *obj, esriReadError_t *code)
{
    unsigned char *data = (unsigned char *)esriCommonStorageAlloc(rec->contentLength);

    ESRI_API_ENTER(esriShapefileReadPolyLineM);
    memset((void*)obj, 0, sizeof(esriPolyLineM_t));

    if(fread((void*)data, 1, rec->contentLength, rec->fileObj->fp) ==
       rec->contentLength)
    {
        int i;
        unsigned char *partData = data + 40;
        unsigned char *pointData = 0;
        unsigned char *measureData = 0;
        obj->xmin = ESRI_DOUBLE_LE(data);
        obj->ymin = ESRI_DOUBLE_LE(data+8);
        obj->xmax = ESRI_DOUBLE_LE(data+16);
        obj->ymax = ESRI_DOUBLE_LE(data+24);
        obj->numParts = ESRI_INT_LE(data+32);
        obj->numPoints = ESRI_INT_LE(data+36);
        
        /* Read the parts */
        obj->parts = (int *)esriMalloc(obj->numParts * sizeof(int));
        for(i = 0; i < obj->numParts; ++i)
        {
            obj->parts[i] = ESRI_INT_LE(partData);
            partData += 4;
        }

        /* Read the points */
        pointData = partData;
        obj->points = (esriPoint_t *)esriMalloc(obj->numPoints * sizeof(esriPoint_t));
        for(i = 0; i < obj->numPoints; ++i)
        {
            obj->points[i].x = ESRI_DOUBLE_LE(pointData);
            obj->points[i].y = ESRI_DOUBLE_LE(pointData+8);
            pointData += 16;
        }

        /* Read the measures*/
        obj->mmin = ESRI_DOUBLE_LE(pointData);
        obj->mmax = ESRI_DOUBLE_LE(pointData+8);
        measureData = pointData + 16;
        obj->measures = (double *)esriMalloc(obj->numPoints * sizeof(double));
        if((obj->mmax < obj->mmin) || (ESRI_FABS(obj->mmax) > ESRI_BIG_NUMBER) || (ESRI_FABS(obj->mmin) > ESRI_BIG_NUMBER))
        {
            memset((void *)obj->measures, 0, obj->numPoints * sizeof(double));
            esriError("Invalid mmin, mmax. Using zeros for measures.");
        }
        else
        {
            for(i = 0; i < obj->numPoints; ++i)
            {
                obj->measures[i] = ESRI_DOUBLE_LE(measureData);
                measureData += 8;
            }
        }

        *code = esriReadErrorSuccess;
    }
    else
        *code = esriReadErrorFailure;

    esriCommonStorageFree(data);
    ESRI_API_LEAVE(esriShapefileReadPolyLineM);
}

void
esriFreePolyLineM(esriPolyLineM_t *obj)
{
    ESRI_API_ENTER(esriFreePolyLineM);
    if(obj != 0)
    {
        if(obj->parts != 0) 
            esriFree(obj->parts);
        if(obj->points != 0)
            esriFree(obj->points);
        if(obj->measures != 0)
            esriFree(obj->measures);
        esriFree(obj);
    }
    ESRI_API_LEAVE(esriFreePolyLineM);
}

void
esriPrintPolyLineM(FILE *fp, esriPolyLineM_t *obj)
{
    int i;
    ESRI_API_ENTER(esriPrintPolyLineM);
    fprintf(fp, "PolyLineM\n");
    fprintf(fp, "\txmin = %g\n", obj->xmin);
    fprintf(fp, "\tymin = %g\n", obj->ymin);
    fprintf(fp, "\txmax = %g\n", obj->xmax);
    fprintf(fp, "\tymax = %g\n", obj->ymax);
    fprintf(fp, "\tnumParts = %d\n", obj->numParts);
    fprintf(fp, "\tnumPoints = %d\n", obj->numPoints);
    fprintf(fp, "\tparts = {\n\t\t");
    for(i = 0; i < obj->numParts; ++i)
        fprintf(fp, "%d ", obj->parts[i]);
    fprintf(fp, "\n\t}\n");
    fprintf(fp, "\tpoints = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, "\t\t%g %g\n", obj->points[i].x, obj->points[i].y);
    fprintf(fp, "\t}\n");
    fprintf(fp, "\tmmin = %g\n", obj->mmin);
    fprintf(fp, "\tmmax = %g\n", obj->mmax);
    fprintf(fp, "\tmeasures = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, " %g", obj->measures[i]);
    fprintf(fp, "}\n");
    ESRI_API_LEAVE(esriPrintPolyLineM);
}

// ****************************************************************************
// Method: esriShapefileReadPolygonM
//
// Purpose: 
//   Reads a polygonM.
//
// Arguments:
//   rec  : The record header returned by a previous call to
//          esriShapefileReadRecordHeader.
//   obj  : The object to be populated.
//   code : The return code, indicating success, failure.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:43:02 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriShapefileReadPolygonM(esriShapefileRecordHeader_t *rec,
    esriPolygonM_t *obj, esriReadError_t *code)
{
    unsigned char *data = (unsigned char *)esriCommonStorageAlloc(rec->contentLength);

    ESRI_API_ENTER(esriShapefileReadPolygonM);
    memset((void*)obj, 0, sizeof(esriPolygonM_t));

    if(fread((void*)data, 1, rec->contentLength, rec->fileObj->fp) ==
       rec->contentLength)
    {
        int i;
        unsigned char *partData = data + 40;
        unsigned char *pointData = 0;
        unsigned char *measureData = 0;
        obj->xmin = ESRI_DOUBLE_LE(data);
        obj->ymin = ESRI_DOUBLE_LE(data+8);
        obj->xmax = ESRI_DOUBLE_LE(data+16);
        obj->ymax = ESRI_DOUBLE_LE(data+24);
        obj->numParts = ESRI_INT_LE(data+32);
        obj->numPoints = ESRI_INT_LE(data+36);

        /* Read the parts */
        obj->parts = (int *)esriMalloc(obj->numParts * sizeof(int));
        for(i = 0; i < obj->numParts; ++i)
        {
            obj->parts[i] = ESRI_INT_LE(partData);
            partData += 4;
        }

        /* Read the points */
        pointData = partData;
        obj->points = (esriPoint_t *)esriMalloc(obj->numPoints * sizeof(esriPoint_t));
        for(i = 0; i < obj->numPoints; ++i)
        {
            obj->points[i].x = ESRI_DOUBLE_LE(pointData);
            obj->points[i].y = ESRI_DOUBLE_LE(pointData+8);
            pointData += 16;
        }

        /* Read the measures*/
        obj->mmin = ESRI_DOUBLE_LE(pointData);
        obj->mmax = ESRI_DOUBLE_LE(pointData+8);
        measureData = pointData + 16;
        obj->measures = (double *)esriMalloc(obj->numPoints * sizeof(double));
        if((obj->mmax < obj->mmin) || (ESRI_FABS(obj->mmax) > ESRI_BIG_NUMBER) || (ESRI_FABS(obj->mmin) > ESRI_BIG_NUMBER))
        {
            memset((void *)obj->measures, 0, obj->numPoints * sizeof(double));
            esriError("Invalid mmin, mmax. Using zeros for measures.");
        }
        else
        {
            for(i = 0; i < obj->numPoints; ++i)
            {
                obj->measures[i] = ESRI_DOUBLE_LE(measureData);
                measureData += 8;
            }
        }

        *code = esriReadErrorSuccess;
    }
    else
        *code = esriReadErrorFailure;

    esriCommonStorageFree(data);
    ESRI_API_LEAVE(esriShapefileReadPolygonM);
}

void
esriFreePolygonM(esriPolygonM_t *obj)
{
    ESRI_API_ENTER(esriFreePolygonM);
    if(obj != 0)
    {
        if(obj->parts != 0) 
            esriFree(obj->parts);
        if(obj->points != 0) 
            esriFree(obj->points);
        if(obj->measures != 0) 
            esriFree(obj->measures);
        esriFree(obj);
    }
    ESRI_API_LEAVE(esriFreePolygonM);
}

void
esriPrintPolygonM(FILE *fp, esriPolygonM_t *obj)
{
    int i;
    ESRI_API_ENTER(esriPrintPolygonM);
    fprintf(fp, "PolygonM\n");
    fprintf(fp, "\txmin = %g\n", obj->xmin);
    fprintf(fp, "\tymin = %g\n", obj->ymin);
    fprintf(fp, "\txmax = %g\n", obj->xmax);
    fprintf(fp, "\tymax = %g\n", obj->ymax);
    fprintf(fp, "\tnumParts = %d\n", obj->numParts);
    fprintf(fp, "\tnumPoints = %d\n", obj->numPoints);
    fprintf(fp, "\tparts = {\n\t\t");
    for(i = 0; i < obj->numParts; ++i)
        fprintf(fp, "%d ", obj->parts[i]);
    fprintf(fp, "\n\t}\n");
    fprintf(fp, "\tpoints = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, "\t\t%g %g\n", obj->points[i].x, obj->points[i].y);
    fprintf(fp, "\t}\n");
    fprintf(fp, "\tmmin = %g\n", obj->mmin);
    fprintf(fp, "\tmmax = %g\n", obj->mmax);
    fprintf(fp, "\tmeasures = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, " %g", obj->measures[i]);
    fprintf(fp, "}\n");
    ESRI_API_LEAVE(esriPrintPolygonM);
}

// ****************************************************************************
// Method: esriShapefileReadPointZ
//
// Purpose: 
//   Reads a pointZ.
//
// Arguments:
//   rec  : The record header returned by a previous call to
//          esriShapefileReadRecordHeader.
//   obj  : The object to be populated.
//   code : The return code, indicating success, failure.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:43:02 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriShapefileReadPointZ(esriShapefileRecordHeader_t *rec,
    esriPointZ_t *obj, esriReadError_t *code)
{
    unsigned char data[32];
    ESRI_API_ENTER(esriShapefileReadPointZ);
    if(fread((void*)data, 1, 32, rec->fileObj->fp) == 32)
    {
        obj->x = ESRI_DOUBLE_LE(data);
        obj->y = ESRI_DOUBLE_LE(data+8);
        obj->z = ESRI_DOUBLE_LE(data+16);
        obj->m = ESRI_DOUBLE_LE(data+24);
        *code = esriReadErrorSuccess;
    }
    else
        *code = esriReadErrorFailure;
    ESRI_API_LEAVE(esriShapefileReadPointZ);
}

void
esriFreePointZ(esriPointZ_t *obj)
{
    ESRI_API_ENTER(esriFreePointZ);
    if(obj != 0)
        esriFree((void*)obj);
    ESRI_API_LEAVE(esriFreePointZ);
}

void
esriPrintPointZ(FILE *fp, esriPointZ_t *obj)
{
    ESRI_API_ENTER(esriPrintPointZ);
    fprintf(fp, "PointZ\n");
    fprintf(fp, "\tx = %g\n", obj->x);
    fprintf(fp, "\ty = %g\n", obj->y);
    fprintf(fp, "\tz = %g\n", obj->z);
    fprintf(fp, "\tm = %g\n", obj->m);
    ESRI_API_LEAVE(esriPrintPointZ);
}

// ****************************************************************************
// Method: esriShapefileReadMultiPointZ
//
// Purpose: 
//   Reads a multipointZ.
//
// Arguments:
//   rec  : The record header returned by a previous call to
//          esriShapefileReadRecordHeader.
//   obj  : The object to be populated.
//   code : The return code, indicating success, failure.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:43:02 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriShapefileReadMultiPointZ(esriShapefileRecordHeader_t *rec,
    esriMultiPointZ_t *obj, esriReadError_t *code)
{
    unsigned char *data = 0;
    ESRI_API_ENTER(esriShapefileReadMultiPointZ);

    data = (unsigned char *)esriCommonStorageAlloc(rec->contentLength);
    memset((void*)obj, 0, sizeof(esriMultiPointZ_t));

    if(fread((void*)data, 1, rec->contentLength, rec->fileObj->fp) ==
       rec->contentLength)
    {
        int i;
        unsigned char *pts = data + 36;
        unsigned char *z = 0;
        unsigned char *measureData = 0;
        obj->xmin = ESRI_DOUBLE_LE(data);
        obj->ymin = ESRI_DOUBLE_LE(data+8);
        obj->xmax = ESRI_DOUBLE_LE(data+16);
        obj->ymax = ESRI_DOUBLE_LE(data+24);
        obj->numPoints = ESRI_INT_LE(data+32);

        /* Read the points */
        obj->points = (esriPoint_t *)esriMalloc(obj->numPoints * sizeof(esriPoint_t));
        for(i = 0; i < obj->numPoints; ++i)
        {
            obj->points[i].x = ESRI_DOUBLE_LE(pts);
            obj->points[i].y = ESRI_DOUBLE_LE(pts+8);
            pts += 16;
        }

        /* Read the Z values */
        obj->zmin = ESRI_DOUBLE_LE(pts);
        obj->zmax = ESRI_DOUBLE_LE(pts+8);
        obj->z = (double *)esriMalloc(obj->numPoints * sizeof(double));
        z = pts + 16;
        for(i = 0; i < obj->numPoints; ++i)
        {
            obj->z[i] = ESRI_DOUBLE_LE(z);
            z += 8;
        }

        /* Read the measures */
        obj->mmin = ESRI_DOUBLE_LE(z);
        obj->mmax = ESRI_DOUBLE_LE(z+8);
        measureData = z + 16;
        obj->measures = (double *)esriMalloc(obj->numPoints * sizeof(double));
        if((obj->mmax < obj->mmin) || (ESRI_FABS(obj->mmax) > ESRI_BIG_NUMBER) || (ESRI_FABS(obj->mmin) > ESRI_BIG_NUMBER))
        {
            memset((void *)obj->measures, 0, obj->numPoints * sizeof(double));
            esriError("Invalid mmin, mmax. Using zeros for measures.");
        }
        else
        {
            for(i = 0; i < obj->numPoints; ++i)
            {
                obj->measures[i] = ESRI_DOUBLE_LE(measureData);
                measureData += 8;
            }
        }

        *code = esriReadErrorSuccess;
    }
    else
        *code = esriReadErrorFailure;

    esriCommonStorageFree(data);
    ESRI_API_LEAVE(esriShapefileReadMultiPointZ);
}

void
esriFreeMultiPointZ(esriMultiPointZ_t *obj)
{
    ESRI_API_ENTER(esriFreeMultiPointZ);
    if(obj != 0)
    {
        if(obj->points != 0)
            esriFree(obj->points);
        if(obj->z != 0)
            esriFree(obj->z);
        if(obj->measures != 0)
            esriFree(obj->measures);
        esriFree(obj);
    }
    ESRI_API_LEAVE(esriFreeMultiPointZ);
}

void
esriPrintMultiPointZ(FILE *fp, esriMultiPointZ_t *obj)
{
    int i;
    ESRI_API_ENTER(esriPrintMultiPointZ);
    fprintf(fp, "MultiPointZ\n");
    fprintf(fp, "\txmin = %g\n", obj->xmin);
    fprintf(fp, "\tymin = %g\n", obj->ymin);
    fprintf(fp, "\txmax = %g\n", obj->xmax);
    fprintf(fp, "\tymax = %g\n", obj->ymax);
    fprintf(fp, "\tnumPoints = %d\n", obj->numPoints);
    fprintf(fp, "\tpoints = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, "\t\t%g %g\n", obj->points[i].x, obj->points[i].y);
    fprintf(fp, "\t}\n");
    fprintf(fp, "\tzmin = %g\n", obj->zmin);
    fprintf(fp, "\tzmax = %g\n", obj->zmax);
    fprintf(fp, "\tz = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, " %g", obj->z[i]);
    fprintf(fp, "}\n");
    fprintf(fp, "\tmmin = %g\n", obj->mmin);
    fprintf(fp, "\tmmax = %g\n", obj->mmax);
    fprintf(fp, "\tmeasures = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, " %g", obj->measures[i]);
    fprintf(fp, "}\n");
    ESRI_API_LEAVE(esriPrintMultiPointZ);
}

// ****************************************************************************
// Method: esriShapefileReadPolyLineZ
//
// Purpose: 
//   Reads a polylineZ.
//
// Arguments:
//   rec  : The record header returned by a previous call to
//          esriShapefileReadRecordHeader.
//   obj  : The object to be populated.
//   code : The return code, indicating success, failure.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:43:02 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriShapefileReadPolyLineZ(esriShapefileRecordHeader_t *rec,
    esriPolyLineZ_t *obj, esriReadError_t *code)
{
    unsigned char *data = (unsigned char *)esriCommonStorageAlloc(rec->contentLength);

    ESRI_API_ENTER(esriShapefileReadPolyLineZ);
    memset((void*)obj, 0, sizeof(esriPolyLineZ_t));

    if(fread((void*)data, 1, rec->contentLength, rec->fileObj->fp) ==
       rec->contentLength)
    {
        int i;
        unsigned char *partData = data + 40;
        unsigned char *pointData = 0;
        unsigned char *z = 0;
        unsigned char *measureData = 0;
        obj->xmin = ESRI_DOUBLE_LE(data);
        obj->ymin = ESRI_DOUBLE_LE(data+8);
        obj->xmax = ESRI_DOUBLE_LE(data+16);
        obj->ymax = ESRI_DOUBLE_LE(data+24);
        obj->numParts = ESRI_INT_LE(data+32);
        obj->numPoints = ESRI_INT_LE(data+36);
        
        /* Read the parts */
        obj->parts = (int *)esriMalloc(obj->numParts * sizeof(int));
        for(i = 0; i < obj->numParts; ++i)
        {
            obj->parts[i] = ESRI_INT_LE(partData);
            partData += 4;
        }

        /* Read the points */
        pointData = partData;
        obj->points = (esriPoint_t *)esriMalloc(obj->numPoints * sizeof(esriPoint_t));
        for(i = 0; i < obj->numPoints; ++i)
        {
            obj->points[i].x = ESRI_DOUBLE_LE(pointData);
            obj->points[i].y = ESRI_DOUBLE_LE(pointData+8);
            pointData += 16;
        }

        /* Read the z values*/
        obj->zmin = ESRI_DOUBLE_LE(pointData);
        obj->zmax = ESRI_DOUBLE_LE(pointData+8);
        z = pointData + 16;
        obj->z = (double *)esriMalloc(obj->numPoints * sizeof(double));
        for(i = 0; i < obj->numPoints; ++i)
        {
            obj->z[i] = ESRI_DOUBLE_LE(z);
            z += 8;
        }

        /* Read the measures*/
        obj->mmin = ESRI_DOUBLE_LE(z);
        obj->mmax = ESRI_DOUBLE_LE(z+8);
        measureData = z + 16;
        obj->measures = (double *)esriMalloc(obj->numPoints * sizeof(double));
        if((obj->mmax < obj->mmin) || (ESRI_FABS(obj->mmax) > ESRI_BIG_NUMBER) || (ESRI_FABS(obj->mmin) > ESRI_BIG_NUMBER))
        {
            memset((void *)obj->measures, 0, obj->numPoints * sizeof(double));
            esriError("Invalid mmin, mmax. Using zeros for measures.");
        }
        else
        {
            for(i = 0; i < obj->numPoints; ++i)
            {
                obj->measures[i] = ESRI_DOUBLE_LE(measureData);
                measureData += 8;
            }
        }

        *code = esriReadErrorSuccess;
    }
    else
        *code = esriReadErrorFailure;

    esriCommonStorageFree(data);
    ESRI_API_LEAVE(esriShapefileReadPolyLineZ);
}

void
esriFreePolyLineZ(esriPolyLineZ_t *obj)
{
    ESRI_API_ENTER(esriFreePolyLineZ);
    if(obj != 0)
    {
        if(obj->parts != 0) 
            esriFree(obj->parts);
        if(obj->points != 0)
            esriFree(obj->points);
        if(obj->z != 0)
            esriFree(obj->z);
        if(obj->measures != 0)
            esriFree(obj->measures);
        esriFree(obj);
    }
    ESRI_API_LEAVE(esriFreePolyLineZ);
}

void
esriPrintPolyLineZ(FILE *fp, esriPolyLineZ_t *obj)
{
    int i;
    ESRI_API_ENTER(esriPrintPolyLineZ);
    fprintf(fp, "PolyLineZ\n");
    fprintf(fp, "\txmin = %g\n", obj->xmin);
    fprintf(fp, "\tymin = %g\n", obj->ymin);
    fprintf(fp, "\txmax = %g\n", obj->xmax);
    fprintf(fp, "\tymax = %g\n", obj->ymax);
    fprintf(fp, "\tnumParts = %d\n", obj->numParts);
    fprintf(fp, "\tnumPoints = %d\n", obj->numPoints);
    fprintf(fp, "\tparts = {\n\t\t");
    for(i = 0; i < obj->numParts; ++i)
        fprintf(fp, "%d ", obj->parts[i]);
    fprintf(fp, "\n\t}\n");
    fprintf(fp, "\tpoints = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, "\t\t%g %g\n", obj->points[i].x, obj->points[i].y);
    fprintf(fp, "\t}\n");
    fprintf(fp, "\tzmin = %g\n", obj->zmin);
    fprintf(fp, "\tzmax = %g\n", obj->zmax);
    fprintf(fp, "\tz = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, " %g", obj->z[i]);
    fprintf(fp, "}\n");
    fprintf(fp, "\tmmin = %g\n", obj->mmin);
    fprintf(fp, "\tmmax = %g\n", obj->mmax);
    fprintf(fp, "\tmeasures = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, " %g", obj->measures[i]);
    fprintf(fp, "}\n");
    ESRI_API_LEAVE(esriPrintPolyLineZ);
}

// ****************************************************************************
// Method: esriShapefileReadPolygonZ
//
// Purpose: 
//   Reads a polygonZ.
//
// Arguments:
//   rec  : The record header returned by a previous call to
//          esriShapefileReadRecordHeader.
//   obj  : The object to be populated.
//   code : The return code, indicating success, failure.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:43:02 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriShapefileReadPolygonZ(esriShapefileRecordHeader_t *rec,
    esriPolygonZ_t *obj, esriReadError_t *code)
{
    unsigned char *data = (unsigned char *)esriCommonStorageAlloc(rec->contentLength);

    ESRI_API_ENTER(esriShapefileReadPolygonZ);
    memset((void*)obj, 0, sizeof(esriPolygonZ_t));

    if(fread((void*)data, 1, rec->contentLength, rec->fileObj->fp) ==
       rec->contentLength)
    {
        int i;
        unsigned char *partData = data + 40;
        unsigned char *pointData = 0;
        unsigned char *z = 0;
        unsigned char *measureData = 0;
        obj->xmin = ESRI_DOUBLE_LE(data);
        obj->ymin = ESRI_DOUBLE_LE(data+8);
        obj->xmax = ESRI_DOUBLE_LE(data+16);
        obj->ymax = ESRI_DOUBLE_LE(data+24);
        obj->numParts = ESRI_INT_LE(data+32);
        obj->numPoints = ESRI_INT_LE(data+36);

        /* Read the parts */
        obj->parts = (int *)esriMalloc(obj->numParts * sizeof(int));
        for(i = 0; i < obj->numParts; ++i)
        {
            obj->parts[i] = ESRI_INT_LE(partData);
            partData += 4;
        }

        /* Read the points */
        pointData = partData;
        obj->points = (esriPoint_t *)esriMalloc(obj->numPoints * sizeof(esriPoint_t));
        for(i = 0; i < obj->numPoints; ++i)
        {
            obj->points[i].x = ESRI_DOUBLE_LE(pointData);
            obj->points[i].y = ESRI_DOUBLE_LE(pointData+8);
            pointData += 16;
        }

        /* Read the z values*/
        obj->zmin = ESRI_DOUBLE_LE(pointData);
        obj->zmax = ESRI_DOUBLE_LE(pointData+8);
        z = pointData + 16;
        obj->z = (double *)esriMalloc(obj->numPoints * sizeof(double));
        for(i = 0; i < obj->numPoints; ++i)
        {
            obj->z[i] = ESRI_DOUBLE_LE(z);
            z += 8;
        }

        /* Read the measures*/
        obj->mmin = ESRI_DOUBLE_LE(z);
        obj->mmax = ESRI_DOUBLE_LE(z+8);
        measureData = z + 16;
        obj->measures = (double *)esriMalloc(obj->numPoints * sizeof(double));
        if((obj->mmax < obj->mmin) || (ESRI_FABS(obj->mmax) > ESRI_BIG_NUMBER) || (ESRI_FABS(obj->mmin) > ESRI_BIG_NUMBER))
        {
            memset((void *)obj->measures, 0, obj->numPoints * sizeof(double));
            esriError("Invalid mmin, mmax. Using zeros for measures.");
        }
        else
        {
            for(i = 0; i < obj->numPoints; ++i)
            {
                obj->measures[i] = ESRI_DOUBLE_LE(measureData);
                measureData += 8;
            }
        }

        *code = esriReadErrorSuccess;
    }
    else
        *code = esriReadErrorFailure;

    esriCommonStorageFree(data);
    ESRI_API_LEAVE(esriShapefileReadPolygonZ);
}

void
esriFreePolygonZ(esriPolygonZ_t *obj)
{
    ESRI_API_ENTER(esriFreePolygonZ);
    if(obj != 0)
    {
        if(obj->parts != 0) 
            esriFree(obj->parts);
        if(obj->points != 0) 
            esriFree(obj->points);
        if(obj->z != 0) 
            esriFree(obj->z);
        if(obj->measures != 0) 
            esriFree(obj->measures);
        esriFree(obj);
    }
    ESRI_API_LEAVE(esriFreePolygonZ);
}

void
esriPrintPolygonZ(FILE *fp, esriPolygonZ_t *obj)
{
    int i;
    ESRI_API_ENTER(esriPrintPolygonZ);
    fprintf(fp, "PolygonZ\n");
    fprintf(fp, "\txmin = %g\n", obj->xmin);
    fprintf(fp, "\tymin = %g\n", obj->ymin);
    fprintf(fp, "\txmax = %g\n", obj->xmax);
    fprintf(fp, "\tymax = %g\n", obj->ymax);
    fprintf(fp, "\tnumParts = %d\n", obj->numParts);
    fprintf(fp, "\tnumPoints = %d\n", obj->numPoints);
    fprintf(fp, "\tparts = {\n\t\t");
    for(i = 0; i < obj->numParts; ++i)
        fprintf(fp, "%d ", obj->parts[i]);
    fprintf(fp, "\n\t}\n");
    fprintf(fp, "\tpoints = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, "\t\t%g %g\n", obj->points[i].x, obj->points[i].y);
    fprintf(fp, "\t}\n");
    fprintf(fp, "\tzmin = %g\n", obj->zmin);
    fprintf(fp, "\tzmax = %g\n", obj->zmax);
    fprintf(fp, "\tz = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, " %g", obj->z[i]);
    fprintf(fp, "}\n");
    fprintf(fp, "\tmmin = %g\n", obj->mmin);
    fprintf(fp, "\tmmax = %g\n", obj->mmax);
    fprintf(fp, "\tmeasures = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, " %g", obj->measures[i]);
    fprintf(fp, "}\n");
    ESRI_API_LEAVE(esriPrintPolygonZ);
}

// ****************************************************************************
// Method: esriShapefileReadMultiPatch.
//
// Purpose: 
//   Reads a multipatch.
//
// Arguments:
//   rec  : The record header returned by a previous call to
//          esriShapefileReadRecordHeader.
//   obj  : The object to be populated.
//   code : The return code, indicating success, failure.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 29 14:43:02 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
esriShapefileReadMultiPatch(esriShapefileRecordHeader_t *rec,
    esriMultiPatch_t *obj, esriReadError_t *code)
{
    unsigned char *data = (unsigned char *)esriCommonStorageAlloc(rec->contentLength);

    ESRI_API_ENTER(esriShapefileReadMultiPatch);
    memset((void*)obj, 0, sizeof(esriMultiPatch_t));

    if(fread((void*)data, 1, rec->contentLength, rec->fileObj->fp) ==
       rec->contentLength)
    {
        int i;
        unsigned char *partData = data + 40;
        unsigned char *partTypeData = 0;
        unsigned char *pointData = 0;
        unsigned char *z = 0;
        unsigned char *measureData = 0;
        obj->xmin = ESRI_DOUBLE_LE(data);
        obj->ymin = ESRI_DOUBLE_LE(data+8);
        obj->xmax = ESRI_DOUBLE_LE(data+16);
        obj->ymax = ESRI_DOUBLE_LE(data+24);
        obj->numParts = ESRI_INT_LE(data+32);
        obj->numPoints = ESRI_INT_LE(data+36);

        /* Read the parts */
        obj->parts = (int *)esriMalloc(obj->numParts * sizeof(int));
        for(i = 0; i < obj->numParts; ++i)
        {
            obj->parts[i] = ESRI_INT_LE(partData);
            partData += 4;
        }

        /* Read the part types */
        partTypeData = partData;
        obj->partTypes = (int *)esriMalloc(obj->numParts * sizeof(int));
        for(i = 0; i < obj->numParts; ++i)
        {
            obj->partTypes[i] = ESRI_INT_LE(partTypeData);
            partTypeData += 4;
        }

        /* Read the points */
        pointData = partTypeData;
        obj->points = (esriPoint_t *)esriMalloc(obj->numPoints * sizeof(esriPoint_t));
        for(i = 0; i < obj->numPoints; ++i)
        {
            obj->points[i].x = ESRI_DOUBLE_LE(pointData);
            obj->points[i].y = ESRI_DOUBLE_LE(pointData+8);
            pointData += 16;
        }

        /* Read the z values*/
        obj->zmin = ESRI_DOUBLE_LE(pointData);
        obj->zmax = ESRI_DOUBLE_LE(pointData+8);
        z = pointData + 16;
        obj->z = (double *)esriMalloc(obj->numPoints * sizeof(double));
        for(i = 0; i < obj->numPoints; ++i)
        {
            obj->z[i] = ESRI_DOUBLE_LE(z);
            z += 8;
        }

        /* Read the measures*/
        obj->mmin = ESRI_DOUBLE_LE(z);
        obj->mmax = ESRI_DOUBLE_LE(z+8);
        measureData = z + 16;
        obj->measures = (double *)esriMalloc(obj->numPoints * sizeof(double));
        if((obj->mmax < obj->mmin) || (ESRI_FABS(obj->mmax) > ESRI_BIG_NUMBER) || (ESRI_FABS(obj->mmin) > ESRI_BIG_NUMBER))
        {
            memset((void *)obj->measures, 0, obj->numPoints * sizeof(double));
            esriError("Invalid mmin, mmax. Using zeros for measures.");
        }
        else
        {
            for(i = 0; i < obj->numPoints; ++i)
            {
                obj->measures[i] = ESRI_DOUBLE_LE(measureData);
                measureData += 8;
            }
        }

        *code = esriReadErrorSuccess;
    }
    else
        *code = esriReadErrorFailure;

    esriCommonStorageFree(data);
    ESRI_API_LEAVE(esriShapefileReadMultiPatch);
}

void
esriFreeMultiPatch(esriMultiPatch_t *obj)
{
    ESRI_API_ENTER(esriFreeMultiPatch);
    if(obj != 0)
    {
        if(obj->parts != 0) 
            esriFree(obj->parts);
        if(obj->partTypes != 0) 
            esriFree(obj->partTypes);
        if(obj->points != 0) 
            esriFree(obj->points);
        if(obj->z != 0) 
            esriFree(obj->z);
        if(obj->measures != 0) 
            esriFree(obj->measures);
        esriFree(obj);
    }
    ESRI_API_LEAVE(esriFreeMultiPatch);
}

void
esriPrintMultiPatch(FILE *fp, esriMultiPatch_t *obj)
{
    int i;
    ESRI_API_ENTER(esriPrintMultiPatch);
    fprintf(fp, "MultiPatch\n");
    fprintf(fp, "\txmin = %g\n", obj->xmin);
    fprintf(fp, "\tymin = %g\n", obj->ymin);
    fprintf(fp, "\txmax = %g\n", obj->xmax);
    fprintf(fp, "\tymax = %g\n", obj->ymax);
    fprintf(fp, "\tnumParts = %d\n", obj->numParts);
    fprintf(fp, "\tnumPoints = %d\n", obj->numPoints);
    fprintf(fp, "\tparts = {\n\t\t");
    for(i = 0; i < obj->numParts; ++i)
        fprintf(fp, "%d ", obj->parts[i]);
    fprintf(fp, "\n\t}\n");
    fprintf(fp, "\tpartTypes = {\n\t\t");
    for(i = 0; i < obj->numParts; ++i)
        fprintf(fp, "%d ", obj->partTypes[i]);
    fprintf(fp, "\n\t}\n");
    fprintf(fp, "\tpoints = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, "\t\t%g %g\n", obj->points[i].x, obj->points[i].y);
    fprintf(fp, "\t}\n");
    fprintf(fp, "\tzmin = %g\n", obj->zmin);
    fprintf(fp, "\tzmax = %g\n", obj->zmax);
    fprintf(fp, "\tz = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, " %g", obj->z[i]);
    fprintf(fp, "}\n");
    fprintf(fp, "\tmmin = %g\n", obj->mmin);
    fprintf(fp, "\tmmax = %g\n", obj->mmax);
    fprintf(fp, "\tmeasures = {\n");
    for(i = 0; i < obj->numPoints; ++i)
        fprintf(fp, " %g", obj->measures[i]);
    fprintf(fp, "}\n");
    ESRI_API_LEAVE(esriPrintMultiPatch);
}
