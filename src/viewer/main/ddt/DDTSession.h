// Copyright (c) 2011, Allinea
// All rights reserved.
//
// This file has been contributed to the VisIt project, which is
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef DDTSESSION_H
#define DDTSESSION_H

#include <QObject>
#include <QString>
#include <QLocalSocket>

#include <ViewerBaseUI.h>

// ****************************************************************************
// Class: DDTSession
//
// Purpose:
//   This class represents a connection to a DDT application over a socket
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//   Jonathan Byrd Fri Feb 1, 2013
//   Add setFocusOnElement
// ****************************************************************************

class DDTSession : public ViewerBaseUI
{
    Q_OBJECT
public:
    DDTSession(const QString server);
    virtual ~DDTSession();

    void disconnect();
    bool connected();
    bool disconnected();

    QString statusString();
    void setFocusOnDomain(const int domain);
    void setFocusOnElement(const int domain, const std::string& variable,
            const int elementNumber, const std::string& value);

signals:
    void statusChanged();

private slots:
    void errorHandler(QLocalSocket::LocalSocketError);
    void completeConnection();
    void readDataFromDDT();

private:
    bool mConnected;
    QLocalSocket* mSocket;
    QString mErrorText;
    QString mServer;
};

#endif // DDTSESSION_H
