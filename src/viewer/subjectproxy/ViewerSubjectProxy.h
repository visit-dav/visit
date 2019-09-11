// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWERSUBJECTPROXY_H
#define VIEWERSUBJECTPROXY_H

#undef Status 
#undef None
#undef EnumTypes
#undef Always

#include <VisItViewer.h>

#include <viewersubjectproxy_exports.h>
#include <iostream>
#include <ViewerProxy.h>
#include <ViewerSubject.h>
#include <vtkQtRenderWindow.h>

#include <SyncAttributes.h>
#include <SocketConnection.h>
#include <SILRestrictionAttributes.h>
#include <QCloseEvent>
#include <QList>
#include <cstddef>
#include <map>

class ViewerWindow;
class ViewerSubjectProxyFactory;

// ****************************************************************************
// Class: ViewerSubjectProxy
//
// Purpose:
//   Control ViewerSubject via a ViewerProxy style interface.
//
// Notes:      
//
// Programmer: Hari Krishnan
// Creation:   Wed May 16 09:34:25 PDT 2012
//
// Modifications:
//   Brad Whitlock, Wed May 16 09:38:10 PDT 2012
//   I moved TestConnection::NeedsRead to the C file.
//
// ****************************************************************************

class VIEWER_SUBJECT_PROXY_API ViewerSubjectProxy : public QObject, public ViewerProxy
{
    Q_OBJECT

    class TestConnection : public Connection
    {
    public:
        TestConnection(){}
        virtual ~TestConnection(){}
        virtual int  Fill(){ return 0; }
        virtual void Flush(){}
        virtual long Size(){ return 0; }
        virtual void Reset() {}

        virtual void Write(unsigned char value) {}
        virtual void Read(unsigned char *address) {}
        virtual void Append(const unsigned char *buf, int count){}
        virtual long DirectRead(unsigned char *buf, long len){ return 0; }
        virtual long DirectWrite(const unsigned char *buf, long len) {return 0;}
        virtual int GetDescriptor() const { return -1; }
        virtual bool NeedsRead(bool blocking = false) const;
    };

    class NonClosableQtRenderWindow : public vtkQtRenderWindow
    {

    public:
        virtual ~NonClosableQtRenderWindow(){}
        virtual void closeEvent(QCloseEvent * event)
        {

            if(ViewerSubjectProxy::viswindows.size() > 1)
            {
                vtkQtRenderWindow::closeEvent(event);
            }
            else
            {
                std::cerr << "Not allowed to close the last Viewer window" << std::endl;
                event->ignore();
            }
        }
    };

    static std::map<int,vtkQtRenderWindow*> viswindows;
    //static std::vector<vtkQtRenderWindow*> viswindows;

    bool initialize;
    ViewerSubject* vissubject;
    ViewerState* gstate;
    ViewerMethods* gmethods;
    PlotPluginManager* plotplugin;
    OperatorPluginManager* operatorplugin;

    // Used to store the sil restriction in avt format.
    avtSILRestriction_p        internalSILRestriction;
    ViewerSubjectProxy*        m_proxy;

    std::string host;
    TestConnection* testconn;


    virtual void
    ProcessInput()
    {
        //Todo: make this connect to define in visitModule
        static int mx = 1000;

        SyncAttributes* a = GetViewerState()->GetSyncAttributes();
        if(a->GetSyncTag() < 0) //a sync tag of -1 means that the cli is waiting..
        {
            mx++;
            a->SetSyncTag(mx);
            a->Notify();
        }
    }

    //override with fake connection ..
    virtual Connection *GetReadConnection() const { return testconn; }
    virtual Connection *GetWriteConnection() const { return testconn; }
    virtual const std::string &GetLocalHostName() const { return host; }
    virtual const std::string &GetLocalUserName() const { return host; }
    virtual void AddArgument(const std::string &arg) {}
    virtual void Close(){}
    virtual void Detach(){}
    virtual void Create(int *argc = 0, char ***argv = 0){}
    virtual void Create(const char *, int *argc = 0, char ***argv = 0){}
    virtual void LoadPlugins();
    virtual void InitializePlugins(PluginManager::PluginCategory t, const char *pluginDir);

public:
    //SIL support
    virtual avtSILRestriction_p GetPlotSILRestriction();
    virtual avtSILRestriction_p GetPlotSILRestriction() const;
    virtual void SetPlotSILRestriction(avtSILRestriction_p newRestriction);
    virtual void SetPlotSILRestriction();
    virtual void Update(Subject *subj);

    ViewerSubjectProxy(int argc,char* argv[]);
    ViewerSubjectProxy(ViewerSubjectProxy* proxy);

    virtual ~ViewerSubjectProxy();

    virtual ViewerState   *GetViewerState() const;
    virtual ViewerMethods *GetViewerMethods() const;
    virtual PlotPluginManager* GetPlotPluginManager() const;
    virtual OperatorPluginManager *GetOperatorPluginManager() const;
    virtual void Initialize(int argc,char* argv[]);

    vtkQtRenderWindow* GetRenderWindow(int i);
    QList<int> GetRenderWindowIDs();

    bool eventFilter(QObject *, QEvent *);
public slots:
    void windowDeleted(QObject*);
    void viewerWindowCreated(ViewerWindow*);
private:
    static void viewerWindowCreatedCallback(ViewerWindow *, void *);
    ViewerSubjectProxyFactory *factory;
    static vtkQtRenderWindow* renderWindow(void* data);
};

#endif
