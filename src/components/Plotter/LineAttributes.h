#ifndef LINE_ATTRIBUTES_ENUM_H
#define LINE_ATTRIBUTES_ENUM_H

typedef enum
{
    SOLID = 0,
    DASH,
    DOT,
    DOTDASH
} _LineStyle;

typedef enum
{
    LW_0 = 0,
    LW_1,
    LW_2,
    LW_3,
    LW_4,
    LW_5,
    LW_6,
    LW_7,
    LW_8,
    LW_9
} _LineWidth;


inline int LineWidth2Int(_LineWidth lw)
{
    switch (lw)
    {
        case LW_0 : return 1; 
        case LW_1 : return 2; 
        case LW_2 : return 3; 
        case LW_3 : return 4; 
        case LW_4 : return 5; 
        case LW_5 : return 6; 
        case LW_6 : return 7; 
        case LW_7 : return 8; 
        case LW_8 : return 9; 
        case LW_9 : return 10; 
        default   : return 1; 
    }
}

inline _LineWidth Int2LineWidth(int lw)
{
    switch (lw)
    {
        case 0  : return LW_0; 
        case 1  : return LW_1;
        case 2  : return LW_2;
        case 3  : return LW_3;
        case 4  : return LW_4;
        case 5  : return LW_5;
        case 6  : return LW_6;
        case 7  : return LW_7;
        case 8  : return LW_8;
        case 9  : return LW_9;
        default : return LW_0; 
    }
}

inline int LineStyle2Int(_LineStyle ls)
{
    switch (ls)
    {
        case SOLID    : return 0; 
        case DASH     : return 1; 
        case DOT      : return 2; 
        case DOTDASH  : return 3; 
        default       : return 4; 
    }
}

inline int LineStyle2StipplePattern(_LineStyle ls)
{
    switch (ls)
    {
        case SOLID    : return 0xFFFF; 
        case DASH     : return 0x00FF; 
        case DOT      : return 0x3333; 
        case DOTDASH  : return 0x31FE; 
        default       : return 0xFFFF; 
    }
}

inline _LineStyle StipplePattern2LineStyle(int ls)
{
    switch (ls)
    {
        case 0xFFFF : return SOLID; 
        case 0x00FF : return DASH; 
        case 0x3333 : return DOT; 
        case 0x31FE : return DOTDASH; 
        default     : return SOLID; 
    }
}

inline _LineStyle Int2LineStyle(int ls)
{
    switch (ls)
    {
        case 0  : return SOLID; 
        case 1  : return DASH; 
        case 2  : return DOT; 
        case 3  : return DOTDASH; 
        default : return SOLID; 
    }
}
#endif
