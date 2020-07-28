#ifndef MFEM_FMS_HPP
#define MFEM_FMS_HPP
#include <mfem.hpp>
#include <fms.h>

/* -------------------------------------------------------------------------- */
/* FMS / MFEM conversion functions */
/* -------------------------------------------------------------------------- */

namespace mfem
{

int ConvertFmsToMfem(FmsDataCollection dc, mfem::Mesh **mesh_p);
int ConvertMfemToFms(const mfem::Mesh *mesh_p, FmsDataCollection *dc);

}

#endif
