/* This is a tiny C application to be compiled with clang/gcc on macOS to be used to
   exercise all the steps involved in codesigning and notarization. See remarks in
   test_notarize.py for more details */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <libgen.h>
#include <limits.h>
#include <unistd.h>

typedef const char *(*zlibVersionFunc)(void);

static void show_dialog(const char *msg)
{
    char cmd[4096];
    snprintf(cmd, sizeof(cmd),
        "osascript -e 'display dialog \"%s\" buttons {\"OK\"} default button 1'",
        msg);
    system(cmd);
}

int main(int argc, char **argv)
{
    char exe_path[PATH_MAX];
    uint32_t size = sizeof(exe_path);

    /* Determine executable path */
    if (_NSGetExecutablePath(exe_path, &size) != 0)
    {
        show_dialog("Could not determine executable path.");
        return 1;
    }

    char resolved[PATH_MAX];
    realpath(exe_path, resolved);

    /* Remove executable name */
    char *macos_dir = dirname(resolved);

    /* Construct Frameworks path */
    char lib_path[PATH_MAX];
    snprintf(lib_path, sizeof(lib_path),
        "%s/../Frameworks/libz-1.2.13.dylib",
        macos_dir);

    /* Load the dylib */
    void *handle = dlopen(lib_path, RTLD_NOW);
    if (!handle)
    {
        char msg[4096];
        snprintf(msg, sizeof(msg),
            "dlopen failed:\n%s\n\n%s",
            lib_path,
            dlerror());
        show_dialog(msg);
        return 2;
    }

    /* Lookup function */
    zlibVersionFunc func = (zlibVersionFunc)dlsym(handle, "zlibVersion");
    if (!func)
    {
        char msg[4096];
        snprintf(msg, sizeof(msg),
            "dlsym failed: %s",
            dlerror());
        show_dialog(msg);
        return 3;
    }

    const char *ver = func();

    char msg[4096];
    snprintf(msg, sizeof(msg),
        "Hello, world!\n\nzlib version: %s",
        ver);

    show_dialog(msg);

    dlclose(handle);
    return 0;
}
