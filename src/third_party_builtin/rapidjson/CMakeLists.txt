# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_add_library(
    NAME    rapidjson
    HEADERS allocators.h
            cursorstreamwrapper.h
            document.h
            encodedstream.h
            encodings.h
            filereadstream.h
            filestream.h
            filewritestream.h
            fwd.h
            istreamwrapper.h
            memorybuffer.h
            memorystream.h
            ostreamwrapper.h
            pointer.h
            prettywriter.h
            rapidjson.h
            reader.h
            schema.h
            stream.h
            stringbuffer.h
            writer.h
            error/en.h
            error/error.h
            internal/biginteger.h
            internal/diyfp.h
            internal/dtoa.h
            internal/ieee754.h
            internal/itoa.h
            internal/meta.h
            internal/pow10.h
            internal/regex.h
            internal/stack.h
            internal/strfunc.h
            internal/strtod.h
            msinttypes/inttypes.h
            msinttypes/stdint.h
            internal/swap.h
    FOLDER  thirdparty_builtin/rapidjson)

