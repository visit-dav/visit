/*******************************************************************************
 *                        SimV2 interface file for SWIG.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 24 14:29:25 PDT 2011
 *
 * Notes: To regenerate the Python bindings, run swig and copy the resulting
 *        .c and .py files into the python directory.
 *
 *        swig -python -I../lib simV2.i
 *
 * Modifications:
 *
 ******************************************************************************/

%module simV2
%{
#include <VisItControlInterface_V2.h>
#include <VisItDataInterface_V2.h>
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
