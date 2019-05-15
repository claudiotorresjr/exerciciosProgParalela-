#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define TAM 2000
#define BLOCK_SIZE 8
#define CHUNK 200
#define Unroll 8
#define NTHREADS 8

void preencheA(double *A);
void preencheB(double *B);


void preencheA(double *A)
{
	int i, j, tamB;
	#pragma omp parallel for private(j, tamB) num_threads(NTHREADS/2) //schedule(static, CHUNK)
	for (i = 0; i < TAM; ++i) 
	{
		tamB = i*TAM;
		for (j = 0; j < TAM; ++j) 
		{	
			A[tamB + j] = i/(j+1.0);
		}
	}
}

void preencheB(double *B)
{
	int i, j, tamB;
	#pragma omp parallel for private(j, tamB) num_threads(NTHREADS/2) //schedule(static, CHUNK)
	for (i = 0; i < TAM; ++i) 
	{
		tamB = i*TAM;
		for (j = 0; j < TAM; ++j) 
		{	
			B[tamB + j] = i-j;
		}
	}
}

int main(int argc, char const *argv[])
{	
	double *A, *B, *Bt, *C;

	int m, r, p;
	int i, istart, iend;
	int j, jstart, jend;
	int k, kstart, kend;

	A =  aligned_alloc(32, TAM*TAM*sizeof(double));
	B =  aligned_alloc(32, TAM*TAM*sizeof(double));
	Bt = aligned_alloc(32, TAM*TAM*sizeof(double));
	C =  aligned_alloc(32, TAM*TAM*sizeof(double));

	memset(C, 0.0, TAM*TAM*sizeof(double));
	
	#pragma omp parallel sections num_threads(2)
	{	
		#pragma omp section
		preencheA(A);
		#pragma omp section
		preencheB(B);	
	}

	omp_set_num_threads(NTHREADS);
	#pragma omp parallel 
	{
		//transposta de B
		#pragma omp for private(j) firstprivate(A, B, C) schedule(static, CHUNK)
		for (i = 0; i < TAM/BLOCK_SIZE; ++i)
		{	
			istart = i*BLOCK_SIZE; iend = istart+BLOCK_SIZE; 
			for (j = 0; j < TAM/BLOCK_SIZE; ++j) 
			{
				jstart = j*BLOCK_SIZE; jend = jstart+BLOCK_SIZE;
				for(k = istart; k < iend; k += Unroll)
				{
					for(m = jstart; m < jend; ++m)
					{
						Bt[k*TAM + k]     = B[m*TAM + k];
						Bt[(k+1)*TAM + m] = B[m*TAM + k+1];
						Bt[(k+2)*TAM + m] = B[m*TAM + k+2];
						Bt[(k+3)*TAM + m] = B[m*TAM + k+3];
						Bt[(k+4)*TAM + m] = B[m*TAM + k+4];
						Bt[(k+5)*TAM + m] = B[m*TAM + k+5];
						Bt[(k+6)*TAM + m] = B[m*TAM + k+6];
						Bt[(k+7)*TAM + m] = B[m*TAM + k+7];
					}
				}
			}
		}

		//se Unroll nao for multiplo de TAM, faz o reminder
		if(TAM%Unroll != 0)
		{
			#pragma omp for private(j) firstprivate(A, B, C) schedule(static, CHUNK)
			for (i = TAM/BLOCK_SIZE - 1; i < TAM/BLOCK_SIZE; ++i)
			{	
				istart = i*BLOCK_SIZE; iend = istart+BLOCK_SIZE; 
				for (j = TAM/BLOCK_SIZE - 1; j < TAM/BLOCK_SIZE; ++j) 
				{
					jstart = j*BLOCK_SIZE; jend = jstart+BLOCK_SIZE;
					for(k = istart+TAM/BLOCK_SIZE - 1; k < iend; ++k)
					{
						for(m = jstart; m < jend; ++m)
						{
							Bt[k*TAM + k] = B[m*TAM + k];
						}
					}
				}
			}
		}

		//multiplicacao de A pela transposta de B
		#pragma omp for private(j, k, m, r, p, istart, iend, jstart, jend, kstart, kend) firstprivate(A, B, C)
		for (i = 0; i < TAM/BLOCK_SIZE; ++i)
		{
			istart = i*BLOCK_SIZE; iend = istart+BLOCK_SIZE; 
			for (j = 0; j < TAM/BLOCK_SIZE; ++j) 
			{
				jstart = j*BLOCK_SIZE; jend = jstart+BLOCK_SIZE;
				for (k = 0; k < TAM/BLOCK_SIZE; ++k)
				{	
					kstart = k*BLOCK_SIZE; kend = kstart+BLOCK_SIZE;
					for(m = istart; m < iend; ++m)
					{
						for(r = jstart; r < jend; r += Unroll)
						{	
							for(p = kstart; p < kend; ++p)
							{
								C[m*TAM + r]   += A[m*TAM + p]*Bt[r*TAM + p];
								C[m*TAM + r+1] += A[m*TAM + p]*Bt[(r+1)*TAM + p];
								C[m*TAM + r+2] += A[m*TAM + p]*Bt[(r+2)*TAM + p];
								C[m*TAM + r+3] += A[m*TAM + p]*Bt[(r+3)*TAM + p];
								C[m*TAM + r+4] += A[m*TAM + p]*Bt[(r+4)*TAM + p];
								C[m*TAM + r+5] += A[m*TAM + p]*Bt[(r+5)*TAM + p];
								C[m*TAM + r+6] += A[m*TAM + p]*Bt[(r+6)*TAM + p];
								C[m*TAM + r+7] += A[m*TAM + p]*Bt[(r+7)*TAM + p];
							}
						}
					}
				}
			}
		}

		//se Unroll nao for multiplo de TAM, faz o reminder
		if(TAM%Unroll != 0)
		{
			#pragma omp for private(j, k, m, r, p, istart, iend, jstart, jend, kstart, kend) firstprivate(A, B, C)		
			for (i = TAM/BLOCK_SIZE - 1; i < TAM/BLOCK_SIZE; ++i)
			{
				istart = i*BLOCK_SIZE; iend = istart+BLOCK_SIZE; 
				for (j = TAM/BLOCK_SIZE - 1; j < TAM/BLOCK_SIZE; ++j) 
				{
					jstart = j*BLOCK_SIZE; jend = jstart+BLOCK_SIZE;
					for (k = TAM/BLOCK_SIZE - 1; k < TAM/BLOCK_SIZE; ++k)
					{	
						kstart = k*BLOCK_SIZE; kend = kstart+BLOCK_SIZE;
						for(m = istart; m < iend; ++m)
						{
							for(r = jstart+TAM/BLOCK_SIZE - 1; r < jend; ++r)
							{	
								for(p = kstart; p < kend; ++p)
								{
									C[m*TAM + r] += A[m*TAM + p]*Bt[r*TAM + p];
								}
							}
						}
					}
				}
			}
		}
	}


	printf("%lf\n", C[TAM*TAM-1]);

	free(A);
	free(B);
	free(Bt);
	free(C);
	return 0;
}


//-28699115.678234