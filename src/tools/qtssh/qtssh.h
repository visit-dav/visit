#ifndef QT_SSH_H
#define QT_SSH_H
#ifdef __cplusplus
extern "C" {
#endif

#include "putty.h"

int qtssh_get_userpass_input(prompts_t *p, unsigned char *in, int inlen);
int qtssh_init(int *argc, char **argv, Config *cfg);

#ifdef __cplusplus
}
#endif
#endif