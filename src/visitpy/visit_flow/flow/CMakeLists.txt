# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************/
# Modifications:
#  Cyrus Harrison, Fri Feb 16 13:41:04 PST 2024
#  Use new cmake commands for pip setup.
#
#****************************************************************************/

# deps for flow
SET(visit_flow_sources src/__init__.py
                       src/core/__init__.py
                       src/core/common.py
                       src/core/errors.py
                       src/core/filter_graph.py
                       src/core/log.py
                       src/core/property_tree.py
                       src/core/registry.py
                       src/core/state_control.py
                       src/core/workspace.py
                       src/filters/__init__.py
                       src/filters/cmd.py
                       src/filters/file_ops.py
                       src/filters/imagick.py
                       src/filters/management.py
                       src/filters/npy_ops.py
                       src/filters/pyocl_batch.py
                       src/filters/pyocl_compile.py
                       src/filters/pyocl_env.py
                       src/filters/pyocl_kernels.py
                       src/filters/pyocl_ops.py
                       src/filters/script_pipeline.py
                       src/parser/__init__.py
                       src/parser/generator.py
                       src/parser/parser.py)


PYTHON_ADD_PIP_SETUP(NAME visit_flow_py_setup
                     DEST_DIR site-packages
                     PY_MODULE_DIR visit_flow
                     PY_SETUP_FILE setup.py
                     PY_SOURCES  ${visit_flow_sources})



