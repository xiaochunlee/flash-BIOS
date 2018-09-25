#include <stdio.h>
#include <string.h>

#include "basefunc.h"

int main(int argc, char **argv)
{

	CommandLineOpt opt;
	memset(&opt, 0, sizeof(opt));
	int rst;

	int val = getCommandLineOpt(argc, argv, &opt);

	if (val == 0)
		printMan();
	else if (val == 1) {
		rst = (int)executeTask(&opt);
		if (rst)
			return 1;
	}

	if (strlen(errorInfo) > 0) {
		printf("errorInfo is : %s\n", errorInfo);
	}
	return 0;

}
