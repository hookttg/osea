



//#include "ecg_signal.h"
//#include "qrs.h"
#include "filter.h"
#include <stdlib.h>
#include <string.h>
#include "win.h"
#include <queue> 
#include <cmath> 
#include "utilities.h"
filter::filter() : tH(0), tG(0), H(0), G(0),
thL(0), tgL(0), hL(0), gL(0), thZ(0), tgZ(0), hZ(0), gZ(0) {
    

}

filter::~filter() {
    if (tH) delete[] tH;
    if (tG) delete[] tG;
    if (H) delete[] H;
    if (G) delete[] G;


}

bool filter::init(const char *fltname) {
    fp = fopen(fltname, "rt");
    if (fp) {
        tH = loadFilter(thL, thZ);
        tG = loadFilter(tgL, tgZ);
        H = loadFilter(hL, hZ);
        G = loadFilter(gL, gZ);
        fclose(fp);
        return true;
    }
    return false;

}



double* filter::loadFilter(int& L, int& Z) const {
    fscanf(fp, "%d", &L);
    fscanf(fp, "%d", &Z);

    double *flt = new double[L];

    for (int i = 0; i < L; i++)
        fscanf(fp, "%lf", &flt[i]);

    return flt;
}

void filter::close() {
    if (tH) {
        delete[] tH;
        tH = 0;
    }
    if (tG) {
        delete[] tG;
        tG = 0;
    }
    if (H) {
        delete[] H;
        H = 0;
    }
    if (G) {
        delete[] G;
        G = 0;
    }


}








