// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtUnstructuredDomainBoundaries.C                    //
// ************************************************************************* //

#include <avtUnstructuredDomainBoundaries.h>

#include <vtkCellData.h>
#include <vtkCellLinks.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkInformation.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

#include <avtGhostData.h>
#include <avtMaterial.h>
#include <avtMixedVariable.h>
#include <avtParallel.h>

#include <DebugStream.h>
#include <VisItException.h>

#include <algorithm>
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
    template <class T> MPI_Datatype GetMPIDataType();
    template <>        MPI_Datatype GetMPIDataType<int>()    { return MPI_INT;  }
    template <>        MPI_Datatype GetMPIDataType<char>()   { return MPI_CHAR; }
    template <>        MPI_Datatype GetMPIDataType<float>()  { return MPI_FLOAT;}
    template <>        MPI_Datatype GetMPIDataType<double>() { return MPI_DOUBLE;}

    template <>        MPI_Datatype GetMPIDataType<unsigned int>()
                       { return MPI_UNSIGNED; }
    template <>        MPI_Datatype GetMPIDataType<unsigned char>()
                       { return MPI_UNSIGNED_CHAR;}
}

#endif


// ****************************************************************************
//
// Disambiguation:
//
// How does this all work? The Exchange functions are called in 
// avtGenericDatabase and their purpose is to exchange data between all the 
// processors. They are sharing data from every domain to every domain, so 
// there is a 2D structure in each of the functions that is essentially a map
// from sendDom id and recvDom id to data to be exchanged. That data takes the 
// form of objects containing scalars and arrays, which have been defined below
// in classes. Each of the exchange functions calls a corresponding 
// "CommunicateXXXInformation" function that takes the maps to domain data to
// be exchanged and iterates through all send/receive domain pairs, filling out
// the maps to exchange data as they go.
// ****************************************************************************

// ****************************************************************************
// A container for storing domain communication mesh data. There is one of 
// these for each pair of domains.
template <typename T>
class MeshDomainData
{
    // contains data for a relation between two domains
public:
    std::vector<std::array<T, 3>> gainedPoints;
    std::vector<int> cellTypes;
    std::vector<std::vector<int>> cellPoints;
    std::vector<int> origPointIds;
    int nGainedPoints;
    int nGainedCells;
    std::vector<int> nPointsPerCell;
};

// ****************************************************************************
// A container for storing domain communication mixed material data. There is 
// one of these for each pair of domains.
class MixedMaterialDomainData
{
    // contains data for a relation between two domains
public:
    int nGainedCells;
    int nGainedMixlen;
    std::vector<int> gainedMatlist;
    std::vector<int> gainedMixmat;
    std::vector<float> gainedMixvf;
};

// ****************************************************************************
// A container for storing domain communication mixed var data. There is one of
// these for each pair of domains.
class MixedVarDomainData
{
    // contains data for a relation between two domains
public:
    int nGainedMixlen;
    std::vector<float> vals;
};

// ****************************************************************************
// A container for storing domain communication var data. There is one of 
// these for each pair of domains.
template <typename T>
class VarDomainData
{
    // contains data for a relation between two domains
public:
    std::vector<T> gainedData;
    int nGainedTuples;
};

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
//  Method:  avtUnstructuredDomainBoundaries::GetDomIndex
//
//  Purpose:
//    Helper to discover the index of the domain id and do some error checking.
//
//  Arguments:
//    domainNum     the domains that are owned by the current process
//    sendDom       the domain sending data
//    recvDom       the domain receiving data
//
//  Programmer:  Justin Privitera
//  Creation:    April 21, 2025
//
//  Modifications:
//
// ****************************************************************************
int
avtUnstructuredDomainBoundaries::GetDomIndex(const std::vector<int> &domainNum,
                                             const int sendDom,
                                             const int recvDom)
{
    const int domIndex = [&]() -> int
    {
        for (size_t domIndex = 0; domIndex < domainNum.size(); domIndex ++)
        {
            if (domainNum[domIndex] == sendDom)
            {
                return domIndex;
            }
        }
        return -1;
    }();
    if (domIndex < 0)
    {
        std::string err_msg = "avtUnstructuredDomainBoundaries::GetDomIndex "
                              "failed to communicate for sendDom " + std::to_string(sendDom) +
                              " and recvDom " + std::to_string(recvDom);
        EXCEPTION1(VisItException, err_msg);
    }

    return domIndex;
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::GetNMixLen
//
//  Purpose:
//       Assess the amount of mix in cells along the boundary.
//       For each cell "C" along the boundary, assume a counting
//       function F(C), where F(C) returns 0 for clean zones and
//       the number of materials in the zone for mixed zones.
//       We are calculating Sum(F(C)) where Sum is taken over
//       all zones along the boundary of the send and recv Doms.
//
//  Arguments:
//    nCells        the number of cells
//    index         the given index
//    matlist       the material list
//    mix_next      the mix next list
//    sendDom       the domain sending data
//
//  Programmer:  Justin Privitera
//  Creation:    April 21, 2025
//
//  Modifications:
//
// ****************************************************************************
int
avtUnstructuredDomainBoundaries::GetNMixLen(const size_t nCells,
                                            const int index,
                                            const int *matlist,
                                            const int *mix_next,
                                            const int sendDom)
{
    int nMixlen = 0;
    for (int cellId = 0; cellId < nCells; cellId ++)
    {
        const int cell = givenCells[index][cellId];
        if (matlist[cell] >= 0)
            continue;
        int current = -matlist[cell] - 1;
        int nmats = 1;
        // nmats < 1000 just to prevent infinite loops if someone
        // set this structure up wrong.
        while ((mix_next[current] != 0) && (nmats < 1000))
        {
            current = mix_next[current] - 1;
            nmats ++;
        }
        nMixlen += nmats;
        if (nmats >= 1000)
        {
            char str[1024];
            snprintf(str, 1024,
                    "The mixed material entry for cell %d "
                    "of domain %d appears to be invalid.  Unable "
                    "to proceed.", givenCells[index][cellId], sendDom);
            EXCEPTION1(VisItException, str);
        }
    }

    return nMixlen;
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::TransferMatInfo
//
//  Purpose:
//       Workhorse to communicate material information from pointers.
//
//  Arguments:
//    nCells        the number of cells
//    index         the given index
//    matlist       the material list
//    mix_mat       the mixed materials list
//    mix_vf        the material volume fractions
//    mix_next      the mix next list
//    out_matlist   the output material list
//    out_mix_mat   the ouput mixed materials list
//    out_mix_vf    the ouput material volume fractions
//
//  Programmer:  Justin Privitera
//  Creation:    April 21, 2025
//
//  Modifications:
//
// ****************************************************************************
void
avtUnstructuredDomainBoundaries::TransferMatInfo(const size_t nCells,
                                                 const int index,
                                                 const int *matlist,
                                                 const int *mix_mat,
                                                 const float *mix_vf,
                                                 const int *mix_next,
                                                 std::vector<int> &out_matlist,
                                                 std::vector<int> &out_mix_mat,
                                                 std::vector<float> &out_mix_vf)
{
    // assumes the output vectors are sized appropriately

    int mixcnt = 0;
    for (int cellId = 0; cellId < nCells; cellId ++)
    {
        const int cell = givenCells[index][cellId];
        if (matlist[cell] >= 0)
        {
            out_matlist[cellId] = matlist[cell];
        }
        else
        {
            int current = -matlist[cell] - 1;
            int nmats = 0;
            bool seenZero = false;
            bool shouldBreak = false;
            do
            {
                out_mix_mat[mixcnt] = mix_mat[current];
                out_mix_vf[mixcnt]  = mix_vf[current];
                mixcnt ++;
                nmats ++;
                if (seenZero)
                    shouldBreak = true;
                else
                    current = mix_next[current] - 1;
                if (mix_next[current] == 0)
                    seenZero = true;
            } while (!shouldBreak);
            out_matlist[cellId] = -nmats;
        }
    }
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
//  Modifications:
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Refactored into a lambda that is called twice.
//    Style updates.
//
// ****************************************************************************

void
avtUnstructuredDomainBoundaries::SetSharedPoints(int d1, int d2,
                                               const vector<int> &d1pts,
                                               const vector<int> &d2pts)
{
    // we do the same thing twice, just changing the order of the inputs
    auto handleGivenIndex = [&](int dFirst,
                                int dSecond,
                                const vector<int> &dFirstPts,
                                const vector<int> &dSecondPts)
    {
        int index = GetGivenIndex(dFirst, dSecond);

        if (index == -1)
        {
            index = static_cast<int>(giveIndex.size());
            // create the pair in place
            giveIndex.emplace_back(dFirst, dSecond);
            // Use emplace_back to add a default-initialized std::vector<int> 
            // to the vector, avoiding creating a temporary object.
            givenCells.emplace_back();
            givenPoints.emplace_back();
            sharedPointsMap.emplace_back();
        }
        else
        {
            sharedPointsMap[index].clear();
        }

        for (size_t i = 0; i < dFirstPts.size(); ++i)
        {
            sharedPointsMap[index][dFirstPts[i]] = dSecondPts[i];
        }
    };

    handleGivenIndex(d1, d2, d1pts, d2pts);
    // And put in the reverse.
    handleGivenIndex(d2, d1, d2pts, d1pts);
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
//  Modifications:
//    Justin Privitera, Thu Dec 12 16:30:22 PST 2024
//    Prevent there from being given cells when there are no given points.
//    Later logic unconditionally sends cells without checking if points
//    exist or not. The correct thing to do is to send 0 for num cells if
//    num points is 0.
// 
//    Justin Privitera, Wed Apr  2 16:17:04 PDT 2025
//    Fixed a bug and logic error when the given index was -1, potentially
//    causing index out of bounds errors later in the function.
//    Removed extraneous index calculation.
//    Style updates.
//    Use good vector semantics.
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
        index = static_cast<int>(giveIndex.size());
        giveIndex.emplace_back(fromDom, toDom);
        givenCells.push_back(cells);
        // Use emplace_back to add a default-initialized std::vector<int> 
        // to the vector, avoiding creating a temporary object.
        givenPoints.emplace_back();
        sharedPointsMap.emplace_back();
    }
    else
    {
        givenCells[index] = cells;
    }

    if (!filterShared)
    {
        givenPoints[index] = points;
    }
    else
    {
        map<int, int> &smap = sharedPointsMap[index];

        // Go through and manually insert the points that are not shared.

        for (size_t i = 0; i < points.size(); ++i)
        {
            if (smap.find(points[i]) == smap.end())
            {
                givenPoints[index].push_back(points[i]);
            }
        }
    }

    // if the number of points to share is 0, then we cannot share cells.
    if (givenPoints[index].empty())
    {
        givenCells[index].clear();
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
//  Modifications:
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Style updates.
//    Added const where possible.
//
// ****************************************************************************

int
avtUnstructuredDomainBoundaries::GetGivenIndex(const int from, const int to)
{
    for (size_t i = 0; i < giveIndex.size(); ++i)
    {
        if (giveIndex[i].first == from && giveIndex[i].second == to)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
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
//    Hank Childs, Tue Jun 21 13:59:47 PDT 2005
//    Fix UMR and memory leak.
//
//    Hank Childs, Fri May 19 13:29:29 PDT 2006
//    Code around VTK memory leak.
//
//    Brad Whitlock, Mon Jun  2 16:05:42 PDT 2014
//    Reinstate call to BuildLinks since it does not leak with the new VTK
//    (and the old workaround did leak with the new VTK).
//
//    Kathleen Biagas, Mon Aug 15 14:09:55 PDT 2016
//    VTK-8, API for updating GhostLevel changed.
//
//    Eric Brugger, Fri Mar 13 15:20:08 PDT 2020
//    Modify to handle NULL meshes.
//
//    Eric Brugger, Mon May 24 11:38:21 PDT 2021
//    Modify to handle meshes with no points or cells.
// 
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Style updates, changed index variable names to be more descriptive.
//    Added const where possible.
//    Use a map of class objects instead of pointers.
//    Use STL containers where possible.
//    Error checking.
//    Use new helpers.
//
// ****************************************************************************

vector<vtkDataSet*>
avtUnstructuredDomainBoundaries::ExchangeMesh(vector<int>       domainNum,
                                            vector<vtkDataSet*> meshes)
{
    if (!meshes.size())
    {
        return ExchangeMeshT<float>(domainNum, meshes);
    }

    int nonNullDomain = 0;
    while (nonNullDomain < meshes.size() && meshes[nonNullDomain] == NULL)
    {
        nonNullDomain++;
    }
    vtkPoints *pts = vtkUnstructuredGrid::SafeDownCast(meshes[nonNullDomain])->GetPoints();

    switch (pts->GetDataType())
    {
        case VTK_FLOAT:
            return ExchangeMeshT<float>(domainNum, meshes);
        case VTK_DOUBLE:
            return ExchangeMeshT<double>(domainNum, meshes);
        case VTK_INT:
            return ExchangeMeshT<int>(domainNum, meshes);
        default:
            string exc_mesg = "avtUnstructuredDomainBoundaries does not know "
                              "how to exchange meshes from array type "
                              + string(pts->GetData()->GetClassName());
            EXCEPTION1(VisItException, exc_mesg);
    }
    // (To avoid compiler warnings). This code is never reached.
    return meshes;
}

// ****************************************************************************
template <typename T>
vector<vtkDataSet*>
avtUnstructuredDomainBoundaries::ExchangeMeshT(vector<int>         domainNum,
                                               vector<vtkDataSet*> meshes)
{
    vector<vtkDataSet*> out(meshes.size(), NULL);

    // Gather the information we need
    vector<int> domain2proc = CreateDomainToProcessorMap(domainNum);

    // Instead of having many disparate pointers, we have grouped them into a
    // single structure we are calling domaindata. There is one domaindata for
    // every pair of domains, hence the map from "sendDom" indices to maps
    // from "recvDom" indices to domaindata objects. So we can get domaindata
    // for a pair of domains like so: domaindata[sendDom][recvDom].
    std::map<int, std::map<int, MeshDomainData<T>>> domaindata;
    CommunicateMeshInformation(domain2proc, domainNum, meshes, domaindata);

    for (size_t domIdIndex = 0; domIdIndex < domainNum.size(); domIdIndex ++)
    {
        const int recvDom = domainNum[domIdIndex];

        vtkUnstructuredGrid *mesh = static_cast<vtkUnstructuredGrid *>(meshes[domIdIndex]);
        if (nullptr == mesh ||
            mesh->GetNumberOfPoints() == 0 || 
            mesh->GetNumberOfCells() == 0)
        {
            continue;
        }

        const int nOldPoints = mesh->GetNumberOfPoints();

        // Find how many points are given to domain recvDom.
        const int nGivenPoints = [&]()
        {
            int sum = 0;
            for (int sendDom = 0; sendDom < nTotalDomains; sendDom ++)
            {
                sum += domaindata[sendDom][recvDom].nGainedPoints;
            }
            return sum;
        }();

        // Create the VTK objects
        vtkUnstructuredGrid *outMesh = vtkUnstructuredGrid::New();
        vtkPoints *outPoints = vtkPoints::New(mesh->GetPoints()->GetDataType());

        outMesh->DeepCopy(meshes[domIdIndex]);
        outMesh->SetPoints(outPoints);
        outPoints->Delete();
        outPoints->SetNumberOfPoints(nOldPoints + nGivenPoints);

        // Copy the old coordinates over
        const T *oldcoord = static_cast<T *>(mesh->GetPoints()->GetVoidPointer(0));
        T *newcoord = static_cast<T *>(outPoints->GetVoidPointer(0));
        std::copy(oldcoord, oldcoord + (3 * nOldPoints), newcoord);

        // Put in the new coordinates
        std::vector<std::map<int, int>> translatedPointsMap(nTotalDomains);
        newcoord += 3 * nOldPoints;
        int newId = nOldPoints;
        for (int sendDom = 0; sendDom < nTotalDomains; sendDom ++)
        {
            if (sendDom == recvDom)
                continue;

            // create references for the domain data here
            MeshDomainData<T> &currDomainData = domaindata[sendDom][recvDom];
            std::vector<std::array<T, 3>> &gainedPoints   = currDomainData.gainedPoints;
            std::vector<int>              &origPointIds   = currDomainData.origPointIds;
            int                           &nGainedPoints  = currDomainData.nGainedPoints;

            const int &nGainedPointsThisDomain = nGainedPoints;
            if (nGainedPointsThisDomain == 0)
                continue;

            // We need to remember what the point id for this exchange
            // of points is.
            startingPoint[std::make_pair(sendDom, recvDom)] = newId;

            for (int gainedPtId = 0; gainedPtId < nGainedPointsThisDomain; gainedPtId ++)
            {
                *(newcoord++) = gainedPoints[gainedPtId][0];
                *(newcoord++) = gainedPoints[gainedPtId][1];
                *(newcoord++) = gainedPoints[gainedPtId][2];
                translatedPointsMap[sendDom][origPointIds[gainedPtId]] = (newId ++);
            }
        }

        const int nOldCells = outMesh->GetNumberOfCells();

        vtkIdList *idList = vtkIdList::New();
        // Put in the new cells
        for (int sendDom = 0; sendDom < nTotalDomains; ++sendDom)
        {
            if (recvDom == sendDom)
                continue;

            // create references for the domain data here
            MeshDomainData<T> &currDomainData = domaindata[sendDom][recvDom];
            std::vector<int>              &cellTypes      = currDomainData.cellTypes;
            std::vector<std::vector<int>> &cellPoints     = currDomainData.cellPoints;
            int                           &nGainedCells   = currDomainData.nGainedCells;
            std::vector<int>              &nPointsPerCell = currDomainData.nPointsPerCell;

            const int &nGainedCellsThisDomain = nGainedCells;

            if (nGainedCellsThisDomain == 0)
                continue;

            // We're going to be giving cells from sendDom to recvDom.
            // The id that the cells will be inserted at is
            // important, and we need to remember.
            startingCell[std::make_pair(sendDom, recvDom)] = outMesh->GetNumberOfCells();

            // We want the map that indexes the ptIds from sendDom into
            // the ptIds of recvDom.
            const int index = GetGivenIndex(sendDom, recvDom);
            std::map<int, int> &smap = sharedPointsMap[index];
            std::map<int, int> &tmap = translatedPointsMap[sendDom];

            for (int gainedCellId = 0; gainedCellId < nGainedCellsThisDomain; gainedCellId ++)
            {
                const int nPointsThisCell = nPointsPerCell[gainedCellId];
                idList->SetNumberOfIds(nPointsThisCell);
                for (int ptId = 0; ptId < nPointsThisCell; ptId ++)
                {
                    const int id = cellPoints[gainedCellId][ptId];
                    if (smap.find(id) != smap.end())
                    {
                        idList->SetId(ptId, smap[id]);
                    }
                    else
                    {
                       idList->SetId(ptId, tmap[id]);
                    }
                }
                outMesh->InsertNextCell(cellTypes[gainedCellId], idList);
            }
        }
        idList->Delete();

        // Create the ghost zone array

        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
        ghostCells->SetName("avtGhostZones");
        ghostCells->SetNumberOfTuples(outMesh->GetNumberOfCells());
        unsigned char *ptr = ghostCells->GetPointer(0);
        for (int i = 0; i < nOldCells; ++i)
            *(ptr++) = 0;
        int nGhostCells = outMesh->GetNumberOfCells() - nOldCells;
        for (int i = 0; i < nGhostCells; ++i)
        {
            *ptr = 0;
            avtGhostData::AddGhostZoneType(*ptr,
                                          DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
            ptr++;
        }
        outMesh->GetCellData()->AddArray(ghostCells);
        ghostCells->Delete();
        outMesh->GetInformation()->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(), 0);

        // Rebuild the links now that we've added ghost cells.
        outMesh->BuildLinks();
        out[domIdIndex] = outMesh;
    }

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
//  Modifications:
//    Brad Whitlock, Thu Sep 16 12:55:50 PDT 2004
//    I replaced specialized ExchangeData function calls with macros that
//    will allow the compiler to instantiate a function somewhat differently
//    depending on the platform to work around a problem with templates
//    using the MSVC6.0 compiler on Windows.
//
//    Brad Whitlock, Sun Apr 22 10:36:38 PDT 2012
//    Double support.
//
//    Eric Brugger, Fri Mar 13 15:20:08 PDT 2020
//    Modify to handle NULL meshes.
//
//    Kathleen Biagas, Thu Oct 31, 2024
//    Ensure all procs are calling the same Exchange function.
//
//    Kathleen Biagas, Fri Nov 1, 2024
//    Added consistency check for dataTypes.
// 
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Style updates.
//    Error checking.
//
// ****************************************************************************

vector<vtkDataArray*>
avtUnstructuredDomainBoundaries::ExchangeScalar(vector<int>           domainNum,
                                                bool                  isPointData,
                                                vector<vtkDataArray*> scalars)
{
    int nonNullDomain = 0;
    int dataType = -1;
    if (! scalars.empty())
    {
        if (domainNum.size() != scalars.size())
        {
            // This should never happen, so throw the exception.
            EXCEPTION1(VisItException,
                       "avtUnstructuredDomainBoundaries:ExchangeScalar "
                       "mismatch between number of domains and per-domain scalars.");
        }
        while (nonNullDomain < domainNum.size() && scalars[nonNullDomain] == NULL)
        {
            nonNullDomain ++;
        }
        dataType = scalars[nonNullDomain]->GetDataType();
    }

    int maxDataType = dataType;
#ifdef PARALLEL
    // Let's get them all to agree on one data type.
    MPI_Allreduce(&dataType, &maxDataType, 1, MPI_INT, MPI_MAX, VISIT_MPI_COMM);

    int hasDataTypeMismatch = ((dataType >= 0) && (dataType != maxDataType));
    int hasDataTypeMismatchMax = hasDataTypeMismatch;
    MPI_Allreduce(&hasDataTypeMismatch, &hasDataTypeMismatchMax, 1, MPI_INT, MPI_MAX, VISIT_MPI_COMM);
    if(hasDataTypeMismatchMax)
    {
        // This should never happen, so throw the exception.
        EXCEPTION1(VisItException,
                   "avtUnstructuredDomainBoundaries:ExchangeScalar "
                   "vtkDataArray data types do not match.");
    }
#endif

    if (maxDataType < 0)
        return scalars;

    // This one's a little more complicated because there are different
    // types of scalars we might encounter. If more cases arise,
    // expand this function.
    switch (maxDataType)
    {
        case VTK_INT:
            return ExchangeData<int>(domainNum, isPointData, scalars);
        case VTK_CHAR:
            return ExchangeData<char>(domainNum, isPointData, scalars);
        case VTK_FLOAT:
            return ExchangeData<float>(domainNum, isPointData, scalars);
        case VTK_DOUBLE:
            return ExchangeData<double>(domainNum, isPointData, scalars);
        case VTK_UNSIGNED_CHAR:
            return ExchangeData<unsigned char>(domainNum, isPointData, scalars);
        case VTK_UNSIGNED_INT:
            return ExchangeData<unsigned int>(domainNum, isPointData, scalars);
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
//  Method:  avtUnstructuredDomainBoundaries::ExchangeVector
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
//  Programmer:  Kevin Griffin
//  Creation:    April 21, 2015
//
//  Modifications:
//    Eric Brugger, Fri Mar 13 15:20:08 PDT 2020
//    Modify to handle NULL meshes.
//
//    Kathleen Biagas, Thu Oct 31, 2024
//    Ensure all procs are calling the same Exchange function.
//
//    Kathleen Biagas, Fri Nov 1, 2024
//    Added consistency check for dataTypes.
// 
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Style updates.
//    Error checking.
//
// ****************************************************************************

vector<vtkDataArray*>
avtUnstructuredDomainBoundaries::ExchangeVector(vector<int> domainNum,
                                                bool isPointData, 
                                                vector<vtkDataArray*> vectors)
{
    int nonNullDomain = 0;
    int dataType = -1;
    if (! vectors.empty())
    {
        if (domainNum.size() != vectors.size())
        {
            // This should never happen, so throw the exception.
            EXCEPTION1(VisItException,
                       "avtUnstructuredDomainBoundaries:ExchangeVector "
                       "mismatch between number of domains and per-domain vectors.");
        }
        while (nonNullDomain < domainNum.size() && vectors[nonNullDomain] == NULL)
        {
            nonNullDomain ++;
        }
        dataType = vectors[nonNullDomain]->GetDataType();
    }

    int maxDataType = dataType;
#ifdef PARALLEL
    // Let's get them all to agree on one data type.
    MPI_Allreduce(&dataType, &maxDataType, 1, MPI_INT, MPI_MAX, VISIT_MPI_COMM);

    int hasDataTypeMismatch = ((dataType >= 0) && (dataType != maxDataType));
    int hasDataTypeMismatchMax = hasDataTypeMismatch;
    MPI_Allreduce(&hasDataTypeMismatch, &hasDataTypeMismatchMax, 1, MPI_INT, MPI_MAX, VISIT_MPI_COMM);
    if(hasDataTypeMismatchMax)
    {
        // This should never happen, so throw the exception.
        EXCEPTION1(VisItException,
                   "avtUnstructuredDomainBoundaries:ExchangeVector "
                   "vtkDataArray data types do not match.");
    }
#endif

    if (maxDataType < 0)
        return vectors;

    // This one's a little more complicated because there are different
    // types of vectors we might encounter. If more cases arise,
    // expand this function.
    switch (maxDataType)
    {
        case VTK_FLOAT:
            return ExchangeData<float>(domainNum, isPointData, vectors);
            break;
        case VTK_DOUBLE:
            return ExchangeData<double>(domainNum, isPointData, vectors);
            break;
        case VTK_INT:
        case VTK_UNSIGNED_INT:
            return ExchangeData<int>(domainNum, isPointData, vectors);
            break;
        default:
            EXCEPTION1(VisItException, "Unknown vector type in "
                       "avtUnstructuredDomainBoundaries::ExchangeVector");
    }
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::ExchangeMaterial
//
//  Purpose:
//    Exchange the ghost zone information for some materials,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    material     an array of materials
//
//  Programmer:  Hank Childs
//  Creation:    March 9, 2007
//
//  Modifications:
//    Eric Brugger, Fri Mar 13 15:20:08 PDT 2020
//    Modify to handle NULL meshes.
//
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Style updates.
//    Added const where possible.
//    Error checking.
// 
// ****************************************************************************

vector<avtMaterial*>
avtUnstructuredDomainBoundaries::ExchangeMaterial(vector<int>    domainNum,
                                                  vector<avtMaterial*> mats)
{
    bool haveMixedMaterials = false;
    if (domainNum.size() != mats.size())
    {
        // This should never happen, so throw the exception.
        EXCEPTION1(VisItException,
                   "avtUnstructuredDomainBoundaries:ExchangeMaterial "
                   "mismatch between number of domains and per-domain materials.");
    }
    for (size_t i = 0; i < domainNum.size(); i++)
        if (mats[i] != NULL && mats[i]->GetMixlen() != 0)
            haveMixedMaterials = true;

    const int max = UnifyMaximumValue(haveMixedMaterials ? 1 : 0);
    haveMixedMaterials = (max > 0 ? true : false);

    vector<avtMaterial*> rv;
    if (haveMixedMaterials)
        rv = ExchangeMixedMaterials(domainNum, mats);
    else
        rv = ExchangeCleanMaterials(domainNum, mats);

    return rv;
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::ExchangeMixedMaterials
//
//  Purpose:
//    Exchange the ghost zone information for some materials,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    material     an array of materials
//
//  Programmer:  Hank Childs
//  Creation:    February 13, 2007
//
//  Modifications:
//    Mark C. Miller, Mon Feb  9 17:11:23 PST 2015
//    Adjust memcpy calls to copy minimum size of old/new buffers. I used
//    MIN instead of always using old sizes because I was not absolutely sure
//    whether the new sizes could indeed be smaller.
//
//    Eric Brugger, Fri Mar 13 15:20:08 PDT 2020
//    Modify to handle NULL meshes.
// 
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Style updates, changed index variable names to be more descriptive.
//    Added const where possible.
//    Use a map of class objects instead of pointers.
//    Use STL containers where possible.
//    Use std::copy instead of memcpy.
//
// ****************************************************************************

vector<avtMaterial*>
avtUnstructuredDomainBoundaries::ExchangeMixedMaterials(vector<int> domainNum,
                                                        vector<avtMaterial*> mats)
{
    vector<int> domain2proc = CreateDomainToProcessorMap(domainNum);

    // Instead of having many disparate pointers, we have grouped them into a
    // single structure we are calling domaindata. There is one domaindata for
    // every pair of domains, hence the map from "sendDom" indices to maps
    // from "recvDom" indices to domaindata objects. So we can get domaindata
    // for a pair of domains like so: domaindata[sendDom][recvDom].
    std::map<int, std::map<int, MixedMaterialDomainData>> domaindata;
    CommunicateMaterialInformation(domain2proc, domainNum, mats, domaindata);

    vector<avtMaterial*> out(mats.size(), NULL);

    for (size_t domIdIndex = 0; domIdIndex < domainNum.size(); domIdIndex ++)
    {
        const int recvDom = domainNum[domIdIndex];

        avtMaterial *oldMat = mats[domIdIndex];
        if (nullptr == oldMat)
        {
            continue;
        }

        //
        // Estimate the sizes we will need for the new object.
        //
        const int oldNCells  = oldMat->GetNZones();
        const int oldMixlen  = oldMat->GetMixlen();
        int       newNCells  = oldNCells;
        int       newMixlen  = oldMixlen;
        for (int sendDom = 0; sendDom < nTotalDomains; sendDom ++)
        {
            newNCells += domaindata[sendDom][recvDom].nGainedCells;
            newMixlen += domaindata[sendDom][recvDom].nGainedMixlen;
        }

        const int realNCells = std::min(oldNCells, newNCells);
        const int realMixlen = std::min(oldMixlen, newMixlen);

        //
        // Start by copying in everything from this domain's material object.
        //
        std::vector<int> new_matlist(newNCells);
        const int *old_matlist = mats[domIdIndex]->GetMatlist();
        std::copy(old_matlist, old_matlist + realNCells, new_matlist.begin());

        std::vector<int> new_mix_next(newMixlen);
        const int *old_mix_next = mats[domIdIndex]->GetMixNext();
        std::copy(old_mix_next, old_mix_next + realMixlen, new_mix_next.begin());

        std::vector<int> new_mix_mat(newMixlen);
        const int *old_mix_mat = oldMat->GetMixMat();
        std::copy(old_mix_mat, old_mix_mat + realMixlen, new_mix_mat.begin());

        std::vector<float> new_mix_vf(newMixlen);
        const float *old_mix_vf = oldMat->GetMixVF();
        std::copy(old_mix_vf, old_mix_vf + realMixlen, new_mix_vf.begin());

        std::vector<int> new_mix_zone(newMixlen);
        const int *old_mix_zone = oldMat->GetMixZone();
        std::copy(old_mix_zone, old_mix_zone + realMixlen, new_mix_zone.begin());

        //
        // Now copy over the ghost information.  By iterating over the
        // domains in order, we will be constructing ghost information in
        // the exact same order as when we construct ghost zones for the mesh.
        //
        int matlist_cnt = oldMat->GetNZones();
        int mixlen_cnt  = oldMat->GetMixlen();
        for (int sendDom = 0 ; sendDom < nTotalDomains ; sendDom ++)
        {
            // create references for the domain data here
            MixedMaterialDomainData &currDomainData = domaindata[sendDom][recvDom];
            const int                &nGainedCells  = currDomainData.nGainedCells;
            const std::vector<int>   &gainedMatlist = currDomainData.gainedMatlist;
            const std::vector<int>   &gainedMixmat  = currDomainData.gainedMixmat;
            const std::vector<float> &gainedMixvf   = currDomainData.gainedMixvf;

            int lml = 0; // "local" mixlen ... mixlen counter for this domain
            for (int mixedCellIdx = 0; mixedCellIdx < nGainedCells; mixedCellIdx ++)
            {
                if (gainedMatlist[mixedCellIdx] >= 0)
                {
                    new_matlist[matlist_cnt] = gainedMatlist[mixedCellIdx];
                    matlist_cnt ++;
                }
                else
                {
                    // Decode how many materials in the cell.
                    new_matlist[matlist_cnt] = -(mixlen_cnt + 1);
                    const int nmats = -gainedMatlist[mixedCellIdx];
                    matlist_cnt ++;
                    for (int matId = 0; matId < nmats; matId ++)
                    {
                        new_mix_mat[mixlen_cnt]  = gainedMixmat[lml];
                        new_mix_vf[mixlen_cnt]   = gainedMixvf[lml];
                        new_mix_zone[mixlen_cnt] = -1;
                        new_mix_next[mixlen_cnt] = (matId < nmats - 1 ? mixlen_cnt + 2 : 0);
                        lml ++;
                        mixlen_cnt ++;
                    }
                }
            }
        }

        out[domIdIndex] = new avtMaterial(oldMat->GetNMaterials(),
                                     oldMat->GetMaterials(), newNCells,
                                     new_matlist.data(), newMixlen, new_mix_mat.data(),
                                     new_mix_next.data(), new_mix_zone.data(), new_mix_vf.data());
    }

    return out;
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::ExchangeCleanMaterials
//
//  Purpose:
//    Exchange the ghost zone information for some materials,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    material     an array of materials
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
//  Modifications:
//    Brad Whitlock, Thu Sep 16 12:55:50 PDT 2004
//    I replaced specialized ExchangeData function calls with macros that
//    will allow the compiler to instantiate a function somewhat differently
//    depending on the platform to work around a problem with templates
//    using the MSVC6.0 compiler on Windows.
//
//    Hank Childs, Tue Feb 13 16:42:36 PST 2007
//    Renamed method to ExchangeCleanMaterials.  This method is now only called
//    if we know that the materials are clean.
//
//    Eric Brugger, Fri Mar 13 15:20:08 PDT 2020
//    Modify to handle NULL meshes.
// 
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Style updates, changed index variable names to be more descriptive.
//    Added const where possible.
//
// ****************************************************************************

vector<avtMaterial*>
avtUnstructuredDomainBoundaries::ExchangeCleanMaterials(vector<int> domainNum,
                                                        vector<avtMaterial*> mats)
{
    //
    // Load the materials into data arrays.
    //
    vector<vtkDataArray *> materialArrays(domainNum.size());
    for (size_t domId = 0; domId < domainNum.size(); domId ++)
    {
        if (mats[domId] == NULL)
            continue;

        // This should never happen, but it doesn't hurt to check.
        if (mats[domId]->GetMixlen() != 0)
        {
            EXCEPTION1(VisItException, "Internal error.  This method should "
                                       "not have been called if there were "
                                       "mixed materials.");
        }

        materialArrays[domId] = vtkIntArray::New();
        const int nZones = mats[domId]->GetNZones();
        materialArrays[domId]->SetNumberOfTuples(nZones);

        int *ptr = static_cast<int *>(materialArrays[domId]->GetVoidPointer(0));
        const int *matPtr = mats[domId]->GetMatlist();

        for (int zoneId = 0; zoneId < nZones; zoneId ++)
        {
            *(ptr++) = *(matPtr++);
        }
    }

    vector<vtkDataArray *> result;
    result = ExchangeData<int>(domainNum, false, materialArrays);

    vector<avtMaterial*> out(mats.size(), NULL);

    for (size_t domId = 0; domId < domainNum.size(); domId ++)
    {
        if (mats[domId] == NULL)
            continue;

        const int nMaterials = mats[domId]->GetNMaterials();
        const int nZones = result[domId]->GetNumberOfTuples();
        int *matPtr = static_cast<int *>(result[domId]->GetVoidPointer(0));

        out[domId] = new avtMaterial(nMaterials, mats[domId]->GetMaterials(),
                                     nZones, matPtr, 0,
                                     NULL, NULL, NULL, NULL);

        materialArrays[domId]->Delete();
        result[domId]->Delete();
    }

    return out;
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::ExchangeMixVar
//
//  Purpose:
//    Exchange the ghost zone information for some mixvars,
//    returning the new ones.
//
//  Arguments:
//    domainNum    an array of domain numbers for each mesh
//    mats         The materials, which are needed to determine lengths.
//    mixvar       an array of mixvars
//
//  Programmer:  Hank Childs
//  Creation:    March 10, 2007
//
//  Modifications:
//
//    Hank Childs, Tue Mar  4 13:29:48 PST 2008
//    Account for domains that do not have mixed variables.
//
//    Kathleen Bonnell, Thu Apr 10 17:56:33 PDT 2008
//    Removed redefinition of 'i'.
// 
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Style updates, changed index variable names to be more descriptive.
//    Added const where possible.
//    Use a map of class objects instead of pointers.
//    Use STL containers where possible.
//    Fixed memory leak with mix variable name never being freed.
//    Early termination in for loop when we find an answer.
//    Struct is now named.
//    Use std::copy instead of memcpy.
//
// ****************************************************************************

vector<avtMixedVariable*>
avtUnstructuredDomainBoundaries::ExchangeMixVar(std::vector<int>                domainNum,
                                                const std::vector<avtMaterial*> mats,
                                                std::vector<avtMixedVariable*>  mixvars)
{
    std::vector<int> domain2proc = CreateDomainToProcessorMap(domainNum);

    // Instead of having many disparate pointers, we have grouped them into a
    // single structure we are calling domaindata. There is one domaindata for
    // every pair of domains, hence the map from "sendDom" indices to maps
    // from "recvDom" indices to domaindata objects. So we can get domaindata
    // for a pair of domains like so: domaindata[sendDom][recvDom].
    std::map<int, std::map<int, MixedVarDomainData>> domaindata;
    CommunicateMixvarInformation(domain2proc, domainNum, mats,
                                 mixvars, domaindata);

    std::vector<avtMixedVariable *> out(mixvars.size(), nullptr);

    //
    // Pretty ugly -- we need to come up with the mixed variable's name.  It
    // could be that not a single domain on the processor has a valid mixed
    // variable.  In that case we need to do global communication to find it.
    // (It really does happen that a domain with no mixed zones gets ghost
    // zones that are mixed.)
    //
    std::string mixvarname = "";
    for (size_t i = 0; i < mixvars.size(); i++)
    {
        if (nullptr != mixvars[i])
        {
            mixvarname = mixvars[i]->GetVarname();
            break;
        }
    }

#ifdef PARALLEL
    int rank;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);

    int length = 0;
    if (! mixvarname.empty())
    {
        length = static_cast<int>(mixvarname.size()) + 1;
    }

    struct LenRank {
        int length;
        int rank;
    };

    LenRank len_rank_in = {length, rank};
    LenRank len_rank_out;

    MPI_Allreduce(&len_rank_in, &len_rank_out, 1, MPI_2INT, MPI_MAXLOC,
                  VISIT_MPI_COMM);
    length = len_rank_out.length;

    // allocate with the desired length
    std::string mvname(length, '\0');
    if (! mixvarname.empty())
    {
        // Copy mixvarname into mvname
        mvname.replace(0, mixvarname.size(), mixvarname);
    }

    MPI_Bcast(&mvname[0], length, MPI_CHAR, len_rank_out.rank, VISIT_MPI_COMM);
    mixvarname = mvname;
#endif

    for (size_t domIdIndex = 0; domIdIndex < domainNum.size(); ++domIdIndex)
    {
        const int recvDom = domainNum[domIdIndex];

        avtMixedVariable *oldMV = mixvars[domIdIndex];

        //
        // Estimate the sizes we will need for the new object.
        //
        const int oldMixlen = (nullptr != oldMV ? oldMV->GetMixlen() : 0);
        int newMixlen = oldMixlen;
        for (int sendDom = 0; sendDom < nTotalDomains; sendDom ++)
        {
            newMixlen += domaindata[sendDom][recvDom].nGainedMixlen;
        }

        if (newMixlen <= 0)
        {
            out[domIdIndex] = nullptr;
            continue;
        }

        //
        // Start by copying in everything from this domain's mixvar object.
        //
        std::vector<float> new_buff(newMixlen);
        int mixlen_cnt = oldMixlen;
        if (mixlen_cnt > 0)
        {
            const float *old_buff = oldMV->GetBuffer();
            std::copy(old_buff, old_buff + mixlen_cnt, new_buff.begin());
        }

        //
        // Now copy over the ghost information.  By iterating over the
        // domains in order, we will be constructing ghost information in
        // the exact same order as when we construct ghost zones for the mesh.
        //
        for (int sendDom = 0; sendDom < nTotalDomains; sendDom ++)
        {
            // create references for the domain data here
            MixedVarDomainData &currDomainData = domaindata[sendDom][recvDom];
            const int                &nGainedMixlen = currDomainData.nGainedMixlen;
            const std::vector<float> &vals          = currDomainData.vals;

            std::copy(vals.begin(), 
                      vals.begin() + nGainedMixlen,
                      new_buff.begin() + mixlen_cnt);

            mixlen_cnt += nGainedMixlen;
        }

        out[domIdIndex] = new avtMixedVariable(new_buff.data(),
                                          newMixlen,
                                          mixvarname.c_str());
    }

    return out;
}


// ****************************************************************************
//  Method: avtUnstructuredDomainBoundaries::RequiresCommunication
//
//  Purpose:
//      Determines if this domain boundaries object will need to perform
//      collective communication to create the type of ghost data requested.
//
//  Programmer: Hank Childs
//  Creation:   February 27, 2005
// 
//  Modifications:
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Simplified so it is a one-liner.
//
// ****************************************************************************

bool
avtUnstructuredDomainBoundaries::RequiresCommunication(avtGhostDataType gtype)
{
    // we want to return true only if there are ghost zones.
    // if there are only ghost nodes or no ghost data, then no communication
    // is required.
    return gtype == GHOST_ZONE_DATA;
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
//  Modifications:
//
//    Mark C. Miller, Thu Mar  9 11:15:29 PST 2006
//    Protected loop with checks for null mesh pointers
//
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Style updates.
//    Added const where possible.
//
// ****************************************************************************

bool
avtUnstructuredDomainBoundaries::ConfirmMesh(vector<int>       domainNum,
                                            vector<vtkDataSet*> meshes)
{
    // Our best bet is to compare some shared points.
    for (size_t i = 0; i < domainNum.size(); ++i)
    {
        if (meshes[i] == 0)
            continue;

        const int d1 = domainNum[i];
        for (size_t j = i + 1; j < domainNum.size(); ++j)
        {
            const int d2 = domainNum[j];
            const int index = GetGivenIndex(d1, d2);
            if (index < 0)
                continue;

            map<int, int> &smap = sharedPointsMap[index];
            if (smap.size() == 0)
                continue;

            if (meshes[j] == 0)
                continue;

            // Found a shared domain, let's take a look.
            const int d1ptId = smap.begin()->first;
            const int d2ptId = smap.begin()->second;

            double pt1[3], pt2[3];
            meshes[i]->GetPoint(d1ptId, pt1);
            meshes[j]->GetPoint(d2ptId, pt2);

            constexpr double epsilon = 1e-12;

            // If these points are too dissimilar, it has to be
            // referring to another mesh.
            if (epsilon < (fabs(pt1[0] - pt2[0]) + 
                           fabs(pt1[1] - pt2[1]) + 
                           fabs(pt1[2] - pt2[2])))
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
//  Modifications:
//    Brad Whitlock, Thu Sep 16 12:58:27 PDT 2004
//    I added conditionally compiled code to work around an apparent template
//    instantiation bug that in the MSVC6.0 compiler that prevented VisIt
//    from building on Windows. I added an argument to contribute to the
//    method signature and the contents of CommunicateDataInformation,
//    which had to be inlined to get it to compile on Windows.
//
//    Hank Childs, Wed Feb 14 15:48:00 PST 2007
//    Fix bug where last entry in the array was being overwritten.
//
//    Brad Whitlock, Sun Apr 22 10:38:45 PDT 2012
//    Remove MSVC 6 code.
//
//    Eric Brugger, Fri Mar 13 15:20:08 PDT 2020
//    Modify to handle NULL meshes.
// 
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Style updates, changed index variable names to be more descriptive.
//    Added const where possible.
//    Use a map of class objects instead of pointers.
//    Use STL containers where possible.
//
// ****************************************************************************

template <typename T>
vector<vtkDataArray*>
avtUnstructuredDomainBoundaries::ExchangeData(vector<int>         &domainNum,
                                              bool                isPointData,
                                              vector<vtkDataArray*> &data)
{
    // Gather the needed information
    vector<int> domain2proc = CreateDomainToProcessorMap(domainNum);
    
    // Instead of having many disparate pointers, we have grouped them into a
    // single structure we are calling domaindata. There is one domaindata for
    // every pair of domains, hence the map from "sendDom" indices to maps
    // from "recvDom" indices to domaindata objects. So we can get domaindata
    // for a pair of domains like so: domaindata[sendDom][recvDom].
    std::map<int, std::map<int, VarDomainData<T>>> domaindata;
    CommunicateDataInformation<T>(domain2proc, domainNum, data, isPointData,
                                  domaindata);

    vector<vtkDataArray*> out(data.size(), NULL);

    int nComponents = 0;
    int nonNullDomain =0;
    while (nonNullDomain < data.size() && nullptr == data[nonNullDomain])
    {
        nonNullDomain ++;
    }
    if (nonNullDomain < data.size())
    {
        nComponents = data[nonNullDomain]->GetNumberOfComponents();
    }

    for (size_t domIdIndex = 0; domIdIndex < domainNum.size(); domIdIndex ++)
    {
        const int recvDom = domainNum[domIdIndex];
        if (nullptr == data[domIdIndex])
            continue;
        out[domIdIndex] = data[domIdIndex]->NewInstance();

        out[domIdIndex]->DeepCopy(data[domIdIndex]);
        out[domIdIndex]->SetName(data[domIdIndex]->GetName());

        int nGivenTuples = 0;
        for (int sendDom = 0; sendDom < nTotalDomains; ++sendDom)
        {
            if (recvDom == sendDom)
                continue;

            nGivenTuples += domaindata[sendDom][recvDom].nGainedTuples;
        }

        if (nGivenTuples > 0)
        {
            out[domIdIndex]->Resize(nGivenTuples + out[domIdIndex]->GetNumberOfTuples());

            // This properly sets the internal size.
            out[domIdIndex]->InsertTuple(data[domIdIndex]->GetNumberOfTuples() + nGivenTuples - 1,
                                         data[domIdIndex]->GetTuple(0));
        }

        for (int sendDom = 0; sendDom < nTotalDomains; sendDom ++)
        {
            if (sendDom == recvDom)
            {
                continue;
            }

            // create references for the domain data here
            VarDomainData<T> &currDomainData = domaindata[sendDom][recvDom];
            const std::vector<T> &gainedData    = currDomainData.gainedData;
            const int            &nGainedTuples = currDomainData.nGainedTuples;

            if (0 == nGainedTuples)
            {
                continue;
            }

            const int refIndex = (isPointData ? startingPoint[pair<int,int>(sendDom, recvDom)]
                                              : startingCell[pair<int,int>(sendDom, recvDom)]);
            T *ptr = static_cast<T *>(out[domIdIndex]->GetVoidPointer(refIndex * nComponents));
            const int nDatCopy = nGainedTuples * nComponents;
            for (int idx = 0; idx < nDatCopy; idx ++)
            {
                (*ptr++) = gainedData[idx];
            }
        }
    }

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
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

vector<int>
avtUnstructuredDomainBoundaries::CreateDomainToProcessorMap(
                                                const vector<int> &domainNum)
{
    // Get the processor rank
    int rank = 0;
#ifdef PARALLEL
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
#endif

    // Create the map
    vector<int> domain2proc(nTotalDomains, -1);
    for (size_t d = 0; d < domainNum.size(); d++)
        domain2proc[domainNum[d]] = rank;

#ifdef PARALLEL
    vector<int> domain2proc_tmp(domain2proc);
    MPI_Allreduce(&domain2proc_tmp[0], &domain2proc[0], nTotalDomains, MPI_INT,
                  MPI_MAX, VISIT_MPI_COMM);
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
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// 
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Style updates, changed index variable names to be more descriptive.
//    Added const where possible.
//    Pass in a map of class objects instead of pointers.
//    Use STL containers where possible.
//    Rely on new helpers.
//    Error checking.
//
// ****************************************************************************

template <class T>
void
avtUnstructuredDomainBoundaries::CommunicateMeshInformation(
                                 const vector<int> &domain2proc,
                                 const vector<int> &domainNum,
                                 const vector<vtkDataSet *> &meshes,
                                 std::map<int, std::map<int, MeshDomainData<T>>> &domaindata)
{
    // Get the processor rank
    int rank = 0;
#ifdef PARALLEL
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    int tags[7];
    GetUniqueMessageTags(tags, 7);
    int mpiNPtsTag             = tags[0];
    int mpiGainedPointsTag     = tags[1];
    int mpiOriginalIdsTag      = tags[2];
    int mpiNumGivenCellsTag    = tags[3];
    int mpiCellTypesTag        = tags[4];
    int mpiNumPointsPerCellTag = tags[5];
    int mpiCellPointIdsTag     = tags[6];
#endif

    vtkIdList *idList = vtkIdList::New();

    for (int sendDom = 0; sendDom < nTotalDomains; sendDom ++)
    {
        for (int recvDom = 0; recvDom < nTotalDomains; recvDom ++)
        {
            // create references for the domain data here
            MeshDomainData<T> &currDomainData = domaindata[sendDom][recvDom];
            std::vector<std::array<T, 3>> &gainedPoints   = currDomainData.gainedPoints;
            std::vector<int>              &cellTypes      = currDomainData.cellTypes;
            std::vector<std::vector<int>> &cellPoints     = currDomainData.cellPoints;
            std::vector<int>              &origPointIds   = currDomainData.origPointIds;
            int                           &nGainedPoints  = currDomainData.nGainedPoints;
            int                           &nGainedCells   = currDomainData.nGainedCells;
            std::vector<int>              &nPointsPerCell = currDomainData.nPointsPerCell;

            // initialize data for this domain pair
            gainedPoints   = std::vector<std::array<T, 3>>();
            cellTypes      = std::vector<int>();
            cellPoints     = std::vector<std::vector<int>>();
            origPointIds   = std::vector<int>();
            nGainedPoints  = 0;
            nGainedCells   = 0;
            nPointsPerCell = std::vector<int>();

            // Cases where no computation is required.
            if (sendDom == recvDom)
                continue;
            if (domain2proc[sendDom] == -1 || domain2proc[recvDom] == -1)
                continue;

            // If this process owns both of the domains, it's an internal
            // calculation: no communication needed
            if (domain2proc[sendDom] == rank && domain2proc[recvDom] == rank)
            {
                // Find the index that corresponds to the sendDom.
                const size_t domIndex = static_cast<size_t>(GetDomIndex(domainNum, sendDom, recvDom));
                
                vtkUnstructuredGrid *givingUg = static_cast<vtkUnstructuredGrid*>(meshes[domIndex]);

                const int index = GetGivenIndex(sendDom, recvDom);

                // If no domain boundary, then there's no work to do.
                if (index < 0)
                    continue;

                size_t nPts = givenPoints[index].size();
                nGainedPoints = static_cast<int>(nPts);

                gainedPoints.resize(nPts);
                origPointIds.resize(nPts);

                T *fromPtr = static_cast<T *>(givingUg->GetPoints()->GetVoidPointer(0));
                for (size_t ptId = 0; ptId < nPts; ptId ++)
                {
                    const int origPtId = givenPoints[index][ptId];
                    origPointIds[ptId] = origPtId;

                    T *ptPtr = fromPtr + 3 * origPtId;
                    gainedPoints[ptId][0] = *(ptPtr++);
                    gainedPoints[ptId][1] = *(ptPtr++);
                    gainedPoints[ptId][2] = *(ptPtr++);
                }

                const size_t nCells = givenCells[index].size();
                nGainedCells = static_cast<int>(nCells);

                cellTypes.resize(nCells);
                cellPoints.resize(nCells);
                nPointsPerCell.resize(nCells);

                for (size_t cellId = 0; cellId < nCells; cellId ++)
                {
                    cellTypes[cellId] = givingUg->GetCellType(givenCells[index][cellId]);
                    givingUg->GetCellPoints(givenCells[index][cellId], idList);

                    const int nPtsForCell = idList->GetNumberOfIds();
                    nPointsPerCell[cellId] = static_cast<int>(nPtsForCell);

                    cellPoints[cellId].resize(nPtsForCell);
                    for (size_t ptId = 0; ptId < nPtsForCell; ptId ++)
                    {
                        cellPoints[cellId][ptId] = idList->GetId(ptId);
                    }
                }
            }
            // All other cases only occur during parallel execution.
#ifdef PARALLEL
            // If this process owns the receiving domain, we recv information.
            else if (domain2proc[recvDom] == rank)
            {
                MPI_Datatype type = GetMPIDataType<T>();
                MPI_Status stat;

                int fRank = domain2proc[sendDom];
                // Get the number of points given
                int nPts;
                MPI_Recv(&nPts, 1, MPI_INT, fRank, mpiNPtsTag,
                         VISIT_MPI_COMM, &stat);

                if (nPts == 0)
                    continue;

                nGainedPoints = nPts;
                gainedPoints.resize(nPts);
                origPointIds.resize(nPts);

                // Get the gained points
                MPI_Recv(gainedPoints.data(), nPts * 3, type,
                         fRank, mpiGainedPointsTag, VISIT_MPI_COMM, &stat);

                // Get the original ids for the gained points
                MPI_Recv(origPointIds.data(), nPts, MPI_INT,
                         fRank, mpiOriginalIdsTag, VISIT_MPI_COMM, &stat);

                // Get the number of given cells
                int nCells;
                MPI_Recv(&nCells, 1, MPI_INT, fRank,
                         mpiNumGivenCellsTag, VISIT_MPI_COMM, &stat);

                nGainedCells = nCells;

                cellTypes.resize(nCells);
                cellPoints.resize(nCells);
                nPointsPerCell.resize(nCells);

                // Get the cell types
                MPI_Recv(cellTypes.data(), nCells, MPI_INT,
                         fRank, mpiCellTypesTag, VISIT_MPI_COMM, &stat);

                // Get the number of points per cell
                MPI_Recv(nPointsPerCell.data(), nCells, MPI_INT,
                         fRank, mpiNumPointsPerCellTag, VISIT_MPI_COMM, &stat);

                // Prepare for getting the cell point ids
                int pntArrSize = 0;
                for (int cellId = 0; cellId < nCells; cellId ++)
                {
                    cellPoints[cellId].resize(nPointsPerCell[cellId]);
                    pntArrSize += nPointsPerCell[cellId];
                }

                // Get the cell point ids
                std::vector<int> pntIds(pntArrSize);
                MPI_Recv(pntIds.data(), pntArrSize, MPI_INT,
                         fRank, mpiCellPointIdsTag, VISIT_MPI_COMM, &stat);

                // Move over the point ids
                for (int cellId = 0; cellId < nCells; cellId ++)
                {
                    for (int ptId = 0; ptId < nPointsPerCell[cellId]; ptId ++)
                    {
                        const int pntIdIndex = ptId + cellId * nPointsPerCell[cellId];
                        cellPoints[cellId][ptId] = pntIds[pntIdIndex];
                    }
                }
            }
            // If this process owns the sending domain, we send information.
            else if (domain2proc[sendDom] == rank)
            {
                MPI_Datatype type = GetMPIDataType<T>();
                int tRank = domain2proc[recvDom];

                const int index = GetGivenIndex(sendDom, recvDom);

                // If no domain boundary, send 0 for nPts, and continue
                // Also continue if there are no given points.
                if (index < 0 || givenPoints[index].size() == 0)
                {
                    const int nPts = 0;
                    MPI_Send(&nPts, 1, MPI_INT, tRank, mpiNPtsTag, VISIT_MPI_COMM);
                    continue;
                }

                // Find the index that corresponds to the sendDom.
                const size_t domIndex = static_cast<size_t>(GetDomIndex(domainNum, sendDom, recvDom));

                vtkUnstructuredGrid *givingUg = (vtkUnstructuredGrid*)meshes[domIndex];

                const int nPts = static_cast<int>(givenPoints[index].size());

                // Build the point data to send
                std::vector<std::array<T, 3>> gainedPtsToSend(nPts);
                std::vector<int> origIdsToSend(nPts);
                T *fromPtr = static_cast<T *>(givingUg->GetPoints()->GetVoidPointer(0));
                for (int ptId = 0; ptId < nPts; ptId ++)
                {
                    const int origPtId = givenPoints[index][ptId];
                    origIdsToSend[ptId] = origPtId;

                    T *ptPtr = fromPtr + 3 * origPtId;
                    gainedPtsToSend[ptId][0] = *(ptPtr++);
                    gainedPtsToSend[ptId][1] = *(ptPtr++);
                    gainedPtsToSend[ptId][2] = *(ptPtr++);
                }

                // Send the number of points given
                MPI_Send(&nPts, 1, MPI_INT, tRank, mpiNPtsTag, VISIT_MPI_COMM);

                // Send the gained points
                MPI_Send(gainedPtsToSend.data(), nPts * 3, type, tRank,
                         mpiGainedPointsTag, VISIT_MPI_COMM);

                // Send the original ids for the gained points
                MPI_Send(origIdsToSend.data(), nPts, MPI_INT, tRank,
                         mpiOriginalIdsTag, VISIT_MPI_COMM);

                // Send the number of given cells
                const int nCells = static_cast<int>(givenCells[index].size());
                MPI_Send(&nCells, 1, MPI_INT, tRank, mpiNumGivenCellsTag,
                         VISIT_MPI_COMM);

                // Prepare for sending the cell info
                std::vector<int> cellDataToSend(nCells);
                std::vector<int> cellPtsToSend;
                std::vector<int> nPtsPerCellToSend(nCells);
                for (int cellId = 0; cellId < nCells; cellId ++)
                {
                    cellDataToSend[cellId] = givingUg->GetCellType(givenCells[index][cellId]);
                    givingUg->GetCellPoints(givenCells[index][cellId], idList);

                    const int nPtsForCell = idList->GetNumberOfIds();
                    nPtsPerCellToSend[cellId] = nPtsForCell;

                    for (int ptId = 0; ptId < nPtsForCell; ptId ++)
                    {
                        cellPtsToSend.push_back(idList->GetId(ptId));
                    }
                }

                // Send the cell types
                MPI_Send(cellDataToSend.data(), nCells, MPI_INT, tRank, mpiCellTypesTag,
                    VISIT_MPI_COMM);

                // Send the number of points per cell
                MPI_Send(nPtsPerCellToSend.data(), nCells, MPI_INT, tRank,
                    mpiNumPointsPerCellTag, VISIT_MPI_COMM);

                // Send the point cells
                MPI_Send(cellPtsToSend.data(), static_cast<int>(cellPtsToSend.size()), MPI_INT,
                         tRank, mpiCellPointIdsTag, VISIT_MPI_COMM);
            }
#endif
        }
    }

    idList->Delete();

#ifdef PARALLEL
    MPI_Barrier(VISIT_MPI_COMM);
#endif
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::CommunicateMixvarInformation
//
//  Purpose:
//    Send and collect information needed to exchange mixed variables.
//
//  Notes:
//    Returned arguments should be passed in as uninitialized pointers.
//    Data will be allocated as necesary, but will need to be freed by
//    the caller. Allocation always occurs two the second level (meaning
//    for example, vals[0][0] will always exist, but its value
//    may be NULL).
//
//  Arguments:
//    domain2proc   the map of domains to processors
//    domainNum     the domains that are owned by the current process
//    mats          the materials owned by the current processor
//    mixvars       the mixed variables owned by the current processor
//  (Returned arguments:)
//    mixGained     the amount of new mixed entries gained from ghost data.
//    vals          the values gained from ghost data.
//
//  Programmer:  Hank Childs
//  Creation:    March 10, 2007
//
//  Modifications:
//
//    Hank Childs, Tue Mar  4 13:40:54 PST 2008
//    Do not assume mixvar is non-NULL for all domains.
// 
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Style updates, changed index variable names to be more descriptive.
//    Added const where possible.
//    Pass in a map of class objects instead of pointers.
//    Use STL containers where possible.
//    Rely on new helpers.
//    Error checking.
//
// ****************************************************************************

void
avtUnstructuredDomainBoundaries::CommunicateMixvarInformation(
                                 const vector<int> &domain2proc,
                                 const vector<int> &domainNum,
                                 const vector<avtMaterial *> &mats,
                                 const vector<avtMixedVariable *> &mixvars,
                                 std::map<int, std::map<int, MixedVarDomainData>> &domaindata)
{
    // Get the processor rank
    int rank = 0;
#ifdef PARALLEL
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    int tags[2];
    GetUniqueMessageTags(tags, 2);
    int mpiNDataTag      = tags[0];
    int mpiGainedValsTag = tags[1];
#endif

    for (int sendDom = 0; sendDom < nTotalDomains; sendDom ++)
    {
        for (int recvDom = 0; recvDom < nTotalDomains; recvDom ++)
        {
            // create references for the domain data here
            MixedVarDomainData &currDomainData = domaindata[sendDom][recvDom];
            int                &nGainedMixlen = currDomainData.nGainedMixlen;
            std::vector<float> &vals          = currDomainData.vals;

            // initialize data for this domain pair
            nGainedMixlen = 0;
            vals          = std::vector<float>();

            // Cases where no computation is required.
            if (sendDom == recvDom)
                continue;
            if (domain2proc[sendDom] == -1 || domain2proc[recvDom] == -1)
                continue;

            // If this process owns both of the domains, it's an internal
            // calculation: no communication needed
            if (domain2proc[sendDom] == rank && domain2proc[recvDom] == rank)
            {
                // Find the index that corresponds to the sendDom.
                const size_t domIndex = static_cast<size_t>(GetDomIndex(domainNum, sendDom, recvDom));

                const int index = GetGivenIndex(sendDom, recvDom);

                // If no domain boundary, then there's no work to do.
                if (index < 0)
                    continue;

                avtMixedVariable *givingVar = mixvars[domIndex];

                const size_t nCells = givenCells[index].size();

                const int *mix_next = mats[domIndex]->GetMixNext();
                const int *matlist  = mats[domIndex]->GetMatlist();
                const int nMixlen = GetNMixLen(nCells, index, matlist, mix_next, sendDom);

                // Now that we have assessed the size, we can allocate memory
                // and populate the buffer.
                nGainedMixlen = nMixlen;
                vals.resize(nMixlen);
                const float *buff = (nullptr != givingVar ? givingVar->GetBuffer() : nullptr);
                
                // we have to protect against reading from buff if it is null
                // we will only attempt to read from it if there is a mixed
                // cell, which is true if nMixlen > 0.
                if (nullptr == buff && nMixlen > 0)
                {
                    std::string err_msg = "avtUnstructuredDomainBoundaries::CommunicateMixvarInformation "
                                          "failed to communicate for sendDom " + std::to_string(sendDom) +
                                          " and recvDom " + std::to_string(recvDom);
                    EXCEPTION1(VisItException, err_msg);
                }
                int nMixlenCounter = 0;
                for (int cellId = 0; cellId < nCells; cellId ++)
                {
                    const int cell = givenCells[index][cellId];
                    if (matlist[cell] < 0)
                    {
                        int current = -matlist[cell] - 1;
                        do
                        {
                            // we assume nMixlenCounter < nMixlen
                            // it is calculated the same way as nMixlen
                            vals[nMixlenCounter++] = buff[current];
                            current = mix_next[current] - 1;
                        } while (mix_next[current] != 0);
                    }
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
                int amt = 0;
                MPI_Recv(&amt, 1, MPI_INT, fRank, mpiNDataTag,
                         VISIT_MPI_COMM, &stat);

                if (amt == 0)
                    continue;

                nGainedMixlen = amt;
                vals.resize(amt);
                // Get the gained materials
                MPI_Recv(vals.data(), amt, MPI_FLOAT,
                         fRank, mpiGainedValsTag, VISIT_MPI_COMM, &stat);
            }
            // If this process owns the sending domain, we send information.
            else if (domain2proc[sendDom] == rank)
            {
                int tRank = domain2proc[recvDom];

                const int index = GetGivenIndex(sendDom, recvDom);

                // If no domain boundary, send 0 for nPts, and continue
                // Also continue if there are no given points.
                int amt = 0;
                if (index < 0 || givenPoints[index].size() == 0)
                {
                    MPI_Send(&amt, 1, MPI_INT,tRank,mpiNDataTag,VISIT_MPI_COMM);
                    continue;
                }
                
                // Find the index that corresponds to the sendDom.
                const size_t domIndex = static_cast<size_t>(GetDomIndex(domainNum, sendDom, recvDom));

                avtMixedVariable *givingVar = mixvars[domIndex];

                const size_t nCells = givenCells[index].size();

                const int *mix_next = mats[domIndex]->GetMixNext();
                const int *matlist  = mats[domIndex]->GetMatlist();
                const int nMixlen = GetNMixLen(nCells, index, matlist, mix_next, sendDom);

                MPI_Send(&nMixlen, 1,MPI_INT,tRank,mpiNDataTag,VISIT_MPI_COMM);

                // Now that we have assessed the size, we can allocate memory
                // and populate the buffer.
                nGainedMixlen = nMixlen;
                std::vector<float> sendBuff(nMixlen);
                const float *buff = (nullptr != givingVar ? givingVar->GetBuffer() : nullptr);
                
                // we have to protect against reading from buff if it is null
                // we will only attempt to read from it if there is a mixed
                // cell, which is true if nMixlen > 0.
                if (nullptr == buff && nMixlen > 0)
                {
                    std::string err_msg = "avtUnstructuredDomainBoundaries::CommunicateMixvarInformation "
                                          "failed to communicate for sendDom " + std::to_string(sendDom) +
                                          " and recvDom " + std::to_string(recvDom);
                    EXCEPTION1(VisItException, err_msg);
                }
                int nMixlenCounter = 0;
                for (int cellId = 0; cellId < nCells; cellId ++)
                {
                    const int cell = givenCells[index][cellId];
                    if (matlist[cell] < 0)
                    {
                        int current = -matlist[cell] - 1;
                        do
                        {
                            // we assume nMixlenCounter < nMixlen
                            // it is calculated the same way as nMixlen
                            sendBuff[nMixlenCounter++] = buff[current];
                            current = mix_next[current] - 1;
                        } while (mix_next[current] != 0);
                    }
                }
                // Send the matlist
                MPI_Send(sendBuff.data(), nMixlen, MPI_FLOAT,
                         tRank, mpiGainedValsTag, VISIT_MPI_COMM);
            }
#endif
        }
    }

#ifdef PARALLEL
    MPI_Barrier(VISIT_MPI_COMM);
#endif
}


// ****************************************************************************
//  Method:  avtUnstructuredDomainBoundaries::CommunicateMaterialInformation
//
//  Purpose:
//    Send and collect information needed to exchange materials. Also
//    generates information from domains handled by this process.
//
//  Notes:
//    Returned arguments should be passed in as uninitialized pointers.
//    Data will be allocated as necesary, but will need to be freed by
//    the caller. Allocation always occurs two the second level (meaning
//    for example, nGainedCells[0][0] will always exist, but its value
//    may be NULL).
//
//    mix_next and mix_zone are *not* communicated, because mix_next must be
//    rebuilt in the context of the original material and mix_zone is no longer
//    applicable.
//
//  Arguments:
//    domain2proc   the map of domains to processors
//    domainNum     the domains that are owned by the current process
//    mats          the materials owned by the current processor
//  (Returned arguments:)
//    nGainedCells  the number of cells gained from other processors.
//    nGainedMixlen the amount of mixed entries gained from other processors.
//    gainedMatlist the matlist from other processors.  mixed cells are
//                  negative and represent the number of mixed materials
//                  in the cell.
//    gainedMixmat  the mixmat from other processors.
//    gainedMixvf   the mixvf from other processors.
//
//  Programmer:  Hank Childs
//  Creation:    February 13, 2007
//
//  Modifications:
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Style updates, changed index variable names to be more descriptive.
//    Added const where possible.
//    Pass in a map of class objects instead of pointers.
//    Use STL containers where possible.
//    Rely on new helpers.
//
// ****************************************************************************

void
avtUnstructuredDomainBoundaries::CommunicateMaterialInformation(
                                 const vector<int> &domain2proc,
                                 const vector<int> &domainNum,
                                 const vector<avtMaterial *> &mats,
                                 std::map<int, std::map<int, MixedMaterialDomainData>> &domaindata)
{
    // Get the processor rank
    int rank = 0;
#ifdef PARALLEL
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    int tags[4];
    GetUniqueMessageTags(tags, 4);
    int mpiNDataTag         = tags[0];
    int mpiGainedMatlistTag = tags[1];
    int mpiGainedMixmatTag  = tags[2];
    int mpiGainedMixvfTag   = tags[3];
#endif

    for (int sendDom = 0; sendDom < nTotalDomains; sendDom ++)
    {
        for (int recvDom = 0; recvDom < nTotalDomains; recvDom ++)
        {
            // create references for the domain data here
            MixedMaterialDomainData &currDomainData = domaindata[sendDom][recvDom];
            int                &nGainedCells  = currDomainData.nGainedCells;
            int                &nGainedMixlen = currDomainData.nGainedMixlen;
            std::vector<int>   &gainedMatlist = currDomainData.gainedMatlist;
            std::vector<int>   &gainedMixmat  = currDomainData.gainedMixmat;
            std::vector<float> &gainedMixvf   = currDomainData.gainedMixvf;

            // initialize data for this domain pair
            nGainedCells  = 0;
            nGainedMixlen = 0;
            gainedMatlist = std::vector<int>();
            gainedMixmat  = std::vector<int>();
            gainedMixvf   = std::vector<float>();

            // Cases where no computation is required.
            if (sendDom == recvDom)
                continue;
            if (domain2proc[sendDom] == -1 || domain2proc[recvDom] == -1)
                continue;

            // If this process owns both of the domains, it's an internal
            // calculation: no communication needed
            if (domain2proc[sendDom] == rank && domain2proc[recvDom] == rank)
            {
                // Find the index that corresponds to the sendDom.
                const size_t domIndex = static_cast<size_t>(GetDomIndex(domainNum, sendDom, recvDom));

                const int index = GetGivenIndex(sendDom, recvDom);

                // If no domain boundary, then there's no work to do.
                if (index < 0)
                    continue;

                avtMaterial *givingMat = mats[domIndex];

                const size_t nCells = givenCells[index].size();
                nGainedCells = static_cast<int>(nCells);

                const int *mix_next = mats[domIndex]->GetMixNext();
                const int *matlist  = mats[domIndex]->GetMatlist();
                const int nMixlen = GetNMixLen(nCells, index, matlist, mix_next, sendDom);
                nGainedMixlen = nMixlen;

                gainedMatlist.resize(nCells);
                gainedMixmat.resize(nMixlen);
                gainedMixvf.resize(nMixlen);

                const int   *mix_mat  = givingMat->GetMixMat();
                const float *mix_vf   = givingMat->GetMixVF();

                TransferMatInfo(nCells, index, matlist, mix_mat, mix_vf, mix_next,
                                gainedMatlist, gainedMixmat, gainedMixvf);
            }

            // All other cases only occur during parallel execution.
#ifdef PARALLEL
            // If this process owns the receiving domain, we recv information.
            else if (domain2proc[recvDom] == rank)
            {
                MPI_Status stat;

                int fRank = domain2proc[sendDom];
                // Get the number of points given
                int amt[2];
                MPI_Recv(amt, 2, MPI_INT, fRank, mpiNDataTag,
                         VISIT_MPI_COMM, &stat);

                if (amt[0] == 0 && amt[1] == 0)
                    continue;

                nGainedCells  = amt[0];
                nGainedMixlen = amt[1];

                gainedMatlist.resize(nGainedCells);
                gainedMixmat.resize(nGainedMixlen);
                gainedMixvf.resize(nGainedMixlen);

                // Get the gained materials
                MPI_Recv(gainedMatlist.data(), nGainedCells, MPI_INT,
                         fRank, mpiGainedMatlistTag, VISIT_MPI_COMM, &stat);

                // Get the gained mixmats
                MPI_Recv(gainedMixmat.data(), nGainedMixlen, MPI_INT,
                         fRank, mpiGainedMixmatTag, VISIT_MPI_COMM, &stat);

                // Get the gained mixvf
                MPI_Recv(gainedMixvf.data(), nGainedMixlen, MPI_FLOAT,
                         fRank, mpiGainedMixvfTag, VISIT_MPI_COMM, &stat);
            }
            // If this process owns the sending domain, we send information.
            else if (domain2proc[sendDom] == rank)
            {
                int tRank = domain2proc[recvDom];

                const int index = GetGivenIndex(sendDom, recvDom);

                // If no domain boundary, send 0 for nPts, and continue
                // Also continue if there are no given points.
                int amt[2] = { 0, 0 };
                if (index < 0 || givenPoints[index].size() == 0)
                {
                    MPI_Send(amt, 2, MPI_INT,tRank,mpiNDataTag,VISIT_MPI_COMM);
                    continue;
                }

                // Find the index that corresponds to the sendDom.
                const size_t domIndex = static_cast<size_t>(GetDomIndex(domainNum, sendDom, recvDom));

                avtMaterial *givingMat = mats[domIndex];

                const int nCells = static_cast<int>(givenCells[index].size());
                nGainedCells = nCells;

                const int *mix_next = mats[domIndex]->GetMixNext();
                const int *matlist  = mats[domIndex]->GetMatlist();
                const int nMixlen = GetNMixLen(nCells, index, matlist, mix_next, sendDom);
                nGainedMixlen = nMixlen;

                amt[0] = nCells;
                amt[1] = nMixlen;
                MPI_Send(amt, 2, MPI_INT,tRank,mpiNDataTag,VISIT_MPI_COMM);

                std::vector<int>   givenMatlist = std::vector<int>(nCells);
                std::vector<int>   givenMixmat  = std::vector<int>(nMixlen);
                std::vector<float> givenMixvf   = std::vector<float>(nMixlen);

                const int   *mix_mat  = givingMat->GetMixMat();
                const float *mix_vf   = givingMat->GetMixVF();

                TransferMatInfo(nCells, index, matlist, mix_mat, mix_vf, mix_next,
                                givenMatlist, givenMixmat, givenMixvf);

                // Send the matlist
                MPI_Send(givenMatlist.data(), nCells, MPI_INT,
                         tRank, mpiGainedMatlistTag, VISIT_MPI_COMM);
                // Send the mixmats
                MPI_Send(givenMixmat.data(), nMixlen, MPI_INT,
                         tRank, mpiGainedMixmatTag, VISIT_MPI_COMM);
                // Send the mixmats
                MPI_Send(givenMixvf.data(), nMixlen, MPI_FLOAT,
                         tRank, mpiGainedMixvfTag, VISIT_MPI_COMM);
            }
#endif
        }
    }

#ifdef PARALLEL
    MPI_Barrier(VISIT_MPI_COMM);
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
//  Modifications:
//    Brad Whitlock, Thu Sep 16 13:03:48 PST 2004
//    I wrapped this method with an ifdef so it is not built using MSVC6.0
//    on Windows.
//
//    Brad Whitlock, Mon Nov 7 09:25:57 PDT 2005
//    I made it use the correct return type for GetMPIDataType() so that it
//    works with other MPI implementations.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Eric Brugger, Fri Mar 13 15:20:08 PDT 2020
//    Modify to handle NULL meshes.
//
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Style updates, changed index variable names to be more descriptive.
//    Added const where possible.
//    Pass in a map of class objects instead of pointers.
//    Use STL containers where possible.
///   Rely on new helpers.
//
// ****************************************************************************
template <class T>
void
avtUnstructuredDomainBoundaries::CommunicateDataInformation(
                                 const vector<int> &domain2proc,
                                 const vector<int> &domainNum,
                                 const vector<vtkDataArray *> &data,
                                 const bool isPointData,
                                 std::map<int, std::map<int, VarDomainData<T>>> &domaindata)
{
    // Get the processor rank
    int rank = 0;
#ifdef PARALLEL
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    int tags[2];
    GetUniqueMessageTags(tags, 2);
    int mpiNumTuplesTag = tags[0];
    int mpiTupleDataTag = tags[1];
#endif

    int nComponents = 0;
    int nonNullDomain = 0;
    while (nonNullDomain < data.size() && nullptr == data[nonNullDomain])
    {
        nonNullDomain ++;
    }
    if (nonNullDomain < data.size())
    {
        nComponents = data[nonNullDomain]->GetNumberOfComponents();
    }

    for (int sendDom = 0; sendDom < nTotalDomains; ++sendDom)
    {
        for (int recvDom = 0; recvDom < nTotalDomains; ++recvDom)
        {
            // create references for the domain data here
            VarDomainData<T> &currDomainData = domaindata[sendDom][recvDom];
            std::vector<T> &gainedData    = currDomainData.gainedData;
            int            &nGainedTuples = currDomainData.nGainedTuples;

            // initialize data for this domain pair
            gainedData    = std::vector<T>();
            nGainedTuples = 0;

            // Cases where no computation is required.
            if (sendDom == recvDom)
                continue;
            if (domain2proc[sendDom] == -1 || domain2proc[recvDom] == -1)
                continue;

            // If this process owns both of the domains, it's an internal
            // calculation: no communication needed
            if (domain2proc[sendDom] == rank && domain2proc[recvDom] == rank)
            {
                // Find the index that corresponds to the sendDom.
                const size_t domIndex = static_cast<size_t>(GetDomIndex(domainNum, sendDom, recvDom));

                const int index = GetGivenIndex(sendDom, recvDom);

                // If no domain boundary, then there's no work to do.
                if (index < 0)
                    continue;

                vector<int> &mapRef = isPointData ? givenPoints[index]
                                                  : givenCells[index];

                const int nTuples = static_cast<int>(mapRef.size());
                nGainedTuples = nTuples;

                gainedData.resize(nTuples * nComponents);

                T *origPtr = static_cast<T *>(data[domIndex]->GetVoidPointer(0));

                for (int tupleId = 0; tupleId < nTuples; ++tupleId)
                {
                    T *ptr = origPtr + mapRef[tupleId] * nComponents;
                    for (int compId = 0; compId < nComponents; ++compId)
                    {
                        gainedData[compId + nComponents * tupleId] = *(ptr++);
                    }
                }
            }
            // All other cases only occur during parallel execution.
#ifdef PARALLEL
            // If this process owns the receving domain, we recv information.
            else if (domain2proc[recvDom] == rank)
            {
                MPI_Datatype type = GetMPIDataType<T>();
                MPI_Status stat;

                int fRank = domain2proc[sendDom];
                // Get the number of incoming tuples
                int nTup;
                MPI_Recv(&nTup, 1, MPI_INT, fRank, mpiNumTuplesTag,
                         VISIT_MPI_COMM, &stat);

                if (nTup == 0)
                    continue;

                nGainedTuples = nTup;
                gainedData.resize(nTup * nComponents);

                // Get the data
                MPI_Recv(gainedData.data(), nTup * nComponents,
                         type, fRank, mpiTupleDataTag, VISIT_MPI_COMM, &stat);
            }
            // If this process owns the sending domain, we send information.
            else if (domain2proc[sendDom] == rank)
            {
                MPI_Datatype type = GetMPIDataType<T>();
                int tRank = domain2proc[recvDom];

                const int index = GetGivenIndex(sendDom, recvDom);

                // If no domain boundary, send 0 for nTuples and continue
                if (index < 0)
                {
                    int nTup = 0;
                    MPI_Send(&nTup, 1, MPI_INT, tRank, mpiNumTuplesTag,
                        VISIT_MPI_COMM);
                    continue;
                }

                // Send the number of tuples
                vector<int> &mapRef = isPointData ? givenPoints[index]
                                                  : givenCells[index];
                const int nTup = static_cast<int>(mapRef.size());
                MPI_Send(&nTup, 1, MPI_INT, tRank, mpiNumTuplesTag,
                    VISIT_MPI_COMM);

                // If there are no tuples to give, continue
                if (nTup == 0)
                    continue;

                // Gather the data for sending

                // Find the index that corresponds to the sendDom.
                const size_t domIndex = static_cast<size_t>(GetDomIndex(domainNum, sendDom, recvDom));

                std::vector<T> dataArr(nTup * nComponents);
                T *origPtr = static_cast<T *>(data[domIndex]->GetVoidPointer(0));

                for (int tupleId = 0; tupleId < nTup; tupleId ++)
                {
                    T *ptr = origPtr + mapRef[tupleId] * nComponents;
                    for (int compId = 0; compId < nComponents; compId ++)
                    {
                        dataArr[compId + nComponents * tupleId] = *(ptr++);
                    }
                }

                // Send the data
                MPI_Send(dataArr.data(), nTup * nComponents, type, tRank,
                    mpiTupleDataTag, VISIT_MPI_COMM);
            }
#endif
        }
    }

#ifdef PARALLEL
    MPI_Barrier(VISIT_MPI_COMM);
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
//    Hank Childs, Sun Feb 27 14:47:45 PST 2005
//    Added argument allDomains.
// 
//    Justin Privitera, Wed Apr 23 17:39:24 PDT 2025
//    Style updates, changed index variable names to be more descriptive.
//    Added const where possible.
//    Used modern C++ range-based for and auto to simplify map iteration.
//
// ****************************************************************************

void
avtUnstructuredDomainBoundaries::CreateGhostNodes(vector<int> domainNum,
                                                  vector<vtkDataSet*> meshes,
                                                  vector<int> &allDomains)
{
    for (size_t domIdIndex = 0; domIdIndex < domainNum.size(); domIdIndex ++)
    {
        vtkDataSet *ds = meshes[domIdIndex];
        const int npts = ds->GetNumberOfPoints();

        vtkUnsignedCharArray *gn = vtkUnsignedCharArray::New();
        gn->SetNumberOfTuples(npts);
        gn->SetName("avtGhostNodes");
        unsigned char *gnp = gn->GetPointer(0);
        for (int ptId = 0; ptId < npts; ptId ++)
        {
            gnp[ptId] = 0;
        }

        for (size_t idx = 0; idx < giveIndex.size(); idx ++)
        {
            if (giveIndex[idx].first != domainNum[domIdIndex])
                continue;
            auto& thisMap = sharedPointsMap[giveIndex[idx].first];
            for (const auto& pair : thisMap)
            {
                const int node = pair.first;
                avtGhostData::AddGhostNodeType(gnp[node], DUPLICATED_NODE);
            }
        }

        ds->GetPointData()->AddArray(gn);
        gn->Delete();
    }
}


