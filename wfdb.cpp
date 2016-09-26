#include <stdio.h>
#include <iostream>
#include <fstream>

#include "wfdb.h"
#include "win.h"
/* read a 16-bit integer in PDP-11 format */
int wfdb_g16(FILE *fp)
{
    int x;

    x = getc(fp);
    return ((int)((short)((getc(fp) << 8) | (x & 0xff))));
}

/* read a 32-bit integer in PDP-11 format */
long wfdb_g32(FILE *fp)
{
    long x, y;

    x = wfdb_g16(fp);
    y = wfdb_g16(fp);
    return ((x << 16) | (y & 0xffff));
}

void wfdb_p16(unsigned int x, FILE *fp) {
    (void) putc((char) x, fp);
    (void) putc((char) (x >> 8), fp);
}

/* write a 32-bit integer in PDP-11 format */
void wfdb_p32(long x, FILE *fp) {
    wfdb_p16((unsigned int) (x >> 16), fp);
    wfdb_p16((unsigned int) x, fp);
}

/* getann: read an annotation from annotator n into *annot */
// need to test and make it work.
int getann(FILE *fp, WFDB_Annotation *annot, long &last_time, short &last_offset) {
    
    int a, len;

    int index;
    last_time += last_offset& DATA; /* annotation time */
    char auxstr[10];
    if (feof(fp))
        return -1;
    annot->time = last_time;
    annot->anntyp = (last_offset & CODE) >> CS; /* set annotation type */
//    annot->subtyp = 0; /* reset subtype field */
    annot->aux = NULL; /* reset aux field */
    while (((last_offset = (unsigned) wfdb_g16(fp)) & CODE) >= PAMIN &&!feof(fp))
        switch (last_offset & CODE) { /* process pseudo-annotations */
            case SKIP: last_time += wfdb_g32(fp);
                break;
            case SUB:
//                annot->subtyp = DATA & last_offset;
                break;
            case CHN:
//                annot->chan = DATA & last_offset;
                break;
            case NUM:
//                annot->num = DATA & last_offset;
                break;
            case AUX: /* auxiliary information */
                len = last_offset & 0377; /* length of auxiliary data */

                /* save length byte */
                /* Now read the data.  Note that an extra byte may be
                   present in the annotation file to preserve word alignment;
                   if so, this extra byte is read and then overwritten by
                   the null in the second statement below. */
                (void) fread(auxstr, 1, (len + 1)&~1, fp);
                auxstr[len] = '\0'; /* add a null */
                break;
            default: break;
        }

    return (0);
}

/* putann: write annotation at annot to annotator n */
int putann2(FILE *fp, WFDB_Annotation *annot, long &last_time,int shift_offset) {
    unsigned annwd;
    unsigned char *ap;
    int i, len;
    long delta;
    long t;
    //struct oadata *oa;
    if (annot->time==0) // to avoid data to shift out
        annot->time = shift_offset+1;
    t = annot->time-shift_offset;

    delta = t - last_time;
    if (annot->anntyp == 0) {
        /* The caller intends to write a null annotation here, but putann
           must not write a word of zeroes that would be interpreted as
           an EOF.  To avoid this, putann writes a SKIP to the location
           just before the desired one;  thus annwd (below) is never 0. */
        wfdb_p16(SKIP, fp);
        wfdb_p32(delta - 1, fp);
        delta = 1;
    } else if (delta > MAXRR || delta < 0L) {
        wfdb_p16(SKIP, fp);
        wfdb_p32(delta, fp);
        delta = 0;
    }
    annwd = (int) delta + ((int) (annot->anntyp) << CS);
    wfdb_p16(annwd, fp);
//    if (annot->subtyp != 0) {
//        annwd = SUB + (DATA & annot->subtyp);
//        wfdb_p16(annwd, fp);
//    }
    //	if (annot->chan != oa->ann.chan) {
    //	    annwd = CHN + (DATA & annot->chan);
    //	    wfdb_p16(annwd, fp);
    //	}
    //	if (annot->num != oa->ann.num) {
    //	    annwd = NUM + (DATA & annot->num);
    //	    wfdb_p16(annwd, fp);
    //	}
    if (annot->aux != NULL && *annot->aux != 0) {
        annwd = AUX + (unsigned) (*annot->aux);
        wfdb_p16(annwd, fp);
        (void) fwrite(annot->aux + 1, 1, *annot->aux, fp);
        if (*annot->aux & 1)
            (void)putc('\0', fp);
    }

    //oa->ann = *annot;
    last_time = t;
    return (0);
}
using namespace std;

void wfdb_p16(unsigned int x, stringstream  &s) {
    s<<(char) (x );
    s<<(char) (x >> 8);
}

/* write a 32-bit integer in PDP-11 format */
void wfdb_p32(long x, stringstream  &s) {
    wfdb_p16((unsigned int) (x >> 16), s);
    wfdb_p16((unsigned int) x, s);


}
/* putann: write annotation at annot to annotator n */
int putann(stringstream &fp, WFDB_Annotation *annot, long &last_time,int shift_offset) {
    unsigned annwd;
    unsigned char *ap;
    int i, len;
    long delta;
    long t;
    //struct oadata *oa;
    if (annot->time==0) // to avoid data to shift out
        annot->time = shift_offset+1;
    t = annot->time-shift_offset;

    delta = t - last_time;
    if (annot->anntyp == 0) {
        /* The caller intends to write a null annotation here, but putann
           must not write a word of zeroes that would be interpreted as
           an EOF.  To avoid this, putann writes a SKIP to the location
           just before the desired one;  thus annwd (below) is never 0. */
        wfdb_p16(SKIP, fp);
        wfdb_p32(delta - 1, fp);
        delta = 1;
    } else if (delta > MAXRR || delta < 0L) {
        wfdb_p16(SKIP, fp);
        wfdb_p32(delta, fp);
        delta = 0;
    }
    annwd = (int) delta + ((int) (annot->anntyp) << CS);
    wfdb_p16(annwd, fp);
//    if (annot->subtyp != 0) {
//        annwd = SUB + (DATA & annot->subtyp);
//        wfdb_p16(annwd, fp);
//    }
    //	if (annot->chan != oa->ann.chan) {
    //	    annwd = CHN + (DATA & annot->chan);
    //	    wfdb_p16(annwd, fp);
    //	}
    //	if (annot->num != oa->ann.num) {
    //	    annwd = NUM + (DATA & annot->num);
    //	    wfdb_p16(annwd, fp);
    //	}
/*    if (annot->aux != NULL && *annot->aux != 0) {
        annwd = AUX + (unsigned) (*annot->aux);
        wfdb_p16(annwd, fp);
        int length=*annot->aux;
        for (int i=0;i<length;i++) {
            fp << (char )annot->aux[1+i];
        }
       // (void) fwrite(annot->aux + 1, 1, *annot->aux, fp);
        if (*annot->aux & 1)
            fp<<'\0';
    }
    */
    //oa->ann = *annot;
    last_time = t;
    return (0);
}




/* Convert sample number to string, using the given sampling
   frequency */
//char time_string[30];
char *samp2time(char *time_string,WFDB_Time t, WFDB_Frequency f)
{
    int hours, minutes, seconds, msec;
    WFDB_Date days;
    double tms;
    long s;

    if (t > 0L ) { /* time interval */
        if (t < 0L) t = -t;
        /* Convert from sample intervals to seconds. */
        s = (long)(t / f);
        msec = (int)((t - s*f)*1000/f + 0.5);
        if (msec == 1000) { msec = 0; s++; }
        t = s;
        seconds = t % 60;
        t /= 60;
        minutes = t % 60;
        hours = t / 60;
        if (hours > 0)
            (void)sprintf(time_string, "%2d:%02d:%02d.%03d",
                          hours, minutes, seconds, msec);
        else
            (void)sprintf(time_string, "   %2d:%02d.%03d",
                          minutes, seconds, msec);
    }
     return (time_string);
}

void wfdb_read(FILE *fd_reader, int pos, int size, short buffer[]) {

    int file_offset=pos*3;
    char* lpc = new char[size*3];
    fseek(fd_reader, file_offset, SEEK_SET);
    fread(lpc, size * 3,1,fd_reader);
    for (int i = 0; i < size; i++) {

        buffer[i] = MAKEWORD(lpc[i * 3 ], (lpc[i * 3 + 1 ] & 0x0f));
        buffer[i+1] = MAKEWORD(lpc[i * 3 + 2], (lpc[i * 3 + 1] & 0xf0) >> 4);
        if (buffer[i] & 0x800)
            buffer[i] |= ~(0xfff); //negative  data, make all the high bit(12 and after) 1
        else buffer[i] &= 0xfff;        //positive data, make all the hight bit 0
        if (buffer[i+1] & 0x800)
            buffer[i+1] |= ~(0xfff); //negative  data, make all the high bit(12 and after) 1
        else buffer[i+1] &= 0xfff;        //positive data, make all the hight bit 0
    }
    //printf("%d\t%d\n",buffer[0],buffer[1]);
    delete lpc;


}