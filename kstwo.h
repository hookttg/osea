//
// Created by guiwen on 8/17/15.
//

#ifndef ECG_KSTWO_H
#define ECG_KSTWO_H

#define MEAN_LEN  30
#define INDEX_NUM  (1500/MEAN_LEN)
//#define INDEX_NUM  60
#define DURA  20 // interval of histogram
#define HIS_NUM (2000/DURA)
//#define HIS_NUM 100
#define INTERVALS  51
int kstwo(int *data1, int *standard, int n1, int n2, int hisgram_len, double &d, double &prob,int &index);
void adtwo(int *data1, int *standard, int n1, int n2, int hisgram_len, double &d, double &prob);
void kuipertwo(int *data1, int *standard, int n1, int n2, int hisgram_len, double &d, double &prob);
#endif //ECG_KSTWO_H
