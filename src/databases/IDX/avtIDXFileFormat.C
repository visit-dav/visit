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

// *************************************************************************
//                           avtIDXFileFormat.C
// *************************************************************************

#include <avtIDXFileFormat.h>

#include <string>
#include <sstream>

#include <vtkFloatArray.h>
#include <vtkTypeFloat32Array.h>
#include <vtkDoubleArray.h>
#include <vtkCharArray.h>
#include <vtkShortArray.h>
#include <vtkIntArray.h>
#include <vtkCellType.h>
#include <vtkCellData.h>
#include <vtkLongArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkSmartPointer.h>
#include <vtkHexahedron.h>
#include <vtkUnstructuredGrid.h>
#include <vtkRenderWindow.h>
#include <vtkMatrix4x4.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLDataParser.h>

#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <avtResolutionSelection.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtVariableCache.h>
#include <avtParallel.h>
#include <avtDatabaseMetaData.h>
#include <avtMultiresSelection.h>
#include <avtCallback.h>
#include <avtView2D.h>
#include <avtView3D.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>

#include <visuscpp/db/dataset/visus_db_dataset.h>
#include <visuscpp/kernel/geometry/visus_position.h>
#include <visuscpp/kernel/core/visus_path.h>

#ifdef PARALLEL
#include <avtParallel.h>
#endif

using std::string;
using namespace Visus;

void avtIDXFileFormat::loadBalance(){
    
  //VisusInfo() << "Load balancing";
    
    int maxdir = 0; // largest extent axis
    int maxextent = 0;
    int maxbox = 0;
    
    for(int i=0; i < boxes.size(); i++){
        Box& box = boxes.at(i);
        
        for(int j=0; j < 3; j++){
            int extent = box.p2[j]-box.p1[j];
            if(extent > maxextent){
                maxdir = j;
                maxextent = extent;
                maxbox = i;
            }
        }
    }
    
    //VisusInfo() << "max dir " << maxdir << " max extent " << maxextent << " box " << maxbox;
    
    int total_extent = 0;
    int avg_ext = 0;
    
    for(int i=0; i < boxes.size(); i++){
        Box& box = boxes.at(i);
        
        total_extent += box.p2[maxdir];
    }
    
    avg_ext = total_extent / nprocs;
    int res_ext = total_extent % nprocs;
    
    //VisusInfo() << "tot ext " << total_extent << " avg ext " << avg_ext << " res ext " << res_ext;
    
    std::vector<Box> newboxes;
    
    for(int i=0; i < boxes.size(); i++){
        Box& box = boxes.at(i);
        
        int loc_avg_ext = box.p2[maxdir] - box.p1[maxdir];
        int loc_res = 0;
        
        if(loc_avg_ext > avg_ext){
            loc_res = loc_avg_ext % avg_ext;
            loc_avg_ext = avg_ext;
        }
        
//        VisusInfo() << "local avg ext " << loc_avg_ext << " local res " << loc_res;
        
        int part_p1 = box.p1[maxdir];
        int part_p2 = box.p1[maxdir] + loc_avg_ext;
        
        Point3d p1(box.p1);
        Point3d p2(box.p2);
        
        //VisusInfo() << "Old box p1: " << p1 << " p2: "<< p2;
        
        while(part_p2 <= box.p2[maxdir]){
            
            p1[maxdir] = part_p1;
            p2[maxdir] = part_p2;
            
            Box newbox(p1,p2);
            newboxes.push_back(newbox);
            
            //VisusInfo() << "New box p1: " << p1 << " p2: "<< p2;
            
            part_p1 += loc_avg_ext;
            part_p2 += loc_avg_ext;
           
        }
        
        if(loc_res > 0){
            Box& boxres = newboxes.at(newboxes.size()-1);
            boxres.p2[maxdir] += loc_res;
//            VisusInfo() << "Residual " << loc_res <<" added to box "<< newboxes.size()-1 <<" p1: " << boxres.p1 << " p2: "<< boxres.p2;
        }

    }
    
    boxes.swap(newboxes);

    VisusInfo() << "Total number of boxes/domains: " << boxes.size();
    VisusInfo() << "----------Boxes----------";
    for(int i=0; i< boxes.size(); i++){
        VisusInfo() << i << " = "<< boxes.at(i).p1 << " , " << boxes.at(i).p2;
    }
    VisusInfo() << "-------------------------";
    
}

// TODO consider the physical box
void avtIDXFileFormat::calculateBoundsAndExtents(){
    
    // TODO deallocate this stuff
    for(int i=0; i< boxes.size(); i++){
        Box& box = boxes.at(i);
        int* my_bounds = new int[3];
            
        my_bounds[0] = box.p2.x-box.p1.x;
        my_bounds[1] = box.p2.y-box.p1.y;
        my_bounds[2] = box.p2.z-box.p1.z;
        
        boxes_bounds.push_back(my_bounds);
        
    }
    
}

// ****************************************************************************
//  Method: avtIDXFileFormat constructor
//
//  Programmer: bremer5 -- generated by xml2avt
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

int avtIDXFileFormat::num_instances=0;

avtIDXFileFormat::avtIDXFileFormat(const char *filename)
: avtMTMDFileFormat(filename)
{
#ifdef PARALLEL
    rank = PAR_Rank();
    nprocs = PAR_Size();
#else
    rank = 0;
    nprocs = 1;
#endif
    
    VisusInfo() << "~~~PROC " << rank << " / " << nprocs;
    
    if (num_instances++<1)
    {
        app.reset(new Application);
    }

    //dataflow
    this->dataflow.reset(new Dataflow);

    string name("file://"); name += Path(filename).toString();

    //try to open a dataset
    dataset.reset(Dataset::loadDataset(name));
    if (!dataset)
    {
        VisusError()<<"could not load "<<name;
        VisusAssert(false); //<ctc> this shouldn't be done in the constructor: no way to fail if there is a problem.
    }
    
//    VisusInfo() <<"dataset loaded";
    dim=dataset->getDimension(); //<ctc> //NOTE: it doesn't work like we want. Instead, when a slice (or box) is added, the full data is read from disk then cropped to the desired subregion. Thus, I/O is never avoided.
    
    access.reset(dataset->createAccess());

    dim = dataset->getDimension();
    
    // TODO (if necessary) read only with rank 0 and then broadcast to the other processors
    vtkSmartPointer<vtkXMLDataParser> parser = vtkSmartPointer<vtkXMLDataParser>::New();
    string upsfilename = Path(filename).toString();
    upsfilename.replace(upsfilename.end()-3, upsfilename.end(),"ups");
    
    parser->SetFileName(upsfilename.c_str());
    if (!parser->Parse())
    {
        VisusInfo()<< "No .ups file " << upsfilename;
        multibox = false;
        
        VisusInfo() << "Single-box mode";
    }else{
        multibox = true;
        VisusInfo() << "Multi-box mode";
    }

    if(multibox){
        vtkXMLDataElement *root = parser->GetRootElement();
        vtkXMLDataElement *level = root->FindNestedElementWithName("Grid")->FindNestedElementWithName("Level");
        int nboxes = level->GetNumberOfNestedElements();
        
        VisusInfo() << "Found " << nboxes << " boxes";
        
        for(int i=0; i < nboxes; i++){

            vtkXMLDataElement *xmlbox = level->GetNestedElement(i);
            string lower(xmlbox->FindNestedElementWithName("lower")->GetCharacterData());
            string upper(xmlbox->FindNestedElementWithName("upper")->GetCharacterData());
            string extra_cells(xmlbox->FindNestedElementWithName("extraCells")->GetCharacterData());
            string resolution(xmlbox->FindNestedElementWithName("resolution")->GetCharacterData());
            
            lower = lower.substr(1,lower.length()-2);
            upper = upper.substr(1,upper.length()-2);
            extra_cells = extra_cells.substr(1,extra_cells.length()-2);
            resolution = resolution.substr(1,resolution.length()-2);
            
            //VisusInfo()<< "lower " << lower << " upper " << upper;
            
            Point3d p1;
            Point3d p2;
            int eCells[3];
            int resdata[3];
        
            std::stringstream ress(resolution);
            std::stringstream ss1(lower);
            std::stringstream ss2(upper);
            std::stringstream ssSpace(extra_cells);
            std::string p1s, p2s, espace, res;
            for (int k=0; k < 3; k++){
                std::getline(ss1, p1s, ',');
                std::getline(ss2, p2s, ',');
                std::getline(ssSpace, espace, ',');
                std::getline(ress, res, ',');
                
                eCells[k] = cint(espace);
                resdata[k] = cint(res);

                p1[k] = cfloat(p1s);
                p2[k] = cfloat(p2s);
                
                p1[k] = p1[k] * resdata[k] * (p2[k]-p1[k]);
                p2[k] = p1[k] + resdata[k] + eCells[k] +1;
            }
            
            VisusInfo() <<"Read box: p1 " << p1.toString() << " p2 "<< p2.toString();
            
            boxes.push_back(Box(p1,p2));
            
        }
        
    }
    else{
        boxes.push_back(dataset->getLogicBox().getGeometricBox());
    }
    
    loadBalance();
    calculateBoundsAndExtents();
    
}


// ****************************************************************************
//  Method: avtIDXFileFormat destructor
//
//  Programmer: Cameron Christensen
//  Creation:   Monday, November 04, 2013
//
// ****************************************************************************

avtIDXFileFormat::~avtIDXFileFormat()
{
    VisusInfo()<<"(avtIDXFileFormat destructor)";
    num_instances--;

    disableSlots(this);

    //call this as soon as possible!
//    if (num_instances==0)
//        ;//emitDestroySignal();
    
    //delete selectionsApplied; //don't think we own this...
}

// ****************************************************************************
//  Method: avtIDXFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: spetruzza, bremer5
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

int
avtIDXFileFormat::GetNTimesteps(void)
{
    int NTimesteps = dataset->getTimesteps()->getMax() - dataset->getTimesteps()->getMin();
  
    return std::max(1,NTimesteps); // Needs to return at least 1!!
}


// ****************************************************************************
//  Method: avtIDXFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: spetruzza, bremer5
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

void
avtIDXFileFormat::FreeUpResources(void)
{
    VisusInfo()<<"avtIDXFileFormat::FreeUpResources...";
    //<ctc> todo... something (is destructor also called?)
}


//bool avtIDXFileFormat::CanCacheVariable(const char *var)
//{
//    return false;
//}

// ****************************************************************************
//  Method: avtIDXFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: spetruzza, bremer5
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

void
avtIDXFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timestate) 
{
    VisusInfo() << rank << ": Meta data";

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "Mesh";
    
    mesh->meshType = AVT_RECTILINEAR_MESH;
    
    mesh->numBlocks = boxes.size();
    mesh->blockOrigin = 0;
    mesh->LODs = dataset->getMaxResolution();
    mesh->spatialDimension = dim;
    mesh->topologicalDimension = dim;
    
    mesh->blockTitle = "box";
    mesh->blockPieceName = "box";
    
    // Set bounds and extents for SLIVR rendering
    // TODO use the physical box (logic_to_physic)
    mesh->hasSpatialExtents = true;
    NdBox logicBox = dataset->getLogicBox();
    mesh->minSpatialExtents[0] = logicBox.p1().x;
    mesh->maxSpatialExtents[0] = logicBox.p2().x;
    mesh->minSpatialExtents[1] = logicBox.p1().y;
    mesh->maxSpatialExtents[1] = logicBox.p2().y;
    mesh->minSpatialExtents[2] = logicBox.p1().z;
    mesh->maxSpatialExtents[2] = logicBox.p2().z;
    
    mesh->hasLogicalBounds = true;
    mesh->logicalBounds[0] = logicBox.p2().x - logicBox.p1().x;
    mesh->logicalBounds[1] = logicBox.p2().y - logicBox.p1().y;
    mesh->logicalBounds[2] = logicBox.p2().z - logicBox.p1().z;
    
    md->Add(mesh);
    
    //VisusInfo() << rank << ": Added mesh";

    const std::vector<Field>& fields = dataset->getFields();
    
    int ndtype;
    for (int i = 0; i < (int) fields.size(); i++)
    {
        std::string fieldname = fields[i].name;
        
        Field field = dataset->getFieldByName(fieldname);
        ndtype=1;
        if (field.dtype.isVector())
            ndtype=field.dtype.ncomponents();
        if (ndtype == 1)
            md->Add(new avtScalarMetaData(fieldname,mesh->name,AVT_ZONECENT));
        else
            md->Add(new avtVectorMetaData(fieldname,mesh->name,AVT_ZONECENT,ndtype));
    }
    
    //VisusInfo() << rank << ": Added fields";
        
    avtRectilinearDomainBoundaries *rdb =
    new avtRectilinearDomainBoundaries(true);
    rdb->SetNumDomains(boxes.size());
    
    for (long long i = 0 ; i < boxes.size() ; i++)
    {
        int extents[6];
        extents[0] = boxes.at(i).p1.x;
        extents[1] = boxes.at(i).p2.x;
        extents[2] = boxes.at(i).p1.y;
        extents[3] = boxes.at(i).p2.y;
        extents[4] = boxes.at(i).p1.z;
        extents[5] = boxes.at(i).p2.z;
        
        rdb->SetIndicesForRectGrid(i, extents);
    }
    rdb->CalculateBoundaries();
    
    //VisusInfo() << rank << ": Calculated boundaries";
    
    void_ref_ptr vr = void_ref_ptr(rdb,
                                   avtStructuredDomainBoundaries::Destruct);
    cache->CacheVoidRef("any_mesh",                  AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, -1, -1, vr);

    return;
}


// ****************************************************************************
//  Method: avtIDXFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: spetruzza
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

vtkDataSet *
avtIDXFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
    //VisusInfo()<< rank << ": start getMesh "<< meshname << " domain " << domain;
    
    Box slice_box;
    
    int* my_bounds = NULL;
    int* my_extents = NULL;

    slice_box = boxes.at(domain);
    my_bounds = boxes_bounds.at(domain);
    
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    int dims[3];
    float *arrayX;
    float *arrayZ;
    float *arrayY;
    vtkFloatArray *coordsX;
    vtkFloatArray *coordsY;
    vtkFloatArray *coordsZ;
    
    int my_dims[3];
    
    my_dims[0] = my_bounds[0]+1;
    my_dims[1] = my_bounds[1]+1;
    my_dims[2] = my_bounds[2]+1;
    
//    VisusInfo() << rank << ": dims " << my_dims[0] << " " << my_dims[1] << " " << my_dims[2];
//    VisusInfo() << rank << ": extent " << slice_box.p1.toString() << " " << slice_box.p2.toString();
    
    rgrid->SetDimensions(my_dims[0], my_dims[1], my_dims[2]);
    
    coordsX = vtkFloatArray::New();
    coordsX->SetNumberOfTuples(my_dims[0]);
    arrayX = (float *) coordsX->GetVoidPointer(0);
    
    for (int i = 0; i < my_dims[0]; i++)
        arrayX[i] = slice_box.p1.x +i;
    rgrid->SetXCoordinates(coordsX);
    
    coordsY = vtkFloatArray::New();
    coordsY->SetNumberOfTuples(my_dims[1]);
    arrayY = (float *) coordsY->GetVoidPointer(0);
    for (int i = 0; i < my_dims[1]; i++)
        arrayY[i] = slice_box.p1.y +i;
    rgrid->SetYCoordinates(coordsY);
    
    coordsZ = vtkFloatArray::New();
    coordsZ->SetNumberOfTuples(my_dims[2]);
    arrayZ = (float *) coordsZ->GetVoidPointer(0);
    for (int i = 0; i < my_dims[2]; i++)
        arrayZ[i] = slice_box.p1.z +i;
    rgrid->SetZCoordinates(coordsZ);
    
    //VisusInfo() << "end mesh";
    
    return rgrid;
    
}

//void
//avtIDXFileFormat::GetCycles(std::vector<int> &cycles)
//{
//    int ncycles, *vals = 0;
//    ncycles = OPEN FILE AND READ THE NUMBER OF CYCLES;
//    READ ncycles INTEGER VALUES INTO THE vals ARRAY;
//    // Store the cycles in the vector.
//    for(int i = 0; i < ncycles; ++i)
//        cycles.push_back(vals[i]);
//    delete [] vals;
//}

void
avtIDXFileFormat::GetTimes(std::vector<double> &times)
{
    std::vector<double> tsteps = dataset->getTimesteps()->asVector();
    times.swap(tsteps);
}


// ****************************************************************************
//  Method: avtIDXFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: spetruzza, bremer5
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

vtkDataArray *
avtIDXFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    
  //VisusInfo()<< rank << ": start getvar " << varname << " domain "<< domain;

    if (!dataset->getTimesteps()->containsTimestep(timestate))
        return NULL;
    
    NdBox my_box;
    int zp2 = (dim == 2) ? 1 : boxes.at(domain).p2.z;
    
    NdPoint p1(boxes.at(domain).p1.x,boxes.at(domain).p1.y,boxes.at(domain).p1.z);
    NdPoint p2(boxes.at(domain).p2.x,boxes.at(domain).p2.y,zp2,1,1);
    my_box.setP1(p1);
    my_box.setP2(p2);

    //VisusInfo() << rank << ": Box query " << my_box.p1().toString() << " p2 " << my_box.p2().toString() << " variable " << varname << " time " << timestate;

    int hr = dataset->getMaxResolution();
    
    // TODO Check memory deallocation (it doesn't work for multiple boxes if I use SharedPtr or UniquePtr)
    Query* box_query = new Query(dataset.get(),'r');
//    UniquePtr<Query> box_query(new Query(dataset.get(),'r'));
    box_query->setLogicPosition(my_box);
    box_query->setField(dataset->getFieldByName(varname));

    box_query->setTime(timestate);
    
    box_query->setStartResolution(0);
    box_query->addEndResolution(hr);
    box_query->setMaxResolution(hr);

// -------- This can be used for lower resolution queries
//    box_query->addEndResolution(sres);
//    box_query->addEndResolution(hr);
//    box_query->setMergeMode(Query::InterpolateSamples);
// --------
    
    box_query->setAccess(access.get());
    box_query->begin();

    VisusReleaseAssert(!box_query->end());
    VisusReleaseAssert(box_query->execute());
    
// -------- This can be used for lower resolution queries
//    box_query->next();
//    VisusReleaseAssert(!box_query->end());
// --------
    
//    printf("idx query result (dim %dx%dx%d) = %lld:\n", box_query->getBuffer()->getWidth(), box_query->getBuffer()->getHeight(), box_query->getBuffer()->getDepth(), box_query->getBuffer()->c_size());

    SharedPtr<Array> data = box_query->getBuffer();

    Field field = dataset->getFieldByName(varname);

    int* my_bounds = boxes_bounds.at(domain);
    int ztuples = (dim == 2) ? 1 : (my_bounds[2]);
    long ntuples = (my_bounds[0])*(my_bounds[1])*ztuples;
    
    int ncomponents=1;
    
    // if( data->c_ptr() != NULL)
    //     VisusInfo()<< rank << ": size data bytes " << data->c_size();
    
    // VisusInfo() << rank << ": size array " << ncomponents*ntuples;
    
    if (field.dtype==DTypes::UINT8)
    {
        vtkUnsignedCharArray*rv = vtkUnsignedCharArray::New();
        rv->SetNumberOfComponents(ncomponents); //<ctc> eventually handle vector data, since visit can actually render it!
       
        // TODO check this unmanaged in the new ViSUS
        // data->unmanaged=true; //giving the data to VisIt which will delete it when it's no longer needed
        rv->SetArray((unsigned char*)data->c_ptr(),ncomponents*ntuples,1/*delete when done*/,vtkDataArrayTemplate<unsigned char>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DTypes::UINT16)
    {
        vtkUnsignedShortArray *rv = vtkUnsignedShortArray::New();
        rv->SetNumberOfComponents(ncomponents);
       
        rv->SetArray((unsigned short*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<unsigned short>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DTypes::UINT32)
    {
        vtkUnsignedIntArray *rv = vtkUnsignedIntArray::New();
        rv->SetNumberOfComponents(ncomponents);
       
        rv->SetArray((unsigned int*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<unsigned int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DTypes::UINT32)
    {
        vtkUnsignedLongArray *rv = vtkUnsignedLongArray::New();
        rv->SetNumberOfComponents(ncomponents);
      
        rv->SetArray((unsigned long*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<unsigned long>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DTypes::INT8)
    {
        vtkCharArray*rv = vtkCharArray::New();
        rv->SetNumberOfComponents(ncomponents);
      //  data->unmanaged=true;
        rv->SetArray((char*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<char>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DTypes::INT16)
    {
        vtkShortArray *rv = vtkShortArray::New();
        rv->SetNumberOfComponents(ncomponents);
      //  data->unmanaged=true;
        rv->SetArray((short*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<short>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DTypes::INT32)
    {
        vtkIntArray *rv = vtkIntArray::New();
        rv->SetNumberOfComponents(ncomponents);
    
        rv->SetArray((int*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DTypes::INT64)
    {
        vtkLongArray *rv = vtkLongArray::New();
        rv->SetNumberOfComponents(ncomponents);
     
        rv->SetArray((long*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<long>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DTypes::FLOAT32)
    {
        vtkFloatArray *rv = vtkFloatArray::New();
        rv->SetNumberOfComponents(ncomponents);
        
        rv->SetArray((float*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        
        return rv;
    }
    if (field.dtype==DTypes::FLOAT64)
    {
        vtkDoubleArray *rv = vtkDoubleArray::New();
        rv->SetNumberOfComponents(ncomponents);
        
        rv->SetArray((double*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<double>::VTK_DATA_ARRAY_FREE);
        return rv;
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtIDXFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: spetruzza, bremer5
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

vtkDataArray *
avtIDXFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
{

  //VisusInfo()<< rank << ": start getVectorVar " << varname << " domain "<< domain;
    
    if (!dataset->getTimesteps()->containsTimestep(timestate))
        return NULL;
    
    NdBox my_box;
    int zp2 = (dim == 2) ? 1 : boxes.at(domain).p2.z;
    
    NdPoint p1(boxes.at(domain).p1.x,boxes.at(domain).p1.y,boxes.at(domain).p1.z);
    NdPoint p2(boxes.at(domain).p2.x,boxes.at(domain).p2.y,zp2,1,1);
    
    my_box.setP1(p1);
    my_box.setP2(p2);
    
    //VisusInfo()<<"Box query " << my_box.p1().toString() << " p2 " << my_box.p2().toString();
    
    int hr = dataset->getMaxResolution();
    
    // TODO Check memory deallocation (it doesn't work for multiple boxes if I use SharedPtr or UniquePtr)
    Query* box_query = new Query(dataset.get(),'r');
    //    UniquePtr<Query> box_query(new Query(dataset.get(),'r'));
    box_query->setLogicPosition(my_box);
    box_query->setField(dataset->getFieldByName(varname));
    box_query->setTime(timestate);
    
    box_query->setStartResolution(0);
    box_query->addEndResolution(hr);
    box_query->setMaxResolution(hr);
    
    // -------- This can be used for lower resolution queries
    //    box_query->addEndResolution(sres);
    //    box_query->addEndResolution(hr);
    //    box_query->setMergeMode(Query::InterpolateSamples);
    // --------
    
    box_query->setAccess(access.get());
    box_query->begin();
    VisusReleaseAssert(!box_query->end());
    VisusReleaseAssert(box_query->execute());
    
    // -------- This can be used for lower resolution queries
    //    box_query->next();
    //    VisusReleaseAssert(!box_query->end());
    // --------
    
 //   printf("idx query result (dim %dx%dx%d) = %lld:\n", box_query->getBuffer()->getWidth(), box_query->getBuffer()->getHeight(), box_query->getBuffer()->getDepth(), box_query->getBuffer()->c_size());
    
    SharedPtr<Array> original_data = box_query->getBuffer();
    
    Field field = dataset->getFieldByName(varname);
    
    int* my_bounds = boxes_bounds.at(domain);
    int ztuples = (dim == 2) ? 1 : (my_bounds[2]);
    long ntuples = (my_bounds[0])*(my_bounds[1])*ztuples;
    
    int ncomponents = 3; // Visit works fine only with 3 components
    
    Array* data = original_data.get();
    
    if(dim == 2)
        data = new Array();
    
    // if( data->c_ptr() != NULL)
    //     VisusInfo()<< "size data bytes " << data->c_size();
    
    // VisusInfo() << "size array " << ncomponents*ntuples;
    
    if (field.dtype.isVectorOf(DTypes::UINT8))
    {
        vtkUnsignedCharArray*rv = vtkUnsignedCharArray::New();
        rv->SetNumberOfComponents(ncomponents);
        rv->SetNumberOfTuples(ntuples);
        // TODO check this unmanaged with the new ViSUS
        //data->unmanaged=true;
        
        if(dim == 2){
            if(!Array::convertTo(*data, *original_data, DTypes::UINT8_RGB)){
                VisusInfo() << "Cast to 3d vector failed";
                return NULL;
            }
            
            original_data.reset();
        }
        
        rv->SetArray((unsigned char*)data->c_ptr(),ncomponents*ntuples,1/*delete when done*/,vtkDataArrayTemplate<unsigned char>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype.isVectorOf(DTypes::UINT16))
    {
        vtkUnsignedShortArray *rv = vtkUnsignedShortArray::New();
        rv->SetNumberOfComponents(ncomponents);
        rv->SetNumberOfTuples(ntuples);
        
        if(dim == 2){
            if(!Array::convertTo(*data, *original_data, DTypes::UINT16_RGB)){
                VisusInfo() << "Cast to 3d vector failed";
                return NULL;
            }
            
            original_data.reset();
        }
        
        rv->SetArray((unsigned short*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<unsigned short>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype.isVectorOf(DTypes::UINT32))
    {
        vtkUnsignedLongArray *rv = vtkUnsignedLongArray::New();
        rv->SetNumberOfComponents(ncomponents);
        rv->SetNumberOfTuples(ntuples);

        if(dim == 2){
            if(!Array::convertTo(*data, *original_data, DTypes::UINT32_RGB)){
                VisusInfo() << "Cast to 3d vector failed";
                return NULL;
            }
            
            original_data.reset();
        }
        
        rv->SetArray((unsigned long*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<unsigned long>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype.isVectorOf(DTypes::INT8))
    {
        vtkCharArray*rv = vtkCharArray::New();
        rv->SetNumberOfComponents(ncomponents);
        rv->SetNumberOfTuples(ntuples);
        
        if(dim == 2){
            if(!Array::convertTo(*data, *original_data, DTypes::INT8_RGB)){
                VisusInfo() << "Cast to 3d vector failed";
                return NULL;
            }
            
            original_data.reset();
        }
        
        rv->SetArray((char*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<char>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype.isVectorOf(DTypes::INT16))
    {
        vtkShortArray *rv = vtkShortArray::New();
        rv->SetNumberOfComponents(ncomponents);
        rv->SetNumberOfTuples(ntuples);
        
        if(dim == 2){
            if(!Array::convertTo(*data, *original_data, DTypes::INT16_RGB)){
                VisusInfo() << "Cast to 3d vector failed";
                return NULL;
            }
            
            original_data.reset();
        }
        
        rv->SetArray((short*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<short>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype.isVectorOf(DTypes::INT32))
    {
        vtkIntArray *rv = vtkIntArray::New();
        rv->SetNumberOfComponents(ncomponents);
        rv->SetNumberOfTuples(ntuples);
        
        if(dim == 2){
            if(!Array::convertTo(*data, *original_data, DTypes::INT32_RGB)){
                VisusInfo() << "Cast to 3d vector failed";
                return NULL;
            }
            
            original_data.reset();
        }
        
        rv->SetArray((int*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype.isVectorOf(DTypes::INT64))
    {
        vtkLongArray *rv = vtkLongArray::New();
        rv->SetNumberOfComponents(ncomponents);
        rv->SetNumberOfTuples(ntuples);
        
        if(dim == 2){
            if(!Array::convertTo(*data, *original_data, DTypes::INT64_RGB)){
                VisusInfo() << "Cast to 3d vector failed";
                return NULL;
            }
            
            original_data.reset();
        }
        
        rv->SetArray((long*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<long>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype.isVectorOf(DTypes::FLOAT32))
    {
        vtkFloatArray *rv = vtkFloatArray::New();
        rv->SetNumberOfComponents(ncomponents);
        rv->SetNumberOfTuples(ntuples);
        
        if(dim == 2){
            if(!Array::convertTo(*data, *original_data, DTypes::FLOAT32_RGB)){
                VisusInfo() << "Cast to 3d vector failed";
                return NULL;
            }
            
            original_data.reset();
        }
        
        rv->SetArray((float*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<float>::VTK_DATA_ARRAY_FREE);
        
        return rv;
    }
    if (field.dtype.isVectorOf(DTypes::FLOAT64))
    {
        vtkDoubleArray *rv = vtkDoubleArray::New();
        rv->SetNumberOfComponents(ncomponents);
        rv->SetNumberOfTuples(ntuples);
        
        if(dim == 2){
            if(!Array::convertTo(*data, *original_data, DTypes::FLOAT64_RGB)){
                VisusInfo() << "Cast to 3d vector failed";
                return NULL;
            }
            
            original_data.reset();
        }
        
        rv->SetArray((double*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<double>::VTK_DATA_ARRAY_FREE);
        return rv;
    }

    return NULL;
}
