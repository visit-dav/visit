/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//  File: FileWriter.C
// ************************************************************************* //

#include <FileWriter.h>

#ifdef HAVE_NETCDF
#include <vtkAbstractArray.h>
#include <vtkDoubleArray.h>

#include <netcdf.h>
#include <map>

using namespace std;

void
POTFilterWriteData::writeNETCDFData(const string &fname,
                                    const vector<string> &meshDimNms,
                                    const vector<vector<double> > &meshDims,
                                    const vector<varInfo> &vars,
                                    double *arr)

{

#if 0
    int ncID;
    int nVals = 0;//arrayShape[0];
    int nArrs = 0;//arrayShape[1];
    int nVars = vars.size();
    int nDims = meshDims.size();

    nc_create(fname.c_str(), NC_CLOBBER, &ncID);
    
    //Define dimensions.
    map<string, int> meshIdMap;
    map<string, int> DimSzMap;
    int *dimIds = new int[nDims];
    for (int i = 0; i < nDims; i++)
    {
        nc_def_dim(ncID, meshDimNms[i].c_str() , meshDims[i].size(), &dimIds[i]);
        meshIdMap[meshDimNms[i]] = dimIds[i];
        DimSzMap[meshDimNms[i]] = meshDims[i].size();
    }
    //Define dim-vars.
    int *dimVarIds = new int[nDims];
    for (int i = 0; i < nDims; i++)
        nc_def_var(ncID, meshDimNms[i].c_str(), NC_DOUBLE, 1, &dimIds[i], &dimVarIds[i]);

    //Define vars.
    int *varIds = new int[nVars];
    for (int i = 0; i < nVars; i++)
    {
        int dimSz = vars[i].dims.size();
        int *dims = new int[dimSz];
        for (int j = 0; j < dimSz; j++)
        {
            map<string, int>::const_iterator it = meshIdMap.find(vars[i].dims[j]);
            if (it != meshIdMap.end())
                dims[j] = (meshIdMap.find(vars[i].dims[j]))->second;
            else
            {
                cout<<"ERROR in dim name "<<vars[i].dims[j]<<endl;
                dims[j] = -1;
            }
        }

        nc_def_var(ncID, vars[i].name.c_str(), NC_DOUBLE, dimSz, dims, &varIds[i]);
    }
    nc_enddef(ncID);

    //Dump out dim values.
    for (int i = 0; i < nDims; i++)
    {
        double *d = new double[meshDims[i].size()];
        for (int j = 0; j < meshDims[i].size(); j++)
            d[j] = meshDims[i][j];
        nc_put_var_double(ncID, dimVarIds[i], d);
        delete [] d;
    }

    //Dump out var values.
    for (int i = 0; i < nVars; i++)
    {
        nVals = 1;
        for (int j = 0; j < vars[i].dims.size(); j++)
            nVals *= (DimSzMap[vars[i].dims[j]])->second;
        double *tmp = new double[nVals];

        for (int j = 0; j < nVals; j++)
            tmp[j] = arr[j*nVars +i];
        nc_put_var_double(ncID, varIds[i], tmp);
        delete [] tmp;
    }


    delete [] dimIds;
    delete [] dimVarIds;
    delete [] varIds;
    
    nc_close(ncID);
#endif
}

void
POTFilterWriteData::writeNETCDFData(const std::string &fname,
                                    const std::vector<std::string> &meshDimNms,
                                    const std::vector<std::vector<double> > &meshDims,
                                    const std::vector<std::string> &varnames,
                                    double *data)
{
    int ncID;
    int nLocs = meshDims[0].size()*meshDims[1].size();
    int nBins = meshDims[2].size();
    int nVars = varnames.size();
    int nDims = meshDims.size();

    //cout<<"FileWriter DUMP: nVars= "<<nVars<<" nLocs= "<<nLocs<<" nBins= "<<nBins<<endl;
    if (nBins == 1)
        nDims = 2;
    
    nc_create(fname.c_str(), NC_CLOBBER, &ncID);

    int *dimIds = new int[nDims];
    for (int i = 0; i < nDims; i++)
        nc_def_dim(ncID, meshDimNms[i].c_str() , meshDims[i].size(), &dimIds[i]);
    
    //Define vars.
    int *dimVarIds = new int[nDims];
    for (int i = 0; i < nDims; i++)
        nc_def_var(ncID, meshDimNms[i].c_str(), NC_DOUBLE, 1, &dimIds[i], &dimVarIds[i]);
    
    int *varIds = new int[nVars];
    for (int i = 0; i < varnames.size(); i++)
        nc_def_var(ncID, varnames[i].c_str(), NC_DOUBLE, nDims, dimIds, &varIds[i]);
    nc_enddef(ncID);

    //dump out dim values.
    for (int i = 0; i < nDims; i++)
    {
        double *d = new double[meshDims[i].size()];
        for (int j = 0; j < meshDims[i].size(); j++)
            d[j] = meshDims[i][j];
        nc_put_var_double(ncID, dimVarIds[i], d);
        delete [] d;
    }

    int nVals = nLocs*nBins;
    double *tmp = new double[nVals];
    for (int i = 0; i < nVars; i++)
    {
        int idx = 0;
        for (int j = 0; j < nLocs; j++)
            for (int k = 0; k < nBins; k++)
                tmp[idx++] = data[j*(nVars*nBins) + i*nBins +k];
        nc_put_var_double(ncID, varIds[i], tmp);
    }
    nc_close(ncID);

    delete [] dimIds;
    delete [] dimVarIds;
    delete [] varIds;
    
    nc_close(ncID);
}

#else

void
POTFilterWriteData::writeNETCDFData(const string &fname,
                                    const vector<string> &meshDimNms,
                                    const vector<vector<double> > &meshDims,
                                    const vector<varInfo> &vars,
                                    double *data)
{
}

#endif

