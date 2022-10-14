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

int main(int argc, char **argv)
{
    char const *peerPath = "/../Resources/bin/visit";
    int const m = 23; /* length of peerPath */
    int useDot = 0;
    char syscmd[8192];

    /* initialize syscmd to all null */
    int i = 0;
    while (i < sizeof(syscmd))
    {
        syscmd[i] = '\0';
        i++;
    }

    /* get length of argv[0] string */
    int n = 0;
    while (argv[0][n]) n++;

    /* walk backwards from end of argv[0] to first slash char */
    int i0 = n;
    while (argv[0][i0] != '/' && i0 >= 0) i0--;

    /* if we didn't actually find a slash char, i0 will be -1
       and we're in a funky place. Just use "." */
    if (i0 == -1)
    {
        i0 = 1;
        useDot = 1;
    }

    /* start building system command */
    if (i0 + m > sizeof(syscmd))
        return 12; /* ENOMEM */

    /* copy chars up to last slash char */
    int i1 = 0;
    while (i1 != i0)
    {
        syscmd[i1] = useDot ? '.' : argv[0][i1];
        i1++;
    }

    /* copy path to tack on after last slash char */
    int i2 = 0;
    while (i1 < i0 + m)
    {
        syscmd[i1] = peerPath[i2];
        i1++;
        i2++;
    }

    /* add any command-line arguments */
    for (i = 1; i < argc; i++)
    {
        syscmd[i1] = ' ';
        i1++;
    
        int i3 = 0;
        while (argv[i][i3])
        {
            syscmd[i1] = argv[i][i3];
            i1++;
            i3++;
        }
    }
    
    /* do what we came here for */
    return system(syscmd);
}
