#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
# define M_PI		3.14159265358979323846	// pi 
# define M_SQRT1_2	0.70710678118654752440	// 1/sqrt(2) 
# define M_SQRT2	1.41421356237309504880	// sqrt(2)

uint8_t NormalisationF(float x) 
{
	if (x > 255) {
		x = 255;
	}
	else if (x < 0 ) {
		x = 0;
	}
	return (uint8_t)x;
}

void Papillon(float x, float y, float *z, float *w)
{
	*z=(x+y);
	*w=(x-y);
}

void Rotation(float k, uint8_t n,uint8_t N,float x, float y, float *z, float *w)
{
	*z=k*(x*cos((n*M_PI)/(2*N)) - y*sin((n*M_PI)/(2*N)) );
	*w=k*(x*sin((n*M_PI)/(2*N)) + y*cos((n*M_PI)/(2*N)) );
}

void loeffler(float *s4, float *sortie)
{

	float *s3 = calloc(8,sizeof(float));
	float *s2 = calloc(8,sizeof(float));
	float *s1 = calloc(8,sizeof(float));
	float *s0 = calloc(8,sizeof(float));

	// Phase 4 --> 3 

	s3[0]=s4[0];
	s3[1]=s4[4];
	s3[2]=s4[2];
	s3[3]=s4[6];
	s3[5]=s4[3]*sqrt(2);
	s3[6]=s4[5]*sqrt(2);
	Papillon(s4[1],s4[7],&s3[7],&s3[4]);
	
	//Phase 3 --> 2

	Papillon(s3[0],s3[1],&s2[0],&s2[1]);
	Rotation( sqrt(2),6,8, s3[2], s3[3], &s2[2],&s2[3]);
	Papillon(s3[4],s3[6],&s2[4],&s2[6]);
	Papillon(s3[7],s3[5],&s2[7],&s2[5]);

	//Phase 2 --> 1

	Papillon(s2[0],s2[3],&s1[0],&s1[3]);
	Papillon(s2[1],s2[2],&s1[1],&s1[2]);
	
	Rotation( 1,3,8, s2[4], s2[7], &s1[4],&s1[7]);
	Rotation( 1,1,8, s2[5], s2[6], &s1[5],&s1[6]);

	//Phase 1 --> 0

	Papillon(s1[0],s1[7],&s0[0],&s0[7]);
	Papillon(s1[1],s1[6],&s0[1],&s0[6]);
	Papillon(s1[2],s1[5],&s0[2],&s0[5]);
	Papillon(s1[3],s1[4],&s0[3],&s0[4]);


	for (int j = 0; j < 8; j++) {
		sortie[j] = s0[j];		
	}
}
	

extern void IDCT(int32_t *input,uint8_t *output) 
{
	float *vecteur_ligne=calloc(8,sizeof(int32_t));
	float *vecteur_colonne=calloc(8,sizeof(float));
	float *sortie=calloc(8,sizeof(float));
	float *sortie2=calloc(8,sizeof(float));
	float *output1=calloc(64,sizeof(float));

	for (int i = 0; i < 8; i++) {

 		for (int j = 0; j < 8; j++) {
			vecteur_ligne[j] = input[8*i+j];
		}
		loeffler(vecteur_ligne,sortie);
		for (int j = 0; j < 8; j++) {
			output1[8*i+j] = sortie[j];
		}
	}


	for (int i = 0; i < 8; i++) { 

 		for (int j = 0; j < 8; j++) {
			vecteur_colonne[j] = output1[i+8*j] ;
		}
		loeffler(vecteur_colonne,sortie2);
		for (int j = 0; j < 8; j++) {
			output[i+8*j] = NormalisationF(sortie2[j]/8.+128.);
		}

	}
}


#if 0
int main(void)
{
	float a=4.;
	float b=8.;
	float *x=malloc(sizeof(*x));
	float *y=malloc(sizeof(*y));
	Papillon(a,b,x,y);
	Rotation(1,8,8, a, b, x,y);
	printf("valeur de x  : %f \n valeur de y : %f \n ", *x,*y);

	uint8_t *output;
	float *vecteur_ligne=calloc(8,sizeof(int32_t));
	float *vecteur_colonne=calloc(8,sizeof(float));
	float *sortie=calloc(8,sizeof(float));
	float *sortie2=calloc(8,sizeof(float));
	int32_t *input=calloc(64,sizeof(float));
	float *output1=calloc(64,sizeof(float));

	for (uint32_t i = 0; i < 64; i++){
		input[i] = i;
	}

	for (int i = 0; i < 8; i++) {

 		for (int j = 0; j < 8; j++) {
			vecteur_ligne[j] = input[8*i+j];
		}

		loeffler(vecteur_ligne,sortie);
		for (int j = 0; j < 8; j++) {
			output1[8*i+j] = sortie[j];
			printf("%3g    ",output1[8*i+j]);
		}
	printf("\n");
	}


	for (int i = 0; i < 8; i++) { 

 		for (int j = 0; j < 8; j++) {
			vecteur_colonne[j] = input[i+8*j] ;
			printf("%3g",vecteur_colonne[j]);
		}
		printf("\n");
		loeffler(vecteur_colonne,sortie2);
		for (int j = 0; j < 8; j++) {
			output[i+8*j] = NormalisationF(sortie2[j]*2./M_SQRT1_2+128.);
		}

	
	
	
	printf("\n");
	return 0;
}
#endif



