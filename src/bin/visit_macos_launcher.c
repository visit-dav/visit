/* Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
   Project developers.  See the top-level LICENSE file for dates and other
   details.  No copyright assignment is required to contribute to VisIt.  */

/******************************************************************************
 *
 * Purpose: Small program with minimal dependencies, compileable to a Mach-O
 * binary executable that bootstraps VisIt launch process on macOS. This is the
 * executable that double-clicking the VisIt icon on macOS starts. It simply
 * turns around and launches VisIt as one ordinarily would on Linux.
 *
 * It is essential for this to be an actual compiled, Mach-O binary executable
 * and not a shell or python script for example, so that it is recognized by
 * macOS security framework as the VisIt application and can be managed by the
 * security framework as its own, unique application with its own unique
 * permissions/settings. See https://github.com/visit-dav/visit/issues/18182.
 *
 * It is assumed that argv[0], this executable's name which possibly includes
 * absolute or relative path components, is peer to ../Resources/bin/visit,
 * the place where the shell frontendlauncher bootstrap is installed on macOS
 * that really handles launching of VisIt.
 *
 * It aims to construct the path to that script and then start it with a 
 * system call.
 * 
 * Mark C. Miller, Wed Oct 12 09:48:36 PDT 2022
 *****************************************************************************/

int system(char const *); /* declare instead of #include <stdlib.h> */

/* Implement this as macro so any error is returned from main.
   Wrap with do-while to enforce normal semicolon termination. */
#define COPYCHARS(CHARS)                     \
do                                           \
{                                            \
    for (int i = 0; CHARS[i]; i++, cmdidx++) \
    {                                        \
        if (cmdidx >= sizeof(syscmd))        \
            return 12; /* ENOMEM */          \
        syscmd[cmdidx] = CHARS[i];           \
    }                                        \
} while (0)

int main(int argc, char **argv)
{
    char const *peerPath = "/../Resources/bin/visit";
    int cmdidx;
    char syscmd[8192];

    /* initialize syscmd to all null chars */
    cmdidx = 0;
    while (cmdidx < sizeof(syscmd))
        syscmd[cmdidx++] = '\0';
    cmdidx = 0;

    /* start by building from full copy of argv[0] */
    COPYCHARS(argv[0]);

    /* walk backwards from end of argv[0] to first slash char */
    while (syscmd[cmdidx] != '/' && cmdidx >= 0) cmdidx--;

    /* if we didn't actually find a slash char, cmdidx will be -1
       and we're in a funky place. Just use "." */
    if (cmdidx == -1)
    {
        cmdidx = 0;
        COPYCHARS(".");
    }

    COPYCHARS(peerPath);

    /* add any command-line arguments */
    for (int j = 1; j < argc; j++)
    {
        COPYCHARS(" ");
        COPYCHARS(argv[j]);
    }
    
    /* do what we came here for */
    return system(syscmd);
}
