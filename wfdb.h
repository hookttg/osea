/* 
 * File:   wfdb.h
 * Author: guiwen
 *
 * Created on June 27, 2014, 10:55 AM
 */
#ifndef wfdb_WFDB_H
#ifndef WFDB_H
#define	WFDB_H
#include "ecgmap.h"
#include <sstream>
/* Simple data types */
typedef int WFDB_Sample; /* units are adus */
typedef int WFDB_Time; /* units are sample intervals */
typedef long WFDB_Date; /* units are days */
typedef double WFDB_Frequency; /* units are Hz (samples/second/signal) */
typedef double WFDB_Gain; /* units are adus per physical unit */
typedef unsigned int WFDB_Group; /* signal group number */
typedef unsigned int WFDB_Signal; /* signal number */
typedef unsigned int WFDB_Annotator; /* annotator number */
/* Annotation word format */
#define CODE	0176000	/* annotation code segment of annotation word */
#define CS	10	/* number of places by which code must be shifted */
#define DATA	01777	/* data segment of annotation word */
#define MAXRR	01777	/* longest interval which can be coded in a word */

/* Pseudo-annotation codes.  Legal pseudo-annotation codes are between PAMIN
   (defined below) and CODE (defined above).  PAMIN must be greater than
   ACMAX << CS (see <ecg/ecgcodes.h>). */
#define PAMIN	((unsigned)(59 << CS))
#define SKIP	((unsigned)(59 << CS))	/* long null annotation */
#define NUM	((unsigned)(60 << CS))	/* change 'num' field */
#define SUB	((unsigned)(61 << CS))	/* subtype */
#define CHN	((unsigned)(62 << CS))	/* change 'chan' field */
#define AUX	((unsigned)(63 << CS))	/* auxiliary information */
#define AUXBUFLEN 520

/* Constants for AHA annotation files only */
#define ABLKSIZ	1024		/* AHA annotation file block length */
#define AUXLEN	6		/* length of AHA aux field */
#define EOAF	0377		/* padding for end of AHA annotation files */

// SaveAnnotation (**aux)   aux data

//class WFDB_ann { /* annotation structure */
//public:
//
//    WFDB_ann() {
//        time = 0; 
//        anntyp = '1';
//        num = 0;
//        aux = NULL;
//        rr = 0;
//        skip = 0;
//        width = 0;
//        height = 0;
//    }
#define SKIPPED  0x01
#define VLEFT 0x02
#define VRIGHT 0x04
#define SLEFT 0x08
#define SRIGHT 0x10
#define MOVED 0x1E

//#define SKIPPED  0x01
#define S_PRCT  0x02
#define S_PTCL  0x04
#define S_PRCL  0x08
#define S_PFCT  0x10
#define S_OEPTCT 0x20
#define S_PTCFNT 0x40

#define V_PTCL   0x80
#define V_PTCR   0x100
#define V_PTCF   0x200

#define C_MOVED   S_PTCL|S_PRCL|V_PTCL|V_PTCR
#define C_DELETE  V_PTCF
#define P_MOVED   S_PRCT|S_PRCL
#define P_DELETE  S_PFCT


struct WFDB_ann { /* annotation structure */
    WFDB_Time time; /* annotation time, in sample intervals from
			   the beginning of the record */
    char anntyp; /* annotation type (< ACMAX, see <wfdb/ecgcodes.h> */
//    signed char subtyp; /* annotation subtype */
//    unsigned char chan; /* channel number */
//    signed char num; /* annotator number */
    unsigned char *aux; /* pointer to auxiliary information */
    float slope;
    int rr; //* use to calculate the beat rr interval */
    short mode; //skip =1 if it is just after a noise or a drop
    short width;
    short height;
};
typedef struct WFDB_ann WFDB_Annotation;
/* write a 16-bit integer in PDP-11 format */
void wfdb_p16(unsigned int x, FILE *fp);


/* write a 32-bit integer in PDP-11 format */
void wfdb_p32(long x, FILE *fp);
/* read a 16-bit integer in PDP-11 format */
int wfdb_g16(FILE *fp);

/* read a 32-bit integer in PDP-11 format */
long wfdb_g32(FILE *fp);
extern int putann(FILE *fp, WFDB_Annotation *annot, long &last_time, int shift = 0);
int getann(FILE *fp, WFDB_Annotation *annot, long &last_time, short &last_offset);
//extern char time_string[30];
//char *fmstimstr(WFDB_Time t, WFDB_Frequency f);
char *samp2time(char *tim_string,WFDB_Time t, WFDB_Frequency f);
void wfdb_read(FILE *fd_reader, int pos, int size, short buffer[]);
static char codemap[][2] = {
    1, 1, //Normal
    2, 2,
    3, 3,
    4, 9, // atrial premature beat
    5, 5, //PVC
    8, 9, //atrial premature beat
    7, 9, ///* nodal (junctional) premature beat */
    11, 1, /* nodal (junctional) escape beat */

    //12,12,//Pace beat
    -1, 7
};
static char codemap_N_S_V[][2] = {
    1, 1, //Normal
    2, 1,
    3, 1,
    4, 9, // atrial premature beat
    5, 5, //PVC
    8, 9, //atrial premature beat
    7, 9, ///* nodal (junctional) premature beat */
    11, 1, /* nodal (junctional) escape beat */

    //12,12,//Pace beat
    -1, 7
};
static char codemap_N_V[][2] = {
    1, 1, //Normal
    2, 1,
    3, 1,
    4, 1, // atrial premature beat
    5, 2, //PVC
    8, 1, //atrial premature beat
    12, 1, //Pace beat
    -1, 7
};
static char codemap_N_S[][2] = {
    1, 1, //Normal
    2, 1,
    3, 1,
    //5, 1, //PVC
    8, 2, //atrial premature beat
    12, 1, //Pace beat
    -1, 7
};
static unsigned char *AFIB_STR = (unsigned char *) "\5(AFIB";
static unsigned char *NORM_STR = (unsigned char *) "\2(N";
#endif	/* WFDB_H */
#endif // wfdb_WFDB_H
