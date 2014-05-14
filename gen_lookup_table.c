#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	double mvpd = (2.5/4096);
	printf("double lookup_table_12bit[4096] = { ");
	int i=0;
	for(i=0; i<4095; i++) //12-bit adc 2^12=4096
	{
		printf("%lf, ", (mvpd*i));
	}
	i++;
	printf("%lf };", (mvpd*i));
	return 0;
}

	
