// ************************************************************************* //
//                      avtUnstructuredDomainBoundaries.C                    //
// ************************************************************************* //

#include <avtUnstructuredDomainBoundaries.h>

#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtGhostData.h>
#include <avtMaterial.h>
#include <avtMixedVariable.h>

#include <DebugStream.h>
#include <VisItException.h>

#include <set>
#include <map>

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

using namespace std;


#ifdef PARALLEL
namespace
{
    template <class T> int GetMPIDataType();
    template <>        int GetMPIDataType<int>()    { return MPI_INT;  }
    template <>        int GetMPIDataType<char>()   { return MPI_CHAR; }
    template <>        int GetMPIDataType<float>()  { return MPI_FLOAT;}

    template <>        int GetMPIDataType<unsigned int>()
                       { return MPI_UNSIGNED; }
    template <>        int GetMPIDataType<unsigned char>()
                       { return MPI_UNSIGNED_CHAR;}
}

#endif


// ****************************************************************************
//  Constructor:  avtUnstructuredDomainBoundaries::
//                  avtUnstructuredDomainBoundaries
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

avtUnstructuredDomainBoundaries::avtUnstructuredDomainBoundaries()
{

}

// ****************************************************************************
//  Destructor:  avtUnstructuredDomainBoundaries::
//                  ~avtUnstructuredDomainBoundaries
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

avtUnstructuredDomainBoundaries::~avtUnstructuredDomainBoundaries()
{

}


// ****************************************************************************
//  Destructor:  avtUnstructuredDomainBoundaries::Destruct
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

void
avtUnstructuredDomainBoundaries::Destruct(void *p)
{
    avtUnstructuredDomainBoundaries *u = (avtUnstructuredDomainBoundaries *)p;
    delete u;
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::SetSharedPoints
//
//  Purpose:
//    Specify the points which are shared between two domains. This is
//    says that the pointid d1pts[0] in domain d1 is the same point as
//    point referenced by pointid d2pts[0] in domain d2. This needs only
//    be called once (if at all) for each pair of domains.
//
//  Arguments:
//    d1        The domain number for the first domain.
//    d2        The domain number for the second domain.
//    d1pts     The ids of points for the first domain.
//    d2pts     The ids of points for the second domain (same size as d2pts).
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

void
avtUnstructuredDomainBoundaries::SetSharedPoints(int d1, int d2,
                                               const vector<int> &d1pts,
                                               const vector<int> &d2pts)
{
    int i;
    int index = GetGivenIndex(d1, d2);

    if (index == -1)
    {
        index = giveIndex.size();
        giveIndex.push_back(pair<int, int> (d1, d2));
        givenCells.resize(givenCells.size() + 1);
        givenPoints.resize(givenPoints.size() + 1);
        sharedPointsMap.resize(sharedPointsMap.size() + 1);
    }
    else
        sharedPointsMap[index].clear();

    for (i = 0; i < d1pts.size(); ++i)
    {
        sharedPointsMap[index][d1pts[i]] = d2pts[i];
    }

    // And put in the reverse.

    index = GetGivenIndex(d2, d1);
    if (index == -1)
    {
        index = giveIndex.size();
        giveIndex.push_back(pair<int, int> (d2, d1));
        givenCells.resize(givenCells.size() + 1);
        givenPoints.resize(givenPoints.size() + 1);
        sharedPointsMap.resize(sharedPointsMap.size() + 1);
    }
    else
        sharedPointsMap[index].clear();

    for (i = 0; i < d1pts.size(); ++i)
        sharedPointsMap[index][d2pts[i]] = d1pts[i];
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::SetGivenCellsAndPoints
//
//  Purpose:
//    Sets the cells and points that are given from one domain to another.
//
//  Notes:
//    Note that points that are shared are not considered to be 'given'.
//    If you are passing in points that are shared between the two
//    domains (identified by SetSharedPoints), set filterShared to true.
//    SetSharedPoints must have been called for the specified domains
//    before using filterShared.
//
//  Arguments:
//    fromDom       The domain that is giving the cells.
//    toDom         The domain that is receiving the cells.
//    cells         The cellIds of 'fromDom' that are being given.
//    points        The pointIds of 'fromDom' that are being given.
//    filterShared  If true, filters out the points that are already shared.
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

void
avtUnstructuredDomainBoundaries::SetGivenCellsAndPoints(int fromDom, int toDom,
                                               const vector<int> &cells,
                                               const vector<int> &points,
                                               bool filterShared)
{
    int index = GetGivenIndex(fromDom, toDom);

    if (index == -1)
    {
        giveIndex.push_back(pair<int, int>(fromDom, toDom));
        givenCells.push_back(cells);
        givenPoints.resize(givenPoints.size() + 1);

        sharedPointsMap.resize(sharedPointsMap.size() + 1);
    }
    else
        givenCells[index] = cells;

    if (!filterShared)
        givenPoints[index] = points;
    else
    {
        int sIndex = GetGivenIndex(fromDom, toDom);
        
        map<int, int> &smap = sharedPointsMap[sIndex];
        
        // Go through and manually insert the points that are not shared.

        int i;
        for (i = 0; i < points.size(); ++i)
        {
            if (smap.find(points[i]) == smap.end())
            {
                givenPoints[index].push_back(points[i]);
            }
        }
    }
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::GetGivenIndex
//
//  Purpose:
//    Get the index of the given data (in vectors givenCells & givenPoints).
//
//  Arguments:
//    from      The domain that is giving the cells/points.
//    to        The domain that is receiving the cells/points.
//
//  Returns:    An index into the data arrays, or -1 if none exist.
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

int
avtUnstructuredDomainBoundaries::GetGivenIndex(int from, int to)
{
    int i;
    for (i = 0; i < giveIndex.size(); ++i)
    {
        if (giveIndex[i].first == from && giveIndex[i].second == to)
            return i;
    }
    return -1;
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::CopyPointer
//
//  Purpose:
//    Copy pointer data.
//
//  Arguments:
//    src           Pointer to source.
//    dest          Pointer to destination.
//    components    How many components in the pointer (> 0).
//    count         The number of components to copy (> 0).
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

template <class T>
void
avtUnstructuredDomainBoundaries::CopyPointer(T *src, T *dest, int components,
                                                              int count)
{
    int i;
    int nIter = count * components;

    *dest = *src;
    for (i = 1; i < nIter; ++i)
        *(++dest) = *(++src);
}
 
// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::ExchangeMesh
//
//  Purpose:
//    Exchange the ghost zone information for some meshes,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    mesh         an array of meshes
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Aug 27 16:34:46 PDT 2004
//    Rename ghost data arrays.  Also properly mark ghost data type.
//
// ****************************************************************************

vector<vtkDataSet*>
avtUnstructuredDomainBoundaries::ExchangeMesh(vector<int>       domainNum,
                                            vector<vtkDataSet*> meshes)
{
    vector<vtkDataSet*> out(meshes.size(), NULL);

    // Gather the information we need
    vector<int> domain2proc = CreateDomainToProcessorMap(domainNum);

    float ***gainedPoints;
    int ***cellTypes;
    int ****cellPoints;
    int ***origPointIds;
    int **nGainedPoints;
    int **nGainedCells;
    int ***nPointsPerCell;
    CommunicateMeshInformation(domain2proc, domainNum, meshes, gainedPoints,
                               cellTypes, cellPoints, origPointIds,
                               nGainedPoints, nGainedCells, nPointsPerCell);

    int d;
    for (d = 0; d < domainNum.size(); d++)
    {
        int recvDom = domainNum[d];
        vtkUnstructuredGrid *mesh = (vtkUnstructuredGrid*)(meshes[d]);
        int nOldPoints = mesh->GetNumberOfPoints();

        // Find how many points are given to domain recvDom.
        int nGivenPoints = 0;
        int i;
        for (i = 0; i < nTotalDomains; ++i)
            nGivenPoints += nGainedPoints[i][recvDom];
        
        // Create the VTK objects
        vtkUnstructuredGrid    *outm  = vtkUnstructuredGrid::New(); 
        vtkPoints              *outp  = vtkPoints::New();

        outm->DeepCopy(meshes[d]);

        outm->SetPoints(outp);
        outp->Delete();
        outp->SetNumberOfPoints(nOldPoints + nGivenPoints);

        // Copy the old coordinates over
        float *oldcoord = (float *)mesh->GetPoints()->GetVoidPointer(0);
        float *newcoord = (float *)outp->GetVoidPointer(0);
        CopyPointer(oldcoord, newcoord, 3, nOldPoints);

        // Put in the new coordinates
        vector<map<int, int> > translatedPointsMap(nTotalDomains);
        newcoord += 3 * nOldPoints;
        int newId = nOldPoints;
        int sendDom;
        for (sendDom = 0; sendDom < nTotalDomains; ++sendDom)
        {
            if (sendDom == recvDom)
                continue;
            
            int nGivenPointsThisDomain = nGainedPoints[sendDom][recvDom];
            if (nGivenPointsThisDomain == 0)
                continue;
            
            // We need to remember what the point id for this exchange
            // of points is.
            startingPoint[pair<int,int>(sendDom, recvDom)] = newId;
            
            float *pts = gainedPoints[sendDom][recvDom];
            int *origPointIdsThisDomain = origPointIds[sendDom][recvDom];

            for (i = 0; i < nGivenPointsThisDomain; ++i)
            {
                *(newcoord++) = *(pts++);
                *(newcoord++) = *(pts++);
                *(newcoord++) = *(pts++);
                translatedPointsMap[sendDom][origPointIdsThisDomain[i]] = 
                                                                        newId++;
            }
        }
        
        int nOldCells = outm->GetNumberOfCells();

        
        vtkIdList *idList = vtkIdList::New();
        // Put in the new cells
        for (sendDom = 0; sendDom < nTotalDomains; ++sendDom)
        {
            if (recvDom == sendDom)
                continue;

            int nGainedCellsThisDomain = nGainedCells[sendDom][recvDom];

            if (nGainedCellsThisDomain == 0)
                continue;
            
            // We're going to be be giving cells from sendDom to recvDom.
            // The id that the cells will be inserted at is
            // important, and we need to remember.
            startingCell[pair<int,int>(sendDom, recvDom)] = outm->
                                                            GetNumberOfCells();
            
            // We want the map that indexes the ptIds from sendDom into
            // the ptIds of recvDom.
            int index = GetGivenIndex(sendDom, recvDom);
            map<int, int> &smap = sharedPointsMap[index];
            map<int, int> &tmap = translatedPointsMap[sendDom];
            
            for (i = 0; i < nGainedCellsThisDomain; ++i)
            {
                int nPointsThisCell = nPointsPerCell[sendDom][recvDom][i];
                idList->SetNumberOfIds(nPointsThisCell);
                int k;
                for (k = 0; k < nPointsThisCell; ++k)
                {
                    int id = cellPoints[sendDom][recvDom][i][k];
                    if (smap.find(id) != smap.end())
                        idList->SetId(k, smap[id]);
                    else
                       idList->SetId(k, tmap[id]); 
                }
                outm->InsertNextCell(cellTypes[sendDom][recvDom][i], idList);
            }
        }
        idList->Delete();

        // Create the ghost zone array

        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
        ghostCells->SetName("avtGhostZones");
        ghostCells->SetNumberOfTuples(outm->GetNumberOfCells());
        unsigned char *ptr = ghostCells->GetPointer(0);
        for (i = 0; i < nOldCells; ++i)
            *(ptr++) = 0;
        int nGhostCells = outm->GetNumberOfCells() - nOldCells;
        for (i = 0; i < nGhostCells; ++i)
        {
            avtGhostData::AddGhostZoneType(*ptr,
                                          DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
            ptr++;
        }
        outm->GetCellData()->AddArray(ghostCells);
        ghostCells->Delete();
        outm->SetUpdateGhostLevel(0);

        outm->BuildLinks();
        out[d] = outm;
    }

    // Now to destroy all of that memory we used.
    // In the simple case (gathering information without parallel
    // communication), new appears invoked 20 times CommunicateMeshInformation.
    // This means that delete should appear 20 times.
    int a,b,c;

    for (a = 0; a < nTotalDomains; ++a)
    {
        for (b = 0; b < nTotalDomains; ++b)
        {
            // These aren't always allocated for each combination.
            // Check to see if they were allocated before deleting.
            if (gainedPoints[a][b])
            {
                for (c = 0; c < nGainedCells[a][b]; ++c)
                {
                    delete [] cellPoints[a][b][c];
                }

                delete [] gainedPoints[a][b];
                delete [] cellTypes[a][b];
                delete [] origPointIds[a][b];
                delete [] cellPoints[a][b];
                delete [] nPointsPerCell[a][b];
            }
        }

        delete [] gainedPoints[a];
        delete [] cellTypes[a];
        delete [] origPointIds[a];
        delete [] cellPoints[a];
        delete [] nGainedPoints[a];
        delete [] nGainedCells[a];
        delete [] nPointsPerCell[a];
    }
    
    delete [] gainedPoints;
    delete [] cellTypes;
    delete [] cellPoints;
    delete [] origPointIds;
    delete [] nGainedPoints;
    delete [] nGainedCells;
    delete [] nPointsPerCell;

    return out;
}

// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::ExchangeScalar
//
//  Purpose:
//    Exchange the ghost zone information for some scalars,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    isPointData  true if this is node-centered, false if cell-centered
//    scalars      an array of scalars
//
//  Programmer:  Akira Haddoxs
//  Creation:    August 11, 2003
//
// ****************************************************************************

vector<vtkDataArray*>
avtUnstructuredDomainBoundaries::ExchangeScalar(vector<int>         domainNum,
                                              bool                isPointData,
                                              vector<vtkDataArray*> scalars)
{
    // We're in a bit of a sticky situation if we don't have any actual data.
    // Without a valid vtkDataArray, we don't know which ExchangeData to
    // call. But if there's no data, nothing will actually be exchanged
    // aside from basic communications (eg: domain2proc, MPI_Barrier),
    // so we'll choose to call one. 
    if (!scalars.size())
    {
        return ExchangeData<float> (domainNum, isPointData, scalars);
    }
    // This one's a little more complicated because there are different
    // types of scalars we might encounter. If more cases arise,
    // expand this function.
    switch (scalars[0]->GetDataType())
    {
        case VTK_INT:
            return ExchangeData<int> (domainNum, isPointData, scalars);
        case VTK_CHAR:
            return ExchangeData<char> (domainNum, isPointData, scalars);
        case VTK_FLOAT:
            return ExchangeData<float> (domainNum, isPointData, scalars);
        case VTK_UNSIGNED_CHAR:
            return ExchangeData<unsigned char>(domainNum, isPointData, scalars);
        case VTK_UNSIGNED_INT:
            return ExchangeData<unsigned int> (domainNum, isPointData, scalars);
        default:
            string exc_mesg = "avtUnstructuredDomainBoundaries does not know "
                              "how to exchange scalars from array type "
                              + string(scalars[0]->GetClassName());
            EXCEPTION1(VisItException, exc_mesg);
    }
    // (To avoid compiler warnings). This code is never reached.
    return scalars;
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::ExchangeFloatVector
//
//  Purpose:
//    Exchange the ghost zone information for some vectors,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    isPointData  true if this is node-centered, false if cell-centered
//    vectors      an array of vectors
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

vector<vtkDataArray*>
avtUnstructuredDomainBoundaries::ExchangeFloatVector(vector<int>      domainNum,
                                              bool                  isPointData,
                                              vector<vtkDataArray*> vectors)
{
    return ExchangeData<float> (domainNum, isPointData, vectors);
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::ExchangeIntVector
//
//  Purpose:
//    Exchange the ghost zone information for some vectors,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    isPointData  true if this is node-centered, false if cell-centered
//    vectors      an array of vectors
//
//  Notes:
//    Taken from ExchangeFloatVector and modified for integer data types.
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

vector<vtkDataArray*>
avtUnstructuredDomainBoundaries::ExchangeIntVector(vector<int>        domainNum,
                                                 bool               isPointData,
                                                 vector<vtkDataArray*> vectors)
{
    return ExchangeData<int> (domainNum, isPointData, vectors);
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::ExchangeMaterial
//
//  Purpose:
//    Exchange the ghost zone information for some materials,
//    returning the new ones. NOT IMPLEMENTED FOR MIXED MATERIALS.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    material     an array of materials
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

vector<avtMaterial*>
avtUnstructuredDomainBoundaries::ExchangeMaterial(vector<int>    domainNum,
                                                vector<avtMaterial*> mats)
{
    //
    // As of its creation, this class has only been used
    // by the Mili fileformat reader, which doesn't use
    // mixed materials. This has yet to be implemented due
    // to lack of data, and lack of time.
    //
    
    //
    // Since the implementation for mixed materials that will someday
    // be made will replace most of this code, we do a small sacrifice
    // of speed to reuse existing components (fortunately, speed shouldn't
    // be an issue since the materials we're working with aren't mixed).
    //    

    //
    // Load the materials into data arrays.
    //
    vector<vtkDataArray *> materialArrays(domainNum.size());
    int i;
    for (i = 0; i < domainNum.size(); ++i)
    {
        if (mats[i]->GetMixlen())
        {
            EXCEPTION1(VisItException, "ExchangeMaterials for mixed materials"
                                       " has not yet been implemented in "
                                       " avtUnstructuredDomainBoundaries. ");
        }
        materialArrays[i] = vtkIntArray::New();
        int nZones = mats[i]->GetNZones();
        materialArrays[i]->SetNumberOfTuples(nZones);

        int *ptr = (int*)(materialArrays[i]->GetVoidPointer(0));
        const int *matPtr = mats[i]->GetMatlist();

        int j;
        for (j = 0; j < nZones; ++j)
            *(ptr++) = *(matPtr++);
    }

    vector<vtkDataArray *> result;
    result = ExchangeData<int> (domainNum, false, materialArrays);

    vector<avtMaterial*> out(mats.size(), NULL);

    for (i = 0; i < domainNum.size(); ++i)
    {
        int nMaterials = mats[i]->GetNMaterials();
        int nZones = result[i]->GetNumberOfTuples();
        int *matPtr = (int *)(result[i]->GetVoidPointer(0));
        
        out[i] = new avtMaterial(nMaterials, mats[i]->GetMaterials(),
                                 nZones, matPtr, 0,
                                 NULL, NULL, NULL, NULL);

        materialArrays[i]->Delete();
        result[i]->Delete();
    }
    
    return out;
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::ExchangeMixVar
//
//  Purpose:
//    Exchange the ghost zone information for some mixvars,
//    returning the new ones. NOT IMPLEMENTED.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    mixvar       an array of mixvars
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

vector<avtMixedVariable*>
avtUnstructuredDomainBoundaries::ExchangeMixVar(vector<int>         domainNum,
                                          const vector<avtMaterial*> mats,
                                          vector<avtMixedVariable*>  mixvars)
{
    //
    // As of its creation, this class has only been used
    // by the Mili fileformat reader, which doesn't use
    // mixed variables. This has yet to be implemented due
    // to lack of data, and lack of time.
    //
    EXCEPTION1(VisItException, "ExchangeMixVar is not yet implemented for "
                               "avtUnstructuredDomainBoundaries.");
    return mixvars;
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::ConfirmMesh
//
//  Purpose:
//      If there is more than one mesh, the boundary information is likely for
//      only one of them.  Confirm that the mesh has the proper dimensions.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    mesh         an array of meshes
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

bool
avtUnstructuredDomainBoundaries::ConfirmMesh(vector<int>       domainNum,
                                            vector<vtkDataSet*> meshes)
{
    // Our best bet is to compare some shared points.
    int i;
    for (i = 0; i < domainNum.size(); ++i)
    {
        int d1 = domainNum[i];
        int j;
        for (j = i + 1; j < domainNum.size(); ++j)
        {
            int d2 = domainNum[j];
            int index = GetGivenIndex(d1, d2);
            if (index < 0)
                continue;

            map<int, int> &smap = sharedPointsMap[index];
            if (smap.size() == 0)
                continue;

            // Found a shared domain, let's take a look.
            int d1ptId = smap.begin()->first;
            int d2ptId = smap.begin()->second;

            float *pt1 = meshes[i]->GetPoint(d1ptId);
            float *pt2 = meshes[j]->GetPoint(d2ptId);

            const double epsilon = 1e-12;
            
            // If these points are too dis-similar, it has to be
            // referring another mesh.
            if (fabs(pt1[0] - pt2[0]) + fabs(pt1[1] - pt2[1])
                                      + fabs(pt1[2] - pt2[2]) > epsilon)
                return false;
            
            // If we reached this point, then we've tested this pair of
            // domains.
            break;
        }
    }
    
    return true;
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::ExchangeData
//
//  Purpose:
//    Exchange the ghost zone information for some data (scalars and
//    vectors of templated type), returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    isPointData  true if this is node-centered, false if cell-centered
//    data         data for those domains
//
//  Returns:
//    vtkDataArrays of the same type as input for each domain in domainNum.
//
//  Programmer:  Akira Haddoxs
//  Creation:    August 15, 2003
//
// ****************************************************************************

template <class T>
vector<vtkDataArray*>
avtUnstructuredDomainBoundaries::ExchangeData(vector<int>         &domainNum,
                                              bool                isPointData,
                                              vector<vtkDataArray*> &data)
{
    // Gather the needed information
    vector<int> domain2proc = CreateDomainToProcessorMap(domainNum);
    T ***gainedData;
    int **nGainedTuples;
    CommunicateDataInformation<T> (domain2proc, domainNum, data, isPointData,
                                                gainedData, nGainedTuples);

    vector<vtkDataArray*> out(data.size(), NULL);
    int nComponents = 0;
    if (data.size())
        nComponents = data[0]->GetNumberOfComponents();


    int i;
    for (i = 0; i < domainNum.size(); ++i)
    {
        int recvDom = domainNum[i];
        out[i] = data[i]->NewInstance();

        out[i]->DeepCopy(data[i]);
        out[i]->SetName(data[i]->GetName());
        
        int sendDom;

        int nGivenTuples = 0;
        for (sendDom = 0; sendDom < nTotalDomains; ++sendDom)
        {
            if (recvDom == sendDom)
                continue;

            nGivenTuples += nGainedTuples[sendDom][recvDom];
        }
        
        out[i]->Resize(nGivenTuples + out[i]->GetNumberOfTuples());

        // This properly sets the internal size.
        out[i]->InsertTuple(data[i]->GetNumberOfTuples() + nGivenTuples - 1,
                            data[i]->GetTuple(0));
        
        for (sendDom = 0; sendDom < nTotalDomains; ++sendDom)
        {
            if (sendDom == recvDom || nGainedTuples[sendDom][recvDom] == 0)
                continue;

            int refIndex;
            if (isPointData)
                refIndex = startingPoint[pair<int,int>(sendDom, recvDom)];
            else
                refIndex = startingCell[pair<int,int>(sendDom, recvDom)];

            T * ptr = (T *)(out[i]->GetVoidPointer(refIndex * nComponents));
            T * copyPtr = gainedData[sendDom][recvDom];

            int nDatCopy = nGainedTuples[sendDom][recvDom] * nComponents;
            int k;
            for (k = 0; k < nDatCopy; ++k)
            {
                (*ptr++) = (*copyPtr++) ;
            }
        }
    }

    int a, b;
    for (a = 0; a < nTotalDomains; ++a)
    {
        for (b = 0; b < nTotalDomains; ++b)
        {
            // These aren't always allocated for each combination.
            // Check to see if they were allocated before deleting.
            if (gainedData[a][b])
            {
                delete [] gainedData[a][b];
            }
        }
        delete [] gainedData[a];
        delete [] nGainedTuples[a];
    }
    delete [] gainedData;
    delete [] nGainedTuples;

    return out;
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::CreateDomainToProcessorMap
//
//  Purpose:
//    Create an array of (ndomains) integers with the rank of the processor
//    (0..nproc-1) which owns each domain, or -1 if no processor owns it.
//
//  Notes:
//    Adapted from avtStructuredDomainBoundaries.
//
//  Arguments:
//    domainNum    an array of domain numbers owned by the current processor
//
//  Programmer:  Akira Haddox
//  Creation:    August 14, 2003
//
// ****************************************************************************

vector<int>
avtUnstructuredDomainBoundaries::CreateDomainToProcessorMap(
                                                const vector<int> &domainNum)
{
    // Get the processor rank
    int rank = 0;
#ifdef PARALLEL
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

    // Create the map
    vector<int> domain2proc(nTotalDomains, -1);
    for (int d = 0; d < domainNum.size(); d++)
        domain2proc[domainNum[d]] = rank;

#ifdef PARALLEL
    vector<int> domain2proc_tmp(domain2proc);
    MPI_Allreduce(&domain2proc_tmp[0], &domain2proc[0], nTotalDomains, MPI_INT,
                  MPI_MAX, MPI_COMM_WORLD);
#endif

    return domain2proc;
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::CommunicateMeshInformation
//
//  Purpose:
//    Send and collect information needed to exchange meshes. Also
//    generates information from domains handled by this process.
//
//  Notes:
//    Returned arguments should be passed in as uninitialized pointers.
//    Data will be allocated as necesary, but will need to be freed by
//    the caller. Allocation always occurs two the second level (meaning
//    for example, gainedPoints[0][0] will always exist, but its value
//    may be NULL).
//
//  Arguments:
//    domain2proc   the map of domains to processors
//    domainNum     the domains that are owned by the current process
//    meshes        the meshes owned by the current processor
//  (Returned arguments:)
//    gainedPoints  an array for each domain of points that are given to that
//                  domain (indexed: [sendDom][recvDom][ptId * 3])
//    cellTypes     an array for each domain of the cell types that are given
//                  to that domain (indexed: [sendDom][recvDom][cell])
//    cellPoints    an array for each domain of the points corresponding to
//                  the given cells (indexed: [sendDom][recvDom][cell][pts]).
//    origPointIds  an array for each domain of the original point id from
//                  the sending domain for the given points
//                  (indexed: [sendDom][recvDom][pt]).
//    nGainedPoints the numer of gained points (indexed: [sendDom][recvDom])
//    nGainedCells  the number of gained cells (indexed: [sendDom][recvDom])
//    nPointsPerCell  the number of points that construct the given cell
//                    (indexed: [sendDom][recvDom][cell]).
//
//  Programmer:  Akira Haddox
//  Creation:    August 14, 2003
//
//  Modifications:
//
//    Mark C. Miller, Wed Jun  9 21:50:12 PDT 2004
//    Eliminated use of MPI_ANY_TAG and modified to use GetUniqueMessageTags
//
// ****************************************************************************

void
avtUnstructuredDomainBoundaries::CommunicateMeshInformation(
                                 const vector<int> &domain2proc,
                                 const vector<int> &domainNum,
                                 const vector<vtkDataSet *> &meshes,
                                 float ***&gainedPoints, int ***&cellTypes,
                                 int ****&cellPoints, int ***&origPointIds,
                                 int **&nGainedPoints, int **&nGainedCells,
                                 int ***&nPointsPerCell)
{
    // Get the processor rank
    int rank = 0;
#ifdef PARALLEL
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int mpiNPtsTag = GetUniqueMessageTag();
    int mpiGainedPointsTag = GetUniqueMessageTag();
    int mpiOriginalIdsTag = GetUniqueMessageTag();
    int mpiNumGivenCellsTag = GetUniqueMessageTag();
    int mpiCellTypesTag = GetUniqueMessageTag();
    int mpiNumPointsPerCellTag = GetUniqueMessageTag();
    int mpiCellPointIdsTag = GetUniqueMessageTag();
#endif

    gainedPoints = new float**[nTotalDomains];
    cellTypes = new int**[nTotalDomains];
    cellPoints = new int***[nTotalDomains];
    origPointIds = new int**[nTotalDomains];
    nGainedPoints = new int*[nTotalDomains];
    nGainedCells = new int*[nTotalDomains];
    nPointsPerCell = new int **[nTotalDomains];

    vtkIdList *idList = vtkIdList::New();
    
    int i;
    int sendDom;
    for (sendDom = 0; sendDom < nTotalDomains; ++sendDom)
    {
        gainedPoints[sendDom] = new float*[nTotalDomains];
        cellTypes[sendDom] = new int*[nTotalDomains];
        cellPoints[sendDom] = new int**[nTotalDomains];
        origPointIds[sendDom] = new int*[nTotalDomains];
        nGainedPoints[sendDom] = new int[nTotalDomains];
        nGainedCells[sendDom] = new int[nTotalDomains];
        nPointsPerCell[sendDom] = new int *[nTotalDomains];

        int recvDom;
        for (recvDom = 0; recvDom < nTotalDomains; ++recvDom)
        {
            gainedPoints[sendDom][recvDom] = NULL;
            cellTypes[sendDom][recvDom] = NULL;
            cellPoints[sendDom][recvDom] = NULL;
            origPointIds[sendDom][recvDom] = NULL;

            nPointsPerCell[sendDom][recvDom] = NULL;

            nGainedPoints[sendDom][recvDom] = 0;
            nGainedCells[sendDom][recvDom] = 0;
            
            // Cases where no computation is required.
            if (sendDom == recvDom)
                continue;
            if (domain2proc[sendDom] == -1 || domain2proc[recvDom] == -1)
                continue;

            // If this process owns both of the domains, it's an internal
            // calculation: no communication needed
            if (domain2proc[sendDom] == rank && domain2proc[recvDom] == rank)
            {
                for (i = 0; i < domainNum.size(); ++i)
                    if (domainNum[i] == sendDom)
                        break;
                
                vtkUnstructuredGrid *givingUg = (vtkUnstructuredGrid*)meshes[i];
                
                int index = GetGivenIndex(sendDom, recvDom);

                // If no domain boundary, then there's no work to do.
                if (index < 0)
                    continue;

                int nPts = givenPoints[index].size();
                nGainedPoints[sendDom][recvDom] += nPts;
                
                gainedPoints[sendDom][recvDom] = new float[nPts * 3];
                origPointIds[sendDom][recvDom] = new int[nPts];

                float *gainedPtr = gainedPoints[sendDom][recvDom];
                int *origIdPtr = origPointIds[sendDom][recvDom];
                float *fromPtr = (float *)(givingUg ->GetPoints()
                                                    ->GetVoidPointer(0));
                for (i = 0; i < nPts; ++i)
                {
                    *(origIdPtr++) = givenPoints[index][i];
                    
                    float *ptPtr = fromPtr + 3 * givenPoints[index][i];
                    *(gainedPtr++) = *(ptPtr++);
                    *(gainedPtr++) = *(ptPtr++);
                    *(gainedPtr++) = *(ptPtr++);
                }

                int nCells = givenCells[index].size();
                nGainedCells[sendDom][recvDom] += nCells;

                cellTypes[sendDom][recvDom] = new int[nCells];
                cellPoints[sendDom][recvDom] = new int *[nCells];
                nPointsPerCell[sendDom][recvDom] = new int[nCells];

                int *cellPtr = cellTypes[sendDom][recvDom];
                int **cellPtsPtr = cellPoints[sendDom][recvDom]; 
                int *nPtsPerCellPtr = nPointsPerCell[sendDom][recvDom]; 

                for (i = 0; i < nCells; ++i)
                {
                    cellPtr[i] = givingUg->GetCellType(givenCells[index][i]);
                    givingUg->GetCellPoints(givenCells[index][i], idList);

                    nPts = idList->GetNumberOfIds();
                    nPtsPerCellPtr[i] = nPts;

                    cellPtsPtr[i] = new int[nPts];
                    int k;
                    for (k = 0; k < nPts; ++k)
                        cellPtsPtr[i][k] = idList->GetId(k);
                }
            }
            // All other cases only occur during parallel execution.
#ifdef PARALLEL
            // If this process owns the receiving domain, we recv information.
            else if (domain2proc[recvDom] == rank)
            {
                MPI_Status stat;

                int fRank = domain2proc[sendDom];
                // Get the number of points given
                int nPts;
                MPI_Recv(&nPts, 1, MPI_INT, fRank, mpiNPtsTag,
                         MPI_COMM_WORLD, &stat);

                if (nPts == 0)
                    continue;
                
                nGainedPoints[sendDom][recvDom] += nPts;
                gainedPoints[sendDom][recvDom] = new float[nPts * 3];
                origPointIds[sendDom][recvDom] = new int[nPts];

                // Get the gained points
                MPI_Recv(gainedPoints[sendDom][recvDom], nPts * 3, MPI_FLOAT,
                         fRank, mpiGainedPointsTag, MPI_COMM_WORLD, &stat);

                // Get the original ids for the gained points
                MPI_Recv(origPointIds[sendDom][recvDom], nPts, MPI_INT,
                         fRank, mpiOriginalIdsTag, MPI_COMM_WORLD, &stat);

                // Get the number of given cells
                int nCells;
                MPI_Recv(&nCells, 1, MPI_INT, fRank,
                         mpiNumGivenCellsTag, MPI_COMM_WORLD, &stat);
                
                nGainedCells[sendDom][recvDom] += nCells;
                
                cellTypes[sendDom][recvDom] = new int[nCells];
                cellPoints[sendDom][recvDom] = new int *[nCells];
                nPointsPerCell[sendDom][recvDom] = new int[nCells];

                // Get the cell types
                MPI_Recv(cellTypes[sendDom][recvDom], nCells, MPI_INT,
                         fRank, mpiCellTypesTag, MPI_COMM_WORLD, &stat);

                // Get the number of points per cell
                MPI_Recv(nPointsPerCell[sendDom][recvDom], nCells, MPI_INT,
                         fRank, mpiNumPointsPerCellTag, MPI_COMM_WORLD, &stat);

                // Prepare for getting the cell point ids
                int k;
                int pntArrSize = 0;
                for (k = 0; k < nCells; ++k)
                {
                    cellPoints[sendDom][recvDom][k] = 
                                new int [nPointsPerCell[sendDom][recvDom][k]];
                    pntArrSize += nPointsPerCell[sendDom][recvDom][k];
                }

                // Get the cell point ids
                int * pntIds = new int[pntArrSize];
                MPI_Recv(pntIds, pntArrSize, MPI_INT,
                         fRank, mpiCellPointIdsTag, MPI_COMM_WORLD, &stat);

                // Move over the point ids
                int *ptr = pntIds;
                for (k = 0; k < nCells; ++k)
                {
                    int p;
                    for (p = 0; p < nPointsPerCell[sendDom][recvDom][k]; ++p)
                        cellPoints[sendDom][recvDom][k][p] = *(ptr++);
                }
                delete [] pntIds;
            }
            // If this process owns the sending domain, we send information.
            else if (domain2proc[sendDom] == rank)
            {
                int tRank = domain2proc[recvDom];

                int index = GetGivenIndex(sendDom, recvDom); 
                
                // If no domain boundary, send 0 for nPts, and continue
                // Also continue if there are no given points.
                if (index < 0 || givenPoints[index].size() == 0)
                {
                    int nPts = 0;
                    MPI_Send(&nPts, 1, MPI_INT, tRank, mpiNPtsTag, MPI_COMM_WORLD);
                    continue;
                }

                for (i = 0; i < domainNum.size(); ++i)
                    if (domainNum[i] == sendDom)
                        break;

                vtkUnstructuredGrid *givingUg = (vtkUnstructuredGrid*)meshes[i];

                int nPts = givenPoints[index].size();

                // Build the point data to send
                float *gainedPtrStart = new float[nPts * 3];
                int *origIdPtrStart = new int[nPts];
                float *fromPtr = (float *)(givingUg ->GetPoints()
                                                    ->GetVoidPointer(0));

                float *gainedPtr = gainedPtrStart;
                int *origIdPtr = origIdPtrStart;

                int k;
                for (k = 0; k < nPts; ++k)
                {
                    *(origIdPtr++) = givenPoints[index][k];
                    
                    float *ptPtr = fromPtr + 3 * givenPoints[index][k];
                    *(gainedPtr++) = *(ptPtr++);
                    *(gainedPtr++) = *(ptPtr++);
                    *(gainedPtr++) = *(ptPtr++);
                }

                // Send the number of points given
                MPI_Send(&nPts, 1, MPI_INT, tRank, mpiNPtsTag, MPI_COMM_WORLD); 
                
                // Send the gained points
                MPI_Send(gainedPtrStart, nPts * 3, MPI_FLOAT, tRank, mpiGainedPointsTag,
                         MPI_COMM_WORLD);
                
                // Send the original ids for the gained points
                MPI_Send(origIdPtrStart, nPts, MPI_INT, tRank, mpiOriginalIdsTag, 
                         MPI_COMM_WORLD);

                // Send the number of given cells
                int nCells = givenCells[index].size();
                MPI_Send(&nCells, 1, MPI_INT, tRank, mpiNumGivenCellsTag, MPI_COMM_WORLD);
                
                // Prepare for sending the cell info
                int *cellPtr = new int[nCells];
                vector<int> cellPtsVector;
                int *nPtsPerCellPtr = new int[nCells];

                int j;
                for (j = 0; j < nCells; ++j)
                {
                    cellPtr[j] = givingUg->GetCellType(givenCells[index][j]);
                    givingUg->GetCellPoints(givenCells[index][j], idList);

                    nPts = idList->GetNumberOfIds();
                    nPtsPerCellPtr[j] = nPts;

                    int k;
                    for (k = 0; k < nPts; ++k)
                        cellPtsVector.push_back(idList->GetId(k));
                }
                
                
                // Send the cell types
                MPI_Send(cellPtr, nCells, MPI_INT, tRank, mpiCellTypesTag,
                    MPI_COMM_WORLD);

                // Send the number of points per cell
                MPI_Send(nPtsPerCellPtr, nCells, MPI_INT, tRank,
                    mpiNumPointsPerCellTag, MPI_COMM_WORLD);

                // Send the point cells
                MPI_Send(&(cellPtsVector[0]), cellPtsVector.size(), MPI_INT,
                         tRank, mpiCellPointIdsTag, MPI_COMM_WORLD);


                delete [] gainedPtrStart;
                delete [] origIdPtrStart;
                delete [] cellPtr;
                delete [] nPtsPerCellPtr;
            }
#endif
        }
    }

    idList->Delete();

#ifdef PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
#endif
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::CommunicateDataInformation
//
//  Purpose:
//    Send and collect information needed to exchange variable data. Also
//    generates information from domains handled by this process.
//
//  Notes:
//    Returned arguments should be passed in as uninitialized pointers.
//    Data will be allocated as necesary, but will need to be freed by
//    the caller. Allocation always occurs two the second level (meaning
//    for example, gainedPoints[0][0] will always exist, but its value
//    may be NULL).
//
//  Arguments:
//    domain2proc   the map of domains to processors
//    domainNum     the domains that are owned by the current process
//    data          the data for the domains owned by the current process
//  (Returned arguments:)
//    gainedData    an array for each domain of data that are given to that
//                  domain (indexed: [sendDom][recvDom])
//    nGainedTuples the number of gained tuples of data
//                  (indexed: [sendDom][recvDom])
//
//  Programmer:  Akira Haddox
//  Creation:    August 14, 2003
//
// ****************************************************************************

template <class T>
void
avtUnstructuredDomainBoundaries::CommunicateDataInformation(
                                 const vector<int> &domain2proc,
                                 const vector<int> &domainNum,
                                 const vector<vtkDataArray *> &data,
                                 bool isPointData,
                                 T ***&gainedData, int **&nGainedTuples)
{
    // Get the processor rank
    int rank = 0;
#ifdef PARALLEL
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int mpiNumTuplesTag = GetUniqueMessageTag();
    int mpiTupleDataTag = GetUniqueMessageTag();
#endif

    int nComponents = 0;
    if (data.size())
        nComponents = data[0]->GetNumberOfComponents();

    gainedData = new T**[nTotalDomains];
    nGainedTuples = new int*[nTotalDomains];

    int sendDom;
    for (sendDom = 0; sendDom < nTotalDomains; ++sendDom)
    {
        gainedData[sendDom] = new T*[nTotalDomains];
        nGainedTuples[sendDom] = new int[nTotalDomains];

        int recvDom;
        for (recvDom = 0; recvDom < nTotalDomains; ++recvDom)
        {
            gainedData[sendDom][recvDom] = NULL;
            nGainedTuples[sendDom][recvDom] = 0;

            // Cases where no computation is required.
            if (sendDom == recvDom)
                continue;
            if (domain2proc[sendDom] == -1 || domain2proc[recvDom] == -1)
                continue;

            // If this process owns both of the domains, it's an internal
            // calculation: no communication needed
            if (domain2proc[sendDom] == rank && domain2proc[recvDom] == rank)
            {
                int i;
                for (i = 0; i < domainNum.size(); ++i)
                    if (domainNum[i] == sendDom)
                        break;

                int index = GetGivenIndex(sendDom, recvDom);

                // If no domain boundary, then there's no work to do.
                if (index < 0)
                    continue;

                vector<int> &mapRef = isPointData ? givenPoints[index] 
                                                  : givenCells[index];
                
                int nTuples = mapRef.size();
                nGainedTuples[sendDom][recvDom] = nTuples;

                gainedData[sendDom][recvDom] = new T[nTuples * nComponents];
                
                T * origPtr = (T*)(data[i]->GetVoidPointer(0));
                T * dataPtr = gainedData[sendDom][recvDom];

                for (i = 0; i < nTuples; ++i)
                {
                    T *ptr = origPtr + mapRef[i] * nComponents;
                    int j;
                    for (j = 0; j < nComponents; ++j)
                    {
                        *(dataPtr++) = *(ptr++);
                    }
                }
            }
            // All other cases only occur during parallel execution.
#ifdef PARALLEL
            // If this process owns the receving domain, we recv information.
            else if (domain2proc[recvDom] == rank)
            {
                int type = GetMPIDataType<T>();
                MPI_Status stat;

                int fRank = domain2proc[sendDom];
                // Get the number of incoming tuples
                int nTup;
                MPI_Recv(&nTup, 1, MPI_INT, fRank, mpiNumTuplesTag,
                         MPI_COMM_WORLD, &stat);
                
                if (nTup == 0)
                    continue;

                nGainedTuples[sendDom][recvDom] = nTup;
                gainedData[sendDom][recvDom] = new T[nTup * nComponents];

                // Get the data
                MPI_Recv(gainedData[sendDom][recvDom], nTup * nComponents,
                         type, fRank, mpiTupleDataTag, MPI_COMM_WORLD, &stat);
            }
            // If this process owns the sending domain, we send information.
            else if (domain2proc[sendDom] == rank)
            {
                int type = GetMPIDataType<T>();
                int tRank = domain2proc[recvDom];

                int index = GetGivenIndex(sendDom, recvDom);

                // If no domain boundary, send 0 for nTuples and continue
                if (index < 0)
                {
                    int nTup = 0;
                    MPI_Send(&nTup, 1, MPI_INT, tRank, mpiNumTuplesTag,
                        MPI_COMM_WORLD);
                    continue;
                }

                // Send the number of tuples
                vector<int> &mapRef = isPointData ? givenPoints[index]
                                                  : givenCells[index];
                int nTup = mapRef.size();
                MPI_Send(&nTup, 1, MPI_INT, tRank, mpiNumTuplesTag,
                    MPI_COMM_WORLD);

                // If there are no tuples to give, continue
                if (nTup == 0)
                    continue;

                // Gather the data for sending
                int dIndex;
                for (dIndex = 0; dIndex < domainNum.size(); ++dIndex)
                    if (domainNum[dIndex] == sendDom)
                        break;

                T * dataArr = new T[nTup * nComponents];
                T * origPtr = (T*)(data[dIndex]->GetVoidPointer(0));
                T * dataPtr = dataArr;

                int i;
                for (i = 0; i < nTup; ++i)
                {
                    T *ptr = origPtr + mapRef[i] * nComponents;
                    int j;
                    for (j = 0; j < nComponents; ++j)
                    {
                        *(dataPtr++) = *(ptr++);
                    }
                }

                // Send the data
                MPI_Send(dataArr, nTup * nComponents, type, tRank,
                    mpiTupleDataTag, MPI_COMM_WORLD);

                delete [] dataArr;
            }
#endif 
        }
    }

#ifdef PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
#endif
}


// ****************************************************************************
//  Method: avtUnstructuredDomainBoundaries::CreateGhostNodes
//
//  Purpose:
//      Creates ghost nodes.
//
//  Programmer: Hank Childs
//  Creation:   August 16, 2004
//
//    Hank Childs, Fri Aug 27 16:34:46 PDT 2004
//    Rename ghost data arrays.
//
// ****************************************************************************

void
avtUnstructuredDomainBoundaries::CreateGhostNodes(vector<int> domainNum,
                                                  vector<vtkDataSet*> meshes)
{
    int  i, j;

    for (i = 0 ; i < domainNum.size() ; i++)
    {
        vtkDataSet *ds = meshes[i];
        int npts = ds->GetNumberOfPoints();

        vtkUnsignedCharArray *gn = vtkUnsignedCharArray::New();
        gn->SetNumberOfTuples(npts);
        gn->SetName("avtGhostNodes");
        unsigned char *gnp = gn->GetPointer(0);
        for (j = 0 ; j < npts ; j++)
            gnp[j] = 0;

        for (j = 0 ; j < giveIndex.size() ; j++)
        {
            if (giveIndex[j].first != domainNum[i])
                continue;
            std::map<int,int> &thisMap = sharedPointsMap[giveIndex[j].first];
            std::map<int,int>::iterator p = thisMap.begin();
            for (p = thisMap.begin() ; p != thisMap.end() ; p++)
            {
                int node = (*p).first;
                avtGhostData::AddGhostNodeType(gnp[node], DUPLICATED_NODE);
            }
        }

        ds->GetPointData()->AddArray(gn);
        gn->Delete();
    }
}


