// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "SimV2Tracing.h"

static int   simv2_trace_indent = 0;
static FILE *simv2_tracefile = NULL;

FILE *
simv2_trace_file(void)
{
    return simv2_tracefile;
}

void
simv2_set_trace_file(FILE *f)
{
    if(simv2_tracefile != NULL)
        fclose(simv2_tracefile);
    simv2_tracefile = f;
}

void
simv2_write_trace_indent(void)
{
    int i;
    for(i = 0; i < simv2_trace_indent; ++i)
        fprintf(simv2_trace_file(), " ");
}

void
simv2_begin_trace_indent(void)
{
    simv2_write_trace_indent();
    simv2_trace_indent += 4;
}

void
simv2_end_trace_indent(void)
{
    simv2_trace_indent -= 4;
    simv2_write_trace_indent();
}
