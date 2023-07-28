#ifndef QT_SSH_H
#define QT_SSH_H
#include "putty.h"
#ifdef __cplusplus
extern "C" {
#endif


int qtssh_get_userpass_input(prompts_t *p, unsigned char *in, int inlen);
int qtssh_init(int *argc, char **argv, Conf *cfg);

#ifdef __cplusplus
}
#endif
#endif

