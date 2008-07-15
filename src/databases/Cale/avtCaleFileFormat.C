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

// ************************************************************************* //
//                            avtCaleFileFormat.C                            //
// ************************************************************************* //

#include <avtCaleFileFormat.h>

#include <string>
#include <vector>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h> 
#include <vtkPoints.h> 
#include <vtkPolyData.h> 
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h> 
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

#include <avtGhostData.h> 
#include <avtDatabaseMetaData.h>
#include <avtMixedVariable.h>
#include <avtVariableCache.h>

#include <Expression.h>
#include <DebugStream.h>

#include <InvalidVariableException.h>
#include <InvalidDBTypeException.h> 


#define TRY_MIXED_SCALARS

using     std::string;

typedef enum {NO_TYPE, CHAR_TYPE, INTEGER_TYPE, FLOAT_TYPE, DOUBLE_TYPE,
              LONG_TYPE, CHARARRAY_TYPE, INTEGERARRAY_TYPE, FLOATARRAY_TYPE,
              DOUBLEARRAY_TYPE, LONGARRAY_TYPE, OBJECT_TYPE} TypeEnum;

// ****************************************************************************
// Method: SymbolInformation
//
// Purpose: 
//   Looks up information about a symbol.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 5 14:09:32 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
SymbolInformation(PDBfile *pdb, const char *name, TypeEnum *t,
    std::string &typeString, int *nTotalElements, int **dimensions,
    int *nDims)
{
    bool retval = false;
    syment *ep = 0;

    // Indicate that there is no type initially.
    if (t)
        *t = NO_TYPE;
    if (nTotalElements)
        *nTotalElements = 0;
    if (dimensions)
        *dimensions = 0;
    if (nDims)
        *nDims = 0;

    if((ep = PD_inquire_entry(pdb, (char *)name, 0, NULL)) != NULL)
    {
        dimdes *dimptr = NULL;
        int i = 0, nd = 0, length = 1;
        int *dims = 0;

        // Return the actual name of type
        typeString = PD_entry_type(ep);

        // Figure out the number of dimensions and the number of elements
        // that are in the entire array.
        dimptr = PD_entry_dimensions(ep);
        if(dimptr != NULL)
        {
            // Figure out the number of dimensions.
            while(dimptr != NULL)
            {
                length *= dimptr->number;
                dimptr = dimptr->next;
                ++nd;
            }

            // Store the dimensions of the array.
            dims = new int[nd];
            dimptr = PD_entry_dimensions(ep);
            while(dimptr != NULL)
            {
                dims[i++] = dimptr->number;
                dimptr = dimptr->next;
            }
        }
        else
        {
            dims = new int[1];
            dims[0] = 1;
        }

        // Print the dimensions to the debug log.
        debug4 << "PDBFileObject::SymbolExists: name=" << name 
               << ", dimensions={";
        for(i = 0; i < nd; ++i)
            debug4 << dims[i] << ", ";
        debug4 << "}" << endl;

        // Set some of the return values.
        if (dimensions)
            *dimensions = dims;
        if (nDims)
            *nDims = nd;
        if (nTotalElements)
            *nTotalElements = length;

        //
        // Take the storage type along with the length to determine the real
        // type that we want to report. Also allocate memory for the
        // variable.
        //
        if (t)
        {
            if(strcmp(PD_entry_type(ep), "char") == 0 ||
               strcmp(PD_entry_type(ep), "string") == 0)
                *t = (length > 1) ? CHARARRAY_TYPE : CHAR_TYPE;
            else if(strcmp(PD_entry_type(ep), "int") == 0 ||
                    strcmp(PD_entry_type(ep), "integer") == 0)
                *t = (length > 1) ? INTEGERARRAY_TYPE : INTEGER_TYPE;
            else if(strcmp(PD_entry_type(ep), "float") == 0)
                *t = (length > 1) ? FLOATARRAY_TYPE : FLOAT_TYPE;
            else if(strcmp(PD_entry_type(ep), "double") == 0)
                *t = (length > 1) ? DOUBLEARRAY_TYPE : DOUBLE_TYPE;
            else if(strcmp(PD_entry_type(ep), "long") == 0)
                *t = (length > 1) ? LONGARRAY_TYPE : LONG_TYPE;
            else
            {
                *t = OBJECT_TYPE;
            }
        }

        retval = true;
    }

    return retval;
}


// ****************************************************************************
// Method: avtCaleFileFormat::Identify
//
// Purpose: 
//   Detects whether a file is a CALE file. The routine throws exceptions if
//   the file is not a CALE file. This routine is used in 
//   cale_visitCommonPluginInfo::SetupDatabase. Doing it this way allows us to
//   check 1 file in a series for CALE conformanace without having to check
//   each and every file, making file loading *much* faster.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Rob Managan
// Creation:   Fri Sep 21 11:29:35 PDT 2007
//
// Modifications:
//   Rob Managan, Tue Jan 15 15:15:58 PST 2008
//   Changed to support other related files.
//
// ****************************************************************************

void
avtCaleFileFormat::Identify(const char *filename)
{
    const char *mName = "avtCaleFileFormat::Identify: ";
    PDBfile *pdb = PD_open((char *)filename,"r") ;
   
    if (pdb == NULL)
    {
        EXCEPTION1(InvalidDBTypeException, "The file could not be opened") ;
    }
    else
    {
        int nnalls = 0, kmax = 0, lmax = 0;
        int pdberr ;
        pdberr  = PD_read(pdb,"/parameters/nnalls",&nnalls) ;
        pdberr |= PD_read(pdb,"/parameters/kmax",&kmax) ;
        pdberr |= PD_read(pdb,"/parameters/lmax",&lmax) ;

        debug5 << mName << "nnalls = " << nnalls << endl;
        debug5 << mName << "kmax = " << kmax << endl;
        debug5 << mName << "lmax = " << lmax << endl;

        if ((!pdberr) || (nnalls != (kmax + 2)*(lmax + 2)))
        {
            PD_close(pdb);
            EXCEPTION1(InvalidDBTypeException, "The file is not a Cale dump") ;
        }
    }
    PD_close(pdb);
}

// ****************************************************************************
//  Method: avtCaleFileFormat constructor
//
//  Programmer: Rob Managan
//  Creation:   Wed Sep 19 13:30:36 PST 2007
//
// ****************************************************************************

avtCaleFileFormat::avtCaleFileFormat(const char *filename)
    : avtSTSDFileFormat(filename)
{
    kminmesh = kmaxmesh = lminmesh = lmaxmesh = -1 ;

    pdbfile = NULL;
}

avtCaleFileFormat::~avtCaleFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
//  Method: avtCaleFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Rob Managan
//  Creation:   Wed Sep 19 13:30:36 PST 2007
//
// ****************************************************************************

void
avtCaleFileFormat::FreeUpResources(void)
{
    if(pdbfile != NULL)
        PD_close(pdbfile);
    pdbfile = NULL;
}

// ****************************************************************************
// Method: avtCaleFileFormat::GetPDBFile
//
// Purpose: 
//   Opens the PDB file if it's not already open.
//
// Returns:    The PDB file descriptor.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 21 11:21:14 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

PDBfile *
avtCaleFileFormat::GetPDBFile()
{
    if(pdbfile == NULL)
        pdbfile = PD_open((char *)filename, "r");

    return pdbfile;
}

// ****************************************************************************
//  Method: avtCaleFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Rob Managan
//  Creation:   Wed Sep 19 13:30:36 PST 2007
//
//  Modifications:
//    Kathleen Bonnell, Tue Jul 1 16:00:00 PDT 2008
//    Removed unreferenced variables.
//
// ****************************************************************************

void
avtCaleFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    const char *mName = "avtCaleFileFormat::PopulateDatabaseMetaData: ";
    //
    // CODE TO ADD A MESH
    //
    std::string meshname = "hydro";
    //
    // Add the mesh to the metadata. Note that this example will 
    // always expose a mesh called "hydro" to VisIt. A real 
    // plug-in may want to read a list of meshes from the data 
    // file. 
    avtMeshMetaData *mmd = new avtMeshMetaData; 
    mmd->name = meshname; 
    mmd->spatialDimension = 2; 
    mmd->topologicalDimension = 2; 
    mmd->meshType = AVT_CURVILINEAR_MESH; 
    mmd->numBlocks = 1; 
    mmd->xLabel = "z-axis";
    mmd->yLabel = "r-axis";
    // mmd->cellOrigin = 1;
    // mmd->nodeOrigin = 1;
    // mmd->blockOrigin = 1;
    md->Add(mmd); 

    //
    // find portion of mesh used in this dump
    if (kminmesh == -1)
        GetUsedMeshLimits();

    //
    // CODE TO ADD A SCALAR VARIABLE
    // Now walk the fpa arrays and put them all out like wsilo.
    // access this list in teh dump through the fpalist structure
    //
    int i, nfpa, nnalls, namix, pdberr;
    int npbin, ngrps, rdifmix;
    typedef struct 
    {
        char name[8];
        int len;
    }  parec;
    
    pdberr =           PD_read(GetPDBFile(),"/parameters/nfpa",&nfpa);
    parec *palist = new parec[nfpa];
    pdberr = pdberr && PD_read(GetPDBFile(),"/parameters/nnalls",&nnalls);
    pdberr = pdberr && PD_read(GetPDBFile(),"/parameters/namix",&namix);
    pdberr = pdberr && PD_read(GetPDBFile(),"/parameters/npbin",&npbin);
    pdberr = pdberr && PD_read(GetPDBFile(),"/parameters/rdifmix",&rdifmix);
    pdberr = pdberr && PD_read(GetPDBFile(),"/parameters/ngrps",&ngrps);
    pdberr = pdberr && PD_read(GetPDBFile(),"fpalist",palist);
    if (!pdberr)
    {
        EXCEPTION1(InvalidDBTypeException,
            "Corrupt dump; error reading array related variables.");
    }

    std::string mesh_for_this_var = meshname; // ??? -- could be multiple meshes
    avtCentering cent = AVT_ZONECENT;

    for ( i = 0 ; i < nfpa ; i++ )
    {
        char  varname[10];
        char *name = palist[i].name;

        // debug4 << " i " << i << " name " << palist[i].name << " len " 
        //        << palist[i].len << endl;

        // z & r go into the mesh data structure 
        if (!strcmp(name, "z") || !strcmp(name, "r"))
            continue;

        if ((palist[i].len == nnalls) | (palist[i].len == namix))
        {
            debug4 << " adding scalar variable " << palist[i].name << " len " 
                   << palist[i].len << endl;
            strncpy(varname,palist[i].name,8);
            if (!strcmp(name, "rvel")   ||
                !strcmp(name, "zvel")   ||
                !strcmp(name, "vr")     ||
                !strcmp(name, "vt")     ||
                !strcmp(name, "rad")    ||
                !strcmp(name, "theta")  ||
                !strcmp(name, "vtheta") ||
                !strcmp(name, "vmag")   ||
                !strcmp(name, "bmhdr")  ||
                !strcmp(name, "bmhdz")  ||
                !strcmp(name, "jmhdr")  ||
                !strcmp(name, "jmhdz")  ||
                !strcmp(name, "omega")  ||
                !strcmp(name, "r2w")    ||
                !strcmp(name, "lt")      )
            {
                cent = AVT_NODECENT;
            }
            else
            {
                cent = AVT_ZONECENT;
            } 
            // Add a scalar to the metadata. 
            avtScalarMetaData *smd = new avtScalarMetaData; 
            smd->name = varname; 
            smd->meshName = mesh_for_this_var; 
            smd->centering = cent;
            if (!strcmp(name, "rvel")   ||
                !strcmp(name, "zvel")   ||
                !strcmp(name, "vr")     ||
                !strcmp(name, "vt")     ||
                !strcmp(name, "vmag"))
            {
                smd->hasUnits = true; 
                smd->units = "cm/microsecond";
            }
            else
            {
                smd->hasUnits = false; 
            }
            md->Add(smd); 
        }
        else if (palist[i].len == npbin*namix)
        {
            cent = AVT_ZONECENT;
            for (int g = 0 ; g < npbin ; g++ )
            {
                sprintf(varname,"%s_%02d",palist[i].name,g);
                debug4 << " adding scalar variable " << varname << " len " 
                       << palist[i].len << endl;
                // Add a scalar to the metadata. 
                avtScalarMetaData *smd = new avtScalarMetaData; 
                smd->name = varname; 
                smd->meshName = mesh_for_this_var; 
                smd->centering = cent;
                md->Add(smd);
            }
        }
        else if (palist[i].len == ngrps*nnalls)
        {
            cent = AVT_ZONECENT;
            for (int g = 0 ; g < ngrps ; g++ )
            {
                sprintf(varname,"%s_%02d",palist[i].name,g);
                debug4 << " adding scalar variable " << varname << " len " 
                       << palist[i].len << endl;
                // Add a scalar to the metadata. 
                avtScalarMetaData *smd = new avtScalarMetaData; 
                smd->name = varname; 
                smd->meshName = mesh_for_this_var; 
                smd->centering = cent;
                md->Add(smd);
            }
        }
        else
        {
            debug4 << " ignoring scalar variable " << palist[i].name << " len " 
                   << palist[i].len << endl;
        }
    }
    delete [] palist;

    //
    // CODE TO ADD A MATERIAL
    //
    int nreg;
    typedef struct
    {
        char name[33];
    }  trcname_str;

    pdberr = PD_read(GetPDBFile(),"/parameters/nreg",&nreg);
    if (!pdberr)
    {
        EXCEPTION1(InvalidDBTypeException,
            "Corrupt dump; error reading # of materials.");
    }
    
    if (nreg >= 1)
    {
        trcname_str *rname = new trcname_str[nreg+1];

        pdberr = PD_read(GetPDBFile(),"/ppa/rname",rname);
        if (!pdberr)
        {
            EXCEPTION1(InvalidDBTypeException,
                "Corrupt dump; error reading material names.");
        }
        strcpy(rname[0].name,"Phony");
       
        int nmats = nreg + 1;
        avtMaterialMetaData *matmd = new avtMaterialMetaData;
        matmd->name = "Materials";
        matmd->meshName = meshname; 
        matmd->numMaterials = nmats; 

        for ( i = 0 ; i <= nreg ; i++ )
        {
            matmd->materialNames.push_back(rname[i].name);
            debug4 << " region " << i << " is named " << rname[i].name << endl;
        }
        md->Add(matmd);
    }

    // Here's the way to add expressions:
    Expression velocity_expr;
    velocity_expr.SetName("velocity");
    velocity_expr.SetDefinition("{zvel, rvel}");
    velocity_expr.SetType(Expression::VectorMeshVar);
    md->AddExpression(&velocity_expr);

    Expression vel_expr;
    vel_expr.SetName("vel");
    vel_expr.SetDefinition("{zvel, rvel}");
    vel_expr.SetType(Expression::VectorMeshVar);
    md->AddExpression(&vel_expr);

    int ifstr, ifstrain, iftpstr, ifmhda, ifmhdb;
    pdberr =           PD_read(GetPDBFile(),"/parameters/ifstr",&ifstr);
    pdberr = pdberr && PD_read(GetPDBFile(),"/parameters/ifstrain",&ifstrain);
    pdberr = pdberr && PD_read(GetPDBFile(),"/parameters/iftpstr",&iftpstr);
    pdberr = pdberr && PD_read(GetPDBFile(),"/parameters/ifmhda",&ifmhda);
    pdberr = pdberr && PD_read(GetPDBFile(),"/parameters/ifmhdb",&ifmhdb);
    if (!pdberr)
    {
        EXCEPTION1(InvalidDBTypeException,
            "Corrupt dump; error reading memory layout parameters.");
    }

    if (ifstr == 1)
    {
        Expression stress_expr;
        stress_expr.SetName("stress");
        stress_expr.SetDefinition("{ { szz, srz }, { srz, srr } }");
        stress_expr.SetType(Expression::TensorMeshVar);
        md->AddExpression(&stress_expr);
    }
    
    if (ifstrain == 1)
    {
        Expression strain_dev_expr;
        strain_dev_expr.SetName("strain_dev");
        strain_dev_expr.SetDefinition("{ { strndzz, strndrz }, "
                                      "{ strndrz, strndrr } }");
        strain_dev_expr.SetType(Expression::TensorMeshVar);
        md->AddExpression(&strain_dev_expr);

        Expression strain_tot_expr;
        strain_tot_expr.SetName("strain_tot");
        strain_tot_expr.SetDefinition("{ { strntzz, strntrz }, "
                                      "{ strntrz, strntrr } }");
        strain_tot_expr.SetType(Expression::TensorMeshVar);
        md->AddExpression(&strain_tot_expr);
    }
    
    if (iftpstr == 1)
    {
        Expression stress_tot_expr;
        stress_tot_expr.SetName("stress_tot");
        stress_tot_expr.SetDefinition("{ { tszz, tsrz }, { tsrz, tsrr } }");
        stress_tot_expr.SetType(Expression::TensorMeshVar);
        md->AddExpression(&stress_tot_expr);
    }
    
    if (ifmhda == 1)
    {
        Expression B_field_expr;
        B_field_expr.SetName("B_field");
        B_field_expr.SetDefinition("{ bmhdz, bmhdr }");
        B_field_expr.SetType(Expression::VectorMeshVar);
        md->AddExpression(&B_field_expr);
    }
    
    if (ifmhdb == 1)
    {
        Expression current_expr;
        current_expr.SetName("current");
        current_expr.SetDefinition("{ jmhdz, jmhdr }");
        current_expr.SetType(Expression::VectorMeshVar);
        md->AddExpression(&current_expr);
    }
    // Use array_compose to group the different energy groups together
    if (npbin > 1)
    {
        Expression pbin_array_expr;
        pbin_array_expr.SetName("pbin_array");
        // std::string expr = "array_compose(expr1, expr2, ..., exprN)"
        std::string expr = "array_compose(";
        char sub_expr[33];
        for ( int g = 0 ; g < npbin ; g++ )
        {
            sprintf(sub_expr,"pbin_%02d",g);
            if (g < npbin-1)
                strcat(sub_expr,", ");
            expr += sub_expr;
        }
        strcpy(sub_expr,")");
        expr += sub_expr;
        pbin_array_expr.SetDefinition(expr);
        pbin_array_expr.SetType(Expression::ArrayMeshVar);
        md->AddExpression(&pbin_array_expr);

        Expression kbin_array_expr;
        kbin_array_expr.SetName("kbin_array");
        // std::string expr = "array_compose(expr1, expr2, ..., exprN)"
        expr = "array_compose(";
        for ( int g = 0 ; g < npbin ; g++ )
        {
            sprintf(sub_expr,"kbin_%02d",g);
            if (g < npbin-1)
                strcat(sub_expr,", ");
            expr += sub_expr;
        }
        strcpy(sub_expr,")");
        expr += sub_expr;
        kbin_array_expr.SetDefinition(expr);
        kbin_array_expr.SetType(Expression::ArrayMeshVar);
        md->AddExpression(&kbin_array_expr);
    }
    if (ngrps > 1)
    {
        Expression flux_array_expr;
        flux_array_expr.SetName("nflux_array");
        // std::string expr = "array_compose(expr1, expr2, ..., exprN)"
        std::string expr = "array_compose(";
        char sub_expr[33] ;
        for ( int g = 0 ; g < ngrps ; g++ )
        {
            sprintf(sub_expr,"nflux_%02d",g);
            if (g < ngrps-1)
                strcat(sub_expr,", ");
            expr += sub_expr;
        }
        strcpy(sub_expr,")");
        expr += sub_expr;
        flux_array_expr.SetDefinition(expr);
        flux_array_expr.SetType(Expression::ArrayMeshVar);
        md->AddExpression(&flux_array_expr);
    }
    // 
    // Add time history curve data
    //

    int ntp, ncp, ncurves, ntimes, tplen ;
    pdberr =           PD_read(GetPDBFile(),"/parameters/ntp",&ntp);
    pdberr = pdberr && PD_read(GetPDBFile(),"/parameters/ncp",&ncp);
    if (!pdberr)
    {
        ntp = ncp = 0;
    }
    
    debug4 << ntp << " time plot sections" << endl;
    for ( int i = 1 ; i <= ntp ; i++ )
    {
        char vname[128], label[128];
        sprintf(vname, "/ppa/tpdata_%d/ncurs",i);
        pdberr = PD_read(GetPDBFile(),vname,&ncurves);
        debug4 << "section " << i << " has " << ncurves << " curves " << endl;

        for ( int icur = 1 ; icur < ncurves ; icur++ )
        {
            sprintf(vname, "/ppa/tpdata_%d/tpcur_%d/tplab",i,icur);
            pdberr = PD_read(GetPDBFile(),vname,label);
            debug4 << "label '" << label << "'" << endl;
            sprintf(vname, "/ppa/tpdata_%d/tpcur_%d/tplen",i,icur);
            pdberr = PD_read(GetPDBFile(),vname,&tplen);
            if (tplen == 0)
                break;
            avtCurveMetaData *cmd = new avtCurveMetaData;
            cmd->name = label;
            // Labels and units are strings so use whatever you want.
            cmd->yLabel = "y-axis"; 
            // cmd->yUnits = "cm"
            cmd->xLabel = "time";
            cmd->xUnits = "microseconds";
            md->Add(cmd);
        }
    }

    debug4 << ncp << " cycle plot sections" << endl;
    for ( int i = 1 ; i <= ncp ; i++ )
    {
        char vname[128], label[128];
        sprintf(vname, "/ppa/cpdata_%d/ncurs",i);
        pdberr = PD_read(GetPDBFile(),vname,&ncurves);
        sprintf(vname, "/ppa/cpdata_%d/ntimes",i);
        pdberr = PD_read(GetPDBFile(),vname,&ntimes);
        debug4 << "section " << i << " has " << ncurves << " curves " << endl;

        for ( int icur = 1 ; icur < ncurves ; icur++ )
        {
            sprintf(vname, "/ppa/cpdata_%d/cpcur_%d/tplab",i,icur);
            pdberr = PD_read(GetPDBFile(),vname,label);
            debug4 << "label '" << label << "'" << endl;
            sprintf(vname, "/ppa/cpdata_%d/cpcur_%d/tplen",i,icur);
            pdberr = PD_read(GetPDBFile(),vname,&tplen);
            if (tplen == 0)
                break;
            avtCurveMetaData *cmd = new avtCurveMetaData;
            cmd->name = label;
            // Labels and units are strings so use whatever you want.
            cmd->yLabel = "y-axis"; 
            // cmd->yUnits = "cm"
            cmd->xLabel = "time";
            cmd->xUnits = "microseconds";
            md->Add(cmd);
        }
    }
}


// ****************************************************************************
//  Method: avtCaleFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Rob Managan
//  Creation:   Wed Sep 19 13:30:36 PST 2007
//
//  Modifications:
//    Kathleen Bonnell, Mon Jul 14 13:34:37 PDT 2008
//    Specify curves as 1D rectilinear grids with yvalues stored in point data.
//
// ****************************************************************************

vtkDataSet *
avtCaleFileFormat::GetMesh(const char *meshname)
{
    const char *mName = "avtCaleFileFormat::GetMesh: ";
    // Determine which mesh to return. 
    if (strcmp(meshname, "hydro") == 0) 
    {
        // Create a VTK object for "hydro" mesh
        int ndims = 2; 
        int dims[3] = {1,1,1};
        int kmax, lmax, lp, nnalls = 0, namix;
        int nk, nl, pdberr;
          
        if (kminmesh == -1) 
            GetUsedMeshLimits();
        // Read the ndims and number of X,Y,Z nodes from file. 
        pdberr =           PD_read(GetPDBFile(),"/parameters/kmax",&kmax);
        pdberr = pdberr && PD_read(GetPDBFile(),"/parameters/lmax",&lmax);
        pdberr = pdberr && PD_read(GetPDBFile(),"/parameters/lp",&lp);
        pdberr = pdberr && PD_read(GetPDBFile(),"/parameters/nnalls",&nnalls);
        pdberr = pdberr && PD_read(GetPDBFile(),"/parameters/namix",&namix);
        double *z = new double[nnalls];
        double *r = new double[nnalls];
        pdberr = pdberr && PD_read(GetPDBFile(),"/arrays/z",z);
        pdberr = pdberr && PD_read(GetPDBFile(),"/arrays/r",r);
        if (!pdberr)
        {
            EXCEPTION1(InvalidDBTypeException,
                       "Corrupt dump; error reading mesh related parameters.");
        }

        nk = kmaxmesh - kminmesh + 1 ; 
        nl = lmaxmesh - lminmesh + 1 ; 
        dims[0] = nk ; 
        dims[1] = nl ; 
        dims[2] = 1 ; 
        int nnodes = dims[0]*dims[1]*dims[2]; 
        // Read the X coordinates from the file. 
        double *xarray = new double[nnodes]; 

        // Read the Y coordinates from the file. 
        double *yarray = new double[nnodes]; 

        // debug4 << "nnalls " << nnalls << " nnodes " << nnodes 
        //        << " kmax,lmax " << kmax <<","<< lmax << " lp "<< lp << endl;
        // debug4 << "kminmesh " << kminmesh << " kmaxmesh " << kmaxmesh 
        //        << " lminmesh " << lminmesh <<" lmaxmesh "<< lmaxmesh << endl;
      
        for ( int l = 0 ; l < nl ; l++ )
        {
            for ( int k = 0 ; k < nk ; k++ )
            {
                xarray[l*nk + k] = z[(l+lminmesh)*lp + k+kminmesh];
                yarray[l*nk + k] = r[(l+lminmesh)*lp + k+kminmesh];
            }
        }
        // 
        // Create the vtkStructuredGrid and vtkPoints objects. 
        // 
        vtkStructuredGrid *sgrid  = vtkStructuredGrid::New(); 
        vtkPoints         *points = vtkPoints::New(); 
        sgrid->SetPoints(points); 
        sgrid->SetDimensions(dims); 
        points->Delete(); 
        points->SetNumberOfPoints(nnodes); 
        // 
        // Copy the coordinate values into the vtkPoints object.
        // 
        float *pts = (float *) points->GetVoidPointer(0); 
        double *xc = xarray; 
        double *yc = yarray; 
        for(int j = 0; j < dims[1]; ++j) 
            for(int i = 0; i < dims[0]; ++i) 
            { 
                *pts++ = *xc++; 
                *pts++ = *yc++; 
                *pts++ = 0.; 
            } 
 
        // Delete temporary arrays
        delete [] xarray; 
        delete [] yarray; 
        delete [] z; 
        delete [] r; 

        // Now that you have your mesh, figure out which cells need 
        // to be removed. 

        int nCells = sgrid->GetNumberOfCells(); 
        int *blanks = new int[namix]; 
        pdberr = PD_read(GetPDBFile(),"/arrays/ireg",blanks);
      
        debug4 << mName <<"nCells " << nCells << " nnalls " << nnalls 
               << " k " << kminmesh << ":" << kmaxmesh << " l " << lminmesh 
               << ":" << lmaxmesh << endl;
        // Now that we have the blanks array, create avtGhostZones. 
        unsigned char realVal = 0, ghost = 0; 
        avtGhostData::AddGhostZoneType(ghost, ZONE_NOT_APPLICABLE_TO_PROBLEM); 
        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New(); 
        ghostCells->SetName("avtGhostZones"); 
        ghostCells->Allocate(nCells); 
        for(int i = 0; i < nCells; ++i) 
        {
            int k, l;
            l = i/(nk-1);
            k = i - l*(nk-1);
            debug4 << mName << " i " << i << " nk " << nk << " k,l " 
                   << k+kminmesh << "," << l+lminmesh << endl;
            if(blanks[(l+lminmesh)*lp + k+kminmesh]) 
            {
                ghostCells->InsertNextValue(realVal);
                debug4 << mName << "real zone at i " << i << endl;
            }
            else
            { 
                ghostCells->InsertNextValue(ghost); 
                debug4 << mName << "Ghost zone at i " << i << endl;
            }
        } 
        sgrid->GetCellData()->AddArray(ghostCells); 
        sgrid->SetUpdateGhostLevel(0); 
        ghostCells->Delete(); 
        // Clean up 
        delete [] blanks; 
        return sgrid; 
    }
    else if (strcmp(meshname, "mesh2") == 0) 
    { 
        // Create a VTK object for "mesh2" 
        return 0; 
    } 
    else // check for time or cycle plot
    {
        int ntp, ncp, ncurves, ntimes, tplen, foundit=0;
        int pdberr;
        pdberr = PD_read(GetPDBFile(),"/parameters/ntp",&ntp);
        pdberr = PD_read(GetPDBFile(),"/parameters/ncp",&ncp);
        double *ttime, *data;

        for ( int i = 1 ; i <= ntp ; i++ )
        {
            char vname[128], label[128];
            sprintf(vname, "/ppa/tpdata_%d/ncurs",i);
            pdberr = PD_read(GetPDBFile(),vname,&ncurves);
            sprintf(vname, "/ppa/tpdata_%d/ntimes",i);
            pdberr = PD_read(GetPDBFile(),vname,&ntimes);
            debug4 << "section " << i << " has " << ncurves 
                   << " curves " << endl;

            for ( int icur = 1 ; icur < ncurves ; icur++ )
            {
                sprintf(vname, "/ppa/tpdata_%d/tpcur_%d/tplab",i,icur);
                pdberr = PD_read(GetPDBFile(),vname,label);
                if (strcmp(meshname,label) == 0)
                {
                    debug4 << "matched label '" << label << "'" << endl;
                    sprintf(vname, "/ppa/tpdata_%d/tpcur_%d/tplen",i,icur);
                    pdberr = PD_read(GetPDBFile(),vname,&tplen);
                    if (tplen == 0) 
                        break;
                    ttime = new double[tplen];
                    data  = new double[tplen];
                    sprintf(vname, "/ppa/tpdata_%d/tpcur_0/tpdat",i,icur);
                    pdberr = PD_read(GetPDBFile(),vname,ttime);
                    sprintf(vname, "/ppa/tpdata_%d/tpcur_%d/tpdat",i,icur);
                    pdberr = PD_read(GetPDBFile(),vname,data);
                    foundit = 1;
                    break;
                }
            }
            if (foundit == 1) 
                break;
        }

        if (foundit == 0)
        {
            for ( int i = 1 ; i <= ncp ; i++ )
            {
                char vname[128], label[128];
                sprintf(vname, "/ppa/cpdata_%d/ncurs",i);
                pdberr = PD_read(GetPDBFile(),vname,&ncurves);
                sprintf(vname, "/ppa/cpdata_%d/ntimes",i);
                pdberr = PD_read(GetPDBFile(),vname,&ntimes);
                debug4 << "section " << i << " has " << ncurves 
                       << " curves " << endl;

                for ( int icur = 1 ; icur < ncurves ; icur++ )
                {
                    sprintf(vname, "/ppa/cpdata_%d/cpcur_%d/tplab",i,icur);
                    pdberr = PD_read(GetPDBFile(),vname,label);
                    if (strcmp(meshname,label) == 0)
                    {
                        debug4 << "matched label '" << label << "'" << endl;
                        sprintf(vname, "/ppa/cpdata_%d/cpcur_%d/tplen",i,icur);
                        pdberr = PD_read(GetPDBFile(),vname,&tplen);
                        if (tplen == 0)
                            break;
                        ttime = new double[tplen];
                        data  = new double[tplen];
                        sprintf(vname, "/ppa/cpdata_%d/cpcur_0/tpdat",i,icur);
                        pdberr = PD_read(GetPDBFile(),vname,ttime);
                        sprintf(vname, "/ppa/cpdata_%d/cpcur_%d/tpdat",i,icur);
                        pdberr = PD_read(GetPDBFile(),vname,data);
                        foundit = 1;
                        break;
                    }
                }
                if (foundit == 1)
                    break ;
            }
        }
        if (foundit == 1)
        {
            vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(tplen, 
                                                                 VTK_DOUBLE);
            vtkDoubleArray *vals = vtkDoubleArray::New();
            vals->SetNumberOfComponents(1);
            vals->SetNumberOfTuples(tplen);
            vals->SetName(meshname);
            rg->GetPointData()->SetScalars(vals);
            vtkDoubleArray *xc = vtkDoubleArray::SafeDownCast(
                                      rg->GetXCoordinates());

            for (int j = 0 ; j < tplen ; j++)
            {
                xc->SetValue(j, ttime[j]);
                vals->SetValue(j, data[j]);
            }

            delete [] ttime;
            delete [] data;
            vals->Delete();
            return rg;
        }
        else
        {
            // No mesh name that we recognize. 
            EXCEPTION1(InvalidVariableException, "no data for this curve"); 
        }
    }
    return 0; 
}


// ****************************************************************************
//  Method: avtCaleFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Rob Managan
//  Creation:   Wed Sep 19 13:30:36 PST 2007
//
//  Modifications:
//    Brad Whitlock, Fri Oct 5 14:10:28 PST 2007
//    Added support for mixed material variables.
//
// ****************************************************************************

vtkDataArray *
avtCaleFileFormat::GetVar(const char *varname)
{
    const char *mName = "avtCaleFileFormat::GetVar: ";

    debug4 << mName << varname << endl;

    int kmax, lmax, lp, nnalls, namix, nvals, pdberr, nk, nl;
    int npbin, ngrps, rdifmix;
    int length, group, grplen;
    pdberr = PD_read(GetPDBFile(),"/parameters/kmax",&kmax);
    pdberr = PD_read(GetPDBFile(),"/parameters/lmax",&lmax);
    pdberr = PD_read(GetPDBFile(),"/parameters/lp",&lp);
    pdberr = PD_read(GetPDBFile(),"/parameters/nnalls",&nnalls);
    pdberr = PD_read(GetPDBFile(),"/parameters/namix",&namix);
    pdberr = PD_read(GetPDBFile(),"/parameters/npbin",&npbin);
    pdberr = PD_read(GetPDBFile(),"/parameters/rdifmix",&rdifmix);
    pdberr = PD_read(GetPDBFile(),"/parameters/ngrps",&ngrps);
    length = namix;
    char    vstring[33];

    if (strncmp(varname,"pbin",4) == 0)
    {
        strcpy(vstring,"/arrays/pbin");
        grplen = namix;
        length = npbin*grplen;
        sscanf(varname,"pbin_%d",&group);
    }
    else if (strncmp(varname,"kbin",4) == 0)
    {
        strcpy(vstring,"/arrays/kbin");
        grplen = namix;
        length = npbin*grplen;
        sscanf(varname,"kbin_%d",&group);
    }
    else if (strncmp(varname,"nflux",5) == 0)
    {
        strcpy(vstring,"/arrays/nflux");
        grplen = nnalls;
        length = ngrps*grplen;
        sscanf(varname,"nflux_%d",&group);
    }
    else
    {
        sprintf(vstring,"/arrays/%s",varname);
        grplen = 1;
        length = namix;
        group = 0;
    }

    double *vararray = new double[length];
    pdberr = PD_read(GetPDBFile(),vstring,vararray);
    double *varray = vararray + group*grplen;

    if (kminmesh == -1)
        GetUsedMeshLimits();
    nk = kmaxmesh - kminmesh + 1; 
    nl = lmaxmesh - lminmesh + 1; 
      
    if (!strcmp(varname, "r")      ||
        !strcmp(varname, "z")      ||
        !strcmp(varname, "rvel")   ||
        !strcmp(varname, "zvel")   ||
        !strcmp(varname, "vr")     ||
        !strcmp(varname, "vz")     ||
        !strcmp(varname, "rad")    ||
        !strcmp(varname, "theta")  ||
        !strcmp(varname, "vtheta") ||
        !strcmp(varname, "vmag")   ||
        !strcmp(varname, "bmhdr")  ||
        !strcmp(varname, "bmhdz")  ||
        !strcmp(varname, "jmhdr")  ||
        !strcmp(varname, "jmhdz")  ||
        !strcmp(varname, "omega")  ||
        !strcmp(varname, "r2w")    ||
        !strcmp(varname, "lt")      )
    {
        int i = 0;
        // node centered 
        nvals = nk*nl;
        // debug4 << "getvar zc: nk " << nk << " nl " << nl << " nvals " 
        //        << nvals << endl;
        vtkFloatArray *rv = vtkFloatArray::New();
        rv->SetNumberOfTuples(nvals);
        for ( int l = 0 ; l < nl ; l++ )
        {
            for (int k = 0 ; k < nk ; k++ )
            {
                rv->SetTuple1(i++, varray[(l+lminmesh)*lp + k+kminmesh]);  
            }
        }
        return rv;
    }
    else
    {
        int i = 0;
        // zone centered 
        nvals = (nk-1)*(nl-1);
        // debug4 << "getvar zc: nk " << nk << " nl " << nl << " nvals " 
        //        << nvals << endl;
        vtkFloatArray *rv = vtkFloatArray::New();
        rv->SetNumberOfTuples(nvals);
        for ( int l = 0 ; l < nl-1 ; l++ )
        {
            for ( int k = 0 ; k < nk-1 ; k++ )
            {
                rv->SetTuple1(i++, varray[(l+lminmesh)*lp + k+kminmesh]); 
            }
        }

#ifdef TRY_MIXED_SCALARS
        TypeEnum t = NO_TYPE;
        std::string typeString;
        int nTotalElements, *dims = 0, ndims = 0;
        if(SymbolInformation(GetPDBFile(), vstring, &t,
                             typeString, &nTotalElements, &dims, &ndims))
        {
            debug4 << mName << "nTotalElements = " << nTotalElements << endl;
            debug4 << mName << "nnalls = " << nnalls << endl;
            // truncate zmass to nnalls since the mixslots hold volume fractions
            if (strcmp(varname,"zmass") == 0)
            {
                nTotalElements = nnalls;
            }
            // for spectral variable set nTotalElements for the group.
            else if ((strncmp(varname,"kbin",4) == 0) | 
                     (strncmp(varname,"pbin",4) == 0))
            {
                if (rdifmix == 0)
                {
                    nTotalElements = nnalls;
                }
                else
                {
                    nTotalElements = namix;
                }
            }
            else if (strncmp(varname,"nflux",5) == 0)
            {
                nTotalElements = nnalls;
            }

            if (nTotalElements > nnalls)
            {
                debug4 << mName << "Variable is mixed. Creating mixvar" << endl;

                // Convert the doubles to floats.
                int mixlen = nTotalElements - nnalls;
                debug4 << mName << "mixlen = " << mixlen << endl;
                float *mixvar = new float[mixlen];
                for(int i = 0; i < mixlen; ++i)
                    mixvar[i] = (float)varray[nnalls + i];

                avtMixedVariable *mv = new avtMixedVariable(mixvar,
                                                            mixlen, varname);
                void_ref_ptr vr = void_ref_ptr(mv, avtMixedVariable::Destruct);
                cache->CacheVoidRef(varname, AUXILIARY_DATA_MIXED_VARIABLE, 
                                    0, 0, vr);
                delete [] mixvar;
            }
            else
                debug4 << mName << "Variable is not mixed." << endl;

            delete [] dims;
        }
        else
        {
            debug4 << mName << "Could not get information for " 
                   << varname << endl;
        }
#endif

        return rv;
    }
}


// ****************************************************************************
//  Method: avtCaleFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Rob Managan
//  Creation:   Wed Sep 19 13:30:36 PST 2007
//
// ****************************************************************************

vtkDataArray *
avtCaleFileFormat::GetVectorVar(const char *varname)
{
    return 0;
}

// ***************************************************************************
//  Method: avtCaleFileFormat::GetCycle
//
//  Purpose: Return the cycle associated with this file
//
// ***************************************************************************
int 
avtCaleFileFormat::GetCycle(void) 
{
    int pdberr ;
    int cycle ;
 
    pdberr = PD_read(GetPDBFile(),"/parameters/cycle",&cycle) ;
    debug4 << " cycle " << cycle << endl ;
    return cycle; 
}

// ***************************************************************************
//  Method: avtCaleFileFormat::GetCycleFromFilename
//
//  Purpose: Return the cycle associated with this file
//
// ***************************************************************************

int 
avtCaleFileFormat::GetCycleFromFilename(const char *f) const 
{ 
    int i,j,n,c;
    char cycstr[10];

    n = strlen(f) - 4; // To get here there had to be a ".pdb" on the file

    j = 0;

    for ( i = n-1 ; i  >= 0 ; i-- )
    {
        if ((f[i] >= '0') && (f[i] <= '9'))
        {
            j++;
        }
        else
        {
            break;
        }
    }

    if (j > 0)
    {
        strncpy(cycstr,f+i+1,j);
        c = atoi(cycstr);
    }
    else
    {
        c = -1;
    }

    debug4 << " cycle from name " << cycstr << endl;
    return(c);
}


// ***************************************************************************
//  Method: avtCaleFileFormat::GetTime
//
//  Purpose: Return the time associated with this file
//
// ***************************************************************************

double 
avtCaleFileFormat::GetTime(void) 
{
    int pdberr ;
    double dtime ;
 
    pdberr = PD_read(GetPDBFile(),"/parameters/time",&dtime) ;
    debug4 << " time " << dtime << endl ;
    return dtime; 
}

#include <avtMaterial.h> 
// ***************************************************************************
//  Method: avtCaleFileFormat::GetAuxiliarData
//
//  Purpose: STMD version of GetAuxiliaryData. 
//
// ***************************************************************************

void * 
avtCaleFileFormat::GetAuxiliaryData(const char *var, 
    const char *type, void *, DestructorFunction &df) 
{
    const char *mName = "avtCaleFileFormat::GetAuxiliaryData: ";
    void *retval = 0; 
    debug4 << mName << "type " << type << " var " << var << endl;
    if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0) 
    {
        int i, kmax, lmax, lp, nnalls, namix, pdberr;
        int nreg, nregx, nk, nl, mixmax;
        int dims[3] = {1,1,1}, ndims = 2; 

        debug4 << mName << "Asked to read material information." << endl;
        pdberr = PD_read(GetPDBFile(),"/parameters/kmax",&kmax);
        pdberr = PD_read(GetPDBFile(),"/parameters/lmax",&lmax);
        pdberr = PD_read(GetPDBFile(),"/parameters/lp",&lp);
        pdberr = PD_read(GetPDBFile(),"/parameters/nnalls",&nnalls);
        pdberr = PD_read(GetPDBFile(),"/parameters/namix",&namix);
        pdberr = PD_read(GetPDBFile(),"/parameters/mixmax",&mixmax);
        pdberr = PD_read(GetPDBFile(),"/parameters/nregx",&nregx);
        pdberr = PD_read(GetPDBFile(),"/parameters/nreg",&nreg);

        if (kminmesh == -1)
            GetUsedMeshLimits();
        nk = kmaxmesh - kminmesh + 1; 
        nl = lmaxmesh - lminmesh + 1; 

        debug4 << mName << "kmax = " << kmax << endl;
        debug4 << mName << "lmax = " << lmax << endl;
        debug4 << mName << "lp = " << lp << endl;
        debug4 << mName << "nnalls = " << nnalls << endl;
        debug4 << mName << "namix = " << namix << endl;
        debug4 << mName << "nreg = " << nreg << endl;

        // Structured mesh case 
        ndims = 2; 
        dims[0] = nk-1; 
        dims[1] = nl-1; 
        dims[2] = 1; 
        // Read the number of materials from the file. This 
        // must have already been read from the file when 
        // PopulateDatabaseMetaData was called. 
        int nmats = nreg+1; 
        // The matnos array contains the list of numbers that 
        // are associated with particular materials. For example, 
        // matnos[0] is the number that will be associated with 
        // the first material and any time it is seen in the 
        // matlist array, that number should be taken to mean 
        // material 1. The numbers in the matnos array must 
        // all be greater than or equal to 1. 
        int *matnos = new int[nmats]; 
        // READ nmats INTEGER VALUES INTO THE matnos ARRAY. 
        // Read the material names from your file format or 
        // make up names for the materials. Use the same 
        // approach as when you created material names in 
        // the PopulateDatabaseMetaData method. 
        char **names = new char *[nmats]; 
        //      READ MATERIAL NAMES FROM YOUR FILE FORMAT UNTIL EACH 
        //      ELEMENT OF THE names ARRAY POINTS TO ITS OWN STRING. 
        typedef struct  
        {
            char name[33];
        }  trcname_str;

        if (nreg >= 1)
        {
            debug4 << mName << "nreg>=1, we have a material." << endl;    
            trcname_str *rname = new trcname_str[nreg+1];
       
            pdberr = PD_read(GetPDBFile(),"/ppa/rname",rname);
            strcpy(rname[0].name,"Phony");
         
            for ( i = 0 ; i < nmats ; i++ )
            {
                matnos[i] = i;
                names[i] = rname[i].name;
            }
            matnos[0] = nmats; // can't have a zero material number
       
            // Read the matlist array, which tells what the material 
            // is for each zone in the mesh. 
            int nzones = dims[0] * dims[1] * dims[2]; 
            int *matlist = new int[nnalls]; 
            int *ireg    = new int[namix]; 
            int *mix_mat  = new int[mixmax];
            int *mix_next = new int[mixmax];
            int *mix_zone = new int[mixmax];
            double *zmass = new double[namix]; // volume fractions are in this 

            int maxmixindx, ir, j;
            pdberr = PD_read(GetPDBFile(),"/arrays/ireg",ireg);
            pdberr = PD_read(GetPDBFile(),"/arrays/zmass",zmass);
         
            for ( i = 0 ; i < nnalls ; i++ )
            {
                matlist[i] = 0;
            }
            for ( i = 0 ; i < mixmax ; i++ )
            {
                mix_mat[i] = 0;
                mix_next[i] = 0;
                mix_zone[i] = 0;
            }
            debug4 << mName << "zeroed out arrays." << endl;    

            int *nmatlst = new int[namix];
            int *grdlst  = new int[mixmax];
            pdberr = PD_read(GetPDBFile(),"/arrays/nmatlst",nmatlst);
            pdberr = PD_read(GetPDBFile(),"/arrays/grdlst",grdlst);
         
            int *rlen  = new int[nregx];
            int *rlencln  = new int[nregx];
            int *rlenmix  = new int[nregx];
            pdberr = PD_read(GetPDBFile(),"/arrays/rlen",rlen);
            pdberr = PD_read(GetPDBFile(),"/arrays/rlencln",rlencln);
            pdberr = PD_read(GetPDBFile(),"/arrays/rlenmix",rlenmix);
         
            int **rndx = new int*[nreg+1];
            int **rndxmix = new int*[nreg+1];
            int **rgrdmix = new int*[nreg+1];
            //
            // set regional index pointers
            //
            rlen[0] = rlencln[0];
            rlenmix[0] = 0;
         
            i = 0;
            for ( ir = 0 ; ir <= nreg ; ir++ )
            {
                rndx[ir] = nmatlst + i;
                rndxmix[ir] = rndx[ir] + rlencln[ir];
                i = i + rlen[ir];
                //debug4 << mName << "ir " << ir << " rlen " << rlen[ir] 
                //       <<  " rlenmix " << rlenmix[ir] << " rndx " 
                //       << rndx[ir]-nmatlst << endl;    
            }
         
            i = 0;
            for ( ir = 0 ; ir <= nreg ; ir++ )
            {
                rgrdmix[ir] = grdlst + i;
                i = i + rlenmix[ir];
                // debug4 << mName << "ir " << ir << " rgrdmix " 
                //        << rgrdmix[ir]-grdlst << endl;    
            }
            // 
            //  By setting up the mixed material arrays correctly, we
            //  don't have to reorder the mixed volume fractions or the
            //  mixed values of the zonal variables.  It's more work now,
            //  but it saves time when writing every zonal variable.
            //
            maxmixindx = 0;
            for ( ir = 0; ir <= nreg; ir++)
            {
                matnos[ir]=ir;
                // debug4 << mName << " start region " << ir << endl;    

                // Do clean zones... 
                for (i = 0 ; i < rlencln[ir] ; i++)
                {
                    matlist[rndx[ir][i]] = ir;
                }

                // Do mixed zones... 
                for (i = 0 ; i < rlenmix[ir] ; i++)
                {
                    int done  = 0;
                    int izone = rgrdmix[ir][i];
                    int indx  = rndxmix[ir][i]-nnalls;
                    // debug4 << mName << " ir " << ir << " i " << i 
                    //        << " indx " <<indx << " izone " << izone << endl;

                    // Find the next mixed index for this same zone 
                    for (int jr = ir+1; jr <= nreg && !done; jr++)
                    {
                        // debug4 << mName << " check region " << jr << endl;
                        for (j = 0 ; j < rlenmix[jr] && !done ; j++)
                        {
                            int jzone = rgrdmix[jr][j];
                            int jndx  = rndxmix[jr][j]-nnalls;
                            // debug4 << mName << " jzone " << jzone 
                            //        << " jndx " << jndx << endl;    
                        
                            if (izone == jzone)
                            {
                                // found it -- set it and stop looking 
                                mix_next[indx] = jndx + 1;
                                done=1;
                            }
                        }
                    }
                
                    mix_mat [indx] = ir;
                    // removezoneghost_index converts a CALE zone index into 
                    // one without ghost zones 
                    mix_zone[indx] = 
                            removezoneghost_index(izone,dims[0],dims[1],lp)+1;
                    // debug4 << mName << " izone " << izone << " ->  " 
                    //        << mix_zone[indx] << endl;    
                    // debug4 << mName << " matlist[izone]  " 
                    //        << matlist[izone] << endl;    

                    // If we're the first mixed mat in this zone, 
                    // set the matlist ptr 
                    if (matlist[izone] == 0)
                        matlist[izone] = -indx-1;
                    // debug4 << mName << " matlist[izone]  " 
                    //        << matlist[izone] << endl;    
                    if (indx>maxmixindx)
                        maxmixindx=indx;
                    // debug4 << mName << " maxmixindx " << maxmixindx << endl;    
                }
            }
            // matnos[0] = nmats;

            int *matlistnp=new int[nzones];
            // copy matlist into array matlistnp without ghost zones 
            for (int l = 0 ; l < nl-1 ; l++ )
            {
                for (int k = 0 ; k < nk-1 ; k++ )
                {
                    matlistnp[l*(nk-1) + k] =
                             matlist[(l + lminmesh)*lp + (k + kminmesh)];
                }
            }
            //
            // convert volume fractions to float
            //
            float *matvf = new float[maxmixindx+1];
            for ( i = 0 ; i < maxmixindx+1 ; i++ )
            {
                matvf[i] = zmass[nnalls+i];
            }

            // READ nzones INTEGERS INTO THE matlist array. 
            // Optionally create mix_mat, mix_next, mix_zone, mix_vf 
            // arrays and read their contents from the file format. 
            // Use the information to create an avtMaterial object. 
            avtMaterial *mat = new avtMaterial( 
                                nmats, 
                                matnos, 
                                names, 
                                ndims, 
                                dims, 
                                0, 
                                matlistnp, 
                                maxmixindx+1, // length of mix arrays 
                                mix_mat, // mix_mat array 
                                mix_next, // mix_next array 
                                mix_zone, // mix_zone array 
                                matvf  // mix_vf array 
                                ); 
            // Clean up. 
            delete [] matnos;
            delete [] rname;
            delete [] names; // top
            delete [] matlist;
            delete [] ireg; // bottom 
            delete [] mix_mat;
            delete [] mix_next;
            delete [] mix_zone;
            delete [] zmass;
            delete [] nmatlst;
            delete [] grdlst;
            delete [] rlen;
            delete [] rlencln;
            delete [] rlenmix;
            delete [] rndx;
            delete [] rndxmix;
            delete [] rgrdmix;
            delete [] matlistnp;
            delete [] matvf;
            debug4 << "Returning mat: " << ((void*)mat) << endl;
            // Set the return values. 
            retval = (void *)mat; 
        }
        else
        {
            debug4 << "nreg < 1. No material information" << endl;
        }
        df = avtMaterial::Destruct; 
    } 
    return retval; 
}

// ***************************************************************************
//  Method: avtCaleFileFormat::GetUsedMeshLimits
//
//  Purpose:    find largest k,l used in mesh
//
// ***************************************************************************

void 
avtCaleFileFormat::GetUsedMeshLimits (void)
{
    int ibc, pdberr;
    int kmax, lmax, nbc, nbcx;
   
    pdberr = PD_read(GetPDBFile(),"/parameters/kmax",&kmax);
    pdberr = PD_read(GetPDBFile(),"/parameters/lmax",&lmax);
    pdberr = PD_read(GetPDBFile(),"/parameters/nbc",&nbc);
    pdberr = PD_read(GetPDBFile(),"/parameters/nbcx",&nbcx);
   
    int *bck1 = new int[nbcx];
    int *bck2 = new int[nbcx];
    int *bcl1 = new int[nbcx];
    int *bcl2 = new int[nbcx];
    pdberr = PD_read(GetPDBFile(),"/arrays/bck1",bck1);
    pdberr = PD_read(GetPDBFile(),"/arrays/bck2",bck2);
    pdberr = PD_read(GetPDBFile(),"/arrays/bcl1",bcl1);
    pdberr = PD_read(GetPDBFile(),"/arrays/bcl2",bcl2);

    kminmesh = kmax;
    lminmesh = lmax;
   
    kmaxmesh = 1;
    lmaxmesh = 1;
    // debug4 << "GetUsedMeshLimits: nbc "<< nbc << " nbcx " << nbcx << endl;
      
    for ( ibc = 1 ; ibc <= nbc ; ibc++ )
    {
        //debug4 << " ibc " << ibc << " k1 " << bck1[ibc] << " l1 " << bcl1[ibc]
        //       << " k2 " << bck2[ibc] << " l2 " << bcl2[ibc] << endl;

        kminmesh = (bck1[ibc] < kminmesh) ? bck1[ibc] : kminmesh;
        kminmesh = (bck2[ibc] < kminmesh) ? bck2[ibc] : kminmesh;

        lminmesh = (bcl1[ibc] < lminmesh) ? bcl1[ibc] : lminmesh;
        lminmesh = (bcl2[ibc] < lminmesh) ? bcl2[ibc] : lminmesh;

        kmaxmesh = (bck1[ibc] > kmaxmesh) ? bck1[ibc] : kmaxmesh;
        kmaxmesh = (bck2[ibc] > kmaxmesh) ? bck2[ibc] : kmaxmesh;

        lmaxmesh = (bcl1[ibc] > lmaxmesh) ? bcl1[ibc] : lmaxmesh;
        lmaxmesh = (bcl2[ibc] > lmaxmesh) ? bcl2[ibc] : lmaxmesh;
    }
   
    nnallsmesh = (kmaxmesh - kminmesh + 1)*(lmaxmesh - lminmesh + 1);
   
    delete [] bck1;
    delete [] bck2;
    delete [] bcl1;
    delete [] bcl2;
}


// ***************************************************************************
//  Function:  removezoneghost_index()
//
//  Inputs  :  in (int): the index to convert
//              zk, zl (int): the new dimensions of the array
//              mode : 0 one row of phony zones, 1 none
//
//  Returns :  (int): the converted index
//
//  Abstract:  Transform a 2D index into an array with an extra row of ghost
//             zones around the max k & l edges into a 2D index into an array
//             without them.
// ***************************************************************************

int 
avtCaleFileFormat::removezoneghost_index(int in,int zk,int zl, int lp)
{
    int k,l;

    l=in/lp;    /* calculate Cale's (k,l) values */
    k=in-(l*lp);
   
    if (kminmesh == -1)
        GetUsedMeshLimits() ;
    l -= lminmesh; /* remove unused zones at origin of mesh */
    k -= kminmesh;

    if (k >= zk  ||  l >= zl)
        return (-1);
    else
        return (l*zk + k);
}

