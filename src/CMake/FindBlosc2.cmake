# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#-----------------------------------------------------------------------------
# Modifications:
#   Kathleen Biagas, Wed Apr 9, 2025
#   Utilize visit_import_third_party.
#
#-----------------------------------------------------------------------------

# Uses the BLOSC2_DIR hint from the config-site .cmake file

visit_import_third_party(BLOSC2 LIBNAMES blosc2 libblosc2)

