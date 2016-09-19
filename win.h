/* 
 * File:   win.h
 * Author: guiwen
 *
 * Created on March 21, 2014, 11:40 AM
 */

#ifndef WIN_H
#define	WIN_H

#ifdef	__cplusplus
extern "C" {
#endif


    typedef unsigned long DWORD;
    typedef int BOOL;
    typedef unsigned char BYTE;
    typedef unsigned short WORD;
    typedef DWORD DWORD_PTR;
    typedef long LONG;
#define MAKEWORD(a, b)      ((WORD)(((BYTE)((a) & 0xff)) | ((WORD)((BYTE)((b) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

#ifdef	__cplusplus
}
#endif

#endif	/* WIN_H */

