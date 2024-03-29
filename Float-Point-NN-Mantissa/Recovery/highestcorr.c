#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include "highestcorr.h"

float   realweight[5] = { 1+(float)124/128, 1+(float)57/128, 1+(float)84/128, 1+(float)47/128, 1+(float)50/128 };

double correlation(const float *x, const float *y, int size) {
    double      Sy      = 0;
    double      Syy     = 0;
    double      Sx      = 0;
    double      Sxx     = 0;
    double      Sxy     = 0;

    for(int i = 0 ; i < size ; i++) {
        Sxy += (float)x[i] * y[i];
        Sx  += (float)x[i];
        Sy  += (float)y[i];
        Sxx += (float)x[i] * (float)x[i];
        Syy += (float)y[i] * (float)y[i];
    }
    return ((double)size*Sxy - Sx * Sy) / sqrt(((double)size * Sxx - Sx * Sx)*((double)size * Syy - Sy* Sy));
}

typedef struct {
    int     maxloc;
    int     maxwt;
    float   maxcorr;
    double  highest;
    //long double sum;
} cr;

void init(cr *unname, int num) {
    unname->maxcorr = num;
    unname->maxloc  = num;
    unname->maxwt   = num;
    unname->highest = num;
    //unname->sum     = 0;
}

#define     TIMINGFN    "../Trace/time.bin"
#define     TRACEFN     "../Trace/152test.bin"
#define     CORRFN      "../Trace/HIGHEST.crr"

void highestCorr() {
    double  corrsum;
    float   weight;
    float   maxcr;
    float   *corr;
    float   **tim;
    float   **mlp;
    float   *cutX;  // mlp trace
    float   *cutY;  // timing trace
    
    unsigned int    trNum       = 5000;
    unsigned int    trLen       = 24000;
    unsigned int    cutsize     = 148;
    
    unsigned int    stepsize    = 1;

    cr      local, global;
    FILE    *RFP, *WFP;

    if((RFP = fopen(TRACEFN, "rb")) == NULL)    { puts("STREAM ERR :: TRACEFN"); return; }
    mlp  = (float**)calloc(sizeof(float*), trNum);     
    for(int i = 0 ; i < trNum ; i++) {      
        mlp[i] = (float*)calloc(sizeof(float), trLen);      
        fread(mlp[i], sizeof(float), trLen, RFP);
    }
    fclose(RFP);

    if((RFP = fopen(TIMINGFN, "rb")) == NULL)    { puts("STREAM ERR :: TIMINGFN"); return; }
    tim  = (float**)calloc(sizeof(float*), 128);
    for(int i = 0 ; i < 128 ; i++) {
        tim[i] = (float*)calloc(sizeof(float), trLen);
        fread(tim[i], sizeof(float), trLen, RFP);
    }

    cutX = (float*)calloc(sizeof(float), cutsize);
    cutY = (float*)calloc(sizeof(float), cutsize);
    corr = (float*)calloc(sizeof(float), trLen);

    printf("\n%lf %lf\n\n", mlp[9][517], mlp[9][665]);  // second weight 

    for(int i = 0 ; i < cutsize ; i++)
        //cutX[i] = mlp[30][866 + i];
        cutX[i] = mlp[11][517 + i];
    
    init(&global, 0);
    for(int wt = 0 ; wt < 128 ; wt++) {
        init(&local, 0);

        corrsum = 0;
        for(int loc = 0 ; loc + stepsize < 18000 ; loc += stepsize) {
            //init(&local, 0);

            for(int i = 0 ; i < cutsize ; i++)
                cutY[i] = tim[wt][loc + i];

            corr[loc] = correlation(cutX, cutY, cutsize);
            //corrsum += fabs(corr[loc]);
            //if(corrsum > local.highest) {
            //    local.highest = corrsum;
            //    local.maxwt   = wt;
            //}
            if(fabs(corr[loc]) > local.maxcorr) {
                local.maxcorr = fabs(corr[loc]);
                local.maxloc  = loc;
                local.maxwt   = wt;
            }
        }
        if(local.maxcorr > global.maxcorr) {
            global = local;
            fflush(stdout);
            printf(" now \t wt[%d] corr[%lf] mantissa[%d]\r", global.maxwt, global.maxcorr, global.maxwt - 1);
        }

        if(wt == 0)     WFP = fopen(CORRFN, "w+b");
        else            WFP = fopen(CORRFN, "a+b");
        fwrite(corr, sizeof(float), trLen, WFP);
        fclose(WFP);
    }
    puts("");

    printf("\n :: RECOVERED MANTISSA :: \nPEAK %lf\t%lf (%d)\n", global.maxcorr, (float)global.maxwt/128, 1 + global.maxwt);
    printf("\n :: REAL Weight Mantissa ::\n");
    for(int i = 0 ; i < 5 ; i++) {
        printf("Node(%d) \t %lf (%d)\n", i, 1 + realweight[i], (int)(realweight[i] * 128));
    }
    puts("");


    free(cutX);
    free(cutY);
    free(corr);

    for(int i = 0 ; i < trNum ; i++)
        free(mlp[i]);
    free(mlp);
    for(int i = 0 ; i < 128 ; i++)
        free(tim[i]);
    free(tim);
}

int main() {
    highestCorr();
}