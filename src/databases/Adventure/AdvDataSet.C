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
#include "AdvData.h"

#include <vtkCellTypes.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>

#include <InvalidVariableException.h>
#include <DebugStream.h>

// ****************************************************************************
//  Method: AdvDataSet::VarInfo::CheckFormat
//
//  Purpose:
//      Check the input format string to see if it contains all the same types
//      and how many of them.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 14 15:59:24 PST 2009
//
//  Modifications:
//
// ****************************************************************************

bool
AdvDataSet::VarInfo::CheckFormat(int &ncomps) const
{
    const char *cptr = format.c_str();
    const int nSupportedTypes = 3;
    const char *supportedTypes[] = {"i4", "f4", "f8"};
    int typeCount[3] = {0, 0, 0};
    for(; *cptr != 0; cptr += 2)
    {
        for(int i = 0; i < nSupportedTypes; ++i)
        {
            if(cptr[0] == supportedTypes[i][0] && cptr[1] == supportedTypes[i][1])
                typeCount[i]++;
        }
    }
    int nTypes = 0;
    int lastIndex = 0;
    for(int i = 0; i < nSupportedTypes; ++i)
    {
        if(typeCount[i] > 0)
        {
            ncomps = typeCount[i];
            nTypes++;
        }
    }
    debug5 << "CheckFormat(" << format << ") = " << (nTypes == 1) << ", ncomps=" << ncomps << endl;

    return nTypes == 1;
}

// ****************************************************************************
// Method: AdvDataSet::AdvDataSet
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:09:38 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

AdvDataSet::AdvDataSet() : modelFile(), resultFile(), domainToSubDomain(), 
    nnodes(0), ncells(0)
{
}

// ****************************************************************************
// Method: AdvDataSet::~AdvDataSet
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:09:50 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

AdvDataSet::~AdvDataSet()
{
}

// ****************************************************************************
// Method: AdvDataSet::SetDomainToSubDomain
//
// Purpose: 
//   Set the domain to subdomain mapping.
//
// Arguments:
//   domainID  : The global domain number.
//   subDomain : The index of the subdomain within this AdvDataSet.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:10:01 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
AdvDataSet::SetDomainToSubDomain(int domainID, int subDomain)
{
    domainToSubDomain[domainID] = subDomain;
}

// ****************************************************************************
// Method: AdvDataSet::GetNumSubDomains
//
// Purpose: 
//   Gets the number of subdomains in this AdvDataSet.
//
// Arguments:
//
// Returns:    The number of subdomains in this AdvDataSet.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:10:50 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

int
AdvDataSet::GetNumSubDomains() const
{
    return domainToSubDomain.size();
}

// ****************************************************************************
// Method: AdvDataSet::AddGlobalNodeIds
//
// Purpose: 
//   This method tries to add global node ids to the input dataset if they
//   are present in the file.
//
// Arguments:
//   ds : The dataset to which we want to add global nodeids.
//   f  : The file that contains the global node ids.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:11:40 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
AdvDataSet::AddGlobalNodeIds(vtkDataSet *ds, AdvDocFile *f)
{
    AdvDocument *doc = 0;
    if ((doc = adv_dio_open_by_property(f, 0, 
        "content_type", "FEGenericAttribute", 
        "label", "NodeIndex_PartToGlobal", 0)) != 0)
    {
        const char *format = adv_dio_get_property(doc, "format");
        if(format != 0 && strcmp(format, "i4") == 0)
        {
            vtkIntArray *arr = vtkIntArray::New();
            arr->SetNumberOfTuples(nnodes);
            arr->SetName("avtGlobalNodeId");
            int *ptr = (int *)arr->GetVoidPointer(0);
            adv_off_t offset = 0;
            for(int i = 0; i < nnodes; i++, ptr++)
                offset += adv_dio_read_int32(doc, offset, ptr);

            ds->GetPointData()->AddArray(arr);
            debug5 << "Added global node ids to mesh" << endl;
        }
        adv_dio_close(doc);
    }
}

// ****************************************************************************
// Method: AdvDataSet::GetMesh
//
// Purpose: 
//   Get the mesh for this AdvDataSet.
//
// Arguments:
//   f : The file that contains the mesh.
//   d : The domain number [we translate this to a subdomain, currently unused]
//   elementType : The elementType to use when creating the mesh.
//
// Returns:    An unstructured mesh dataset.
//
// Note:       If we change to returning individual subdomains as separate
//             meshes, I would assemble them all in this call and cache them.
//             Doing that would allow them to all use the same vtkPoints.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:12:29 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
AdvDataSet::GetMesh(AdvDocFile *f, int d, AdvDataSet::AdvElementType elementType)
{
    const char *mName = "AdvDataSet::GetMesh: ";

    // Get this domain.
    int domain = domainToSubDomain[d];

    AdvDocument *doc = 0;

    // ************************************************************************
    // Read nodes
    // ************************************************************************
    if ( (doc = adv_dio_open_by_property(f, 0, "content_type", "Node", 0)) == 0) 
    {
        debug5 << mName << "No content_type[Node]" << endl; 
        return 0;
    }

    this->nnodes = 0;
    if ( !adv_dio_get_property_int32(doc, "num_items", &this->nnodes))  
    {
        debug4 << mName << "No property num_items" << endl; 
        return 0;
    }
    debug4 << mName << "There are " << this->nnodes << " nodes. " << endl;

    vtkPoints *points = vtkPoints::New();
    points->SetDataTypeToDouble();
    points->SetNumberOfPoints(this->nnodes);
    double *pts = (double *) points->GetVoidPointer(0);
    adv_off_t offset = 0;
    for(int i = 0; i < this->nnodes; ++i)
    {
        offset += adv_dio_read_float64(doc, offset, pts);
        offset += adv_dio_read_float64(doc, offset, pts+1);
        offset += adv_dio_read_float64(doc, offset, pts+2);
        pts += 3;
    }
    adv_dio_close(doc);

    // ************************************************************************
    // Read connectivity
    // ************************************************************************
    AdvDocument *nodeIndexDoc = 0;
    if ((nodeIndexDoc = adv_dio_open_by_property(f, 0, 
        "content_type", "HDDM_FEGenericAttribute", 
        "label", "NodeIndex_SubdomainToPart", 0)) == 0)
    {
        points->Delete();
        return 0;
    }

    if ((doc = adv_dio_open_by_property(f, 0, "content_type", "HDDM_Element", 0)) == 0)
    {
        points->Delete();
        debug4 << mName << "No content_type[HDDM_Element]" << endl; 
        return 0;
    }

    int num_subdomains = 0;
    if (!adv_dio_get_property_int32(doc, "num_subdomains", &num_subdomains))  
    {
        points->Delete();
        debug4 << "No property num_subdomains" << endl; 
        return 0;
    }

    int num_nodes_per_element = 0;
    if (!adv_dio_get_property_int32(doc, "num_nodes_per_element", 
        &num_nodes_per_element))  
    {
        points->Delete();
        debug4 << "No property num_nodes_per_element" << endl; 
        return 0;
    }

    this->ncells = 0;
    if (!adv_dio_get_property_int32(doc, "sum_items", 
        &this->ncells))  
    {
        points->Delete();
        debug4 << "No property num_nodes_per_element" << endl; 
        return 0;
    }
    debug4 << mName << "ncells = " << this->ncells << endl;

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New(); 
    ugrid->SetPoints(points);
    points->Delete();
    ugrid->Allocate(this->ncells);
    vtkIdType verts[10];
    offset = 0;

    adv_off_t nodeOffset = 0;
    for (int nd = 0; nd < num_subdomains; nd++)
    {
        // Read the mapping of subdomain nodeids to part nodeids.
        int nnode = 0;
        nodeOffset += adv_dio_read_int32(nodeIndexDoc, nodeOffset, &nnode);
        int *subDomainNodeID2PartNodeID = new int[nnode];
//        debug4 << "Subdomain " << nd << " nodeid to part nodeid mapping." << endl;
        for (int i = 0; i < nnode; i++)
        {
            nodeOffset += adv_dio_read_int32(nodeIndexDoc, nodeOffset, &subDomainNodeID2PartNodeID[i]);
//            debug4 << i << " -> " << subDomainNodeID2PartNodeID[i] << endl;
        }

        // This is the number of cells in the sub_domain but we can
        // skip over it since we're grouping subdomains for now.
        int element_num_items = 0;
        offset += adv_dio_read_int32(doc, offset, &element_num_items);

        // Read the nodeids for the cell and add the cell to the ugrid.
        debug4 << "Subdomain " << nd << " has " << element_num_items << " cells" << endl;
        for (int i = 0; i < element_num_items; i++)
        {
//            debug4 << "    cell " << i << " (";
            for (int j = 0; j < num_nodes_per_element; j++)
            {
                int tmp;
                offset += adv_dio_read_int32(doc, offset, &tmp);
                verts[j] = subDomainNodeID2PartNodeID[tmp];
//                debug4 << " " << verts[j];
            }
//            debug4 << ")\n";

            if(elementType == ADVENTURE_ELEMENT_TET4)
                ugrid->InsertNextCell(VTK_TETRA, 4, verts);
            else if(elementType == ADVENTURE_ELEMENT_TET10)
            {
                // Reorder the connectivity since Adv and VTK use different node ordering.
                vtkIdType conn[10];
                conn[0] = verts[0];
                conn[1] = verts[1];
                conn[2] = verts[2];
                conn[3] = verts[3];
                conn[4] = verts[4];
                conn[5] = verts[7];
                conn[6] = verts[5];
                conn[7] = verts[6];
                conn[8] = verts[9];
                conn[9] = verts[8];
                ugrid->InsertNextCell(VTK_QUADRATIC_TETRA, 10, conn);
            }
            else if(elementType == ADVENTURE_ELEMENT_HEX8)
            {
                // NOTE: There could be vertex ordering issues here. I need test data.
                ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            }
        }

        delete [] subDomainNodeID2PartNodeID;
    }

    adv_dio_close(nodeIndexDoc);
    adv_dio_close(doc);

    // Add global node ids if they are available.
    AddGlobalNodeIds(ugrid, f);

    return ugrid;
}

// ****************************************************************************
// Method: ConvertTensor
//
// Purpose: 
//   This method converts 6-component tensors to 9-component tensors, which
//   VisIt can deal with. Without this step, VisIt was saying that there was
//   no data for the 6-component tensors.
//
// Arguments:
//   input : The input data array.
//
// Returns:    A vtkDataArray that is most of the time the same as the input
//             except for the case of 6-component tensors.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:18:39 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
ConvertTensor(vtkDataArray *input)
{
    vtkDataArray *retval = input;

    if (input->GetNumberOfComponents() == 6)
    {
        vtkDataArray *new_rv = input->NewInstance();
        int ntuples = input->GetNumberOfTuples();
        new_rv->SetNumberOfComponents(9);
        new_rv->SetNumberOfTuples(ntuples);
        for (int i = 0 ; i < ntuples ; i++)
        {
            double orig_vals[6];
            double new_vals[9];
            input->GetTuple(i, orig_vals);
            new_vals[0] = orig_vals[0];  // XX
            new_vals[1] = orig_vals[3];  // XY
            new_vals[2] = orig_vals[5];  // XZ
            new_vals[3] = orig_vals[3];  // YX
            new_vals[4] = orig_vals[1];  // YY
            new_vals[5] = orig_vals[4];  // YZ
            new_vals[6] = orig_vals[5];  // ZX
            new_vals[7] = orig_vals[4];  // ZY
            new_vals[8] = orig_vals[2];  // ZZ
            new_rv->SetTuple(i, new_vals);
        }
        input->Delete();
        retval = new_rv;
    }

    return retval;
}

// ****************************************************************************
// Method: AdvDataSet::GetElementVar
//
// Purpose: 
//   Gets a cell centered variable.
//
// Arguments:
//   f        : The file that contains the variable.
//   domainID : The domain to get [unused currently]
//   var      : Information about the variable we want to get.
//
// Returns:    A VTK data array.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:20:36 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
AdvDataSet::GetElementVar(AdvDocFile *f, int domainID, const AdvDataSet::VarInfo &var)
{
    vtkDataArray *retval = 0;
    int subDomain = domainToSubDomain[domainID];

    if(var.fega_type == "AllElementVariable")
        retval = GetAllElementVariable(f, subDomain, var);
    else if(var.fega_type == "AllElementConstant")
        retval = GetAllConstant(f, subDomain, var, ncells);

    retval = ConvertTensor(retval);

    return retval;
}

// ****************************************************************************
// Method: GetDocument
//
// Purpose: 
//   Gets the document containing the desired variable.
//
// Arguments:
//   f   : The file that contains the variable.
//   var : The name of the variable.
//
// Returns:    The document
//
// Note:       If the document can't be found then an exception is thrown.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:21:53 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

AdvDocument *
GetDocument(AdvDocFile *f, const std::string &var)
{
    AdvDocument *doc = adv_dio_open_by_property(f, 0, 
        "content_type", "HDDM_FEGenericAttribute",
        "label", var.c_str(), 0);
    if(doc == 0)
    {
        doc = adv_dio_open_by_property(f, 0, 
            "content_type", "FEGenericAttribute",
            "label", var.c_str(), 0);
    }
    if(doc == 0)
    {
        EXCEPTION1(InvalidVariableException, var.c_str());
    }
    return doc;
}

// ****************************************************************************
// Method: AdvDataSet::GetAllElementVariable
//
// Purpose: 
//   Gets a cell centered variable that exists over the mesh.
//
// Arguments:
//   f         : The file that contains the data.
//   subDomain : The subDomain that we want [unused currently]
//   var       : Information about the variable we want.
//
// Returns:    A VTK data array.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:23:01 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
AdvDataSet::GetAllElementVariable(AdvDocFile *f, int subDomain,
    const AdvDataSet::VarInfo &var)
{
    AdvDocument *doc = GetDocument(f, var.label);
    vtkDataArray *retval = 0;
    adv_off_t offset = 0;

    int ncomp = 1;
    var.CheckFormat(ncomp);

    std::string format(var.format.substr(0,2));
    if(format == "f8")
    {
        vtkDoubleArray *arr = vtkDoubleArray::New();
        arr->SetNumberOfComponents(ncomp);
        arr->SetNumberOfTuples(ncells);
        double *ptr = (double *)arr->GetVoidPointer(0);
        for(int i = 0; i < GetNumSubDomains(); ++i)
        {
            int valuesPerDomain = 0;
            offset += adv_dio_read_int32(doc, offset, &valuesPerDomain);
            for(int j = 0; j < valuesPerDomain; ++j, ptr += ncomp)
                offset += adv_dio_read_float64v(doc, offset, ncomp, ptr);
        }
        retval = arr;
    }
    else if(format == "f4")
    {
        vtkFloatArray *arr = vtkFloatArray::New();
        arr->SetNumberOfComponents(ncomp);
        arr->SetNumberOfTuples(ncells);
        float *ptr = (float *)arr->GetVoidPointer(0);
        for(int i = 0; i < GetNumSubDomains(); ++i)
        {
            int valuesPerDomain = 0;
            offset += adv_dio_read_int32(doc, offset, &valuesPerDomain);
            for(int j = 0; j < valuesPerDomain; ++j, ptr += ncomp)
                offset += adv_dio_read_float32v(doc, offset, ncomp, ptr);
        }
        retval = arr;
    }
    else if(format == "i4")
    {
        vtkIntArray *arr = vtkIntArray::New();
        arr->SetNumberOfComponents(ncomp);
        arr->SetNumberOfTuples(ncells);
        int *ptr = (int *)arr->GetVoidPointer(0);
        for(int i = 0; i < GetNumSubDomains(); ++i)
        {
            int valuesPerDomain = 0;
            offset += adv_dio_read_int32(doc, offset, &valuesPerDomain);
            for(int j = 0; j < valuesPerDomain; ++j, ptr += ncomp)
                offset += adv_dio_read_int32v(doc, offset, ncomp, ptr);
        }
        retval = arr;
    }
    else
    {
        adv_dio_close(doc);
        EXCEPTION1(InvalidVariableException, var.label.c_str());
    }

    adv_dio_close(doc);

    return retval;
}

// ****************************************************************************
// Method: AdvDataSet::GetAllConstant
//
// Purpose: 
//   Returns a data array filled with a constant from the file.
//
// Arguments:
//   f         : The file that contains the data.
//   subDomain : The subDomain that we want [unused currently]
//   var       : Information about the variable we want.
//   ntuples   : The number of tuples to create in the return data array.
//
// Returns:    A VTK data array.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:24:31 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
AdvDataSet::GetAllConstant(AdvDocFile *f, int subDomain, 
    const AdvDataSet::VarInfo &var, int ntuples)
{
    AdvDocument *doc = GetDocument(f, var.label);
    vtkDataArray *retval = 0;
    adv_off_t offset = 0;

    int ncomp = 1;
    var.CheckFormat(ncomp);

    std::string format(var.format.substr(0,2));
    if(format == "f8")
    {
        vtkDoubleArray *arr = vtkDoubleArray::New();
        arr->SetNumberOfComponents(ncomp);
        arr->SetNumberOfTuples(ntuples);
        double *ptr = (double *)arr->GetVoidPointer(0);
        // Read the 1 data value.
        adv_dio_read_float64v(doc, offset, ncomp, ptr);
        // Replicate it over all cells.
        double *cell0 = ptr;
        ptr += ncomp;
        for(int j = 1; j < ntuples; ++j)
        {
            memcpy(ptr, cell0, ncomp * sizeof(double));
            ptr += ncomp;
        }
        retval = arr;
    }
    else if(format == "f4")
    {
        vtkFloatArray *arr = vtkFloatArray::New();
        arr->SetNumberOfComponents(ncomp);
        arr->SetNumberOfTuples(ntuples);
        float *ptr = (float *)arr->GetVoidPointer(0);
        // Read the 1 data value.
        adv_dio_read_float32v(doc, offset, ncomp, ptr);
        // Replicate it over all cells.
        float *cell0 = ptr;
        ptr += ncomp;
        for(int j = 1; j < ntuples; ++j)
        {
            memcpy(ptr, cell0, ncomp * sizeof(float));
            ptr += ncomp;
        }
        retval = arr;
    }
    else if(format == "i4")
    {
        vtkIntArray *arr = vtkIntArray::New();
        arr->SetNumberOfComponents(ncomp);
        arr->SetNumberOfTuples(ntuples);
        int *ptr = (int *)arr->GetVoidPointer(0);
        // Read the 1 data value.
        adv_dio_read_int32v(doc, offset, ncomp, ptr);
        // Replicate it over all cells.
        int *cell0 = ptr;
        ptr += ncomp;
        for(int j = 1; j < ntuples; ++j)
        {
            memcpy(ptr, cell0, ncomp * sizeof(int));
            ptr += ncomp;
        }
        retval = arr;
    }
    else
    {
        adv_dio_close(doc);
        EXCEPTION1(InvalidVariableException, var.label.c_str());
    }

    adv_dio_close(doc);

    return retval;
}

// ****************************************************************************
// Method: AdvDataSet::GetNodeVar
//
// Purpose: 
//   Returns a node centered data array
//
// Arguments:
//   f            : The file that contains the data.
//   nodeIndexDoc : The document that contains the subDomain node index to 
//                  part node index.
//   domainID     : The domain that we want [unused currently]
//   var          : Information about the variable we want.
//
// Returns:    A VTK data array.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:24:31 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
AdvDataSet::GetNodeVar(AdvDocFile *f, AdvDocument *nodeIndexDoc, int domainID, 
    const AdvDataSet::VarInfo &var)
{
    vtkDataArray *retval = 0;
    int subDomain = domainToSubDomain[domainID];

    if(var.fega_type == "AllNodeVariable")
        retval = GetAllNodeVariable(f, nodeIndexDoc, subDomain, var);
    else if(var.fega_type == "AllNodeConstant")
        retval = GetAllConstant(f, subDomain, var, nnodes);

    retval = ConvertTensor(retval);

    return retval;
}

// ****************************************************************************
// Method: ReadSubDomainNodeID2PartNodeID
//
// Purpose: 
//   Reads the subdomain nodeid to part nodeid mapping.
//
// Arguments:
//   nodeIndexDoc : The dpcument that contains the mapping.
//   nodeOffset   : The offset being used to read the mapping.
//
// Returns:    An int array containing the mapping.
//
// Note:       The return array must be freed by the caller.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:27:45 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

int *
ReadSubDomainNodeID2PartNodeID(AdvDocument *nodeIndexDoc, adv_off_t &nodeOffset)
{
    // Read the mapping of subdomain nodeids to part nodeids.
    int nnode = 0;
    nodeOffset += adv_dio_read_int32(nodeIndexDoc, nodeOffset, &nnode);

    int *subDomainNodeID2PartNodeID = new int[nnode];
    for (int i = 0; i < nnode; i++)
        nodeOffset += adv_dio_read_int32(nodeIndexDoc, nodeOffset, &subDomainNodeID2PartNodeID[i]);

    return subDomainNodeID2PartNodeID;
}

// ****************************************************************************
// Method: AdvDataSet::GetAllNodeVariable
//
// Purpose: 
//   Gets a node centered variable that exists over the mesh.
//
// Arguments:
//   f            : The file that contains the data.
//   nodeIndexDoc : The document that contains the subDomain node index to 
//                  part node index.
//   subDomain    : The subDomain that we want [unused currently]
//   var          : Information about the variable we want.
//
// Returns:    A VTK data array.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:23:01 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
AdvDataSet::GetAllNodeVariable(AdvDocFile *f, AdvDocument *nodeIndexDoc, 
    int subDomain, const AdvDataSet::VarInfo &var)
{
    AdvDocument *doc = GetDocument(f, var.label);
    vtkDataArray *retval = 0;
    adv_off_t offset = 0;
    adv_off_t nodeOffset = 0;

    int ncomp = 1;
    var.CheckFormat(ncomp);

    std::string format(var.format.substr(0,2));
    if(format == "f8")
    {
        vtkDoubleArray *arr = vtkDoubleArray::New();
        arr->SetNumberOfComponents(ncomp);
        arr->SetNumberOfTuples(nnodes);
        double *ptr = (double *)arr->GetVoidPointer(0);
        memset(ptr, 0, sizeof(double) * nnodes * ncomp);
        double *tmp = new double[ncomp];
        for(int i = 0; i < GetNumSubDomains(); ++i)
        {
            int *subDomainNodeID2PartNodeID = ReadSubDomainNodeID2PartNodeID(nodeIndexDoc, nodeOffset);

            int nnodesInSubDomain = 0;
            offset += adv_dio_read_int32(doc, offset, &nnodesInSubDomain);

            for(int nodeid = 0; nodeid < nnodesInSubDomain; ++nodeid)
            {
                offset += adv_dio_read_float64v(doc, offset, ncomp, tmp);
                int partnodeid = subDomainNodeID2PartNodeID[nodeid];
                memcpy(ptr + partnodeid * ncomp, tmp, ncomp * sizeof(double));
            }

            delete [] subDomainNodeID2PartNodeID;
        }
        delete [] tmp;
        retval = arr;
    }
    else if(format == "f4")
    {
        vtkFloatArray *arr = vtkFloatArray::New();
        arr->SetNumberOfComponents(ncomp);
        arr->SetNumberOfTuples(nnodes);
        float *ptr = (float *)arr->GetVoidPointer(0);
        memset(ptr, 0, sizeof(float) * nnodes * ncomp);
        float *tmp = new float[ncomp];
        for(int i = 0; i < GetNumSubDomains(); ++i)
        {
            int *subDomainNodeID2PartNodeID = ReadSubDomainNodeID2PartNodeID(nodeIndexDoc, nodeOffset);

            int nnodesInSubDomain = 0;
            offset += adv_dio_read_int32(doc, offset, &nnodesInSubDomain);

            for(int nodeid = 0; nodeid < nnodesInSubDomain; ++nodeid)
            {
                offset += adv_dio_read_float32v(doc, offset, ncomp, tmp);
                int partnodeid = subDomainNodeID2PartNodeID[nodeid];
                memcpy(ptr + partnodeid * ncomp, tmp, ncomp * sizeof(float));
            }

            delete [] subDomainNodeID2PartNodeID;
        }
        delete [] tmp;
        retval = arr;
    }
    else if(format == "i4")
    {
        vtkIntArray *arr = vtkIntArray::New();
        arr->SetNumberOfComponents(ncomp);
        arr->SetNumberOfTuples(nnodes);
        int *ptr = (int *)arr->GetVoidPointer(0);
        memset(ptr, 0, sizeof(int) * nnodes * ncomp);
        int *tmp = new int[ncomp];
        for(int i = 0; i < GetNumSubDomains(); ++i)
        {
            int *subDomainNodeID2PartNodeID = ReadSubDomainNodeID2PartNodeID(nodeIndexDoc, nodeOffset);

            int nnodesInSubDomain = 0;
            offset += adv_dio_read_int32(doc, offset, &nnodesInSubDomain);

            for(int nodeid = 0; nodeid < nnodesInSubDomain; ++nodeid)
            {
                offset += adv_dio_read_int32v(doc, offset, ncomp, tmp);
                int partnodeid = subDomainNodeID2PartNodeID[nodeid];
                memcpy(ptr + partnodeid * ncomp, tmp, ncomp * sizeof(int));
            }

            delete [] subDomainNodeID2PartNodeID;
        }
        delete [] tmp;
        retval = arr;
    }
    else
    {
        adv_dio_close(doc);
        EXCEPTION1(InvalidVariableException, var.label.c_str());
    }

    adv_dio_close(doc);

    return retval;
}

