#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>

/*
 * ============================================================================
 * Hook Library: Intercepting strcmp via LD_PRELOAD
 * ============================================================================
 * When loaded with LD_PRELOAD, this shared object takes precedence over libc.so.
 * We resolve the original strcmp pointer using dlsym(RTLD_NEXT, "strcmp"),
 * allowing us to inspect, log, or manipulate function calls dynamically.
 */

// Function pointer signature for standard strcmp
static int (*real_strcmp)(const char *, const char *) = NULL;

int strcmp(const char *s1, const char *s2) {
    if (!real_strcmp) {
        // Find next instance of strcmp in the dynamic link search order (libc)
        real_strcmp = dlsym(RTLD_NEXT, "strcmp");
    }

    // Intercept and print function arguments to stderr
    fprintf(stderr, "\n[HOOK INTERCEPTED strcmp]\n");
    fprintf(stderr, "    Arg 1 (User Input) : \"%s\"\n", s1);
    fprintf(stderr, "    Arg 2 (Target Key) : \"%s\"\n\n", s2);

    // Call through to original libc strcmp
    return real_strcmp(s1, s2);
}
