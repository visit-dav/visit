/*******************************************************************************
 *                        SimV2 interface file for SWIG.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 24 14:29:25 PDT 2011
 *
 * Notes: To regenerate the Python bindings, run swig and copy the resulting
 *        .cxx and .py files into the python directory.
 *
 *        swig -c++ -python -I../lib simV2.i
 *
 * Modifications:
 *
 ******************************************************************************/

%module simV2
%{
#define SWIG_FILE_WITH_INIT
#include <VisItControlInterface_V2.h>
#include <VisItDataInterface_V2.h>
#include <stdio.h>
#include <simV2_python_config.h>
#include <simV2_PyObject.h>
#include <simV2_custom.h>
%}

%init %{
#ifdef SIMV2_USE_NUMPY
import_array();
#endif
%}

#ifdef SWIGPYTHON
%include "python/simV2_python.i"
#endif

/* Parse these headers to generate wrappers */
%include <VisItControlInterface_V2.h>
%include <VisItInterfaceTypes_V2.h>

#if 1
/* Let's just include a subset of the functions to expose via SWIG. */
%include "simV2_data_objects.i"
#else
/* We would normally do this.
%include <VisIt_CommandMetaData.h>
%include <VisIt_CSGMesh.h>
%include <VisIt_CurveData.h>
%include <VisIt_CurveMetaData.h>
%include <VisIt_CurvilinearMesh.h>
%include <VisIt_DomainBoundaries.h>
%include <VisIt_DomainList.h>
%include <VisIt_DomainNesting.h>
%include <VisIt_ExpressionMetaData.h>
%include <VisIt_MaterialData.h>
%include <VisIt_MaterialMetaData.h>
%include <VisIt_MeshMetaData.h>
%include <VisIt_NameList.h>
%include <VisIt_PointMesh.h>
%include <VisIt_RectilinearMesh.h>
%include <VisIt_SimulationMetaData.h>
%include <VisIt_SpeciesData.h>
%include <VisIt_SpeciesMetaData.h>
%include <VisIt_UnstructuredMesh.h>
%include <VisIt_VariableData.h>
%include <VisIt_VariableMetaData.h>
*/
#endif
