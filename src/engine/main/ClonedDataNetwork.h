// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CLONEDDATA_NETWORK_H
#define CLONEDDATA_NETWORK_H

#include <avtDataObject.h>
#include <avtPlot.h>
#include <DataNetwork.h>
#include <engine_main_exports.h>

// ****************************************************************************
//  Class: ClonedDataNetwork
//
//  Purpose:
//    Stores a single AVT network.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 18, 2004 
//
//  Modifications:
//
// ****************************************************************************

class ENGINE_MAIN_API ClonedDataNetwork : public DataNetwork
{
public:
                          ClonedDataNetwork(DataNetwork *);
    virtual              ~ClonedDataNetwork(void);

    virtual void          SetPlot(avtPlot *_plot) { cPlot = _plot; };

    virtual void          ReleaseData(void);

    virtual avtPlot_p     GetPlot(void) { return cPlot; };

    // One time value doesn't make sense for this, indicate.
    virtual int           GetTime() {return -1;};

protected:
    avtPlot_p             cPlot;
};

#endif
