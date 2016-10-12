/*****************************************************************************
FILE:  qrsfilt.cpp
AUTHOR:	Patrick S. Hamilton
REVISED:	5/13/2002
  ___________________________________________________________________________

qrsfilt.cpp filter functions to aid beat detecton in electrocardiograms.
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

You may contact the author by e-mail (pat@eplimited.edu) or postal mail
(Patrick Hamilton, E.P. Limited, 35 Medford St., Suite 204 Somerville,
MA 02143 USA).  For updates to this software, please visit our website
(http://www.eplimited.com).
  __________________________________________________________________________

	This file includes QRSFilt() and associated filtering files used for QRS
	detection.  Only QRSFilt() and deriv1() are called by the QRS detector
	other functions can be hidden.
	Revisions:
		5/13: Filter implementations have been modified to allow simplified
			modification for different sample rates.
*******************************************************************************/
#include <math.h>
#include "qrsdet.h"

// Local Prototypes.
/*int lpfilt( int datum ,int init) ;
int hpfilt( int datum, int init ) ;
int deriv1( int x0, int init ) ;
int deriv2( int x0, int init ) ;
int mvwint(int datum, int init) ;
*/
/*class {
private:
    //lpfilt
    long lpfilt_y1 = 0, lpfilt_y2 = 0 ;
    int lpfilt_data[LPBUFFER_LGTH], lpfilt_ptr = 0 ;
    //hpfilt
    long hpfilt_y=0 ;
    int hpfilt_data[HPBUFFER_LGTH], hpfilt_ptr = 0 ;
    //derive1
    int deriv1_derBuff[DERIV_LENGTH], deriv1_derI = 0 ;
    //deriv2
    int deriv2_derBuff[DERIV_LENGTH], deriv2_derI = 0 ;
    //mvwint
    long mvwint_sum = 0 ;
    int mvwint_data[WINDOW_WIDTH];
    int mvwint_ptr = 0 ;

public:
    int lpfilt( int datum ,int init) ;
    int hpfilt( int datum, int init ) ;
    int deriv1( int x0, int init ) ;
    int deriv2( int x0, int init ) ;
    int mvwint(int datum, int init) ;
}QRSFILTCLAS;*/

/******************************************************************************
* Syntax:
*	int QRSFilter(int datum, int init) ;
* Description:
*	QRSFilter() takes samples of an ECG signal as input and returns a sample of
*	a signal that is an estimate of the local energy in the QRS bandwidth.  In
*	other words, the signal has a lump in it whenever a QRS complex, or QRS
*	complex like artifact occurs.  The filters were originally designed for data
*  sampled at 200 samples per second, but they work nearly as well at sample
*	frequencies from 150 to 250 samples per second.
*
*	The filter buffers and static variables are reset if a value other than
*	0 is passed to QRSFilter through init.
*******************************************************************************/
/*int QRSFilter(int datum,int init)
{
    int fdatum ;
    if(init)
    {
        qrsfilt1.hpfilt( 0, 1 ) ;		// Initialize filters.
        qrsfilt1.lpfilt( 0, 1 ) ;
        qrsfilt1.mvwint( 0, 1 ) ;
        qrsfilt1.deriv1( 0, 1 ) ;
        qrsfilt1.deriv2( 0, 1 ) ;
    }
    fdatum = qrsfilt1.lpfilt( datum, 0 ) ;		// Low pass filter data.
    fdatum = qrsfilt1.hpfilt( fdatum, 0 ) ;	// High pass filter data.
    fdatum = qrsfilt1.deriv2( fdatum, 0 ) ;	// Take the derivative.
    fdatum = fabs(fdatum) ;				// Take the absolute value.
    fdatum = qrsfilt1.mvwint( fdatum, 0 ) ;	// Average over an 80 ms window .
    return(fdatum) ;
}*/
/*int QRSFilter(int datum,int init)
	{
	int fdatum ;
	if(init)
		{
		hpfilt( 0, 1 ) ;		// Initialize filters.
		lpfilt( 0, 1 ) ;
		mvwint( 0, 1 ) ;
		deriv1( 0, 1 ) ;
		deriv2( 0, 1 ) ;
		}
	fdatum = lpfilt( datum, 0 ) ;		// Low pass filter data.
	fdatum = hpfilt( fdatum, 0 ) ;	// High pass filter data.
	fdatum = deriv2( fdatum, 0 ) ;	// Take the derivative.
	fdatum = fabs(fdatum) ;				// Take the absolute value.
	fdatum = mvwint( fdatum, 0 ) ;	// Average over an 80 ms window .
	return(fdatum) ;
	}
*/
void QRSFILTcls::ResetFilter()
{
	//lpfilt
	lpfilt_y1 = 0 ;
	lpfilt_y2 = 0 ;
	for(int i=0;i<LPBUFFER_LGTH;i++)
	{
		lpfilt_data[i] = 0 ;
	}
	lpfilt_ptr = 0 ;
	//hpfilt
	hpfilt_y = 0 ;
	for(int i=0;i<HPBUFFER_LGTH;i++)
	{
		hpfilt_data[i] = 0;
	}
	hpfilt_ptr = 0 ;
	//derive1
	for(int i=0;i<DERIV_LENGTH;i++)
	{
		deriv1_derBuff[i] = 0 ;
	}
	deriv1_derI = 0 ;
	//deriv2
	for(int i=0;i<DERIV_LENGTH;i++)
	{
		deriv2_derBuff[i] = 0 ;
	}
	deriv2_derI = 0 ;
	//mvwint
	mvwint_sum = 0 ;
	for(int i=0;i<WINDOW_WIDTH;i++)
	{
		mvwint_data[i] = 0;
	}
	mvwint_ptr = 0;
}
/*************************************************************************
*  lpfilt() implements the digital filter represented by the difference
*  equation:
*
* 	y[n] = 2*y[n-1] - y[n-2] + x[n] - 2*x[t-24 ms] + x[t-48 ms]
*
*	Note that the filter delay is (LPBUFFER_LGTH/2)-1
*
**************************************************************************/
int QRSFILTcls::lpfilt( int datum ,int init)
{
    //static long y1 = 0, y2 = 0 ;
    //static int data[LPBUFFER_LGTH], ptr = 0 ;
    long y0 ;
    int output, halfPtr ;
    if(init)
    {
        for(lpfilt_ptr = 0; lpfilt_ptr < LPBUFFER_LGTH; ++lpfilt_ptr)
            lpfilt_data[lpfilt_ptr] = 0 ;
        lpfilt_y1 = lpfilt_y2 = 0 ;
        lpfilt_ptr = 0 ;
    }
    halfPtr = lpfilt_ptr-(LPBUFFER_LGTH/2) ;	// Use halfPtr to index
    if(halfPtr < 0)							// to x[n-6].
        halfPtr += LPBUFFER_LGTH ;
    y0 = (lpfilt_y1 << 1) - lpfilt_y2 + datum - (lpfilt_data[halfPtr] << 1) + lpfilt_data[lpfilt_ptr] ;
    lpfilt_y2 = lpfilt_y1;
    lpfilt_y1 = y0;
    output = y0 / ((LPBUFFER_LGTH*LPBUFFER_LGTH)/4);
    lpfilt_data[lpfilt_ptr] = datum ;			// Stick most recent sample into
    if(++lpfilt_ptr == LPBUFFER_LGTH)	// the circular buffer and update
        lpfilt_ptr = 0 ;					// the buffer pointer.
    return(output) ;
}

/******************************************************************************
*  hpfilt() implements the high pass filter represented by the following
*  difference equation:
*
*	y[n] = y[n-1] + x[n] - x[n-128 ms]
*	z[n] = x[n-64 ms] - y[n] ;
*
*  Filter delay is (HPBUFFER_LGTH-1)/2
******************************************************************************/
int QRSFILTcls::hpfilt( int datum, int init )
	{
	//static long y=0 ;
	//static int data[HPBUFFER_LGTH], ptr = 0 ;
	int z, halfPtr ;
	if(init)
		{
		for(hpfilt_ptr = 0; hpfilt_ptr < HPBUFFER_LGTH; ++hpfilt_ptr)
            hpfilt_data[hpfilt_ptr] = 0 ;
            hpfilt_ptr = 0 ;
            hpfilt_y = 0 ;
		}
        hpfilt_y += datum - hpfilt_data[hpfilt_ptr];
	halfPtr = hpfilt_ptr-(HPBUFFER_LGTH/2) ;
	if(halfPtr < 0)
		halfPtr += HPBUFFER_LGTH ;
	z = hpfilt_data[halfPtr] - (hpfilt_y / HPBUFFER_LGTH);
        hpfilt_data[hpfilt_ptr] = datum ;
	if(++hpfilt_ptr == HPBUFFER_LGTH)
        hpfilt_ptr = 0 ;
	return( z );
	}

/*****************************************************************************
*  deriv1 and deriv2 implement derivative approximations represented by
*  the difference equation:
*
*	y[n] = x[n] - x[n - 10ms]
*
*  Filter delay is DERIV_LENGTH/2
*****************************************************************************/
int QRSFILTcls::deriv1(int x, int init)
	{
	//static int derBuff[DERIV_LENGTH], derI = 0 ;
	int y ;
	if(init != 0)
		{
		for(deriv1_derI = 0; deriv1_derI < DERIV_LENGTH; ++deriv1_derI)
            deriv1_derBuff[deriv1_derI] = 0 ;
            deriv1_derI = 0 ;
		return(0) ;
		}
	y = x - deriv1_derBuff[deriv1_derI] ;
    deriv1_derBuff[deriv1_derI] = x ;
	if(++deriv1_derI == DERIV_LENGTH)
        deriv1_derI = 0 ;
	return(y) ;
	}
int QRSFILTcls::deriv2(int x, int init)
	{
	//static int derBuff[DERIV_LENGTH], derI = 0 ;
	int y ;
	if(init != 0)
		{
		for(deriv2_derI = 0; deriv2_derI < DERIV_LENGTH; ++deriv2_derI)
            deriv2_derBuff[deriv2_derI] = 0 ;
            deriv2_derI = 0 ;
		return(0) ;
		}
	y = x - deriv2_derBuff[deriv2_derI] ;
        deriv2_derBuff[deriv2_derI] = x ;
	if(++deriv2_derI == DERIV_LENGTH)
        deriv2_derI = 0 ;
	return(y) ;
	}


/*****************************************************************************
* mvwint() implements a moving window integrator.  Actually, mvwint() averages
* the signal values over the last WINDOW_WIDTH samples.
*****************************************************************************/
int QRSFILTcls::mvwint(int datum, int init)
	{
	//static long sum = 0 ;
	//static int data[WINDOW_WIDTH];
	//static int ptr = 0 ;
	int output;
	if(init)
		{
		for(mvwint_ptr = 0; mvwint_ptr < WINDOW_WIDTH ; ++mvwint_ptr)
            mvwint_data[mvwint_ptr] = 0 ;
            mvwint_sum = 0 ;
            mvwint_ptr = 0 ;
		}
        mvwint_sum += datum ;
        mvwint_sum -= mvwint_data[mvwint_ptr] ;
        mvwint_data[mvwint_ptr] = datum ;
	if(++mvwint_ptr == WINDOW_WIDTH)
        mvwint_ptr = 0 ;
	if((mvwint_sum / WINDOW_WIDTH) > 32000)
		output = 32000 ;
	else
		output = mvwint_sum / WINDOW_WIDTH ;
	return(output) ;
	}
