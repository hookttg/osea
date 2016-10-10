/* 
 * File:   filter.h
 * Author: guiwen
 *
 * Created on November 17, 2014, 10:24 PM
 */

#ifndef FILTER_H
#define	FILTER_H

#include <string>
#include "singleton.h"
#define _MAX_PATH2 1024

class filter  {
public:
    filter();
    //filter(const filter& fwt);
    ~filter();


    // Operations
    bool init(const char *fltname);

    void close();





    char FilterName[_MAX_PATH2];

    double *tH, *tG; //analysis filters
    double *H, *G; //synth filters
    int thL, tgL, hL, gL; //filters lenghts
    int thZ, tgZ, hZ, gZ; //filter centers

private:
        //filters inits
    FILE* fp;
    double* loadFilter(int &L, int &Z) const;


};


typedef Singleton<filter> filterCoefs;
#endif




