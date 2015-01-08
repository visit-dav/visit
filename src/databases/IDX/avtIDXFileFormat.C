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

#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkCharArray.h>
#include <vtkShortArray.h>
#include <vtkIntArray.h>
#include <vtkLongArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkRenderWindow.h>
#include <vtkMatrix4x4.h>

#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <avtResolutionSelection.h>
//#include <avtFrustumSelection.h>
#include <avtDatabaseMetaData.h>
#include <avtMultiresSelection.h>
#include <avtCallback.h>
#include <avtView2D.h>
#include <avtView3D.h>
//<fixme>#include <avtView3D.h>
//<fixme>#include <avtWorldSpaceToImageSpaceTransform.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>

#include <visuscpp/db/visus_db.h>

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

using std::string;
VISUS_USE_NAMESPACE

/////////////////////////////////////////////
class DummyNode : public DataflowNode , public IContentHolder
{
 public:

    //constructor
    DummyNode(String name="") : DataflowNode(name)
    {
        addInputPort("data");
        addOutputPort("data"); //if you do not need the original data, simply do not connect it!
    }

    //destructor
    virtual ~DummyNode()
    {}

    //getContentPhysicPosition (from IContentHolder class)
    virtual Position getContentPhysicPosition()
    {
        return Position::invalid();
    }

    //from dataflow interface
    virtual bool processInput()
    {
        VisusAssert(false); //this really shouldn't be called anymore!
        return true;
    }

 private:

    VISUS_DECLARE_NON_COPYABLE(DummyNode);


};


///////////////////////////////////////////////////////////
bool avtIDXQueryNode::publish(const DictObject &evt)
{
    SharedPtr<Array> data  =DynamicPointerCast<Array>   (evt.getattr("data"));
    SharedPtr<Position>dims=DynamicPointerCast<Position>(evt.getattr("dims"));
    SharedPtr<Position> pos=DynamicPointerCast<Position>(evt.getattr("position"));

    if (pos)
    {
        VisusInfo()<<"avtIDXQueryNode::publish: got position: "<<pos->box.toString();
        VisusInfo()<<"                          got dims:     "<<dims->box.toString();
        node->bounds[0] = dims->box.p2.x - dims->box.p1.x;
        node->bounds[1] = dims->box.p2.y - dims->box.p1.y;
        node->bounds[2] = dims->box.p2.z - dims->box.p1.z;

        //if (node->resolution==1) //<ctc> use this to control whether or not to read new position (first few positions are very bad due to camera not being setup properly)
        {
            node->extents[0] = pos->box.p1.x;
            node->extents[1] = pos->box.p2.x;
            node->extents[2] = pos->box.p1.y;
            node->extents[3] = pos->box.p2.y;
            node->extents[4] = pos->box.p1.z;
            node->extents[5] = pos->box.p2.z;
        }
    }
    else if (data)
    {
        VisusInfo()<<"avtIDXQueryNode::publish: got data of resolution <"<<data->dims.x<<","<<data->dims.y<<","<<data->dims.z<<">";
        VisusAssert(node->bounds[0]==data->dims.x);
        VisusAssert(node->bounds[1]==data->dims.y);
        VisusAssert(node->bounds[2]==data->dims.z);
        node->haveData=true;
        node->data=data;
    }
    else
        VisusWarning()<<"Error in avtIDXQueryNode::publish";

    //return QueryNode::publish(evt); //don't even need to send it down the line.
    return true;
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
  first_time = true;
    resolution = -1;
    haveData=false;

    selectionsList = std::vector<avtDataSelection_p>();
    selectionsApplied = NULL; 

#ifdef PARALLEL
    rank = PAR_Rank();
#else
    rank = 0;
#endif

#ifdef PARALLEL
    nblocks = PAR_Size();
#else
    nblocks = 1;
#endif

    if (num_instances++<1)
    {
        app.reset(new Application);
        app->setCommandLine(0,NULL);
        ENABLE_VISUS_DB();
        ENABLE_VISUS_IDX();
    }

    //dataflow
    this->dataflow.reset(new Dataflow);
    this->dataflow->oninput.connect(bind(&avtIDXFileFormat::onDataflowInput,this));

    string name("file://"); name += Path(filename).toString();

    //try to open a dataset
    dataset.reset(Dataset::loadDataset(name));
    if (!dataset)
    {
        VisusError()<<"could not load "<<name;
        VisusAssert(false); //<ctc> this shouldn't be done in the constructor: no way to fail if there is a problem.
    }
    dim=dataset->dimension; //<ctc> //NOTE: it doesn't work like we want. Instead, when a slice (or box) is added, the full data is read from disk then cropped to the desired subregion. Thus, I/O is never avoided.

    //connect dataset
    DatasetNode *dataflow_dataset = new DatasetNode;
    dataflow_dataset->setDataset(dataset);

    //VisusInfo()<<"creating the query node...";
    query=new avtIDXQueryNode(this);

    //position
    {
        Position* position=new Position();
        position->box=dataflow_dataset->getContentPhysicPosition().toAABB();
        if (dim==3)
        {
            position->box=position->box.scaleAroundCenter(1.0);
            //VisusInfo()<<"setting estimate_hz to true";
            query->getInputPort("estimate_hz")->writeValue(SharedPtr<BoolObject>(new BoolObject(false)));
            //VisusInfo()<<"done setting estimate_hz";
            resolution=dataset->max_resolution/2;
        }
        else
        {
            const int ref=2;
            double Z=position->box.center()[ref];
            position->box.p1[ref]=Z;
            position->box.p2[ref]=Z;
            query->getInputPort("estimate_hz")->writeValue(SharedPtr<BoolObject>(new BoolObject(true)));
        }
        query->getInputPort("position")->writeValue(SharedPtr<Object>(position));
    }

    query->setAccessIndex(0);//<ctc> I think default (0) is fine...

    //VisusInfo()<<"adding the dataflow_dataset to the dataflow...";
    dataflow->addNode(dataflow_dataset);
    dataflow->addNode(query);
    this->dataflow->connectNodes(dataflow_dataset,"dataset","dataset",query);

    //only load one level (VisIt doesn't support streaming)
    query->getInputPort("progression")->writeValue(SharedPtr<IntObject>(new IntObject(0)));

    //fieldname
    query->getInputPort("fieldname")->writeValue(SharedPtr<StringObject>(new StringObject(dataset->default_field.name)));

    //position_only (default)
    query->getInputPort("position_only")->writeValue(SharedPtr<BoolObject>(new BoolObject(true)));

    bounds[0] = dataset->logic_box.to.x-dataset->logic_box.from.x+1;
    bounds[1] = dataset->logic_box.to.y-dataset->logic_box.from.y+1;
    bounds[2] = dataset->logic_box.to.z-dataset->logic_box.from.z+1;

    Box physic_box=dataflow_dataset->getContentPhysicPosition().toAABB();
    fullextents[0] = extents[0] = physic_box.p1.x;
    fullextents[1] = extents[1] = physic_box.p2.x;
    fullextents[2] = extents[2] = physic_box.p1.y;
    fullextents[3] = extents[3] = physic_box.p2.y;
    fullextents[4] = extents[4] = physic_box.p1.z;
    fullextents[5] = extents[5] = physic_box.p2.z;
    
    DummyNode *dummy=new DummyNode;
    this->dataflow->addNode(dummy);
    this->dataflow->connectNodes(query,"data","data",dummy);

    //must dispatch once to propagate dataflow_dataset->query connection
    this->dataflow->dispatchPublishedMessages();
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
    if (num_instances==0)
        ;//emitDestroySignal(); 

    //delete selectionsApplied; //don't think we own this...
}

/////////////////////////////////////////////////////////////////////////////
void avtIDXFileFormat::onDataflowInput(DataflowNode *dnode)
{
    //VisusInfo()<<"avtIDXFileFormat::onDataflowInput...";
    if (!dnode)
    {
        VisusAssert(false);
        return;
    }

    //VisusInfo()<<"calling dnode->processInput()...";
    bool ret=dnode->processInput();
    VisusInfo()<<"avtIDXFileFormat::onDataflowInput: dnode->processInput() returned "<<ret;  //true == success
}

// ****************************************************************************
//  Method: avtIDXFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: bremer5 -- generated by xml2avt
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

int
avtIDXFileFormat::GetNTimesteps(void)
{
    int NTimesteps = dataset->time_range.delta()/dataset->time_range.step;
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
//  Programmer: bremer5 -- generated by xml2avt
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

void
avtIDXFileFormat::FreeUpResources(void)
{
    VisusInfo()<<"avtIDXFileFormat::FreeUpResources...";
    //<ctc> todo... something (is destructor also called?)
}


bool avtIDXFileFormat::CanCacheVariable(const char *var)
{
    return false;
}

// ****************************************************************************
//  Method: avtIDXFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: bremer5 -- generated by xml2avt
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

void
avtIDXFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timestate) 
{
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "Mesh";
    mesh->meshType = AVT_RECTILINEAR_MESH;
    mesh->numBlocks = 1;
    mesh->blockOrigin = 0;
    mesh->LODs = dataset->max_resolution;
    mesh->spatialDimension = dim;     
    mesh->topologicalDimension = dim;

    mesh->SetExtents(fullextents);
    mesh->hasSpatialExtents = true;

    md->Add(mesh);

    // dynamic decomposition and multires (<ctc> are both needed or only multires?)
    md->SetFormatCanDoDomainDecomposition(true);
    md->SetFormatCanDoMultires(true);

    std::vector<string> &fieldnames = dataset->fieldnames;
    int ndtype;
    for (int i = 0; i < (int) fieldnames.size(); i++)
    {
        Field field = dataset->getFieldByName(fieldnames[i]);
        ndtype=1;
        if (field.dtype.isVector())
            ndtype=field.dtype.ncomponents();
        if (ndtype == 1)
            md->Add(new avtScalarMetaData(fieldnames[i],mesh->name,AVT_ZONECENT));
        else
            md->Add(new avtVectorMetaData(fieldnames[i],mesh->name,AVT_ZONECENT,ndtype));
    }

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
//  Programmer: bremer5 -- generated by xml2avt
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

vtkDataSet *
avtIDXFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
    NdBox slice_box = dataset->logic_box;

    //
    // Determine the mesh starting location and size of each cell.
    //
    CalculateMesh(timestate);

    //
    // Create the grid.
    //
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    int dims[3];
    float *arrayX;
    float *arrayZ;
    float *arrayY;
    vtkFloatArray *coordsX;
    vtkFloatArray *coordsY;
    vtkFloatArray *coordsZ;
    
    dims[0] = bounds[0] + 1;
    dims[1] = bounds[1] + 1;
    dims[2] = bounds[2] + 1;

    Point3d spacing((extents[1] - extents[0] + 1) / bounds[0],
                    (extents[3] - extents[2] + 1) / bounds[1],
                    (extents[5] - extents[4] + 1) / bounds[2]);

    rgrid->SetDimensions(dims[0], dims[1], dims[2]);

    coordsX = vtkFloatArray::New();
    coordsX->SetNumberOfTuples(dims[0]);
    arrayX = (float *) coordsX->GetVoidPointer(0);
    for (int i = 0; i < dims[0]; i++)
        arrayX[i] = extents[0] + i * spacing.x;
    rgrid->SetXCoordinates(coordsX);

    coordsY = vtkFloatArray::New();
    coordsY->SetNumberOfTuples(dims[1]);
    arrayY = (float *) coordsY->GetVoidPointer(0);
    for (int i = 0; i < dims[1]; i++)
        arrayY[i] = extents[2] + i * spacing.y;
    rgrid->SetYCoordinates(coordsY);

    coordsZ = vtkFloatArray::New();
    coordsZ->SetNumberOfTuples(dims[2]);
    arrayZ = (float *) coordsZ->GetVoidPointer(0);
    for (int i = 0; i < dims[2]; i++)
        arrayZ[i] = extents[4] + i * spacing.z;
    rgrid->SetZCoordinates(coordsZ);

    return rgrid;
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
//  Programmer: bremer5 -- generated by xml2avt
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

vtkDataArray *
avtIDXFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    //
    // Determine the mesh starting location and size of each cell.
    //
    CalculateMesh(timestate);

    string name(varname);
    NdBox slice_box = dataset->logic_box;

    query->getInputPort("fieldname")->writeValue(SharedPtr<StringObject>(new StringObject(varname)));
    query->getInputPort("position_only")->writeValue(SharedPtr<BoolObject>(new BoolObject(false)));
    query->getInputPort("time")->writeValue(SharedPtr<IntObject>(new IntObject(timestate)));

    this->dataflow->oninput.emitSignal(query);

    VisusInfo()<<"query started (Scalar), waiting for data...";
    Clock t0(Clock::now());

    //Ack! What if the onDataflowInput that calls query->processInput returns false?!
    // need to fail gracefully or... should that never happen?

    //wait for the data to arrive.
    haveData=false;
    while (!haveData && t0.msec()<5000) ;//wait

    Clock::timestamp_t msec=t0.msec();
    if (!haveData)
    {
      VisusInfo()<<msec<<"ms passed without getting any data. Returning NULL";
      return NULL;
    }

    VisusInfo()<<msec<<"ms to fetch data, now send it to VisIt.";
    
    Field field = dataset->getFieldByName(varname);
    NdPoint dims(bounds[0],bounds[1],bounds[2],1,1);
    long ntuples = dims.innerProduct();
    int ncomponents=1;
    if (field.dtype==DType::UINT8)
    {
        vtkUnsignedCharArray*rv = vtkUnsignedCharArray::New();
        rv->SetNumberOfComponents(ncomponents); //<ctc> eventually handle vector data, since visit can actually render it!
        data->unmanaged=true; //giving the data to VisIt which will delete it when it's no longer needed
        rv->SetArray((unsigned char*)data->c_ptr(),ncomponents*ntuples,1/*delete when done*/,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::UINT16)
    {
        vtkUnsignedShortArray *rv = vtkUnsignedShortArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((unsigned short*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::UINT32)
    {
        vtkUnsignedIntArray *rv = vtkUnsignedIntArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((unsigned int*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::UINT32)
    {
        vtkUnsignedLongArray *rv = vtkUnsignedLongArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((unsigned long*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::INT8)
    {
        vtkCharArray*rv = vtkCharArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((char*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::INT16)
    {
        vtkShortArray *rv = vtkShortArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((short*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::INT32)
    {
        vtkIntArray *rv = vtkIntArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((int*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::INT64)
    {
        vtkLongArray *rv = vtkLongArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((long*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::FLOAT32)
    {
        vtkFloatArray *rv = vtkFloatArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((float*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::FLOAT64)
    {
        vtkDoubleArray *rv = vtkDoubleArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((double*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
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
//  Programmer: bremer5 -- generated by xml2avt
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

vtkDataArray *
avtIDXFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
{
    //
    // Determine the mesh starting location and size of each cell.
    //
    CalculateMesh(timestate);

    string name(varname);
    NdBox slice_box = dataset->logic_box;

    query->getInputPort("fieldname")->writeValue(SharedPtr<StringObject>(new StringObject(varname)));
    query->getInputPort("position_only")->writeValue(SharedPtr<BoolObject>(new BoolObject(false)));
    query->getInputPort("time")->writeValue(SharedPtr<IntObject>(new IntObject(timestate)));

    this->dataflow->oninput.emitSignal(query);

    VisusInfo()<<"query started (Vector), waiting for data...";
    Clock t0(Clock::now());

    //Ack! What if the onDataflowInput that calls query->processInput returns false?!
    // need to fail gracefully or... should that never happen?

    //wait for the data to arrive.
    haveData=false;
    while (!haveData && t0.msec()<5000) ;//wait

    Clock::timestamp_t msec=t0.msec();
    if (!haveData)
    {
      VisusInfo()<<msec<<"ms passed without getting any data. Returning NULL";
      return NULL;
    }

    VisusInfo()<<msec<<"ms to fetch data, now send it to VisIt.";
    
    Field field = dataset->getFieldByName(varname);
    NdPoint dims(bounds[0],bounds[1],bounds[2],1,1);
    long ntuples = dims.innerProduct();
    int ncomponents=3;
    if (field.dtype==DType::UINT8)
    {
        vtkUnsignedCharArray*rv = vtkUnsignedCharArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((unsigned char*)data->c_ptr(),ncomponents*ntuples,1/*delete when done*/,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::UINT16)
    {
        vtkUnsignedShortArray *rv = vtkUnsignedShortArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((unsigned short*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::UINT32)
    {
        vtkUnsignedIntArray *rv = vtkUnsignedIntArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((unsigned int*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::UINT32)
    {
        vtkUnsignedLongArray *rv = vtkUnsignedLongArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((unsigned long*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::INT8)
    {
        vtkCharArray*rv = vtkCharArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((char*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::INT16)
    {
        vtkShortArray *rv = vtkShortArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((short*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::INT32)
    {
        vtkIntArray *rv = vtkIntArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((int*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::INT64)
    {
        vtkLongArray *rv = vtkLongArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((long*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::FLOAT32)
    {
        vtkFloatArray *rv = vtkFloatArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((float*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    if (field.dtype==DType::FLOAT64)
    {
        vtkDoubleArray *rv = vtkDoubleArray::New();
        rv->SetNumberOfComponents(ncomponents);
        data->unmanaged=true;
        rv->SetArray((double*)data->c_ptr(),ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        return rv;
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtIDXFileFormat::RegisterDataSelections
//
//  Purpose:
//      Tries to read requests for specific resolutions.
//
//  Programmer: Tom Fogal
//  Creation:   August 5, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtIDXFileFormat::RegisterDataSelections(
    const std::vector<avtDataSelection_p>& sels, std::vector<bool>* applied)
{
    this->selectionsList    = sels;
    this->selectionsApplied = applied;
}


// ****************************************************************************
//  Method: avtIDXFileFormat::CalculateMesh
//
//  Purpose:
//    Calculates the parameters defining the mesh for the current multi
//    resolution data selection.
//
//  Arguments:
//    timestate The time state.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Dec 20 12:20:07 PDT 2013
//
//  Modifications:
//
// ****************************************************************************

void
avtIDXFileFormat::CalculateMesh(int timestate)
{
    //
    // Get the multi resolution data selection. Set default values for the
    // transform matrix and cell area in case there isn't a multi resolution
    // data selection.
    //
    double transformMatrix[16] = {DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX,
                                  DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX,
                                  DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX,
                                  DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX};
    double cellArea = 0.002;
    int windowSize[2] = {1024, 1024};

    avtMultiresSelection *selection = NULL;
    for (int i = 0; i < selectionsList.size(); i++)
    {
        if (string(selectionsList[i]->GetType()) == "Multi Resolution Data Selection")
        {
            selection = (avtMultiresSelection *) *(selectionsList[i]);

            selection->GetCompositeProjectionTransformMatrix(transformMatrix);
            cellArea = selection->GetDesiredCellArea();
            selection->GetSize(windowSize);

            (*selectionsApplied)[i] = true;
        }
        else if (string(selectionsList[i]->GetType()) == "avtResolutionSelection")
        {
            const avtResolutionSelection* sel = static_cast<const avtResolutionSelection*>(*selectionsList[i]);
            resolution = sel->resolution();

            (*selectionsApplied)[i] = true;
        }
        else if (string(selectionsList[i]->GetType()) == "Resample Data Selection")
        {
            ;//ignore
        }
    }

    //
    // Set the time.
    //
    query->getInputPort("time")->writeValue(SharedPtr<IntObject>(new IntObject(timestate)));

    //
    // If there isn't a selection then read at a resolution of 20.
    //
    if (transformMatrix[0] == DBL_MAX && transformMatrix[1] == DBL_MAX &&
        transformMatrix[2] == DBL_MAX && transformMatrix[3] == DBL_MAX)
    {
        query->getInputPort("enable_viewdep")->writeValue(SharedPtr<BoolObject>(new BoolObject(false)));
        query->getInputPort("resolution")->writeValue(SharedPtr<IntObject>(new IntObject(20)));
        query->getInputPort("position_only")->writeValue(SharedPtr<BoolObject>(new BoolObject(true)));
        this->dataflow->oninput.emitSignal(query);

        return;
    }

    //
    // Set the frustum for doing a view dependent read.
    //
    Visus::Matrix mvp(transformMatrix);
    Visus::Viewport visus_viewport(0,0,windowSize[0],windowSize[1]);
    SharedPtr<Frustum> visus_frustum(new Frustum);
    visus_frustum->loadProjection(mvp);
    visus_frustum->setViewport(visus_viewport);

    //<ctc> this may be the problem loading session files: these viewports are not yet valid...


    //
    // Set up the view selection.
    //
    if (this->resolution >= 0)
    {
        //
        // Go with the fixed resolution from the MultiresControl.
        //
        query->getInputPort("enable_viewdep")->writeValue(SharedPtr<BoolObject>(new BoolObject(true)));
        if (!first_time)
          query->getInputPort("viewdep")->writeValue(visus_frustum);
        else
          first_time = false;
        query->getInputPort("resolution")->writeValue(SharedPtr<IntObject>(new IntObject(this->resolution)));
        query->getInputPort("position_only")->writeValue(SharedPtr<BoolObject>(new BoolObject(true)));
        this->dataflow->oninput.emitSignal(query);
    }
    else
    {
        //
        // Go with view dependent selections.
        //
        query->getInputPort("enable_viewdep")->writeValue(SharedPtr<BoolObject>(new BoolObject(true)));
        query->getInputPort("viewdep")->writeValue(visus_frustum);

        //
        // Convert the cellArea to a quality for doing a view dependent read.
        //
        int max_size = (1. / cellArea) * (1. / cellArea);
        bool fits    = false;
        int quality  = 1;
        while (!fits && quality > -10)
        {
            quality--;
            query->getInputPort("quality")->writeValue(SharedPtr<IntObject>(new IntObject(quality)));
            query->getInputPort("position_only")->writeValue(SharedPtr<BoolObject>(new BoolObject(true)));
            this->dataflow->oninput.emitSignal(query);
            fits = bounds[0] * bounds[1] * bounds[2] < max_size ? true : false;
        }
    }

    //
    // Set the actual multi resolution selection back into the selection.
    //
    if (selection != NULL)
    {
        //
        // Handle the cases where we end up with a mesh with zero or negative
        // extents in one or both directions.
        //
        extents[0] = std::max(extents[0],fullextents[0]);
        extents[1] = std::min(extents[1],fullextents[1]);
        extents[2] = std::max(extents[2],fullextents[2]);
        extents[3] = std::min(extents[3],fullextents[3]);
        extents[4] = std::max(extents[4],fullextents[4]);
        extents[5] = std::min(extents[5],fullextents[5]);

        double xDelta = (extents[1]-extents[0]+1)/(float)bounds[0];
        double yDelta = (extents[3]-extents[2]+1)/(float)bounds[1];
        double zDelta = (extents[5]-extents[4]+1)/(float)bounds[2];
        cellArea = sqrt(xDelta * xDelta + yDelta * yDelta + zDelta * zDelta);

        selection->SetActualExtents(extents);
        selection->SetActualCellArea(cellArea);
    }
}
