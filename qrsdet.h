/*****************************************************************************
FILE:  qrsdet.h
AUTHOR:	Patrick S. Hamilton
REVISED:	4/16/2002
  ___________________________________________________________________________

qrsdet.h QRS detector parameter definitions
Copywrite (C) 2000 Patrick S. Hamilton

This file is free software; you can redistribute it and/or modify it under
the terms of the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option) any
later version.

This software is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU Library General Public License for more
details.

You should have received a copy of the GNU Library General Public License along
with this library; if not, write to the Free Software Foundation, Inc., 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA.

You may contact the author by e-mail (pat@eplimited.com) or postal mail
(Patrick Hamilton, E.P. Limited, 35 Medford St., Suite 204 Somerville,
MA 02143 USA).  For updates to this software, please visit our website
(http://www.eplimited.com).
  __________________________________________________________________________
  Revisions:
	4/16: Modified to allow simplified modification of digital filters in
   	qrsfilt().
*****************************************************************************/
#include "bdac.h"
#include <iostream>
#include <fstream>
#include <wfdb/ecgcodes.h>
#include "template.pb.h"
using namespace temp;
using namespace std;

//#define SYS_CONF "/home/healthwe2/Desktop/osea/osea-firstserver/etc/healthme.conf"
//#define DIST_CONF  "/home/healthwe2/Desktop/osea/osea-firstserver/etc/healthme/dist.txt"
#define SYS_CONF "/etc/healthme.conf"
#define DIST_CONF  "/etc/healthme/dist.txt"
#define MERRGENUM 1
//#define MS_PER_SAMPLE	(  1000/  SAMPLE_RATE)
#define MS10	( (10*SAMPLE_RATE+500)/ 1000 )
#define MS25	( (25*SAMPLE_RATE+500)/ 1000 )
#define MS30	( (30*SAMPLE_RATE+500)/ 1000 )
#define MS80	( (80*SAMPLE_RATE+500)/ 1000 )
#define MS95	( (95*SAMPLE_RATE+500)/ 1000 )
#define MS100	( (100*SAMPLE_RATE+500)/ 1000 )
#define MS125	( (125*SAMPLE_RATE+500)/ 1000 )
#define MS150	( (150*SAMPLE_RATE+500)/ 1000 )
#define MS160	( (160*SAMPLE_RATE+500)/ 1000 )
#define MS175	( (175*SAMPLE_RATE+500)/ 1000 )
#define MS195	( (195*SAMPLE_RATE+500)/ 1000 )
#define MS200	( (200*SAMPLE_RATE+500)/ 1000 )
#define MS220	( (220*SAMPLE_RATE+500)/ 1000 )
#define MS250	( (250*SAMPLE_RATE+500)/ 1000 )
#define MS300	( (300*SAMPLE_RATE+500)/ 1000 )
#define MS360	( (360*SAMPLE_RATE+500)/ 1000 )
#define MS450	( (450*SAMPLE_RATE+500)/ 1000 )
#define MS1000	SAMPLE_RATE
#define MS1500	( (1500*SAMPLE_RATE+500)/ 1000 )
#define DERIV_LENGTH	MS10
#define LPBUFFER_LGTH ((int) (2*MS25))
#define HPBUFFER_LGTH MS125

//#define PRE_BLANK	MS200//qrsdet.c
#define PRE_BLANK	MS195 //qrsdet2.c
#define WINDOW_WIDTH	MS80			// Moving window integration width.
#define	FILTER_DELAY (int) (((double) DERIV_LENGTH/2) + ((double) LPBUFFER_LGTH/2 - 1) + (((double) HPBUFFER_LGTH-1)/2) + PRE_BLANK)  // filter delays plus 200 ms blanking delay
#define DER_DELAY	WINDOW_WIDTH + FILTER_DELAY + MS100
#define _MAX_PATH 1024

#define MEANBUFFER_LGTH 5
#define SPECQRS_lGTH 51   //special qrs area from -25 to 25 based the point
class QRSFILTcls{
private:
    //meanfilt
    int meanfilt_data[MEANBUFFER_LGTH];
    int meanfilt_id;
    int meandata_count;
    //lpfilt
    long lpfilt_y1 ;
    long lpfilt_y2 ;
    int lpfilt_data[LPBUFFER_LGTH];
    int lpfilt_ptr ;
    //hpfilt
    long hpfilt_y ;
    int hpfilt_data[HPBUFFER_LGTH];
    int hpfilt_ptr ;
    //derive1
    int deriv1_derBuff[DERIV_LENGTH] ;
    int deriv1_derI ;
    //deriv2
    int deriv2_derBuff[DERIV_LENGTH] ;
    int deriv2_derI ;
    //mvwint
    long mvwint_sum ;
    int mvwint_data[WINDOW_WIDTH];
    int mvwint_ptr ;

public:
    int meanfilt( int datum ,int init) ;
    int lpfilt( int datum ,int init) ;
    int hpfilt( int datum, int init ) ;
    int deriv1( int x0, int init ) ;
    int deriv2( int x0, int init ) ;
    int mvwint(int datum, int init) ;
    void ResetFilter();
};

class QRSdetcls{
public:
    int DDBuffer[DER_DELAY];
    int DDPtr ;	/* Buffer holding derivative data. */
    int Dly ;// = 0 ;


    int det_thresh;
    int qpkcnt ;
    int qrsbuf[MAXBUFFERS], noise[MAXBUFFERS], rrbuf[MAXBUFFERS] ;//average qrs noise r2r
    int rsetBuff[MAXBUFFERS], rsetCount; //r reset
    int nmean, qmean, rrmean ;
    int count, sbpeak, sbloc, sbcount;
    int maxder, lastmax ;
    int initBlank, initMax ;
    int preBlankCnt, tempPeak ;
    int max , timeSinceMax , lastDatum ;
    int dataqrs[10];////useinqrs

public:
    QRSFILTcls qrsfilt1;
    int QRSDetFront( int datum);
    void ResetQRSdet();
    int datafilt;
    int QRSFilter(int datum,int init);
    int Peak( int datum, int init );

public:

    int QRSDet( int datum, int init );
};

#define NB_LENGTH	MS1500

class NOISEcls{
private:
    int NoiseBuffer[NB_LENGTH];
    int NBPtr;
    int NoiseEstimate ;
public:
    void ResetNOISE();
    int GetNoiseEstimate();
    int NoiseCheck(int datum, int delay, int RR, int beatBegin, int beatEnd);
};

//rythmcnk
#define RBB_LENGTH	8
#define LEARNING	0

class RYTHMCHKcls{

private:
    int RRBuffer[RBB_LENGTH], RRTypes[RBB_LENGTH], BeatCount ;
    int ClassifyState ;
    int BigeminyFlag ;

public:
    void ResetRhythmChk(void);
    int RhythmChk(int rr);
    int IsBigeminy(void);

};


//match classify
#define DM_BUFFER_LENGTH	180
#define MAXPREV	8	// Number of preceeding beats used as beat features.
class MATCHcls{

public://match
    int BeatTemplates[MAXTYPES][BEATLGTH] ;
    int BeatCounts[MAXTYPES] ;
    int BeatWidths[MAXTYPES] ;
    int BeatClassifications[MAXTYPES] ;
    int BeatBegins[MAXTYPES] ;
    int BeatEnds[MAXTYPES] ;
    int BeatsSinceLastMatch[MAXTYPES] ;
    int BeatAmps[MAXTYPES] ;
    int BeatCenters[MAXTYPES] ;
    double MIs[MAXTYPES][MAXTYPES] ;
    int TypeCount;
    int CombineInType;
    int CombineDelType;
public:
    void ResetMatch(void);
    int GetTypesCount(void);
    int GetBeatTypeCount(int type);
    int GetBeatWidth(int type);
    int GetBeatCenter(int type);
    int GetBeatClass(int type);
    void SetBeatClass(int type, int beatClass);
    int NewBeatType(int *newBeat );
    void BestMorphMatch(int *newBeat,int *matchType,double *matchIndex, double *mi2,
                        int *shiftAdj);
    void UpdateBeatType(int matchType,int *newBeat, double mi2,
                        int shiftAdj);
    int GetDominantType(void);
    void ClearLastNewType(void);
    int GetBeatBegin(int type);
    int GetBeatEnd(int type);
    int GetBeatAmp(int type);
    double DomCompare2(int *newBeat, int domType);
    double DomCompare(int newType, int domType);
    void BeatCopy(int srcBeat, int destBeat);
    int MinimumBeatVariation(int type);
    int WideBeatVariation(int type);

    //classify
private:
    int DomType ;
    int RecentRRs[MAXTYPES], RecentTypes[MAXTYPES] ;
    RYTHMCHKcls rythmcnk;
    //DomMonitor()
    int NewDom, DomRhythm ;
    int DMBeatTypes[DM_BUFFER_LENGTH], DMBeatClasses[DM_BUFFER_LENGTH] ;
    int DMBeatRhythms[DM_BUFFER_LENGTH] ;
    int DMNormCounts[MAXTYPES], DMBeatCounts[MAXTYPES], DMIrregCount;
    int brIndex;

    //Classify()
public:
    int morphType, runCount;
    int lastIsoLevel , lastRhythmClass , lastBeatWasNew ;

public:
    int Classify(int *newBeat,int rr, int noiseLevel, int *beatMatch, int *fidAdj,
                 int init);
    int DomMonitor(int morphType, int rhythmClass, int beatWidth, int rr, int reset);
    int GetNewDominantType(void);
    int GetDomRhythm(void);
    void AdjustDomData(int oldType, int newType);
    void CombineDomData(int oldType, int newType);
    int GetRunCount();
    int TempClass(int rhythmClass, int morphType,
                  int beatWidth, int domWidth, int domType,
                  int hfNoise, int noiseLevel, int blShift, double domIndex);

    //postclas
private:
    int PostClass[MAXTYPES][MAXPREV], PCInitCount;
    int PCRhythm[MAXTYPES][MAXPREV] ;
    int lastRC, lastWidth ;
    double lastMI2 ;

public:
    void ResetPostClassify();
    void PostClassify(int *recentTypes, int domType, int *recentRRs, int width, double mi2,
                      int rhythmClass);
    int CheckPostClass(int type);
    int CheckPCRhythm(int type);
};

#define ECG_BUFFER_LENGTH	1000
#define BEAT_QUE_LENGTH	10

class BDAC{
private:
    NOISEcls noise1;
public:
    QRSdetcls qrsdet1;
    MATCHcls match1;

public:
    int ECGBuffer[ECG_BUFFER_LENGTH], ECGBufferIndex;  // Circular data buffer.
    int ECGBufferfilt[ECG_BUFFER_LENGTH];
private:
    int BeatBuffer[BEATLGTH] ;                              //100
    int BeatQue[BEAT_QUE_LENGTH], BeatQueCount;  // Buffer of detection delays.
    int RRCount;
    int InitBeatFlag;

public:
    void NoiseCmp(int * noise);
    void ResetBDAC(void);
    int BeatDetectAndClassify(int ecgSample, int *beatType, int *beatMatch);
};

class TESTRECORD{
private:
    int modeltype[MAXTYPES];     //new create
    int modeltypenum[MAXTYPES];  //new create
    int modelnum;
    long lasttime;               //use in putann
    temp::Template tmp;          //use in modelwrite
    bool compareQRS(char* qrsbuf,char* specbuf);

public:
    int Recordnum;

public:
    void WRITE_THE_TMP(int beatType,BDAC bdac,std::vector< std::vector<int> >&m_clusters,std::vector<int> &m_type, std::vector<int>& Q_type, long DetectionTime,int* modeltypen,int* modeltypev,int* delnum);
    int TestRecord(const char *data_file_path);
    int ReSearchQRS(const char *data_file_path, std::vector<int> locatebegin , std::vector<int> locateend, std::vector<std::vector<int>>*  FindrunS);
    int ReSearchSpecial(const char *data_file_path, int locatebegin, int locateend, int locatepoint , std::vector<int> *locatespecial);
    int TestRecord__edf(const char *data_file_path);
    int TestRecord__edf_short(const char *data_file_path);
    int TestRecord_read_save(const char *data_file_path);
    int TestRecord_qrs(const char *data_file_path); ////useinqrs
    void WRITE_THE_TMP2(int beatType,BDAC bdac,std::vector< std::vector<int> >& m_clusters,std::vector<int>& m_type,std::vector<int>& Q_type,std::vector<int>& S_type,
                                    long DetectionTime,int* modeltypen,int* modeltypev,int* delnum);
    void WRITE_THE_TMP3(int beatType,std::vector<int>& m_type,std::vector<int>& Q_type,std::vector<int>& S_type,std::vector<int>& V_type, long DetectionTime);

private:
    void Initial();
};