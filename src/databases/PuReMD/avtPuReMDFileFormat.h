// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtPuReMDFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_PuReMD_FILE_FORMAT_H
#define AVT_PuReMD_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>

#include <vector>
#include <utility>
#include <set>


// ****************************************************************************
//  Class: avtPuReMDFileFormat
//
//  Purpose:
//      Reads trajectory files from the PuReMD code.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 16, 2010
//
// ****************************************************************************

class avtPuReMDFileFormat : public avtMTSDFileFormat
{
  public:
                       avtPuReMDFileFormat(const char *);
    virtual           ~avtPuReMDFileFormat() {;};

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "PuReMD"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

    virtual bool          HasInvariantMetaData(void) const { return false; };

  protected:
    enum AtomFormat
    {
        AF_NONE,
        AF_POS_Q,
        AF_POS_FORCE_Q,
        AF_POS_VEL_Q,
        AF_POS_VEL_FORCE_Q
    };

    struct UCV
    {
        double v[3][3];
        double *operator[](int i) { return v[i]; }
        UCV()
        {
            v[0][0] = 1; v[0][1] = 0; v[0][2] = 0;
            v[1][0] = 0; v[1][1] = 1; v[1][2] = 0;
            v[2][0] = 0; v[2][1] = 0; v[2][2] = 1;
        }
    };

    ifstream                       in;
    std::string                    filename;

    std::vector<int>               cycles;
    std::vector<double>            times;
    std::vector<int>               nBonds;
    std::vector<UCV>               unitCell;
    std::vector<istream::pos_type> filePositions;
    bool                           metaDataRead;
    int                            nTimeSteps;
    int                            nAtoms;
    AtomFormat                     atomFormat;

    std::vector<int>               atomElement;
    std::vector<int>               atomSpecies;
    std::vector<double>            atomWeight;

    std::set<int>                  existingElements;

    int                            currentTimestep;
    std::vector<float>             x;
    std::vector<float>             y;
    std::vector<float>             z;
    std::vector<float>             vx;
    std::vector<float>             vy;
    std::vector<float>             vz;
    std::vector<float>             fx;
    std::vector<float>             fy;
    std::vector<float>             fz;
    std::vector<float>             q;
    std::vector<std::pair<int,int> > bonds;

    virtual void    PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    void            OpenFileAtBeginning();
    void            ReadTimeStep(int);
    void            ReadAllMetaData();

    virtual void           GetCycles(std::vector<int>&);
    virtual void           GetTimes(std::vector<double>&);
};


#endif
