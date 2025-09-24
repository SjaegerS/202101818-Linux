#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	int a,b,result;
	char c;
	a = atoi(argv[1]);
	c = argv[2][0];
        b = atoi(argv[3]);

	if (c == '+') {
		result = a + b;
        	printf("%d\n", result);
	} else if (c == '-') {
        	result = a - b;
        	printf("%d\n", result);
    	}else if (c == 'x') {
		result = a * b;                                                                                                   printf("%d\n", result);
	}else if (c == '/') {
		result = a / b;                                                                                                   printf("%d\n", result);
        }

}
