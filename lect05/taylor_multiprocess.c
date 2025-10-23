#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#define N 4
#define MAXLINE 100

void sinx_taylor(int num_elements, int terms, double* x, double* result)
{
	int fd[2*N];
	int pid;
	char message[MAXLINE], line[MAXLINE];
	int length;
	int child_id;

	for(int i = 0; i<num_elements; i++){
		pipe(fd + 2*i);

		child_id = i;
		pid = fork();
		if(pid ==0) {
			break;
		} else {
			close(fd[2*i+1]);
		}
	}

	if(pid ==0){
		close(fd[2*child_id]);
		double value = x[child_id];
		double numer = x[child_id] * x[child_id] * x[child_id];
		double denom = 6.;
		int sign = -1;

		for(int j=1; j<=terms; j++){
			value += (double)sign * numer / denom;
			numer *= x[child_id] * x[child_id];
			denom *= (2.*(double)j+2.) * (2.*(double)j +3);
			sign *= -1;
		}
		result[child_id] = value;
		sprintf(message, "%lf", result[child_id]);
		length = strlen(message)+1;
		write(fd[2*child_id+1], message, length);
		exit(child_id);
	}else {
		for(int i=0;i<num_elements; i++){
			int status;
			wait(&status);
			int child_id= status >> 8;
			read(fd[2*child_id], line, MAXLINE);
			result[child_id] = atof(line);
		}

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
	      	
