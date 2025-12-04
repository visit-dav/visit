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
//    Eric Brugger, Mon Nov 24 10:17:20 PST 2025
//    I added AddNode so that any Netnodes added after the DataNetwork was
//    cloned can be saved and then deleted in ReleaseData.
//
// ****************************************************************************

class ENGINE_MAIN_API ClonedDataNetwork : public DataNetwork
{
public:
                          ClonedDataNetwork(DataNetwork *);
    virtual              ~ClonedDataNetwork(void);

    virtual void          SetPlot(avtPlot *_plot) { cPlot = _plot; };

    virtual void          AddNode(Netnode *);

    virtual void          ReleaseData(void);

    virtual avtPlot_p     GetPlot(void) { return cPlot; };

    // One time value doesn't make sense for this, indicate.
    virtual int           GetTime() {return -1;};

protected:
    avtPlot_p             cPlot;
    std::vector<Netnode*> cNodeList;
};

#endif
