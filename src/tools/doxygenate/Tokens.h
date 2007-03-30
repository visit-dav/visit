/* ************************************************************************* */
/*                                 Tokens.h                                  */
/* ************************************************************************* */

#ifndef DOX_TOKENS
#define DOX_TOKENS

typedef enum
{
    TK_COMMENT          = 257,
    TK_OTHER,          /* 258 */
    TK_NEWLINE,        /* 259 */
    TK_ARGUMENTS,      /* 260 */
    TK_AUTHOR,         /* 261 */
    TK_CREATION,       /* 263 */
    TK_PURPOSE,        /* 264 */
    TK_RETURN,         /* 265 */
    TK_WARNING,        /* 266 */
    TK_LEFT_BRACE,     /* 267 */
    TK_RIGHT_BRACE,    /* 268 */
    TK_CLASS           /* 269 */
} DoxToken;

#endif

