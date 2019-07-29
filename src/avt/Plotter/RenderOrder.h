// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef RENDER_ORDER_ENUM_H
#define RENDER_ORDER_ENUM_H

//NOTES:
//  ABSOLUTELY_LAST will be rendered AFTER
//  transparent plots.
//
typedef enum
{
    MUST_GO_FIRST = 0,
    DOES_NOT_MATTER,
    MUST_GO_LAST,
    ABSOLUTELY_LAST,
    MAX_ORDER
} RenderOrder;

// Conversion functions for RenderOrder
int RenderOrder2Int(RenderOrder order);
RenderOrder Int2RenderOrder(int order);
const char *RenderOrderName(RenderOrder order);
const char *RenderOrderName(int order);

#endif
