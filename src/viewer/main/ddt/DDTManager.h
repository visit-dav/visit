// Copyright (c) 2011, Allinea
// All rights reserved.
//
// This file has been contributed to the VisIt project, which is
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef DDTMANAGER_H
#define DDTMANAGER_H

#include <ViewerBaseUI.h>
#include <string>

// Command strings to send to ddtsim
#define DDTSIM_CMD_STOP "stop"
#define DDTSIM_CMD_PLAY "run"
#define DDTSIM_CMD_STEP "step"

class DDTSession;
class ViewerWindow;

// ****************************************************************************
// Class: DDTManager
//
// Purpose:
//   This singleton class manages VisIt's interaction with DDT and provides
//   helper methods for detecting ddtsim-based simulations.
//
// Notes:
//   Currently only supports connection to only one DDT application at a time
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

class DDTManager : public ViewerBaseUI
{
    Q_OBJECT
public:
    static DDTManager* getInstance();
    static bool isDatabaseDDTSim(std::string db);
    static bool isDDTSim(ViewerWindow*);

    DDTSession* getSession();
    DDTSession* getSessionNC();
    void disconnect();

public slots:
    DDTSession* makeConnection();
    void statusChanged();

private:
    DDTManager();
    void updateWindowInfo();
    static DDTManager* instance;

    DDTSession *mSession;
};

void VIEWER_API DDTInitialize();

#endif // DDTMANAGER_H
