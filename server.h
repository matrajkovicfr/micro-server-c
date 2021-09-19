#ifndef server_h
#define server_h

#include <stdio.h>
#include <string.h>

// Header struct: e.g. Accept-Language: en-us (name: value)
// Limit 20 header definitions per request
typedef struct {
    char *name;
    char *value;
} header_t;

static header_t requestHeaders[20] = {NULL};

void serve(const char *port);

#endif
