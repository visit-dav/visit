
// ************************************************************************* //
//                            avtCMATFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_CMAT_FILE_FORMAT_H
#define AVT_CMAT_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>
#include <string>

class vtkFloatArray;

// ****************************************************************************
// Class: avtCMATFileFormat
//
// Purpose:
//     Reads in CMAT files as a plugin to VisIt.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 17 13:33:58 PST 2004
//
// Modifications:
//
// ****************************************************************************

class avtCMATFileFormat : public avtSTMDFileFormat
{
public:
    avtCMATFileFormat(const char *filename);
    virtual ~avtCMATFileFormat();

    virtual const char    *GetType(void)   { return "CMAT"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);

protected:
    void Initialize(bool);

    static const int n_zones_per_dom;

    int            xdims;
    int            ydims;
    float          xmin;
    float          xmax;
    float          ymin;
    float          ymax;
    std::string    title;
    int            nYPerDomain;
    int            numDomains;
    float         *data;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
