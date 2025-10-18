#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#define N 4

void sinx_taylor(int num_elements, int terms, double* x, double* result)
{
	int pipes[N][2];
	pid_t pid;

	for(int i = 0; i<num_elements; i++){
		pipe(pipes[i]);
		pid = fork();

		if(pid ==0)
		{
			close(pipes[i][0]);

			double value = x[i];
			double numer = x[i] * x[i] * x[i];
			double denom = 6.;
			int sign = -1;

			for(int j=1; j<=terms; j++){
				value += (double)sign * numer / denom;
				numer *= x[i] * x[i];
				denom *= (2.*(double)j+2.) * (2.*(double)j +3);
				sign *= -1;
			}

			write(pipes[i][1], &value, sizeof(double));
			close(pipes[i][1]);
			exit(0);
		} else {
			close(pipes[i][1]);
		}
		
	}

	for(int i = 0; i < num_elements; i++)
	{
		read(pipes[i][0], &result[i], sizeof(double));
		close(pipes[i][0]);
	}

	for (int i =0; i < num_elements; i++)
	{
		wait(NULL);
	}
}

int main()
{
	double x[N] = {0, M_PI/6., M_PI/3., 0.134};
	double res[N];

	sinx_taylor(N, 3, x, res);
	for(int i = 0; i<N; i++){
		printf("sin(%.2f) by Taylor Series = %f\n",x[i], res[i]);
		printf("sin(%.2f) =  %f\n", x[i], sin(x[i]));
	}
	return 0;
}
	      	
