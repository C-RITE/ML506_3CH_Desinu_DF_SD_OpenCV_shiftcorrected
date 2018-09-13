#include <math.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include "windows.h"

#define STAB_SUCCESS             0
#define STAB_INVALID_DATA       -1
#define STAB_INVALID_FFT_SIZE   -2
#define STAB_INVALID_BUFFER     -3
#define STAB_MEM_OVERFLOW       -4
#define STAB_NO_REFERENCE       -5

typedef struct {
	float real;
	float imag;
} COMPLEX;


class CStabFFT
{
private:
	COMPLEX **m_fftRef;
	COMPLEX **m_fftTar;

	int    m_imgWidth;
	int    m_imgHeight;
	int    m_nCenX;
	int    m_nCenY;
	int    m_nIdxX;
	int    m_nIdxY;
	float  m_fSparcity;

	int  FFT(int dir, int m, float *x, float *y);
	int  FFT2D(COMPLEX **c, int nx, int ny, int m, int n, int dir);
	int  CheckSizeFFT(int size, int *power2);

public:
	CStabFFT();
	~CStabFFT();

	int  ParamsInit(int fftNX, int fftNY, int sizeX, int sizeY, float sparcity);
	int  GetCenterXY(BYTE *image, int *deltaX, int *deltaY);
};