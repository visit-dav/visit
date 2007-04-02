#include <LibPipelineStubs.h>
#include <StubReferencedException.h>

#define DEFSTUB(retArg, clName, methName, methArgs, retVal)    \
retArg clName::methName  methArgs                              \
{                                                              \
    EXCEPTION1(StubReferencedException, #clName #methName);    \
    return retVal;                                             \
}

DEFSTUB(void, vtkAppendFilter, AddInput, (vtkDataSet*), /*void*/);
DEFSTUB(void*, vtkAppendFilter, New, (), 0);

DEFSTUB(void, vtkAppendPolyData,AddInput, (vtkPolyData*), /*void*/);
DEFSTUB(void*, vtkAppendPolyData, New, (), 0);

DEFSTUB(void*, vtkCleanPolyData, New, (), 0);

DEFSTUB(void*, vtkDataSetReader, GetOutput, (), 0);
DEFSTUB(void*, vtkDataSetReader, New, (), 0);

DEFSTUB(void, vtkDataSetWriter, SetInput, (vtkDataSet*), /*void*/);
DEFSTUB(void*, vtkDataSetWriter, New, (), 0);

DEFSTUB(char*, vtkDataWriter, RegisterAndGetOutputString, (), 0);

DEFSTUB(void, vtkWriter, SetInput, (vtkDataObject*), /*void*/);

DEFSTUB(void*, vtkPolyDataReader, GetOutput, (), 0);
DEFSTUB(void*, vtkPolyDataReader, New, (), 0);

DEFSTUB(void*, vtkRectilinearGridReader, GetOutput, (), 0);          
DEFSTUB(void*, vtkRectilinearGridReader, New, (), 0);          

DEFSTUB(void*, vtkStructuredGridReader, GetOutput, (), 0);          
DEFSTUB(void*, vtkStructuredGridReader, New, (), 0);          

DEFSTUB(void*, vtkStructuredPointsReader, GetOutput, (), 0);          
DEFSTUB(void*, vtkStructuredPointsReader, New, (), 0);          

DEFSTUB(void, vtkStructuredPointsWriter, SetInput, (vtkImageData*), /*void*/);
DEFSTUB(void*, vtkStructuredPointsWriter, New, (), 0);          

DEFSTUB(void*, vtkUnstructuredGridReader, GetOutput, (), 0);          
DEFSTUB(void*, vtkUnstructuredGridReader, New, (), 0);          

DEFSTUB(void, vtkCSGGrid, DiscretizeSpace, (int,int,int,double,double,double,double,double,double,double), /*void*/);          
DEFSTUB(void, vtkCSGGrid, DiscretizeSurfaces, (int,double,double,double,double,double,double,double), /*void*/);          
DEFSTUB(void, vtkCSGGrid, DiscretizeSpace, (int,double,double,double,double,double,double,double), /*void*/);          
