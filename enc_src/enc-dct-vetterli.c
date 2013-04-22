#include "../include/enc-dct.h"

int8_t NormalisationV(float x) 
{
	if (x > 255) {
		x = 255;
	} else if (x < 0 ) {
		x = 0;
	}
	return (int8_t)x;
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
		X4[i]= x[2*i]; 
		X4[M-i-1]= x[2*i+1]; 
	}
}


void AfficheVecteur(float *y, int8_t M)
{
	for (int8_t i = 0; i < M; i++){
		printf("%f ",y[i]);
	}
	printf("\n \n");
}

float DCT_rec(float *x, int8_t k, int8_t N);
float DCTsimple(float *x, int8_t k, int8_t N);
float cos_simple(float *x, int8_t k, int8_t N);
float sin_simple(float *x, int8_t k, int8_t N);

float sinDFT(float *x, int8_t k, int8_t N)
{
	static int32_t profondeur = 0;
	profondeur++;
	if (N == 1 || N == 2 || k == 0 || k == N) {
		return 0; 
	} 
	if ((k > N/2) && (k < N)){
		return -sinDFT(x,N-k,N);
	}
	assert(k <= N/2);
	float X1[N/2];
	ConstructionVecteurX1(x,N,X1);
	float comp1 = sinDFT(X1,k,N/2);
	float comp3 = sin_simple(X1,k,N/2);
	print_comp("(profondeur %u) sinDFT(X1,%i,%u) = %g,  ***    ",
		   profondeur,k,N/2,comp1);
	print_comp("sinDFT(X1,%i,%u) theorique = %g\n",k,N/2,comp3);
	if (N/4 > 0){
		float X3[N/4];
		ConstructionVecteurX3(x,N,X3);
		float comp2 = 0;
		comp2 = DCT_rec(X3,N/4-k,N/4);
		return comp1 + comp2;
	} else {
		return comp1;
	}
}

float cosDFT(float *x, int8_t k, int8_t N)
{
	static int32_t profondeur = 0;
	profondeur++;

 	if ( k == N) {
		float out = 0;
		for (int i = 0; i < N; i++) {
			out += x[i]; 
		}
		return out; 
	} else if (k == 0) {
		float out = 0;
		for (int i = 0; i < N; i++) {
			out += x[i]; 
		}
		return out; 
	} else if (N == 1) {
		return x[0];
	} else if ((k > N/2) && (k < N)){
		return cosDFT(x,N-k,N);
	}
	assert(k <= N/2);
	float X1[N/2];
	ConstructionVecteurX1(x,N,X1);
	float comp1 = cosDFT(X1,k,N/2);
	float comp3 = cos_simple(X1,k,N/2);
	print_comp("(profondeur %u) cosDFT(X1,%i,%u) = %g,  ***    ",profondeur,
		   k,N/2,comp1);
	print_comp("cosDFT(X1,%i,%u) theorique = %g\n",k,N/2,comp3);
	if (N/4 > 0){
		float X2[N/4];
		ConstructionVecteurX2(x,N,X2);
		float comp2 = 0;
		if (k < N/4) {
			comp2 = DCT_rec(X2,k,N/4);
			print_comp("DCT_rec(X2,%i,%u) = %g,  ***    ",k,N/4,
				   comp2);
			print_comp("DCT_rec(X2,%i,%u) theorique = %g\n",k,N/4,
				   DCTsimple(X2,k,N/4));
		} else if (k > N/4) {
			comp2 = -DCT_rec(X2,N/2-k,N/4);
			print_comp("DCT_rec(X2,%i,%u) = %g,  ***    ",k,N/4,
				   comp2);
			print_comp("DCT_rec(X2,%i,%u) theorique = %g\n",k,N/4,
				   -DCTsimple(X2,N/2-k,N/4));
		}
		return comp1 + comp2;
	} else {
		if (k % 2){
			return x[0]-(N-1)*x[1];
		} else {
			return x[0]+(N-1)*x[1];
		}
	}

}

float DCT_rec(float *x, int8_t k, int8_t N)
{
	static int32_t profondeur = 0;
	profondeur++;
	if (k == N) {
		return 0;
	} else if (k < 0) {
		return DCT_rec(x,-k,N);
	} else if (N == 1){
		switch (k % 4){
		case 0: 
			return x[0];
			break;
		case 1:
			return 0;
			break;
		case 2:
			return x[0];
			break;
		case 3:
			return 0;
			break;
		}
	}
	assert(k < N);
	float X4[N];
	ConstructionVecteurX4(x,N,X4);
	float comp1 = cosDFT(X4,k,N);
	float comp2 = sinDFT(X4,k,N);
	float c = cos((2.*M_PI*k)/(4*N));
	float s = sin((2.*M_PI*k)/(4*N));
	float p1 = c+s;
	float s1 = comp1 + comp2;
	float m1 = s1*c;
	float m2 = p1*comp2;
	return (m1-m2);
}

float DCTsimple(float *x, int8_t k, int8_t N)
{
	float sum = 0;
	for (int32_t i = 0; i < N; i++) {
		sum = sum + x[i] * cos((2.*M_PI*(2*i+1)*k)/(4*N));
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

extern void DCT_vetterli(uint8_t *input, int32_t *output) 
{
	float *vecteur_ligne = calloc(8,sizeof(int32_t));
	float *vecteur_colonne = calloc(8,sizeof(float));
	float *output1 = calloc(64,sizeof(float));
	for (int i = 0; i < 8; i++) {
 		for (int j = 0; j < 8; j++) {
			vecteur_ligne[j] = input[8*i+j];
		}
		for (int j = 0; j < 8; j++) {
			float result = DCTsimple(vecteur_ligne,j,8);
			output1[8*i+j] = result;
		}
	}
	for (int i = 0; i < 8; i++) { 
 		for (int j = 0; j < 8; j++) {
			vecteur_colonne[j] = output1[i+8*j] ;
		}
		for (int j = 0; j < 8; j++) {
			float result = DCTsimple(vecteur_colonne,j,8);
			output[i+8*j] = NormalisationV(result/16. +128.);
		}
	}
}


#if 0

/* Test de DCT-Vetterli */
int main(void)
{
	int8_t N = 64;

	float x[N];
	for (int8_t i = 0; i < N; i++){
		x[i] = i;
	}

	float *X1 = calloc(N/2,sizeof(float));
	float *X2 = calloc(N/4,sizeof(float));
	float *X3 = calloc(N/4,sizeof(float));
	float *X4 = calloc(N/2,sizeof(float));

	ConstructionVecteurX1(x,N,X1);
	ConstructionVecteurX2(x,N,X2);
	ConstructionVecteurX3(x,N,X3);
	ConstructionVecteurX4(x,N,X4);
	
	#if 0
	AfficheVecteur(x,N);
	AfficheVecteur(X1,N/2);
        AfficheVecteur(X2,N/4);
	AfficheVecteur(X3,N/4);
 	AfficheVecteur(X4,N/2);
	#endif

	for (int8_t k = 0; k < N; k++) {
		float result = DCT_rec(x,k,N);
		float result2 = DCTsimple(x,k,N);
		printf("**** comp %2i : DCTsimple = %f    DCT Vet = %f\n",
							k,result2,result);
	}
	printf(" \n *****************************************  \n \n ");

	for (int8_t k = 0; k < N; k++) {
		float result = DCT_rec(x,k,N);
		float result2 = DCTsimple(x,k,N);
		printf("**** comp %2i : DCTsimple[i] - DCT Vet[i] = %f\n",
					k,Valeur_absolue(result2-result));
	}
}

#endif
