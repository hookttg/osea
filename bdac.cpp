/*****************************************************************************
FILE:  bdac.cpp
AUTHOR:	Patrick S. Hamilton
REVISED:	5/13/2002
  ___________________________________________________________________________

bdac.cpp: Beat Detection And Classification
Copywrite (C) 2001 Patrick S. Hamilton

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

You may contact the author by e-mail (pat@eplimited.edu) or postal mail
(Patrick Hamilton, E.P. Limited, 35 Medford St., Suite 204 Somerville,
MA 02143 USA).  For updates to this software, please visit our website
(http://www.eplimited.com).
  __________________________________________________________________________

bdac.cpp contains functions for handling Beat Detection And Classification.
The primary function calls a qrs detector.  When a beat is detected it waits
until a sufficient number of samples from the beat have occurred.  When the
beat is ready, BeatDetectAndClassify passes the beat and the timing
information on to the functions that actually classify the beat.

Functions in bdac.cpp require functions in the following files:
		qrsfilt.cpp
		qrsdet.cpp
		classify.cpp
		rythmchk.cpp
		noisechk.cpp
		analbeat.cpp
		match.cpp
		postclas.cpp

 __________________________________________________________________________

	Revisions:
		5/13/02:
			Encapsulated down sampling from input stream to beat template in
			the function DownSampleBeat.

			Constants related to time are derived from SAMPLE_RATE in qrsdet
         and BEAT_SAMPLE_RATE in bcac.h.

*******************************************************************************/
#include "qrsdet.h"	// For base SAMPLE_RATE

#define ECG_BUFFER_LENGTH	1000	// Should be long enough for a beat
											// plus extra space to accommodate
											// the maximum detection delay.
#define BEAT_QUE_LENGTH	10			// Length of que for beats awaiting
											// classification.  Because of
											// detection delays, Multiple beats
											// can occur before there is enough data
											// to classify the first beat in the que.

// Internal function prototypes.
void DownSampleBeat(int *beatOut, int *beatIn) ;

/******************************************************************************
	ResetBDAC() resets static variables required for beat detection and
	classification.
*******************************************************************************/

void BDAC::ResetBDAC(void)
	{
		//NOISEcls noise1;
		noise1.ResetNOISE();
		//QRSdetcls qrsdet1;
		qrsdet1.ResetQRSdet();
		//MATCHcls match1;
		match1.ResetMatch();
	int dummy = 0 ;
    //qrsdetclassone.QRSDet(0,1) ;	// Reset the qrs detector
        qrsdet1.QRSDet(0,1) ;	// Reset the qrs detector
	RRCount = 0 ;
	ECGBufferIndex = 0;
	match1.Classify(BeatBuffer,0,0,&dummy,&dummy,1) ;
	InitBeatFlag = 1 ;
    BeatQueCount = 0 ;	// Flush the beat que.

	}

/*****************************************************************************
Syntax:
	int BeatDetectAndClassify(int ecgSample, int *beatType, *beatMatch) ;
Description:
	BeatDetectAndClassify() implements a beat detector and classifier.
	ECG samples are passed into BeatDetectAndClassify() one sample at a
	time.  BeatDetectAndClassify has been designed for a sample rate of
	200 Hz.  When a beat has been detected and classified the detection
	delay is returned and the beat classification is returned through the
	pointer *beatType.  For use in debugging, the number of the template
   that the beat was matched to is returned in via *beatMatch.
Returns
	BeatDetectAndClassify() returns 0 if no new beat has been detected and
	classified.  If a beat has been classified, BeatDetectAndClassify returns
	the number of samples since the approximate location of the R-wave.
****************************************************************************/
int BDAC::BeatDetectAndClassify(int ecgSample, int *beatType, int *beatMatch)
	{
	int detectDelay=0, rr=0, i=0, j=0 ;
	int noiseEst = 0, beatBegin=0, beatEnd=0 ;
	int domType=0 ;
	int fidAdj=0 ;
	int tempBeat[BEAT_DIV_SAMPLE*BEATLGTH] ;//(SAMPLE_RATE/BEAT_SAMPLE_RATE)

        // Store new sample in the circular buffer.

        ECGBuffer[ECGBufferIndex] = ecgSample ; //qrsdet1.datafilt;////ECGdata put in buffer   //local data

	// Increment RRInterval count.

	++RRCount ;

	// Increment detection delays for any beats in the que.

	for(i = 0; i < BeatQueCount; ++i)
		++BeatQue[i] ;

	// Run the sample through the QRS detector.

	detectDelay = qrsdet1.QRSDet(ecgSample,0) ;                           //qrsdet2.c
	ECGBufferfilt[ECGBufferIndex] = qrsdet1.datafilt;
	if(++ECGBufferIndex == ECG_BUFFER_LENGTH)//ECGdata index
		ECGBufferIndex = 0 ;

	if(detectDelay != 0)
		{
		BeatQue[BeatQueCount] = detectDelay ;
		++BeatQueCount ;
		}

	// Return if no beat is ready for classification.

	if((BeatQue[0] < (BEATLGTH-FIDMARK)*BEAT_DIV_SAMPLE)//(SAMPLE_RATE/BEAT_SAMPLE_RATE)
		|| (BeatQueCount == 0))
		{
		noise1.NoiseCheck(ecgSample,0,rr, beatBegin, beatEnd) ;	// Update noise check buffer //noisechk.c
		return 0 ;
		}



        // Otherwise classify the beat at the head of the que.
	rr = RRCount - BeatQue[0] ;	// Calculate the R-to-R interval
	detectDelay = RRCount = BeatQue[0] ;

	// Estimate low frequency noise in the beat.
	// Might want to move this into classify().

	domType = match1.GetDominantType() ;                                      //match.c
	if(domType == -1)
		{
		beatBegin = MS250 ;
		beatEnd = MS300 ;
		}
	else
		{
		beatBegin = BEAT_DIV_SAMPLE*(FIDMARK-match1.GetBeatBegin(domType)) ;//(SAMPLE_RATE/BEAT_SAMPLE_RATE)
		beatEnd = BEAT_DIV_SAMPLE*(match1.GetBeatEnd(domType)-FIDMARK) ;//(SAMPLE_RATE/BEAT_SAMPLE_RATE)
		}

	noiseEst = noise1.NoiseCheck(ecgSample,detectDelay,rr,beatBegin,beatEnd) ;//noisechk.c


	// Copy the beat from the circular buffer to the beat buffer
	// and reduce the sample rate by averageing pairs of data
	// points.

	j = ECGBufferIndex - detectDelay - BEAT_DIV_SAMPLE*FIDMARK ;//(SAMPLE_RATE/BEAT_SAMPLE_RATE)
	if(j < 0) j += ECG_BUFFER_LENGTH ;

	for(i = 0; i < BEAT_DIV_SAMPLE*BEATLGTH; ++i)//(SAMPLE_RATE/BEAT_SAMPLE_RATE)
		{
		tempBeat[i] = ECGBuffer[j] ;                              //local data
		if(++j == ECG_BUFFER_LENGTH)
			j = 0 ;
		}

	DownSampleBeat(BeatBuffer,tempBeat) ;//bdac.c local

	// Update the QUE.

	for(i = 0; i < BeatQueCount-1; ++i)
		BeatQue[i] = BeatQue[i+1] ;
	--BeatQueCount ;


	// Skip the first beat.

	if(InitBeatFlag)
		{
		InitBeatFlag = 0 ;
		*beatType = 13 ;
		*beatMatch = 0 ;
		fidAdj = 0 ;
		}
	// Classify all other beats.
	else
		{
		*beatType = match1.Classify(BeatBuffer,rr,noiseEst,beatMatch,&fidAdj,0) ;//classify.c
		fidAdj *= BEAT_DIV_SAMPLE;//SAMPLE_RATE/BEAT_SAMPLE_RATE ;
      }

	// Ignore detection if the classifier decides that this
	// was the trailing edge of a PVC.

	if(*beatType == MAXTYPES)//100
		{
		RRCount += rr ;
		return(0) ;
		}

	// Limit the fiducial mark adjustment in case of problems with
	// beat onset and offset estimation.
        //fidAdj = fidAdj/2;
	if(fidAdj > MS80)
		fidAdj = MS80 ;
	else if(fidAdj < -MS80)
		fidAdj = -MS80 ;

    //查找周围值最大的点
	int n=ECGBufferIndex-detectDelay-fidAdj-20;
    if(n<0)
		n=ECG_BUFFER_LENGTH+n;
    int roundmax = ECGBufferfilt[n];
        int dn = 0;
    for(int as = 0;as<40;as++)
    {
        if(ECGBufferfilt[n]>roundmax)
        {
            roundmax = ECGBufferfilt[n];
            dn = as;
        }
        n++;
        if(n==ECG_BUFFER_LENGTH)
            n=0;
    }
	//return(detectDelay-fidAdj);
	return(detectDelay-fidAdj+20-dn) ;

	}

void DownSampleBeat(int *beatOut, int *beatIn)
	{
	int i ;

	for(i = 0; i < BEATLGTH; ++i)
		beatOut[i] = (beatIn[i<<1]+beatIn[(i<<1)+1])>>1 ;
	}
