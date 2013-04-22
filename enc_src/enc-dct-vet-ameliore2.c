#include "../include/enc-dct.h"

int8_t NormalisationV(float x) 
{
	if (x > 255) {
		x = 255;
	}
	else if (x < 0 ) {
		x = 0;
	}
	return (int8_t)x;
}

float Coefficient_C(int8_t x) 
{ 
	if (x == 0) {
		return(M_SQRT1_2); 
	} else {
		return (1.);
	}
}

void ConstructionVecteurX1(float *x, int8_t M, float *X1)
{
	if (M == 1) {
		X1[0] = x[0];
	} else {
		for (int i = 0; i < M/2; i++) {
			X1[i] = x[2*i];
		}
	}
}


void ConstructionVecteurX2(float *x, int8_t M, float *X2)
{
	assert(M >= 4);
	for (int i = 0; i < M/4; i++) {
			X2[i] = x[2*i+1]+x[M-2*i-1];
	}
}

void ConstructionVecteurX3(float *x, int8_t M, float *X3)
{
	assert(M >= 4);
	for (int i = 0; i < M/4; i++) {
		if (i % 2) {
			X3[i] = x[M-2*i-1] - x[2*i+1];
		} else {
			X3[i] = x[2*i+1] - x[M-2*i-1];
		}
	}
}

void ConstructionVecteurX4(float *x, int8_t M, float *X4)
{
	for (int i = 0; i < M/2; i++) {
		X4[i] = x[2*i]; 
		X4[M-i-1] = x[2*i+1]; 
	}
}


void AfficheVecteur(float *y, int8_t M)
{
	for (int8_t i = 0; i < M; i++){
		printf("%f ",y[i]);
	}
	printf("\n \n");
}

	

/* ************************************************ */

void DCT_rec(float *x, int8_t N, float *out);
float DCTsimple(float *x, int8_t k, int8_t N);
float cos_simple(float *x, int8_t k, int8_t N);
float sin_simple(float *x, int8_t k, int8_t N);

/* ************************************************ */


void AfficheVect(float *out, float *out2, int8_t N)
{
	for (int8_t k = 0; k < N; k++){
		printf(" Simple  : %f     Vetterli : %f \n ", out[k],out2[k]);
	}
}



void sinDFT(float *x, int8_t N, float *out)
{
	if (N == 2) {
		out[0] = 0; 
		out[1] = 0; 
		return;
	}

	float outsin[N/2];
	float outdct[N/4];
	float X1[N/2];
	float X3[N/4];

	ConstructionVecteurX1(x,N,X1);
	ConstructionVecteurX3(x,N,X3);
	sinDFT(X1, N/2, outsin);
	DCT_rec(X3,N/4,outdct);

	out[0] = 0;

	for (int8_t k = 1; k < N/2; k++){
		int8_t i = (int8_t)N/4 - (int8_t)k;
		out[k] = outsin[k] + outdct[abs(i)];
		printf(" %i \n ",i);
	}

	out[N/2] = 0;

	for (int8_t k = N/2 + 1; k < N; k++){
		out[k] = -out[N-k];
	}
}

void cosDFT(float *x, int8_t N, float *out)
{
	if (N == 2) {
		out[0] = x[0] + x[1]; 
		out[1] = x[0] - x[1]; 
		return;
	}

	float outcos[N/2];
	float outdct[N/4];
	float X1[N/2];
	float X2[N/4];
	ConstructionVecteurX1(x,N,X1);
	ConstructionVecteurX2(x,N,X2);
	cosDFT(X1, N/2, outcos);
	DCT_rec(X2,N/4,outdct);

	for (int8_t k = 0; k < N/4; k++){
		out[k] = outcos[k] + outdct[k];
	}

	out[N/4] = outcos[N/4];

	for (int8_t k = N/4 + 1; k < N/2; k++){
		out[k] = outcos[k] - outdct[(N/2) - k];
	}

	out[N/2] = outcos[0] - outdct[0];

	for (int8_t k = N/2 + 1; k < N; k++){
		out[k]=out[N-k];
	}
}

void DCT_rec(float *x, int8_t N, float *out)
{
	if (N == 1) {
		out[0] = x[0]; 
		return;	

	} else if (N == 2) {
		out[0] = x[0] + x[1]; 
		out[1] = (x[0] - x[1])*M_SQRT1_2; 
		return;
	} 

	float costemp[N];	
	float sintemp[N];
	float X4[N];
	ConstructionVecteurX4(x,N,X4);
	cosDFT(X4,N,costemp);
	sinDFT(X4,N,sintemp);
	for (int8_t k=0; k < N ; k++){
		out[k] = costemp[k]*cos((2.*M_PI*k)/(4.*N)) - sin((2.*M_PI*k)/(4.*N))*sintemp[k];
	}		
}

float DCTsimple(float *x, int8_t k, int8_t N)
{
	float denom = (float) 4*N;
	float sum = 0;
	for (int32_t i = 0; i < N; i++) {
		sum = sum + x[i] * cos((2.*M_PI*(2*i+1)*k)/denom);
	}
	return sum;
 }

float cos_simple(float *x, int8_t k, int8_t N)
{
	float sum = 0;
	for (int32_t i = 0; i < N; i++) {
		sum = sum + x[i] * cos((2.*M_PI*i*k)/(N));
	}
	return sum;
 }

float sin_simple(float *x, int8_t k, int8_t N)
{
	float sum = 0;
	for (int32_t i = 0; i < N; i++) {
		sum = sum + x[i] * sin((2.*M_PI*i*k)/(N));
	}
	return sum;
 }


#if 1

/* Test de DCT-Vetterli */

int main(void)
{
	int8_t N = 16;
	float x[N];
	float out[N];
	float outsin[N];
	
	for (int8_t i = 0; i < N; i++){
		x[i] = 2*i + i*i;
	}

	DCT_rec(x,N,out);

	for (int8_t k = 0; k < N; k++) {
		float result = DCTsimple(x,k,N);
		printf("**** comp %2i : DCTsimple = %f    DCT Vet = %f\n"
		       ,k,result,out[k]);
	}
}
#endif
