//
// Created by guiwen on 8/12/15.
//


/*
 * ksdist.cpp
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <deque>
#include "utilities.h"
#include "wfdb.h"
#include "cluster.h"
#include "kstwo.h"
#include "nr3.h"
#include "ksdist.h"


#define MAXBUF 2400
#define MAXP 30
#define NNDIF 0.05
#define ABS(A) ((A) < 0 ? -(A) : (A))
#define ROFFERR 1e-10
int strtim(char *buf);
void usage(char *);
#define PI 3.14159265


int kstwo(int *test_data, int *standard, int n1, int n2, int hisgram_len, double &d, double &prob,int &width) {
    int j1 = 0, j2 = 0;
    int indexMax=0;
    double d1, d2, dt, en1, en2, en, fn1 = 0.0, fn2 = 0.0;
    KSdist ks;
    double sum1[100];
    double sum2[100];
    if (n2==0) //no match
    {
        d=1;
        prob=0;
        return 0;
    }

    en1 = n1;
    en2 = n2;
    d = 0.0;
    while (j1 < hisgram_len) {
        fn1 += (double) test_data[j1] / (double) n1;
        fn2 += (double) standard[j1] / (double) n2;
        sum1[j1]=fn1;
        sum2[j1]=fn2;
        if ((dt = fabs(fn2 - fn1)) > d) {
            d = dt;
            indexMax = j1;
        }
        j1++;
    }
    en = sqrt(en1 * en2 / (en1 + en2));
    prob = ks.qks((en + 0.12 + 0.11 / en) * d);
//    int posMax,posMin;
//    double min,max;
//    double *minSums;
//    int whichMin;
//    posMax = posMin = 0;
//    if (sum1[0] > sum2[0]) {
//        min = sum2[0];
//        max = sum1[0];
//        minSums = sum2 ;
//        whichMin=2;
//    } else {
//        min = sum1[0];
//        max = sum2[0];
//        minSums = sum1;
//        whichMin=1;
//    }
//    while (posMin<100)
//    {
//        while (minSums[posMin]<= max && posMin<100) posMin++;
//        if (width< (posMin-posMax))
//                width=posMin-posMax;
//        max = minSums[posMin];
//        int tmp=posMin;
//
//        posMin=posMax;
//        posMax=tmp;
//        if (whichMin == 1) {
//            minSums = sum2 + posMin;
//            whichMin = 2;
//        } else {
//            minSums = sum1 + posMin;
//            whichMin = 1;
//        }
//
//
//
//
//    };
    return indexMax;
}

void adtwo(int *test_data, int *standard, int n1, int n2, int hisgram_len, double &d, double &prob) {
    int j1 = 0, j2 = 0;
    double d1, d2, dt, en1, en2, en, fn1 = 0.0, fn2 = 0.0;
    KSdist ks;

    en1 = n1;
    en2 = n2;
    d = 0.0;
    while (j1 < hisgram_len) {
        fn1 += (double) test_data[j1] / (double) n1;
        fn2 += (double) standard[j1] / (double) n2;

        if ((dt = fabs(fn2 - fn1) / sqrt(fn2 * (1 - fn2))) > d) d = dt;
        j1++;
    }
    en = sqrt(en1 * en2 / (en1 + en2));
    prob = ks.qks((en + 0.12 + 0.11 / en) * d);
}

void kuipertwo(int *test_data, int *standard, int n1, int n2, int hisgram_len, double &v, double &prob) {
    int j1 = 0, j2 = 0;
    double d1, d2, dt, en1, en2, en, fn1 = 0.0, fn2 = 0.0;
    KSdist ks;

    en1 = n1;
    en2 = n2;
    d1 = 0.0;
    d2 = 0.0;
    dt = 0.0;
    if (n2==0) //no match
    {
        v=1;
        prob=0;
        return ;
    }
    while (j1 < hisgram_len) {
        fn1 += (double) test_data[j1] / (double) n1;
        fn2 += (double) standard[j1] / (double) n2;

        dt = fn2 - fn1;
        if (dt > d1) d1 = dt;

        dt = fn1 - fn2;
        if (dt > d2) d2 = dt;

        j1++;
    }
    v = d1 + d2;
    en = sqrt(en1 * en2 / (en1 + en2));
    prob = ks.qks((en + 0.155 + 0.24 / en) * v);
}