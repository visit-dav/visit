# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Mon Dec 27, 17:52:39 MST 2010
#   Added high-level hdf5 lib to search on Windows. (hdf5_hldll).
#
#   Kathleen Biagas, Wed Oct 19 09:58:16 MST 2011
#   Remove ${VISIT_MSVC_VERSION} from lib location.
#
#   Kathleen Biagas, Thu Jan 9 18:47:21 PDT 2014
#   Add patch from John Cary for hdf5 without 'dll' suffix on name.
#
#****************************************************************************/

# Use the HDF5_DIR hint from the config-site .cmake file


OPTION(HDF5_LIBNAMES_AFFIX_DLL "Whether HDF5 library base names end with dll" ON)
IF(WIN32)
  if(HDF5_LIB_NAME)
    SET_UP_THIRD_PARTY(HDF5 LIBS ${HDF5_LIB_NAME})
  else()
    if(HDF5_LIBNAMES_AFFIX_DLL)
      SET_UP_THIRD_PARTY(HDF5 LIBS hdf5dll hdf5_hldll)
    else()
      SET_UP_THIRD_PARTY(HDF5 LIBS hdf5 hdf5_hl)
    endif()
  endif()
ELSE()
  SET_UP_THIRD_PARTY(HDF5 LIBS hdf5)
ENDIF()
