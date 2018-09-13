#include "stdafx.h"
#include <windowsx.h>
#include <memory.h>
#include <mmsystem.h>
#include "StabFFT.h"


CStabFFT::CStabFFT()
{
	m_imgWidth  = 0;
	m_imgHeight = 0;
	m_nIdxX     = 0;
	m_nIdxY     = 0;
	m_nCenX     = 0;
	m_nCenY     = 0;
	m_fftRef    = NULL;
	m_fftTar    = NULL;
}

CStabFFT::~CStabFFT()
{
	int i;

	if (m_fftRef != NULL) {
		for (i = 0; i < m_nCenX; i ++) free(m_fftRef[i]);
		free(m_fftRef);
	}
	if (m_fftTar != NULL) {
		for (i = 0; i < m_nCenX; i ++) free(m_fftTar[i]);
		free(m_fftTar);
	}
}

// set global reference
int CStabFFT::ParamsInit(int fftNX, int fftNY, int sizeX, int sizeY, float sparcity) {
	int i;

	m_imgWidth  = sizeX;
	m_imgHeight = sizeY;
	m_fSparcity = sparcity;

	if (fftNX > m_imgWidth || fftNY > m_imgHeight) return STAB_INVALID_DATA;

	if (m_nCenX != fftNX || m_nCenY != fftNY) {
		if (m_fftRef != NULL) {
			for (i = 0; i < m_nCenX; i ++) free(m_fftRef[i]);
			free(m_fftRef);
		}
		if (m_fftTar != NULL) {
			for (i = 0; i < m_nCenX; i ++) free(m_fftTar[i]);
			free(m_fftTar);
		}

		m_nCenX     = fftNX;
		m_nCenY     = fftNY;

		m_fftRef = (COMPLEX **)malloc(m_nCenX*sizeof(COMPLEX));
		for (i = 0; i < m_nCenX; i ++) 
			m_fftRef[i] = (COMPLEX *)malloc(m_nCenY*sizeof(COMPLEX));

		m_fftTar = (COMPLEX **)malloc(m_nCenX*sizeof(COMPLEX));
		for (i = 0; i < m_nCenX; i ++) 
			m_fftTar[i] = (COMPLEX *)malloc(m_nCenY*sizeof(COMPLEX));
	}

	// check if FFT width is a power of 2
	if (CheckSizeFFT(m_nCenX, &m_nIdxX) != 0) return STAB_INVALID_FFT_SIZE;
	// check if FFT height is a power of 2
	if (CheckSizeFFT(m_nCenY, &m_nIdxY) != 0) return STAB_INVALID_FFT_SIZE;

	return STAB_SUCCESS;
}


int CStabFFT::GetCenterXY(BYTE *image, int *deltaX, int *deltaY) 
{
	int    i, j, n;
	int    offsetX, offsetY, max_x, max_y;
	float  real, imag, dMax;
	
	offsetX = (m_imgWidth-m_nCenX)/2;
	offsetY = (m_imgHeight-m_nCenY)/2;

	

	// process the patch from forward scanning
	for (j = 0; j < m_nCenY; j ++) {
		n = (offsetY+j)*m_imgWidth*2;
		for (i = 0; i < m_nCenX; i ++) {
			m_fftTar[i][j].real = image[n+i+offsetX];
			m_fftTar[i][j].imag = 0;
		}
	}

	FFT2D(m_fftTar, m_nCenX, m_nCenY, m_nIdxX, m_nIdxY, 1);

	// process the patch from backward scanning
	for (j = 0; j < m_nCenY; j ++) {
		n = (offsetY+j)*m_imgWidth*2;
		for (i = 0; i < m_nCenX; i ++) {
			m_fftRef[i][j].real = image[n+m_imgWidth*2-1-i-offsetX];
			m_fftRef[i][j].imag = 0;
		}
	}

	FFT2D(m_fftRef, m_nCenX, m_nCenY, m_nIdxX, m_nIdxY, 1);

	// do dot product of reference and target
	for (j = 0; j < m_nCenY; j ++) {
		for (i = 0; i < m_nCenX; i ++) {
			real = m_fftRef[i][j].real*m_fftTar[i][j].real + m_fftRef[i][j].imag*m_fftTar[i][j].imag;
			imag = m_fftRef[i][j].imag*m_fftTar[i][j].real - m_fftRef[i][j].real*m_fftTar[i][j].imag;
			m_fftTar[i][j].real = real;
			m_fftTar[i][j].imag = imag;
		}
	}
	// do inverse 2D FFT on the result of fft(x).*conj(fft(y))
	FFT2D(m_fftTar, m_nCenX, m_nCenY, m_nIdxX, m_nIdxY, -1);

	// retrieve real part of    fft(x).*conj(fft(y))
	max_x = max_y = -1000000000;
	dMax = -1000000000;
	for (j = 0; j < m_nCenY; j ++) {
		for (i = 0; i < m_nCenX; i ++) {
			if (dMax < m_fftTar[i][j].real) {
				dMax  = m_fftTar[i][j].real;
				max_x = i;
				max_y = j;
			}
		}
	}

	max_x = (max_x>=m_nCenX/2) ? max_x-m_nCenX : max_x;
	max_y = (max_y>=m_nCenY/2) ? max_y-m_nCenY : max_y;
	*deltaX = max_x;
	*deltaY = max_y;

	return STAB_SUCCESS;
}

/*--------------------------------------------------------------------*/
/*  This computes an in-place complex-to-complex FFT                  */
/*  x and y are the real and imaginary arrays of 2^m points.          */
/*  dir =  1 gives forward transform                                  */
/*  dir = -1 gives reverse transform                                  */
/*                                                                    */
/*    Formula: forward                                                */
/*                 N-1                                                */
/*                 ---                                                */
/*             1   \          - j k 2 pi n / N                        */
/*     X(n) = ---   >   x(k) e                    = forward transform */
/*             N   /                                n=0..N-1          */
/*                 ---                                                */
/*                 k=0                                                */
/*                                                                    */
/*     Formula: reverse                                               */
/*                 N-1                                                */
/*                 ---                                                */
/*                 \          j k 2 pi n / N                          */
/*     X(n) =       >   x(k) e                    = forward transform */
/*                 /                                n=0..N-1          */
/*                 ---                                                */
/*                 k=0                                                */
/*                                                                    */
/*--------------------------------------------------------------------*/
/*  1 dimensional Fast Fourier Transform                              */ 
/*                                                                    */
/*  parameters:                                                       */
/*    1. direction of FFT, integer, (IN),                             */
/*       dir=1 forward FFT, dir=-1 inverse FFT                        */
/*    2. length of the array, integer                                 */
/*    3. real part of the array, (I/O)                                */
/*    4. imaginary part of the array, (I/O)                           */
/*                                                                    */
/*--------------------------------------------------------------------*/
int CStabFFT::FFT(int dir,int m,float *x,float *y)
{
	long nn,i,i1,j,k,i2,l,l1,l2;
	float c1,c2,tx,ty,t1,t2,u1,u2,z;

	/* Calculate the number of points */
	nn = 1;
	for (i=0;i<m;i++)
		nn *= 2;

	/* Do the bit reversal */
	i2 = nn >> 1;
	j = 0;
	for (i=0;i<nn-1;i++) {
		if (i < j) {
			tx = x[i];
			ty = y[i];
			x[i] = x[j];
			y[i] = y[j];
			x[j] = tx;
			y[j] = ty;
		}
		k = i2;
		while (k <= j) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}

	/* Compute the FFT */
	c1 = -1.0;
	c2 = 0.0;
	l2 = 1;
	for (l=0;l<m;l++) {
		l1 = l2;
		l2 <<= 1;
		u1 = 1.0;
		u2 = 0.0;
		for (j=0;j<l1;j++) {
			for (i=j;i<nn;i+=l2) {
				i1 = i + l1;
				t1 = u1 * x[i1] - u2 * y[i1];
				t2 = u1 * y[i1] + u2 * x[i1];
				x[i1] = x[i] - t1;
				y[i1] = y[i] - t2;
				x[i] += t1;
				y[i] += t2;
			}
			z =  u1 * c1 - u2 * c2;
			u2 = u1 * c2 + u2 * c1;
			u1 = z;
		}
		c2 = sqrt((1.0f - c1) / 2.0f);
		if (dir == 1)
			c2 = -c2;
		c1 = sqrt((1.0f + c1) / 2.0f);
	}

	/* Scaling for forward transform */
	if (dir == 1) {
		for (i=0;i<nn;i++) {
			x[i] /= (float)nn;
			y[i] /= (float)nn;
		}
	}

	return STAB_SUCCESS;
}


/*------------------------------------------------------------------*/
/*  2 dimensional Fast Fourier Transform                            */ 
/*                                                                  */
/*  parameters:                                                     */
/*    1. 2-D image array, (IN/OUT)                                  */
/*    2. width of the image, integer                                */
/*    3. height of the image, integer                               */
/*    4. power of dimension, integer                                */
/*    5. direction of FFT, integer, (IN),                           */
/*       dir=1 forward FFT, dir=-1 inverse FFT                      */
/*                                                                  */
/*------------------------------------------------------------------*/
int CStabFFT::FFT2D(COMPLEX **c, int nx, int ny, int m, int n, int dir)
{
	int i,j;
	float *real,*imag;

	/* Transform the rows */
	real = (float *)malloc(nx * sizeof(float));
	imag = (float *)malloc(nx * sizeof(float));
	if (real == NULL || imag == NULL) {
		return STAB_MEM_OVERFLOW;
	}
	
	for (j=0;j<ny;j++) {
		for (i=0;i<nx;i++) {
			real[i] = c[i][j].real;
			imag[i] = c[i][j].imag;
		}
		FFT(dir,m,real,imag);
		for (i=0;i<nx;i++) {
			c[i][j].real = real[i];
			c[i][j].imag = imag[i];
		}
	}
	free(real);
	free(imag);

	/* Transform the columns */
	real = (float *)malloc(ny * sizeof(float));
	imag = (float *)malloc(ny * sizeof(float));
	if (real == NULL || imag == NULL) {
		return STAB_MEM_OVERFLOW;
	}

	for (i=0;i<nx;i++) {
		for (j=0;j<ny;j++) {
			real[j] = c[i][j].real;
			imag[j] = c[i][j].imag;
		}
		FFT(dir,n,real,imag);
		for (j=0;j<ny;j++) {
			c[i][j].real = real[j];
			c[i][j].imag = imag[j];
		}
	}

	free(real);
	free(imag);

	return STAB_SUCCESS;
}



// check if FFT width is a power of 2
int CStabFFT::CheckSizeFFT(int size, int *power2) {
	int rem, idx;
	
	rem = size;
	idx = 0;
	do {
		rem = rem >> 1;
		idx ++;
	} while (rem > 0);
	*power2 = idx-1;
	rem = 1;
	do {
		rem = rem << 1;
		idx --;
	} while (idx > 1);
	if (rem != size) return STAB_INVALID_FFT_SIZE;

	return STAB_SUCCESS;
}

