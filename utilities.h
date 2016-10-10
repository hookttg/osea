/* 
 * File:   utilities.h
 * Author: guiwen
 *
 * Created on July 2, 2014, 10:31 AM
 */

#ifndef UTILITIES_H
#define	UTILITIES_H
#include "circular.h"
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

double SmEn(double *databuf, int ,double ,int ,double);  
double sampen(double *y, int M, double r, int n);
int transfer(double samp) ;

int transfer(double samp) ;
double entropy(double *data, int n,int dura);
double ApEn(const double* data, unsigned int m, double r, unsigned int N, double std);
double SmEn(const double* data, unsigned int m, double r, unsigned int N, double std);
void SmEnHead(const double* data, unsigned int m, double r, unsigned int N, double std,int &Cm,int &Cm1) ;
void SmEnTail(const double* data, unsigned int m, double r, unsigned int N, double std,int &Cm,int &Cm1) ;
void SmEnWindow(const double* data, unsigned int m, double r, unsigned int N, double std,int &Cm,int &Cm1) ;
void SmEnHead(CircularBuffer<double> &data, unsigned int m, double r, unsigned int N, double std,int &Cm,int &Cm1,int offset) ;
void SmEnTail(CircularBuffer<double> &data, unsigned int m, double r, unsigned int N, double std,int &Cm,int &Cm1,int offset) ;
void SmEnWindow(CircularBuffer<double> &data, unsigned int m, double r, unsigned int N, double std,int &Cm,int &Cm1, int offset=0) ;
double SmEns(const double* data, unsigned int m, double rs[], unsigned int N, double stds[],double *out, int len);

double minimum(const double& v1, const double& v2);
void nthlargest(double databuf[], double largest[], int m, int n);
void nthlargest1(double databuf[], double largest[], int m, int n);
double adjust(double values[], double sum, int m,int n);
double adjust1(double values[], double sum, int m,int n);
int DFA(double *);
double Std(double *data, int len);
double Median(double arr[], int n);
double meanLinker(double databuf[],int len);
extern const int NBUCKETS; // or whatever
extern const double INTERVAL;
extern int const ADJUST_LEN;
double lms(double *W, double *X, double N, double ref,double in,double &error, double &out);
double norm_lms(double *W, double *X, double N, double ref,double in,double &error, double &out);
double correlation(double Xs[], double Ys[], int len) ;
double correlation(CircularBuffer<double> &Xs, int Xpos,CircularBuffer<double> &Ys, int Ypos, int len) ;
void remove_extension(char* path);
char *get_file_name(const char *path);
bool dir_exists(const char* pzPath) ;
void get_latest_sub_dir(const char *path, char *latest_sub_dir, char *name);
void add_sub_dir(int fd,int wd, const char *path);
#endif	/* UTILITIES_H */

