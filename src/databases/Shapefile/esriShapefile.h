#ifndef ESRI_SHAPEFILE_H
#define ESRI_SHAPEFILE_H
#include <stdio.h>

#ifdef  __cplusplus
extern "C" {
#endif

// ****************************************************************************
// File: esriShapefile.h
//
// Purpose:
//   This header file contains functions to read geometry from ESRI shapefiles
//   into structures that can be used for other purposes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 25 14:25:25 PST 2005
//
// Modifications:
//   
// ****************************************************************************

typedef enum {
    esriNullShape = 0,
    esriPoint = 1,
    esriPolyLine = 3,
    esriPolygon = 5,
    esriMultiPoint = 8,
    esriPointZ = 11,
    esriPolyLineZ = 13,
    esriPolygonZ = 15,
    esriMultiPointZ = 18,
    esriPointM = 21,
    esriPolyLineM = 23,
    esriPolygonM = 25,
    esriMultiPointM = 28,
    esriMultiPatch = 31
} esriShapeType_t;
    
typedef struct
{
    int             file_code;
    int             unused0;
    int             unused1;
    int             unused2;
    int             unused3;
    int             unused4;
    int             file_length;
    int             version;
    esriShapeType_t shape_type;
    double          xmin;
    double          xmax;
    double          ymin;
    double          ymax;
    double          zmin;
    double          zmax;
    double          mmin;
    double          mmax;
} esriShapefileHeader_t;

typedef struct
{
    FILE                  *fp;
    char                  *filename;
    esriShapefileHeader_t  header;
} esriShapefile_t;

typedef struct
{
    /* To simplify the calls to read the data. */
    esriShapefile_t       *fileObj;
    int                    recordNumber;
    /* Number of 16 bit words in the record content. */
    size_t                 contentLength;
    esriShapeType_t        shapeType;
} esriShapefileRecordHeader_t;

typedef enum
{
    esriFileErrorSuccess,
    esriFileErrorInvalidFile,
    esriFileErrorNotFound
} esriFileError_t;

typedef enum
{
    esriReadErrorSuccess,
    esriReadErrorFailure
} esriReadError_t;

/* Memory allocation, deallocation routines. */
#define esriMalloc(S)      esriMallocEx(__FILE__,__LINE__, #S,S)
#define esriFree(P)        esriFreeEx(__FILE__,__LINE__, #P, P)
#define esriFreeShape(T,P) esriFreeShapeEx(__FILE__,__LINE__, #T","#P, T, P)
void *esriMallocEx(const char *f, const int line, const char *src, size_t s);
void esriFreeEx(const char *f, const int line, const char *src, void *ptr);
void esriFreeShapeEx(const char *f, const int line, const char *src,
                     esriShapeType_t, void *ptr);


void esriInitialize(int commonStorage, void (*errFunc)(const char *));
void esriFinalize(void);
esriShapefile_t *esriShapefileOpen(const char *filename, esriFileError_t *code);
void esriShapefileClose(esriShapefile_t *);

void esriShapefileReadRecordHeader(esriShapefile_t *file,
                                   esriShapefileRecordHeader_t *rec,
                                   esriReadError_t *code);

typedef struct
{
    double          x;
    double          y;
} esriPoint_t;

void esriShapefileReadPoint(esriShapefileRecordHeader_t *rec,
                            esriPoint_t *,
                            esriReadError_t *code);
void esriPrintPoint(FILE *fp, esriPoint_t *mp);
void esriFreePoint(esriPoint_t *);


typedef struct
{
    double       xmin;      /* Bounding box */
    double       ymin;
    double       xmax;
    double       ymax;
    int          numPoints; /* Number of points */
    esriPoint_t *points;    /* The points in the set */
} esriMultiPoint_t;

void esriShapefileReadMultiPoint(esriShapefileRecordHeader_t *rec,
                                 esriMultiPoint_t *,
                                 esriReadError_t *code);
void esriFreeMultiPoint(esriMultiPoint_t *);
void esriPrintMultiPoint(FILE *fp, esriMultiPoint_t *obj);


typedef struct
{
    double       xmin;      /* Bounding box */
    double       ymin;
    double       xmax;
    double       ymax;
    int          numParts;  /* Number of parts. */
    int          numPoints; /* Number of points. */
    int         *parts;     /* The start of the part in the points array. */
    esriPoint_t *points;    /* The points in the set */
} esriPolyLine_t;

void esriShapefileReadPolyLine(esriShapefileRecordHeader_t *rec,
                               esriPolyLine_t *,
                               esriReadError_t *code);
void esriFreePolyLine(esriPolyLine_t *);
void esriPrintPolyLine(FILE *fp, esriPolyLine_t *obj);


typedef struct
{
    double       xmin;      /* Bounding box */
    double       ymin;
    double       xmax;
    double       ymax;
    int          numParts;  /* Number of parts. */
    int          numPoints; /* Number of points. */
    int         *parts;     /* The start of the part in the points array. */
    esriPoint_t *points;    /* The points in the set */
} esriPolygon_t;

void esriShapefileReadPolygon(esriShapefileRecordHeader_t *rec,
                               esriPolygon_t *,
                               esriReadError_t *code);
void esriFreePolygon(esriPolygon_t *);
void esriPrintPolygon(FILE *fp, esriPolygon_t *obj);


typedef struct
{
    double          x;
    double          y;
    double          m;
} esriPointM_t;

void esriShapefileReadPointM(esriShapefileRecordHeader_t *rec,
                             esriPointM_t *,
                             esriReadError_t *code);
void esriPrintPointM(FILE *fp, esriPointM_t *mp);
void esriFreePointM(esriPointM_t *);


typedef struct
{
    double       xmin;       /* Bounding box */
    double       ymin;
    double       xmax;
    double       ymax;
    int          numPoints;  /* Number of Points */
    esriPoint_t *points;     /* The Points in the set */
    double       mmin;
    double       mmax;
    double      *measures;
} esriMultiPointM_t;

void esriShapefileReadMultiPointM(esriShapefileRecordHeader_t *rec,
                                  esriMultiPointM_t *,
                                  esriReadError_t *code);
void esriFreeMultiPointM(esriMultiPointM_t *);
void esriPrintMultiPointM(FILE *fp, esriMultiPointM_t *obj);


typedef struct
{
    double       xmin;      /* Bounding box */
    double       ymin;
    double       xmax;
    double       ymax;
    int          numParts;  /* Number of parts. */
    int          numPoints; /* Number of points. */
    int         *parts;     /* The start of the part in the points array. */
    esriPoint_t *points;    /* The points in the set */
    double       mmin;
    double       mmax;
    double      *measures;  /* Measures for all points. */
} esriPolyLineM_t;

void esriShapefileReadPolyLineM(esriShapefileRecordHeader_t *rec,
                               esriPolyLineM_t *,
                               esriReadError_t *code);
void esriFreePolyLineM(esriPolyLineM_t *);
void esriPrintPolyLineM(FILE *fp, esriPolyLineM_t *obj);


typedef struct
{
    double       xmin;      /* Bounding box */
    double       ymin;
    double       xmax;
    double       ymax;
    int          numParts;  /* Number of parts. */
    int          numPoints; /* Number of points. */
    int         *parts;     /* The start of the part in the points array. */
    esriPoint_t *points;    /* The points in the set */
    double       mmin;
    double       mmax;
    double      *measures;  /* Measures for all points. */
} esriPolygonM_t;

void esriShapefileReadPolygonM(esriShapefileRecordHeader_t *rec,
                               esriPolygonM_t *,
                               esriReadError_t *code);
void esriFreePolygonM(esriPolygonM_t *);
void esriPrintPolygonM(FILE *fp, esriPolygonM_t *obj);


typedef struct
{
    double          x;
    double          y;
    double          z;
    double          m;
} esriPointZ_t;

void esriShapefileReadPointZ(esriShapefileRecordHeader_t *rec,
                             esriPointZ_t *,
                             esriReadError_t *code);
void esriPrintPointZ(FILE *fp, esriPointZ_t *mp);
void esriFreePointZ(esriPointZ_t *);


typedef struct
{
    double       xmin;       /* Bounding box */
    double       ymin;
    double       xmax;
    double       ymax;
    int          numPoints;  /* Number of parts. */
    esriPoint_t *points;     /* The Points in the set */
    double       zmin;
    double       zmax;
    double      *z;          /* The Z component of the coordinates */
    double       mmin;
    double       mmax;
    double      *measures;   /* Measures for all points. */
} esriMultiPointZ_t;

void esriShapefileReadMultiPointZ(esriShapefileRecordHeader_t *rec,
                                  esriMultiPointZ_t *,
                                  esriReadError_t *code);
void esriFreeMultiPointZ(esriMultiPointZ_t *);
void esriPrintMultiPointZ(FILE *fp, esriMultiPointZ_t *obj);


typedef struct
{
    double       xmin;      /* Bounding box */
    double       ymin;
    double       xmax;
    double       ymax;
    int          numParts;  /* Number of parts. */
    int          numPoints; /* Number of points. */
    int         *parts;     /* The start of the part in the points array. */
    esriPoint_t *points;    /* The points in the set */
    double       zmin;
    double       zmax;
    double      *z;         /* The Z component of the coordinates */
    double       mmin;
    double       mmax;
    double      *measures;  /* Measures for all points. */
} esriPolyLineZ_t;

void esriShapefileReadPolyLineZ(esriShapefileRecordHeader_t *rec,
                               esriPolyLineZ_t *,
                               esriReadError_t *code);
void esriFreePolyLineZ(esriPolyLineZ_t *);
void esriPrintPolyLineZ(FILE *fp, esriPolyLineZ_t *obj);


typedef struct
{
    double       xmin;      /* Bounding box */
    double       ymin;
    double       xmax;
    double       ymax;
    int          numParts;  /* Number of parts. */
    int          numPoints; /* Number of points. */
    int         *parts;     /* The start of the part in the points array. */
    esriPoint_t *points;    /* The points in the set */
    double       zmin;
    double       zmax;
    double      *z;         /* The Z component of the coordinates. */
    double       mmin;
    double       mmax;
    double      *measures;  /* Measures for all points. */
} esriPolygonZ_t;

void esriShapefileReadPolygonZ(esriShapefileRecordHeader_t *rec,
                               esriPolygonZ_t *,
                               esriReadError_t *code);
void esriFreePolygonZ(esriPolygonZ_t *);
void esriPrintPolygonZ(FILE *fp, esriPolygonZ_t *obj);


typedef struct
{
    double       xmin;      /* Bounding box */
    double       ymin;
    double       xmax;
    double       ymax;
    int          numParts;  /* Number of parts. */
    int          numPoints; /* Number of points. */
    int         *parts;     /* The start of the part in the points array. */
    int         *partTypes; /* Number of points */
    esriPoint_t *points;    /* The points in the set */
    double       zmin;
    double       zmax;
    double      *z;         /* The Z component of the coordinates. */
    double       mmin;
    double       mmax;
    double      *measures;  /* Measures for all points. */
} esriMultiPatch_t;

void esriShapefileReadMultiPatch(esriShapefileRecordHeader_t *rec,
                                 esriMultiPatch_t *,
                                 esriReadError_t *code);
void esriFreeMultiPatch(esriMultiPatch_t *);
void esriPrintMultiPatch(FILE *fp, esriMultiPatch_t *obj);

#ifdef  __cplusplus
}
#endif

#endif
