#ifndef MATERIAL_HELPERS_H
#define MATERIAL_HELPERS_H
#include <VisItDataInterface_V2.h>

/* Helper functions for setting up a VisIt_MaterialData */

VisIt_MaterialData *VisIt_MaterialData_alloc(int nCells, int *arrlen);

int VisIt_MaterialData_addMaterial(VisIt_MaterialData *m, const char *matname);

void VisIt_MaterialData_addCleanCell(VisIt_MaterialData *m, int cell, int id);

void VisIt_MaterialData_addMixedCell(VisIt_MaterialData *m, int cell, int *id, float *vf, int nmats, int *arrlen);

#endif
