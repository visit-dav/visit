#ifndef DECLARE_DATA_CALLBACKS_H
#define DECLARE_DATA_CALLBACKS_H
#define NO_ARGS(A) (*)
#define CB_ARGS(A) (*cb)

#define DECLARE_DATA_CALLBACKS(DECL) \
   /* Read callbacks */ \
   DECL(ActivateTimestep,    int,          (void *)) \
   DECL(GetMetaData,         visit_handle, (void *)) \
   DECL(GetMesh,             visit_handle, (int, const char *, void *)) \
   DECL(GetMaterial,         visit_handle, (int, const char *, void *)) \
   DECL(GetSpecies,          visit_handle, (int, const char *, void *)) \
   DECL(GetVariable,         visit_handle, (int, const char *, void *)) \
   DECL(GetMixedVariable,    visit_handle, (int, const char *, void *)) \
   DECL(GetCurve,            visit_handle, (const char *, void *)) \
   DECL(GetDomainList,       visit_handle, (const char *, void *)) \
   DECL(GetDomainBoundaries, visit_handle, (const char *, void *)) \
   DECL(GetDomainNesting,    visit_handle, (const char *, void *)) \
   /* Write callbacks */ \
   DECL(WriteBegin,          int, (const char *, void *)) \
   DECL(WriteEnd,            int, (const char *, void *)) \
   DECL(WriteMesh,           int, (const char *, int, int, visit_handle, visit_handle, void *)) \
   DECL(WriteVariable,       int, (const char *, const char *, int, visit_handle, visit_handle, void *))

#endif
