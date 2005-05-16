// ************************************************************************* //
//                            avtGenericDatabase.C                           //
// ************************************************************************* //

#include <avtGenericDatabase.h>

#include <float.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataRelevantPointsFilter.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridBoundaryFilter.h>
#include <vtkUnstructuredGridFacelistFilter.h>
#include <vtkVisItUtility.h>

#include <snprintf.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <avtDatasetCollection.h>
#include <avtDatasetVerifier.h>
#include <avtDomainBoundaries.h>
#include <avtDomainNesting.h>
#include <avtFileFormatInterface.h>
#include <avtMixedVariable.h>
#include <avtParallel.h>
#include <avtSILGenerator.h>
#include <avtSILRestrictionTraverser.h>
#include <avtSourceFromDatabase.h>
#include <avtTypes.h>
#include <avtUnstructuredPointBoundaries.h>
#include <PickAttributes.h>
#include <PickVarInfo.h>
#include <TetMIR.h>
#include <ZooMIR.h>

#include <DebugStream.h>
#include <BadDomainException.h>
#include <ImproperUseException.h>
#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>
#include <NoInputException.h>
#include <TimingsManager.h>

using     std::string;
using     std::vector;


// Function prototypes for static functions.
static const char   *GetOriginalVariableName(const avtDatabaseMetaData *,
                                             const char *);

// ****************************************************************************
//  Function: GetArrayTypeName 
//
//  Purpose: Given a vtk data array, return the a char * for its type name
//
//  Programmer: Mark C. Miller 
//  Creation:   April 4, 2005 
// 
// ****************************************************************************

static const char *DataArrayTypeName(vtkDataArray *arr)
{
    switch (arr->GetDataType())
    {
        case VTK_VOID:           return "void*"; 
        // case VTK_BIT: not sure what to do here
        case VTK_CHAR:           return "char"; 
        case VTK_UNSIGNED_CHAR:  return "unsigned char"; 
        case VTK_SHORT:          return "short";
        case VTK_UNSIGNED_SHORT: return "unsigned short";
        case VTK_INT:            return "int";
        case VTK_UNSIGNED_INT:   return "unsigned int";
        case VTK_LONG:           return "long";
        case VTK_UNSIGNED_LONG:  return "unsigned long";
        case VTK_FLOAT:          return "float";
        case VTK_DOUBLE:         return "double";
        case VTK_ID_TYPE:        return "vtkIdType";
    }
}

// ****************************************************************************
//  Function: PrecisionInBytes
//
//  Purpose: Given a vtk data array, return its precision in bytes 
//
//  Programmer: Mark C. Miller 
//  Creation:   April 4, 2005 
// 
// ****************************************************************************

static int
PrecisionInBytes(vtkDataArray *var)
{
    switch (var->GetDataType())
    {
        case VTK_VOID:           return sizeof(void*);
        // case VTK_BIT: not sure what to do here
        case VTK_CHAR:           return sizeof(char);
        case VTK_UNSIGNED_CHAR:  return sizeof(unsigned char);
        case VTK_SHORT:          return sizeof(short);
        case VTK_UNSIGNED_SHORT: return sizeof(unsigned short);
        case VTK_INT:            return sizeof(int);
        case VTK_UNSIGNED_INT:   return sizeof(unsigned int);
        case VTK_LONG:           return sizeof(long);
        case VTK_UNSIGNED_LONG:  return sizeof(unsigned long);
        case VTK_FLOAT:          return sizeof(float);
        case VTK_DOUBLE:         return sizeof(double);
        case VTK_ID_TYPE:        return sizeof(vtkIdType);
    }
}

// ****************************************************************************
//  Function: IsAdmissibleDataType
//
//  Purpose: Given a vector of admissible types and a given type, return
//  whehter or not the given type is in the vector
//
//  Programmer: Mark C. Miller 
//  Creation:   April 4, 2005 
// 
// ****************************************************************************

static bool
IsAdmissibleDataType(const vector<int>& admissibleTypes, const int type)
{
    for (int i = 0; i < admissibleTypes.size(); i++)
    {
        if (admissibleTypes[i] == type)
            return true;
    }
    return false;
}

// ****************************************************************************
//  Template: ConvertToType
//
//  Purpose: Template for conversion, usually to float 
//
//  Programmer: Mark C. Miller 
//  Creation:   April 4, 2005 
// 
// ****************************************************************************
template <class oT, class iT>
static void ConvertToType(oT *obuf, const iT* ibuf, int n)
{
    for (int i = 0; i < n; i++)
        obuf[i] = (oT) ibuf[i];
}

// ****************************************************************************
//  Function: ConvertDataArrayToFloat 
//
//  Purpose: Given a vtk data array, make a copie of it that is converted
//  to float
//
//  Programmer: Mark C. Miller 
//  Creation:   April 4, 2005 
// 
//  Modifications:
//    Jeremy Meredith, Tue Apr  5 11:49:30 PDT 2005
//    Removed the templating.  It was causing compiler errors.
//
// ****************************************************************************

static vtkDataArray * 
ConvertDataArrayToFloat(vtkDataArray *oldArr)
{
    vtkDataArray *newArr = 0; 

    if (oldArr->GetDataType() != VTK_FLOAT)
    {
        debug5 << "avtGenericDatabase::Converting vktDataArray, \""
               << oldArr->GetName() << "\" from type \""
               << DataArrayTypeName(oldArr) << "\" to \"float\"" << endl;

        newArr = vtkFloatArray::New();

        int numTuples = oldArr->GetNumberOfTuples();
        int numComponents = oldArr->GetNumberOfComponents();

        newArr->SetNumberOfComponents(numComponents);
        newArr->SetNumberOfTuples(numTuples);

        float *newBuf = (float*) newArr->GetVoidPointer(0);
        void *oldBuf = oldArr->GetVoidPointer(0);

        int numValues = numTuples * numComponents;
        switch (oldArr->GetDataType())
        {
            case VTK_CHAR:
                ConvertToType(newBuf, (char*) oldBuf, numValues);
                break;
            case VTK_UNSIGNED_CHAR:
                ConvertToType(newBuf, (unsigned char*) oldBuf, numValues);
                break;
            case VTK_SHORT:
                ConvertToType(newBuf, (short*) oldBuf, numValues);
                break;
            case VTK_UNSIGNED_SHORT:
                ConvertToType(newBuf, (unsigned short*) oldBuf, numValues);
                break;
            case VTK_INT:
                ConvertToType(newBuf, (int*) oldBuf, numValues);
                break;
            case VTK_UNSIGNED_INT:
                ConvertToType(newBuf, (unsigned int*) oldBuf, numValues);
                break;
            case VTK_LONG:
                ConvertToType(newBuf, (long*) oldBuf, numValues);
                break;
            case VTK_UNSIGNED_LONG:
                ConvertToType(newBuf, (unsigned long*) oldBuf, numValues);
                break;
            case VTK_DOUBLE:
                ConvertToType(newBuf, (double*) oldBuf, numValues);
                break;
            case VTK_ID_TYPE:
                ConvertToType(newBuf, (vtkIdType*) oldBuf, numValues);
                break;
            default:
                {   char msg[256];
                    SNPRINTF(msg, sizeof(msg), "Cannot convert from type \"%s\" to float",
                        DataArrayTypeName(oldArr));
                    EXCEPTION1(ImproperUseException, msg); 
                }
        }
    }

    return newArr;
}

// ****************************************************************************
//  Funcion: GetCoordDataArray
//
//  Purpose: Given a vtkDataSet, return a sample the first of its
//  coordinate arrays
//
//  Programmer: Mark C. Miller 
//  Creation:   April 4, 2005 
// 
// ****************************************************************************
static vtkDataArray*
GetCoordDataArray(vtkDataSet *ds)
{
    switch (ds->GetDataObjectType())
    {
        case VTK_POLY_DATA:
        case VTK_STRUCTURED_GRID:
        case VTK_UNSTRUCTURED_GRID:
            {
                vtkPointSet *ps = vtkPointSet::SafeDownCast(ds);
                if (ps != 0)
                {
                    return ps->GetPoints()->GetData();
                }
            }
            break;

        case VTK_RECTILINEAR_GRID:
            {
                vtkRectilinearGrid *rg = vtkRectilinearGrid::SafeDownCast(ds);
                if (rg != 0)
                {
                    return rg->GetXCoordinates();
                }
            }
            break;

        case VTK_IMAGE_DATA:
        case VTK_STRUCTURED_POINTS:
            return 0;
            break;
    }
    return 0;
}

// ****************************************************************************
//  Function: ConvertDataSetToFloat
//
//  Purpose: Given a vtkDataSet, convert its coordinate arrays, if any,
//  to float
//
//  Programmer: Mark C. Miller 
//  Creation:   April 4, 2005 
// 
// ****************************************************************************
static vtkDataSet * 
ConvertDataSetToFloat(vtkDataSet *oldds)
{
    vtkDataSet *newds = 0;

    switch (oldds->GetDataObjectType())
    {
        case VTK_POLY_DATA:
        case VTK_STRUCTURED_GRID:
        case VTK_UNSTRUCTURED_GRID:
            {
                vtkPointSet *oldps = vtkPointSet::SafeDownCast(oldds);
                if (oldps != 0)
                {
                    vtkDataArray *oldArr = oldps->GetPoints()->GetData();
                    vtkDataArray *newArr = ConvertDataArrayToFloat(oldArr);

                    if (newArr != 0)
                    {
                        vtkPointSet *newps = oldps->NewInstance(); 
                        newps->DeepCopy(oldps);

                        vtkPoints *newpts = vtkPoints::New();
                        newpts->DeepCopy(oldps->GetPoints());

                        newpts->SetData(newArr);
                        newArr->Delete();

                        newps->SetPoints(newpts);
                        newpts->Delete();

                        newds = newps;
                    }
                }
            }
            break;

        case VTK_RECTILINEAR_GRID:
            {
                vtkRectilinearGrid *oldrg = vtkRectilinearGrid::SafeDownCast(oldds);
                if (oldrg != 0)
                {
                    vtkDataArray *oldX = oldrg->GetXCoordinates();
                    vtkDataArray *newX = ConvertDataArrayToFloat(oldX);
                    vtkDataArray *oldY = oldrg->GetYCoordinates();
                    vtkDataArray *newY = ConvertDataArrayToFloat(oldY);
                    vtkDataArray *oldZ = oldrg->GetZCoordinates();
                    vtkDataArray *newZ = ConvertDataArrayToFloat(oldZ);

                    if (newX != 0 && newY != 0 && newZ != 0)
                    {
                        vtkRectilinearGrid *newrg = vtkRectilinearGrid::New();
                        newrg->DeepCopy(oldrg);

                        newrg->SetXCoordinates(newX);
                        newX->Delete();

                        newrg->SetYCoordinates(newY);
                        newY->Delete();

                        newrg->SetZCoordinates(newZ);
                        newZ->Delete();

                        newds = newrg;
                    }
                }
            }
            break;

        case VTK_IMAGE_DATA:
        case VTK_STRUCTURED_POINTS:
            // we have nothing to do here
            break;
    }
    return newds;
}


// ****************************************************************************
//  Method: avtGenericDatabase constructor
//
//  Arguments:
//      inter   The interface -- the database will delete this object when it
//              is done with it.
//
//  Programmer: Hank Childs
//  Creation:   March 1, 2001
// 
//  Modifications:
//
//    Hank Childs, Thu Sep 20 14:19:34 PDT 2001
//    Give the formats a chance to put stuff in the cache.
//
//    Hank Childs, Fri Mar 11 11:24:25 PST 2005
//    Initialize non-cachable references.
//
// ****************************************************************************

avtGenericDatabase::avtGenericDatabase(avtFileFormatInterface *inter)
{
    Interface = inter;
    Interface->SetCache(&cache);
    lastTimestep = -1;
    nonCachableVariableReference = NULL;
    nonCachableMeshReference = NULL;
}


// ****************************************************************************
//  Method: avtGenericDatabase destructor
//
//  Programmer: Hank Childs
//  Creation:   March 1, 2001
//
// ****************************************************************************

avtGenericDatabase::~avtGenericDatabase()
{
    if (Interface != NULL)
    {
        delete Interface;
    }
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetFilename
//
//  Purpose:
//      Gets the name of the database.
//
//  Returns:    The name for the database.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
// ****************************************************************************

const char *
avtGenericDatabase::GetFilename(int ts)
{
    return Interface->GetFilename(ts);
}


// ****************************************************************************
//  Method: avtGenericDatabase::SetDatabaseMetaData
//
//  Purpose:
//      Sets the database meta-data using the file format interface.
//
//  Arguments:
//      md        : The meta-data to set.
//      timeState : The time state that we're interested in.
//
//  Programmer: Hank Childs
//  Creation:   March 2, 2001
//
//  Modifications:
//    Brad Whitlock, Wed May 14 09:15:18 PDT 2003
//    I added the optional timeState argument.
//
//    Hank Childs, Tue Feb 15 07:21:10 PST 2005
//    Replace forbidden characters of expression language.
//
// ****************************************************************************

void
avtGenericDatabase::SetDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    Interface->SetDatabaseMetaData(md, timeState);

    std::vector<char>        forbiddenChars;
    std::vector<std::string> replacementStrs;

    forbiddenChars.push_back(' ');
    replacementStrs.push_back("_");

    forbiddenChars.push_back('\n');
    replacementStrs.push_back("_nl_");

    forbiddenChars.push_back('\t');
    replacementStrs.push_back("_tab_");

    forbiddenChars.push_back('.');
    replacementStrs.push_back("_dot_");

    forbiddenChars.push_back('!');
    replacementStrs.push_back("_");
    forbiddenChars.push_back('@');
    replacementStrs.push_back("_at_");
    forbiddenChars.push_back('#');
    replacementStrs.push_back("_number_");
    forbiddenChars.push_back('$');
    replacementStrs.push_back("_dollar_");
    forbiddenChars.push_back('%');
    replacementStrs.push_back("_percent_");
    forbiddenChars.push_back('^');
    replacementStrs.push_back("_carat_");
    forbiddenChars.push_back('&');
    replacementStrs.push_back("_ampersand_");
    forbiddenChars.push_back('*');
    replacementStrs.push_back("_star_");

    forbiddenChars.push_back('-');
    replacementStrs.push_back("_hyphen_");
    forbiddenChars.push_back('+');
    replacementStrs.push_back("_plus_");
    forbiddenChars.push_back('=');
    replacementStrs.push_back("_equal_");
    forbiddenChars.push_back('|');
    replacementStrs.push_back("_pipe_");
    forbiddenChars.push_back('~');
    replacementStrs.push_back("_tilde_");
    forbiddenChars.push_back(':');
    replacementStrs.push_back("_colon_");
    forbiddenChars.push_back(';');
    replacementStrs.push_back("_semicolon_");
    forbiddenChars.push_back('\"');
    replacementStrs.push_back("_quote_");
    forbiddenChars.push_back('\'');
    replacementStrs.push_back("_quote_");
    forbiddenChars.push_back('.');
    replacementStrs.push_back("_dot_");
    forbiddenChars.push_back(',');
    replacementStrs.push_back("_comma_");
    forbiddenChars.push_back('?');
    replacementStrs.push_back("_question_");

    forbiddenChars.push_back('[');
    replacementStrs.push_back("_");
    forbiddenChars.push_back(']');
    replacementStrs.push_back("_");

    forbiddenChars.push_back('{');
    replacementStrs.push_back("_");
    forbiddenChars.push_back('}');
    replacementStrs.push_back("_");

    forbiddenChars.push_back('<');
    replacementStrs.push_back("_");
    forbiddenChars.push_back('>');
    replacementStrs.push_back("_");

    forbiddenChars.push_back('(');
    replacementStrs.push_back("_");
    forbiddenChars.push_back(')');
    replacementStrs.push_back("_");

    md->ReplaceForbiddenCharacters(forbiddenChars, replacementStrs);
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetOutput
//
//  Purpose:
//      Takes a database specification and generates the correct dataset
//      output.
//
//  Arguments:
//      spec    A database specification.
//      v       An object that represents the validity of the dataset.
//
//  Returns:    A domain tree with a dataset for each domain.
//
//  Programmer: Hank Childs
//  Creation:   March 1, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Apr  9 14:47:12 PDT 2001
//    Made this method return avtDataTree_p.
//
//    Hank Childs, Tue May 22 10:47:31 PDT 2001
//    Use a database specification instead of a SIL restriction.
//
//    Hank Childs, Tue Jul 24 14:17:14 PDT 2001
//    Add data validity argument.
//
//    Hank Childs, Fri Oct  5 15:30:57 PDT 2001
//    Account for file formats that do their own material selection.
//
//    Kathleen Bonnell, Fri Oct 19 15:33:35 PDT 2001 
//    Create domain labels for output data tree when requested. 
//
//    Hank Childs, Tue Oct 23 09:18:43 PDT 2001
//    Add support for reading in multiple variables.
//
//    Eric Brugger, Mon Nov  5 13:32:49 PST 2001
//    Modified to always compile the timing code.
//
//    Jeremy Meredith, Wed Nov 21 15:11:45 PST 2001
//    Disabled ghost zone communication temporarily.
//
//    Hank Childs, Wed Nov 28 13:01:00 PST 2001
//    Do not communicate ghost zones if they are not appropriate.
//
//    Kathleen Bonnell,  Fri Nov 30 10:36:41 PST 2001
//    Create Original Zones array if data spec specifies they are needed.
//
//    Jeremy Meredith, Thu Dec 13 11:42:01 PST 2001
//    Re-enabled ghost zone communication again.
//
//    Hank Childs, Wed Dec 19 09:37:04 PST 2001
//    Added support for species selection.
//
//    Kathleen Bonnell, Mon May 20 17:01:31 PDT 2002  
//    Added 'domains' parameter to CreateOriginalZones. 
//
//    Hank Childs, Mon Sep 30 17:56:43 PDT 2002
//    Add support for creating structured indices.
//
//    Hank Childs, Fri Nov 22 16:39:13 PST 2002
//    Use the SIL restriction traverser since SIL restriction routines were
//    antiquated.
//
//    Kathleen Bonnell, Fri Dec  6 12:10:04 PST 2002   
//    Add OriginalCellsArray whenever MatSelect will be performed,
//    to ensure that pick will operate correctly.  
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003
//    Tell the MetaData when the originalCells array has been added.
// 
//    Jeremy Meredith, Thu Jun 12 09:06:49 PDT 2003
//    Added the data spec to the input of PopulateDataObjectInformation.
//
//    Jeremy Meredith, Sat Aug  2 20:40:04 PDT 2003
//    If any processor decides to do material selection, they all should.
//    Unified shouldDoMatSelect across all processors.
//
//    Hank Childs, Wed Nov 12 10:34:57 PST 2003
//    Communicate ghost zones after applying AMR nesting.  This is because it
//    is difficult to do them in the opposite order (nesting must be on orig
//    indices), but fairly straight-forward to do them in new order.
//
//    Hank Childs, Fri Jan  9 13:46:43 PST 2004
//    Use a dataset verifier before passing data into routines like the MIR.
//
//    Kathleen Bonnell, Fri May 28 18:31:15 PDT 2004 
//    Add OriginalNodesArray whenever MatSelect will be performed,
//    to ensure that pick will operate correctly.  
//
//    Kathleen Bonnell, Wed Jun 23 17:04:23 PDT 2004 
//    Add TRY-CATCH, so that no process skips the parallel communication. 
//
//    Hank Childs, Wed Aug 11 08:14:16 PDT 2004
//    Only create ghost data if it is requested.
//
//    Mark C. Miller, Thu Aug 12 12:10:37 PDT 2004
//    I changed behavior when processors have errors during the read phase.
//    Previously, only the err'ing processor knew about and would return
//    early. Since we are doing an MPI_Allreduce for the mat select mode,
//    I simple tacked on a second int indicating if anyone had errors
//    and included that in the all reduce as well. Now, if any processor
//    encounters an error during the read phase, they will are return
//    together.
//
//    Hank Childs, Thu Sep 23 09:28:58 PDT 2004
//    Add support for getting global ids.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added vector of bools for data selections that plugins apply
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector' and 'vector<bool>' to 'boolVector'.
//
//    Hank Childs, Sun Feb 27 14:47:45 PST 2005
//    Pass "allDomains" to CommunicateGhosts.
//
//    Hank Childs, Sat Mar  5 19:26:05 PST 2005
//    Do not do collective communication if we are in DLB mode.
//
//    Hank Childs, Mon Mar 28 15:14:39 PST 2005
//    Add some more timing information.
//
// ****************************************************************************

avtDataTree_p
avtGenericDatabase::GetOutput(avtDataSpecification_p spec,
                              avtSourceFromDatabase *src)
{
    int timerHandle = visitTimer->StartTimer();
    int timeStep = spec->GetTimestep();

    UpdateInternalState(timeStep);

    //
    // Determine which domains the SIL restriction has turned on.
    //
    avtSILRestriction_p silr = spec->GetRestriction();
    avtSILRestrictionTraverser trav(silr);
    intVector domains, allDomains;
    trav.GetDomainList(domains);
    trav.GetDomainListAllProcs(allDomains);

    //
    // Set up a data tree for each of the domains.
    //
    int                    nDomains = domains.size();
    avtDatasetCollection   datasetCollection(nDomains);

    bool shouldDoMatSelect = false;
    bool hadError = false;
    boolVector selectionsApplied;
    TRY
    {
        //
        // This is the primary routine that reads things in from disk.
        //
        ReadDataset(datasetCollection, domains, spec, src, selectionsApplied);

        //
        // Now that we have read things in from disk, verify that the dataset
        // is valid, since routines like the MIR downstream will assume they are.
        //
        avtDatasetVerifier verifier;
        vtkDataSet **ds_list = new vtkDataSet*[nDomains];
        int i;
        for (i = 0 ; i < nDomains ; i++)
        {
            ds_list[i] = datasetCollection.GetDataset(i, 0);
        }
        verifier.VerifyDatasets(nDomains, ds_list, domains);
        delete [] ds_list;

        //
        // Do species selection if appropriate.
        //
        boolVector speciesList;
        if (trav.GetSpecies(speciesList))
        {
            SpeciesSelect(datasetCollection, domains, speciesList, spec, src);
        }


        //
        //  HACK!!! Pick requires original cells array whenever 
        //  MaterialSelection has occurred.  Rather than incur the expense of 
        //  re-execution at StartPick, always send the array when MatSelected,
        //  until a better solution is derived.
        // 
        for (i = 0 ; i < datasetCollection.GetNDomains() ; i++)
        {
            shouldDoMatSelect = shouldDoMatSelect || 
                                datasetCollection.needsMatSelect[i];
        }
    }
    CATCH2(VisItException, e)
    {
        //
        //  Only set an error condition, the early exit from this method
        //  is after the parallel communication.  There may be a better
        //  way to handle this.
        //
        hadError = true;
        debug1 << "Catching the exception at the generic database level." 
               << endl;
        avtDataValidity &v = src->GetOutput()->GetInfo().GetValidity();
        v.ErrorOccurred();
        string tmp = e.Message(); // Otherwise there is a const problem.
        v.SetErrorMessage(tmp);
    }
    ENDTRY

    avtDataValidity &validity = src->GetOutput()->GetInfo().GetValidity();
    bool canDoCollectiveCommunication = !validity.GetIsThisDynamic();
#ifdef PARALLEL
    int t1 = visitTimer->StartTimer();
    if (canDoCollectiveCommunication)
    {
        //
        // If any processor decides to do material selection, they all should
        // If any processor had an error, they all should return
        //
        int tmp[2], rtmp[2];
        tmp[0] = (shouldDoMatSelect ? 1 : 0);
        tmp[1] = (hadError ? 1 : 0);
        MPI_Allreduce(tmp, rtmp, 2, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
        shouldDoMatSelect = bool(rtmp[0]);
        hadError = bool(rtmp[1]);
    }
    visitTimer->StopTimer(t1, "Waiting for all processors to catch up");
#endif

    if (hadError)
    {
        avtDataTree_p rv = new avtDataTree();
        return rv;
    }

    if (shouldDoMatSelect)
    {
        spec->TurnZoneNumbersOn();
        spec->TurnNodeNumbersOn();
    }

    //
    // Add node numbers if requested.
    //
    if (spec->NeedNodeNumbers())
    {
        CreateOriginalNodes(datasetCollection, domains, src);
        //
        // Tell everything downstream that we do have original cells.
        //
        avtDatabaseMetaData *md = GetMetaData(timeStep);
        string meshname = md->MeshForVar(spec->GetVariable());
        GetMetaData(timeStep)->SetContainsOriginalNodes(meshname, true);
    }

    //
    // Add zone numbers if requested.
    //
    if (spec->NeedZoneNumbers())
    {
        CreateOriginalZones(datasetCollection, domains, src);
        //
        // Tell everything downstream that we do have original cells.
        //
        avtDatabaseMetaData *md = GetMetaData(timeStep);
        string meshname = md->MeshForVar(spec->GetVariable());
        GetMetaData(timeStep)->SetContainsOriginalCells(meshname, true);
    }

    //
    // Add global node numbers if requested.
    //
    if (spec->NeedGlobalNodeNumbers())
    {
        CreateGlobalNodes(datasetCollection, domains, src, spec);
    }

    //
    // Add zone numbers if requested.
    //
    if (spec->NeedGlobalZoneNumbers())
    {
        CreateGlobalZones(datasetCollection, domains, src, spec);
    }

    //
    // Add the indices for a structured mesh if requested.
    //
    if (spec->NeedStructuredIndices())
    {
        CreateStructuredIndices(datasetCollection, src);
        //
        // Tell everything downstream that we do have original cells.
        //
        avtDatabaseMetaData *md = GetMetaData(timeStep);
        string meshname = md->MeshForVar(spec->GetVariable());
        GetMetaData(timeStep)->SetContainsOriginalCells(meshname, true);
    }

    //
    // Apply ghosting when domains nest within other domains (AMR meshes)
    //
    ApplyGhostForDomainNesting(datasetCollection, domains, allDomains, spec,
                               canDoCollectiveCommunication);

    //
    // Communicates ghost zones if they are not present and we have domain
    // boundary information.
    //
    bool didGhosts = false;

    avtGhostDataType ghostType = spec->GetDesiredGhostDataType();
    if (shouldDoMatSelect)
        ghostType = GHOST_ZONE_DATA;

    bool ghostDataIsNeeded = false;
    if (ghostType != NO_GHOST_DATA)
        ghostDataIsNeeded = true;

    bool canCreateGhostData = true;
    if (spec->MustMaintainOriginalConnectivity())
        canCreateGhostData = false;

    if (canCreateGhostData && ghostDataIsNeeded)
    {
        didGhosts = CommunicateGhosts(ghostType, datasetCollection, domains, 
                                      spec, src, allDomains, 
                                      canDoCollectiveCommunication);
    }

    //
    // Finally, do the material selection.
    //
    MaterialSelect(datasetCollection, domains, spec, src, didGhosts);

    //
    // Now make something that AVT will understand downstream.
    //
    avtDataTree_p rv = datasetCollection.AssembleDataTree(domains);
  
    char str[1024];
    sprintf(str, "Getting dataset for %s", spec->GetVariable());
    visitTimer->StopTimer(timerHandle, str);
    visitTimer->DumpTimings();

    //
    // As we have actually read the data, our opinion on what the data looked
    // like may have changed slightly (we may have decided it had ghost zones
    // for example), so call this again.
    //
    avtDataObject_p dob = src->GetOutput();
    if (nDomains == 0)
        dob->GetInfo().GetValidity().SetHasEverOwnedAnyDomain(false);
    PopulateDataObjectInformation(dob, spec->GetVariable(), timeStep, 
        selectionsApplied, spec);

    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::UpdateInternalState
//
//  Purpose:
//      Do some internal bookkeeping.  This is mostly to handle current
//      timestep information.
//
//  Arguments:
//      ts      The incoming timestep.
//
//  Programmer: Hank Childs
//  Creation:   October 26, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Jan  7 17:34:00 PST 2002 
//    Clear out the cache when the timestep changes.
//
// ****************************************************************************

void
avtGenericDatabase::UpdateInternalState(int ts)
{
    //
    // We will run out of file descriptors if we are not careful!
    //
    if (ts != lastTimestep)
    {
        if (lastTimestep != -1)
        {
            int allDomains = -1;
            debug4 << "Generic database freeing up resources "
                   << " (file descriptors, memory) for timestep "
                   << lastTimestep << endl;
            Interface->FreeUpResources(lastTimestep, allDomains);
            cache.ClearTimestep(lastTimestep);
        }
    }
    lastTimestep = ts;
}


// ****************************************************************************
//  Method: avtGenericDatabase::FreeUpResources
//
//  Purpose:
//      Release any system resources that we're using.
//
//  Programmer: Sean Ahern
//  Creation:   Tue May 21 12:00:49 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Fri Mar 11 11:24:25 PST 2005
//    Free up the memory references to non-cachable variables.
//
// ****************************************************************************

void
avtGenericDatabase::FreeUpResources(void)
{
    debug4 << "Generic database freeing up resources (file descriptors, "
           << "memory)" << endl;
    Interface->FreeUpResources(-1, -1);
    cache.ClearTimestep(lastTimestep);
    ManageMemoryForNonCachableVar(NULL);
    ManageMemoryForNonCachableMesh(NULL);
}


// ****************************************************************************
//  Method: avtGenericDatabase::ManageMemoryForNonCachableVar
//
//  Purpose:
//      Some variables are non-cachable.  But we own their memory and want
//      to do bookkeeping right away.  This is a spot where you can register
//      a variable and it will be kept until the method is called again.
//      It is assumed that by this time the variable has been associated with
//      a data set that now owns it.
//
//  Programmer: Hank Childs
//  Creation:   March 11, 2005
//
// ****************************************************************************

void
avtGenericDatabase::ManageMemoryForNonCachableVar(vtkDataArray *v)
{
    if (nonCachableVariableReference != NULL)
    {
        nonCachableVariableReference->Delete();
        nonCachableVariableReference = NULL;
    }
    nonCachableVariableReference = v;
    if (nonCachableVariableReference != NULL)
        nonCachableVariableReference->Register(NULL);
}
 

// ****************************************************************************
//  Method: avtGenericDatabase::ManageMemoryForNonCachableMesh
//
//  Purpose:
//      Some meshes are non-cachable.  But we own their memory and want
//      to do bookkeeping right away.  This is a spot where you can register
//      a mesh and it will be kept until the method is called again.
//      It is assumed that by this time the variable has been associated with
//      a data set that now owns it.
//
//  Programmer: Hank Childs
//  Creation:   March 11, 2005
//
// ****************************************************************************

void
avtGenericDatabase::ManageMemoryForNonCachableMesh(vtkDataSet *v)
{
    if (nonCachableMeshReference != NULL)
    {
        nonCachableMeshReference->Delete();
        nonCachableMeshReference = NULL;
    }
    nonCachableMeshReference = v;
    if (nonCachableMeshReference != NULL)
        nonCachableMeshReference->Register(NULL);
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetDataset
//
//  Purpose:
//      Determines what the dataset type is requested (scalar var, material,
//      etc.) and calls the appropriate routine.
//
//  Arguments:
//      varname      The variable for that domain.
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      matname      The material of interest (may be _all).
//      vars2nd      The list of secondary variables.
//      src          The source from a database.
//
//  Returns:         The dataset for that block.
//
//  Programmer:  Hank Childs
//  Creation:    November 7, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Mar  1 09:20:52 PST 2001
//    Moved from avtDatabase to avtGenericDatabase.
//
//    Hank Childs, Mon Mar 19 16:35:56 PST 2001
//    Added logic for vector datasets.
//
//    Hank Childs, Tue Mar 27 08:59:23 PST 2001 
//    Added logic for mesh datasets.
//
//    Hank Childs, Fri Oct  5 15:52:13 PDT 2001
//    Added a argument for the database source.  Also an argument for the
//    material name.
//
//    Hank Childs, Tue Oct 23 09:18:43 PDT 2001
//    Added support for secondary variables.
//
//    Hank Childs, Mon Nov  5 16:54:18 PST 2001
//    Extend support for secondary variables to include vectors.
//
//    Hank Childs, Fri Aug  1 21:49:01 PDT 2003
//    Treat curves like meshes.
//
//    Hank Childs, Mon Sep 22 07:48:34 PDT 2003
//    Added support for tensors.
//
//    Brad Whitlock, Sat Apr 2 00:46:37 PDT 2005
//    Added support for labels.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added support for data type conversion
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetDataset(const char *varname, int ts, int domain,
                        const char *matname, const vector<CharStrRef> &vars2nd, 
                        avtSourceFromDatabase *src)
{
    vtkDataSet *rv = NULL;
    avtVarType type = GetMetaData(ts)->DetermineVarType(varname);

    // get information to control data type 
    const avtDataSpecification_p dspec = src->GetFullDataSpecification();
    bool needNativePrecision = dspec->NeedNativePrecision();
    vector<int> admissibleDataTypes = dspec->GetAdmissibleDataTypes();

    if (strcmp(matname, "_all") == 0)
    {
        Interface->TurnMaterialSelectionOff();
    }
    else
    {
        Interface->TurnMaterialSelectionOn(matname);
    }

    switch (type)
    {
      case AVT_SCALAR_VAR:
        rv = GetScalarVarDataset(varname, ts, domain, matname,
                 needNativePrecision, admissibleDataTypes);
        break;

      case AVT_VECTOR_VAR:
        rv = GetVectorVarDataset(varname, ts, domain, matname,
                 needNativePrecision, admissibleDataTypes);
        break;

      case AVT_TENSOR_VAR:
        rv = GetTensorVarDataset(varname, ts, domain, matname,
                 needNativePrecision, admissibleDataTypes);
        break;

      case AVT_SYMMETRIC_TENSOR_VAR:
        rv = GetSymmetricTensorVarDataset(varname, ts, domain, matname,
                 needNativePrecision, admissibleDataTypes);
        break;

      case AVT_LABEL_VAR:
        rv = GetLabelVarDataset(varname, ts, domain, matname,
                 needNativePrecision, admissibleDataTypes);
        break;

      case AVT_MATERIAL:
        rv = GetMaterialDataset(varname, ts, domain, matname,
                 needNativePrecision, admissibleDataTypes);
        break;

      case AVT_MESH:
      case AVT_CURVE:
        rv = GetMeshDataset(varname, ts, domain, matname,
                 needNativePrecision, admissibleDataTypes);
        break;

      case AVT_MATSPECIES:
        rv = GetSpeciesDataset(varname, ts, domain, matname,
                 needNativePrecision, admissibleDataTypes);
        break;

      default:
        EXCEPTION1(InvalidVariableException, varname);
    }

    if (rv != NULL && vars2nd.size() > 0)
    {
        AddSecondaryVariables(rv, ts, domain, matname, vars2nd,
            needNativePrecision, admissibleDataTypes);
    }

    Interface->TurnMaterialSelectionOff();

    //
    // Make sure that every domain we read has its extents merged into the
    // extents for all the dataset.
    //
    src->MergeExtents(rv);

    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetScalarVarDataset
//
//  Purpose:
//      Constructs a dataset for a scalar variable.
//
//  Arguments:
//      varname      The variable for that domain.
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//
//  Returns:         The dataset for that block.
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Mar  1 09:20:52 PST 2001
//    Moved from avtDatabase to avtGenericDatabase.
//
//    Hank Childs, Mon Oct  8 10:54:30 PDT 2001
//    Added material argument.
//
//    Hank Childs, Thu Oct 11 13:04:25 PDT 2001
//    Added support for NULL meshes.
//
//    Hank Childs, Tue Oct 23 09:18:43 PDT 2001
//    Added support for secondary variables.
//
//    Hank Childs, Thu Oct 25 15:26:08 PDT 2001
//    Allow for file formats that don't have domains.
//
//    Hank Childs, Mon Nov  5 16:55:49 PST 2001
//    Remove support of secondary variables in this routine in favor of a more
//    general routine that supports vectors.
//
//    Kathleen Bonnell, Fri Feb  8 08:48:06 PST 2002 
//    vtkScalars has been deprecated in VTK 4.0.  Use vtkDataArray instead.
//
//    Jeremy Meredith, Wed Mar 19 12:22:20 PST 2003
//    Allow for a NULL var as well as a NULL mesh.
//
//    Mark C. Miller, Mon Jan 10 14:06:06 PST 2005
//    Changed order of retrieval of mesh and variable to get the mesh
//    first and then the variable. This is so plugin can handle things
//    like removal of arb-poly zones correctly.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added support for data type conversion
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetScalarVarDataset(const char *varname, int ts,
                                        int domain, const char *material,
                                        const bool needNativePrecision,
                                        const vector<int>& admissibleDataTypes)
{
    const avtScalarMetaData *smd = GetMetaData(ts)->GetScalar(varname);
    if (smd == NULL)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    string meshname  = GetMetaData(ts)->MeshForVar(varname);
    vtkDataSet *mesh = GetMesh(meshname.c_str(), ts, domain, material,
                               needNativePrecision, admissibleDataTypes);

    if (mesh == NULL)
    {
        //
        // Some file formats don't have a mesh for every domain (like Exodus
        // when material selection is applied).  Just propagate the NULL up.  
        //
        return NULL;
    }

    vtkDataArray *var  = GetScalarVariable(varname, ts, domain, material,
                             needNativePrecision, admissibleDataTypes);

    if (var == NULL)
    {
        //
        // Some variables don't have a var for every domain, even if the
        // mesh exists there.  Just propagate the NULL up.  
        //
        return NULL;
    }


    //
    // Set up the scalar var's name in case we have more than one.
    //
    var->SetName(varname);

    if (smd->centering == AVT_NODECENT)
    {
        mesh->GetPointData()->SetScalars(var);
    }
    else
    {
        mesh->GetCellData()->SetScalars(var);
    }

    return mesh;
}


// ****************************************************************************
//  Method: avtGenericDatabase::AddSecondaryVariables
//
//  Purpose:
//      Adds the secondary variables to the dataset.
//
//  Arguments:
//      ds           The vtk dataset to add to.
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//      vars2nd      A list of secondary variable to add.
//
//  Programmer: Hank Childs
//  Creation:   November 5, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Feb  8 08:48:06 PST 2002
//    vtkScalars has been deprecated in VTK 4.0.  Use vtkDataArray instead.
//
//    Hank Childs, Tue Sep 17 09:50:24 PDT 2002
//    Add support for species variables.
//
//    Hank Childs, Fri Jul 25 10:49:56 PDT 2003
//    Allow for meshes to be specified (then ignored).
//
//    Hank Childs, Thu Aug 21 11:01:22 PDT 2003
//    Allow for materials to be specified (then ignored).
//
//    Hank Childs, Sat Oct 18 10:09:40 PDT 2003
//    Add support for tensors.
//
//    Brad Whitlock, Sat Apr 2 00:46:56 PDT 2005
//    Added support for labels.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added support for data type conversion
//
// ****************************************************************************

void
avtGenericDatabase::AddSecondaryVariables(vtkDataSet *ds, int ts, int domain,
                  const char *material, const vector<CharStrRef> &vars2nd,
                  const bool needNativePrecision,
                  const vector<int>& admissibleDataTypes)
{
    int nzones = ds->GetNumberOfCells();

    //
    // If we have any secondary arrays, then fetch those as well.
    //
    int num2ndVars = vars2nd.size();
    for (int i = 0 ; i < num2ndVars ; i++)
    {
        const char *varName = *(vars2nd[i]);
        avtDatabaseMetaData *md = GetMetaData(ts);
        avtVarType vt = md->DetermineVarType(varName);

        //
        // They asked for the mesh as a secondary variable.  Just ignore this
        // request -- it is likely from the expression code.
        //
        if (vt == AVT_MESH || vt == AVT_MATERIAL)
            continue;

        //
        // Do some preparation.  Decide the variable type and
        // if it is node centered or zone centered.
        //
        vtkDataSetAttributes *atts = NULL;
        switch (vt)
        {
          case AVT_SCALAR_VAR:
            {
                const avtScalarMetaData *smd=GetMetaData(ts)->GetScalar(varName);
                if (smd->centering == AVT_NODECENT)
                {
                    atts = ds->GetPointData();
                }
                else
                {
                    atts = ds->GetCellData();
                }
            }
            break;

          case AVT_SYMMETRIC_TENSOR_VAR:
            {
                const avtSymmetricTensorMetaData *vmd = 
                                       GetMetaData(ts)->GetSymmTensor(varName);
                if (vmd->centering == AVT_NODECENT)
                {
                    atts = ds->GetPointData();
                }
                else
                {
                    atts = ds->GetCellData();
                }
            }
            break;

          case AVT_TENSOR_VAR:
            {
                const avtTensorMetaData *vmd = 
                                           GetMetaData(ts)->GetTensor(varName);
                if (vmd->centering == AVT_NODECENT)
                {
                    atts = ds->GetPointData();
                }
                else
                {
                    atts = ds->GetCellData();
                }
            }
            break;

          case AVT_VECTOR_VAR:
            {
                const avtVectorMetaData *vmd = 
                                           GetMetaData(ts)->GetVector(varName);
                if (vmd->centering == AVT_NODECENT)
                {
                    atts = ds->GetPointData();
                }
                else
                {
                    atts = ds->GetCellData();
                }
            }
            break;

          case AVT_LABEL_VAR:
            {
                const avtLabelMetaData *lmd=GetMetaData(ts)->GetLabel(varName);
                if (lmd->centering == AVT_NODECENT)
                {
                    atts = ds->GetPointData();
                }
                else
                {
                    atts = ds->GetCellData();
                }
            }
            break;

          case AVT_MATSPECIES:
            atts = ds->GetCellData();
            break;

          default:
            EXCEPTION1(InvalidVariableException, varName);
        }

        //
        // Okay, now get the variable and add them to the dataset.
        //
        vtkDataArray *dat = NULL;
        switch (vt)
        {
          case AVT_SCALAR_VAR:
            dat = GetScalarVariable(varName, ts, domain, material,
                      needNativePrecision, admissibleDataTypes);
            break;
          case AVT_VECTOR_VAR:
            dat = GetVectorVariable(varName, ts, domain, material,
                      needNativePrecision, admissibleDataTypes);
            break;
          case AVT_TENSOR_VAR:
            dat = GetTensorVariable(varName, ts, domain, material,
                      needNativePrecision, admissibleDataTypes);
            break;
          case AVT_SYMMETRIC_TENSOR_VAR:
            dat = GetSymmetricTensorVariable(varName, ts, domain, material,
                      needNativePrecision, admissibleDataTypes);
            break;
          case AVT_LABEL_VAR:
            dat = GetLabelVariable(varName, ts, domain, material);
            break;
          case AVT_MATSPECIES:
            dat = GetSpeciesVariable(varName, ts, domain, material, nzones);
            break;
          default:
            EXCEPTION1(InvalidVariableException, varName);
        }
           
        dat->SetName(varName);
        atts->AddArray(dat);
    }
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetMeshDataset
//
//  Purpose:
//      Constructs a dataset for a mesh variable.
//
//  Arguments:
//      varname      The variable for that domain.
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//
//  Returns:         The dataset for that block.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 22, 2001 
//
//  Modifications:
// 
//    Kathleen Bonnell, Fri Jun 15 11:34:26 PDT 2001
//    Call method AddOriginalCellsArray.
//
//    Hank Childs, Mon Oct  8 10:54:30 PDT 2001
//    Added material argument.
//
//    Hank Childs, Thu Oct 11 13:04:25 PDT 2001
//    Added support for NULL meshes.
//
//    Kathleen Bonnell, Mon Mar 25 17:30:07 PST 2002     
//    Removed call to AddOriginalCellsArray.  Instead made the
//    mesh filter set the necessary flag in the dataspecification,
//    so the method is only called from one place. 
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added support for data type conversion
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetMeshDataset(const char *varname, int ts, int domain,
                                   const char *material,
                                   const bool needNativePrecision,
                                   const vector<int>& admissibleDataTypes)
{
    string meshname  = GetMetaData(ts)->MeshForVar(varname);
    vtkDataSet *mesh = GetMesh(meshname.c_str(), ts, domain, material,
                           needNativePrecision, admissibleDataTypes);

    return mesh;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetVectorVarDataset
//
//  Purpose:
//      Constructs a dataset for a vector variable.
//
//  Arguments:
//      varname      The variable for that domain.
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//
//  Returns:         The dataset for that block.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Oct  8 10:54:30 PDT 2001
//    Added material argument.
//
//    Hank Childs, Thu Oct 11 13:04:25 PDT 2001
//    Added support for NULL meshes.
//
//    Hank Childs, Thu Jan  3 10:29:02 PST 2002
//    Name the vector so we can reference it later.
//
//    Kathleen Bonnell, Fri Feb  8 08:48:06 PST 2002 
//    vtkVectors has been deprecated in VTK 4.0.  Use vtkDataArray instead.
//
//    Jeremy Meredith, Wed Mar 19 12:22:20 PST 2003
//    Allow for a NULL var as well as a NULL mesh.
//
//    Jeremy Meredith, Fri Jun 18 14:18:48 PDT 2004
//    Allow for non-3-component vectors.  This will be used in the short
//    term for colors, though we should have a better solution when colors
//    are no longer categorized as vectors.
//
//    Mark C. Miller, Mon Jan 10 14:06:06 PST 2005
//    Changed order of retrieval of mesh and variable to get the mesh
//    first and then the variable. This is so plugin can handle things
//    like removal of arb-poly zones correctly.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added support for data type conversion
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetVectorVarDataset(const char *varname, int ts,
                                        int domain, const char *material,
                                        const bool needNativePrecision,
                                        const vector<int>& admissibleDataTypes)
{
    const avtVectorMetaData *vmd = GetMetaData(ts)->GetVector(varname);
    if (vmd == NULL)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    string meshname  = GetMetaData(ts)->MeshForVar(varname);
    vtkDataSet *mesh = GetMesh(meshname.c_str(), ts, domain, material,
                            needNativePrecision, admissibleDataTypes);

    if (mesh == NULL)
    {
        //
        // Some file formats don't have a mesh for every domain (like Exodus
        // when material selection is applied).  Just propagate the NULL up.  
        //
        return NULL;
    }

    vtkDataArray *var = GetVectorVariable(varname, ts, domain, material,
                            needNativePrecision, admissibleDataTypes);

    if (var == NULL)
    {
        //
        // Some variables don't have a var for every domain, even if the
        // mesh exists there.  Just propagate the NULL up.  
        //
        return NULL;
    }

    //
    // Set up the vector var's name in case we have more than one.
    //
    var->SetName(varname);

    if (vmd->centering == AVT_NODECENT)
    {
        if (var->GetNumberOfComponents() == 3)
        {
            mesh->GetPointData()->SetVectors(var);
        }
        else
        {
            mesh->GetPointData()->AddArray(var);
        }
    }
    else
    {
        if (var->GetNumberOfComponents() == 3)
        {
            mesh->GetCellData()->SetVectors(var);
        }
        else
        {
            mesh->GetCellData()->AddArray(var);
        }
    }

    return mesh;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetTensorVarDataset
//
//  Purpose:
//      Constructs a dataset for a tensor variable.
//
//  Arguments:
//      varname      The variable for that domain.
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//
//  Returns:         The dataset for that block.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 10 14:06:06 PST 2005
//    Changed order of retrieval of mesh and variable to get the mesh
//    first and then the variable. This is so plugin can handle things
//    like removal of arb-poly zones correctly.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added support for data type conversion
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetTensorVarDataset(const char *varname, int ts,
                                        int domain, const char *material,
                                        const bool needNativePrecision,
                                        const vector<int>& admissibleDataTypes)
{
    const avtTensorMetaData *tmd = GetMetaData(ts)->GetTensor(varname);
    if (tmd == NULL)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    string meshname  = GetMetaData(ts)->MeshForVar(varname);
    vtkDataSet *mesh = GetMesh(meshname.c_str(), ts, domain, material,
                            needNativePrecision, admissibleDataTypes);

    if (mesh == NULL)
    {
        //
        // Some file formats don't have a mesh for every domain (like Exodus
        // when material selection is applied).  Just propagate the NULL up.  
        //
        return NULL;
    }

    vtkDataArray *var = GetTensorVariable(varname, ts, domain, material,
                            needNativePrecision, admissibleDataTypes);

    if (var == NULL)
    {
        //
        // Some variables don't have a var for every domain, even if the
        // mesh exists there.  Just propagate the NULL up.  
        //
        return NULL;
    }

    //
    // Set up the tensor var's name in case we have more than one.
    //
    var->SetName(varname);

    if (tmd->centering == AVT_NODECENT)
    {
        mesh->GetPointData()->SetTensors(var);
    }
    else
    {
        mesh->GetCellData()->SetTensors(var);
    }

    return mesh;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetSymmetricTensorVarDataset
//
//  Purpose:
//      Constructs a dataset for a symmetric tensor variable.
//
//  Arguments:
//      varname      The variable for that domain.
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//
//  Returns:         The dataset for that block.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 10 14:06:06 PST 2005
//    Changed order of retrieval of mesh and variable to get the mesh
//    first and then the variable. This is so plugin can handle things
//    like removal of arb-poly zones correctly.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added support for data type conversion
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetSymmetricTensorVarDataset(const char *varname, int ts,
                                        int domain, const char *material,
                                        const bool needNativePrecision,
                                        const vector<int>& admissibleDataTypes)
{
    const avtSymmetricTensorMetaData *tmd = 
                                         GetMetaData(ts)->GetSymmTensor(varname);
    if (tmd == NULL)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    string meshname  = GetMetaData(ts)->MeshForVar(varname);
    vtkDataSet *mesh = GetMesh(meshname.c_str(), ts, domain, material,
                               needNativePrecision, admissibleDataTypes);

    if (mesh == NULL)
    {
        //
        // Some file formats don't have a mesh for every domain (like Exodus
        // when material selection is applied).  Just propagate the NULL up.  
        //
        return NULL;
    }

    vtkDataArray *var = GetSymmetricTensorVariable(varname, ts, domain,
                                                   material,
                                                   needNativePrecision,
                                                   admissibleDataTypes);

    if (var == NULL)
    {
        //
        // Some variables don't have a var for every domain, even if the
        // mesh exists there.  Just propagate the NULL up.  
        //
        return NULL;
    }

    //
    // Set up the tensor var's name in case we have more than one.
    //
    var->SetName(varname);

    if (tmd->centering == AVT_NODECENT)
    {
        mesh->GetPointData()->SetTensors(var);
    }
    else
    {
        mesh->GetCellData()->SetTensors(var);
    }

    return mesh;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetMaterialDataset
//
//  Purpose:
//      Constructs a dataset for a material.  This is only the mesh, as the
//      material is retrieved through the GetAuxiliaryData mechanism.
//
//  Arguments:
//      matname      The material for that domain (like "mat1").
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//
//  Returns:         The dataset for that block.
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Mar  1 09:20:52 PST 2001
//    Moved from avtDatabase to avtGenericDatabase.
//
//    Eric Brugger, Thu Sep 20 10:12:56 PDT 2001
//    I modified the routine to return a non material selected mesh, since
//    that occurs later.
//
//    Hank Childs, Mon Oct  8 10:54:30 PDT 2001
//    Added material argument.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added support for data type conversion
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetMaterialDataset(const char *matname, int ts, int domain,
                                       const char *material,
                                       const bool needNativePrecision,
                                       const vector<int>& admissibleDataTypes)
{
    string meshname  = GetMetaData(ts)->MeshForVar(matname);
    vtkDataSet *mesh = GetMesh(meshname.c_str(), ts, domain, material,
                               needNativePrecision, admissibleDataTypes);

    return mesh;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetSpeciesDataset
//
//  Purpose:
//      Constructs a dataset for a species.  This is only the mesh, as the
//      species is retrieved through the GetAuxiliaryData mechanism.
//
//  Arguments:
//      specname     The species for that domain (like "spec1").
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//
//  Returns:         The dataset for that block.
//
//  Programmer: Hank Childs and Jeremy Meredith
//  Creation:   December 19, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Feb  8 08:48:06 PST 2002
//    vtkScalars has been deprecated in VTK 4.0.  Use vtkDataArray and 
//    vtkFloatArray instead.
//
//    Hank Childs, Thu Jul  4 13:07:23 PDT 2002
//    Add a name to the mixed variable.
//    
//    Hank Childs, Tue Sep 17 09:50:24 PDT 2002
//    Create the species variable in its own subroutine.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added support for data type conversion
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetSpeciesDataset(const char *specname, int ts, int domain,
                                      const char *material,
                                      const bool needNativePrecision,
                                      const vector<int>& admissibleDataTypes)
{
    string meshname  = GetMetaData(ts)->MeshForVar(specname);
    vtkDataSet *mesh = GetMesh(meshname.c_str(), ts, domain, material,
                               needNativePrecision, admissibleDataTypes);

    if (mesh == NULL)
    {
        //
        // Some file formats don't have a mesh for every domain (like Exodus
        // when material selection is applied).  Just propagate the NULL up.  
        //
        return NULL;
    }

    vtkDataArray *spec = GetSpeciesVariable(specname, ts, domain, material,
                                            mesh->GetNumberOfCells());
    mesh->GetCellData()->SetScalars(spec);
    spec->Delete();
    
    return mesh;
}

// ****************************************************************************
// Method: avtGenericDatabase::GetLabelVarDataset
//
// Purpose: 
//   Gets a label variable and its mesh.
//
// Arguments:
//      varname      The variable for that domain
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//
// Returns:    A vtkDataSet containing the data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Sat Apr 2 00:38:47 PDT 2005
//
// Modifications:
//   
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added support for data type conversion
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetLabelVarDataset(const char *varname, int ts,
                                       int domain, const char *material,
                                       const bool needNativePrecision,
                                       const vector<int>& admissibleDataTypes)
{
    const avtLabelMetaData *lmd = GetMetaData(ts)->GetLabel(varname);
    if (lmd == NULL)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    string meshname  = GetMetaData(ts)->MeshForVar(varname);
    vtkDataSet *mesh = GetMesh(meshname.c_str(), ts, domain, material,
                               needNativePrecision, admissibleDataTypes);

    if (mesh == NULL)
    {
        //
        // Some file formats don't have a mesh for every domain (like Exodus
        // when material selection is applied).  Just propagate the NULL up.  
        //
        return NULL;
    }

    vtkDataArray *var = GetLabelVariable(varname, ts, domain, material);

    if (var == NULL)
    {
        //
        // Some variables don't have a var for every domain, even if the
        // mesh exists there.  Just propagate the NULL up.  
        //
        return NULL;
    }


    //
    // Set up the scalar var's name in case we have more than one.
    //
    var->SetName(varname);

    if (lmd->centering == AVT_NODECENT)
    {
        mesh->GetPointData()->SetScalars(var);
    }
    else
    {
        mesh->GetCellData()->SetScalars(var);
    }

    //
    // Add some field data that indicates this dataset is a set of labels.
    //
    vtkUnsignedIntArray *lvs = vtkUnsignedIntArray::New();
    lvs->SetNumberOfTuples(1);
    lvs->SetName("avtLabelVariableSize");
    unsigned int *uintptr = (unsigned int *)lvs->GetVoidPointer(0);
    *uintptr = var->GetNumberOfComponents();
    mesh->GetFieldData()->AddArray(lvs);

    return mesh;
}

// ****************************************************************************
//  Method: avtGenericDatabase::GetSpeciesVariable
//
//  Purpose:
//      Creates a zonal variable that serves as the species.  Also sets up
//      a mixed variable for use with material selection.
//
//  Arguments:
//      specname    The name of the species.
//      ts          The timestep.
//      domain      The domain number.
//      material    The name of the associated material.
//      nzones      The number of zones in the mesh for this domain.
//
//  Returrns:   An array (of all 1.'s) that is the species.
//
//  Notes:      The calling routine must de-reference the returned array.
//
//  Programmer: Hank Childs
//  Creation:   September 17, 2002
//
// ****************************************************************************

vtkDataArray *
avtGenericDatabase::GetSpeciesVariable(const char *specname, int ts, 
                                  int domain, const char *material, int nzones)
{
    int i;

    vtkFloatArray *allOnes = vtkFloatArray::New();
    allOnes->SetNumberOfTuples(nzones);
    float *ptr = allOnes->GetPointer(0);
    for (i = 0 ; i < nzones ; i++)
    {
        *ptr = 1.;
        ptr++;
    }
    allOnes->SetName(specname);

    //
    // Now create a mixed variable.  We should probably be caching this so we
    // don't generate every time we ask for this dataset.
    //
    avtMaterial *mat = GetMaterial(domain, specname, ts);
    int mixlen = mat->GetMixlen();
    float *mixvarbuff = new float[mixlen];
    for (i = 0 ; i < mixlen ; i++)
        mixvarbuff[i] = 1.;
    avtMixedVariable *mixvar = new avtMixedVariable(mixvarbuff, mixlen, 
                                                    specname);
    delete [] mixvarbuff;
    
    //
    // After this routine is called, the main routine will look for mixed
    // variables in the cache.  So: all we have to do is put this mixed
    // variable in the cache and we can forget about it.
    //
    void_ref_ptr vr = void_ref_ptr(mixvar, avtMixedVariable::Destruct);
    cache.CacheVoidRef(specname, AUXILIARY_DATA_MIXED_VARIABLE, ts, domain,vr);

    return allOnes;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetScalarVariable
//
//  Purpose:
//      Checks to see if a variable is already in cache and fetches it if it
//      is not.
//
//  Arguments:
//      varname    The name of the variable.
//      ts         The timestep for the variable.
//      domain     The domain for the variable.
//      material   The name of the material we are getting.
//
//  Returns:    A vtkScalars for the variable.
//
//  Programmer: Hank Childs
//  Creation:   October 31, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Mar  1 09:20:52 PST 2001
//    Moved from avtDatabase to avtGenericDatabase.
//
//    Hank Childs, Mon Oct  8 10:54:30 PDT 2001
//    Added material argument.
//
//    Kathleen Bonnell, Fri Feb  8 08:48:06 PST 2002 
//    vtkScalars has been deprecated in VTK 4.0.  Use vtkDataArray instead.
//
//    Hank Childs, Fri Mar 14 20:56:40 PST 2003
//    Turned off caching in some instances.
//
//    Hank Childs, Tue Feb 15 07:21:10 PST 2005
//    Make translations when we have hidden characters.
//
//    Hank Childs, Fri Mar 11 11:24:25 PST 2005
//    Fix memory leak.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added support for data type conversion
//
// ****************************************************************************

vtkDataArray *
avtGenericDatabase::GetScalarVariable(const char *varname, int ts, int domain,
                                      const char *material,
                                      const bool needNativePrecision,
                                      const vector<int>& admissibleDataTypes)
{

    //
    // We have to be leery about doing any caching when the variables are
    // defined on sub-meshes.  This is because if we add new secondary
    // variables, some parts of the mesh may drop out, and the size of the
    // cached objects are now wrong.
    //
    vtkDataArray *var = NULL;
    if (!Interface->HasVarsDefinedOnSubMeshes())
    {
        //
        // First, look for native precision data in the cache
        //
        var = (vtkDataArray *) cache.GetVTKObject(varname,
                         avtVariableCache::NATIVE_SCALARS_NAME,
                         ts, domain, material);

        if ((var != NULL) &&
            IsAdmissibleDataType(admissibleDataTypes, var->GetDataType())) 
        {
            if (!needNativePrecision &&
                (PrecisionInBytes(var) > sizeof(float)))
            {
                var = (vtkDataArray *) cache.GetVTKObject(varname,
                                 avtVariableCache::SCALARS_NAME,
                                 ts, domain, material);

                if (var == NULL)
                {
                    //
                    // At this point, we know we've got ONLY native data
                    // in the cache but either visit is NOT requesting
                    // native data or the native precision is greater
                    // than float. So, even though the pipeline supports
                    // this data, we're going to convert because there
                    // is no value added in running with higher precision
                    // when it is not necessary. After we've converted
                    // we'll cache the converted result.
                    //
                    var = (vtkDataArray *) cache.GetVTKObject(varname,
                        avtVariableCache::NATIVE_SCALARS_NAME,
                        ts, domain, material);
                    vtkDataArray *var1 = ConvertDataArrayToFloat(var);
                    if (var1)
                    {
                        cache.CacheVTKObject(varname,
                            avtVariableCache::SCALARS_NAME,
                            ts, domain, material, var1);
                        var1->Delete();
                        var = var1;
                    }
                }
            }
        }
        else
        {
            var = (vtkDataArray *) cache.GetVTKObject(varname,
                             avtVariableCache::SCALARS_NAME,
                             ts, domain, material);
        }
    }

    //
    // Translate the variable name into something the interface understands.
    // Note: use the "real_varname" when talking to the Interface, but use
    // the standard "varname" for caching and all other places.
    //
    const avtScalarMetaData *smd = GetMetaData(ts)->GetScalar(varname);
    if (smd == NULL)
        EXCEPTION1(InvalidVariableException, varname);
    const char *real_varname = varname;
    if (smd->originalName != smd->name && smd->originalName != "")
    {
        real_varname = smd->originalName.c_str();
    }

    if (var == NULL)
    {
        //
        // We haven't read in this domain before, so fetch it from the files.
        //
        var = Interface->GetVar(ts, domain, real_varname);

        if (var != NULL)
        {
            //
            // Convert BEFORE caching only if native precision is greater
            // than float and VisIt has NOT asked for native precision
            //
            const char *cacheTypeName =
                var->GetDataType() == VTK_FLOAT ?
                avtVariableCache::SCALARS_NAME :
                avtVariableCache::NATIVE_SCALARS_NAME;
            if (PrecisionInBytes(var) > sizeof(float) && !needNativePrecision)
            {
                vtkDataArray *var1 = ConvertDataArrayToFloat(var);
                if (var1)
                {
                    var->Delete();
                    var = var1;
                    cacheTypeName = avtVariableCache::SCALARS_NAME;
                }
            }

            //
            // Cache the variable if we can
            //
            if (Interface->CanCacheVariable(real_varname))
            {
                cache.CacheVTKObject(varname, cacheTypeName, ts, domain,
                                     material, var);
            }
            else
            {
                ManageMemoryForNonCachableVar(var);
            }

            //
            // We need to decrement the reference count of the variable 
            // returned from FetchVar, but we could not do it previously 
            // because it would knock the count down to 0 and delete it.
            // Since we have cached it, we can do it now.
            //
            var->Delete();

            //
            // Convert BEFORE returning to VisIt only if precision in 
            // cache is NOT supported by current pipeline. Also, if we
            // convert here, go ahead and cache the converted result too.
            //
            if (!IsAdmissibleDataType(admissibleDataTypes, var->GetDataType()))
            {
                vtkDataArray *var1 = ConvertDataArrayToFloat(var);

                if (var1)
                {
                    cacheTypeName = avtVariableCache::SCALARS_NAME;

                    if (Interface->CanCacheVariable(real_varname))
                    {
                        cache.CacheVTKObject(varname, cacheTypeName, ts, domain,
                                             material, var1);
                    }
                    else
                    {
                        ManageMemoryForNonCachableVar(var1);
                    }
                    var1->Delete();
                    var = var1;
                }
            }
        }
    }

    return var;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetVectorVariable
//
//  Purpose:
//      Checks to see if a variable is already in cache and fetches it if it
//      is not.
//
//  Arguments:
//      varname    The name of the variable.
//      ts         The timestep for the variable.
//      domain     The domain for the variable.
//      material   The name of the material we are getting.
//
//  Returns:    A vtkDataArray for the variable.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Oct  8 10:54:30 PDT 2001
//    Added material argument.
//
//    Kathleen Bonnell, Fri Feb  8 08:48:06 PST 2002 
//    vtkVectors has been deprecated in VTK 4.0.  Use vtkDataArray instead.
//
//    Hank Childs, Fri Mar 14 20:56:40 PST 2003
//    Turned off caching in some instances.
//
//    Hank Childs, Tue Feb 15 07:21:10 PST 2005
//    Make translations when we have hidden characters.
//
//    Hank Childs, Fri Mar 11 11:24:25 PST 2005
//    Fix memory leak.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added support for data type conversion
//
// ****************************************************************************

vtkDataArray *
avtGenericDatabase::GetVectorVariable(const char *varname, int ts, int domain,
                                      const char *material,
                                      const bool needNativePrecision,
                                      const vector<int>& admissibleDataTypes)
{
    //
    // We have to be leery about doing any caching when the variables are
    // defined on sub-meshes.  This is because if we add new secondary
    // variables, some parts of the mesh may drop out, and the size of the
    // cached objects are now wrong.
    //
    vtkDataArray *var = NULL;
    if (!Interface->HasVarsDefinedOnSubMeshes())
    {
        //
        // First, look for native precision data in the cache
        //
        var = (vtkDataArray *) cache.GetVTKObject(varname,
                         avtVariableCache::NATIVE_VECTORS_NAME,
                         ts, domain, material);

        if ((var != NULL) &&
            IsAdmissibleDataType(admissibleDataTypes, var->GetDataType()))
        {
            if (!needNativePrecision &&
                (PrecisionInBytes(var) > sizeof(float)))
            {
                var = (vtkDataArray *) cache.GetVTKObject(varname,
                                 avtVariableCache::VECTORS_NAME,
                                 ts, domain, material);

                if (var == NULL)
                {
                    //
                    // At this point, we know we've got ONLY native data
                    // in the cache but either visit is NOT requesting
                    // native data or the native precision is greater
                    // than float. So, even though the pipeline supports
                    // this data, we're going to convert because there
                    // is no value added in running with higher precision
                    // when it is not necessary. After we've converted
                    // we'll cache the converted result.
                    //
                    var = (vtkDataArray *) cache.GetVTKObject(varname,
                         avtVariableCache::NATIVE_VECTORS_NAME,
                         ts, domain, material);
                    vtkDataArray *var1 = ConvertDataArrayToFloat(var);
                    if (var1)
                    {
                        cache.CacheVTKObject(varname,
                            avtVariableCache::VECTORS_NAME,
                            ts, domain, material, var1);
                        var1->Delete();
                        var = var1;
                    }
                }
            }
        }
        else
        {
            var = (vtkDataArray *) cache.GetVTKObject(varname,
                                 avtVariableCache::VECTORS_NAME,
                                 ts, domain, material);
        }
    }

    //
    // Translate the variable name into something the interface understands.
    // Note: use the "real_varname" when talking to the Interface, but use
    // the standard "varname" for caching and all other places.
    //
    const avtVectorMetaData *vmd = GetMetaData(ts)->GetVector(varname);
    if (vmd == NULL)
        EXCEPTION1(InvalidVariableException, varname);
    const char *real_varname = varname;
    if (vmd->originalName != vmd->name && vmd->originalName != "")
    {
        real_varname = vmd->originalName.c_str();
    }

    if (var == NULL)
    {
        //
        // We haven't read in this domain before, so fetch it from the files.
        //
        var = Interface->GetVectorVar(ts, domain, real_varname);

        if (var != NULL)
        {
            //
            // Convert IMMEDIATELY only if native precision is greater
            // than float and VisIt has NOT asked for native precision
            //
            const char *cacheTypeName = 
                var->GetDataType() == VTK_FLOAT ?
                avtVariableCache::VECTORS_NAME :
                avtVariableCache::NATIVE_VECTORS_NAME;
            if (PrecisionInBytes(var) > sizeof(float) && !needNativePrecision)
            {
                vtkDataArray *var1 = ConvertDataArrayToFloat(var);
                if (var1)
                {
                    var->Delete();
                    var = var1;
                    cacheTypeName = avtVariableCache::VECTORS_NAME;
                }
            }

            if (Interface->CanCacheVariable(real_varname))
            {
                cache.CacheVTKObject(varname, cacheTypeName, 
                                     ts, domain, material, var);
            }
            else
            {
                ManageMemoryForNonCachableVar(var);
            }

            //
            // We need to decrement the reference count of the variable 
            // returned from FetchVar, but we could not do it previously 
            // because it would knock the count down to 0 and delete it.
            // Since we have cached it, we can do it now.
            //
            var->Delete();

            //
            // Convert BEFORE returning to VisIt only if precision in
            // cache is NOT supported by current pipeline. Also, if we
            // convert here, go ahead and cache the converted result too.
            //
            if (!IsAdmissibleDataType(admissibleDataTypes, var->GetDataType()))
            {
                vtkDataArray *var1 = ConvertDataArrayToFloat(var);

                if (var1)
                {
                    if (Interface->CanCacheVariable(real_varname))
                    {
                        cache.CacheVTKObject(varname, avtVariableCache::VECTORS_NAME,
                                             ts, domain, material, var1);
                    }
                    else
                    {
                        ManageMemoryForNonCachableVar(var1);
                    }
                    var1->Delete();
                    var = var1;
                }
            }
        }
    }

    return var;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetTensorVariable
//
//  Purpose:
//      Checks to see if a variable is already in cache and fetches it if it
//      is not.
//
//  Arguments:
//      varname    The name of the variable.
//      ts         The timestep for the variable.
//      domain     The domain for the variable.
//      material   The name of the material we are getting.
//
//  Returns:    A vtkDataArray for the variable.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Feb 15 07:21:10 PST 2005
//    Make translations when we have hidden characters.
//
//    Hank Childs, Fri Mar 11 11:24:25 PST 2005
//    Fix memory leak.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added support for data type conversion
//
// ****************************************************************************

vtkDataArray *
avtGenericDatabase::GetTensorVariable(const char *varname, int ts, int domain,
                                      const char *material,
                                      const bool needNativePrecision,
                                      const vector<int>& admissibleDataTypes)
{
    //
    // We have to be leery about doing any caching when the variables are
    // defined on sub-meshes.  This is because if we add new secondary
    // variables, some parts of the mesh may drop out, and the size of the
    // cached objects are now wrong.
    //
    vtkDataArray *var = NULL;
    if (!Interface->HasVarsDefinedOnSubMeshes())
    {
        //
        // First, look for native precision data in the cache
        //
        var = (vtkDataArray *) cache.GetVTKObject(varname,
                         avtVariableCache::NATIVE_TENSORS_NAME,
                         ts, domain, material);

        if ((var != NULL) &&
            IsAdmissibleDataType(admissibleDataTypes, var->GetDataType()))
        {
            if (!needNativePrecision &&
                (PrecisionInBytes(var) > sizeof(float)))
            {
                var = (vtkDataArray *) cache.GetVTKObject(varname,
                                 avtVariableCache::TENSORS_NAME,
                                 ts, domain, material);

                if (var == NULL)
                {
                    //
                    // At this point, we know we've got ONLY native data
                    // in the cache but either visit is NOT requesting
                    // native data or the native precision is greater
                    // than float. So, even though the pipeline supports
                    // this data, we're going to convert because there
                    // is no value added in running with higher precision
                    // when it is not necessary. After we've converted
                    // we'll cache the converted result.
                    //
                    var = (vtkDataArray *) cache.GetVTKObject(varname,
                        avtVariableCache::NATIVE_TENSORS_NAME,
                        ts, domain, material);
                    vtkDataArray *var1 = ConvertDataArrayToFloat(var);
                    if (var1)
                    {
                        cache.CacheVTKObject(varname,
                            avtVariableCache::TENSORS_NAME,
                            ts, domain, material, var1);
                        var1->Delete();
                        var = var1;
                    }
                }
            }
        }
        else
        {
            var = (vtkDataArray *) cache.GetVTKObject(varname,
                             avtVariableCache::TENSORS_NAME,
                             ts, domain, material);
        }

    }

    //
    // Translate the variable name into something the interface understands.
    // Note: use the "real_varname" when talking to the Interface, but use
    // the standard "varname" for caching and all other places.
    //
    const avtTensorMetaData *tmd = GetMetaData(ts)->GetTensor(varname);
    if (tmd == NULL)
        EXCEPTION1(InvalidVariableException, varname);
    const char *real_varname = varname;
    if (tmd->originalName != tmd->name && tmd->originalName != "")
    {
        real_varname = tmd->originalName.c_str();
    }

    if (var == NULL)
    {
        //
        // We haven't read in this domain before, so fetch it from the files.
        // Note: we use the vector var interface to get tensors.
        //
        var = Interface->GetVectorVar(ts, domain, real_varname);

        if (var != NULL)
        {
            //
            // Convert BEFORE caching only if native precision is greater
            // than float and VisIt has NOT asked for native precision
            //
            const char *cacheTypeName = 
                var->GetDataType() == VTK_FLOAT ?
                avtVariableCache::TENSORS_NAME :
                avtVariableCache::NATIVE_TENSORS_NAME;
            if (PrecisionInBytes(var) > sizeof(float) && !needNativePrecision)
            {
                vtkDataArray *var1 = ConvertDataArrayToFloat(var);
                if (var1)
                {
                    var->Delete();
                    var = var1;
                    cacheTypeName = avtVariableCache::TENSORS_NAME;
                }
            }

            if (Interface->CanCacheVariable(real_varname))
            {
                cache.CacheVTKObject(varname, cacheTypeName,
                                     ts, domain, material, var);
            }
            else
            {
                ManageMemoryForNonCachableVar(var);
            }

            //
            // We need to decrement the reference count of the variable 
            // returned from FetchVar, but we could not do it previously 
            // because it would knock the count down to 0 and delete it.
            // Since we have cached it, we can do it now.
            //
            var->Delete();

            //
            // Convert BEFORE returning to VisIt only if precision in
            // cache is NOT supported by current pipeline. Also, if we
            // convert here, go ahead and cache the converted result too.
            //
            if (!IsAdmissibleDataType(admissibleDataTypes, var->GetDataType()))
            {
                vtkDataArray *var1 = ConvertDataArrayToFloat(var);

                if (var1)
                {
                    if (Interface->CanCacheVariable(real_varname))
                    {
                        cache.CacheVTKObject(varname, avtVariableCache::TENSORS_NAME,
                                             ts, domain, material, var1);
                    }
                    else
                    {
                        ManageMemoryForNonCachableVar(var1);
                    }
                    var1->Delete();
                    var = var1;
                }
            }
        }
    }

    return var;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetSymmetricTensorVariable
//
//  Purpose:
//      Checks to see if a variable is already in cache and fetches it if it
//      is not.
//
//  Arguments:
//      varname    The name of the variable.
//      ts         The timestep for the variable.
//      domain     The domain for the variable.
//      material   The name of the material we are getting.
//
//  Returns:    A vtkDataArray for the variable.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Feb 15 07:21:10 PST 2005
//    Make translations when we have hidden characters.
//
//    Hank Childs, Fri Mar 11 11:24:25 PST 2005
//    Fix memory leak.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added support for data type conversion
//
// ****************************************************************************

vtkDataArray *
avtGenericDatabase::GetSymmetricTensorVariable(const char *varname, int ts,
                                               int domain,const char *material,
                                               const bool needNativePrecision,
                                        const vector<int>& admissibleDataTypes)
{
    //
    // We have to be leery about doing any caching when the variables are
    // defined on sub-meshes.  This is because if we add new secondary
    // variables, some parts of the mesh may drop out, and the size of the
    // cached objects are now wrong.
    //
    vtkDataArray *var = NULL;
    if (!Interface->HasVarsDefinedOnSubMeshes())
    {
        //
        // First, look for native precision data in the cache
        //
        var = (vtkDataArray *) cache.GetVTKObject(varname,
                         avtVariableCache::NATIVE_SYMMETRIC_TENSORS_NAME,
                         ts, domain, material);

        if ((var != NULL) &&
            IsAdmissibleDataType(admissibleDataTypes, var->GetDataType()))
        {
            if (!needNativePrecision &&
                (PrecisionInBytes(var) > sizeof(float)))
            {
                var = (vtkDataArray *) cache.GetVTKObject(varname,
                                 avtVariableCache::SYMMETRIC_TENSORS_NAME,
                                 ts, domain, material);

                if (var == NULL)
                {
                    //
                    // At this point, we know we've got ONLY native data
                    // in the cache but either visit is NOT requesting
                    // native data or the native precision is greater
                    // than float. So, even though the pipeline supports
                    // this data, we're going to convert because there
                    // is no value added in running with higher precision
                    // when it is not necessary. After we've converted
                    // we'll cache the converted result.
                    //
                    var = (vtkDataArray *) cache.GetVTKObject(varname,
                        avtVariableCache::NATIVE_SYMMETRIC_TENSORS_NAME,
                        ts, domain, material);
                    vtkDataArray *var1 = ConvertDataArrayToFloat(var);
                    if (var1)
                    {
                        cache.CacheVTKObject(varname,
                            avtVariableCache::SYMMETRIC_TENSORS_NAME,
                            ts, domain, material, var1);
                        var1->Delete();
                        var = var1;
                    }
                }
            }
        }
        else
        {
            var = (vtkDataArray *) cache.GetVTKObject(varname,
                             avtVariableCache::SYMMETRIC_TENSORS_NAME,
                             ts, domain, material);
        }
    }

    //
    // Translate the variable name into something the interface understands.
    // Note: use the "real_varname" when talking to the Interface, but use
    // the standard "varname" for caching and all other places.
    //
    const avtSymmetricTensorMetaData *smd = 
                                       GetMetaData(ts)->GetSymmTensor(varname);
    if (smd == NULL)
        EXCEPTION1(InvalidVariableException, varname);
    const char *real_varname = varname;
    if (smd->originalName != smd->name && smd->originalName != "")
    {
        real_varname = smd->originalName.c_str();
    }

    if (var == NULL)
    {
        //
        // We haven't read in this domain before, so fetch it from the files.
        // Note: we use the vector var interface to get tensors.
        //
        var = Interface->GetVectorVar(ts, domain, real_varname);

        if (var != NULL)
        {
            //
            // Convert BEFORE caching only if native precision is greater
            // than float and VisIt has NOT asked for native precision
            //
            const char *cacheTypeName = 
                var->GetDataType() == VTK_FLOAT ?
                avtVariableCache::SYMMETRIC_TENSORS_NAME :
                avtVariableCache::NATIVE_SYMMETRIC_TENSORS_NAME;
            if (PrecisionInBytes(var) > sizeof(float) && !needNativePrecision)
            {
                vtkDataArray *var1 = ConvertDataArrayToFloat(var);
                if (var1)
                {
                    var->Delete();
                    var = var1;
                    cacheTypeName = avtVariableCache::SYMMETRIC_TENSORS_NAME;
                }
            }

            //
            // Cache the variable if we can
            //
            if (Interface->CanCacheVariable(real_varname))
            {
                cache.CacheVTKObject(varname,
                                     avtVariableCache::SYMMETRIC_TENSORS_NAME,
                                     ts, domain, material, var);
            }
            else
            {
                ManageMemoryForNonCachableVar(var);
            }

            //
            // We need to decrement the reference count of the variable 
            // returned from FetchVar, but we could not do it previously 
            // because it would knock the count down to 0 and delete it.
            // Since we have cached it, we can do it now.
            //
            var->Delete();

            //
            // Convert BEFORE returning to VisIt only if precision in
            // cache is NOT supported by current pipeline. Also, if we
            // convert here, go ahead and cache the converted result too.
            //
            if (!IsAdmissibleDataType(admissibleDataTypes, var->GetDataType()))
            {
                vtkDataArray *var1 = ConvertDataArrayToFloat(var);
                
                if (var1)
                {
                    if (Interface->CanCacheVariable(real_varname))
                    {
                        cache.CacheVTKObject(varname,
                            avtVariableCache::SYMMETRIC_TENSORS_NAME,
                            ts, domain, material, var1);
                    }
                    else
                    {
                        ManageMemoryForNonCachableVar(var1);
                    }
                    var1->Delete();
                    var = var1;
                }
            }
        }
    }

    return var;
}

// ****************************************************************************
// Method: avtGenericDatabase::GetLabelVariable
//
// Purpose: 
//   Gets the specified label data array from the database.
//
// Arguments:
//      varname    The name of the variable.
//      ts         The timestep for the variable.
//      domain     The domain for the variable.
//      material   The name of the material we are getting.
//
// Returns:    The data array
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Sat Apr 2 00:43:11 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
avtGenericDatabase::GetLabelVariable(const char *varname, int ts, int domain,
                                     const char *material)
{

    //
    // We have to be leery about doing any caching when the variables are
    // defined on sub-meshes.  This is because if we add new secondary
    // variables, some parts of the mesh may drop out, and the size of the
    // cached objects are now wrong.
    //
    vtkDataArray *var = NULL;
    if (!Interface->HasVarsDefinedOnSubMeshes())
    {
        var = (vtkDataArray *) cache.GetVTKObject(varname,
                         avtVariableCache::LABELS_NAME, ts, domain, material);
    }

    //
    // Translate the variable name into something the interface understands.
    // Note: use the "real_varname" when talking to the Interface, but use
    // the standard "varname" for caching and all other places.
    //
    const avtLabelMetaData *lmd = GetMetaData(ts)->GetLabel(varname);
    if (lmd == NULL)
        EXCEPTION1(InvalidVariableException, varname);
    const char *real_varname = varname;
    if (lmd->originalName != lmd->name && lmd->originalName != "")
    {
        real_varname = lmd->originalName.c_str();
    }

    if (var == NULL)
    {
        //
        // We haven't read in this domain before, so fetch it from the files.
        //
        var = Interface->GetVar(ts, domain, real_varname);

        if (var != NULL)
        {
            if (Interface->CanCacheVariable(real_varname))
            {
                cache.CacheVTKObject(varname, avtVariableCache::LABELS_NAME,
                                     ts, domain, material, var);
            }
            else
                ManageMemoryForNonCachableVar(var);

            //
            // We need to decrement the reference count of the variable 
            // returned from FetchVar, but we could not do it previously 
            // because it would knock the count down to 0 and delete it.
            // Since we have cached it, we can do it now.
            //
            var->Delete();
        }
    }

    return var;
}

// ****************************************************************************
//  Method: avtGenericDatabase::GetMesh
//
//  Purpose:
//      Checks to see if a mesh is already in cache and fetches it if it's not.
//
//  Arguments:
//      meshname   The name of the mesh.
//      ts         The timestep for the mesh.
//      domain     The domain for the mesh.
//      material   The name of the material we are getting.
//
//  Returns:    A vtkDataSet for the mesh.  The caller of this routine owns
//              the mesh.
//
//  Programmer: Hank Childs
//  Creation:   October 31, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Mar  1 09:20:52 PST 2001
//    Moved from avtDatabase to avtGenericDatabase.
//
//    Kathleen Bonnell, Tue Jun 12 14:34:02 PDT 2001 
//    Added Code to pass ghost-level information along if necessary. 
//
//    Hank Childs, Wed Aug 22 14:45:33 PDT 2001
//    Added case for poly data meshes.
//
//    Hank Childs, Mon Oct  8 10:54:30 PDT 2001
//    Added material argument.
//
//    Hank Childs, Thu Oct 11 13:04:25 PDT 2001
//    Better support for returning NULL.
//
//    Hank Childs, Wed Jan  9 14:05:05 PST 2002
//    Removed unnecessary code in favor of MakeObject.
//
//    Hank Childs, Tue Jun 25 19:44:24 PDT 2002
//    Copy over the field data.
//
//    Hank Childs, Mon Sep 30 09:11:13 PDT 2002
//    Indicate if we have ghost zones or not.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    Use NewInstance instead of MakeObject, new vtk api.
//
//    Hank Childs, Fri Mar 14 20:56:40 PST 2003
//    Turned off caching in some instances.
//
//    Hank Childs, Tue Jul 29 16:48:28 PDT 2003
//    Scale mesh when appropriate.  Also cache bounds.
//
//    Hank Childs, Sun Jun 27 10:47:38 PDT 2004
//    Copy over more information about ghosts or global indexing.
//
//    Hank Childs, Fri Jul  9 14:24:21 PDT 2004
//    No longer scale the mesh, since the camera and lighting now both handle
//    large and small problems better.
//
//    Mark C. Miller, Mon Aug  9 19:12:24 PDT 2004
//    Removed code to pass along 'avtGlobalNodeIds' and 'ElementGlobalIds'
//    point and cell data arrays. That information is now obtained through
//    the GetAuxiliaryData interface.
//
//    Hank Childs, Fri Aug 27 16:16:52 PDT 2004
//    Rename ghost data arrays.
//
//    Hank Childs, Tue Feb 15 07:21:10 PST 2005
//    Make translations when we have hidden characters.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added support for data type conversion
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetMesh(const char *meshname, int ts, int domain,
                            const char *material, const bool needNativePrecision,
                            const vector<int>& admissibleDataTypes)
{
    //
    // We have to be leery about doing any caching when the variables are
    // defined on sub-meshes.  This is because if we add new secondary
    // variables, some parts of the mesh may drop out, and the size of the
    // cached objects are now wrong.
    //
    vtkDataSet *mesh = NULL;
    if (!Interface->HasVarsDefinedOnSubMeshes())
    {

        //
        // First, look for native precision data in the cache
        //
        mesh = (vtkDataSet *) cache.GetVTKObject(meshname,
                         avtVariableCache::NATIVE_DATASET_NAME,
                         ts, domain, material);

        if ((mesh != NULL) && IsAdmissibleDataType(admissibleDataTypes,
                                  GetCoordDataArray(mesh)->GetDataType()))
        {
            if (!needNativePrecision &&
                (PrecisionInBytes(GetCoordDataArray(mesh)) > sizeof(float)))
            {
                mesh = (vtkDataSet *) cache.GetVTKObject(meshname,
                                 avtVariableCache::DATASET_NAME,
                                 ts, domain, material);

                if (mesh == NULL)
                {
                    //
                    // At this point, we know we've got ONLY native data
                    // in the cache but either visit is NOT requesting
                    // native data or the native precision is greater
                    // than float. So, even though the pipeline supports
                    // this data, we're going to convert because there
                    // is no value added in running with higher precision
                    // when it is not necessary. After we've converted
                    // we'll cache the converted result.
                    //
                    mesh = (vtkDataSet *) cache.GetVTKObject(meshname,
                        avtVariableCache::NATIVE_DATASET_NAME,
                        ts, domain, material);
                    vtkDataSet *mesh1 = ConvertDataSetToFloat(mesh);
                    if (mesh1)
                    {
                        cache.CacheVTKObject(meshname,
                            avtVariableCache::DATASET_NAME,
                            ts, domain, material, mesh1);
                        mesh1->Delete();
                        mesh = mesh1;
                    }
                }
            }
        }
        else
        {
            mesh = (vtkDataSet *) cache.GetVTKObject(meshname,
                         avtVariableCache::DATASET_NAME,
                         ts, domain, material);
        }
    }

    //
    // Translate the variable name into something the interface understands.
    // Note: use the "real_varname" when talking to the Interface, but use
    // the standard "varname" for caching and all other places.
    //
    const avtMeshMetaData *mmd = GetMetaData(ts)->GetMesh(meshname);
    const avtCurveMetaData *cmd = GetMetaData(ts)->GetCurve(meshname);
    if (cmd == NULL && mmd == NULL)
        EXCEPTION1(InvalidVariableException, meshname);
    const char *real_meshname = meshname;
    if (mmd != NULL && 
        mmd->originalName != mmd->name && mmd->originalName != "")
    {
        real_meshname = mmd->originalName.c_str();
    }
    if (cmd != NULL && 
        cmd->originalName != cmd->name && cmd->originalName != "")
    {
        real_meshname = cmd->originalName.c_str();
    }

    if (mesh == NULL)
    {
        //
        // We haven't read in this domain before, so fetch it from the files.
        //
        mesh = Interface->GetMesh(ts, domain, real_meshname);

        if (mesh == NULL)
        {
            debug5 << "Mesh returned by file format is NULL for domain "
                   << domain << ", material = " << material << endl;
            return NULL;
        }

        //
        // Force an Update.  This needs to be done and if we do it when we
        // read it in, then it guarantees it only happens once.
        //
        mesh->Update();

        AssociateBounds(mesh);

        //
        // Convert BEFORE caching only if native precision is greater
        // than float and VisIt has NOT asked for native precision
        //
        const char *cacheTypeName = 
            GetCoordDataArray(mesh)->GetDataType() == VTK_FLOAT ?
            avtVariableCache::DATASET_NAME :
            avtVariableCache::NATIVE_DATASET_NAME;
        if (PrecisionInBytes(GetCoordDataArray(mesh)) > sizeof(float) &&
            !needNativePrecision)
        {
            vtkDataSet *mesh1 = ConvertDataSetToFloat(mesh);
            if (mesh1)
            {
                mesh->Delete();
                mesh = mesh1;
                cacheTypeName = avtVariableCache::DATASET_NAME;
            }
        }

        if (Interface->CanCacheVariable(real_meshname))
        {
            cache.CacheVTKObject(meshname, cacheTypeName, ts,
                                 domain, material, mesh);
        }
        else
        {
            ManageMemoryForNonCachableMesh(mesh);
        }

        //
        // We need to decrement the reference count of the variable returned
        // from FetchMesh, but we could not do it previously because it
        // would knock the count down to 0 and delete it.  Since we have
        // cached it, we can do it now.
        //
        mesh->Delete();

        //
        // Convert BEFORE returning to VisIt only if precision in
        // cache is NOT supported by current pipeline. Also, if we
        // convert here, go ahead and cache the converted result too.
        //
        if (!IsAdmissibleDataType(admissibleDataTypes,
                                  GetCoordDataArray(mesh)->GetDataType()))
        {
            vtkDataSet *mesh1 = ConvertDataSetToFloat(mesh);

            if (mesh1)
            {
                if (Interface->CanCacheVariable(real_meshname))
                {
                    cache.CacheVTKObject(meshname, avtVariableCache::DATASET_NAME,
                                         ts, domain, material, mesh1);
                }
                else
                {
                    ManageMemoryForNonCachableMesh(mesh1);
                }
                mesh1->Delete();
                mesh = mesh1;
            }
        }
    }

    //
    // We cannot return this mesh exactly.  If we do, when we try to plot
    // another variable, it will use the same mesh and reset the variable,
    // invalidating the mesh we have in the pipeline.  To get around this, we
    // can copy the structure of the mesh (which is cheap and done through
    // reference counting).
    //
    vtkDataSet *rv = (vtkDataSet *) mesh->NewInstance();
    rv->CopyStructure(mesh);

    //
    // There are some mesh variables that we want to copy over -- namely
    // those with ghost information or global numbering information.
    //
    if (mesh->GetCellData()->GetArray("avtGhostZones"))
    {
        rv->GetCellData()->AddArray(
            mesh->GetCellData()->GetArray("avtGhostZones"));
        GetMetaData(ts)->SetContainsGhostZones(meshname, AVT_HAS_GHOSTS);
    }
    if (mesh->GetPointData()->GetArray("avtGhostNodes"))
    {
        rv->GetPointData()->AddArray(
            mesh->GetPointData()->GetArray("avtGhostNodes"));
        GetMetaData(ts)->SetContainsGhostZones(meshname, AVT_HAS_GHOSTS);
    }
    rv->GetFieldData()->ShallowCopy(mesh->GetFieldData());

    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data by asking the interface if it has it.  Also
//      performs caching.
//
//  Programmer: Hank Childs
//  Creation:   March 1, 2001
//
//  Modifications:
//
//    Hank Childs, Tue May 22 10:47:31 PDT 2001
//    Use a database specification instead of a SIL restriction.  Also took
//    care of hacks for caching/memory management.
//
//    Hank Childs, Wed Oct 10 15:47:49 PDT 2001
//    Don't cache back NULL data.
//
//    Hank Childs, Mon Aug 23 14:14:23 PDT 2004
//    Look for auxiliary data where the data is of type that applies to all
//    timesteps (for example, global node ids where the nodes don't change
//    over time).
//
//    Mark C. Miller, Mon Oct 18 13:02:37 PDT 2004
//    Added code to set variable name from 'args' for Data/Spatial extents
//
//    Mark C. Miller, Wed Oct 20 12:01:10 PDT 2004
//    Added call to ActivateTimestep to deal with cases in which the *first*
//    thing we attempt to read from the file is auxiliary data
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector'.
//
//    Hank Childs, Tue Feb 15 07:21:10 PST 2005
//    Make translations when we have hidden characters.
//
// ****************************************************************************

void
avtGenericDatabase::GetAuxiliaryData(avtDataSpecification_p spec,
                                     VoidRefList &rv, const char *type, 
                                     void *args)
{
    //
    // Get the fields out of the database specification.  Use the SIL
    // specification since it captures the concept of a SIL restriction as well
    // as the concept of an object that spans all domains, like an interval
    // tree (which it represents as domain -1).
    //
    int ts = spec->GetTimestep();
    avtSILSpecification sil = spec->GetSIL();
    const char *var = spec->GetVariable();
    if ((strcmp(type, AUXILIARY_DATA_SPATIAL_EXTENTS) == 0) ||
        (strcmp(type, AUXILIARY_DATA_DATA_EXTENTS) == 0))
    {
        if (args != NULL)
            var = (char *) args;
    }

    //
    // It is concievable to arrive here with a file that has not yet
    // been opened. So, we need to active this timestep which will
    // be a no-op if the file has been opened and will open the file
    // if it has not been opened.
    //
    ActivateTimestep(ts);

    const char *real_var = GetOriginalVariableName(GetMetaData(ts), var);

    intVector domains;
    sil.GetDomainList(domains);

    //
    // Populate the void reference list.
    //
    rv.nList = domains.size();
    rv.list = new void_ref_ptr[rv.nList];

    for (int i = 0 ; i < domains.size() ; i++)
    {
        //
        // See if we already have the data lying around for this timestep or
        // for "all" (= -1) timesteps.  Examples of "all" timesteps entities
        // are global node ids where the nodes do not change over time.
        //
        void_ref_ptr vr = cache.GetVoidRef(var, type, ts, domains[i]);
        if (*vr == NULL)
        {
            vr = cache.GetVoidRef(var, type, -1, domains[i]);
        }

        if (*vr != NULL)
        {
            //
            // We have it, so share a reference and you're done.
            //
            rv.list[i] = vr;
        }
        else
        {
            //
            // We did not have it, so calculate it and then store it.
            //
            DestructorFunction df;
            void *d = Interface->GetAuxiliaryData(real_var, ts, domains[i],
                                                  type, args, df);

            if (d != NULL)
            {
                //
                // Putting 'd' into the world of reference pointers saves some
                // headaches.
                //
                void_ref_ptr vr = void_ref_ptr(d, df);
                cache.CacheVoidRef(var, type, ts, domains[i], vr);
                rv.list[i] = vr;
            }
        }
    }
}


// ****************************************************************************
//  Method: avtGenericDatabase::PopulateSIL
//
//  Purpose:
//      Populates a SIL from the database meta-data.
//
//  Arguments:
//      sil       : The sil to populate.
//      timeState : The time state that we're interested in.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
//  Modifications:
//    Hank Childs, Fri Sep  6 12:24:55 PDT 2002
//    Relocated old routine to avtSILGenerator.  Blew away outdated comments.
//
//    Brad Whitlock, Wed May 14 09:12:42 PDT 2003
//    Added optional timeState argument.
//
// ****************************************************************************

void
avtGenericDatabase::PopulateSIL(avtSIL *sil, int timeState)
{
    int timerHandle = visitTimer->StartTimer();
    avtDatabaseMetaData *md = GetMetaData(timeState);
    avtSILGenerator gen;
    gen.CreateSIL(md, sil);
    visitTimer->StopTimer(timerHandle, "Creating a SIL object.");
    visitTimer->DumpTimings();
}


// ****************************************************************************
//  Method: avtGenericDatabase::PopulateIOInformation
//
//  Purpose:
//      Populates the I/O Hints for a database, which allows domains that
//      should be processed on the same processor for I/O performance reasons
//      to do so.
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2001
//
//  Modifications:
//    Mark C. Miller, Tue Mar 16 14:40:19 PST 2004
//    Added timestep argument
//
// ****************************************************************************

void
avtGenericDatabase::PopulateIOInformation(int ts, avtIOInformation &ioInfo)
{
    Interface->PopulateIOInformation(ts, ioInfo);
}

// ****************************************************************************
//  Function: AddOriginalCellsArray
//
//  Purpose:
//    Creates an array of 'original zone numbers' to attach to the dataset.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 8, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Mon Nov 12 09:38:17 PST 2001
//    Don't re-create array if it is already present.  Made values in
//    original cell array be global in nature, so multiple-domain datasets
//    have unique "original zones" across domains.  Turn on flag that
//    ensures this array will be copied by vtk filters.
//
//    Kathleen Bonnell, Tue Mar 26 15:58:05 PST 2002 
//    Added new argument to keep track of starting cell number.
//
//    Kathleen Bonnell, Mon May 20 17:01:31 PDT 2002  
//    Changed int parameter to domain number.  Add domain as another
//    component to the array. 
//
//    Hank Childs, Sat Sep  7 08:34:08 PDT 2002
//    Sped up routine and fixed memory leak.
//
//    Hank Childs, Mon Sep 30 18:01:26 PDT 2002
//    Do not force the domain to be encoded.
//
//    Hank Childs, Fri Jan 17 08:47:57 PST 2003
//    Re-ordered the location of the start-timer call to make sure that it
//    would have a matching stop-timer.
//
// ****************************************************************************

void
avtGenericDatabase::AddOriginalCellsArray(vtkDataSet *ds, const int domain)
{
    if (ds == NULL || ds->GetCellData()->GetArray("avtOriginalCellNumbers"))
    {
        // Array already created, probably by GetMeshDataset
        return;
    }
    int timerHandle = visitTimer->StartTimer();
    vtkUnsignedIntArray *origZones = vtkUnsignedIntArray::New();
    origZones->SetName("avtOriginalCellNumbers");
    bool encodeDomains = (domain >= 0 ? true : false);
    int nComps = (encodeDomains ? 2 : 1);
    origZones->SetNumberOfComponents(nComps);
    int ncells = ds->GetNumberOfCells();
    origZones->SetNumberOfTuples(ncells);
    unsigned int *ptr = origZones->GetPointer(0);
    for (int i = 0; i < ncells; i++)
    {
        if (encodeDomains)
        {
            *ptr = domain;
            ptr++;
        }
        *ptr = i;
        ptr++;
    }
    ds->GetCellData()->AddArray(origZones);
    origZones->Delete();
    ds->GetCellData()->CopyFieldOn("avtOriginalCellNumbers");
    visitTimer->StopTimer(timerHandle, "Creating original zones array");
}


// ****************************************************************************
//  Function: AddOriginalNodesArray
//
//  Purpose:
//    Creates an array of 'original node numbers' to attach to the dataset.
//
//  Programmer: Hank Childs
//  Creation:   June 18, 2003
//
//  Kathleen bonnell, Wed Jun 16 13:54:28 PDT 2004
//  Changed orgiNodes array to vtkIntArray from vtkUnsignedIntArray, to
//  accommodate -1 (used by MIR when creating nodes).
//
// ****************************************************************************

void
avtGenericDatabase::AddOriginalNodesArray(vtkDataSet *ds, const int domain)
{
    if (ds == NULL || ds->GetPointData()->GetArray("avtOriginalNodeNumbers"))
    {
        // DataSet is NULL or array is already created -- return.
        return;
    }

    int timerHandle = visitTimer->StartTimer();
    vtkIntArray *origNodes = vtkIntArray::New();
    origNodes->SetName("avtOriginalNodeNumbers");
    bool encodeDomains = (domain >= 0 ? true : false);
    int nComps = (encodeDomains ? 2 : 1);
    origNodes->SetNumberOfComponents(nComps);
    int nnodes = ds->GetNumberOfPoints();
    origNodes->SetNumberOfTuples(nnodes);
    int *ptr = origNodes->GetPointer(0);
    for (int i = 0; i < nnodes; i++)
    {
        if (encodeDomains)
        {
            *ptr = domain;
            ptr++;
        }
        *ptr = i;
        ptr++;
    }
    ds->GetPointData()->AddArray(origNodes);
    origNodes->Delete();
    ds->GetPointData()->CopyFieldOn("avtOriginalNodeNumbers");
    visitTimer->StopTimer(timerHandle, "Creating original nodes array");
}


// ****************************************************************************
//  Method: avtGenericDatabase::MaterialSelect
//
//  Purpose:
//      Determines if the dataset needs to be material selected and performs
//      the selection if it does.
//
//  Arguments:
//      ds      The input as a vtk dataset.
//      mat     The avt material.
//      mvl     A list of the mixed variables.
//      dom     The domain number for this dataset.
//      var     The variable for this dataset (to determine the material).
//      ts      The timestep of interest.
//      mnames  The names of the materials we want.
//      reUseMIR  Whether or not the MIR can be re-used.
//
//  Output Arguments:
//      subdivisionOccurred              true if subdiv occurred in MIR
//      notAllCellsSubdivided            true if some cells were left whole
//
//  Returns:    A data tree representation of the dataset, material selected or
//              not based on the sil restriction.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2001
//
//  Arguments:
//
//    Hank Childs, Tue Jul 24 14:17:14 PDT 2001
//    Added didIt argument.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001 
//    Retrieve material names and pass them to the output tree. 
//
//    Jeremy Meredith, Mon Oct  1 12:09:06 PDT 2001
//    Pass mixvar info to GetZonalVariable.
//
//    Hank Childs, Fri Oct  5 16:27:34 PDT 2001
//    Added mnames argument and removed arguments didIt and silr.
//
//    Kathleen Bonnell, Fri Oct 19 15:33:35 PDT 2001 
//    Added labels argument, to be used when creating the data tree instead
//    of mnames. 
//
//    Hank Childs, Mon Nov 26 09:01:36 PST 2001
//    Added material argument.
//
//    Hank Childs, Wed Dec  5 10:57:23 PST 2001
//    Added mixvar argument.
//
//    Eric Brugger, Tue Dec 11 13:02:39 PST 2001
//    I modified the material selection to create a single dataset with
//    all the cells instead of a dataset per material.  This causes the
//    internal material boundaries to be eliminated.
//
//    Jeremy Meredith, Wed Dec 19 20:13:05 PST 2001
//    Added understanding of species as a zonal variable.
//
//    Hank Childs, Fri Feb  1 15:20:32 PST 2002
//    Material select all of the variables.
//
//    Hank Childs, Wed Feb  6 11:45:12 PST 2002
//    Check before overwriting ghost levels array.
//
//    Jeremy Meredith, Thu Mar 14 18:02:31 PST 2002
//    Added a flag to split materials into their own datasets.
//
//    Jeremy Meredith, Thu Mar 14 18:03:22 PST 2002
//    Added a flag for the helper MaterialSelect which says whether or
//    not the dataspec says we need internal surfaces.
//
//    Kathleen Bonnell, Fri Mar 15 10:11:23 PST 2002   
//    Use new VTK methods for changing active scalars in PointData and CellData.
//
//    Hank Childs, Thu Jul  4 09:30:05 PDT 2002
//    Fit MIRs new interface to handle secondary variables without so much
//    hoopla.  Also add support for an arbitrary number of mixvars.
//
//    Jeremy Meredith, Tue Aug 13 14:36:40 PDT 2002
//    Added the needValidConnectivity flag, as well as the two subdivision
//    return flags.
//
//    Kathleen Bonnell, Thu Aug 15 20:08:34 PDT 2002  
//    Use the blockOrigin when setting a domain label (instead of adding 1
//    to the domain number). 
//    
//    Hank Childs, Sun Aug 18 22:33:35 PDT 2002
//    Added argument for whether or not the MIR can be re-used.
//
//    Kathleen Bonnell, Thu Sep  5 13:53:15 PDT 2002  
//    Added support for creating group labels. 
//
//    Hank Childs, Thu Sep 26 08:22:40 PDT 2002
//    Cleaned up huge memory leak.
//
//    Hank Childs, Wed Oct  2 10:09:53 PDT 2002
//    Go back to using reference pointers because the MIR may not have been
//    cached.
//
//    Jeremy Meredith, Thu Oct 24 15:37:05 PDT 2002
//    Added smoothing option and clean zones only option.
//
//    Jeremy Meredith, Thu Jun 12 09:07:28 PDT 2003
//    Added needBoundarySurfaces option.
//
//    Hank Childs, Tue Jul 22 21:48:09 PDT 2003
//    Added a flag for whether or not we communicated ghosts.
//
//    Hank Childs, Fri Sep 12 16:27:38 PDT 2003
//    Added a flag for whether or not reconstruction was forced.
//
//    Jeremy Meredith, Fri Sep  5 15:31:53 PDT 2003
//    Added a flag for the MIR algorithm.
//
//    Kathleen Bonnell, Thu Sep 18 11:49:16 PDT 2003 
//    Add 'vtkOriginalDimensions' field data to output, if input was
//    structured (so Pick can return correct zone/node numbers).
//
//    Jeremy Meredith, Wed Oct 15 17:25:48 PDT 2003
//    Added code to correctly handle clean-zones-only MIR.
//
//    Hank Childs, Fri Apr  9 09:15:25 PDT 2004
//    Use the material object to determine the material indices, not meta-data
//    from timestep 0.
//
//    Hank Childs, Wed Jul 28 15:28:23 PDT 2004
//    Make sure that the material object and the dataset are sized 
//    appropriately before starting to execute.
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector', and 'vector<string>' to 
//    'stringVector'.
//
//    Hank Childs, Thu Mar 10 16:48:19 PST 2005
//    Fix memory leak with boundary plots.
//
//    Mark C. Miller, Thu Apr 21 09:37:41 PDT 2005
//    Made error message a little clearer
//
// ****************************************************************************

avtDataTree_p
avtGenericDatabase::MaterialSelect(vtkDataSet *ds, avtMaterial *mat,
                        vector<avtMixedVariable *> mvl,int dom,const char *var,
                        int ts, stringVector &mnames, stringVector &labels,
                        bool needInternalSurfaces,
                        bool needBoundarySurfaces,
                        bool needValidConnectivity,
                        bool needSmoothMaterialInterfaces,
                        bool needCleanZonesOnly,
                        bool reconstructionForced,
                        int  mirAlgorithm,
                        bool didGhosts,
                        bool &subdivisionOccurred,
                        bool &notAllCellsSubdivided,
                        bool reUseMIR)
{
    //
    // We need to have the material indices as well.
    //
    intVector mindex;
    GetMaterialIndices(mat, mnames, mindex);

    if (mat == NULL || ds == NULL || mat->GetNZones() !=ds->GetNumberOfCells())
    {
        char msg[128];
        SNPRINTF(msg,sizeof(msg),"The material object with nzones=%d and dataset "
            "object with ncells=%d do not agree.", mat->GetNZones(),
            ds->GetNumberOfCells());
        EXCEPTION1(InvalidDBTypeException, msg); 
    }

    //
    // Make room for the "mixed" material (i.e. for clean-zones-only)
    //
    mindex.push_back(mat->GetNMaterials());

    //
    // Determine the topological dimension.
    //
    avtDatabaseMetaData *md = GetMetaData(ts);
    string meshname = md->MeshForVar(var);
    const avtMeshMetaData *mmd = md->GetMesh(meshname);
    if (mmd == NULL)
    {
        return NULL;
    }
    int topoDim = mmd->topologicalDimension;

    void_ref_ptr vr_mir = GetMIR(dom, var, ts, ds, mat, topoDim,
                                 needValidConnectivity,
                                 needSmoothMaterialInterfaces,
                                 needCleanZonesOnly, mirAlgorithm,
                                 didGhosts,
                                 subdivisionOccurred,
                                 notAllCellsSubdivided, reUseMIR);
    MIR *mir = (MIR *) (*vr_mir);

    int numSelected = mindex.size();

    intVector selMats;
    int numOutput = (needInternalSurfaces ?  numSelected : 1);
    vtkDataSet **out_ds = new vtkDataSet *[numOutput];

    avtVarType type = GetMetaData(ts)->DetermineVarType(var);
    vtkIntArray *origDims = NULL;
    if (ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        int *dims = ((vtkStructuredGrid*)ds)->GetDimensions();
        origDims = vtkIntArray::New();
        origDims->SetName("vtkOriginalDimensions");
        origDims->SetNumberOfComponents(1);
        origDims->SetNumberOfTuples(3);
        origDims->SetValue(0, dims[0]);
        origDims->SetValue(1, dims[1]);
        origDims->SetValue(2, dims[2]);
    }
    else if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        int *dims = ((vtkRectilinearGrid*)ds)->GetDimensions();
        origDims = vtkIntArray::New();
        origDims->SetName("vtkOriginalDimensions");
        origDims->SetNumberOfComponents(1);
        origDims->SetNumberOfTuples(3);
        origDims->SetValue(0, dims[0]);
        origDims->SetValue(1, dims[1]);
        origDims->SetValue(2, dims[2]);
    }
   

    for (int d=0; d<numOutput; d++)
    {
        if (needInternalSurfaces)
        {
            selMats.clear();
            selMats.push_back(mindex[d]);
        }
        else
        {
            selMats = mindex;
        }

        bool  setUpMaterialVariable = false;
        if (type == AVT_MATERIAL || reconstructionForced)
        {
            setUpMaterialVariable = true;
        }

        out_ds[d] = mir->GetDataset(selMats, ds, mvl, setUpMaterialVariable,
                                    mat);

        if (out_ds != NULL && out_ds[d]->GetNumberOfCells() == 0)
        {
            out_ds[d]->Delete();
            out_ds[d] = NULL;
        }

        if (out_ds[d] && needBoundarySurfaces)
        {
            //
            // We need to extract the internal boundaries
            //
            if (out_ds[d]->GetDataObjectType() != VTK_UNSTRUCTURED_GRID)
            {
                EXCEPTION1(ImproperUseException, "MaterialSelect did not "
                           "get a VTK_UNSTRUCTURED_GRID\n")
            }

            vtkUnstructuredGridBoundaryFilter *bf =
                vtkUnstructuredGridBoundaryFilter::New();

            vtkDataSet *in_ds = out_ds[d];

            bf->SetInput((vtkUnstructuredGrid*)in_ds);
            in_ds->Delete();
            out_ds[d] = bf->GetOutput();
            bf->Update();

            out_ds[d]->Register(NULL);
            out_ds[d]->SetSource(NULL);
            bf->Delete();

            if (out_ds != NULL && out_ds[d]->GetNumberOfCells() == 0)
            {
                out_ds[d]->Delete();
                out_ds[d] = NULL;
            }

        }
        if (out_ds[d] && origDims)
        {
            out_ds[d]->GetFieldData()->AddArray(origDims);
            out_ds[d]->GetFieldData()->CopyFieldOn("vtkOriginalDimensions");
        }
    }
    if (origDims)
        origDims->Delete();

    //
    // Create new labels.
    //
    stringVector labelStrings;

    if (type == AVT_MATERIAL)
    {
        if (needInternalSurfaces)
        {
            labelStrings = labels;
            // add the one for the "mixed material"
            labelStrings.push_back("mixed");
        }
        else
        {
            //
            // Create a new label of the form "%d;%d;%s;...", which consists
            // of the number of materials, followed by pairs of material
            // number and material name for each material.  Make sure the last
            // one is for the "mixed material".
            //
            char   buff[32];
            string label;
            sprintf(buff, "%d;", numSelected);
            label += buff;
            for (int i = 0; i < numSelected; i++)
            {
                sprintf(buff, "%d;", mindex[i]);
                if (i == numSelected-1)
                    label += string(buff) + "mixed;";
                else
                    label += string(buff) + labels[i] + ";";
            }
            labelStrings.push_back(label);
        }
    }
    else
    {
        char label[40];
        avtSubsetType sT = GetMetaData(ts)->DetermineSubsetType(var);
        if (sT == AVT_DOMAIN_SUBSET || sT == AVT_UNKNOWN_SUBSET)
        {
            //
            // Create a new label which is the domain number.
            // We are doing a material-selected domain-subset plot,
            // or a material-selected non-subset plot. 
            //
            int domOrigin  =  GetMetaData(ts)->GetMesh(meshname)->blockOrigin;
            sprintf(label, "%d", dom + domOrigin);
            for (int d = 0; d < numOutput; d++)
            {
                labelStrings.push_back(label);
            }
        }
        else if (sT == AVT_GROUP_SUBSET)
        {
            //
            // Create a new label which is the group number.
            //
            int gID  =  GetMetaData(ts)->GetMesh(meshname)->groupIds[dom];
            sprintf(label, "%d", gID);
            for (int d = 0; d < numOutput; d++)
            {
                labelStrings.push_back(label);
            }
        }
    }

    avtDataTree_p outDT = new avtDataTree(numOutput, out_ds, dom, labelStrings);
    for (int i = 0 ; i < numOutput ; i++)
    {
        if (out_ds[i] != NULL)
        {
            out_ds[i]->Delete();
        }
    }
    delete [] out_ds;

    return outDT;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetMaterialIndices
//
//  Purpose:
//      Gets a list of material indices based on a list of material names.
//
//  Arguments:
//      mat     The material we should get the indices from.
//      mn      The material names.
//      ml      A place to put the material list.
//
//  Programmer: Hank Childs
//  Creation:   October 5, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Apr  9 09:02:57 PDT 2004
//    Use the avtMaterial for the correct timestep to get the indices.
//
//    Brad Whitlock, Wed Apr 14 12:14:42 PDT 2004
//    I fixed it for Windows.
//
//    Hank Childs, Wed Jul 28 15:27:45 PDT 2004
//    Make sure material is valid before moving on.
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector', and 'vector<string>' to 
//    'stringVector'.
//
// ****************************************************************************

void
avtGenericDatabase::GetMaterialIndices(avtMaterial *mat, stringVector &mn,
                                       intVector &ml)
{
    if (mat == NULL)
    {
        debug1 << "ERROR: The file format returned a NULL material object."
               << endl;
        EXCEPTION1(InvalidDBTypeException, "\"Unable to sucessfully read "
                                           "the material information.\"");
    }

    const stringVector &matlist = mat->GetCompleteMaterialList();

    int nstr = mn.size();
    for (int i = 0 ; i < nstr ; i++)
    {
        bool foundMatch = false;
        for (int j = 0 ; j < matlist.size() ; j++)
        {
            if (mn[i] == matlist[j])
            {
                ml.push_back(j);
                foundMatch = true;
                break;
            }
        }
        if (!foundMatch)
        {
            debug1 << "Was unable to match material " << mn[i].c_str() << " against "
                   << "any material for this dataset.  It is likely that this "
                   << "material exists at another timestep, but not at this "
                   << "one." << endl;
        }
    }
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetMaterial
//
//  Purpose:
//      Gets a material for a specific domain and a variable.  This handles
//      going through the auxiliary data mechanism and caching issues.
//
//  Arguments:
//      dom     The domain we want the material for.
//      var     A variable (could be vector, scalar, mesh, or material).
//      ts      The timestep of interest.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Apr 4 16:25:10 PST 2002
//    Changed CopyTo to an inline template function.
//
// ****************************************************************************

avtMaterial *
avtGenericDatabase::GetMaterial(int dom, const char *var, int ts)
{
    avtDatabaseMetaData *md = GetMetaData(ts);

    //
    // Identify the material we want.
    //
    string meshname = md->MeshForVar(var);
    string matname  = md->MaterialOnMesh(meshname);

    avtDataSpecification_p spec = new avtDataSpecification(matname.c_str(),
                                                           ts, dom);
    VoidRefList mats;
    GetAuxiliaryData(spec, mats, AUXILIARY_DATA_MATERIAL, NULL);
    if (mats.nList != 1)
    {
        EXCEPTION0(ImproperUseException);
    }
    
    avtMaterial *rv = (avtMaterial *) *(mats.list[0]);

    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetSpecies
//
//  Purpose:
//      Gets a species for a specific domain and a variable.  This handles
//      going through the auxiliary data mechanism and caching issues.
//
//  Arguments:
//      dom     The domain we want the species for.
//      var     A variable (could be vector, scalar, mesh, mat, or species).
//      ts      The timestep of interest.
//
//  Programmer: Jeremy Meredith
//  Creation:   December 17, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Apr 4 16:25:10 PST 2002
//    Changed CopyTo to an inline template function.
//
// ****************************************************************************

avtSpecies *
avtGenericDatabase::GetSpecies(int dom, const char *var, int ts)
{
    avtDatabaseMetaData *md = GetMetaData(ts);

    //
    // Identify the species we want.
    //
    string meshname = md->MeshForVar(var);
    string specname = md->SpeciesOnMesh(meshname);

    avtDataSpecification_p dspec = new avtDataSpecification(specname.c_str(),
                                                            ts, dom);
    VoidRefList specs;
    GetAuxiliaryData(dspec, specs, AUXILIARY_DATA_SPECIES, NULL);
    if (specs.nList != 1)
    {
        EXCEPTION0(ImproperUseException);
    }
    
    avtSpecies *rv = (avtSpecies *) *(specs.list[0]);

    return rv;
}

// ****************************************************************************
//  Method: avtGenericDatabase::GetGlobalNodeIds
//
//  Purpose:
//      Gets a global node numbers for a specific domain and a variable.
//      This handles going through the auxiliary data mechanism and caching
//      issues.
//
//  Arguments:
//      dom     The domain we want the species for.
//      var     A variable (could be vector, scalar, mesh, mat, or species).
//      ts      The timestep of interest.
//
//  Programmer: Mark C. Miller 
//  Creation:   August 5, 2004
//
//  Modifications:
//
//    Hank Childs, Thu Sep 23 09:48:24 PDT 2004
//    Name the global node ids array.
//
//    Kathleen Bonnell, Thu Dec  9 14:11:44 PST 2004 
//    Only set 'ContainsGlobalNodeIds' to true if the returned array
//    is not null.  Pass meshname instead of var to SetContainsGlobalNodeIds. 
//
// ****************************************************************************

vtkDataArray *
avtGenericDatabase::GetGlobalNodeIds(int dom, const char *var, int ts)
{
    avtDatabaseMetaData *md = GetMetaData(ts);

    //
    // Identify the mesh we want global node ids for
    //
    string meshname = md->MeshForVar(var);

    avtDataSpecification_p dspec = new avtDataSpecification(meshname.c_str(),
                                                            ts, dom);
    VoidRefList gnodeIds;
    GetAuxiliaryData(dspec, gnodeIds, AUXILIARY_DATA_GLOBAL_NODE_IDS, NULL);
    if (gnodeIds.nList > 1)
    {
        EXCEPTION0(ImproperUseException);
    }
    else if (gnodeIds.nList == 1)
    {
        vtkDataArray *arr = (vtkDataArray *) *(gnodeIds.list[0]);
        if (arr != NULL)
        {
            md->SetContainsGlobalNodeIds(meshname, true);
            arr->SetName("avtGlobalNodeNumbers");
        }
        return arr;
    }
    else
    {
        return NULL;
    }
}

// ****************************************************************************
//  Method: avtGenericDatabase::GetGlobalZoneIds
//
//  Purpose:
//      Gets a global zone numbers for a specific domain and a variable.
//      This handles going through the auxiliary data mechanism and caching
//      issues.
//
//  Arguments:
//      dom     The domain we want the species for.
//      var     A variable (could be vector, scalar, mesh, mat, or species).
//      ts      The timestep of interest.
//
//  Programmer: Mark C. Miller 
//  Creation:   August 5, 2004
//
//  Modifications:
//
//    Hank Childs, Thu Sep 23 09:48:24 PDT 2004
//    Name the global zone ids array.
//
//    Kathleen Bonnell, Thu Dec  9 14:11:44 PST 2004 
//    Only set 'ContainsGlobalZoneIds' to true if the returned array
//    is not null.  Pass meshname instead of var to SetContainsGlobalZoneIds. 
//
// ****************************************************************************

vtkDataArray *
avtGenericDatabase::GetGlobalZoneIds(int dom, const char *var, int ts)
{
    avtDatabaseMetaData *md = GetMetaData(ts);

    //
    // Identify the mesh we want global node ids for
    //
    string meshname = md->MeshForVar(var);

    avtDataSpecification_p dspec = new avtDataSpecification(meshname.c_str(),
                                                            ts, dom);
    VoidRefList gzoneIds;
    GetAuxiliaryData(dspec, gzoneIds, AUXILIARY_DATA_GLOBAL_ZONE_IDS, NULL);
    if (gzoneIds.nList > 1)
    {
        EXCEPTION0(ImproperUseException);
    }
    else if (gzoneIds.nList == 1)
    {
        vtkDataArray *arr = (vtkDataArray *) *(gzoneIds.list[0]);
        if (arr != NULL)
        {
            arr->SetName("avtGlobalZoneNumbers");
            md->SetContainsGlobalZoneIds(meshname, true);
        }
        return arr;
    }
    else
    {
        return NULL;
    }
}

// ****************************************************************************
//  Method: avtGenericDatabase::SpeciesSelect
//
//  Purpose:
//      Performs species selection.
//
//  Arguments:
//      dsc       The dataset collection.  This will be modified.
//      domains   The domain list.
//      specList  A list of species that are on and off.
//      spec      The data specification.
//      src       The source from a database.  Used for progress.
//
//  Programmer:   Hank Childs
//  Creation:     December 19, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Feb  8 08:48:06 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Thu Jul  4 14:53:05 PDT 2002
//    Reflect interface change that supports multiple mixed variables.
//
//    Hank Childs, Tue Sep 17 10:19:15 PDT 2002
//    Add support for all secondary variables.  Also fix memory leak.
//
//    Hank Childs, Tue Sep 24 08:25:28 PDT 2002
//    I screwed up the progress bar for species selection on my last checkin.
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector', and 'vector<bool>' to 'boolVector'.
//
// ****************************************************************************

void
avtGenericDatabase::SpeciesSelect(avtDatasetCollection &dsc, 
                      intVector &domains, boolVector &specList,
                      avtDataSpecification_p &spec, avtSourceFromDatabase *src)
{
    char *progressString = "Doing species selection";
    src->DatabaseProgress(0, 0, progressString);

    int timestep = spec->GetTimestep();

    int nDomains = dsc.GetNDomains();
    for (int i = 0 ; i < nDomains ; i++)
    {
        int m = 0; // Using 0 assumes that the file format does not do matsel. 
        vtkDataSet *ds = dsc.GetDataset(i, m);

        int n_cell_vars = ds->GetCellData()->GetNumberOfArrays();
        // If we don't make a copy of the cell data and then go mucking with
        // the order of the arrays, we may process some twice and not others.
        vtkCellData *inCD = vtkCellData::New();
        inCD->ShallowCopy(ds->GetCellData());
        for (int j = 0 ; j < n_cell_vars ; j++)
        {
            vtkDataArray *arr = inCD->GetArray(j);
            if (arr->GetDataType() != VTK_FLOAT ||
                arr->GetNumberOfComponents() != 1)
            {
                continue;
            }

            string var = arr->GetName();
            bool activeVar = false;
            if (ds->GetCellData()->GetScalars() == arr)
            {
                activeVar = true;
            }
            avtMaterial *mat = GetMaterial(domains[i], var.c_str(), timestep);
            avtSpecies *species = GetSpecies(domains[i],var.c_str(),timestep);

            avtMixedVariable *mixvar = (avtMixedVariable *)
                                                       *(dsc.GetMixVar(i,var));

            avtMixedVariable *mixVarOut = NULL;
            vtkDataArray     *scalarOut = NULL;
            MIR::SpeciesSelect(specList, mat, species, arr, mixvar, 
                               scalarOut, mixVarOut);
            ds->GetCellData()->RemoveArray(var.c_str());
            ds->GetCellData()->AddArray(scalarOut);
            if (activeVar)
            {
                ds->GetCellData()->SetActiveScalars(var.c_str());
            }
            scalarOut->Delete();
            if (mixVarOut != NULL)
            {
                void_ref_ptr vr = void_ref_ptr(mixVarOut,
                                               avtMixedVariable::Destruct);
                dsc.AddMixVar(i, vr);
            }
            src->DatabaseProgress(i*n_cell_vars+j, nDomains*n_cell_vars,
                                  progressString);
        }
        inCD->Delete();
    }

    src->DatabaseProgress(1, 0, progressString);
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetMIR
//
//  Purpose:
//      Gets the Material Interface Reconstruction (MIR), by first checking
//      to see if it was cached and returning the cached object if it was.
//      It then goes ahead and retrieves the material and reconstructs the
//      mesh if necessary.
//
//  Arguments:
//      domain    The domain of interest.
//      varname   The name of the variable (not necessarily the material name).
//      timestep  The timestep of interest.
//      ds        The dataset to reconstruct.
//      mat       The material.
//      topoDim   Whether this is a 3D mesh or 2D mesh.
//      needValidConnectivity
//                True if any subdivision must imply complete subdivision
//      reUseMIR  True if we can use this MIR on subsequent runs.
//
//  Output Arguments:
//      subdivisionOccurred              true if subdiv occurred in MIR
//      notAllCellsSubdivided            true if some cells were left whole
//
//  Returns:      The MIR object.
//
//  Programmer:   Hank Childs
//  Creation:     July 23, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Nov 26 09:06:07 PST 2001
//    Added material argument.  This allowed for the material to be read in
//    at the same time as the mesh, which allowed for ghost zone information
//    to be communicated.
//
//    Brad Whitlock, Thu Apr 4 16:25:10 PST 2002
//    Changed CopyTo to an inline template function.
//
//    Jeremy Meredith, Tue Aug 13 14:34:20 PDT 2002
//    Allowed leaving clean zones whole when possible.  This means
//    two *different* MIRs need to be in the cache.
//
//    Hank Childs, Sun Aug 18 22:33:35 PDT 2002
//    Add a boolean argument about whether we can re-use the MIR.
//
//    Hank Childs, Tue Sep 24 15:24:56 PDT 2002
//    Remove unneeded reference counting.
//
//    Hank Childs, Wed Oct  2 08:15:53 PDT 2002
//    Make the return type be a void_ref_ptr.
//
//    Jeremy Meredith, Thu Oct 24 15:36:34 PDT 2002
//    Added smoothing option and clean zones only option.
//
//    Hank Childs, Tue Jul 22 21:48:09 PDT 2003
//    Added a flag for whether or not we communicated ghosts.
//
//    Jeremy Meredith, Fri Sep  5 15:31:33 PDT 2003
//    Added the new MIR algorithm.
//
//    Jeremy Meredith, Thu Sep 18 11:31:23 PDT 2003
//    Made the new MIR algorithm work in 2D as well.
//
// ****************************************************************************
void_ref_ptr
avtGenericDatabase::GetMIR(int domain, const char *varname, int timestep,
                           vtkDataSet *ds, avtMaterial *mat, int topoDim,
                           bool needValidConnectivity,
                           bool needSmoothMaterialInterfaces,
                           bool needCleanZonesOnly, 
                           int  mirAlgorithm,
                           bool didGhosts,
                           bool &subdivisionOccurred,
                           bool &notAllCellsSubdivided, bool reUseMIR)
{
    char cacheLbl[1000];
    sprintf(cacheLbl, "MIR_%s_%s_%s_%s_%s",
            needValidConnectivity        ? "FullSubdiv" : "MinimalSubdiv",
            needSmoothMaterialInterfaces ? "Smooth"     : "NotSmooth",
            needCleanZonesOnly           ? "CleanOnly"  : "SplitMixed",
            didGhosts                    ? "DidGhosts"  : "NoDidGhosts",
            mirAlgorithm==0              ? "TetMIR"     : "ZooMIR");

    //
    // See if we already have the data lying around.
    //
    avtDatabaseMetaData *md = GetMetaData(timestep);
    string meshname = md->MeshForVar(varname);
    string matname  = md->MaterialOnMesh(meshname);
    void_ref_ptr vr = void_ref_ptr();
    if (reUseMIR)
    {
        vr = cache.GetVoidRef(matname.c_str(), cacheLbl, timestep, domain);
    }

    if (*vr == NULL)
    {
        //
        // We will need the material to calculate the reconstruction.
        //
        if (mat == NULL)
        {
            EXCEPTION0(NoInputException);
        }

        MIR *mir = NULL;

        //
        // Right new the new algorithm (index==1) is only
        // available in 3D.
        //
        if (mirAlgorithm == 1)
            mir = new ZooMIR;
        else
            mir = new TetMIR;

        mir->SetLeaveCleanZonesWhole(!needValidConnectivity);
        mir->SetSmoothing(needSmoothMaterialInterfaces);
        mir->SetCleanZonesOnly(needCleanZonesOnly);
        if (topoDim == 3)
        {
            mir->Reconstruct3DMesh(ds, mat);
        }
        else
        {
            mir->Reconstruct2DMesh(ds, mat);
        }

        //
        // Putting the MIR into the world of reference pointers saves some
        // headaches.
        //
        vr = void_ref_ptr(mir, MIR::Destruct);
        
        if (reUseMIR)
        {
            cache.CacheVoidRef(matname.c_str(), cacheLbl, timestep, domain,vr);
        }
    }

    MIR *rv = (MIR *) *vr;
    subdivisionOccurred   = rv->SubdivisionOccurred();
    notAllCellsSubdivided = rv->NotAllCellsSubdivided();
    return vr;
}


// ****************************************************************************
//  Method: avtGenericDatabase::PrepareMaterialSelect
//
//  Purpose:
//      Determines if we need to do a material selection.  Notifies the file
//      format interface of the materials if we should do a material selection,
//      so that it can do the material selection as we read.
//
//  Arguments:
//      dom          The domain.
//      forceMIROn   A boolean saying whether we should force MIR on.
//      silr         The SIL restriction.
//      mnames       A place to put the list of material names.
//      
//  Returns:    true if we need to perform material selection, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   October 5, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Nov 22 16:39:13 PST 2002
//    Use the SIL restriction traverser since SIL restriction routines were
//    antiquated.
//
//    Hank Childs, Wed Aug 13 08:09:20 PDT 2003
//    Do not longer key off type, instead use a flag that has been explicitly
//    set.
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<string>' to 'stringVector'.
//
// ****************************************************************************

bool
avtGenericDatabase::PrepareMaterialSelect(int dom, bool forceMIROn,
                      avtSILRestrictionTraverser &trav, stringVector &mnames)
{
    //
    // Have the SIL restriction determine if material selection is necessary
    // for this domain.
    //
    bool needMatSel;
    mnames = trav.GetMaterials(dom, needMatSel);

    //
    // If we were told to do material interface reconstruction, we should do
    // it, regardless of SIL.
    //
    needMatSel |= forceMIROn;

    return needMatSel;
}


// ****************************************************************************
//  Method: avtGenericDatabase::CanDoDynamicLoadBalancing
//
//  Purpose:
//      Determines whether or not we can do dynamic load balancing.
//
//  Programmer: Hank Childs
//  Creation:   October 25, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Feb 27 11:20:39 PST 2005
//    Added argument and significantly beefed up logic.
//
// ****************************************************************************

bool
avtGenericDatabase::CanDoDynamicLoadBalancing(avtDataSpecification_p dataspec)
{
    // 
    // Make sure the plugin has registered any domain boundary information
    // we need.
    //
    ActivateTimestep(dataspec->GetTimestep());

    //
    // If the plugin is doing collective communication, then we can't do DLB.
    //
    if (!Interface->CanDoDynamicLoadBalancing())
        return false;

    //
    // It's possible that we can't create ghost zones, even if asked.  If
    // this is the case, then there is no point in going further.
    //
    avtDatasetCollection emptyCollection(0);
    intVector emptyDomainList;
    avtDomainBoundaries *dbi = GetDomainBoundaryInformation(emptyCollection, 
                                                    emptyDomainList, dataspec);
    if (dbi == NULL)
        return true;

    //
    // Check to see if we need to create ghost data.  If so, then we may need 
    // to do cross-block communication, which will mean we can't do DLB.
    //
    avtSILRestrictionTraverser trav(dataspec->GetRestriction());
    avtGhostDataType gtype = dataspec->GetDesiredGhostDataType();
    if (gtype != GHOST_ZONE_DATA)
    {
        if (dataspec->MustDoMaterialInterfaceReconstruction())
            gtype = GHOST_ZONE_DATA;
        else
        {
            bool doMatSel = !trav.UsesAllMaterials();
            if (doMatSel)
                gtype = GHOST_ZONE_DATA;
        }
    }

    bool reqParCom = dbi->RequiresCommunication(gtype);
    bool canDoDLB = !reqParCom;
    return canDoDLB;
}


// ****************************************************************************
//  Method: avtGenericDatabase::HasInvariantMetaData
//
//  Purpose:
//      Indicates whether or not metadata can vary with time 
//
//  Programmer: Mark C. Miller 
//  Creation:   September 30, 2003 
//
// ****************************************************************************

bool
avtGenericDatabase::HasInvariantMetaData(void) const
{
    return Interface->HasInvariantMetaData();
}

// ****************************************************************************
//  Method: avtGenericDatabase::HasInvariantSIL
//
//  Purpose:
//      Indicates whether or not the SIL can vary with time 
//
//  Programmer: Mark C. Miller 
//  Creation:   September 30, 2003 
//
// ****************************************************************************

bool
avtGenericDatabase::HasInvariantSIL(void) const
{
    return Interface->HasInvariantSIL();
}

// ****************************************************************************
//  Method: avtGenericDatabase::ActivateTimestep
//
//  Purpose: Provide collective entry-point into database for preparing to
//  read a possibly new timestep
//
//  Programmer: Mark C. Miller 
//  Creation:   March 16, 2004 
//
// ****************************************************************************

void
avtGenericDatabase::ActivateTimestep(int stateIndex)
{
    Interface->ActivateTimestep(stateIndex);
}

// ****************************************************************************
//  Method: avtGenericDatabase::ReadDataset
//
//  Purpose:
//      Reads in a dataset through the file format interface.
//
//  Arguments:
//      ds        The dataset collection.
//      domains   A list of domains to be read.
//      spec      A data specification.
//      src       The source object.
//
//  Programmer:   Hank Childs
//  Creation:     October 26, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 28 13:01:00 PST 2001
//    Remove ghost zone arrays for filters where ghost zones are not
//    appropriate.
//
//    Hank Childs, Wed Dec  5 10:33:52 PST 2001
//    Read in mixed variables so that they can be communicated in the ghost
//    zone communication phase (if appropriate).
//
//    Hank Childs, Thu Jul  4 14:53:05 PDT 2002
//    Reflect interface change that supports multiple mixed variables.
//
//    Kathleen Bonnell, Wed Sep  4 14:57:31 PDT 2002 
//    Replace NeedDomainLabels with a check of subset type to determine
//    what kind of labels get created, if any. 
//
//    Jeremy Meredith, Thu Oct 24 16:04:22 PDT 2002
//    Reordered a big chunk of code to allow users to force material
//    interface reconstruction if they have mixed variables.
//
//    Hank Childs, Tue Oct 29 07:05:36 PST 2002
//    Changed the location of the declaration of the 'labels' string to inside
//    a for loop, ensuring that it will be properly cleaned out over each
//    iteration of the loop.
//
//    Hank Childs, Fri Nov 22 16:39:13 PST 2002
//    Use the SIL restriction traverser since SIL restriction routines were
//    antiquated.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003 
//    Initialize ContainsOriginalCells in the MetaData. 
//
//    Mark C. Miller, Mon Feb  9 15:41:57 PST 2004
//    Added call to new interface method, ActivateTimestep
//
//    Mark C. Miller, Mon Feb 23 20:38:47 PST 2004
//    Made call to ActivateTimestep accept the timestep as an argument
//
//    Mark C. Miller, Tue Mar 16 10:10:02 PST 2004
//    Made call to ActivateTimestep call this class' implementation instead
//    of referring to it through 'Interface->' explicitly
//    
//    Kathleen Bonnell, Fri May 28 18:31:15 PDT 2004 
//    Initialize ContainsOriginalNodes in the MetaData. 
//
//    Jeremy Meredith, Fri Jul  9 17:32:38 PDT 2004
//    It is possible to get multiple secondary variables in the data
//    specification and cause all sorts of problems (see '4798). I added a
//    method to get the secondary variable list without duplicating either the
//    primary variable or other secondary variables, and made use of it here.
//
//    Hank Childs, Wed Aug 11 08:14:16 PDT 2004
//    Do not simply remove the vtkGhostLevels array so we can pretend we don't
//    have ghost zones.
//    
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added call to RegisterDataSelections with the plugins
//
//    Kathleen Bonnell, Thu Dec  9 14:11:44 PST 2004 
//    Initialize ContainsGlobalNode/ZoneIds in the MetaData. 
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector', 'vector<bool>' to 'boolVector',
//    and 'vector<string>' to 'stringVector'.
//
//    Hank Childs, Tue Feb 15 07:21:10 PST 2005
//    Make translations when we have hidden characters.
//
// ****************************************************************************

void
avtGenericDatabase::ReadDataset(avtDatasetCollection &ds, intVector &domains,
                      avtDataSpecification_p &spec, avtSourceFromDatabase *src,
                      boolVector &selectionsApplied)
{
    int timerHandle = visitTimer->StartTimer();
    int ts = spec->GetTimestep();

    //
    // This will get unset later if we determine that we actually do have
    // ghosts.
    //
    avtDatabaseMetaData *md = GetMetaData(ts);
    string meshname = md->MeshForVar(spec->GetVariable());
    md->SetContainsGhostZones(meshname, AVT_NO_GHOSTS);
    md->SetContainsOriginalCells(meshname, false);
    md->SetContainsOriginalNodes(meshname, false);
    md->SetContainsGlobalZoneIds(meshname, false);
    md->SetContainsGlobalNodeIds(meshname, false);

    //
    // Set up some things we will want for later.
    //
    const char *var = spec->GetVariable();
    const vector<CharStrRef> &vars2nd = 
                               spec->GetSecondaryVariablesWithoutDuplicates();
    avtSILRestriction_p silr = spec->GetRestriction();

    char  progressString[1024];
    sprintf(progressString, "Reading from %s", Interface->GetType());

    stringVector blockNames;
    intVector gIds;
    int domOrigin = 0;

    avtSubsetType subT = GetMetaData(ts)->DetermineSubsetType(var);
    if (subT == AVT_DOMAIN_SUBSET || subT == AVT_GROUP_SUBSET)
    {
        string meshName = GetMetaData(ts)->MeshForVar(var);
        blockNames =  GetMetaData(ts)->GetMesh(meshName)->blockNames;
        domOrigin  =  GetMetaData(ts)->GetMesh(meshName)->blockOrigin;
        gIds       =  GetMetaData(ts)->GetMesh(meshName)->groupIds;
    }

    //
    // Some file formats have variables that are defined for only some of
    // the materials.  In that case, we have to tell the file format interface
    // of all of the variables we will be interested in.
    //
    const char *real_var = GetOriginalVariableName(md, var);
    vector<CharStrRef> real_vars2nd;
    int i;
    for (i = 0 ; i < vars2nd.size() ; i++)
    {
        const char *str = GetOriginalVariableName(md, *(vars2nd[i]));
        char *v2 = new char[strlen(str)+1];
        strcpy(v2, str);
        CharStrRef ref = v2;
        real_vars2nd.push_back(ref);
    }
    Interface->RegisterVariableList(real_var, vars2nd);

    //
    // Some file formats are interested in knowing about data selections
    //
    vector<avtDataSelection_p> selList = spec->GetAllDataSelections();
    for (i = 0; i < selList.size(); i++)
        selectionsApplied.push_back(false);
    Interface->RegisterDataSelections(selList, &selectionsApplied);

    //
    // Some file formats may need to engage in global communication when
    // changing time-steps. Provide that opportunity here 
    //
    ActivateTimestep(ts);

    //
    // Iterate through each of the domains and do material selection as we go.
    //
    src->DatabaseProgress(0, 0, progressString);
    int nDomains = domains.size();
    avtSILRestrictionTraverser trav(silr);
    for (i = 0 ; i < nDomains ; i++)
    {
        stringVector labels;
        stringVector matnames;
        bool forceMIR = spec->MustDoMaterialInterfaceReconstruction();
        bool doSelect = PrepareMaterialSelect(domains[i], forceMIR, trav, 
                                              matnames);
        int nmats = matnames.size();
        vtkDataSet *single_ds = NULL;

        if (!doSelect || !Interface->PerformsMaterialSelection())
        {
            // We know we want the dataset as a whole
            debug5 << "Generic database instructing get for var = " 
                   << var << ", timestep = " << ts << " domain = "
                   << domains[i] << endl;
            single_ds = GetDataset(var, ts, domains[i], "_all", vars2nd, src);

            // Determine if there are mixed vars.  If so, force
            // material selection if it was requested for mixed vars
            if (spec->NeedMixedVariableReconstruction())
            {
                bool hasmixvar = cache.HasVoidRef(var,
                                                  AUXILIARY_DATA_MIXED_VARIABLE,
                                                  ts, domains[i]);
                for (int kk = 0 ; kk < vars2nd.size() && !hasmixvar ; kk++)
                {
                    hasmixvar |= cache.HasVoidRef(*(vars2nd[kk]),
                                                  AUXILIARY_DATA_MIXED_VARIABLE,
                                                  ts, domains[i]);
                }

                if (hasmixvar)
                    doSelect = true;
            }

            // if we're not doing material selection for sure, set nmats to 1
            if (!doSelect)
                nmats = 1;
        }

        //
        //  Allocate space for our dataset
        //
        ds.SetNumMaterials(i, nmats);

        //
        //  Prepare labels; we need nmats labels.
        //
        if (subT == AVT_DOMAIN_SUBSET)
        { 
            if (blockNames.empty())
            {
                char temp[512];
                sprintf(temp, "%d", domains[i] + domOrigin);
                for (int l = 0; l < nmats; l++)
                {
                    labels.push_back(temp);
                }
            }
            else 
            {
                for (int l = 0; l < nmats; l++)
                    labels.push_back(blockNames[domains[i]]);
            }
        }
        else if (subT == AVT_GROUP_SUBSET)
        {
            char temp[512];
            sprintf(temp, "%d", gIds[domains[i]]);
            for (int l = 0; l < nmats; l++)
                labels.push_back(temp);
        }
        else if (subT == AVT_MATERIAL_SUBSET)
        {
            labels = matnames;
        }

        ds.labels[i] = labels;

        //
        //  Finish setting up the dataset
        //
        if (doSelect)
        {
            if (Interface->PerformsMaterialSelection())
            {
                ds.needsMatSelect[i] = false;

                //
                // This file format does its own material selection, so get the
                // dataset one material at a time.
                //
                for (int j = 0 ; j < nmats ; j++)
                {
                    debug5 << "Generic database instructing get for var = " 
                           << var << ", timestep = " << ts << " domain = "
                           << domains[i] << ", material = " 
                           << matnames[j].c_str() << endl;
                    vtkDataSet *ds1 = GetDataset(var, ts, domains[i],
                                          matnames[j].c_str(), vars2nd, src);
                    ds.SetDataset(i, j, ds1);
                    if (ds1 != NULL)
                    {
                        ds1->Delete();
                    }
                }
            }
            else
            {
                ds.needsMatSelect[i] = true;

                ds.SetDataset(i, 0, single_ds);
                if (single_ds != NULL)
                {
                    single_ds->Delete();
                }

                // Get the material
                ds.matnames[i] = matnames;
                avtMaterial *mat = GetMaterial(domains[i], var, ts);
                ds.SetMaterial(i, mat);

                // Get the mixed variables
                void_ref_ptr vr=cache.GetVoidRef(var,
                                                 AUXILIARY_DATA_MIXED_VARIABLE,
                                                 ts, domains[i]);
                if (*vr != NULL)
                {
                    ds.AddMixVar(i, vr);
                }
                for (int kk = 0 ; kk < vars2nd.size() ; kk++)
                {
                    vr = cache.GetVoidRef(*(vars2nd[kk]),
                                          AUXILIARY_DATA_MIXED_VARIABLE, ts,
                                          domains[i]);
                    if (*vr != NULL)
                    {
                         ds.AddMixVar(i, vr);
                    }
                }
            }

            src->InvalidateZones();
        }
        else
        {
            ds.needsMatSelect[i] = false;

            ds.SetDataset(i, 0, single_ds);
            if (single_ds != NULL)
            {
                single_ds->Delete();
            }
        }

        src->DatabaseProgress(i, nDomains, progressString);
    }

    src->DatabaseProgress(1, 0, progressString);

    visitTimer->StopTimer(timerHandle, "Reading dataset");
    visitTimer->DumpTimings();
}


// ****************************************************************************
//  Method: avtGenericDatabase::CommunicateGhosts
//
//  Purpose:
//      Uses a domain boundary information object to communicate the ghost
//      zones.
//
//  Arguments:
//      ghostType The type of ghost data to create.
//      ds        The dataset collection.
//      doms      A list of domains.
//      spec      A data specification.
//      src       The source object.
//
//  Returns:      True if ghost zones were communicated.  False otherwise.
//
//  Programmer:   Hank Childs
//  Creation:     October 26, 2001
//
//  Modifications:
//
//    Jeremy Meredith, Tue Dec  4 13:52:39 PST 2001
//    Added material and mixvar support.  Added parallel and progress support.
//
//    Jeremy Meredith, Fri Dec 21 12:25:29 PST 2001
//    Added species-scalar support.
//
//    Hank Childs, Wed Jan  2 08:35:34 PST 2002
//    Communicate secondary variables.
//
//    Hank Childs, Sun Jan 13 14:04:32 PST 2002
//    Clean up hack that resulted from poorly understood circumstances.
//
//    Hank Childs, Wed Mar 27 08:38:02 PST 2002
//    Make sure that the mesh we want to exchange ghost zones for is the mesh
//    that we have boundary information for.
//
//    Kathleen Bonnell, Wed Mar 27 15:47:14 PST 2002   
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Fri May 17 12:52:08 PDT 2002  
//    Make sure avtOriginalCellNumbers is exhanged if needed. 
//    
//    Hank Childs, Tue Jun 25 20:34:15 PDT 2002
//    Copy over the field data.
//
//    Hank Childs, Thu Jul  4 14:53:05 PDT 2002
//    Reflect interface change that supports multiple mixed variables.
//
//    Hank Childs, Wed Jul 10 11:33:20 PDT 2002
//    A reference was being removed from a scalar variable that should not
//    have been.
//
//    Kathleen Bonnell, Wed Jul 10 16:02:56 PDT 2002 
//    Copy FieldData arrays individually, as ExchangeMesh may set
//    an array in the NewList, and ShallowCopy will overwrite that.  
//   
//    Hank Childs, Sun Aug 18 22:33:35 PDT 2002
//    Added return value.
//
//    Hank Childs, Mon Sep 30 09:29:36 PDT 2002
//    Tell output that we have ghost zones.
//
//    Hank Childs, Wed Oct  2 10:09:53 PDT 2002
//    Also communicate the original zone numbers if we are creating structured
//    indices.
//
//    Hank Childs, Wed Jun 18 09:34:38 PDT 2003
//    Communicate node numbers.
//
//    Kathleen Bonnell, Wed Jun 18 17:47:10 PDT 2003  
//    When telling downstream items that ghost zones are present, use
//    AVT_CREATED_GHOSTS to distinguish between those designated by the file
//    format (AVT_HAS_GHOSTS) and those created here. (Needed currently so
//    that Pick can return the correct cell id and/or coords).
//
//    Hank Childs, Mon Oct 20 16:45:11 PDT 2003
//    Allow for materials and meshes to be specified (and then ignored).
//    They are typically specified as a by-product of expressions.
//
//    Hank Childs, Wed Nov 12 10:37:22 PST 2003
//    Also check to see if there is per-timestep domain boundary information,
//    since this can change from timestep-to-timestep for AMR meshes.
//
//    Hank Childs, Sun Jun 27 11:02:42 PDT 2004
//    Add support for identifying ghost nodes using global indexing.
//
//    Jeremy Meredith, Fri Jul  9 17:32:38 PDT 2004
//    It is possible to get multiple secondary variables in the data
//    specification and cause all sorts of problems (see '4798). I added a
//    method to get the secondary variable list without duplicating either the
//    primary variable or other secondary variables, and made use of it here.
//
//    Mark C. Miller, Mon Aug  9 19:12:24 PDT 2004
//    Removed avtMeshMetadata local variable.
//    Changed logic for testing existence of global node ids to attempt to
//    Get global node ids and if they exist, use them.
//
//    Mark C. Miller, Tue Aug 10 14:16:36 PDT 2004
//    Added check for if ghosts had been read from file and return immediately
//
//    Mark C. Miller, Wed Aug 11 14:41:06 PDT 2004
//    Moved check for if ghost had been read from file to just before ghost
//    node stuff. Made it contribute to results for shouldStop
//    
//    Mark C. Miller, Mon Aug 16 15:01:27 PDT 2004
//    Fixed missing initialization of boolean haveGlobalNodeIds
//    Removed extraneous debug statements
//    
//    Hank Childs, Sat Aug 14 06:41:00 PDT 2004
//    Allow for ghost nodes to be created.  Put real work in subroutines.
//
//    Brad Whitlock, Thu Sep 16 11:04:57 PDT 2004
//    Removed a stray semicolon.
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector'.
//
//    Hank Childs, Sun Feb 27 14:47:45 PST 2005
//    Added "allDomains" argument.
//
//    Hank Childs, Sun Mar  6 09:14:52 PST 2005
//    Only do collective communication if we are not in DLB mode.
//
// ****************************************************************************

bool
avtGenericDatabase::CommunicateGhosts(avtGhostDataType ghostType, 
                      avtDatasetCollection &ds, intVector &doms,
                      avtDataSpecification_p &spec, avtSourceFromDatabase *src,
                      intVector &allDomains, bool canDoCollectiveCommunication)
{
    int portion1 = visitTimer->StartTimer();

    int  i;

    int ts = spec->GetTimestep();
    avtDatabaseMetaData *md = GetMetaData(ts);
    const char *varname = spec->GetVariable();
    string meshname = md->MeshForVar(varname);

    //
    // We may already have ghost data from reading it out of the file.  If this
    // is the case, just return now.
    //
    int shouldStop = 0;
    if (md->GetContainsGhostZones(meshname) == AVT_HAS_GHOSTS)
        shouldStop = 1;
#ifdef PARALLEL
    if (canDoCollectiveCommunication)
    {
        int  parallelShouldStop;
        MPI_Allreduce(&shouldStop, &parallelShouldStop, 1, MPI_INT, MPI_MAX,
                      MPI_COMM_WORLD);
        shouldStop = parallelShouldStop;
    }
#endif
    if (shouldStop != 0)
        return false;

    avtDomainBoundaries *dbi = GetDomainBoundaryInformation(ds, doms, spec);
    bool hasDomainBoundaryInfo = (dbi != NULL);
#ifdef PARALLEL
    if (canDoCollectiveCommunication)
    {
        int hdbi = (hasDomainBoundaryInfo ? 1 : 0);
        int phdbi;
        MPI_Allreduce(&hdbi, &phdbi, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
        if (phdbi < 1)
            hasDomainBoundaryInfo = false;
    }
#endif

    //
    // Determine if we have global node ids.
    //
    bool haveDomainWithoutGlobalNodeIds = false;
    bool haveGlobalNodeIdsForAtLeastOneDom = false;
    for (i = 0 ; i < doms.size() ; i++)
    {
        if (GetGlobalNodeIds(doms[i], meshname.c_str(), ts) == NULL)
            haveDomainWithoutGlobalNodeIds = true;
        else
            haveGlobalNodeIdsForAtLeastOneDom = true;
    }
#ifdef PARALLEL
    if (canDoCollectiveCommunication)
    {
        int tmp1[2], tmp2[2];
        tmp1[0] = (haveDomainWithoutGlobalNodeIds ? 1 : 0);
        tmp1[1] = (haveGlobalNodeIdsForAtLeastOneDom ? 1 : 0);
        MPI_Allreduce(tmp1, tmp2, 2, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
        haveDomainWithoutGlobalNodeIds = (tmp2[0] == 1 ? true : false);
        haveGlobalNodeIdsForAtLeastOneDom = (tmp2[1] == 1 ? true : false);
    }
#endif
    bool canUseGlobalNodeIds = !haveDomainWithoutGlobalNodeIds &&
                               haveGlobalNodeIdsForAtLeastOneDom;

    visitTimer->StopTimer(portion1, "Prepatory time for ghost zone creation."
                                    "  This also counts synchronization.");
    int portion2 = visitTimer->StartTimer();

    //
    // Now its decision time.  We know what tools we can use -- whether or not
    // we have domain boundary information and whether or not we can use global
    // node ids.  We also know what kind of ghost data we need.  So call the
    // proper subroutine to do it.
    //
    bool s = false;
    if (ghostType == GHOST_NODE_DATA)
    {
        if (hasDomainBoundaryInfo)
            s = CommunicateGhostNodesFromDomainBoundariesFromFile(ds, doms,
                                                       spec, src, allDomains);
        else if (canUseGlobalNodeIds)
            s = CommunicateGhostNodesFromGlobalNodeIds(ds, doms, spec, src);
    }
    else if (ghostType == GHOST_ZONE_DATA)
    {
        if (hasDomainBoundaryInfo)
            s = CommunicateGhostZonesFromDomainBoundariesFromFile(ds, doms, 
                                                                  spec, src);
        else if (canUseGlobalNodeIds)
            s = CommunicateGhostZonesFromGlobalNodeIds(ds, doms, spec, src);
    }
    else
    {
        debug1 << "Internal error: asked to communicate ghost data, but ghost "
               << "type is neither ghost zones or ghost nodes." << endl;
    }
    visitTimer->StopTimer(portion2, "Time to actually communicate ghost data");
    
    bool madeGhosts = s;
    if (madeGhosts)
    {
        // 
        // This will tell everything downstream that we have created ghost
        // zones.
        //
        md->SetContainsGhostZones(meshname, AVT_CREATED_GHOSTS);
    }

    bool madeNewZones = madeGhosts && (ghostType == GHOST_ZONE_DATA);
    return madeNewZones;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetDomainBoundaryInformation
//
//  Purpose:
//      Gets the domain boundary information and confirms that the DBI actually
//      applies to the mesh we have.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector'.
//
// ****************************************************************************

avtDomainBoundaries *
avtGenericDatabase::GetDomainBoundaryInformation(avtDatasetCollection &ds,
                                                 intVector &doms,
                                                 avtDataSpecification_p spec)
{
    //
    // Try getting the domain boundary information.  If we don't have it for
    // *any* timestep, try getting it for *this* timestep (it can change with
    // AMR datasets).
    //
    void_ref_ptr vr = cache.GetVoidRef("any_mesh",
                                   AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                                  -1, -1);
    if (*vr == NULL)
        vr = cache.GetVoidRef("any_mesh",
                              AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                              spec->GetTimestep(), -1);

    //
    // If we couldn't find DBI, then we are done, so return.
    //
    avtDomainBoundaries *dbi = (avtDomainBoundaries*)*vr;
    if (dbi == NULL)
        return NULL;

    //
    // Make sure that this mesh is the mesh we have boundary information
    // for.
    //
    vector<vtkDataSet *> confirmlist;
    for (int i = 0 ; i < doms.size() ; i++)
    {
        confirmlist.push_back(ds.GetDataset(i, 0));
    }
    bool haveRightMesh = dbi->ConfirmMesh(doms, confirmlist);
    if (!haveRightMesh)
        dbi = NULL;

    return dbi;
}


// ****************************************************************************
//  Method: avtGenericDatabase::
//                            CommunicateGhostZonesFromDomainBoundariesFromFile
//
//  Purpose:
//      Creates ghost zones using domain boundary information that comes from
//      a file.
//
//  Arguments:
//      ds        The dataset collection.
//      doms      A list of domains.
//      spec      A data specification.
//      src       The source object.
//
//  Notes:      This routine used to be wholely contained by CommunicateGhosts.
//
//  Programmer: Hank Childs
//  Creation:   August 13, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector'.
//
//    Hank Childs, Sun Mar  6 09:14:52 PST 2005
//    Removed parallel error checking, since all of that error checking is
//    already performed by the calling function.
//
// ****************************************************************************

bool
avtGenericDatabase::CommunicateGhostZonesFromDomainBoundariesFromFile(
                      avtDatasetCollection &ds, intVector &doms,
                      avtDataSpecification_p &spec, avtSourceFromDatabase *src)
{
    //
    //  Setup
    //
    char  progressString[1024] = "Calculating ghost zones";
    src->DatabaseProgress(0, 0, progressString);
    src->DatabaseProgress(0, 100,
                     "Calculating ghost zones: waiting for all MPI tasks");

    //
    // Obtain some info about what we are operating on.
    //
    int ts = spec->GetTimestep();
    avtDatabaseMetaData *md = GetMetaData(ts);
    const char *varname = spec->GetVariable();
    string meshname = md->MeshForVar(varname);

    avtDomainBoundaries *dbi = GetDomainBoundaryInformation(ds, doms, spec);

    //
    // This method should never be called if the domain boundary information
    // doesn't exist, but make sure just in case.  If this does happen, we
    // could get parallel deadlock.
    //
    if (dbi == NULL)
    {
        debug1 << "Not applying ghost zones because the boundary "
               << "information does not apply to this mesh." << endl;
        return false;
    }

    return CommunicateGhostZonesFromDomainBoundaries(dbi, ds, doms, spec, src);
}


// ****************************************************************************
//  Method: avtGenericDatabase::CommunicateGhostZonesFromDomainBoundaries
//
//  Purpose:
//      Creates ghost zones using domain boundary information.
//
//  Arguments:
//      ds        The dataset collection.
//      doms      A list of domains.
//      spec      A data specification.
//      src       The source object.
//
//  Notes:      This routine used to be wholely contained by CommunicateGhosts.
//
//  Programmer: Hank Childs
//  Creation:   August 17, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector'.
//
//    Hank Childs, Sun Mar 13 10:47:59 PST 2005
//    Fix memory leak.
//
// ****************************************************************************

bool
avtGenericDatabase::CommunicateGhostZonesFromDomainBoundaries(
                      avtDomainBoundaries *dbi,
                      avtDatasetCollection &ds, intVector &doms,
                      avtDataSpecification_p &spec, avtSourceFromDatabase *src)
{
    int   i, j, k;

    int t1 = visitTimer->StartTimer();

    int   localstage;
    int   nlocalstage;
    char  progressString[1024] = "Calculating ghost zones";

    //
    // Obtain some info about what we are operating on.
    //
    int ts = spec->GetTimestep();
    avtDatabaseMetaData *md = GetMetaData(ts);
    const char *varname = spec->GetVariable();
    avtVarType type = md->DetermineVarType(varname);
    string meshname = md->MeshForVar(varname);

    // Setup materials
    int anymats    = false;
    int allmats    = true; 
    int most_mixvars = 0;
    const int didnt_get_any = 100;
    int least_mixvars = didnt_get_any; 
    for (i = 0 ; i < doms.size() ; i++)
    {
        avtMaterial *mat = ds.GetMaterial(i);
        if (mat)
            anymats = true;
        else
            allmats = false;

        if (mat != NULL && mat->GetMixlen() > 0)
        {
            int num = ds.GetAllMixVars(i).size();
            most_mixvars = (most_mixvars > num ? most_mixvars : num);
            least_mixvars = (least_mixvars < num ? least_mixvars : num);
        }
    }

#ifdef PARALLEL
    int anymats_tmp    = anymats;
    int allmats_tmp    = allmats; 
    int least_mix_tmp  = least_mixvars;
    int most_mix_tmp   = most_mixvars;
    MPI_Allreduce(&anymats_tmp, &anymats,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
    MPI_Allreduce(&allmats_tmp, &allmats,1,MPI_INT,MPI_MIN,MPI_COMM_WORLD);
    MPI_Allreduce(&most_mix_tmp, &most_mixvars, 1, MPI_INT, MPI_MAX,
                  MPI_COMM_WORLD);
    MPI_Allreduce(&least_mix_tmp, &least_mixvars, 1, MPI_INT, MPI_MIN,
                  MPI_COMM_WORLD);
#endif

    if (anymats && !allmats)
        EXCEPTION1(VisItException, "Material selection must be performed "
                   "on all domains.  Please see a VisIt developer.");
    if (least_mixvars != didnt_get_any)
    {
        if (most_mixvars != least_mixvars)
        {
            debug1 << "Not all of the domains have the same number of "
                   << "mixvars." << endl;
            debug1 << "Most_mixvars = " << most_mixvars << endl;
            debug1 << "Least_mixvars = " << least_mixvars << endl;
            EXCEPTION1(VisItException, "Mixed variables must be defined "
                       "on all domains that have mixed materials. "
                       " Please see a VisIt developer.");
        }
    }
    int nummixvars = most_mixvars;

    localstage  = 1;
    nlocalstage = (1 + 
                   ((type==AVT_SCALAR_VAR || type==AVT_VECTOR_VAR) ? 1:0) +
                   (anymats ? 1:0) + 
                   (nummixvars > 0 ? 1:0) +
                   (spec->NeedZoneNumbers()||spec->NeedStructuredIndices()
                            ? 1: 0));

    //
    //  Exchange Meshes
    //
    src->DatabaseProgress(localstage++, nlocalstage,
                          "Creating ghost zones for meshes");

    vector<vtkDataSet *> list;
    for (i = 0 ; i < doms.size() ; i++)
    {
        list.push_back(ds.GetDataset(i, 0));
        list[i]->Register(NULL);
    }
    vector<vtkDataSet *> newList = dbi->ExchangeMesh(doms, list);

    //
    //  Copy FieldData arrays indivdually, as there may be
    //  some already in existence in newList.
    //
    for (i = 0 ; i < doms.size() ; i++)
    {
        vtkFieldData *fd = list[i]->GetFieldData();
        for (k = 0; k < fd->GetNumberOfArrays(); k++)
        {
            newList[i]->GetFieldData()->AddArray(fd->GetArray(k));
        }
        ds.SetDataset(i, 0, newList[i]);
        newList[i]->Delete();
    }

    //
    //  Exchange Variables
    //
    if (type == AVT_SCALAR_VAR || type == AVT_MATSPECIES)
    {
        src->DatabaseProgress(localstage++, nlocalstage,
                              "Creating ghost zones for scalars");

        avtCentering centering;
        if (type == AVT_MATSPECIES)
            centering = AVT_ZONECENT;
        else
            centering = GetMetaData(ts)->GetScalar(varname)->centering;

        bool isPointData = (centering == AVT_NODECENT ? true : false);
        vector<vtkDataArray *> scalars;
        for (i = 0 ; i < doms.size() ; i++)
        {
            vtkDataSet *ds1 = list[i];
            vtkDataArray *s   = NULL;
            if (centering == AVT_NODECENT)
            {
                s = ds1->GetPointData()->GetScalars();
            }
            else
            {
                s = ds1->GetCellData()->GetScalars();
            }
            scalars.push_back(s);
        }
        vector<vtkDataArray *> scalarsOut;
        scalarsOut = dbi->ExchangeScalar(doms, isPointData, scalars);
        for (i = 0 ; i < doms.size() ; i++)
        {
            vtkDataSet *ds1 = ds.GetDataset(i, 0);
            vtkDataArray *s   = scalarsOut[i];
            if (centering == AVT_NODECENT)
            {
                ds1->GetPointData()->SetScalars(s);
            }
            else
            {
                ds1->GetCellData()->SetScalars(s);
            }
            s->Delete();
        }
    }

    if (type == AVT_VECTOR_VAR)
    {
        src->DatabaseProgress(localstage++, nlocalstage,
                              "Creating ghost zones for vectors");

        const avtVectorMetaData *vmd = GetMetaData(ts)->GetVector(varname);
        bool isPointData = (vmd->centering == AVT_NODECENT ? true : false);
        vector<vtkDataArray *> vectors;
        for (i = 0 ; i < doms.size() ; i++)
        {
            vtkDataSet *ds1 = list[i];
            vtkDataArray *s   = NULL;
            if (vmd->centering == AVT_NODECENT)
            {
                s = ds1->GetPointData()->GetVectors();
            }
            else
            {
                s = ds1->GetCellData()->GetVectors();
            }
            vectors.push_back(s);
        }
        vector<vtkDataArray *> vectorsOut;
        vectorsOut = dbi->ExchangeFloatVector(doms, isPointData, vectors);
        for (i = 0 ; i < doms.size() ; i++)
        {
            vtkDataSet *ds1 = ds.GetDataset(i, 0);
            vtkDataArray *s   = vectorsOut[i];
            if (vmd->centering == AVT_NODECENT)
            {
                ds1->GetPointData()->SetVectors(s);
            }
            else
            {
                ds1->GetCellData()->SetVectors(s);
            }
            s->Delete();
        }
    }

    //
    // Exchange secondary variables.
    //
    const vector<CharStrRef> &var2nd = 
                           spec->GetSecondaryVariablesWithoutDuplicates();
    avtDatabaseMetaData *metadata = GetMetaData(ts);
    for (i = 0 ; i < var2nd.size() ; i++)
    {
        CharStrRef curVar  = var2nd[i];
        avtVarType varType = metadata->DetermineVarType(*curVar);
        switch (varType)
        {
          case AVT_SCALAR_VAR:
            {
                avtCentering centering = metadata->GetScalar(*curVar)
                                                               ->centering;
                bool isPointData = (centering == AVT_NODECENT ? true : false);
                vector<vtkDataArray *> scalars;
                for (j = 0 ; j < doms.size() ; j++)
                {
                    vtkDataSet *ds1 = list[j];
                    vtkDataSetAttributes *atts = NULL;
                    if (isPointData)
                    {
                        atts = ds1->GetPointData();
                    }
                    else
                    {
                        atts = ds1->GetCellData();
                    }
                    scalars.push_back(atts->GetArray(*curVar));
                }
                vector<vtkDataArray *> scalarsOut;
                scalarsOut = dbi->ExchangeScalar(doms,isPointData,scalars);
                for (j = 0 ; j < doms.size() ; j++)
                {
                    vtkDataSet *ds1 = ds.GetDataset(j, 0);
                    vtkDataSetAttributes *atts = NULL;
                    if (isPointData)
                    {
                        atts = ds1->GetPointData();
                    }
                    else
                    {
                        atts = ds1->GetCellData();
                    }
                    atts->AddArray(scalarsOut[j]);
                    scalarsOut[j]->Delete();
                }
            }
            break;
          case AVT_MATSPECIES:
            {
                vector<vtkDataArray *> scalars;
                for (j = 0 ; j < doms.size() ; j++)
                {
                    scalars.push_back(list[j]->GetCellData()->GetArray(
                                                                 *curVar));
                }
                vector<vtkDataArray *> scalarsOut;
                scalarsOut = dbi->ExchangeScalar(doms, false, scalars);
                for (j = 0 ; j < doms.size() ; j++)
                {
                    vtkDataSet *ds1 = ds.GetDataset(j, 0);
                    ds1->GetCellData()->AddArray(scalarsOut[j]);
                    scalarsOut[j]->Delete();
                }
            }
            break;
          case AVT_VECTOR_VAR:
            {
                avtCentering centering = metadata->GetVector(*curVar)
                                                               ->centering;
                bool isPointData = (centering == AVT_NODECENT ? true : false);
                vector<vtkDataArray *> vectors;
                for (j = 0 ; j < doms.size() ; j++)
                {
                    vtkDataSet *ds1 = list[j];
                    vtkDataSetAttributes *atts = NULL;
                    if (isPointData)
                    {
                        atts = ds1->GetPointData();
                    }
                    else
                    {
                        atts = ds1->GetCellData();
                    }
                    vectors.push_back(atts->GetArray(*curVar));
                }
                vector<vtkDataArray *> vectorsOut;
                vectorsOut = dbi->ExchangeFloatVector(doms,isPointData,
                                                      vectors);
                for (j = 0 ; j < doms.size() ; j++)
                {
                    vtkDataSet *ds1 = ds.GetDataset(j, 0);
                    vtkDataSetAttributes *atts = NULL;
                    if (isPointData)
                    {
                        atts = ds1->GetPointData();
                    }
                    else
                    {
                        atts = ds1->GetCellData();
                    }
                    atts->AddArray(vectorsOut[j]);
                    vectorsOut[j]->Delete();
                }
            }
            break;

          case AVT_MATERIAL:
          case AVT_MESH:
            // These typically come about because of expressions.
            // Just ignore -- it will be handled elsewhere.
            continue;

          default:
            EXCEPTION1(VisItException, "Cannot exchange secondary "
                     "variables that aren't scalars, vectors, or species");
        }
    }

    //
    //  Exchange Materials and Mixed Variables
    //
    if (anymats)
    {
        src->DatabaseProgress(localstage++, nlocalstage,
                              "Creating ghost zones for materials");

        // materials
        vector<avtMaterial*> matList;
        for (i = 0 ; i < doms.size() ; i++)
            matList.push_back(ds.GetMaterial(i));

        vector<avtMaterial*> newMatList = dbi->ExchangeMaterial(doms, matList);

        for (i = 0 ; i < doms.size() ; i++)
            ds.SetMaterial(i, newMatList[i]);
        ds.MaterialsShouldBeFreed();

        // mixvars
        if (nummixvars > 0)
        {
            src->DatabaseProgress(localstage++, nlocalstage,
                                  "Creating ghost zones for mixed vars");
            for (i = 0 ; i < nummixvars ; i++)
            {
                vector<avtMixedVariable*> mixvarList;
                int numDomains = doms.size();
                for (j = 0 ; j < numDomains ; j++)
                {
                    avtMaterial *mat = matList[j];
                    if (mat != NULL && mat->GetMixlen() > 0)
                    {
                        avtMixedVariable *mv = (avtMixedVariable *)
                                                 *(ds.GetAllMixVars(j)[i]);
                        mixvarList.push_back(mv);
                    }
                    else
                    {
                        mixvarList.push_back(NULL);
                    }
                }

                vector<avtMixedVariable*> newMixvarList = 
                                dbi->ExchangeMixVar(doms, matList, mixvarList);

                for (j = 0 ; j < doms.size() ; j++)
                    if (newMixvarList[j] != NULL)
                    {
                        void_ref_ptr vr = void_ref_ptr(newMixvarList[j],
                                              avtMixedVariable::Destruct);
                        ds.ReplaceMixVar(j, vr);
                    }
            }
        }
    }

    //
    // Exchange OriginalNodes Arrays.
    //
    if (spec->NeedNodeNumbers())
    {
        vector<vtkDataArray *> nodeNums;
        for (j = 0 ; j < doms.size() ; j++)
        {
            vtkDataSet *ds1 = list[j];
            nodeNums.push_back(ds1->GetPointData()->GetArray(
                                                "avtOriginalNodeNumbers"));
        }
        vector<vtkDataArray *> nodeNumsOut;
        nodeNumsOut = dbi->ExchangeIntVector(doms,true,nodeNums);
        for (j = 0 ; j < doms.size() ; j++)
        {
            vtkDataSet *ds1 = ds.GetDataset(j, 0);
            ds1->GetPointData()->AddArray(nodeNumsOut[j]);
            nodeNumsOut[j]->Delete();
        }
    }

    //
    // Exchange OriginalCells Arrays.
    //
    if (spec->NeedZoneNumbers() || spec->NeedStructuredIndices())
    {
        vector<vtkDataArray *> cellNums;
        for (j = 0 ; j < doms.size() ; j++)
        {
            vtkDataSet *ds1 = list[j];
            cellNums.push_back(ds1->GetCellData()->GetArray(
                                                "avtOriginalCellNumbers"));
        }
        vector<vtkDataArray *> cellNumsOut;
        cellNumsOut = dbi->ExchangeIntVector(doms,false,cellNums);
        for (j = 0 ; j < doms.size() ; j++)
        {
            vtkDataSet *ds1 = ds.GetDataset(j, 0);
            ds1->GetCellData()->AddArray(cellNumsOut[j]);
            cellNumsOut[j]->Delete();
        }
    }

    //
    // We added references to the mesh with no ghost zones so that we could
    // reference it through the communication.  Now that we are done,
    // remove those references.
    //
    for (i = 0 ; i < doms.size() ; i++)
    {
        list[i]->Delete();
    }

    src->DatabaseProgress(1, 0, progressString);

    visitTimer->StopTimer(t1, "Actual communication of ghost zones");
    return true;
}


// ****************************************************************************
//  Method: avtGenericDatabase::
//                            CommunicateGhostNodesFromDomainBoundariesFromFile
//
//  Purpose:
//      Creates ghost nodes using domain boundary information.
//
//  Arguments:
//      ds           The dataset collection.
//      doms         A list of domains.
//      spec         A data specification.
//      src          The source object.
//      allDomains   A list of all domains being used.  Important for DLB.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector'.
//
//    Hank Childs, Sun Feb 27 14:47:45 PST 2005
//    Added allDomains argument.
//
//    Hank Childs, Sun Mar  6 09:14:52 PST 2005
//    Removed parallel error checking, since all of that error checking is
//    already performed by the calling function.
//
// ****************************************************************************

bool
avtGenericDatabase::CommunicateGhostNodesFromDomainBoundariesFromFile(
                      avtDatasetCollection &ds, intVector &doms,
                      avtDataSpecification_p &spec, avtSourceFromDatabase *src,
                      intVector &allDomains)
{
    //
    //  Setup
    //
    char  progressString[1024] = "Calculating ghost nodes";
    src->DatabaseProgress(0, 0, progressString);
    src->DatabaseProgress(0, 100,
                     "Calculating ghost nodes: waiting for all MPI tasks");

    //
    // Obtain some info about what we are operating on.
    //
    int ts = spec->GetTimestep();
    avtDatabaseMetaData *md = GetMetaData(ts);
    const char *varname = spec->GetVariable();
    string meshname = md->MeshForVar(varname);

    avtDomainBoundaries *dbi = GetDomainBoundaryInformation(ds, doms, spec);

    //
    // This method should never be called if the domain boundary information
    // doesn't exist, but make sure just in case.  If this does happen, we
    // could get parallel deadlock.
    //
    if (dbi == NULL)
    {
        debug1 << "Not applying ghost nodes because the boundary "
               << "information does not apply to this mesh." << endl;
        return false;
    }

    vector<vtkDataSet *> list;
    for (int i = 0 ; i < doms.size() ; i++)
        list.push_back(ds.GetDataset(i, 0));
    dbi->CreateGhostNodes(doms, list, allDomains);

    src->DatabaseProgress(1, 0, progressString);

    return true;
}


// ****************************************************************************
//  Method: avtGenericDatabase::CommunicateGhostZonesFromGlobalNodeIds
//
//  Purpose:
//      Creates ghost zones using global node ids.
//
//  Arguments:
//      ds        The dataset collection.
//      doms      A list of domains.
//      spec      A data specification.
//      src       The source object.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2004
//
//  Modifications:
//
//    Hank Childs, Wed Sep 29 16:55:26 PDT 2004
//    Update name of global node numbers, since name was recently changed
//    out from underneath us.
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector'.
//
//    Hank Childs, Mon Apr  4 13:21:09 PDT 2005
//    Fix problem where curDisp was being deleted twice in parallel.
//
//    Mark C. Miller, Thu Apr 21 09:37:41 PDT 2005
//    Fixed leak for 'ln' object
// ****************************************************************************

bool
avtGenericDatabase::CommunicateGhostZonesFromGlobalNodeIds(
                      avtDatasetCollection &ds, intVector &doms, 
                      avtDataSpecification_p &spec, avtSourceFromDatabase *src)
{
    //
    // The game plan here is to create a domain boundaries object using the
    // global node ids.
    //
    int   i, j, k, l;

    int ts = spec->GetTimestep();
    avtDatabaseMetaData *md = GetMetaData(ts);
    const char *varname = spec->GetVariable();
    string meshname = md->MeshForVar(varname);
    int numDomains = md->GetMesh(meshname)->numBlocks;

    //
    // Make sure we are dealing with unstructured grids.
    //
    for (i = 0 ; i < doms.size() ; i++)
    {
        vtkDataSet *d = ds.GetDataset(i, 0);
        if (d == NULL)
            continue;
        if (d->GetDataObjectType() != VTK_UNSTRUCTURED_GRID)
        {
            debug1 << "Not able to create ghost zones for anything but "
                   << "unstructured grids." << endl;
            return false;
        }
    }

    //
    // Most of the nodes in a dataset are internal.  It is possible to run
    // this algorithm using all of those nodes, but it will make the number
    // of nodes considered *huge*.  So let's figure out which nodes are 
    // internal and exteral.
    //
    int t1 = visitTimer->StartTimer();
    vector<vtkIntArray *> gni;
    vector<vtkIntArray *> lni;
    for (i = 0 ; i < doms.size() ; i++)
    {
        vtkUnstructuredGrid *d = (vtkUnstructuredGrid *) ds.GetDataset(i, 0);
        vtkUnstructuredGrid *copy = vtkUnstructuredGrid::New();
        copy->CopyStructure(d);
        vtkDataArray *gn = GetGlobalNodeIds(doms[i], meshname.c_str(), ts);
        copy->GetPointData()->AddArray(gn);
        vtkIntArray *ln = vtkIntArray::New();
        int npts = gn->GetNumberOfTuples();
        ln->SetNumberOfTuples(npts);
        int *ptr = ln->GetPointer(0);
        for (j = 0 ; j < npts ; j++)
            ptr[j] = j;
        ln->SetName("avtOriginalNodeId");
        copy->GetPointData()->AddArray(ln);
        ln->Delete();
        vtkUnstructuredGridFacelistFilter *ff =
                                      vtkUnstructuredGridFacelistFilter::New();
        ff->SetInput(copy);
        vtkPolyDataRelevantPointsFilter *rpf =
                                        vtkPolyDataRelevantPointsFilter::New();
        rpf->SetInput(ff->GetOutput());
        rpf->Update();
        vtkIntArray *g = (vtkIntArray *)
            rpf->GetOutput()->GetPointData()->GetArray("avtGlobalNodeNumbers");
        if (g == NULL)  // Name of array changed out from underneath us?
            EXCEPTION0(ImproperUseException);
        vtkIntArray *l = (vtkIntArray *)
               rpf->GetOutput()->GetPointData()->GetArray("avtOriginalNodeId");

        g->Register(NULL);
        l->Register(NULL);
        copy->Delete();
        rpf->Delete();
        ff->Delete();

        gni.push_back(g);
        lni.push_back(l);
    }
    visitTimer->StopTimer(t1, "Finding nodes external to domain.");

    //
    // Identify what the biggest id is.
    //
    int maxId = -1;
    for (i = 0 ; i < doms.size() ; i++)
    {
        vtkIntArray *int_gni = gni[i];
        int *ptr = int_gni->GetPointer(0);
        int nvals = int_gni->GetNumberOfTuples();
        for (j = 0 ; j < nvals ; j++)
            maxId = (maxId < ptr[j] ? ptr[j] : maxId);
    }

    maxId += 1;  // Its easier to work with ranges if maxId is one bigger
                 // than the actual biggest id.

    //
    // Identify what the maximum id is across all processors.
    //
    maxId = UnifyMaximumValue(maxId);
    int num_procs = PAR_Size();
    int rank = PAR_Rank();

    //
    // Break the range up among all processors.
    //
    int t2 = visitTimer->StartTimer();
    int numIdsPerProc = maxId / num_procs + 1;
    int myMin = numIdsPerProc * rank;
    int myMax = numIdsPerProc * (rank+1);
    if (myMax > maxId)
        myMax = maxId;

    //
    // We will do the bookkeeping for this processor's range.  Set up
    // the variables for that now.
    //
    int mySize = myMax-myMin;
    intVector index_for_node(mySize, -1);

    int curIndex = 0;
    int curSize = 4096;
    vector< intVector > doms_for_id(curSize);
    vector< intVector > local_ids_for_id(curSize);

    //
    // Go through all of the ids for this processor and either update
    // our arrays or determine which processor they should go to.
    //
    vector< intVector > global_ids_for_proc(num_procs);
    vector< intVector > doms_for_proc(num_procs);
    vector< intVector > local_ids_for_proc(num_procs);
    for (i = 0 ; i < doms.size() ; i++)
    {
        vtkIntArray *int_gni = gni[i];
        vtkIntArray *int_lni = lni[i];
        int *ptr  = int_gni->GetPointer(0);
        int *ptr2 = int_lni->GetPointer(0);
        int nvals = int_gni->GetNumberOfTuples();
        for (j = 0 ; j < nvals ; j++)
        {
            if (ptr[j] < myMin || ptr[j] >= myMax)
            {
                int otherProc = ptr[j] / numIdsPerProc;
                global_ids_for_proc[otherProc].push_back(ptr[j]);
                doms_for_proc[otherProc].push_back(doms[i]);
                local_ids_for_proc[otherProc].push_back(ptr2[j]);
            }
            else
            {
                int idx = ptr[j]-myMin;
                int ifn = index_for_node[idx];
                if (ifn == -1)
                {
                    if ((curIndex + 1) >= curSize)
                    {
                        curSize *= 2;
                        doms_for_id.resize(curSize);
                        local_ids_for_id.resize(curSize);
                    }
                    index_for_node[idx] = curIndex;
                    curIndex++;
                    ifn = index_for_node[idx];
                }
                doms_for_id[ifn].push_back(doms[i]);
                local_ids_for_id[ifn].push_back(ptr2[j]);
            }
        }
    }

    // 
    // We are now done with the arrays of global node ids that are on the
    // exterior of the dataset, so let's clean that up now.
    //
    for (i = 0 ; i < doms.size() ; i++)
    {
        gni[i]->Delete();
        lni[i]->Delete();
    }
    gni.clear();
    lni.clear();

#ifdef PARALLEL
    // 
    // Now take all the nodes that fell into other processors' ranges and
    // send them there using all-to-all communication.
    //
    int *sendcount = new int[num_procs];
    int *recvcount = new int[num_procs];
    int totalSend = 0;
    for (i = 0 ; i < num_procs ; i++)
    {
        sendcount[i] = 3*global_ids_for_proc[i].size();
        totalSend += sendcount[i];
    }

    MPI_Alltoall(sendcount, 1, MPI_INT, recvcount, 1, MPI_INT, MPI_COMM_WORLD);

    int totalRecv = 0;
    for (i = 0 ; i < num_procs ; i++)
    {
        totalRecv += recvcount[i];
    }

    int *senddisp = new int[num_procs];
    int *recvdisp = new int[num_procs];
    senddisp[0] = 0;
    recvdisp[0] = 0;
    for (i = 1 ; i < num_procs ; i++)
    {
        senddisp[i] = senddisp[i-1] + sendcount[i-1];
        recvdisp[i] = recvdisp[i-1] + recvcount[i-1];
    }

    int *big_send_buffer = new int[totalSend];
    int idx = 0;
    for (i = 0 ; i < num_procs ; i++)
    {
        for (j = 0 ; j < global_ids_for_proc[i].size() ; j++)
        {
            big_send_buffer[idx++] = global_ids_for_proc[i][j];
            big_send_buffer[idx++] = doms_for_proc[i][j];
            big_send_buffer[idx++] = local_ids_for_proc[i][j];
        }
    }

    int *big_recv_buffer = new int[totalRecv];
  
    //
    // We are now ready to transfer the actual node ids.
    //
    MPI_Alltoallv(big_send_buffer, sendcount, senddisp, MPI_INT,
                  big_recv_buffer, recvcount, recvdisp, MPI_INT,
                  MPI_COMM_WORLD);

    //
    // Now take everything in the receive buffer and add update our
    // node list counters with it.
    //
    if ((totalRecv % 3) != 0)
    {
        EXCEPTION0(ImproperUseException);
    }
    intVector Domain2Proc(numDomains, -1);
    int index = 0;
    for (i = 0 ; i < num_procs ; i++)
    {
        for (j = 0 ; j < recvcount[i] ; j += 3)
        {
            int glob_id = big_recv_buffer[index];
            int dom_id = big_recv_buffer[index+1];
            int local_id = big_recv_buffer[index+2];
            if (glob_id < myMin || glob_id >= myMax)
            {
                EXCEPTION0(ImproperUseException);
            }
    
            int idx = glob_id-myMin;
            int ifn = index_for_node[idx];
            if (ifn == -1)
            {
                if ((curIndex + 1) >= curSize)
                {
                    curSize *= 2;
                    doms_for_id.resize(curSize);
                    local_ids_for_id.resize(curSize);
                }
                index_for_node[idx] = curIndex;
                curIndex++;
                ifn = index_for_node[idx];
            }
            doms_for_id[ifn].push_back(dom_id);
            local_ids_for_id[ifn].push_back(local_id);
            Domain2Proc[dom_id] = i;

            index += 3;
        }
    }
#endif
    visitTimer->StopTimer(t2, "Creating table of shared nodes.");

    // 
    // We now have all the info we need to identify which nodes are shared
    // between which domains (for our portion of the global indexing space,
    // at least).
    //
    int t4 = visitTimer->StartTimer();
    vector< intVector > pairs(numDomains);
    vector< vector< intVector > > first_val(numDomains);
    vector< vector< intVector > > second_val(numDomains);
    for (i = myMin ; i < myMax ; i++)
    {
        int idx = i-myMin;
        int indirection_index = index_for_node[idx];
        if (indirection_index < 0)
            continue;
        if (doms_for_id[indirection_index].size() <= 1)
            continue;

        for (j = 0 ; j < doms_for_id[indirection_index].size() ; j++)
        {
            int primary_dom = doms_for_id[indirection_index][j];
            int primary_local_node = local_ids_for_id[indirection_index][j];
            for (k = 0 ; k < doms_for_id[indirection_index].size() ; k++)
            {
                int secondary_dom = doms_for_id[indirection_index][k];
                int secondary_local_node = 
                                        local_ids_for_id[indirection_index][k];
                // 'if' test should be true only when j == k.
                if (primary_dom == secondary_dom)
                    continue;
                int match = -1;
                for (l = 0 ; l < pairs[primary_dom].size() ; l++)
                    if (pairs[primary_dom][l] == secondary_dom)
                    {
                        match = l;
                        break;
                    }
                if (match == -1)
                {
                    pairs[primary_dom].push_back(secondary_dom);
                    int newSize = first_val[primary_dom].size()+1;
                    first_val[primary_dom].resize(newSize);
                    second_val[primary_dom].resize(newSize);
                    match = pairs[primary_dom].size()-1;
                }
                first_val[primary_dom][match].push_back(primary_local_node);
                second_val[primary_dom][match].push_back(secondary_local_node);
            }
        }
    }

#ifdef PARALLEL
    //
    // We now have a list of all shared points for our portion of the global 
    // indexing space.  But the matches we are interested in have to go to
    // other processors.  So do more all-to-all communication.
    //

    //
    // First, decide the size of what we are going to send to each processor.
    //
    for (i = 0 ; i < num_procs ; i++)
        sendcount[i] = 0;
    for (i = 0 ; i < numDomains ; i++)
    {
        int proc = Domain2Proc[i];
        int numPairs = pairs[i].size();
        for (j = 0 ; j < numPairs ; j++)
        {
            int domain2 = pairs[i][j];
            int proc2 = Domain2Proc[domain2];
            int numShared = first_val[i][j].size();
            if (proc >= 0)
                sendcount[proc] += 2+1+numShared*2;
            if (proc2 >= 0 && proc2 != proc)
                sendcount[proc2] += 2+1+numShared*2;
        }
    }
    
    totalSend = 0;
    for (i = 0 ; i < num_procs ; i++)
    {
        totalSend += sendcount[i];
    }

    MPI_Alltoall(sendcount, 1, MPI_INT, recvcount, 1, MPI_INT, MPI_COMM_WORLD);

    totalRecv = 0;
    for (i = 0 ; i < num_procs ; i++)
    {
        totalRecv += recvcount[i];
    }

    senddisp[0] = 0;
    recvdisp[0] = 0;
    for (i = 1 ; i < num_procs ; i++)
    {
        senddisp[i] = senddisp[i-1] + sendcount[i-1];
        recvdisp[i] = recvdisp[i-1] + recvcount[i-1];
    }

    delete [] big_send_buffer;
    big_send_buffer = new int[totalSend];
    idx = 0;
    int *curDisp = new int[num_procs];
    for (i = 0 ; i < num_procs ; i++)
        curDisp[i] = senddisp[i];

    for (i = 0 ; i < numDomains ; i++)
    {
        int proc = Domain2Proc[i];
        int numPairs = pairs[i].size();
        for (j = 0 ; j < numPairs ; j++)
        {
            int domain2 = pairs[i][j];
            int proc2 = Domain2Proc[domain2];
            int numShared = first_val[i][j].size();
            if (proc >= 0)
            {
                int *buffer = big_send_buffer + curDisp[proc];
                curDisp[proc] += 2+1+numShared*2;
                buffer[0] = i;
                buffer[1] = domain2;
                buffer[2] = numShared;
                for (k = 0 ; k < numShared ; k++)
                    buffer[k+3] = first_val[i][j][k];
                for (k = 0 ; k < numShared ; k++)
                    buffer[k+3+numShared] = second_val[i][j][k];
            }
            if (proc2 >= 0 && proc2 != proc)
            {
                int *buffer = big_send_buffer + curDisp[proc2];
                curDisp[proc2] += 2+1+numShared*2;
                buffer[0] = i;
                buffer[1] = domain2;
                buffer[2] = numShared;
                for (k = 0 ; k < numShared ; k++)
                    buffer[k+3] = first_val[i][j][k];
                for (k = 0 ; k < numShared ; k++)
                    buffer[k+3+numShared] = second_val[i][j][k];
            }
        }
    }
    delete [] curDisp;
    delete [] big_recv_buffer;
    big_recv_buffer = new int[totalRecv];
  
    //
    // We are now ready to transfer the actual sets of shared points.
    //
    MPI_Alltoallv(big_send_buffer, sendcount, senddisp, MPI_INT,
                  big_recv_buffer, recvcount, recvdisp, MPI_INT,
                  MPI_COMM_WORLD);

    //
    // Now add the shared points to the domain boundary information that comes
    // from other processors.
    //
    for (i = 0 ; i < num_procs ; i++)
    {
        int ap = 0; // ap => amountProcessed
        int amountToProcess = recvcount[i];
        int *buffer = big_recv_buffer + recvdisp[i];
        while (ap < amountToProcess)
        {
            int domain1 = buffer[ap++];
            int domain2 = buffer[ap++];
            int numShared = buffer[ap++];

            if (domain1 == domain2)
            {
                // Should not happen...
                buffer += 2*numShared;
                continue;
            }

            int match = -1;
            for (l = 0 ; l < pairs[domain1].size() ; l++)
                if (pairs[domain1][l] == domain2)
                {
                    match = l;
                    break;
                }
            if (match == -1)
            {
                pairs[domain1].push_back(domain2);
                int newSize = first_val[domain1].size()+1;
                first_val[domain1].resize(newSize);
                second_val[domain1].resize(newSize);
                match = pairs[domain1].size()-1;
            }

            int numCurVals = first_val[domain1][match].size();
            int newNumVals = numCurVals + numShared;
            
            first_val[domain1][match].resize(newNumVals);
            second_val[domain1][match].resize(newNumVals);

            for (j = 0 ; j < numShared ; j++)
                first_val[domain1][match][numCurVals+j] = buffer[ap++];
            for (j = 0 ; j < numShared ; j++)
                second_val[domain1][match][numCurVals+j] = buffer[ap++];
        }
    }

    delete [] sendcount;
    delete [] recvcount;
    delete [] senddisp;
    delete [] recvdisp;
    delete [] big_send_buffer;
    delete [] big_recv_buffer;
#endif
    visitTimer->StopTimer(t4, "Setting up shared nodes list.");

    //
    // Create domain boundary information object by registering the shared
    // points.
    //
    avtUnstructuredPointBoundaries upb;
    upb.SetTotalNumberOfDomains(numDomains);
    for (i = 0 ; i < numDomains ; i++)
    {
        for (j = 0 ; j < pairs[i].size() ; j++)
        {
            upb.SetSharedPoints(i, pairs[i][j], first_val[i][j],
                                                 second_val[i][j]);
        }
    }

    return CommunicateGhostZonesFromDomainBoundaries(&upb, ds, doms, spec,src);
}


// ****************************************************************************
//  Method: avtGenericDatabase::CommunicateGhostNodesFromGlobalNodeIds
//
//  Purpose:
//      Creates ghost nodes using global node ids.
//
//  Arguments:
//      ds        The dataset collection.
//      doms      A list of domains.
//      spec      A data specification.
//      src       The source object.
//
//  Notes:      This routine used to be wholely contained by CommunicateGhosts.
//
//  Programmer: Hank Childs
//  Creation:   August 13, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Aug 27 16:16:52 PDT 2004
//    Rename ghost data arrays.
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector'.
//
// ****************************************************************************

bool
avtGenericDatabase::CommunicateGhostNodesFromGlobalNodeIds(
                      avtDatasetCollection &ds, intVector &doms, 
                      avtDataSpecification_p &spec, avtSourceFromDatabase *src)
{
    int   i, j;

    int ts = spec->GetTimestep();
    avtDatabaseMetaData *md = GetMetaData(ts);
    const char *varname = spec->GetVariable();
    string meshname = md->MeshForVar(varname);

    //
    // Identify what the biggest id is.
    //
    int maxId = -1;
    for (i = 0 ; i < doms.size() ; i++)
    {
        vtkDataArray *gni = GetGlobalNodeIds(doms[i], meshname.c_str(), ts);
        vtkIntArray *int_gni = (vtkIntArray *) gni;
        int *ptr = int_gni->GetPointer(0);
        int nvals = int_gni->GetNumberOfTuples();
        for (j = 0 ; j < nvals ; j++)
            maxId = (maxId < ptr[j] ? ptr[j] : maxId);
    }

    maxId += 1;  // Its easier to work with ranges if maxId is one bigger
                 // than the actual biggest id.

    //
    // Identify what the maximum id is across all processors.
    //
    maxId = UnifyMaximumValue(maxId);
    int num_procs = PAR_Size();
    int rank = PAR_Rank();

    //
    // Break the range up among all processors.
    //
    int numIdsPerProc = (maxId / num_procs) + 1;
    int myMin = numIdsPerProc * rank;
    int myMax = numIdsPerProc * (rank+1);
    if (myMax > maxId)
        myMax = maxId;

    //
    // We will do the bookkeeping for this processor's range.  Set up
    // the variables for that now.
    //
    int mySize = myMax-myMin;
    int *allIds = new int[mySize];
    for (i = 0 ; i < mySize ; i++)
        allIds[i] = 0;

    //
    // Go through all of the ids for this processor and either update
    // our arrays or determine which processor they should go to.
    //
    vector< intVector > ids_for_proc(num_procs);
    for (i = 0 ; i < doms.size() ; i++)
    {
        vtkDataArray *gni = GetGlobalNodeIds(doms[i], meshname.c_str(), ts); 
        vtkIntArray *int_gni = (vtkIntArray *) gni;
        int *ptr = int_gni->GetPointer(0);
        int nvals = int_gni->GetNumberOfTuples();
        for (j = 0 ; j < nvals ; j++)
        {
            if (ptr[j] < myMin || ptr[j] >= myMax)
            {
                int otherProc = ptr[j] / numIdsPerProc;
                ids_for_proc[otherProc].push_back(ptr[j]);
            }
            else
                allIds[ptr[j]-myMin]++;
        }
    }

#ifdef PARALLEL
    // 
    // Now take all the nodes that fell into other processors' ranges and
    // send them there using all-to-all communication.
    //
    int *sendcount = new int[num_procs];
    int *recvcount = new int[num_procs];
    int totalSend = 0;
    for (i = 0 ; i < num_procs ; i++)
    {
        sendcount[i] = ids_for_proc[i].size();
        totalSend += sendcount[i];
    }

    MPI_Alltoall(sendcount, 1, MPI_INT, recvcount, 1, MPI_INT, MPI_COMM_WORLD);

    int totalRecv = 0;
    for (i = 0 ; i < num_procs ; i++)
    {
        totalRecv += recvcount[i];
    }

    int *senddisp = new int[num_procs];
    int *recvdisp = new int[num_procs];
    senddisp[0] = 0;
    recvdisp[0] = 0;
    for (i = 1 ; i < num_procs ; i++)
    {
        senddisp[i] = senddisp[i-1] + sendcount[i-1];
        recvdisp[i] = recvdisp[i-1] + recvcount[i-1];
    }

    int *big_send_buffer = new int[totalSend];
    int idx = 0;
    for (i = 0 ; i < num_procs ; i++)
        for (j = 0 ; j < ids_for_proc[i].size() ; j++)
            big_send_buffer[idx++] = ids_for_proc[i][j];

    int *big_recv_buffer = new int[totalRecv];
  
    //
    // We are now ready to transfer the actual node ids.
    //
    MPI_Alltoallv(big_send_buffer, sendcount, senddisp, MPI_INT,
                  big_recv_buffer, recvcount, recvdisp, MPI_INT,
                  MPI_COMM_WORLD);

    //
    // Now take everything in the receive buffer and add update our
    // node list counters with it.
    //
    for (i = 0 ; i < totalRecv ; i++)
    {
        if (big_recv_buffer[i] < myMin || big_recv_buffer[i] >= myMax)
        {
            EXCEPTION0(ImproperUseException);
        }
        allIds[big_recv_buffer[i]-myMin]++;
    }

    //
    // We now know definitively whether each node is a ghost or not ghost.
    //  Now get that information back to the processors
    // that requested it.  Because we know what order things were sent in,
    // it is sufficient to simply send back a yes/no answer with no
    // further information.
    //
    // Note: since we are now sending back information about what we
    // received, all the "send" sizes become "receive" sizes and 
    // vice-versa.
    //
    int new_totalSend = totalRecv;
    int new_totalRecv = totalSend;
    int *new_senddisp = recvdisp;
    int *new_recvdisp = senddisp;
    int *new_sendcount = recvcount;
    int *new_recvcount = sendcount;
    char *new_big_send_buffer = new char[new_totalSend];
    char *new_big_recv_buffer = new char[new_totalRecv];

    for (i = 0 ; i < new_totalSend ; i++)
        new_big_send_buffer[i] = 
                            (allIds[big_recv_buffer[i]-myMin] > 1 ? 1 : 0);

    MPI_Alltoallv(new_big_send_buffer, new_sendcount, new_senddisp,MPI_CHAR,
                  new_big_recv_buffer, new_recvcount, new_recvdisp,MPI_CHAR,
                  MPI_COMM_WORLD);

    // 
    // We are almost there!  Each processor has now sent us whether or not
    // the nodes we requested are ghost nodes are not.  Note: the info
    // is coming back to us in *exactly* the order requested, so we can
    // take the values and start blindly assigning them.  There is some
    // subtlety to the indexing here.
    //
    intVector num_used_from_proc(num_procs, 0);
#endif

    for (i = 0 ; i < doms.size() ; i++)
    {
        vtkDataSet *d = ds.GetDataset(i, 0);
        vtkDataArray *gni = GetGlobalNodeIds(doms[i], meshname.c_str(), ts); 
        vtkIntArray *int_gni = (vtkIntArray *) gni;
        int *ptr = int_gni->GetPointer(0);
        int nvals = int_gni->GetNumberOfTuples();
        vtkUnsignedCharArray *ghost_nodes = vtkUnsignedCharArray::New();
        ghost_nodes->SetName("avtGhostNodes");
        ghost_nodes->SetNumberOfTuples(nvals);
        for (j = 0 ; j < nvals ; j++)
        {
            if (ptr[j] >= myMin && ptr[j] < myMax)
            {
                if (allIds[ptr[j]-myMin] > 1)
                    ghost_nodes->SetValue(j, 1);
                else
                    ghost_nodes->SetValue(j, 0);
            }
            else
            {
#ifdef PARALLEL
                int otherProc = ptr[j] / numIdsPerProc;
                int idx = new_recvdisp[otherProc] + 
                          num_used_from_proc[otherProc];
                char val = new_big_recv_buffer[idx];
                num_used_from_proc[otherProc]++;
                ghost_nodes->SetValue(j, val);
#else
                EXCEPTION0(ImproperUseException);
#endif
            }
        }
        vtkDataSet *d2 = d->NewInstance();
        d2->ShallowCopy(d);
        d2->GetPointData()->AddArray(ghost_nodes);
        ds.SetDataset(i, 0, d2);
        d2->Delete();
        ghost_nodes->Delete();
    }

    delete [] allIds;
#ifdef PARALLEL
    delete [] new_big_send_buffer;
    delete [] new_big_recv_buffer;
    delete [] big_recv_buffer;
    delete [] big_send_buffer;
    delete [] senddisp;
    delete [] recvdisp;
    delete [] sendcount;
    delete [] recvcount;
#endif

    return true;
}


// ****************************************************************************
//  Method: avtGenericDatabase::ApplyGhostForDomainNesting
//
//  Purpose:
//      When domains nest within each other (as for an AMR mesh), this method
//      will apply the "vtkGhostLevels" label to any zones in coarser domains 
//      that have finer domains in the current selection.
//
//      Note: In addition to knowing the list of domains being processed on
//      this processor, we also need to know the list of domains being
//      processed on all processors. The fact is, this processor may contain
//      a coarse domain that should have zones ghosted out due to a finer
//      patch in the current selection but living on another processor 
//
//  Arguments:
//      ds        The dataset collection.
//      doms      A list of domains.
//      spec      A data specification.
//
//  Returns:      True if any zones where ghosted.  False otherwise.
//
//  Programmer:   Mark C. Miller
//  Creation:     October 13, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Nov 17 17:45:39 PST 2003
//    Clean up memory leak.
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector'.
//
//    Hank Childs, Thu Jan  6 16:47:50 PST 2005
//    Added calls to confirm mesh.
//
//    Hank Childs, Sat Mar  5 19:26:05 PST 2005
//    Added argument canDoCollectiveCommunication.
//
// ****************************************************************************

bool
avtGenericDatabase::ApplyGhostForDomainNesting(avtDatasetCollection &ds, 
   intVector &doms, intVector &allDoms, avtDataSpecification_p &spec, 
   bool canDoCollectiveCommunication)
{
    bool rv = false;

    int ts = spec->GetTimestep();
    avtDatabaseMetaData *md = GetMetaData(ts);
    string meshname = md->MeshForVar(spec->GetVariable());
    
    void_ref_ptr vr = cache.GetVoidRef(meshname.c_str(),
                                   AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                                   ts, -1);
    if (*vr == NULL)
        vr = cache.GetVoidRef("any_mesh",
                              AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                              spec->GetTimestep(), -1);

    avtDomainNesting *dn = (avtDomainNesting*)*vr;
    vector<vtkDataSet *> list;

    int shouldStop = 0;
    if (*vr != NULL)
    {
        int  i;
        for (i = 0 ; i < doms.size() ; i++)
        {
            list.push_back(ds.GetDataset(i, 0));
        }

        bool rightMesh = dn->ConfirmMesh(doms, list);
        if (!rightMesh)
            shouldStop = 1;
    }
    else
        shouldStop = 1;

#ifdef PARALLEL
    if (canDoCollectiveCommunication)
    {
        int  parallelShouldStop;
        MPI_Allreduce(&shouldStop, &parallelShouldStop, 1, MPI_INT, MPI_MAX,
                      MPI_COMM_WORLD);
        shouldStop = parallelShouldStop;
    }
#endif

    if (!shouldStop)  
    {
        rv = dn->ApplyGhost(doms, allDoms, list);

        //
        // Tell everything downstream that we do have ghost zones.
        //
        if (rv)
        {
            GetMetaData(ts)->SetContainsGhostZones(meshname, 
                                                   AVT_CREATED_GHOSTS);
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::MaterialSelect
//
//  Purpose:
//      Perform a material selection.
//
//  Arguments:
//      ds        The dataset collection.
//      domains   A list of domains to be read.
//      spec      A data specification.
//      src       The source object.
//      didGhost  Indicates whether we created ghosts.
//
//  Programmer:   Hank Childs
//  Creation:     October 26, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Nov 26 09:01:36 PST 2001
//    Account for materials being read in while the mesh is being read in.
//
//    Hank Childs, Thu Jul  4 16:34:33 PDT 2002
//    Account for cases where there are multiple mixed variables.
//
//    Jeremy Meredith, Tue Aug 13 14:39:30 PDT 2002
//    Added the two subdivision flags and set their counterparts
//    in the output avtDataValidity structure.
//
//    Hank Childs, Wed Oct  9 10:16:15 PDT 2002
//    Change test for if we are using all the domains, since it did not work
//    in parallel.
//
//    Jeremy Meredith, Thu Oct 24 15:37:24 PDT 2002
//    Added smoothing option and clean zones only option.
//
//    Hank Childs, Fri Jan 17 09:43:03 PST 2003
//    Make sure to stop the timer if we return early.
//
//    Jeremy Meredith, Wed Mar 19 16:50:58 PST 2003
//    Added support for variables defined on only a subset of the domains.
//
//    Jeremy Meredith, Thu Jun 12 09:09:30 PDT 2003
//    Added a flag for internal boundary surfaces.
//
//    Hank Childs, Tue Jul 22 21:48:09 PDT 2003
//    Send down information about whether or not we communicated ghost zones.
//
//    Hank Childs, Fri Sep 12 16:27:38 PDT 2003
//    Send down info about whether the MIR was forced.
//
//    Jeremy Meredith, Fri Sep  5 15:32:30 PDT 2003
//    Added a flag for the MIR algorithm.
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector'.
//
//    Kathleen Bonnell, Thu Feb  3 09:27:22 PST 2005 
//    Set MIROccurred flag in avtDataAttributes. 
//
// ****************************************************************************

void
avtGenericDatabase::MaterialSelect(avtDatasetCollection &ds,
                            intVector &domains, avtDataSpecification_p &spec,
                            avtSourceFromDatabase *src, bool didGhosts)
{
    int i, timerHandle = visitTimer->StartTimer();

    const char *var = spec->GetVariable();
    int         ts  = spec->GetTimestep();

    char  progressString[1024] = "Performing material selection";

    bool shouldDoMatSelect = false;
    for (i = 0 ; i < ds.GetNDomains() ; i++)
    {
        shouldDoMatSelect = shouldDoMatSelect || ds.needsMatSelect[i];
    }
    if (! shouldDoMatSelect)
    {
        visitTimer->StopTimer(timerHandle, "Deciding not to material "
                                           "select data");
        return;
    }

    bool subdivisionOccurred   = false;
    bool notAllCellsSubdivided = false;

    src->DatabaseProgress(0, 0, progressString);

    //
    // If we are communicating ghost zones ourselves, the MIR objects we
    // create could be invalid if we are using a different set of domains.  To
    // be on the safe side, only re-use MIRs if we are re-using all of the 
    // data.  We could cache all of the MIRs and re-use which ever ones make 
    // sense based on which domains are being used, but that could blow the
    // cache pretty quickly.
    //
    bool reUseMIR = true;
    if (didGhosts)
    {
        if (!spec->UsesAllDomains())
        {
            reUseMIR = false;
        }
    }

    for (i = 0 ; i < ds.GetNDomains() ; i++)
    {
        if (ds.GetDataset(i,0)!=NULL  &&  ds.needsMatSelect[i])
        {
            vector<avtMixedVariable *> mvl;
            vector<void_ref_ptr> &vrl = ds.GetAllMixVars(i);
            for (int j = 0 ; j < vrl.size() ; j++)
            {
                avtMixedVariable *mv = (avtMixedVariable *) *vrl[j];
                mvl.push_back(mv);
            }

            bool so;
            bool nacs;
            ds.avtds[i] = MaterialSelect(ds.GetDataset(i,0), ds.GetMaterial(i),
                                mvl, domains[i], var, ts,
                                ds.matnames[i], ds.labels[i],
                                spec->NeedInternalSurfaces(),
                                spec->NeedBoundarySurfaces(),
                                spec->NeedValidFaceConnectivity(),
                                spec->NeedSmoothMaterialInterfaces(),
                                spec->NeedCleanZonesOnly(), 
                                spec->MustDoMaterialInterfaceReconstruction(),
                                spec->UseNewMIRAlgorithm() ? 1 : 0,
                                didGhosts, so, nacs, reUseMIR);

            notAllCellsSubdivided = notAllCellsSubdivided || nacs ||
                (subdivisionOccurred && !so) || (!subdivisionOccurred && so);
            subdivisionOccurred = subdivisionOccurred || so;
        }

        src->DatabaseProgress(i, ds.GetNDomains(), progressString);
    }
    src->DatabaseProgress(1, 0, progressString);
    src->GetOutput()->GetInfo().GetAttributes().SetMIROccurred(true);
    src->GetOutput()->GetInfo().GetValidity().SetSubdivisionOccurred(
                                                        subdivisionOccurred);
    src->GetOutput()->GetInfo().GetValidity().SetNotAllCellsSubdivided(
                                                        notAllCellsSubdivided);

    visitTimer->StopTimer(timerHandle, "Material selecting dataset");
    visitTimer->DumpTimings();
}


// ****************************************************************************
//  Method: avtGenericDatabase::CreateGlobalZones
//
//  Purpose:
//    Create global zones array. 
//
//  Programmer:   Hank Childs
//  Creation:     September 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector'.
//
// ****************************************************************************

void
avtGenericDatabase::CreateGlobalZones(avtDatasetCollection &ds, 
                              intVector &domains, avtSourceFromDatabase *src,
                              avtDataSpecification_p &spec)
{
    char  progressString[1024] = "Creating Global Zones Array";
    src->DatabaseProgress(0, 0, progressString);
    for (int i = 0 ; i < ds.GetNDomains() ; i++)
    {
        vtkDataArray *arr = GetGlobalZoneIds(domains[i], spec->GetVariable(),
                                             spec->GetTimestep());
        ds.GetDataset(i, 0)->GetCellData()->AddArray(arr);
        src->DatabaseProgress(i, ds.GetNDomains(), progressString);
    }
    src->DatabaseProgress(1, 0, progressString);
}


// ****************************************************************************
//  Method: avtGenericDatabase::CreateGlobalNodes
//
//  Purpose:
//    Create global nodes array. 
//
//  Programmer:   Hank Childs 
//  Creation:     September 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector'.
//
// ****************************************************************************

void
avtGenericDatabase::CreateGlobalNodes(avtDatasetCollection &ds, 
                              intVector &domains, avtSourceFromDatabase *src,
                              avtDataSpecification_p &spec)
{
    char  progressString[1024] = "Creating Global Nodes Array";
    src->DatabaseProgress(0, 0, progressString);
    for (int i = 0 ; i < ds.GetNDomains() ; i++)
    {
        vtkDataArray *arr = GetGlobalNodeIds(domains[i], spec->GetVariable(),
                                             spec->GetTimestep());
        ds.GetDataset(i, 0)->GetPointData()->AddArray(arr);
        src->DatabaseProgress(i, ds.GetNDomains(), progressString);
    }
    src->DatabaseProgress(1, 0, progressString);
}


// ****************************************************************************
//  Method: avtGenericDatabase::CreateOriginalZones
//
//  Purpose:
//    Create original zones array. 
//
//  Arguments:
//      ds        The dataset collection.
//      src       The source object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     November 12, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Tue Mar 26 15:58:05 PST 2002
//    Make this method keep track of a starting cell number for each data set.
//
//    Kathleen Bonnell, Mon May 20 17:01:31 PDT 2002  
//    Removed starting cell number.  Pass domain number to 
//    AddOriginalCellsArray.  
//    
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector'.
//
// ****************************************************************************

void
avtGenericDatabase::CreateOriginalZones(avtDatasetCollection &ds, 
                              intVector &domains, avtSourceFromDatabase *src)
{
    char  progressString[1024] = "Creating Original Zones Array";
    src->DatabaseProgress(0, 0, progressString);
    for (int i = 0 ; i < ds.GetNDomains() ; i++)
    {
        AddOriginalCellsArray(ds.GetDataset(i, 0), domains[i]);
        src->DatabaseProgress(i, ds.GetNDomains(), progressString);
    }
    src->DatabaseProgress(1, 0, progressString);
}


// ****************************************************************************
//  Method: avtGenericDatabase::CreateOriginalNodes
//
//  Purpose:
//    Create original nodes array. 
//
//  Arguments:
//      ds        The dataset collection.
//      src       The source object.
//
//  Programmer:   Hank Childs 
//  Creation:     June 18, 2003
//
//  Modifications:
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector'.
//
// ****************************************************************************

void
avtGenericDatabase::CreateOriginalNodes(avtDatasetCollection &ds, 
                              intVector &domains, avtSourceFromDatabase *src)
{
    char  progressString[1024] = "Creating Original Nodes Array";
    src->DatabaseProgress(0, 0, progressString);
    for (int i = 0 ; i < ds.GetNDomains() ; i++)
    {
        AddOriginalNodesArray(ds.GetDataset(i, 0), domains[i]);
        src->DatabaseProgress(i, ds.GetNDomains(), progressString);
    }
    src->DatabaseProgress(1, 0, progressString);
}


// ****************************************************************************
//  Method: avtGenericDatabase::CreateStructuredIndices
//
//  Purpose:
//      Create an array that contains the index number of each cell.
//
//  Arguments:
//      ds        The dataset collection.
//      src       The source object.
//
//  Programmer:   Hank Childs 
//  Creation:     September 30, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Feb 3 11:46:47 PDT 2005
//    Added a check for NULL meshes since a case popped up where some of the
//    return meshes were NULL.
//
// ****************************************************************************

void
avtGenericDatabase::CreateStructuredIndices(avtDatasetCollection &dsc, 
                                            avtSourceFromDatabase *src)
{
    char  progressString[1024] = "Creating Structured Indices";
    src->DatabaseProgress(0, 0, progressString);
    for (int i = 0 ; i < dsc.GetNDomains() ; i++)
    {
        vtkDataSet *ds = dsc.GetDataset(i, 0);
        if(ds == NULL)
        {
            debug1 << "Requested structured indices for NULL mesh." << endl;
            continue;
        }
        int dstype = ds->GetDataObjectType();
        int dims[3];
        if (dstype == VTK_RECTILINEAR_GRID)
        {
            vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) ds;
            rgrid->GetDimensions(dims);
        }
        else if (dstype == VTK_STRUCTURED_GRID)
        {
            vtkStructuredGrid *sgrid = (vtkStructuredGrid *) ds;
            sgrid->GetDimensions(dims);
        }
        else
        {
            debug1 << "Requested structured indices for non-structured grid."
                   << endl;
            continue;
        }
        vtkUnsignedIntArray *orig_dims = vtkUnsignedIntArray::New();
        orig_dims->SetNumberOfTuples(3);
        orig_dims->SetValue(0, dims[0]);
        orig_dims->SetValue(1, dims[1]);
        orig_dims->SetValue(2, dims[2]);
        orig_dims->SetName("avtOriginalStructuredDimensions");
        ds->GetFieldData()->AddArray(orig_dims);
        orig_dims->Delete();
        AddOriginalCellsArray(ds, -1);
        src->DatabaseProgress(i, dsc.GetNDomains(), progressString);
    }
    src->DatabaseProgress(1, 0, progressString);
}


// ****************************************************************************
//  Method: avtGenericDatabase::NumStagesForFetch
//
//  Purpose:
//      This returns how many stages there are for the fetch.  Some databases
//      have more than one stage (read from file format, perform material
//      selection, etc).
//
//  Arguments:
//      spec    The data specification that will be sent in.
//
//  Returns:    A good default number of stages (1).
//
//  Programmer: Hank Childs
//  Creation:   October 26, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Dec 19 20:14:30 PST 2001
//    Added a stage for species selection.
//
//    Hank Childs, Mon Sep 30 18:01:26 PDT 2002
//    Added stages for creating the original zones and structured indices
//    arrays.
//
//    Hank Childs, Fri Nov 22 16:39:13 PST 2002
//    Use the SIL restriction traverser since SIL restriction routines were
//    antiquated.
//
//    Hank Childs, Wed Jun 18 09:34:38 PDT 2003
//    Added a stage for needing node numbers.
//
//    Hank Childs, Wed Aug 13 08:09:20 PDT 2003
//    We should do MIR if we are told to by the data spec, not if the type of
//    variable is material.
//
//    Jeremy Meredith, Wed Aug 20 09:56:19 PDT 2003
//    Refactored MIR into a base and subclass.  This will allow us to swap
//    in other MIR algorithms more easily.
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'vector<int>' to 'intVector', and 'vector<bool>' to 'boolVector'.
//
// ****************************************************************************

int
avtGenericDatabase::NumStagesForFetch(avtDataSpecification_p spec)
{
    int numStages = 1;   // Always one for I/O.

    avtSILRestriction_p silr = spec->GetRestriction();
    avtSILRestrictionTraverser trav(silr);
    intVector domains;
    trav.GetDomainList(domains);
    
    //
    // Determine if there will be a material selection phase.
    //
    bool needMatSel = false;
    
    needMatSel |= spec->MustDoMaterialInterfaceReconstruction();
    if (!needMatSel)
    {
        for (int i = 0 ; i < domains.size() ; i++)
        {
            bool needMatSelForThisDom;
            trav.GetMaterials(domains[i], needMatSelForThisDom);
            if (needMatSelForThisDom)
            {
                needMatSel = true;
                break;
            }
        }
    }

    //
    // Some file formats do their own selection.  They have a combined stage,
    // so do not add one in that case.
    //
    if (needMatSel && !Interface->PerformsMaterialSelection())
    {
        numStages += 1;
    }

    //
    // Now determine if there will be a phase to communicate ghost zones.
    //
    void_ref_ptr vr = cache.GetVoidRef("any_mesh",
                                  AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                                  -1, -1);
    if (*vr != NULL && trav.UsesAllData())
    {
        numStages += 1;
    }

    boolVector list;
    if (trav.GetSpecies(list))
    {
        numStages += 1;
    }

    if (spec->NeedNodeNumbers())
    {
        numStages += 1;
    }

    if (spec->NeedZoneNumbers())
    {
        numStages += 1;
    }
  
    if (spec->NeedStructuredIndices())
    {
        numStages += 1;
    }

    return numStages;
}


// ****************************************************************************
//  Method: avtGenericDatabase::QueryScalars
//
//  Purpose:
//    Queries the db regarding scalar var info for a specific cell or nodes. 
//
//  Arguments:
//    varName     The variable on which to retrieve data.
//    dom         The domain to query.
//    zone        The zone to query.
//    ts          The timestep to query.
//    nodes       The nodes to query.
//    varInfo     A place to store the results. 
//    zonePick    Whether or not the pick was a zone pick.
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     June 27, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Made filling of values contigent upon it not being filled already.
//    Don't set variable name, it should be set already.
//
//    Kathleen Bonnell, Fri Jun 20 13:57:30 PDT 2003  
//    Add support for node-pick. 
//
//    Mark C. Miller, 30Sep03 added timestep argument
//    
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004
//    Set PickVarInfo::treatAsASCII from ScalarMetaData::treatAsASCII.
//
//    Hank Childs, Fri Aug 20 14:05:54 PDT 2004
//    Initialize variable to remove compiler warning.
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'std::vector<int>' to 'intVector', 'std::vector<std::string>'
//    to 'stringVector', and 'std::vector<double>' to 'doubleVector'.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added dummy args for data type conversion for calls to get variable
//
// ****************************************************************************

bool
avtGenericDatabase::QueryScalars(const string &varName, const int dom, 
                                 const int element, const int ts, 
                                 const intVector &incidentElements, 
                                 PickVarInfo &varInfo, const bool zonePick)
{
    bool rv = false;
    int i;
    if (varInfo.GetValues().empty())
    {
        doubleVector vals;
        stringVector names;
        const avtScalarMetaData *smd = GetMetaData(ts)->GetScalar(varName);
        if (!smd)
        {
             debug5 << "Querying scalar var, but could not retrieve"
                    << " meta data!" << endl;
             return false;
        }
        char temp[80];

        //
        // admissibleDataTypes and needNative are place-holders for
        // information that will eventually be obtained either through
        // additional args to this method or from PickVarInfo
        //
        vector<int> admissibleDataTypes;
        admissibleDataTypes.push_back(VTK_FLOAT);
        bool needNative = false;
        vtkDataArray *scalars = GetScalarVariable(varName.c_str(), ts, dom,
                                                  "_all", needNative,
                                                  admissibleDataTypes);
        if (scalars) 
        {
            varInfo.SetTreatAsASCII(smd->treatAsASCII);
            bool zoneCent = false, validCentering = true;
            if (smd->centering == AVT_NODECENT)
            {
                varInfo.SetCentering(PickVarInfo::Nodal);
                zoneCent = false; 
            }
            else if (smd->centering == AVT_ZONECENT)
            {
                varInfo.SetCentering(PickVarInfo::Zonal);
                zoneCent = true; 
            }
            else 
            {
                validCentering = false; 
            }

            if (validCentering)
            {
                if (zoneCent != zonePick) 
                {
                    // the info we're after is associated with incidentElements
                    for (i = 0; i < incidentElements.size(); i++)
                    {
                        sprintf(temp, "(%d)", incidentElements[i]);
                        names.push_back(temp);
                        vals.push_back(scalars->GetTuple1(incidentElements[i]));
                    }
                }
                else 
                {
                    // the info we're after is associated with element
                    sprintf(temp, "(%d)", element);
                    names.push_back(temp);
                    vals.push_back(scalars->GetTuple1(element));
                }
            }
        }
        if (!vals.empty())
        {
            varInfo.SetNames(names);
            varInfo.SetValues(vals);
            vals.clear();
            names.clear();
            rv = true;
        }
    }

 
    //
    //  If this is a mixed-var, then get the material values.
    //
    void_ref_ptr vr = cache.GetVoidRef(varName.c_str(), 
                                AUXILIARY_DATA_MIXED_VARIABLE, 
                                ts, dom); 

    if (*vr != NULL)
    {
        avtMixedVariable *mv = (avtMixedVariable*)(*vr);
        avtMaterial *mat = GetMaterial(dom, varName.c_str(), ts);
        vector<CellMatInfo> matInfo; 
        stringVector mN;
        doubleVector mV;
        intVector nMats;
        int i, j, nMatsPerZone;
        bool mixed = false;
        
        if (zonePick)
        { 
            // check that zones reported by pick are accurate for retrieving
            // material var info
            if (element < 0 || element >= mat->GetNZones())
            {
                debug5 << "CANNOT QUERY MATFRACS, ZONE IS OUT OF RANGE" << endl;
                return rv;
            }
            matInfo = mat->ExtractCellMatInfo(element);
            nMatsPerZone = 0;
            for (int i = 0; i < matInfo.size(); i++)
            { 
                if (matInfo[i].mix_index != -1)
                {
                    mixed = true;
                    mN.push_back(matInfo[i].name);
                    mV.push_back(mv->GetBuffer()[matInfo[i].mix_index]);
                    nMatsPerZone++;
                }
            }
            nMats.push_back(nMatsPerZone);
        }
        else
        {
            // check that zones reported by pick are accurate for retrieving
            // material var info
            bool zonesInRange = true;
            int nmatzones = mat->GetNZones();
            for (i = 0; i < incidentElements.size(); i++)
            {
                if (incidentElements[i] < 0 || incidentElements[i] >= nmatzones)
                {
                    zonesInRange = false;
                    break; 
                }
            }
            if (!zonesInRange)
            {
                debug5 << "CANNOT QUERY MATFRACS ZONE IS OUT OF RANGE" << endl;
                return rv;
            }
            for (j = 0; j < incidentElements.size(); j++)
            {
                nMatsPerZone = 0;
                matInfo = mat->ExtractCellMatInfo(incidentElements[j]);
                for (i = 0; i < matInfo.size(); i++)
                { 
                    if (matInfo[i].mix_index != -1)
                    {
                        mixed = true;
                        mN.push_back(matInfo[i].name);
                        mV.push_back(mv->GetBuffer()[matInfo[i].mix_index]);
                        nMatsPerZone++;
                    }
                }
                nMats.push_back(nMatsPerZone);
            }
        }
        if (mixed)
        {
            varInfo.SetMixVar(true);
            varInfo.SetMixNames(mN);
            varInfo.SetMixValues(mV);
            varInfo.SetNumMatsPerZone(nMats);
        }
        rv = true;
    }

    // 
    // This is where we could allow the interface to add more information.
    // 
    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::QueryVectors
//
//  Purpose:
//    Queries the db regarding vector var info for a specific cell or nodes.  
//
//  Arguments:
//    varName     The variable on which to retrieve data.
//    dom         The domain to query.
//    zone        The zone to query.
//    ts          The timestep to query.
//    nodes       The nodes to query.
//    varInfo     A place to store the results. 
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     June 27, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Made filling of values contigent upon it not being filled already.
//    Don't set variable name, it should be set already.
//
//    Kathleen Bonnell, Fri Jun 20 13:57:30 PDT 2003  
//    Add support for node-pick. 
//    
//    Hank Childs, Fri Aug 20 14:05:54 PDT 2004
//    Initialize variable to remove compiler warning.
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'std::vector<int>' to 'intVector', 'std::vector<std::string>'
//    to 'stringVector', and 'std::vector<double>' to 'doubleVector'.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added dummy args for data type conversion for calls to get variable
//
// ****************************************************************************

bool
avtGenericDatabase::QueryVectors(const string &varName, const int dom, 
                                 const int element, const int ts, 
                                 const intVector &incidentElements, 
                                 PickVarInfo &varInfo, const bool zonePick)
{
    bool rv = false;
    if (varInfo.GetValues().empty())
    {
        const avtVectorMetaData *vmd = GetMetaData(ts)->GetVector(varName);
        if (!vmd)
        {
            debug5 << "Querying vector var, but could not retrieve"
                   << " meta data!" << endl;
            return false;
        }
        stringVector names;
        doubleVector vals;
        char buff[80];

        //
        // admissibleDataTypes and needNativePrecision are placeholders for
        // information that will eventually come from either new args to
        // this method or from PickVarInfo
        //
        vector<int> admissibleDataTypes;
        admissibleDataTypes.push_back(VTK_FLOAT);
        bool needNativePrecision = false;
        vtkDataArray *vectors = GetVectorVariable(varName.c_str(), ts, dom,
                                                  "_all", needNativePrecision,
                                                  admissibleDataTypes);
        int nComponents = 0;; 
        double *temp = NULL; 
        double mag = 0.;
        if (vectors)
        {
            bool zoneCent = false, validCentering = true;
            if (vmd->centering == AVT_NODECENT)
            {
                varInfo.SetCentering(PickVarInfo::Nodal);
                zoneCent = false;
            }
            else if (vmd->centering == AVT_ZONECENT)
            {
                varInfo.SetCentering(PickVarInfo::Zonal);
                zoneCent = true;
            }
            else 
            {
                validCentering = false;
            }
            if (validCentering)
            {
                nComponents = vectors->GetNumberOfComponents();
                temp = new double[nComponents];
                if (zonePick != zoneCent) 
                { 
                    // info we're after is associated with incidentElements
                    for (int k = 0; k < incidentElements.size(); k++)
                    {
                        sprintf(buff, "(%d)", incidentElements[k]); 
                        names.push_back(buff); 
                        vectors->GetTuple(incidentElements[k], temp);
                        mag = 0.;
                        for (int i = 0; i < nComponents; i++)
                        {
                            vals.push_back(temp[i]);
                            mag += (temp[i] * temp[i]); 
                        }
                        mag = sqrt(mag);
                        vals.push_back(mag);
                    }
                }
                else 
                {
                    // info we're after is associated with element 
                    sprintf(buff, "(%d)", element);
                    names.push_back(buff); 
                    vectors->GetTuple(element, temp);
                    mag = 0.;
                    for (int i = 0; i < nComponents; i++)
                    {
                        vals.push_back(temp[i]);
                        mag +=  (temp[i] * temp[i]); 
                    }
                    mag = sqrt(mag);
                    vals.push_back(mag);
                }
                delete [] temp;
            }
        }
        if (!vals.empty())
        {
            varInfo.SetNames(names);
            varInfo.SetValues(vals);
            vals.clear();
            names.clear();
            rv = true;
        }
    }
    // 
    // This is where we could allow the interface to add more information.
    // 
    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::QueryTensors
//
//  Purpose:
//    Queries the db regarding tensor var info for a specific cell or nodes.  
//
//  Arguments:
//    varName     The variable on which to retrieve data.
//    dom         The domain to query.
//    zone        The zone to query.
//    ts          The timestep to query.
//    nodes       The nodes to query.
//    varInfo     A place to store the results. 
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Hank Childs
//  Creation:     September 22, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Aug 20 14:05:54 PDT 2004
//    Initialize variable to remove compiler warning.
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'std::vector<int>' to 'intVector', 'std::vector<std::string>'
//    to 'stringVector', and 'std::vector<double>' to 'doubleVector'.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added dummy args for data type conversion for calls to get variable
//
// ****************************************************************************

bool
avtGenericDatabase::QueryTensors(const string &varName, const int dom, 
                                 const int element, const int ts, 
                                 const intVector &incidentElements, 
                                 PickVarInfo &varInfo, const bool zonePick)
{
    bool rv = false;
    if (varInfo.GetValues().empty())
    {
        const avtTensorMetaData *tmd = GetMetaData(ts)->GetTensor(varName);
        if (!tmd)
        {
            debug5 << "Querying tensor var, but could not retrieve"
                   << " meta data!" << endl;
            return false;
        }

        stringVector names;
        doubleVector vals;
        char buff[80];

        //
        // admissibleDataTypes and needNativePrecision are placeholders for
        // information that will eventually come from additional args to this
        // method or PickVarInfo
        //
        vector<int> admissibleDataTypes;
        admissibleDataTypes.push_back(VTK_FLOAT);
        bool needNativePrecision = false;
        vtkDataArray *tensors = GetTensorVariable(varName.c_str(), ts, dom,
                                                  "_all", needNativePrecision,
                                                  admissibleDataTypes);
        int nComponents = 0;; 
        double *temp = NULL; 
        if (tensors)
        {
            bool zoneCent = false, validCentering = true;
            if (tmd->centering == AVT_NODECENT)
            {
                varInfo.SetCentering(PickVarInfo::Nodal);
                zoneCent = false;
            }
            else if (tmd->centering == AVT_ZONECENT)
            {
                varInfo.SetCentering(PickVarInfo::Zonal);
                zoneCent = true;
            }
            else 
            {
                validCentering = false;
            }
            if (validCentering)
            {
                nComponents = tensors->GetNumberOfComponents();
                temp = new double[nComponents];
                if (zonePick != zoneCent) 
                { 
                    // info we're after is associated with incidentElements
                    for (int k = 0; k < incidentElements.size(); k++)
                    {
                        sprintf(buff, "(%d)", incidentElements[k]); 
                        names.push_back(buff); 
                        tensors->GetTuple(incidentElements[k], temp);
                        for (int i = 0; i < nComponents; i++)
                            vals.push_back(temp[i]);
                    }
                }
                else 
                {
                    // info we're after is associated with element 
                    sprintf(buff, "(%d)", element);
                    names.push_back(buff); 
                    tensors->GetTuple(element, temp);
                    for (int i = 0; i < nComponents; i++)
                        vals.push_back(temp[i]);
                }
                delete [] temp;
            }
        }
        if (!vals.empty())
        {
            varInfo.SetNames(names);
            varInfo.SetValues(vals);
            vals.clear();
            names.clear();
            rv = true;
        }
    }
    // 
    // This is where we could allow the interface to add more information.
    // 
    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::QuerySymmetricTensors
//
//  Purpose:
//    Queries the db regarding tensor var info for a specific cell or nodes.  
//
//  Arguments:
//    varName     The variable on which to retrieve data.
//    dom         The domain to query.
//    zone        The zone to query.
//    ts          The timestep to query.
//    nodes       The nodes to query.
//    varInfo     A place to store the results. 
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Hank Childs
//  Creation:     September 22, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Aug 20 14:05:54 PDT 2004
//    Initialize variable to remove compiler warning.
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'std::vector<int>' to 'intVector', 'std::vector<std::string>'
//    to 'stringVector', and 'std::vector<double>' to 'doubleVector'.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added dummy args for data type conversion for calls to get variable
//
// ****************************************************************************

bool
avtGenericDatabase::QuerySymmetricTensors(const string &varName,
                                 const int dom, const int element,const int ts,
                                 const intVector &incidentElements,
                                 PickVarInfo &varInfo, const bool zonePick)
{
    bool rv = false;
    if (varInfo.GetValues().empty())
    {
        const avtSymmetricTensorMetaData *tmd 
                                     = GetMetaData(ts)->GetSymmTensor(varName);
        if (!tmd)
        {
            debug5 << "Querying tensor var, but could not retrieve"
                   << " meta data!" << endl;
            return false;
        }

        stringVector names;
        doubleVector vals;
        char buff[80];

        //
        // admissibleDataTypes and needNativePrecision are placeholds for 
        // information that will eventually come from new args to this method
        // or from PickVarInfo
        //
        bool needNativePrecision = false;
        vector<int> admissibleDataTypes;
        admissibleDataTypes.push_back(VTK_FLOAT);
        vtkDataArray *tensors = GetSymmetricTensorVariable(varName.c_str(), ts,
                                                           dom, "_all",
                                                           needNativePrecision,
                                                           admissibleDataTypes);
        int nComponents = 0;; 
        if (tensors)
        {
            bool zoneCent = false, validCentering = true;
            if (tmd->centering == AVT_NODECENT)
            {
                varInfo.SetCentering(PickVarInfo::Nodal);
                zoneCent = false;
            }
            else if (tmd->centering == AVT_ZONECENT)
            {
                varInfo.SetCentering(PickVarInfo::Zonal);
                zoneCent = true;
            }
            else 
            {
                validCentering = false;
            }
            if (validCentering)
            {
                nComponents = tensors->GetNumberOfComponents();
                double *temp = new double[nComponents];
                if (zonePick != zoneCent) 
                { 
                    // info we're after is associated with incidentElements
                    for (int k = 0; k < incidentElements.size(); k++)
                    {
                        sprintf(buff, "(%d)", incidentElements[k]); 
                        names.push_back(buff); 
                        tensors->GetTuple(incidentElements[k], temp);
                        for (int i = 0; i < nComponents; i++)
                            vals.push_back(temp[i]);
                    }
                }
                else 
                {
                    // info we're after is associated with element 
                    sprintf(buff, "(%d)", element);
                    names.push_back(buff); 
                    tensors->GetTuple(element, temp);
                    for (int i = 0; i < nComponents; i++)
                        vals.push_back(temp[i]);
                }
                delete [] temp;
            }
        }
        if (!vals.empty())
        {
            varInfo.SetNames(names);
            varInfo.SetValues(vals);
            vals.clear();
            names.clear();
            rv = true;
        }
    }
    // 
    // This is where we could allow the interface to add more information.
    // 
    return rv;
}

// ****************************************************************************
// Method: avtGenericDatabase::QueryLabels
//
// Purpose: 
//   Queries the database for a label dataset.
//
// Arguments:
//    varName     The variable on which to retrieve data.
//    dom         The domain to query.
//    zone        The zone to query.
//    ts          The timestep to query.
//    nodes       The nodes to query.
//    varInfo     A place to store the results. 
//
// Returns:    True if data was retrieved; otherwise false.
//
// Note:       Pretty much the same as QueryVectors except that we don't
//             calculate a magnitude.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 4 11:51:56 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool
avtGenericDatabase::QueryLabels(const string &varName, const int dom, 
                                 const int element, const int ts, 
                                 const intVector &incidentElements, 
                                 PickVarInfo &varInfo, const bool zonePick)
{
    bool rv = false;
    if (varInfo.GetValues().empty())
    {
        const avtLabelMetaData *lmd = GetMetaData(ts)->GetLabel(varName);
        if (!lmd)
        {
            debug5 << "Querying label var, but could not retrieve"
                   << " meta data!" << endl;
            return false;
        }
        stringVector names;
        doubleVector vals;
        char buff[80];
        vtkDataArray *labels = GetLabelVariable(varName.c_str(), ts, dom,
                                                "_all");
        int nComponents = 0; 
        double *temp = NULL; 
        if (labels)
        {
            bool zoneCent = false, validCentering = true;
            if (lmd->centering == AVT_NODECENT)
            {
                varInfo.SetCentering(PickVarInfo::Nodal);
                zoneCent = false;
            }
            else if (lmd->centering == AVT_ZONECENT)
            {
                varInfo.SetCentering(PickVarInfo::Zonal);
                zoneCent = true;
            }
            else 
            {
                validCentering = false;
            }
            if (validCentering)
            {
                nComponents = labels->GetNumberOfComponents();
                temp = new double[nComponents];
                if (zonePick != zoneCent) 
                { 
                    // info we're after is associated with incidentElements
                    for (int k = 0; k < incidentElements.size(); k++)
                    {
                        sprintf(buff, "(%d)", incidentElements[k]); 
                        names.push_back(buff); 
                        labels->GetTuple(incidentElements[k], temp);
                        for (int i = 0; i < nComponents; i++)
                            vals.push_back(temp[i]);
                    }
                }
                else 
                {
                    // info we're after is associated with element 
                    sprintf(buff, "(%d)", element);
                    names.push_back(buff); 
                    labels->GetTuple(element, temp);
                    for (int i = 0; i < nComponents; i++)
                        vals.push_back(temp[i]);
                }
                delete [] temp;
            }
        }
        if (!vals.empty())
        {
            varInfo.SetNames(names);
            varInfo.SetValues(vals);
            vals.clear();
            names.clear();
            rv = true;
        }
    }
    // 
    // This is where we could allow the interface to add more information.
    // 
    return rv;
}

// ****************************************************************************
//  Method: avtGenericDatabase::QueryMaterial
//
//  Purpose:
//    Queries the db regarding material var info for a specific cell or nodes.  
//
//  Arguments:
//    varName     The variable on which to retrieve data.
//    dom         The domain to query.
//    zone        The zone to query.
//    ts          The timestep to query.
//    nodes       The nodes to query.
//    varInfo     A place to store the results. 
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     June 27, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Don't set variable name, it should be set already.
//
//    Kathleen Bonnell, Fri Dec  6 12:10:04 PST 2002   
//    Added test for zone out of range in regards to material zones. 
//
//    Kathleen Bonnell, Fri Jun 20 13:57:30 PDT 2003  
//    Add support for node-pick. 
//    
//    Kathleen Bonnell, Thu Nov 20 15:11:52 PST 2003
//    Removed call to varInfo.SetVarIsMaterial.
//    
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'std::vector<int>' to 'intVector', 'std::vector<std::string>'
//    to 'stringVector', and 'std::vector<double>' to 'doubleVector'.
//    
// ****************************************************************************

bool
avtGenericDatabase::QueryMaterial(const string &varName, const int dom, 
                                  const int element, const int ts, 
                                  const intVector &incidentElements, 
                                  PickVarInfo &varInfo, const bool zonePick)
{
    doubleVector volFracs;
    stringVector matNames;
    stringVector zoneNames;
    intVector    nMats;
    vector<CellMatInfo> matInfo; 
    avtMaterial *mat = GetMaterial(dom, varName.c_str(), ts);
    int i, j;
    if (mat == NULL)
    {
        return false;
    }

    char buff[80];
    int numMatsThisZone;
    if (zonePick) // zone number is stored in element
    {
        if (element < 0 || element >= mat->GetNZones())
        {
            debug5 << "CANNOT QUERY MATERIALS ZONE IS OUT OF RANGE" << endl;
            return false;
        }
        numMatsThisZone = 0;
        matInfo = mat->ExtractCellMatInfo(element);
        for (i = 0; i < matInfo.size(); i++)
        {
            if (matInfo[i].vf > 0.)
            {
                matNames.push_back(matInfo[i].name);
                volFracs.push_back(matInfo[i].vf);
                numMatsThisZone++;
            }
        }
        nMats.push_back(numMatsThisZone);
    }
    else // zones are stored in incidentElements.
    {
        bool zonesInRange = true;
        int nmatzones = mat->GetNZones();
        for (i = 0; i < incidentElements.size(); i++)
        {
            if (incidentElements[i] < 0 || incidentElements[i] >= nmatzones)
            {
                zonesInRange = false;
                break; 
            }
        }
        if (!zonesInRange)
        {
            debug5 << "CANNOT QUERY MATERIALS ZONE IS OUT OF RANGE" << endl;
            return false;
        }
        for (j = 0; j < incidentElements.size(); j++)
        {
            numMatsThisZone = 0;
            sprintf(buff, "(%d)", incidentElements[j]);
            zoneNames.push_back(buff);
            matInfo = mat->ExtractCellMatInfo(incidentElements[j]);
            for (i = 0; i < matInfo.size(); i++)
            {
                if (matInfo[i].vf > 0.)
                {
                    matNames.push_back(matInfo[i].name);
                    volFracs.push_back(matInfo[i].vf);
                    numMatsThisZone++;
                }
            }
            nMats.push_back(numMatsThisZone);
        }
    }
    varInfo.SetCentering(PickVarInfo::None);
    varInfo.SetNames(zoneNames);
    varInfo.SetMixNames(matNames);
    varInfo.SetMixValues(volFracs);
    varInfo.SetNumMatsPerZone(nMats);

    if (!matNames.empty())
        matNames.clear();
    if (!volFracs.empty())
        volFracs.clear();
    if (!zoneNames.empty())
        zoneNames.clear();
    if (!nMats.empty())
        nMats.clear();

    // 
    // This is where we could allow the interface to add more information.
    // 
    return true;
}


// ****************************************************************************
//  Method: avtGenericDatabase::QueryNodes
//
//  Purpose:
//    Queries the db regarding material var info for a specific cell or nodes.  
//
//  Arguments:
//    varName       The variable on which to retrieve data.
//    dom           The domain to query.
//    zone          The zone to query.
//    ts            The timestep to query.
//    nodes         A place to store the nodes.
//    ppt           A place to store the 'real' picked point for point meshes.
//    useNodeCoords Whether or not to fill in the node coordinates.
//    logicalNodes  Whether or not to use logical indices for the node coordinates.
//    nCoords       A place to store the node coordinates.
//    logicalZones  Whether or not to fill in the zone coordinates.
//    zCoords       A place to store the zone coordinates.
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     December 6, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 27 14:09:40 PST 2002
//    Added useNodeCoords && nodeCoords arguments.  Fill in nodeCoords as
//    needed.
//
//    Kathleen Bonnell, Tue Sep 16 13:33:30 PDT 2003 
//    Use "base_index" if available when creating logical coords. 
//    
//    Kathleen Bonnell, Tue Nov 18 14:07:13 PST 2003 
//    Added support for logical zone coords. 
//    
//    Kathleen Bonnell, Wed Nov 26 14:35:29 PST 2003
//    Set ppt to Cell center if doing PickByZone (ppt[0] == FLT_MAX). 
//    
//    Kathleen Bonnell, Wed Dec 17 15:04:57 PST 2003 
//    Updated args list to include multiple types of Coordinates. 
//    
//    Kathleen Bonnell, Wed Oct 20 17:01:38 PDT 2004 
//    Replaced get-cell-center code with single call to 
//    vtkVisItUtility::GetCellCenter.
//    
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'std::vector<int>' to 'intVector', and 'std::vector<std::string>'
//    to 'stringVector'.
//    
//    Hank Childs, Sun Mar 13 13:42:04 PST 2005
//    Fix memory leak.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added dummy args for data type conversion for calls to get variable
//
// ****************************************************************************

bool
avtGenericDatabase::QueryNodes(const string &varName, const int dom, 
                               const int zone, bool &zoneIsGhost, const int ts, 
                               intVector &nodes, intVector &ghostNodes, 
                               const bool includeGhosts, float ppt[3],
                               const int dim, const bool physicalNodes, 
                               const bool logicalDNodes, const bool logicalBNodes,
                               stringVector &pnCoords, stringVector &dnCoords,
                               stringVector &bnCoords,
                               const bool logicalDZones, const bool logicalBZones,
                               stringVector &dzCoords, stringVector &bzCoords)
{
    // needNativePrecision and admissibleDataTypes are placeholders for when
    // this information will come from somewhere else
    bool needNativePrecision = false;
    vector<int> admissibleDataTypes;
    admissibleDataTypes.push_back(VTK_FLOAT);
    string meshName = GetMetaData(ts)->MeshForVar(varName);
    vtkDataSet *ds = GetMeshDataset(meshName.c_str(), ts, dom, "_all",
                                    needNativePrecision, admissibleDataTypes);
    bool rv = false; 
    if (ds)
    {
        vtkIdList *ptIds = vtkIdList::New();
        ds->GetCellPoints(zone, ptIds);
        float coord[3];
        int ijk[3];
        char buff[80];
        int type = ds->GetDataObjectType();
        unsigned char *gn = NULL;
        unsigned char *gz = NULL;
        if (includeGhosts)
        {
            vtkUnsignedCharArray *gzone = 
               (vtkUnsignedCharArray*) ds->GetCellData()->GetArray("avtGhostZones");
            if (gzone)
            {
                gz = gzone->GetPointer(0);
                if (gz[zone])
                    zoneIsGhost = true;
            }
            vtkUnsignedCharArray *gnode = 
               (vtkUnsignedCharArray*) ds->GetPointData()->GetArray("avtGhostNodes");
            if (gnode)
                gn = gnode->GetPointer(0);
        }
        if (logicalDZones && (type == VTK_RECTILINEAR_GRID ||
                             type == VTK_STRUCTURED_GRID ))
        {
            vtkVisItUtility::GetLogicalIndices(ds, true, zone, ijk, false);
            if (dim == 2)
            {
                sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
            }
            else 
            {
                sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
            }
            dzCoords.push_back(buff);
        }
        if (logicalBZones && (type == VTK_RECTILINEAR_GRID ||
                             type == VTK_STRUCTURED_GRID ))
        {
            vtkVisItUtility::GetLogicalIndices(ds, true, zone, ijk, true);
            if (dim == 2)
            {
                sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
            }
            else 
            {
                sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
            }
            bzCoords.push_back(buff);
        }
        vtkIdList *cells = vtkIdList::New();
        int nGnodes = 0;
        for (int i = 0; i < ptIds->GetNumberOfIds(); i++)
        {
            vtkIdType id = ptIds->GetId(i);
            if (includeGhosts) 
            {
                if (gn && gn[id])
                {
                    ghostNodes.push_back(1);
                    nGnodes++;
                }
                else if (gz)
                {
                    int nGhosts = 0;
                    ds->GetPointCells(id, cells);
                    for (int j = 0; j < cells->GetNumberOfIds(); j++)
                    {
                        nGhosts += (int)gz[cells->GetId(j)];
                    }
                    if (nGhosts > 0 && nGhosts == cells->GetNumberOfIds())
                    {
                        ghostNodes.push_back(1);
                        nGnodes++;
                    }
                    else 
                    {
                        ghostNodes.push_back(0);
                    }
                }
                else 
                {
                    ghostNodes.push_back(0);
                }
            }
            nodes.push_back(id);
            if (logicalDNodes && (type == VTK_RECTILINEAR_GRID ||
                                 type == VTK_STRUCTURED_GRID ))
            {
                vtkVisItUtility::GetLogicalIndices(ds, false, 
                                         ptIds->GetId(i), ijk, false);
                if (dim == 2)
                {
                    sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
                }
                else 
                {
                    sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
                }
                dnCoords.push_back(buff);
            }
            if (logicalBNodes && (type == VTK_RECTILINEAR_GRID ||
                                 type == VTK_STRUCTURED_GRID ))
            {
                vtkVisItUtility::GetLogicalIndices(ds, false, 
                                         ptIds->GetId(i), ijk, true);
                if (dim == 2)
                {
                    sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
                }
                else 
                {
                    sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
                }
                bnCoords.push_back(buff);
            }
            if (physicalNodes)
            {
                ds->GetPoint(ptIds->GetId(i), coord);
                if (dim == 2)
                {
                    sprintf(buff, "<%g, %g>", coord[0], coord[1]);
                }
                else 
                {
                    sprintf(buff, "<%g, %g, %g>", coord[0], coord[1], coord[2]);
                }
                pnCoords.push_back(buff);
            }
        }
        cells->Delete();
        zoneIsGhost |= (nGnodes > 0 && nGnodes == ptIds->GetNumberOfIds());
        ptIds->Delete();
        if (nodes.size() == 1) // point mesh
        {
            //
            //  Set pick point to be the same as the cell
            //  node for point mesh.
            //
            ds->GetPoint(nodes[0], ppt);
        }
        else if (ppt[0] == FLT_MAX)
        {
            vtkVisItUtility::GetCellCenter(ds->GetCell(zone), ppt);
        }
        rv = true;
        ds->Delete();
    }
    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::QueryMesh
//
//  Purpose:
//    Queries the db regarding mesh info for a domain. 
//
//  Arguments:
//    varName     The variable on which to retrieve data.
//    dom         The domain to query.
//    meshInfo     A place to store the results. 
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     April 18, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Tue Sep  9 16:51:10 PDT 2003
//    Changed PickVarInfo argument to std::string. 
//
//    Kathleen Bonnell, Wed Jun  9 17:41:00 PDT 2004 
//    Added showName argument. 
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Removed use of 'std::'. 
//    
// ****************************************************************************

bool
avtGenericDatabase::QueryMesh(const string &varName, const int ts,
                              const int dom, string &meshInfo,
                              const bool showName)
{
    bool rv = false;
    string mesh = GetMetaData(ts)->MeshForVar(varName);
    const avtMeshMetaData *mmd = GetMetaData(ts)->GetMesh(mesh);
    char temp[256];
    if (!mmd)
    {
         debug5 << "Querying mesh, but could not retrieve"
                << " meta data!" << endl;
         return false;
    }
    if (showName)
    {
        sprintf(temp, "%s ", mesh.c_str());
        meshInfo += temp;
    }
    if (mmd->numGroups > 0 && dom < mmd->groupIds.size())
    {
         sprintf(temp, "%s %d " , mmd->groupPieceName.c_str(), 
                 mmd->groupIds[dom]);
         meshInfo += temp;
         rv = true;
    }
    if (mmd->numBlocks > 1)
    {
        if ( mmd->blockNames.size() == 0)
        {
             sprintf(temp, "%s %d " , mmd->blockPieceName.c_str(), 
                     dom + mmd->blockOrigin);
             meshInfo += temp;
        }
        else 
        {
             sprintf(temp, "%s %s " , mmd->blockPieceName.c_str(), 
                     mmd->blockNames[dom].c_str());
             meshInfo += temp;
        }
        rv = true;
    }
    // 
    // This is where we could allow the interface to add more information.
    // 
    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::QueryZones
//
//  Purpose:
//    Queries the db regarding zones incident upon a specific node. 
//
//  Arguments:
//    varName       The variable on which to retrieve data.
//    dom           The domain to query.
//    foundEl       IN:  zone that contains the picked point. 
//                  OUT: The node closest to the picked point.
//    ts            The timestep to query.
//    zones         A place to store the zones.
//    ppt           IN:  The picked point. 
//                  OUT: The node coordinates. 
//    dimension     The spatial dimension.
//    useNodeCoords Whether or not to supply coordinates for the node.
//    logicalNodes  Whether the node coords should be logical. 
//    nodeCoords    A place to store the node coordinates.
//    logicalZones  Whether or not to supply coordinates for the zone. 
//    zoneCoords    A place to store the zone coordinates.
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     June 20, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Tue Sep 16 13:33:30 PDT 2003 
//    Use "base_index" if available when creating logical coords. 
//    
//    Kathleen Bonnell, Tue Nov 18 14:07:13 PST 2003 
//    Added support for logical zone coords. 
//    
//    Kathleen Bonnell, Wed Nov 26 14:35:29 PST 2003
//    Use foundEl as minId if doing PickByNode (ppt[0] == FLT_MAX). 
//    
//    Kathleen Bonnell, Wed Dec 17 15:04:57 PST 2003 
//    Updated args list to include multiple types of Coordinates. 
//    
//    Kathleen Bonnell, Thu Jun 17 12:58:47 PDT 2004 
//    Only search for the node if it hasn't already been discovered, 
//    use the generic ds->GetPoint() method.
//    
//    Hank Childs, Fri Aug 27 16:16:52 PDT 2004
//    Rename ghost data arrays.
//
//    Kathleen Bonnell, Thu Sep 23 17:48:37 PDT 2004 
//    Added args to support ghost-zone retrieval if requested.
//
//    Kathleen Bonnell, Thu Oct 21 18:02:50 PDT 2004 
//    Correctly test whether a zone is ghost or not. 
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'std::vector<int>' to 'intVector', and 
//    'std::vector<std::string>'.
//    
//    Hank Childs, Thu Mar 10 10:23:02 PST 2005
//    Removed memory leak.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added dummy args for data type conversion for calls to get variable
//
// ****************************************************************************

bool
avtGenericDatabase::QueryZones(const string &varName, const int dom, 
                               int &foundEl, bool &elIsGhost, 
                               const int ts, intVector &zones, 
                               intVector &ghostZ, bool includeGhosts,
                               float ppt[3], const int dimension,
                               const bool physicalNodes, 
                               const bool logicalDNodes, 
                               const bool logicalBNodes, 
                               stringVector &pnodeCoords,
                               stringVector &dnodeCoords,
                               stringVector &bnodeCoords,
                               const bool logicalDZones, 
                               const bool logicalBZones, 
                               stringVector &dzoneCoords,
                               stringVector &bzoneCoords)
{
    // needNativePrecision and admissibleDataTypes are placeholders for when
    // this information will come from somewhere else
    bool needNativePrecision = false;
    vector<int> admissibleDataTypes;
    admissibleDataTypes.push_back(VTK_FLOAT);
    string meshName = GetMetaData(ts)->MeshForVar(varName);
    vtkDataSet *ds = GetMeshDataset(meshName.c_str(), ts, dom, "_all",
                         needNativePrecision, admissibleDataTypes);
    bool rv = false; 
    if (ds)
    {
        vtkIdList *ids = vtkIdList::New();
        vtkIdType *idptr; 
        vtkIdType minId = foundEl;
        float coord[3];
        int ijk[3];
        char buff[80];
        int type = ds->GetDataObjectType();
        if (minId == -1)
        {
            minId = ds->FindPoint(ppt);
        }

        if ( minId != -1)
        {
            ppt[0] = ds->GetPoint(minId)[0];
            ppt[1] = ds->GetPoint(minId)[1];
            ppt[2] = ds->GetPoint(minId)[2];

            foundEl = minId;

            if (logicalDNodes  && (type == VTK_STRUCTURED_GRID || 
                                   type == VTK_RECTILINEAR_GRID))
            {
                vtkVisItUtility::GetLogicalIndices(ds, false, minId, ijk, false);
                if (dimension == 2)
                {
                    sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
                }
                else 
                {
                    sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
                }
                dnodeCoords.push_back(buff);
            }
            if (logicalBNodes  && (type == VTK_STRUCTURED_GRID || 
                                   type == VTK_RECTILINEAR_GRID))
            {
                vtkVisItUtility::GetLogicalIndices(ds, false, minId, ijk, true);
                if (dimension == 2)
                {
                    sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
                }
                else 
                {
                    sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
                }
                bnodeCoords.push_back(buff);
            }
            if (physicalNodes)
            {
                ds->GetPoint(minId, coord); 
                if (dimension  == 2)
                {
                    sprintf(buff, "<%g, %g>", coord[0], coord[1]);
                }
                else 
                {
                    sprintf(buff, "<%g, %g, %g>", coord[0], coord[1], coord[2]);
                }
                pnodeCoords.push_back(buff);
            }
            ds->GetPointCells(minId, ids);
            int nCells = ids->GetNumberOfIds();
            if (nCells > 0)
            {
                vtkUnsignedCharArray *ghostArray = (vtkUnsignedCharArray*)
                    ds->GetCellData()-> GetArray("avtGhostZones");   
                unsigned char *ghosts = NULL;
                if (ghostArray)
                    ghosts = ghostArray->GetPointer(0);
                int nGhosts = 0;
                idptr = ids->GetPointer(0);
                for (int i = 0;i < nCells; i++)
                {
                    if (ghosts && ghosts[idptr[i]] > 0)
                    {
                        if (includeGhosts)
                        {
                            ghostZ.push_back(1);
                            nGhosts++;
                        }
                        else
                            continue; 
                    }
                    else if (includeGhosts)
                    {
                        ghostZ.push_back(0);
                    }
                    zones.push_back(idptr[i]);

                    if (logicalDZones && (type == VTK_STRUCTURED_GRID || 
                                          type == VTK_RECTILINEAR_GRID))
                    {
                        vtkVisItUtility::GetLogicalIndices(ds, true, idptr[i], 
                                              ijk, false);
                        if (dimension == 2)
                        {
                            sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
                        }
                        else 
                        {
                            sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
                        }
                        dzoneCoords.push_back(buff);
                    }
                    if (logicalBZones && (type == VTK_STRUCTURED_GRID || 
                                          type == VTK_RECTILINEAR_GRID))
                    {
                        vtkVisItUtility::GetLogicalIndices(ds, true, idptr[i], 
                                              ijk, true);
                        if (dimension == 2)
                        {
                            sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
                        }
                        else 
                        {
                            sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
                        }
                        bzoneCoords.push_back(buff);
                    }
                    elIsGhost |= (nGhosts == nCells);
                }
                rv = true;
            }
        } // found valid point
        ids->Delete();
        ds->Delete();
    }
    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::AssociateBounds
//
//  Purpose:
//      Calls GetBounds on the dataset one time and associates the information
//      with the dataset.  This avoids having the routine called repeatedly.
//
//  Programmer: Hank Childs
//  Creation:   July 29, 2003
//
// ****************************************************************************

void
avtGenericDatabase::AssociateBounds(vtkDataSet *ds)
{
    float bounds[6];
    ds->GetBounds(bounds);
    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetName("avtOriginalBounds");
    arr->SetNumberOfTuples(6);
    for (int i = 0 ; i < 6 ; i++)
        arr->SetTuple1(i, bounds[i]);
    ds->GetFieldData()->AddArray(arr);
    arr->Delete();
}


// ****************************************************************************
//  Method: avtGenericDatabase::ScaleMesh
//
//  Purpose:
//      Scales the mesh in if it is too big.
//
//  Programmer: Hank Childs
//  Creation:   July 29, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Dec 16 09:49:13 PST 2003
//    Improve clarity of warning message.
//
//    Hank Childs, Sat Mar 27 11:51:10 PST 2004
//    Do not let scaling go into an infinite loop.
//
//    Hank Childs, Thu Apr 22 07:56:13 PDT 2004
//    Make sure that all of the dimensions are too big or too small, not just
//    one of them.
//
// ****************************************************************************

void
avtGenericDatabase::ScaleMesh(vtkDataSet *ds)
{
    bool needXScaling = false;
    bool needYScaling = false;
    bool needZScaling = false;
    double scaleFactor = 1.;

    float bounds[6];
    if (ds->GetFieldData()->GetArray("avtOriginalBounds") != NULL)
    {
        vtkDataArray *arr = ds->GetFieldData()->GetArray("avtOriginalBounds");
        for (int i = 0 ; i < 6 ; i++)
            bounds[i] = arr->GetTuple1(i);
    }
    else
    {
        ds->GetBounds(bounds);
    }

    if (bounds[1] - bounds[0] > 1e8)
    {
        scaleFactor = 1.;
        double temp = bounds[1] - bounds[0];
        int iterations = 0;
        while (temp > 1e6 && (iterations++ < 100))
        {
            temp /= 10.;
            scaleFactor *= 10.;
        }
        needXScaling = true;
    }
    if ((bounds[1] > bounds[0]) && (bounds[1] - bounds[0] < 1e-6))
    {
        scaleFactor = 1.;
        double temp = bounds[1] - bounds[0];
        int iterations = 0;
        while (temp < 1e-3 && (iterations++ < 100))
        {
            temp *= 10.;
            scaleFactor /= 10.;
        }
        needXScaling = true;
    }
    if (bounds[3] - bounds[2] > 1e8)
    {
        scaleFactor = 1.;
        double temp = bounds[3] - bounds[2];
        int iterations = 0;
        while (temp > 1e6 && (iterations++ < 100))
        {
            temp /= 10.;
            scaleFactor *= 10.;
        }
        needYScaling = true;
    }
    if ((bounds[3] > bounds[2]) && (bounds[3] - bounds[2] < 1e-6))
    {
        scaleFactor = 1.;
        double temp = bounds[3] - bounds[2];
        int iterations = 0;
        while (temp < 1e-3 && (iterations++ < 100))
        {
            temp *= 10.;
            scaleFactor /= 10.;
        }
        needYScaling = true;
    }
    if (bounds[5] - bounds[4] > 1e8)
    {
        scaleFactor = 1.;
        double temp = bounds[5] - bounds[4];
        int iterations = 0;
        while (temp > 1e6 && (iterations++ < 100))
        {
            temp /= 10.;
            scaleFactor *= 10.;
        }
        needZScaling = true;
    }
    if ((bounds[5] > bounds[4]) && (bounds[5] - bounds[4] < 1e-6))
    {
        scaleFactor = 1.;
        double temp = bounds[5] - bounds[4];
        int iterations = 0;
        while (temp < 1e-3 && (iterations++ < 100))
        {
            temp *= 10.;
            scaleFactor /= 10.;
        }
        needZScaling = true;
    }

    //
    // If any of the three dimensions don't need to be scaled, then don't
    // scale.  However, the Z-test is a little tricky, since we might only
    // have a 2D dataset.
    //
    if (!needXScaling || !needYScaling)
    {
        return;
    }
    if (!needZScaling && (bounds[4] != bounds[5]))
    {
        return;
    }
    
    static bool haveIssuedWarning = false;
    if (!haveIssuedWarning)
    {
        char msg[1024] = "The spatial extents of the dataset are too large or "
                         "too small for VisIt to "
                  "handle natively.  As a result, the dataset is being scaled."
                  "  This will affect the labels for axes, as well as "
                  "coordinate arguments, such as the origin for a slice."
                  "This message will only be issued one time per session, "
                  "even if additional datasets are scaled.";
        avtCallback::IssueWarning(msg);
        haveIssuedWarning = true;
    }

    int dstype = ds->GetDataObjectType();
    switch (dstype)
    {
      case VTK_STRUCTURED_GRID:
      case VTK_UNSTRUCTURED_GRID:
      case VTK_POLY_DATA:
        {
            vtkPointSet *ps = (vtkPointSet *) ds;
            vtkPoints *pts = ps->GetPoints();
            vtkPoints *newPts = vtkPoints::New();
            int npts = pts->GetNumberOfPoints();
            newPts->SetNumberOfPoints(npts);
            float *np = (float *) newPts->GetVoidPointer(0);
            float *p  = (float *) pts->GetVoidPointer(0);
            int nvals = 3*npts;
            for (int i = 0 ; i < nvals ; i++)
            {
                np[i] = p[i] / scaleFactor;
            }
            ps->SetPoints(newPts);
            newPts->Delete();
        }
        break;
      case VTK_RECTILINEAR_GRID:
        {
            vtkRectilinearGrid *rg = (vtkRectilinearGrid *) ds;
            vtkDataArray *in[3];
            in[0] = rg->GetXCoordinates();
            in[1] = rg->GetYCoordinates();
            in[2] = rg->GetZCoordinates();
            vtkFloatArray *out[3];
            for (int i = 0 ; i < 3 ; i++)
            {
                out[i] = vtkFloatArray::New();
                int ntuples = in[i]->GetNumberOfTuples();
                out[i]->SetNumberOfTuples(ntuples);
                for (int j = 0 ; j < ntuples ; j++)
                    out[i]->SetTuple1(j, in[i]->GetTuple1(j) / scaleFactor);
            }
            rg->SetXCoordinates(out[0]);
            rg->SetYCoordinates(out[1]);
            rg->SetZCoordinates(out[2]);
            out[0]->Delete();
            out[1]->Delete();
            out[2]->Delete();
        }
        break;

      default:
        debug1 << "Warning: a mesh should be scaled, but VisIt could not do "
               << "it because it was an unknown meshtype." << endl;
    }

    if (ds->GetFieldData()->GetArray("avtOriginalBounds") != NULL)
    {
        vtkDataArray *arr = ds->GetFieldData()->GetArray("avtOriginalBounds");
        for (int i = 0 ; i < 6 ; i++)
            arr->SetTuple1(i, arr->GetTuple1(i) / scaleFactor);
    }
}


// ****************************************************************************
//  Method: avtGenericDatabase::QuerySpecies
//
//  Purpose:
//    Queries the db regarding species var info for a specific cell/s. 
//
//  Arguments:
//    varName           The variable on which to retrieve data.
//    dom               The domain to query.
//    element           The element to query.
//    ts                The timestep to query.
//    incidentElements  The incident elements to query.
//    varInfo           A place to store the results. 
//    zonePick          Whether or not the pick was a zone pick.
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     November 20, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'std::vector<int>' to 'intVector', 'std::vector<std::string>'
//    to stringVector, 'std::vector<double>' to 'doubleVector'.  Removed
//    use of 'std::'. 
//    
//    Mark C. Miller, Mon Jan 10 14:06:06 PST 2005
//    Changed order of retrieval of mesh and variable to get the mesh
//    first and then the variable. This is so plugin can handle things
//    like removal of arb-poly zones correctly.
//
//    Hank Childs, Thu Mar 10 17:33:17 PST 2005
//    Fix memory leak.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added dummy args for data type conversion for calls to get variable
//
//    Kathleen Bonnell, Fri May 13 16:26:41 PDT 2005 
//    Fix memory leak.
//
// ****************************************************************************

bool
avtGenericDatabase::QuerySpecies(const string &varName, const int dom, 
                                 const int element, const int ts, 
                                 const intVector &incidentElements, 
                                 PickVarInfo &varInfo, const bool zonePick)
{
    const avtSpeciesMetaData *smd = GetMetaData(ts)->GetSpecies(varName);
    if (!smd)
    {
        debug5 << "Querying species var, but could not retrieve"
               << " meta data!" << endl;
        return false;
    }

    // 
    // Retrieve the species sum for the cell/s, if it has not been
    // retrieved already.
    // 
    string matName = smd->materialName;
    string meshname  = GetMetaData(ts)->MeshForVar(varName);
    bool needNativePrecision = false;
    vector<int> admissibleDataTypes;
    admissibleDataTypes.push_back(VTK_FLOAT);
    vtkDataSet *mesh = GetMesh(meshname.c_str(), ts, dom, matName.c_str(),
                           needNativePrecision, admissibleDataTypes);
    vtkDataArray *species = GetSpeciesVariable(varName.c_str(), ts, dom, 
                                matName.c_str(), mesh->GetNumberOfCells());
    mesh->Delete();
    avtMaterial *mat = GetMaterial(dom, matName.c_str(), ts);
    avtSpecies *spec = GetSpecies(dom, varName.c_str(), ts);

    if (species == NULL)
    {
        debug5 << "Querying species var, but could not retrieve"
               << " data array!" << endl;
        return false;
    }
    if (mat == NULL)
    {
        debug5 << "Querying species var, but could not retrieve"
               << " avtMaterial!" << endl;
        return false;
    }
    if (spec == NULL)
    {
        debug5 << "Querying species var, but could not retrieve"
               << " avtSpecies!" << endl;
        return false;
    }

    doubleVector vals = varInfo.GetValues();
    stringVector names;
    char buff[80];
    int i, j, k;
    varInfo.SetCentering(PickVarInfo::Zonal);
    bool getVal = (vals.size() == 0);

    if (!zonePick) 
    {
        // the info we're after is associated with incidentElements
        for (i = 0; i < incidentElements.size(); i++)
        {
            sprintf(buff, "(%d)", incidentElements[i]);
            names.push_back(buff);
            if (getVal)
                vals.push_back(species->GetTuple1(incidentElements[i]));
        }
    }
    else 
    {
        // the info we're after is associated with element
        sprintf(buff, "(%d)", element);
        names.push_back(buff);
        if (getVal)
            vals.push_back(species->GetTuple1(element));
    }
    species->Delete();

    //
    // Retrieve the mass fractions for each species within each material
    //

    intVector    nMats;
    stringVector matNames;
    intVector    nSpecs;
    stringVector specNames;
    doubleVector massFracs;

    vector<CellMatInfo> matInfo;

    int numMatsThisZone = 0;

    if (zonePick) // zone number is stored in element
    {
        if (element < 0 || element >= mat->GetNZones())
        {
            debug5 << "CANNOT QUERY SPECIES ZONE IS OUT OF RANGE" << endl;
            return false;
        }
        matInfo = mat->ExtractCellMatInfo(element);
        for (i = 0; i < matInfo.size(); i++)
        {
            int numSpecsThisMat = 0;
            vector<CellSpecInfo> specInfo;
            specInfo = spec->ExtractCellSpecInfo(element,matInfo[i].matno,mat);
            for (j = 0; j < specInfo.size(); j++)
            {
                specNames.push_back(specInfo[j].name);
                massFracs.push_back(specInfo[j].mf);
                numSpecsThisMat++;
            }
            if (specInfo.size() > 0)
            {
                matNames.push_back(matInfo[i].name);
                numMatsThisZone++;
                nSpecs.push_back(numSpecsThisMat);
            }
        }
        nMats.push_back(numMatsThisZone);
    }
    else
    {
        bool zonesInRange = true;
        int nmatzones = mat->GetNZones();
        for (i = 0; i < incidentElements.size(); i++)
        {
            if (incidentElements[i] < 0 || incidentElements[i] >= nmatzones)
            {
                zonesInRange = false;
                break; 
            }
        }
        if (!zonesInRange)
        {
            debug5 << "CANNOT QUERY MATERIALS ZONE IS OUT OF RANGE" << endl;
            return false;
        }
        for (j = 0; j < incidentElements.size(); j++)
        {
            numMatsThisZone = 0;
            matInfo = mat->ExtractCellMatInfo(incidentElements[j]);
            for (i = 0; i < matInfo.size(); i++)
            {
                int numSpecsThisMat = 0;
                vector<CellSpecInfo> specInfo;
                specInfo = spec->ExtractCellSpecInfo(incidentElements[j],
                    matInfo[i].matno,mat);
                for (k = 0; k < specInfo.size(); k++)
                {
                    specNames.push_back(specInfo[k].name);
                    massFracs.push_back(specInfo[k].mf);
                    numSpecsThisMat++;
                }
                if (specInfo.size() > 0)
                {
                    matNames.push_back(matInfo[i].name);
                    numMatsThisZone++;
                    nSpecs.push_back(numSpecsThisMat);
                }
            }
            nMats.push_back(numMatsThisZone);
        }
    }

    varInfo.SetNames(names);
    varInfo.SetValues(vals);
    varInfo.SetNumMatsPerZone(nMats);
    varInfo.SetMatNames(matNames);
    varInfo.SetNumSpecsPerMat(nSpecs);
    varInfo.SetMixNames(specNames);
    varInfo.SetMixValues(massFracs);

    if (!nMats.empty())
        nMats.clear();
    if (!matNames.empty())
        matNames.clear();
    if (!nSpecs.empty())
        nSpecs.clear();
    if (!specNames.empty())
        specNames.clear();
    if (!massFracs.empty())
        massFracs.clear();
 
    return true;
}

// ****************************************************************************
//  Method: avtGenericDatabase::FindElementForPoint
//
//  Purpose:
//     Searches for the node (elementName = 'node') closest to the passed 
//     point or the zone (elementName = 'zone') which contains the point.
//
//  Returns:
//      True for successful search, false otherwise.
// 
//  Arguments:
//    var       The variable to use in searching the database.
//    ts        The timestep to use in searching the database.
//    dom       The domain to use in searching the database.
//    elType    Specified which type of element (node, zone) to search for.
//    pt        The point to use in searching the database.
//    elNum     A place to store the zone or node number associated with the
//              point pt.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 13, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Mon Apr 19 15:49:05 PDT 2004 
//    Ensure that the timestep being queried is the active one.
//    
//    Hank Childs, Thu Mar 10 17:33:17 PST 2005
//    Fix memory leak.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added dummy args for data type conversion for calls to get variable
//
// ****************************************************************************

bool                
avtGenericDatabase::FindElementForPoint(const char *var, const int ts, 
                        const int dom, const char *elementName, 
                        float pt[3], int &elNum)
{
    ActivateTimestep(ts);

    // needNativePrecision and admissibleDataTypes are placeholders for when
    // this information will come from somewhere else
    bool needNativePrecision = false;
    vector<int> admissibleDataTypes;
    admissibleDataTypes.push_back(VTK_FLOAT);
    string mesh = GetMetaData(ts)->MeshForVar(var);
    vtkDataSet *ds = GetMeshDataset(mesh.c_str(), ts, dom, "_all",
                         needNativePrecision, admissibleDataTypes);

    if (strcmp(elementName, "node") == 0)
    {
        elNum = ds->FindPoint(pt);
    }
    else if (strcmp(elementName, "zone") == 0)
    {
        elNum = vtkVisItUtility::FindCell(ds, pt);
    }
    ds->Delete();

    return  (elNum != -1);
}
                                      
// ****************************************************************************
//  Method: avtGenericDatabase::GetDomainName
//
//  Purpose:
//    Gets a string representing the passed domain.
//
//  Arguments:
//    var       The variable to use in searching the database.
//    ts        The timestep to use in searching the database.
//    dom       The domain to use in searching the database.
//    domName   A place to store the domain name. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 22, 2003
//
//    Kathleen Bonnell, Mon Apr 19 15:49:05 PDT 2004 
//    Ensure that the timestep being queried is the active one.
//    
// ****************************************************************************

void
avtGenericDatabase::GetDomainName(const string &varName, const int ts,
                                const int dom, string &domName)
{
    ActivateTimestep(ts);

    string mesh = GetMetaData(ts)->MeshForVar(varName.c_str());
    const avtMeshMetaData *mmd = GetMetaData(ts)->GetMesh(mesh);
    char temp[256];
    if (mmd)
    {
        if (mmd->numBlocks > 1)
        {
            if ( mmd->blockNames.size() == 0)
            {
                 sprintf(temp, "%s %d" , mmd->blockPieceName.c_str(), 
                         dom + mmd->blockOrigin);
                 domName = temp;
            }
            else 
            {
                 sprintf(temp, "%s %s" , mmd->blockPieceName.c_str(), 
                         mmd->blockNames[dom].c_str());
                 domName = temp;
            }
        }
    }
}


// ****************************************************************************
//  Method: avtGenericDatabase::QueryCoords
//
//  Purpose:
//    Gets the geometric center of a zone, or node coords. 
//
//  Arguments:
//    var       The variable to use in searching the database.
//    dom       The domain to use in searching the database.
//    id        The zone or node number to use in searching the database.
//    ts        The timestep to use in searching the database.
//    coord     A place to store the zone center.
//    forZone   Whether to find coords for a zone or a node.
//
//  Returns:    True if query is a success, false otherwise.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 25, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu May 27 17:46:25 PDT 2004
//    Take ghost zones into account.
//
//    Kathleen Bonnell, Thu Jun 10 18:15:11 PDT 2004 
//    Renamed from QueryZoneCenter to QueryCoords, added bool arg. 
//
//    Hank Childs, Fri Aug 27 16:16:52 PDT 2004
//    Rename ghost data arrays.
//
//    Kathleen Bonnell, Wed Oct 20 17:01:38 PDT 2004 
//    Replaced get-cell-center code with single call to 
//    vtkVisItUtility::GetCellCenter.
//    
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Removed 'std::' from 'std::string'.
//    vtkVisItUtility::GetCellCenter.
//
//    Kathleen Bonnell, Thu Dec 16 17:11:19 PST 2004
//    Added bool arg specifying whether the id is global or not, find
//    local id if so.
//
//    Eric Brugger, Wed Dec 29 15:20:37 PST 2004
//    I added a call to ActivateTimestep to handle changing time steps.
//
//    Kathleen Bonnell, Tue Jan 25 07:59:28 PST 2005 
//    Added const char* arg to QueryCoords for meshName, use the meshname
//    to determine where the coords should come from.
//
//    Hank Childs, Thu Mar 10 10:23:02 PST 2005
//    Fix memory leak.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added dummy args for data type conversion for calls to get variable
//
// ****************************************************************************

bool
avtGenericDatabase::QueryCoords(const string &varName, const int dom, 
       const int id, const int ts, float coord[3], const bool forZone,
       const bool useGlobalId, const char *mN)
{
    ActivateTimestep(ts);

    int currentid = id;
    if (useGlobalId)
    {
        currentid = LocalIdForGlobal(dom, varName, ts, forZone, id);
        if (currentid == -1) 
            return false;
    }
    string meshName;
    if (mN == NULL || strcmp(mN, "default") == 0)
        meshName = GetMetaData(ts)->MeshForVar(varName);
    else 
        meshName = mN; 
    vtkDataSet *ds =  NULL;
    TRY
    {
        // needNativePrecision and admissibleDataTypes are placeholders for when
        // this information will come from somewhere else
        bool needNativePrecision = false;
        vector<int> admissibleDataTypes;
        admissibleDataTypes.push_back(VTK_FLOAT);
        ds = GetMeshDataset(meshName.c_str(), ts, dom, "_all",
                            needNativePrecision, admissibleDataTypes);
    }
    CATCH(BadDomainException)
    {
        ; // do nothing, possible if domain is not defined on specified mesh.
    }
    ENDTRY
    bool rv = false; 
    if (ds)
    {
        if (forZone)
        {
            int zone = currentid;
            if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID ||
                ds->GetDataObjectType() == VTK_STRUCTURED_GRID) 
            {
                if (ds->GetCellData()->GetArray("avtGhostZones") != NULL) 
                {
                    int dims[3], ijk[3] = {0, 0, 0};
                    vtkVisItUtility::GetDimensions(ds, dims);
                    vtkVisItUtility::GetLogicalIndices(ds, true, zone, ijk, false, false);
                    vtkIntArray *realDims = 
                        (vtkIntArray*)ds->GetFieldData()->GetArray("avtRealDims");
                    if (realDims != NULL)
                    {
                        ijk[0] += realDims->GetValue(0);
                        ijk[1] += realDims->GetValue(2);
                        ijk[2] += realDims->GetValue(4);
                    }
                    zone = ijk[0] + 
                           ijk[1] * (dims[0]-1) +
                           ijk[2] * (dims[0]-1) * (dims[1]-1);
                }
            }
            vtkVisItUtility::GetCellCenter(ds->GetCell(zone), coord);
            rv = true;
        }
        else 
        {
            int node = currentid;
            if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID ||
                ds->GetDataObjectType() == VTK_STRUCTURED_GRID) 
            {
                if (ds->GetCellData()->GetArray("avtGhostZones") != NULL) 
                {
                    int dims[3], ijk[3] = {0, 0, 0};
                    vtkVisItUtility::GetDimensions(ds, dims);
                    vtkVisItUtility::GetLogicalIndices(ds, false, node, ijk, 
                                                       false, false);
                    vtkIntArray *realDims = 
                        (vtkIntArray*)ds->GetFieldData()->GetArray("avtRealDims");
                    if (realDims != NULL)
                    {
                        ijk[0] += realDims->GetValue(0);
                        ijk[1] += realDims->GetValue(2);
                        ijk[2] += realDims->GetValue(4);
                    }
                    node = ijk[0] + 
                           ijk[1] * (dims[0]) +
                           ijk[2] * (dims[0]) * (dims[1]);
                }
            }
            float *pt = ds->GetPoint(node);
            coord[0] = pt[0] ;
            coord[1] = pt[1] ;
            coord[2] = pt[2] ;
            rv = true;
        }
        ds->Delete();
    }
    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::QueryGlobalIds
//
//  Purpose:
//    Given an element id, and a list of incident elements, find corresponding 
//    global ids.
//
//  Arguments:
//    dom       The domain to use in searching the database.
//    var       The variable to use in searching the database.
//    ts        The timestep to use in searching the database.
//    zonal     Whether the element is a zonal or nodal id.
//    element   The zone or node number to use in searching the database.
//    incEls    A list of nodes/zone incident to element. 
//    globalElement   A place to store the global element. 
//    globalIncEls    A place to store the global incident elements. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 15, 2004 
//
//  Modifications:
//    
// ****************************************************************************

void        
avtGenericDatabase::QueryGlobalIds(const int dom, const string &var, const int ts,
                                   const bool zonal, const int element, 
                                   const intVector &incEls, int &globalElement, 
                                    intVector &globalIncEls)
{
    vtkIntArray *globalZones = 
        (vtkIntArray*)GetGlobalZoneIds(dom, var.c_str(), ts);
    vtkIntArray *globalNodes = 
        (vtkIntArray*)GetGlobalNodeIds(dom, var.c_str(), ts);

    globalElement = -1;
    if (!globalIncEls.empty())
        globalIncEls.clear();

    if (zonal)
    {
        if (globalZones)
            globalElement = globalZones->GetValue(element);
        if (globalNodes)
            for (int i = 0; i < incEls.size(); i++)
                globalIncEls.push_back(globalNodes->GetValue(incEls[i]));
    }
    else 
    {
        if (globalNodes)
            globalElement = globalNodes->GetValue(element);
        if (globalZones)
            for (int i = 0; i < incEls.size(); i++)
                globalIncEls.push_back(globalZones->GetValue(incEls[i]));
    }
}


// ****************************************************************************
//  Method: avtGenericDatabase::LocalIdForGlobal
//
//  Purpose:
//    Given an element id, and a list of incident elements, find corresponding 
//    global ids.
//
//  Returns:    The element whose global counterpart is globalElement.
//
//  Arguments:
//    dom       The domain to use in searching the database.
//    var       The variable to use in searching the database.
//    ts        The timestep to use in searching the database.
//    zonal     Whether the element is a zonal or nodal id.
//    globalElement   A place to store the global element. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 15, 2004 
//
//  Modifications:
//    
// ****************************************************************************

int        
avtGenericDatabase::LocalIdForGlobal(const int dom, const string &var, 
    const int ts, const bool zonal, const int globalElement)
{
    int retVal = -1;

    vtkIntArray *globalIds = NULL; 
    if (zonal)
    {
        globalIds = (vtkIntArray*)GetGlobalZoneIds(dom, var.c_str(), ts);
    }
    else 
    {
        globalIds = (vtkIntArray*)GetGlobalNodeIds(dom, var.c_str(), ts);
    }

    if (globalIds)
    {
        int n = globalIds->GetNumberOfTuples();
        int *g = globalIds->GetPointer(0);
        for (int i = 0; i < n && retVal == -1; i++)
        {
            retVal = (g[i] == globalElement ? i : -1);
        }
    }
    return retVal; 
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetOriginalVariableName
//
//  Purpose:
//      Gets the original variable name from the variable name being used
//      outside the database (this translation is necessary for expressions).
//      It is not known whether this is a scalar, vector, etc.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2005
//
// ****************************************************************************

static const char *
GetOriginalVariableName(const avtDatabaseMetaData *md, const char *varname)
{
    const avtScalarMetaData *smd = md->GetScalar(varname);
    if (smd != NULL)
    {
        if (smd->originalName != smd->name && smd->originalName != "")
            return smd->originalName.c_str();
        return varname;
    }

    const avtVectorMetaData *vmd = md->GetVector(varname);
    if (vmd != NULL)
    {
        if (vmd->originalName != vmd->name && vmd->originalName != "")
            return vmd->originalName.c_str();
        return varname;
    }

    const avtTensorMetaData *tmd = md->GetTensor(varname);
    if (tmd != NULL)
    {
        if (tmd->originalName != tmd->name && tmd->originalName != "")
            return tmd->originalName.c_str();
        return varname;
    }

    const avtSymmetricTensorMetaData *stmd = md->GetSymmTensor(varname);
    if (stmd != NULL)
    {
        if (stmd->originalName != stmd->name && stmd->originalName != "")
            return stmd->originalName.c_str();
        return varname;
    }

    const avtMaterialMetaData *matmd = md->GetMaterial(varname);
    if (matmd != NULL)
    {
        if (matmd->originalName != matmd->name && matmd->originalName != "")
            return matmd->originalName.c_str();
        return varname;
    }

    const avtMeshMetaData *meshmd = md->GetMesh(varname);
    if (meshmd != NULL)
    {
        if (meshmd->originalName != meshmd->name && meshmd->originalName != "")
            return meshmd->originalName.c_str();
        return varname;
    }

    const avtCurveMetaData *cmd = md->GetCurve(varname);
    if (cmd != NULL)
    {
        if (cmd->originalName != cmd->name && cmd->originalName != "")
            return cmd->originalName.c_str();
        return varname;
    }

    return varname;
}


