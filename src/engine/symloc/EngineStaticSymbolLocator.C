/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <iostream.h>
#include <vector>

#include <DebugStream.h>

class GeneralDatabasePluginInfo;
class EngineDatabasePluginInfo;
class GeneralPlotPluginInfo;
class EnginePlotPluginInfo;
class GeneralOperatorPluginInfo;
class EngineOperatorPluginInfo;

#define DECLARE_DATABASE(X) \
   extern const char *X##VisItPluginVersion; \
   extern "C" GeneralDatabasePluginInfo *X##_GetGeneralInfo(void); \
   extern "C" EngineDatabasePluginInfo *X##_GetEngineInfo(void);

#define DECLARE_OPERATOR(X) \
   extern const char *X##VisItPluginVersion; \
   extern "C" GeneralOperatorPluginInfo *X##_GetGeneralInfo(void); \
   extern "C" EngineOperatorPluginInfo *X##_GetEngineInfo(void);

#define DECLARE_PLOT(X) \
   extern const char *X##VisItPluginVersion; \
   extern "C" GeneralPlotPluginInfo *X##_GetGeneralInfo(void); \
   extern "C" EnginePlotPluginInfo *X##_GetEngineInfo(void);

DECLARE_PLOT(Boundary)
DECLARE_PLOT(Contour)
DECLARE_PLOT(Curve)
DECLARE_PLOT(FilledBoundary)
DECLARE_PLOT(Histogram)
DECLARE_PLOT(Label)
DECLARE_PLOT(Mesh)
DECLARE_PLOT(Molecule)
DECLARE_PLOT(MultiCurve)
DECLARE_PLOT(ParallelCoordinates)
DECLARE_PLOT(Pseudocolor)
DECLARE_PLOT(Scatter)
DECLARE_PLOT(Spreadsheet)
DECLARE_PLOT(Streamline)
DECLARE_PLOT(Subset)
DECLARE_PLOT(Surface)
DECLARE_PLOT(Tensor)
DECLARE_PLOT(Truecolor)
DECLARE_PLOT(Vector)
DECLARE_PLOT(Volume)

DECLARE_OPERATOR(BoundaryOp)
DECLARE_OPERATOR(Box)
DECLARE_OPERATOR(Clip)
DECLARE_OPERATOR(Cone)
DECLARE_OPERATOR(CoordSwap)
DECLARE_OPERATOR(CreateBonds)
DECLARE_OPERATOR(Cylinder)
DECLARE_OPERATOR(DeferExpression)
DECLARE_OPERATOR(Displace)
DECLARE_OPERATOR(DualMesh)
DECLARE_OPERATOR(Elevate)
DECLARE_OPERATOR(ExternalSurface)
DECLARE_OPERATOR(IndexSelect)
DECLARE_OPERATOR(InverseGhostZone)
DECLARE_OPERATOR(Isosurface)
DECLARE_OPERATOR(Isovolume)
DECLARE_OPERATOR(Lineout)
DECLARE_OPERATOR(OnionPeel)
DECLARE_OPERATOR(PDF)
DECLARE_OPERATOR(PersistentParticles)
DECLARE_OPERATOR(Project)
DECLARE_OPERATOR(Reflect)
DECLARE_OPERATOR(Replicate)
DECLARE_OPERATOR(Resample)
DECLARE_OPERATOR(Revolve)
DECLARE_OPERATOR(Slice)
DECLARE_OPERATOR(SphereSlice)
DECLARE_OPERATOR(ThreeSlice)
DECLARE_OPERATOR(Threshold)
DECLARE_OPERATOR(Transform)
DECLARE_OPERATOR(Tube)

DECLARE_DATABASE(Silo)
DECLARE_DATABASE(VTK)

#define CHECK_PLUGIN(X) \
   { \
   std::string thisPlugin = #X; \
   std::string combined = thisPlugin + "VisItPluginVersion"; \
   if (sym == combined) \
       return (void *) &X##VisItPluginVersion; \
   combined = thisPlugin + "_GetGeneralInfo"; \
   if (sym == combined) \
       return (void *) &X##_GetGeneralInfo; \
   combined = thisPlugin + "_GetEngineInfo"; \
   if (sym == combined) \
       return (void *) &X##_GetEngineInfo; \
   }

void *
fake_dlsym(const std::string &sym)
{
    debug1 << "Asked for " << sym << endl;

    CHECK_PLUGIN(Boundary)
    CHECK_PLUGIN(Contour)
    CHECK_PLUGIN(Curve)
    CHECK_PLUGIN(FilledBoundary)
    CHECK_PLUGIN(Histogram)
    CHECK_PLUGIN(Label)
    CHECK_PLUGIN(Mesh)
    CHECK_PLUGIN(Molecule)
    CHECK_PLUGIN(MultiCurve)
    CHECK_PLUGIN(ParallelCoordinates)
    CHECK_PLUGIN(Pseudocolor)
    CHECK_PLUGIN(Scatter)
    CHECK_PLUGIN(Spreadsheet)
    CHECK_PLUGIN(Streamline)
    CHECK_PLUGIN(Subset)
    CHECK_PLUGIN(Surface)
    CHECK_PLUGIN(Tensor)
    CHECK_PLUGIN(Truecolor)
    CHECK_PLUGIN(Vector)
    CHECK_PLUGIN(Volume)

    CHECK_PLUGIN(BoundaryOp)
    CHECK_PLUGIN(Box)
    CHECK_PLUGIN(Clip)
    CHECK_PLUGIN(Cone)
    CHECK_PLUGIN(CoordSwap)
    CHECK_PLUGIN(CreateBonds)
    CHECK_PLUGIN(Cylinder)
    CHECK_PLUGIN(DeferExpression)
    CHECK_PLUGIN(Displace)
    CHECK_PLUGIN(DualMesh)
    CHECK_PLUGIN(Elevate)
    CHECK_PLUGIN(ExternalSurface)
    CHECK_PLUGIN(IndexSelect)
    CHECK_PLUGIN(InverseGhostZone)
    CHECK_PLUGIN(Isosurface)
    CHECK_PLUGIN(Isovolume)
    CHECK_PLUGIN(Lineout)
    CHECK_PLUGIN(OnionPeel)
    CHECK_PLUGIN(PDF)
    CHECK_PLUGIN(PersistentParticles)
    CHECK_PLUGIN(Project)
    CHECK_PLUGIN(Reflect)
    CHECK_PLUGIN(Replicate)
    CHECK_PLUGIN(Resample)
    CHECK_PLUGIN(Revolve)
    CHECK_PLUGIN(Slice)
    CHECK_PLUGIN(SphereSlice)
    CHECK_PLUGIN(ThreeSlice)
    CHECK_PLUGIN(Threshold)
    CHECK_PLUGIN(Transform)
    CHECK_PLUGIN(Tube)

    CHECK_PLUGIN(Silo)
    CHECK_PLUGIN(VTK)

    debug1 << "fake_dlsym can't find symbol " << sym << endl;
    return NULL;
}


#ifdef PARALLEL

  #define ADD_DATABASE_PLUGIN(X) { std::string thisPlugin = #X; \
                        std::pair<std::string, std::string> p("", "libE"+thisPlugin+"Database_par.a");  libs.push_back(p); \
                        std::pair<std::string, std::string> p2("", "libI"+thisPlugin+"Database.a");  libs.push_back(p2); \
    }

#else

  #define ADD_DATABASE_PLUGIN(X) { std::string thisPlugin = #X; \
                        std::pair<std::string, std::string> p("", "libE"+thisPlugin+"Database_ser.a");  libs.push_back(p); \
                        std::pair<std::string, std::string> p2("", "libI"+thisPlugin+"Database.a");  libs.push_back(p2); \
   }

#endif

#ifdef PARALLEL

  #define ADD_OPERATOR_PLUGIN(X) { std::string thisPlugin = #X; \
                        std::pair<std::string, std::string> p("", "libE"+thisPlugin+"Operator_par.a");  libs.push_back(p); \
                        std::pair<std::string, std::string> p2("", "libI"+thisPlugin+"Operator.a");  libs.push_back(p2); \
    }

#else

  #define ADD_OPERATOR_PLUGIN(X) { std::string thisPlugin = #X; \
                        std::pair<std::string, std::string> p("", "libE"+thisPlugin+"Operator_ser.a");  libs.push_back(p); \
                        std::pair<std::string, std::string> p2("", "libI"+thisPlugin+"Operator.a");  libs.push_back(p2); \
   }

#endif

#ifdef PARALLEL

  #define ADD_PLOT_PLUGIN(X) { std::string thisPlugin = #X; \
                        std::pair<std::string, std::string> p("", "libE"+thisPlugin+"Plot_par.a");  libs.push_back(p); \
                        std::pair<std::string, std::string> p2("", "libI"+thisPlugin+"Plot.a");  libs.push_back(p2); \
    }

#else

  #define ADD_PLOT_PLUGIN(X) { std::string thisPlugin = #X; \
                        std::pair<std::string, std::string> p("", "libE"+thisPlugin+"Plot_ser.a");  libs.push_back(p); \
                        std::pair<std::string, std::string> p2("", "libI"+thisPlugin+"Plot.a");  libs.push_back(p2); \
   }

#endif

void
StaticGetSupportedLibs(std::vector<std::pair<std::string, std::string> > &libs,
                       const std::string &pluginType)
{
    if (pluginType == "plot")
    {
        ADD_PLOT_PLUGIN(Boundary)
        ADD_PLOT_PLUGIN(Contour)
        ADD_PLOT_PLUGIN(Curve)
        ADD_PLOT_PLUGIN(FilledBoundary)
        ADD_PLOT_PLUGIN(Histogram)
        ADD_PLOT_PLUGIN(Label)
        ADD_PLOT_PLUGIN(Mesh)
        ADD_PLOT_PLUGIN(Molecule)
        ADD_PLOT_PLUGIN(MultiCurve)
        ADD_PLOT_PLUGIN(ParallelCoordinates)
        ADD_PLOT_PLUGIN(Pseudocolor)
        ADD_PLOT_PLUGIN(Scatter)
        ADD_PLOT_PLUGIN(Spreadsheet)
        ADD_PLOT_PLUGIN(Streamline)
        ADD_PLOT_PLUGIN(Subset)
        ADD_PLOT_PLUGIN(Surface)
        ADD_PLOT_PLUGIN(Tensor)
        ADD_PLOT_PLUGIN(Truecolor)
        ADD_PLOT_PLUGIN(Vector)
        ADD_PLOT_PLUGIN(Volume)
    }

    if (pluginType == "operator")
    {
        ADD_OPERATOR_PLUGIN(BoundaryOp)
        ADD_OPERATOR_PLUGIN(Box)
        ADD_OPERATOR_PLUGIN(Clip)
        ADD_OPERATOR_PLUGIN(Cone)
        ADD_OPERATOR_PLUGIN(CoordSwap)
        ADD_OPERATOR_PLUGIN(CreateBonds)
        ADD_OPERATOR_PLUGIN(Cylinder)
        ADD_OPERATOR_PLUGIN(DeferExpression)
        ADD_OPERATOR_PLUGIN(Displace)
        ADD_OPERATOR_PLUGIN(DualMesh)
        ADD_OPERATOR_PLUGIN(Elevate)
        ADD_OPERATOR_PLUGIN(ExternalSurface)
        ADD_OPERATOR_PLUGIN(IndexSelect)
        ADD_OPERATOR_PLUGIN(InverseGhostZone)
        ADD_OPERATOR_PLUGIN(Isosurface)
        ADD_OPERATOR_PLUGIN(Isovolume)
        ADD_OPERATOR_PLUGIN(Lineout)
        ADD_OPERATOR_PLUGIN(OnionPeel)
        ADD_OPERATOR_PLUGIN(PDF)
        ADD_OPERATOR_PLUGIN(PersistentParticles)
        ADD_OPERATOR_PLUGIN(Project)
        ADD_OPERATOR_PLUGIN(Reflect)
        ADD_OPERATOR_PLUGIN(Replicate)
        ADD_OPERATOR_PLUGIN(Resample)
        ADD_OPERATOR_PLUGIN(Revolve)
        ADD_OPERATOR_PLUGIN(Slice)
        ADD_OPERATOR_PLUGIN(SphereSlice)
        ADD_OPERATOR_PLUGIN(ThreeSlice)
        ADD_OPERATOR_PLUGIN(Threshold)
        ADD_OPERATOR_PLUGIN(Transform)
        ADD_OPERATOR_PLUGIN(Tube)
    }
    
    if (pluginType == "database")
    {
        ADD_DATABASE_PLUGIN(Silo)
        ADD_DATABASE_PLUGIN(VTK)
    }
}


