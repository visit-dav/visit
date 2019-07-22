// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef LINE_ATTRIBUTES_ENUM_H
#define LINE_ATTRIBUTES_ENUM_H

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

#endif
