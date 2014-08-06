/*****************************************************************************
*
* Copyright (c) 2000 - 2013, Lawrence Livermore National Security, LLC
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

#if 1
    string name("file://"); name += Path(filename).toString();
#else
    //<ctc> todo: add http:// url to ".urlidx" file and read it, or just open selected file directly
    string name(Path(filename).getFileNameWithoutExtension());
    name="http://atlantis.sci.utah.edu/mod_visus?dataset="+name; //<ctc> works!
#endif

    //try to open a dataset
    dataset.reset(Dataset::loadDataset(name));
    if (!dataset)
    {
        VisusError()<<"could not load "<<name;
        VisusAssert(false); //<ctc> this shouldn't be done in the constructor: no way to fail if there is a problem.
    }
    dim=dataset->dimension; //<ctc> how do we tell it when to extract only a slice? maybe it just works...

    //connect dataset
    DatasetNode *dataflow_dataset = new DatasetNode;
    dataflow_dataset->setDataset(dataset);

    VisusInfo()<<"creating the query node...";
    query=new avtIDXQueryNode(this);

    //position
    {
        Position* position=new Position();
        position->box=dataflow_dataset->getContentPhysicPosition().toAABB();
        if (dim==3)
        {
            position->box=position->box.scaleAroundCenter(1.0);
        }
        else
        {
            const int ref=2;
            double Z=position->box.center()[ref];
            position->box.p1[ref]=Z;
            position->box.p2[ref]=Z;
        }
        query->getInputPort("position")->writeValue(SharedPtr<Object>(position));
    }

    query->setAccessIndex(0);//<ctc> I think default (0) is fine...

    VisusInfo()<<"adding the dataflow_dataset to the dataflow...";
    dataflow->addNode(dataflow_dataset);
    dataflow->addNode(query);
    this->dataflow->connectNodes(dataflow_dataset,"dataset","dataset",query);

    //only load one level (VisIt doesn't support streaming)
    query->getInputPort("progression")->writeValue(SharedPtr<IntObject>(new IntObject(0)));

    //fieldname
    query->getInputPort("fieldname")->writeValue(SharedPtr<StringObject>(new StringObject(dataset->default_field.name)));

    //position_only (default)
    query->getInputPort("position_only")->writeValue(SharedPtr<BoolObject>(new BoolObject(true)));

    VisusInfo()<<"querying the bounds...";
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
    
    NdPoint dims(bounds[0],bounds[1],bounds[2],1,1);
    NdBox   exts(NdPoint(extents[0],extents[2],extents[4]),NdPoint(extents[1],extents[3],extents[5]));
    VisusInfo()<<"bounds:  "<<dims.toString();
    VisusInfo()<<"extents: "<<exts.toString();

    VisusInfo()<<"attaching a dummy node (to force processing, otherwise no output is produced)";
    DummyNode *dummy=new DummyNode;
    this->dataflow->addNode(dummy);
    this->dataflow->connectNodes(query,"data","data",dummy);

    //must dispatch once to propagate dataflow_dataset->query connection
    this->dataflow->dispatchPublishedMessages();

    VisusInfo()<<"end of constructor!";
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
    VisusInfo()<<"avtIDXFileFormat::onDataflowInput: dnode->processInput() returned "<<ret;
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
    //VisusInfo()<<"avtIDXFileFormat::GetNTimesteps...";
    int NTimesteps = dataset->time_range.delta()/dataset->time_range.step;
    //VisusInfo()<<"range: "<<dataset->time_range.delta()<<",step: "<<dataset->time_range.step;
    //VisusInfo()<<"\tnum_timesteps="<<NTimesteps;
    return std::max(1,NTimesteps); //<ctc> needs to return at least 1!!
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


///////////////////////////////////////////////////////////
Frustum* calcFrustum(double tileXmin, double tileXmax,
                     double tileYmin, double tileYmax,int dim)
{
    VisusInfo()<<"Calculating local frustum based on world frustum of <L,R,B,T>=<"<<tileXmin<<","<<tileXmax<<","<<tileYmin<<","<<tileYmax<<">";

    UniquePtr<Frustum> frustum(new Frustum);
    if (dim==2)
    {
        GLOrthoCamera camera;
        const int *sz=avtCallback::GetCurrentWindowAtts().GetSize();    
        camera.setViewport(Viewport(0,0,sz[0],sz[1]));
        GLOrthoParams params(tileXmin,tileXmax,tileYmin,tileYmax,-10,10);
        camera.setOrthoParams(params);
        frustum.reset(new Frustum(camera.getFrustum()));

        const GLOrthoParams& orthoParams(camera.getOrthoParams());
        VisusInfo()<<"local (ortho) frustum is <L,R,B,T>=<"<<orthoParams.left<<","<<orthoParams.right<<","<<orthoParams.bottom<<","<<orthoParams.top<<">";
    }
    else if (dim==3)
    {
        //<ctc> todo: handle 3d
    }

#if 0
    //get window attributes, then use avtViewInfo because it's a more direct mapping
    const View3DAttributes &atts3d=avtCallback::GetCurrentWindowAtts().GetView3D();
    //<fixme> avtView3D v3d;  v3d.SetFromView3DAttributes(&atts3d);
    //<fixme> avtViewInfo vi; v3d.SetViewInfoFromView(vi);
    //ViewInfo vi; vi.SetFromView3D(atts3d);

    double scale[3] = {1,1,1};
    const int *sz=avtCallback::GetCurrentWindowAtts().GetSize();    
    float aspect=(float)sz[0]/(float)sz[1];
    vtkMatrix4x4 *transform = vtkMatrix4x4::New();
    //<fixme> avtWorldSpaceToImageSpaceTransform::CalculateTransform(vi, transform, scale, aspect);

    frustum->setViewport(Viewport(0,0,sz[0],sz[1]));
    frustum->loadProjection(Matrix((double*)(&transform->Element[0])));
    frustum->loadModelview(Matrix::identity());

#if 0 //<fixme>
    Point3d pan(vi.imagePan[0]*sz[0],vi.imagePan[1]*sz[1],0);
    Point3d pos(vi.camera[0],vi.camera[2],vi.camera[2]);
    Point3d la(vi.focus[0],vi.focus[1],vi.focus[2]);
    Point3d vup(vi.viewUp[0],vi.viewUp[1],vi.viewUp[2]);
#if 0
    frustum->loadModelview(Matrix::lookAt(pos,la,vup));
    frustum->loadProjection(Matrix::perspective(vi.viewAngle,aspect,vi.nearPlane,vi.farPlane));
#endif
 
    VisusInfo()<<"viewport:   "<<frustum->getViewport().toString();
    VisusInfo()<<"modelview:  "<<frustum->getModelview().toString();
    VisusInfo()<<"projection: "<<frustum->getProjection().toString();

    VisusInfo()<<"created from...";
    VisusInfo()<<"\tcamera.pos: "<<pos;
    VisusInfo()<<"\tcamera.la:  "<<la;
    VisusInfo()<<"\tcamera.vup: "<<vup;
    VisusInfo()<<"\tpan*window: "<<pan;
    VisusInfo()<<"\tcamera.fov  "<<vi.viewAngle;
    VisusInfo()<<"\taspect      "<<aspect;
    VisusInfo()<<"\tcamera.pan: "<<vi.imagePan[0]<<","<<vi.imagePan[1];
    VisusInfo()<<"\tcamera.zoom:"<<vi.imageZoom;
    VisusInfo()<<"\twindow.w:   "<<sz[0];
    VisusInfo()<<"\twindow.h:   "<<sz[1];
    VisusInfo()<<"\tnearPlane:  "<<vi.nearPlane;
    VisusInfo()<<"\tfarPlane:   "<<vi.farPlane;
#endif
#endif
    
    return frustum.release();
}

bool avtIDXFileFormat::CanCacheVariable(const char *var)
{
    //VisusInfo()<<"avtIDXFileFormat::CanCacheVariable("<<var<<")";
    return false;
    //return std::string(var)!="CC_Mesh";
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
    VisusInfo()<<"avtIDXFileFormat::PopulateDatabaseMetaData(timestate("<<timestate<<"))";

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "Mesh";
    mesh->meshType = AVT_RECTILINEAR_MESH;
    mesh->numBlocks = 1;
    mesh->blockOrigin = 0;
    mesh->LODs = dataset->max_resolution;
    mesh->spatialDimension = dim;     
    mesh->topologicalDimension = dim;

    {
        NdPoint dims(bounds[0],bounds[1],bounds[2],1,1);
        //NdBox   exts(NdPoint(extents[0],extents[2],extents[4]),NdPoint(extents[1],extents[3],extents[5]));
        NdBox   exts(NdPoint(fullextents[0],fullextents[2],fullextents[4]),NdPoint(fullextents[1],fullextents[3],fullextents[5]));
        VisusInfo()<<"PopulateDatabaseMetaData: bounds:  "<<dims.toString();
        VisusInfo()<<"PopulateDatabaseMetaData: extents: "<<exts.toString();
    }

    //<ctc> maybe these are tripping things up for panning...
    // mesh->SetBounds(bounds);
    // mesh->hasLogicalBounds = true;

    mesh->SetExtents(fullextents);  //mrtest always sets full extents, so let's try it...
    //mesh->SetExtents(extents);
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

    //resolution=avtCallback::idx_get_resolution_hack();
    //cerr<<"resolution is at "<<resolution<<endl;

    VisusInfo() << "avtIDXFileFormat::GetMesh(timestate("<<timestate<<") domain("<<domain<<") meshname("<<meshname<<") resolution("<<resolution<<"))";

    //
    // Determine the mesh starting location and size of each cell.
    //
    // double tileXmin, tileXmax, tileYmin, tileYmax;
    // int nx, ny;

    CalculateMesh(timestate);//tileXmin, tileXmax, tileYmin, tileYmax,timestate);


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
    
    dims[0] = bounds[0]+1; //visit is so weird... if I have a x by y array, it wants me to say it's x+1 by y+1 :P
    dims[1] = bounds[1]+1;
    dims[2] = bounds[2]+1;

    {
        NdPoint dims(bounds[0],bounds[1],bounds[2],1,1);
        NdBox   exts(NdPoint(extents[0],extents[2],extents[4]),NdPoint(extents[1],extents[3],extents[5]));
        VisusInfo()<<"GetMesh: bounds:  "<<dims.toString();
        VisusInfo()<<"GetMesh: extents: "<<exts.toString();
    }

    // Point3d spacing((fullextents[1]-fullextents[0]+1)/bounds[0],(fullextents[3]-fullextents[2]+1)/bounds[1],(fullextents[5]-fullextents[4]+1)/bounds[2]);
    // if (resolution==1) //see note in publish above
    Point3d spacing((extents[1]-extents[0]+1)/bounds[0],(extents[3]-extents[2]+1)/bounds[1],(extents[5]-extents[4]+1)/bounds[2]);

    rgrid->SetDimensions(dims[0], dims[1], dims[2]);
    VisusInfo()<<"avtIDXFileFormat::GetMesh() returning <"<<dims[0]<<"x"<<dims[1]<<"x"<<dims[2]<<"> mesh";//for resolution "<<resolution<<" (resReduction="<<resReduction<<")";

    coordsX = vtkFloatArray::New();
    coordsX->SetNumberOfTuples(dims[0]);
    arrayX = (float *) coordsX->GetVoidPointer(0);
    for (int i = 0; i < dims[0]; i++)
        //arrayX[i] = i * resReduction;
        arrayX[i] = extents[0]+i*spacing.x;
    //arrayX[dims[0]-1] = slice_box.to[0] + 1.; //<ctc> maybe still need something like this
    rgrid->SetXCoordinates(coordsX);

    coordsY = vtkFloatArray::New();
    coordsY->SetNumberOfTuples(dims[1]);
    arrayY = (float *) coordsY->GetVoidPointer(0);
    for (int i = 0; i < dims[1]; i++)
        //arrayY[i] = i * resReduction;
        arrayY[i] = extents[2]+i*spacing.y;
    //arrayY[dims[1]-1] = slice_box.to[1] + 1.; //<ctc> maybe still need something like this
    rgrid->SetYCoordinates(coordsY);

    coordsZ = vtkFloatArray::New();
    coordsZ->SetNumberOfTuples(dims[2]);
    arrayZ = (float *) coordsZ->GetVoidPointer(0);
    for (int i = 0; i < dims[2]; i++)
        //arrayZ[i] = slice_box.from[2] + i * resReduction;
        arrayZ[i] = extents[4]+i*spacing.z;
    //arrayZ[dims[2]-1] = slice_box.to[2] + 1.; //<ctc> maybe still need something like this
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
    //resolution=avtCallback::idx_get_resolution_hack();
    //VisusInfo()<<"resolution is at "<<resolution<<endl;
    VisusInfo() << "avtIDXFileFormat::GetVar(timestate("<<timestate<<") domain("<<domain<<") varname("<<varname<<") resolution("<<resolution<<"))";

    //
    // Determine the mesh starting location and size of each cell.
    //
    // double tileXmin, tileXmax, tileYmin, tileYmax;
    // int nx, ny;

    CalculateMesh(timestate);//tileXmin, tileXmax, tileYmin, tileYmax, timestate);

    string name(varname);
    NdBox slice_box = dataset->logic_box;

    query->getInputPort("fieldname")->writeValue(SharedPtr<StringObject>(new StringObject(varname)));
    query->getInputPort("position_only")->writeValue(SharedPtr<BoolObject>(new BoolObject(false)));
    query->getInputPort("time")->writeValue(SharedPtr<IntObject>(new IntObject(timestate)));

    this->dataflow->oninput.emitSignal(query);

    VisusInfo()<<"query started, waiting for data...";
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
      return NULL;//vtkUnsignedShortArray *rv = vtkUnsignedShortArray::New();
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
    VisusInfo()<<"avtIDXFileFormat::GetVectorVar(timestate("<<timestate<<") domain("<<domain<<") varname("<<varname<<"))";

    //
    // Determine the mesh starting location and size of each cell.
    //
    // double tileXmin, tileXmax, tileYmin, tileYmax;
    // int nx, ny;

    CalculateMesh(timestate);//tileXmin, tileXmax, tileYmin, tileYmax, timestate);

    string name(varname);
    NdBox slice_box = dataset->logic_box;

    query->getInputPort("fieldname")->writeValue(SharedPtr<StringObject>(new StringObject(varname)));
    query->getInputPort("position_only")->writeValue(SharedPtr<BoolObject>(new BoolObject(false)));
    query->getInputPort("time")->writeValue(SharedPtr<IntObject>(new IntObject(timestate)));

    this->dataflow->oninput.emitSignal(query);

    VisusInfo()<<"query started, waiting for data...";
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
      return NULL;//vtkUnsignedShortArray *rv = vtkUnsignedShortArray::New();
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
    VisusInfo()<<"avtIDXFileFormat::RegisterDataSelections";
    this->selectionsList    = sels;
    this->selectionsApplied = applied;
}


// ****************************************************************************
//  Method: avtMRTestFileFormat::CalculateMesh
//
//  Purpose:
//    Calculates the parameters defining the mesh for the current multi
//    resolution data selection.
//
//  Arguments:
//    tileXmin  The tile aligned minimum X value of the mesh.
//    tileXmax  The tile aligned maximum X value of the mesh.
//    tileYmin  The tile aligned minimum Y value of the mesh.
//    tileYmax  The tile aligned maximum Y value of the mesh.
//    nx        The number of zones in the X direction.
//    ny        The number of zones in the Y direction.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Dec 20 12:20:07 PDT 2013
//
//  Modifications:
//
// ****************************************************************************

void
avtIDXFileFormat::CalculateMesh(/*double &tileXmin, double &tileXmax,
                                  double &tileYmin, double &tileYmax, */
                                int timestate)
{
    VisusInfo()<<"avtIDXFileFormat::CalculateMesh(timestep="<<timestate<<")";
    //
    // Get the multi resolution data selection.
    //
    double transformMatrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    double viewArea = (fullextents[1]-fullextents[0]) * (fullextents[3]-fullextents[2]);
    double desired_extents[6] = {this->fullextents[0],this->fullextents[1],this->fullextents[2],this->fullextents[3],this->fullextents[4],this->fullextents[5]};
    double cellArea = .002;

    double viewport[6] = {this->fullextents[0],this->fullextents[1],this->fullextents[2],this->fullextents[3],this->fullextents[4],this->fullextents[5]};
    int windowSize[2] = {0,0};

    avtMultiresSelection *selection = NULL;
    for (int i = 0; i < selectionsList.size(); i++)
    {
        VisusInfo()<<"avtIDXFileFormat: selection type: "<<selectionsList[i]->GetType();
        if (string(selectionsList[i]->GetType()) == "Multi Resolution Data Selection")
        {
            selection = (avtMultiresSelection *) *(selectionsList[i]);

            selection->GetCompositeProjectionTransformMatrix(transformMatrix);
            cellArea = selection->GetDesiredCellArea();

            viewArea = selection->GetViewArea();
            selection->GetDesiredExtents(desired_extents);
            selection->GetViewport(viewport);
            selection->GetSize(windowSize);

            VisusInfo()<<"\n\tcellArea: "<<cellArea
                       <<"\n\tviewArea: "<<viewArea
                       <<"\n\twindowSize: "<<windowSize[0]<<","<<windowSize[1]
                       <<"\n\tviewport: "<<viewport[0]<<","<<viewport[1]<<","<<viewport[2]<<","<<viewport[3]<<","<<viewport[4]<<","<<viewport[5]
                       <<"\n\tdesired_extents: "<<desired_extents[0]<<","<<desired_extents[1]<<","<<desired_extents[2]<<","<<desired_extents[3]<<","<<desired_extents[4]<<","<<desired_extents[5]
                       <<"\n\tMVP matrix\n\t\t: "<<transformMatrix[0]<<","<<transformMatrix[1]<<","<<transformMatrix[2]<<","<<transformMatrix[3]<<"\n\t\t"<<transformMatrix[4]<<","<<transformMatrix[5]<<","<<transformMatrix[6]<<","<<transformMatrix[7]<<"\n\t\t"<<transformMatrix[8]<<","<<transformMatrix[9]<<","<<transformMatrix[10]<<transformMatrix[11]<<"\n\t\t"<<transformMatrix[12]<<","<<transformMatrix[13]<<","<<transformMatrix[14]<<","<<transformMatrix[15]
                       <<"\n";

            (*selectionsApplied)[i] = true;
        }
        else if (string(selectionsList[i]->GetType()) == "avtResolutionSelection")
        {
            const avtResolutionSelection* sel = static_cast<const avtResolutionSelection*>(*selectionsList[i]);
            VisusInfo()<<"\tnew resolution: "<<sel->resolution()<<", (old resolution: "<<resolution<<")\n";
            if (resolution!=sel->resolution())
            {
                ;//ClearCache();
            }
            resolution = sel->resolution();

            (*selectionsApplied)[i] = true;
        }
        else if (string(selectionsList[i]->GetType()) == "Resample Data Selection")
        {
            ;//ignore
        }
        else
        {
            VisusInfo()<<"avtIDXFileFormat: unhandled selection "<<selectionsList[i]->GetType()<<"!\n";
        }
    }

    //if no valid transformation matrix set extents and dims to full dataset
    if (transformMatrix[0] == DBL_MAX && transformMatrix[1] == DBL_MAX &&
        transformMatrix[2] == DBL_MAX && transformMatrix[3] == DBL_MAX)
    {
        NdPoint dims(bounds[0],bounds[1],bounds[2],1,1);
        double xDelta = (extents[1]-extents[0])/(float)dims.x;
        double yDelta = (extents[3]-extents[2])/(float)dims.y;
        double zDelta = (extents[5]-extents[4])/(float)dims.z;
        cellArea = sqrt(xDelta * xDelta + yDelta * yDelta + zDelta * zDelta);
        VisusInfo()<<"CalculateMesh (default, invalid transformMatrix): bounds:  "<<dims.toString();
        VisusInfo()<<"CalculateMesh (default, invalid transformMatrix): cellArea: "<<cellArea;
        NdBox   exts(NdPoint(extents[0],extents[2],extents[4]),NdPoint(extents[1],extents[3],extents[5]));
        VisusInfo()<<"CalculateMesh (default, invalid transformMatrix): extents: "<<exts.toString();
        selection->SetActualExtents(extents);
        selection->SetActualCellArea(cellArea);
        return;
    }

    //set frustum for view dependent read
    VisusInfo()<<"setting frustum for view-dependent read";
    Visus::Matrix mvp(transformMatrix);
    Visus::Viewport visus_viewport(0,0,windowSize[0],windowSize[1]);
    SharedPtr<Frustum> visus_frustum(new Frustum);
    visus_frustum->loadProjection(mvp);
    visus_frustum->setViewport(visus_viewport);

    //set time
    query->getInputPort("time")->writeValue(SharedPtr<IntObject>(new IntObject(timestate)));

    //
    //view-dependent resolution selection (if resolution is not set using MultiresControl)
    //
    query->getInputPort("enable_viewdep")->writeValue(SharedPtr<BoolObject>(new BoolObject(this->resolution<0)));
    query->getInputPort("resolution")->writeValue(SharedPtr<IntObject>(new IntObject(this->resolution)));
    query->getInputPort("viewdep")->writeValue(visus_frustum);
    Int64 max_size = 128*128*128; // 2gb but visit always casts to double --> 16gb!
    bool fits      = false;
    int count      = 0; //just in case query->processInput returns 0 (it shouldn't, but it still happens and I'm not sure why. bad viewport?)
    int quality    = 0;
    while (!fits && count<10)
    {
        count++;
        query->getInputPort("quality")->writeValue(SharedPtr<IntObject>(new IntObject(quality--)));
            
        //get the size of the target but don't fetch data (we don't know the varname yet)
        query->getInputPort("position_only")->writeValue(SharedPtr<BoolObject>(new BoolObject(true)));
        this->dataflow->oninput.emitSignal(query);

        NdPoint dims(bounds[0],bounds[1],bounds[2],1,1);
        fits = this->resolution>0 ? true : dims.innerProduct()<=max_size;
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

        {
            //VisusInfo()<<"now we should have the bounds and extents of the data!";
            NdBox   exts(NdPoint(extents[0],extents[2],extents[4]),NdPoint(extents[1],extents[3],extents[5]));
            VisusInfo()<<"CalculateMesh: extents: "<<exts.toString();
        }

        NdPoint dims(bounds[0],bounds[1],bounds[2],1,1);
        double xDelta = (extents[1]-extents[0]+1)/(float)dims.x;
        double yDelta = (extents[3]-extents[2]+1)/(float)dims.y;
        double zDelta = (extents[5]-extents[4]+1)/(float)dims.z;
        cellArea = sqrt(xDelta * xDelta + yDelta * yDelta + zDelta * zDelta);
        VisusInfo()<<"CalculateMesh: bounds:  "<<dims.toString();
        VisusInfo()<<"CalculateMesh: cellArea: "<<cellArea;
        selection->SetActualExtents(extents);
        selection->SetActualCellArea(cellArea);
        //selection->SetViewArea(dims.innerProduct());
        double viewArea=(extents[1]-extents[0]+1)*(extents[3]-extents[2]+1)*(extents[5]-extents[4]+1);
        //selection->SetViewArea(viewArea);
        VisusInfo()<<"CalculateMesh: viewArea: "<<viewArea;

        {
            double                  extents2[6];
            double area;
            avtView2D::CalculateExtentsAndArea(extents2, area, transformMatrix);
            NdBox   exts(NdPoint(extents2[0],extents2[2],extents2[4]),NdPoint(extents2[1],extents2[3],extents2[5]));
            VisusInfo()<<"\navtView2D::CalculateExtentsAndArea: area:  "<<area;
            VisusInfo()<<"avtView2D::CalculateExtentsAndArea: extents: "<<exts.toString();
            //selection->SetActualExtents(extents2);
        }
    }
}

