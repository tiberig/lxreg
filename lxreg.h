#ifndef LXREG_H
#define LXREG_H

int lxreg_connect(const char *location);
int lxreg_get_int32(char *location, int *value);
int lxreg_set_int32(char *location, int value);

#endif
