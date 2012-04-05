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
        virtual void Write(unsigned char value) {}
        virtual void Read(unsigned char *address) {}
        virtual void Append(const unsigned char *buf, int count){}
        virtual long DirectRead(unsigned char *buf, long len){ return 0; }
        virtual long DirectWrite(const unsigned char *buf, long len) {return 0;}
        virtual int GetDescriptor(){ return -1; }
        virtual bool NeedsRead(bool blocking = false) const
        {
            //does this logic because visitModule calls NeedsRead once
            //before entering loop and expects to get false..
            #ifdef _WIN32
            Sleep(1);
            #else
            usleep(1000);
            #endif
            static int val = 0;
            if(val < 1)
            {
                val++;
                return false;
            }
            return true;
        }
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
    PlotPluginManager* cli_plotplugin;
    OperatorPluginManager* cli_operatorplugin;

    // Used to store the sil restriction in avt format.
    avtSILRestriction_p        internalSILRestriction;


    std::string host;
    TestConnection* testconn;


    virtual void
    ProcessInput()
    {
        //Todo: make this connect to define in visitModule
        static int mx = 1000;

        SyncAttributes* a = GetViewerState()->GetSyncAttributes();
        if(a->GetSyncTag() < 0)
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

    //SIL support
    virtual avtSILRestriction_p GetPlotSILRestriction();
    virtual avtSILRestriction_p GetPlotSILRestriction() const;
    virtual void SetPlotSILRestriction(avtSILRestriction_p newRestriction);
    virtual void SetPlotSILRestriction();
    virtual void Update(Subject *subj);

public:

    ViewerSubjectProxy();

    ViewerSubjectProxy(ViewerState* istate, ViewerMethods* imethods);

    virtual ~ViewerSubjectProxy();

    virtual ViewerState   *GetViewerState() const;
    virtual ViewerMethods *GetViewerMethods() const;
    virtual PlotPluginManager* GetPlotPluginManager() const;
    virtual OperatorPluginManager *GetOperatorPluginManager() const;
    virtual void Initialize(int argc,char* argv[]);

    vtkQtRenderWindow* GetRenderWindow(int i);
    QList<int> GetRenderWindowIDs();

public slots:
    void windowDeleted(QObject*);
private:
    static vtkQtRenderWindow* renderWindow(void* data);
};

#endif
