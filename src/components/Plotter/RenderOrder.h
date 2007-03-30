#ifndef RENDER_ORDER_ENUM_H
#define RENDER_ORDER_ENUM_H

typedef enum
{
    MUST_GO_FIRST = 0,
    DOES_NOT_MATTER,
    MUST_GO_LAST,
    MAX_ORDER
} RenderOrder;

// Conversion functions for RenderOrder
int RenderOrder2Int(RenderOrder order);
RenderOrder Int2RenderOrder(int order);
const char *RenderOrderName(RenderOrder order);
const char *RenderOrderName(int order);

#endif
