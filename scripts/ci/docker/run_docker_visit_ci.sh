#!/bin/bash
# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

# exec docker run to create a container from our image
echo "docker run -t -i visit-ci-develop:current"
docker run -t -i visit-ci-develop:current /bin/bash


