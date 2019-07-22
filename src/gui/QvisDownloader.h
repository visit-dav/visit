// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_DOWNLOADER_H
#define QVIS_DOWNLOADER_H
#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>

class QByteArray;
class QFile;
class QNetworkReply;

// ****************************************************************************
// Class: QvisDownloader
//
// Purpose:
//   Downloads files from the VisIt web site.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  2 10:26:40 PDT 2008
//
// Modifications:
//   Brad Whitlock, Tue Nov  1 13:03:43 PDT 2011
//   I rewrote it in terms of QNetworkAccessManager.
//
// ****************************************************************************

class QvisDownloader : public QObject
{
    Q_OBJECT
public:
    QvisDownloader(QObject *parent);
    virtual ~QvisDownloader();

    bool get(const QUrl &url, QByteArray *bytes);
    bool get(const QUrl &url, const QString &localFile);

signals:
    void done(bool error);
    void downloadProgress(qint64, qint64);
private slots:
    void finished(QNetworkReply *reply);
private:
    QNetworkAccessManager *manager;
    
    QByteArray *bytes;
    QFile      *file;
};

#endif
