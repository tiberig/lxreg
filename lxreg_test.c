#include<stdio.h>

#include "lxreg.h"

int main(void)
{
	int i;

	if (0 != lxreg_connect("./reg")) {
		return -1;
	}

	i = 0;
	lxreg_get_int32("./reg/bla",  &i);
	printf("read from registry bla = %d\n", i);

	i++;
	lxreg_set_int32("./reg/bla",  i);

	return 0;
}
