#
# For now, let's create a subset of the SimV2 data object API that strips out
# the get methods from the data objects since we don't really need them.
#

cat ../lib/VisIt_CommandMetaData.h \
../lib/VisIt_CSGMesh.h \
../lib/VisIt_CurveData.h \
../lib/VisIt_CurveMetaData.h \
../lib/VisIt_CurvilinearMesh.h \
../lib/VisIt_DomainBoundaries.h \
../lib/VisIt_DomainList.h \
../lib/VisIt_DomainNesting.h \
../lib/VisIt_ExpressionMetaData.h \
../lib/VisIt_MaterialData.h \
../lib/VisIt_MaterialMetaData.h \
../lib/VisIt_MeshMetaData.h \
../lib/VisIt_NameList.h \
../lib/VisIt_PointMesh.h \
../lib/VisIt_RectilinearMesh.h \
../lib/VisIt_SimulationMetaData.h \
../lib/VisIt_SpeciesData.h \
../lib/VisIt_SpeciesMetaData.h \
../lib/VisIt_UnstructuredMesh.h \
../lib/VisIt_VariableData.h \
../lib/VisIt_VariableMetaData.h \
| grep -v "_get" > simV2_data_objects.i
