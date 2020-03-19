// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtBOVFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_BOV_FILE_FORMAT_H
#define AVT_BOV_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtBOVFileFormat
//
//  Purpose:
//      Reads "brick of float" files.
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Mar  4 16:02:03 PST 2005
//    Added support for deferring reading in all of the TOC.
//
//    Brad Whitlock, Thu Mar 16 14:54:01 PST 2006
//    Added byte offset so we can skip past a header. I also added a new
//    flag for dividing a brick into bricklets.
//
//    Brad Whitlock, Thu May 4 09:36:53 PDT 2006
//    Added support for double and int.
//
//    Hank Childs, Thu Apr 24 13:29:41 PDT 2008
//    Change char *'s to strings and float *'s to vector<double>.
//
//    Jeremy Meredith, Thu Jul 24 14:55:41 EDT 2008
//    Convert most int's to long longs to support >4GB files.
//
//    Brad Whitlock, Wed Apr  8 09:40:02 PDT 2009
//    I added short int support.
//
//    Hank Childs, Sat Apr 24 18:21:42 PDT 2010
//    Add proper support for time.
//
//    Hank Childs, Tue Nov 30 11:41:44 PST 2010
//    Only overset the time if we know if it is accurate.
//
//    Hank Childs, Tue Nov 30 13:43:49 PST 2010
//    Return INVALID_CYCLE and INVALID_TIME if we don't know them.  This 
//    prevents oversetting elsewhere.
//
//    David Camp, Mon Aug 22 12:59:31 PDT 2011
//    Added the ReadTOC function to the GetCycle and GetTime methods.
//    Need this for pathlines.
//
//    Alister Maguire, Thu Sep  7 09:02:03 PDT 2017
//    Added DBOptionsAttributes as a constructor argument. 
//
// ****************************************************************************

class avtBOVFileFormat : public avtSTMDFileFormat
{
  public:
                               avtBOVFileFormat(const char *, 
                                   DBOptionsAttributes *);
    virtual                   ~avtBOVFileFormat();

    virtual const char        *GetType(void) { return "Brick of values"; };

    virtual vtkDataSet        *GetMesh(int, const char *);
    virtual vtkDataArray      *GetVar(int, const char *);
    virtual vtkDataArray      *GetVectorVar(int, const char *);
    virtual void              *GetAuxiliaryData(const char *var, int,
                                                const char *type, void *args,
                                                DestructorFunction &);

    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual int                GetCycle(void) { ReadTOC(); return (cycleIsAccurate ? cycle : INVALID_CYCLE); };
    virtual double             GetTime(void) { ReadTOC(); return (timeIsAccurate ? dtime : INVALID_TIME); };
    void                       ActivateTimestep(void);

  protected:
    typedef enum {ByteData, ShortData, IntegerData, FloatData, DoubleData} DataFormatEnum;

    void                       ReadTOC(void);
    void                       ReadWholeAndExtractBrick(void *dest, bool gzipped,
                                  void *file_handle, void *gz_handle,
                                  unsigned long long unit_size,
                                  unsigned long long x_start,
                                  unsigned long long x_stop, 
                                  unsigned long long y_start,
                                  unsigned long long y_stop, 
                                  unsigned long long z_start,
                                  unsigned long long z_stop, 
                                  unsigned long long dx, unsigned long long dy,
                                  unsigned long long whole_size);

    bool                       haveReadTOC;
    char                      *path;
    std::string                file_pattern;
    int                        cycle;
    bool                       cycleIsAccurate;
    double                     dtime;
    bool                       timeIsAccurate;
    long long                  full_size[3];
    long long                  bricklet_size[3];
    long long                  byteOffset;
    std::string                varname;
    bool                       hasBoundaries;
    bool                       declaredEndianess;
    bool                       littleEndian;
    bool                       nodalCentering;
    bool                       divideBrick;
    bool                       byteToFloatTransform;
    DataFormatEnum             dataFormat;
    long long                  dataNumComponents;
    double                     min, max;
    double                     origin[3];
    double                     dimensions[3];
    std::vector<double>        var_brick_min;
    std::vector<double>        var_brick_max;
};


#endif


