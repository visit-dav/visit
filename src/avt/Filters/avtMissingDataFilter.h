// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
#ifndef AVT_MISSING_DATA_FILTER_H
#define AVT_MISSING_DATA_FILTER_H

#include <filters_exports.h>

#include <avtDatabaseMetaData.h>
#include <avtDataTreeIterator.h>


// ****************************************************************************
// Class: avtMissingDataFilter
//
// Purpose:
//   This filter generates/removes missing data from its inputs.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 10 09:36:35 PST 2012
//
// Modifications:
//
//   Dave Pugmire, Thu Mar 22 13:06:30 EDT 2012
//   Added canDoCollectiveCommunication flag to detect and handle when we
//   are streaming.
//   
//   Eric Brugger, Mon Jul 21 14:40:46 PDT 2014
//   Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class AVTFILTERS_API avtMissingDataFilter : public avtDataTreeIterator
{
public:
    avtMissingDataFilter();
    virtual ~avtMissingDataFilter();

    virtual const char                 *GetType(void);
    virtual const char                 *GetDescription(void);

    void SetMetaData(const avtDatabaseMetaData *md);

    void SetGenerateMode(bool);
    void SetRemoveMode(bool);

protected:
    virtual void          PreExecute(void);
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void          PostExecute(void);

    virtual avtContract_p ModifyContract(avtContract_p);
    virtual bool          FilterUnderstandsTransformedRectMesh();

    stringVector          MissingDataVariables(avtDataRequest_p spec, 
                              const avtDatabaseMetaData *md) const;
    avtCentering          MissingDataCentering(const stringVector &vars) const;
    vtkDataArray         *MissingDataBuildMask(vtkDataSet *in_ds,
                              avtDataRequest_p spec, 
                              const avtDatabaseMetaData *md, 
                              bool &missing, avtCentering &cent) const;
    bool                  TagMissingData(vtkDataSet *in_ds, 
                              vtkDataArray *missingData, 
                              const stringVector &varsMissingData, 
                              avtCentering centering) const;

    bool                removedData;
    bool                generateMode;
    bool                removeMode;
    bool                canDoCollectiveCommunication;
    avtContract_p       contract;
    avtDatabaseMetaData metadata;
};

#endif
