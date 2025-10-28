# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#   Kathleen Biagas, Mon Mar 24, 2025
#   Utilize visit_import_third_party.
#
#   Kathleen Biagas, Thu July 10, 2025
#   Add support for version 3.3.4.
#
#*****************************************************************************

# Uses the OPENEXR_DIR hint from the config-site .cmake file

visit_import_third_party(OPENEXR
    LIBS Iex-3_3 IlmThread-3_3 Imath-3_1 OpenEXR-3_3 OpenEXRCore-3_3)

