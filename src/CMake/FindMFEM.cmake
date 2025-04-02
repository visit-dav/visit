# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#-----------------------------------------------------------------------------
# Modifications:
#   Kathleen Biagas, Tue June 25, 2024
#   Create exportable library via blt_import_library.
#
#   Kathleen Biagas, Mon Mar 31, 2025
#   Utilize visit_import_third_party.
#
#-----------------------------------------------------------------------------

# Uses the MFEM_DIR hint from the config-site .cmake file

visit_import_third_party(MFEM LIBS mfem)

