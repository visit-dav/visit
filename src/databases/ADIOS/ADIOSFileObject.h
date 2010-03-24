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

#ifndef ADIOS_FILE_OBJECT_H
#define ADIOS_FILE_OBJECT_H
#include <string>
#include <vector>
#include <map>
#include <visitstream.h>

//NOTE: #include <mpi.h> *MUST* become before the adios includes.
#ifdef PARALLEL
#include <mpi.h>
#else
#define _NOMPI
#endif

extern "C"
{
#include <adios_read.h>
}

template <class T> static inline void
SwapIndices(int dim, T *arr)
{
    if (dim <= 1) return;
    else if (dim == 2)
    {
        T i0 = arr[0], i1 = arr[1];
        arr[0] = i1; arr[1] = i0;
    }
    else if (dim == 3)
    {
        T i0 = arr[0], i2 = arr[2];
        arr[0] = i2; arr[2] = i0;
    }               
}

class ADIOSVar;
class ADIOSScalar;
class ADIOSAttr;
class vtkDataArray;
class vtkFloatArray;
class vtkPoints;

// ****************************************************************************
//  Class: ADIOSFileObject
//
//  Purpose:
//      Wrapper around an ADIOS file.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 16:15:32 EST 2010
//
//  Modifications:
//
//  Dave Pugmire, Tue Mar  9 12:40:15 EST 2010
//  Major overhaul. Added scalars, attributes, and reorganized the class.
//
//  Dave Pugmire, Wed Mar 17 15:29:24 EDT 2010
//  Add a vtkFloatArray ReadVariable method.
//
//  Dave Pugmire, Wed Mar 24 16:43:32 EDT 2010
//  Add GetStringScalar.
//
// ****************************************************************************

class ADIOSFileObject
{
  public:
    typedef std::map<std::string, ADIOSVar>::const_iterator varIter;

    ADIOSFileObject(const char *fname);
    ADIOSFileObject(const std::string &fname);
    virtual ~ADIOSFileObject();

    bool Open();
    void Close();
    bool IsOpen() const {return fp != NULL;}
    std::string Filename() const {return fileName;}
    int NumTimeSteps();
    void GetCycles(std::vector<int> &cycles);

    //Attributes
    bool GetIntAttr(const std::string &nm, int &val);
    bool GetDoubleAttr(const std::string &nm, double &val);
    bool GetStringAttr(const std::string &nm, std::string &val);
    
    //Scalars.
    bool GetIntScalar(const std::string &nm, int &val);
    bool GetDoubleScalar(const std::string &nm, double &val);
    bool GetStringScalar(const std::string &nm, std::string &val);
    
    //Coordinates.
    bool ReadCoordinates(const std::string &nm,
                         int ts,
                         vtkPoints **pts);

    //Variables.
    bool ReadVariable(const std::string &nm,
                      int ts,
                      vtkDataArray **array);
    bool ReadVariable(const std::string &nm,
                      int ts,
                      vtkFloatArray **array);
    
    std::map<std::string, ADIOSVar> variables;
    std::map<std::string, ADIOSScalar> scalars;
    std::map<std::string, ADIOSAttr> attributes;

  protected:
    std::string fileName;

    ADIOS_FILE *fp;
    ADIOS_GROUP **gps;

    void OpenGroup(int grpIdx);
    void CloseGroup(int grpIdx);
    
    static vtkPoints *AllocatePoints(ADIOS_DATATYPES &t);
    static vtkDataArray *AllocateArray(ADIOS_DATATYPES &t);
};


// ****************************************************************************
//  Class: ADIOSVar
//
//  Purpose:
//      Wrapper around ADIOS variable.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 16:15:32 EST 2010
//
// ****************************************************************************

class ADIOSVar
{
  public:
    ADIOSVar();
    ADIOSVar(const std::string &nm, int grpIdx, ADIOS_VARINFO *avi);
    ~ADIOSVar() {}

    void GetReadArrays(int ts, uint64_t *s, uint64_t *c, int *ntuples);
    
    void SwapIndices();
    ADIOS_DATATYPES type;
    int dim, groupIdx, varid, timedim;
    uint64_t start[3], count[3], global[3];
    std::string name;
    double extents[2];
};

// ****************************************************************************
//  Class: ADIOSScalar
//
//  Purpose:
//      Wrapper around ADIOS scalar.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

class ADIOSScalar
{
  public:
    ADIOSScalar() {Set("", adios_unknown, NULL);}
    ADIOSScalar(const std::string &nm, ADIOS_VARINFO *avi)
    {
        Set(nm, avi->type, avi->value);
    }
    
    ~ADIOSScalar()
    {
        if(ptr)
            free(ptr);
        ptr = NULL;
        sz = 0;
    }
    
    std::string Name() const {return name;}
    bool IsInt() const {return type == adios_integer;}
    bool IsFloat() const {return type == adios_real;}
    bool IsDouble() const {return type == adios_double;}
    bool IsString() const {return type == adios_string;}
    
    int AsInt() const
    {
        int v;
        memcpy(&v,ptr,sizeof(int));
        return v;
    }
    float AsFloat() const
    {
        float v;
        memcpy(&v,ptr,sizeof(float));
        return v;
    }
    double AsDouble() const
    {
        double v;
        memcpy(&v,ptr,sizeof(double));
        return v;
    }
    std::string AsString() const
    {
        std::string v = (char *)ptr;
        return v;
    }

    ADIOSScalar& operator=(const ADIOSScalar &s)
    {
        name = s.name;
        sz = s.sz;
        type = s.type;
        ptr = new unsigned char[sz];
        memcpy(ptr, s.ptr, sz);
    }

  protected:
    std::string name;
    size_t sz;
    void *ptr;
    ADIOS_DATATYPES type;

    void Set(const std::string &nm, ADIOS_DATATYPES t, void *p)
    {
        name = nm;
        type = t;
        ptr = NULL;
        sz = 0;
        if (t != adios_unknown)
            sz = adios_type_size(t, p);
        if (sz > 0)
        {
            ptr = malloc(sz);
            memcpy(ptr, p, sz);
        }
    }

};

// ****************************************************************************
//  Class: ADIOSAttr
//
//  Purpose:
//      Wrapper around ADIOS attribute.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

class ADIOSAttr : public ADIOSScalar
{
  public:
    ADIOSAttr() {Set("", adios_unknown, NULL);}
    ADIOSAttr(const std::string &nm, ADIOS_DATATYPES t, void *p)
    {
        Set(nm, t, p);
    }
};

// ****************************************************************************
//  Class: operator<<
//
//  Purpose:
//      Stream output for ADIOSVar
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************


inline std::ostream& operator<<(std::ostream& out, const ADIOSVar &v)
{
    out<<"ADIOSVar: "<<v.name<<endl;
    out<<"  dim= "<<v.dim<<" timedim= "<<v.timedim<<endl;
    out<<"  type= "<<v.type<<" ids= "<<v.groupIdx<<" "<<v.varid<<endl;
    out<<"  global= ["<<v.global[0]<<" "<<v.global[1]<<" "<<v.global[2]<<"]"<<endl;
    out<<"  start= ["<<v.start[0]<<" "<<v.start[1]<<" "<<v.start[2]<<"]"<<endl;
    out<<"  count= ["<<v.count[0]<<" "<<v.count[1]<<" "<<v.count[2]<<"]"<<endl;
    return out;
}


// ****************************************************************************
//  Class: operator<<
//
//  Purpose:
//      Stream output for ADIOSScalar
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

inline std::ostream& operator<<(std::ostream& out, const ADIOSScalar &s)
{
    out<<"ADIOSScalar: "<<s.Name()<<" value= ";
    if (s.IsInt()) out<<s.AsInt();
    else if (s.IsFloat()) out<<s.AsFloat();
    else if (s.IsDouble()) out<<s.AsDouble();
    else if (s.IsString()) out<<s.AsString();
    out<<endl;
    return out;
}

// ****************************************************************************
//  Class: operator<<
//
//  Purpose:
//      Stream output for ADIOSAttr
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

inline std::ostream& operator<<(std::ostream& out, const ADIOSAttr &s)
{
    out<<"ADIOSAttr: "<<s.Name()<<" value= ";
    if (s.IsInt()) out<<s.AsInt();
    else if (s.IsFloat()) out<<s.AsFloat();
    else if (s.IsDouble()) out<<s.AsDouble();
    else if (s.IsString()) out<<s.AsString();
    out<<endl;
    return out;
}


#endif
