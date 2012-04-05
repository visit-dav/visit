%Module pyqt_pyqtviewer 0

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class PyQtViewer {

%TypeHeaderCode
#include <pyqtviewer.h>
%End

public:

    PyQtViewer();
    PyQtViewer(SIP_PYLIST argv /DocType="list-of-str"/) /PostHook=__pyQtQAppHook__/ [(int &argc, char **argv)];
%MethodCode
        // The Python interface is a list of argument strings that is modified.

        int argc;
        char **argv;

        // Convert the list.
        if ((argv = visitcore_ArgvToC(a0, argc)) == NULL)
            sipIsErr = 1;
        else
        {
            // Create it now the arguments are right.
            static int nargc;
            nargc = argc;

            Py_BEGIN_ALLOW_THREADS
            sipCpp = new PyQtViewer(nargc, argv);
            Py_END_ALLOW_THREADS

            // Now modify the original list.
            visitcore_UpdatePyArgv(a0, argc, argv);
        }
%End

    ~PyQtViewer();

    QMainWindow* GetRenderWindow(int i);
    QList<int> GetRenderWindowIDs();
};

%ModuleHeaderCode
char ** visitcore_ArgvToC(PyObject *, int &);
void visitcore_UpdatePyArgv(PyObject *, int, char **);
%End

%ModuleCode
// Convert a Python argv list to a conventional C argc count and argv array.
char **visitcore_ArgvToC(PyObject *argvlist, int &argc)
{
    argc = PyList_GET_SIZE(argvlist);

    // Allocate space for two copies of the argument pointers, plus the
    // terminating NULL.
    char **argv = new char *[2 * (argc + 1)];

    // Convert the list.
    for (int a = 0; a < argc; ++a)
    {
        PyObject *arg_obj = PyList_GET_ITEM(argvlist, a);
        const char *arg = sipString_AsLatin1String(&arg_obj);

        if (arg)
        {
            arg = qstrdup(arg);
            Py_DECREF(arg_obj);
        }
        else
        {
            // Try not to mess up Qt's argument parsing by simply missing out
            // the argument.
            arg = "unknown";
        }

        argv[a] = argv[a + argc + 1] = const_cast<char *>(arg);
    }

    argv[argc + argc + 1] = argv[argc] = NULL;

    return argv;
}


// Remove arguments from the Python argv list that have been removed from the
// C argv array.
void visitcore_UpdatePyArgv(PyObject *argvlist, int argc, char **argv)
{
    for (int a = 0, na = 0; a < argc; ++a)
    {
        // See if it was removed.
        if (argv[na] == argv[a + argc + 1])
            ++na;
        else
            PyList_SetSlice(argvlist, na, na + 1, 0);
    }
}
%End
