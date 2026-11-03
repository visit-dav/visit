# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_add_library(
    NAME     visit_fmt
    HEADERS  args.h
             base.h
             chrono.h
             color.h
             compile.h
             core.h
             format-inl.h
             format.h
             os.h
             ostream.h
             printf.h
             ranges.h
             std.h
             visit_fmt.h
             xchar.h
    INCLUDES $<BUILD_INTERFACE:${VISIT_SOURCE_DIR}/third_party_builtin/fmt-12.1.0/>
             $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/third_party_builtin>
    FOLDER   thirdparty_builtin/visit_fmt)

