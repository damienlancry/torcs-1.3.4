/***************************************************************************
                          tgf.cpp -- The Gaming Framework
                             -------------------
    created              : Fri Aug 13 22:31:43 CEST 1999
    copyright            : (C) 1999 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: tgf.cpp,v 1.16.2.3 2011/12/28 15:01:57 berniw Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include <errno.h>
#include <portability.h>

#include <tgf.h>
#include <time.h>

extern void gfDirInit(void);
extern void gfModInit(void);
extern void gfOsInit(void);
extern void gfParamInit(void);
extern void gfRlstInit(void);


static bool __NOISY__;

void setNoisy(bool noisy)
{
	__NOISY__ = noisy;
}
bool getNoisy()
{
	return __NOISY__;
}


#ifdef WIN32
#include <crtdbg.h>
#include <assert.h>


void * _tgf_win_malloc(size_t size)
{
#ifdef _DEBUG
	char * p = (char*)GlobalAlloc(GMEM_FIXED, size + 3*sizeof(int));
	*(int*)(p) = size + 3*sizeof(int);
	*((int*)p + 1) = 123456789;
	*((int*)(p + size + 3*sizeof(int)) - 1) = 987654321;

	return p + 2*sizeof(int);
#else // _DEBUG
	char * p = (char*)GlobalAlloc(GMEM_FIXED, size + sizeof(int));
	if (p == NULL) {
		return NULL;
	}
	*(int*)(p) = size;
	return p + sizeof(int);
#endif // _DEBUG
}


void * _tgf_win_calloc(size_t num, size_t size)
{
	void * p = _tgf_win_malloc(num * size);
	memset(p, 0, num * size);
	return p;
}


void * _tgf_win_realloc(void * memblock, size_t size)
{
	if (size == 0) {
		_tgf_win_free(memblock);
		return NULL;
	}

	void * p = _tgf_win_malloc(size);
	if (p == NULL) {
		return NULL;
	}

	if (memblock != NULL) {
#ifdef _DEBUG
		memcpy(p, memblock, min(*(int*)((char*)memblock-2*sizeof(int)), (int)size));
#else // _DEBUG
		memcpy(p, memblock, min(*(int*)((char*)memblock-sizeof(int)), (int)size));
#endif // _DEBUG
		_tgf_win_free(memblock);
	}
	return p;
}


void _tgf_win_free(void * memblock)
{
	if (!memblock) {
		return;
	}

#ifdef _DEBUG
	char * p = (char*)memblock - 2*sizeof(int);

	if (!_CrtIsValidPointer(p, sizeof(int), TRUE)) {
		assert(0);
	}

	if (!_CrtIsValidPointer(p, *(int*)p, TRUE)) {
		assert( 0 );
	}

	if (*((int*)p + 1) != 123456789) {
		assert( 0 );
	}

	if(*((int*)(p + *(int*)p ) - 1) != 987654321) {
		assert( 0 );
	}

	GlobalFree((char*)memblock - 2*sizeof(int));
#else // _DEBUG
	GlobalFree((char*)memblock - sizeof(int));
#endif // _DEBUG
}


char * _tgf_win_strdup(const char * str)
{
	char * s = (char*)_tgf_win_malloc(strlen(str)+1);
	strcpy(s,str);

	return s;
}
#endif // WIN32


void GfInit(void)
{
	gfDirInit();
	gfModInit();
	gfOsInit();
	gfParamInit();
}


void gfMeanReset(tdble v, tMeanVal *pvt)
{
	int i;

	for (i = 0; i < GF_MEAN_MAX_VAL; i++) {
		pvt->val[i] = v;
	}
}


tdble gfMean(tdble v, tMeanVal *pvt, int n, int w)
{
	int i;
	tdble sum;

	if (n > pvt->curNum) {
		if (pvt->curNum < GF_MEAN_MAX_VAL) {
			pvt->curNum++;
		}
		n = pvt->curNum;
	} else {
		pvt->curNum = n;
	}

	sum = 0;
	for (i = 0; i < n; i++) {
		pvt->val[i] = pvt->val[i + 1];
		sum += pvt->val[i];
	}

	pvt->val[n] = v;
	sum += (tdble)w * v;
	sum /= (tdble)(n + w);

	return sum;
}


/** Convert a time in seconds (float) to an ascii string.
    @ingroup	screen
    @param	sec	Time to convert
    @param	sgn	Flag to indicate if the sign (+) is to be displayed for positive values of time.
    @return	Time string.
    @warning	The returned string has to be free by the caller.
 */
char * GfTime2Str(tdble sec, int sgn)
{
	const int BUFSIZE = 256;
	char buf[BUFSIZE];
	const char* sign;

	if (sec < 0.0) {
		sec = -sec;
		sign = "-";
	} else {
		if (sgn) {
			sign = "+";
		} else {
			sign = "  ";
		}
	}

	int h = (int)(sec / 3600.0);
	sec -= 3600 * h;
	int m = (int)(sec / 60.0);
	sec -= 60 * m;
	int s = (int)(sec);
	sec -= s;
	int c = (int)floor((sec) * 100.0);

	if (h) {
		snprintf(buf, BUFSIZE, "%s%2.2d:%2.2d:%2.2d:%2.2d", sign,h,m,s,c);
	} else if (m) {
		snprintf(buf, BUFSIZE, "   %s%2.2d:%2.2d:%2.2d", sign,m,s,c);
	} else {
		snprintf(buf, BUFSIZE, "      %s%2.2d:%2.2d", sign,s,c);
	}
	return strdup(buf);
}


static char *localDir = strdup("");
static char *libDir = strdup("");
static char *dataDir = strdup("");


char * GetLocalDir(void)
{
	return localDir;
}


void SetLocalDir(char *buf)
{
	free(localDir);
	localDir = strdup(buf);
}


char * GetLibDir(void)
{
	return libDir;
}


void SetLibDir(char *buf)
{
	free(libDir);
	libDir = strdup(buf);
}


char * GetDataDir(void)
{
	return dataDir;
}


void SetDataDir(char *buf)
{
	free(dataDir);
	dataDir = strdup(buf);
}


static int singleTextureMode = 0;


int GetSingleTextureMode (void)
{
	return singleTextureMode;
}


void SetSingleTextureMode (void)
{
	singleTextureMode = 1;
}


int GfNearestPow2 (int x)
{
	int r;

	if (!x) {
		return 0;
	}

	x++;
	r = 1;
	while ((1 << r) < x) {
		r++;
	}
	r--;

	return (1 << r);
}


int GfCreateDir(char *path)
{
	if (path == NULL) {
		return GF_DIR_CREATION_FAILED;
	}

	const int BUFSIZE = 1024;
	char buf[BUFSIZE];
	strncpy(buf, path, BUFSIZE);
	path = buf;

#ifdef WIN32
#define mkdir(x) _mkdir(x)

	// Translate path.
	const char DELIM = '\\';
	int i;
	for (i = 0; i < BUFSIZE && buf[i] != '\0'; i++) {
		if (buf[i] == '/') {
			buf[i] = DELIM;
		}
	}

#else // WIN32
#define mkdir(x) mkdir((x), S_IRWXU);

	const char DELIM = '/';

#endif // WIN32

	int err = mkdir(buf);
	if (err == -1) {
		if (errno == ENOENT) {
			char *end = strrchr(buf, DELIM);
			*end = '\0';
			GfCreateDir(buf);
			*end = DELIM;
			err = mkdir(buf);

		}
	}

	if (err == -1 && errno != EEXIST) {
		return GF_DIR_CREATION_FAILED;
	} else {
		return GF_DIR_CREATED;
	}
}

/* flags used to remove damage, time-limit and fuel consumption */
static bool _damageLimit = true;
static bool _fuelConsumption = true;
static bool _laptimeLimit = true;
/* timeout for UDP connection */
static long int _timeout = -1;
/* timeout for UDP connection */
static short int _port = -1;

/*version tag*/
static char *_version;

/* Helper to set and get the flags used to remove damage, time-limit and fuel consumption */
void setDamageLimit(bool damageLimit)
{
	_damageLimit = damageLimit;
}
bool getDamageLimit()
{
	return _damageLimit;
}
void setFuelConsumption(bool fuelConsumption)
{
	_fuelConsumption = fuelConsumption;
}
bool getFuelConsumption()
{
	return _fuelConsumption;
}
void setLaptimeLimit(bool laptimeLimit)
{
	_laptimeLimit = laptimeLimit;
}
bool getLaptimeLimit()
{
	return _laptimeLimit;
}

/* Helper to set and get the timeout of UDP comm */
void setTimeout(long int timeout)
{
	_timeout = timeout;
}

long int getTimeout()
{
	return _timeout;
}
/* Helper to set and get the timeout of UDP comm */
void setListenPort(short int port)
{
	_port = port;
}

short int getListenPort()
{
	return _port;
}

void setVersion(char *version)
{
	_version = (char *)malloc(strlen(version) + 1);
	strcpy(_version,version);
}
char* getVersion()
{
	return _version;
}
