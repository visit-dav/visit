// ************************************************************************* //
//                              avtDataValidity.C                            //
// ************************************************************************* //

#include <avtDataValidity.h>

#include <avtDataObjectString.h>
#include <avtDataObjectWriter.h>


// ****************************************************************************
//  Method: avtDataValidity constructor
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep 30 16:33:37 PDT 2003
//    Moved all real work into "Reset".  Blew away all previous comments since
//    they were no longer meaningful.
//
// ****************************************************************************

avtDataValidity::avtDataValidity()
{
    Reset();
}


// ****************************************************************************
//  Method: avtDataValidity destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDataValidity::~avtDataValidity()
{
    ;
}


// ****************************************************************************
//  Method: avtDataValidity::Reset
//
//  Purpose:
//      Resets the data validity object to a state where everything is valid.
//
//  Programmer: Hank Childs
//  Creation:   September 30, 2003
//
//  Modifications:
//
//    Mark C. Miller, Thu Jan 29 16:40:25 PST 2004
//    Added hasEverOwnedAnyDomain
//
//    Kathleen Bonnell, Thu Mar  2 14:04:06 PST 2006
//    Added originalZonesIntact.
//
// ****************************************************************************

void
avtDataValidity::Reset(void)
{
    zonesPreserved                    = true;
    originalZonesIntact               = true;
    dataMetaDataPreserved             = true;
    spatialMetaDataPreserved          = true;
    operationFailed                   = false;
    usingAllData                      = false;
    usingAllDomains                   = false;
    isThisDynamic                     = false;
    pointsWereTransformed             = false;
    wireframeRenderingIsInappropriate = false;
    normalsAreInappropriate           = false;
    subdivisionOccurred               = false;
    notAllCellsSubdivided             = false;
    disjointElements                  = false;
    queryable                         = true;
    hasEverOwnedAnyDomain             = true;
    errorOccurred                     = false;
    errorString                       = "";
}


// ****************************************************************************
//  Method: avtDataValidity::Copy
//
//  Purpose:
//      Copies the data validity information from the argument to this object.
//
//  Arguments:
//      di      Data validity information from another object.
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Sep 14 09:38:58 PDT 2001
//    Copy over usingAllDomains.
//
//    Kathleen Bonnell, Fri Oct 12 12:07:01 PDT 2001 
//    Copy over isThisDynamic.
//
//    Kathleen Bonnell, Wed Dec 12 10:50:01 PST 2001 
//    Copy over pointsWereTransformed.
//
//    Hank Childs, Sun Jun 23 23:08:13 PDT 2002
//    Copy over wireframeRenderingIsInappropriate.
//
//    Hank Childs, Tue Aug  6 10:57:20 PDT 2002
//    Copy over normalsAreInappropriate.
//
//    Jeremy Meredith, Tue Aug 13 10:10:35 PDT 2002
//    Added subdivisionOccurred and notAllCellsSubdivided.
//
//    Hank Childs, Sun Aug 18 10:58:23 PDT 2002
//    Copy over disjointElements.
//
//    Kathleen Bonnell, Tue Oct 22 11:44:55 PDT 2002   
//    Copy over queryable.
//
//    Hank Childs, Fri May 16 10:18:12 PDT 2003
//    Copy over errorOccurred, errorString.
//
//    Mark C. Miller, Thu Jan 29 16:40:25 PST 2004 
//    Added hasEverOwnedAnyDomain
//
//    Kathleen Bonnell, Thu Mar  2 14:04:06 PST 2006
//    Added originalZonesIntact.
//
// ****************************************************************************

void
avtDataValidity::Copy(const avtDataValidity &di)
{
    zonesPreserved                    = di.zonesPreserved;
    originalZonesIntact               = di.originalZonesIntact;
    spatialMetaDataPreserved          = di.spatialMetaDataPreserved;
    dataMetaDataPreserved             = di.dataMetaDataPreserved;
    operationFailed                   = di.operationFailed;
    usingAllData                      = di.usingAllData;
    usingAllDomains                   = di.usingAllDomains;
    isThisDynamic                     = di.isThisDynamic;
    pointsWereTransformed             = di.pointsWereTransformed;
    wireframeRenderingIsInappropriate = di.wireframeRenderingIsInappropriate;
    normalsAreInappropriate           = di.normalsAreInappropriate;
    subdivisionOccurred               = di.subdivisionOccurred;
    notAllCellsSubdivided             = di.notAllCellsSubdivided;
    disjointElements                  = di.disjointElements;
    queryable                         = di.queryable;
    hasEverOwnedAnyDomain             = di.hasEverOwnedAnyDomain;
    errorOccurred                     = di.errorOccurred;
    errorString                       = di.errorString;
}


// ****************************************************************************
//  Method: avtDataValidity::Merge
//
//  Purpose:
//      Merges the data validity information from the argument and this object.
//
//  Arguments:
//      di      Data validity information from another object.
//
//  Programmer: Hank Childs
//  Creation:   March 25, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Sep 14 09:38:58 PDT 2001
//    Account for usingAllDomains.
//
//    Kathleen Bonnell, Fri Oct 12 12:07:01 PDT 2001 
//    Account for isThisDynamic.
//
//    Kathleen Bonnell, Wed Dec 12 10:50:01 PST 2001 
//    Account for pointsWereTransformed.
//
//    Hank Childs, Sun Jun 23 23:08:13 PDT 2002
//    Account for wireframeRenderingIsInappropriate.
//
//    Hank Childs, Tue Aug  6 10:57:20 PDT 2002
//    Account for normalsAreInappropriate.
//
//    Jeremy Meredith, Tue Aug 13 10:10:35 PDT 2002
//    Added subdivisionOccurred and notAllCellsSubdivided.
//
//    Hank Childs, Sun Aug 18 10:58:23 PDT 2002
//    Account for disjointElements.
//
//    Kathleen Bonnell, Tue Oct 22 11:44:55 PDT 2002   
//    Account for queryable.
//
//    Hank Childs, Fri May 16 10:18:12 PDT 2003
//    Account for errorOccurred, errorString.
//
//    Mark C. Miller, Thu Jan 29 16:40:25 PST 2004
//    Added hasEverOwnedAnyDomain 
//
//    Kathleen Bonnell, Thu Mar  2 14:04:06 PST 2006
//    Added originalZonesIntact.
//
// ****************************************************************************

void
avtDataValidity::Merge(const avtDataValidity &di)
{
    zonesPreserved           = zonesPreserved && di.zonesPreserved;
    originalZonesIntact      = originalZonesIntact && di.originalZonesIntact;
    spatialMetaDataPreserved = spatialMetaDataPreserved
                               && di.spatialMetaDataPreserved;
    dataMetaDataPreserved    = dataMetaDataPreserved
                               && di.dataMetaDataPreserved;
    usingAllData             = usingAllData && di.usingAllData;
    usingAllDomains          = usingAllDomains && di.usingAllDomains;
    isThisDynamic            = isThisDynamic && di.isThisDynamic;
    pointsWereTransformed   = pointsWereTransformed || di.pointsWereTransformed;
    wireframeRenderingIsInappropriate = wireframeRenderingIsInappropriate ||
                                        di.wireframeRenderingIsInappropriate;
    normalsAreInappropriate  = normalsAreInappropriate ||
                               di.normalsAreInappropriate;
    disjointElements = disjointElements && di.disjointElements;
    queryable = queryable && di.queryable;
    hasEverOwnedAnyDomain = hasEverOwnedAnyDomain || di.hasEverOwnedAnyDomain;

    // If not all cells were subdivided in either dataset, or if all were
    // in one but not both, then not all of our cells were subdivided
    notAllCellsSubdivided  = notAllCellsSubdivided || di.notAllCellsSubdivided
                          || (subdivisionOccurred  && !di.subdivisionOccurred)
                          || (!subdivisionOccurred &&  di.subdivisionOccurred);
    subdivisionOccurred    = subdivisionOccurred   || di.subdivisionOccurred;

    //
    // Very conscious decision here -- if an operation failed on one processor,
    // but not on others, then say that the operation succeeded.  This is
    // because if the operation *really* failed, an exception would have been
    // thrown.  OperationFailed is used for things like "a contour filter had
    // no output".
    //
    operationFailed          = operationFailed && di.operationFailed;

    if (!errorOccurred && di.errorOccurred)
        errorString = di.errorString;
    errorOccurred = errorOccurred || di.errorOccurred;
}


// ****************************************************************************
//  Method: avtDataValidity::Write
//
//  Purpose:
//      Writes the data validity object out to a stream (string).
//
//  Arguments:
//      str      The string to write to.
//      wrtr     The writer that has information/methods about the destination
//               format.
//
//  Notes:     When adding a new bool, you must make the temporary array that
//             the values are placed into bigger.
//
//  Progammer: Hank Childs
//  Creation:  March 25, 2001
//
//  Modifications:
//
//    Hank Childs, Sat May 26 10:06:58 PDT 2001
//    Made use of avtDataObjectString::Append instead of string::append to
//    prevent bottleneck.
//
//    Hank Childs, Fri Sep 14 09:38:58 PDT 2001
//    Write usingAllDomains.
//
//    Hank Childs, Wed Sep 19 10:16:29 PDT 2001
//    Make use of array writes.
//
//    Kathleen Bonnell, Fri Oct 12 12:07:01 PDT 2001 
//    Write isThisDynamic.
//
//    Kathleen Bonnell, Wed Dec 12 10:50:01 PST 2001 
//    Write pointsWereTransformed.
//
//    Hank Childs, Sun Jun 23 23:08:13 PDT 2002
//    Write wireframeRenderingIsInappropriate.
//
//    Hank Childs, Tue Aug  6 10:57:20 PDT 2002
//    Write normalsAreInappropriate.
//
//    Jeremy Meredith, Tue Aug 13 10:10:35 PDT 2002
//    Added subdivisionOccurred and notAllCellsSubdivided.
//
//    Hank Childs, Sun Aug 18 10:58:23 PDT 2002
//    Write disjointElements.
//
//    Kathleen Bonnell, Tue Oct 22 11:44:55 PDT 2002   
//    Write queryable.
//
//    Hank Childs, Fri May 16 10:18:12 PDT 2003
//    Write errorOccrred, errorString.
//
//    Mark C. Miller, Thu Jan 29 16:40:25 PST 2004
//    Added hasEverOwnedAnyDomain
//
//    Kathleen Bonnell, Thu Mar  2 14:04:06 PST 2006
//    Added originalZonesIntact.
//
// ****************************************************************************

void
avtDataValidity::Write(avtDataObjectString &str,
                       const avtDataObjectWriter *wrtr)
{
    const int numVals = 18;
    int  vals[numVals];

    vals[0] = (zonesPreserved ? 1 : 0);
    vals[1] = (originalZonesIntact ? 1 : 0);
    vals[2] = (dataMetaDataPreserved ? 1 : 0);
    vals[3] = (spatialMetaDataPreserved ? 1 : 0);
    vals[4] = (operationFailed ? 1 : 0);
    vals[5] = (usingAllData ? 1 : 0);
    vals[6] = (usingAllDomains ? 1 : 0);
    vals[7] = (isThisDynamic ? 1 : 0);
    vals[8] = (pointsWereTransformed ? 1 : 0);
    vals[9] = (wireframeRenderingIsInappropriate ? 1 : 0);
    vals[10] = (normalsAreInappropriate ? 1 : 0);
    vals[11]= (subdivisionOccurred ? 1 : 0);
    vals[12]= (notAllCellsSubdivided ? 1 : 0);
    vals[13]= (disjointElements ? 1 : 0);
    vals[14]= (queryable ? 1 : 0);
    vals[15]= (hasEverOwnedAnyDomain ? 1 : 0);
    vals[16]= (errorOccurred ? 1 : 0);
    vals[17]= errorString.size();
    wrtr->WriteInt(str, vals, numVals);

    str.Append((char *) errorString.c_str(), errorString.size(),
                     avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY); 
}


// ****************************************************************************
//  Method: avtDataValidity::Read
//
//  Purpose:
//      Read the data validity object from a stream (string).
//
//  Arguments:
//      input  The string to read from.
//
//  Returns:   The amount read.
//
//  Progammer: Hank Childs
//  Creation:  March 25, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Sep 14 09:38:58 PDT 2001
//    Read usingAllDomains.
//
//    Kathleen Bonnell, Fri Oct 12 12:07:01 PDT 2001 
//    Read isThisDynamic.
//
//    Kathleen Bonnell, Wed Dec 12 10:50:01 PST 2001 
//    Read pointsWereTransformed.
//
//    Hank Childs, Sun Jun 23 23:08:13 PDT 2002
//    Read wireframeRenderingIsInappropriate.
//
//    Hank Childs, Tue Aug  6 10:57:20 PDT 2002
//    Read normalsAreInappropriate.
//
//    Jeremy Meredith, Tue Aug 13 10:10:35 PDT 2002
//    Added subdivisionOccurred and notAllCellsSubdivided.
//
//    Hank Childs, Sun Aug 18 10:58:23 PDT 2002
//    Read disjointElements.
//
//    Kathleen Bonnell, Tue Oct 22 11:44:55 PDT 2002   
//    Read queryable.
//
//    Hank Childs, Fri May 16 10:18:12 PDT 2003
//    Read errorOccrred, errorString.
//
//    Mark C. Miller, Thu Jan 29 16:40:25 PST 2004
//    Added hasEverOwnedAnyDomain, careful to put in right order in sequence
//
//    Kathleen Bonnell, Thu Mar  2 14:04:06 PST 2006
//    Added originalZonesIntact.
//
// ****************************************************************************

int
avtDataValidity::Read(char *input)
{
    int  size = 0;

    // read zones preserved
    int zp;
    memcpy(&zp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    if (zp == 0)
    {
        InvalidateZones();
    }

    // read zones intact
    int zi;
    memcpy(&zi, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    if (zi == 0)
    {
        ZonesSplit();
    }

    // read data extents preserved
    int dep;
    memcpy(&dep, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    if (dep == 0)
    {
        InvalidateDataMetaData();
    }

    // read spatial extents preserved
    int sep;
    memcpy(&sep, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    if (sep == 0)
    {
        InvalidateSpatialMetaData();
    }

    // read operation failed
    int of;
    memcpy(&of, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    if (of == 1)
    {
        InvalidateOperation();
    }

    // read using all data
    int uad;
    memcpy(&uad, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetUsingAllData((uad == 1 ? true : false));

    // read using all domains
    int uadom;
    memcpy(&uadom, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetUsingAllDomains((uadom == 1 ? true : false));

    // read is this dynamic
    int itd;
    memcpy(&itd, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetIsThisDynamic((itd == 1 ? true : false));

    // read points were transformed
    int pwt;
    memcpy(&pwt, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetPointsWereTransformed((pwt == 1 ? true : false));

    // read whether wireframe renderings are appropriate.
    int wr;
    memcpy(&wr, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetWireframeRenderingIsInappropriate((wr == 1 ? true : false));

    // read whether normals are appropriate.
    int naa;
    memcpy(&naa, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetNormalsAreInappropriate((naa == 1 ? true : false));

    // read whether subdivision occurred
    int subdiv;
    memcpy(&subdiv, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetSubdivisionOccurred((subdiv == 1 ? true : false));

    // read whether some cells were not subdivided
    int notall;
    memcpy(&notall, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetNotAllCellsSubdivided((notall == 1 ? true : false));

    // read whether the elements are disjoint.
    int dis_elem;
    memcpy(&dis_elem, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetDisjointElements((dis_elem == 1 ? true : false));

    // read whether the object is queryable.
    int queryable;
    memcpy(&queryable, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetQueryable((queryable == 1 ? true : false));

    // read whether the object has ever owned any domain.
    int everOwned;
    memcpy(&everOwned, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetHasEverOwnedAnyDomain((everOwned == 1 ? true : false));

    int eo;
    memcpy(&eo, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    if (eo != 0)
        ErrorOccurred();

    int esSize;
    memcpy(&esSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string l(input, esSize);
    errorString = l;
    size += esSize;
    input += esSize;

    return size;
}


