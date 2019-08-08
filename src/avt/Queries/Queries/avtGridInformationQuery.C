/*****************************************************************************
*
* Copyright (c) 2013, Intelligent Light
*
*****************************************************************************/
#include <avtGridInformationQuery.h>

#include <MapNode.h>

#include <vtkCellData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedCharArray.h>

#include <avtParallel.h>

#include <cstdlib>

const int avtGridInformationQuery::RIGHT_HANDED = 0;
const int avtGridInformationQuery::LEFT_HANDED = 1;

// ****************************************************************************
// Method: avtGridInformationQuery::avtGridInformationQuery
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep  6 16:09:07 PDT 2013
//
// Modifications:
//
// ****************************************************************************

avtGridInformationQuery::avtGridInformationQuery() : avtDatasetQuery(), 
    getExtents(false), getHandedness(false), getGhostTypes(false), gridInfo()
{
}

// ****************************************************************************
// Method: avtGridInformationQuery::~avtGridInformationQuery
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep  6 16:09:07 PDT 2013
//
// Modifications:
//
// ****************************************************************************
avtGridInformationQuery::~avtGridInformationQuery()
{
}

// ****************************************************************************
// Method: avtGridInformationQuery::PreExecute
//
// Purpose: 
//   Clear an internal variable.
//
// Arguments:
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep  6 16:09:07 PDT 2013
//
// Modifications:
//
// ****************************************************************************

void
avtGridInformationQuery::PreExecute(void)
{
    gridInfo.clear();
}

// ****************************************************************************
// Method: avtGridInformationQuery::Execute
//
// Purpose: 
//   Execute the query on this domain's dataset.
//
// Arguments:
//   ds     : The VTK dataset to process.
//   domain : The domain number of this dataset.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep  6 16:09:07 PDT 2013
//
// Modifications:
//   Brad Whitlock, Tue Jun  9 17:06:10 PDT 2015
//   Added ghosttypes.
// 
// ****************************************************************************

void
avtGridInformationQuery::Execute(vtkDataSet *ds, const int domain)
{
    GridInfo info; 
    info.handedness = RIGHT_HANDED;
    info.ghosttypes = 0;

    if(ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::SafeDownCast(ds);
        rgrid->GetDimensions(info.dims);
        info.meshType = AVT_RECTILINEAR_MESH;
    }
    else if(ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        vtkStructuredGrid *sgrid = vtkStructuredGrid::SafeDownCast(ds);
        sgrid->GetDimensions(info.dims);
        info.meshType = AVT_CURVILINEAR_MESH;
    }
    else if(ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(ds);
        info.dims[0] = ugrid->GetNumberOfPoints();
        info.dims[1] = ugrid->GetNumberOfCells();
        info.dims[2] = 0;
        info.meshType = AVT_UNSTRUCTURED_MESH;

        // TODO: an additional calculation to determine grid "handedness".
        // grid.handedness = ??;
    }
    else if(ds->GetDataObjectType() == VTK_POLY_DATA)
    {
        vtkPolyData *pd = vtkPolyData::SafeDownCast(ds);
        info.dims[0] = pd->GetNumberOfPoints();
        info.dims[1] = pd->GetNumberOfCells();
        info.dims[2] = 0;
        info.meshType = AVT_UNSTRUCTURED_MESH;
    }
    else
    {
        info.dims[0] = info.dims[1] = info.dims[2] = 0;
        info.meshType = AVT_UNKNOWN_MESH;
    }

    if(getGhostTypes)
    {
        vtkUnsignedCharArray *ghostZones = 
           (vtkUnsignedCharArray*)ds->GetCellData()->GetArray("avtGhostZones");
        int ghost = 0;
        if(ghostZones != NULL)
        {
            for(vtkIdType i = 0; i < ghostZones->GetNumberOfTuples(); i++)
            {
                int gcell = (int)ghostZones->GetValue(i);
                ghost |= gcell;
            }
        }
        info.ghosttypes = ghost;
    }

    if(getExtents)
        ds->GetBounds(info.extents); // does not take into account rectxform.

    gridInfo[domain] = info;
}

// ****************************************************************************
// Function: sort_grid_record
//
// Purpose: 
//   qsort callback for grid records.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep  6 16:09:07 PDT 2013
//
// Modifications:
//
// ****************************************************************************

static int
sort_grid_record(const void *a, const void *b)
{
    int *A = (int *)a;
    int *B = (int *)b;
    if(A[0] < B[0])
        return -1;
    else if(A[0] == B[0])
        return 0;
    return 1;
}

// ****************************************************************************
// Function: sort_extents_record
//
// Purpose: 
//   qsort callback for extents records.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep  6 16:09:07 PDT 2013
//
// Modifications:
//
// ****************************************************************************

static int
sort_extents_record(const void *a, const void *b)
{
    double *A = (double *)a;
    double *B = (double *)b;
    if(A[0] < B[0])
        return -1;
    else if(A[0] == B[0])
        return 0;
    return 1;
}

// ****************************************************************************
// Method: avtGridInformationQuery::PostExecute
//
// Purpose: 
//   Communicate all processors' grid information to rank 0 and fill in the
//   different query outputs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep  6 16:09:07 PDT 2013
//
// Modifications:
//
// ****************************************************************************

void
avtGridInformationQuery::PostExecute(void)
{
    // Pack the map into an array that we can send to the rank 0 processor.
    int ndoms = (int)gridInfo.size();
    const int recordSize = 7;
    int *sendBuf = new int[(ndoms+1) * recordSize];
    int *iptr = sendBuf;
    for(std::map<int,GridInfo>::const_iterator pos = gridInfo.begin(); pos != gridInfo.end(); ++pos)
    {
        iptr[0] = pos->first;
        iptr[1] = (int)pos->second.meshType;
        iptr[2] = pos->second.dims[0];
        iptr[3] = pos->second.dims[1];
        iptr[4] = pos->second.dims[2];
        iptr[5] = pos->second.handedness;
        iptr[6] = pos->second.ghosttypes;

        iptr += recordSize;
    }

    // Get the map on rank 0
    int *receiveBuf = NULL, *receiveCounts = NULL;
    CollectIntArraysOnRootProc(receiveBuf, receiveCounts, sendBuf, ndoms * recordSize);
    delete [] sendBuf;

    // Get the extents.
    double *receiveExtents = NULL;
    const int drecordSize = 7;
    if(getExtents)
    {
        double *dsend = new double[(ndoms+1)*drecordSize];
        double *dptr = dsend;
        for(std::map<int,GridInfo>::const_iterator pos = gridInfo.begin(); pos != gridInfo.end(); ++pos)
        {
            dptr[0] = pos->first;
            dptr[1] = pos->second.extents[0];
            dptr[2] = pos->second.extents[1];
            dptr[3] = pos->second.extents[2];
            dptr[4] = pos->second.extents[3];
            dptr[5] = pos->second.extents[4];
            dptr[6] = pos->second.extents[5];
            dptr += drecordSize;
        }

        int *receiveExtentsCounts = NULL;
        CollectDoubleArraysOnRootProc(receiveExtents, receiveExtentsCounts, dsend, ndoms * drecordSize);
        delete [] dsend;
        delete [] receiveExtentsCounts;
    }

    if(PAR_Rank() == 0)
    {
        // Get the number of records.
        int nrec = 0;
        for(int i = 0; i < PAR_Size(); ++i)
            nrec += receiveCounts[i];
        nrec /= recordSize;

        // Need to sort records just in case domains are assigned randomly.
        qsort(receiveBuf, nrec, sizeof(int)*recordSize, sort_grid_record);

        // Need to sort extents records just in case domains are assigned randomly.
        if(getExtents)
            qsort(receiveExtents, nrec, sizeof(double)*drecordSize, sort_extents_record);

        // Iterate through the records, which will be packed one after the next in
        // the receiveBuf array. Each record is "recordSize" ints.
        int *rec = receiveBuf;
        double *ext = receiveExtents;
        MapNode resultNode;
        std::string resultMsg;
        doubleVector resultValues;
        for(int i = 0; i < nrec; ++i, rec += recordSize)
        {
            // Build up the XML result.
            MapNode domain_result;
            domain_result["meshType"] = rec[1];
            intVector ijk;
            ijk.push_back(rec[2]);
            ijk.push_back(rec[3]);
            ijk.push_back(rec[4]);
            domain_result["dimensions"] = ijk;
            if(getExtents)
            {
                doubleVector e;
                for(int j = 0; j < 6; ++j)
                    e.push_back(ext[j+1]);
                domain_result["extents"] = e;
            }
            if(getHandedness)
            {
                domain_result["handedness"] = rec[5];
            }
            if(getGhostTypes)
            {
                domain_result["ghosttypes"] = rec[6];
            }
            char key[20];
            snprintf(key, 20, "%d", rec[0]);
            resultNode[key] = domain_result;

            // Build up the result values.
            resultValues.push_back(rec[0]);
            resultValues.push_back(rec[1]);
            resultValues.push_back(rec[2]);
            resultValues.push_back(rec[3]);
            resultValues.push_back(rec[4]);
            if(getHandedness)
            {
                resultValues.push_back(rec[5]);
            }
            if(getGhostTypes)
            {
                resultValues.push_back(rec[6]);
            }

            // Build up the result message.
            const char *mt = "AVT_UNKNOWN_MESH";
            if(rec[1] == (int)AVT_RECTILINEAR_MESH)
                mt = "AVT_RECTILINEAR_MESH";
            else if(rec[1] == (int)AVT_CURVILINEAR_MESH)
                mt = "AVT_CURVILINEAR_MESH";
            else if(rec[1] == (int)AVT_UNSTRUCTURED_MESH)
                mt = "AVT_UNSTRUCTURED_MESH";

            char handed[30];
            if(getHandedness)
            {
                if(rec[5] == LEFT_HANDED)
                    strcpy(handed, ", handedness=LEFT_HANDED");
                else
                    strcpy(handed, ", handedness=RIGHT_HANDED");
            }
            else
                handed[0] = '\0';

            char gt[300];
            if(getGhostTypes)
            {
                std::string g;
                unsigned char gz = (unsigned char)rec[6];
                int count = 0;
                if(avtGhostData::IsGhostZoneType(gz, DUPLICATED_ZONE_INTERNAL_TO_PROBLEM))
                {
                    g.append("DUPLICATED_ZONE_INTERNAL_TO_PROBLEM");
                    ++count;
                }
                if(avtGhostData::IsGhostZoneType(gz, ENHANCED_CONNECTIVITY_ZONE))
                {
                    if(count > 0) g.append(", ");
                    g.append("ENHANCED_CONNECTIVITY_ZONE");
                    ++count;
                }
                if(avtGhostData::IsGhostZoneType(gz, REDUCED_CONNECTIVITY_ZONE))
                {
                    if(count > 0) g.append(", ");
                    g.append("REDUCED_CONNECTIVITY_ZONE");
                    ++count;
                }
                if(avtGhostData::IsGhostZoneType(gz, REFINED_ZONE_IN_AMR_GRID))
                {
                    if(count > 0) g.append(", ");
                    g.append("REFINED_ZONE_IN_AMR_GRID");
                    ++count;
                }
                if(avtGhostData::IsGhostZoneType(gz, ZONE_EXTERIOR_TO_PROBLEM))
                {
                    if(count > 0) g.append(", ");
                    g.append("ZONE_EXTERIOR_TO_PROBLEM");
                    ++count;
                }
                if(avtGhostData::IsGhostZoneType(gz, ZONE_NOT_APPLICABLE_TO_PROBLEM))
                {
                    if(count > 0) g.append(", ");
                    g.append("ZONE_NOT_APPLICABLE_TO_PROBLEM");
                    ++count;
                }

                snprintf(gt, 300, ", ghosttypes=%d={%s}", rec[6], g.c_str());
            }
            else
            {
                gt[0] = '\0';
            }

            char msg[1024];
            if(getExtents)
            {
                // Add the extents onto the result values.
                for(int j = 0; j < 6; ++j)
                    resultValues.push_back(ext[j+1]);

                snprintf(msg, 1024, "Grid %d: type=%s, dims={%d,%d,%d}%s, extents={%lg,%lg,%lg,%lg,%lg,%lg}%s\n", 
                         rec[0], mt, rec[2], rec[3], rec[4], handed,
                         ext[1], ext[2], ext[3], ext[4], ext[5], ext[6], gt);
                resultMsg.append(msg);

                ext += 7;
            }
            else
            {
                snprintf(msg, 1024, "Grid %d: type=%s, dims={%d,%d,%d}%s%s\n", 
                         rec[0], mt, rec[2], rec[3], rec[4], handed, gt);
                resultMsg.append(msg);
            }
        }

        SetResultMessage(resultMsg);
        SetResultValues(resultValues);
        SetXmlResult(resultNode.ToXML());
    }

    delete [] receiveBuf;
    delete [] receiveCounts;
    delete [] receiveExtents;
}

// ****************************************************************************
// Method: avtGridInformationQuery::SetInputParams
//
// Purpose: 
//   Set the input parameters.
//
// Arguments:
//   params : A MapNode containing the parameters.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep  6 16:09:07 PDT 2013
//
// Modifications:
//
// ****************************************************************************

void 
avtGridInformationQuery::SetInputParams(const MapNode &params)
{
    if (params.HasEntry("get_extents"))
    {
        getExtents = params.GetEntry("get_extents")->ToBool();
    }

    if (params.HasEntry("get_handedness"))
    {
        getHandedness = params.GetEntry("get_handedness")->ToBool();
    }

    if (params.HasEntry("get_ghosttypes"))
    {
        getGhostTypes = params.GetEntry("get_ghosttypes")->ToBool();
    }
}

// ****************************************************************************
// Method: avtGridInformationQuery::GetDefaultInputParams
//
// Purpose: 
//   Populate a MapNode with the default input parameters for this query.
//
// Arguments:
//  params : The MapNode to fill in.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep  6 16:09:07 PDT 2013
//
// Modifications:
//
// ****************************************************************************

void
avtGridInformationQuery::GetDefaultInputParams(MapNode &params)
{
    params["get_extents"] = false;
    params["get_handedness"] = false;
    params["get_ghosttypes"] = false;

    params["use_actual_data"] = 0; // use original data by default
}
