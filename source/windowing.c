#include <stdio.h>
#include <math.h>
#include <complex.h>


#define N 30
float IN[N];
float complex out[N];
void dft(float in[],float complex out[],size_t n){
    for (size_t f=0;f<n;++f){
        out[f]=0;
        for (size_t i =0;i<n;++i){
            float t=(float)i/n;
            out[f] += in[i]*cexp(2*M_PI*I*f*t);
        }
    }


}

int main(){

    for (size_t i = 0; i <N ; i++) {
        float t =(float)i/N;
        IN[i]=sinf(2*M_PI*1.12*t);

    }
        for (size_t i = 0; i <N ; i++) {
            float t = (IN[i]+1)/2;
            for (size_t j = 0; j <t*N ; ++j) {

                printf(" ");
            }
            printf("*\n");
         
        }
    printf("----------------------------------------\n");
    for (size_t i = 0; i <N ; i++) {
        float t=(float) i/(N-1);
        float v =0.5f - 0.5f * cosf(2*M_PI*t);
        IN[i] =v;

            for (size_t j = 0; j <v*N ; ++j) {

                printf(" ");
            }
            printf("*\n");
    }
    printf("----------------------------------------\n");
        for (size_t i = 0; i <N ; i++) {
            float t = (IN[i]+1)/2;
            for (size_t j = 0; j <t*N ; ++j) {

                printf(" ");
            }
            printf("*\n");
         
        }
    return 0;
 
    for (size_t i = 0; i <N ; i++) {
        float t =(float)i/N;
        IN[i]=sinf(2*M_PI*1.12*t);

    }
    dft(IN,out,N);

    float mx=0;
    for (size_t i = 0; i <N ; i++) {
        float a=cabsf(out[i]);
        if (a>mx) mx=a;

    }

    for (size_t i = 0; i <N ; i++) {
        float a=cabsf(out[i]);
        float t= a/mx;
            for (size_t j = 0; j <t*N ; ++j) {
                printf("*");
            }
            printf("\n");
    }

    //for (size_t k=0;k<3;k++){
    //    for (size_t i = 0; i <N ; i++) {
    //        float t = (IN[i]+1)/2;
    //        for (size_t j = 0; j <t*N ; ++j) {

    //            printf(" ");
    //        }
    //        printf("*\n");
    //     
    //    }
    //}
 return 0;
}
