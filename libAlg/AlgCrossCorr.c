#pragma ident "MRC HGU $Id$"
/************************************************************************
* Project:      Mouse Atlas
* Title:        AlgCrossCor.c
* Date:         January 2001
* Author:       Bill Hill
* Copyright:	2001 Medical Research Council, UK.
*		All rights reserved.
* Address:	MRC Human Genetics Unit,
*		Western General Hospital,
*		Edinburgh, EH4 2XU, UK.
* Purpose:	Frequency domain cross correlation functions for the
*		MRC Human Genetics Unit numerical algorithm library.
* $Revision$
* Maintenance:  Log changes below, with most recent at top of list.
************************************************************************/
#include <Alg.h>
#include <math.h>
#include <float.h>

/************************************************************************
* Function:	AlgCrossCorrelate2D
* Returns:	AlgError:		Error code.
* Purpose:	Cross correlates the given 2D double arrays leaving
*		the result in the first of the two arrays.
*		The cross correlation data are un-normalized.
* Global refs:	-
* Parameters:	double **data0:		Data for/with obj0's FFT 
*					(source: AlcDouble2Malloc)
*					which holds the cross	
*					correlation data on return.
*		double **data1:		Data for/with obj1's FFT 
*					(source: AlcDouble2Malloc).
*		int nX:			Number of columns in each of the
*					data arrays.
*		int nY:			Number of lines in each of the
*					data arrays.
************************************************************************/
AlgError	AlgCrossCorrelate2D(double **data0, double **data1,
			            int nX, int nY)
{
  int		tI0,
		tI1,
		idX,
		idY,
  		nX2,
		nY2;
  double	tD1,
		tD2,
		tD3,
		tD4;
  double	*tDP1,
		*tDP2;
  double	*reBuf = NULL,
		*imBuf = NULL;
  AlgError	errNum = ALG_ERR_NONE;
  const int	cThr = 1,
  		minN = 8,
  		maxN = 1048576;

  if((data0 == NULL) || (data1 == NULL) ||
     (nX < minN) || (nX > maxN) || (nY < minN) || (nY > maxN))
  {
     errNum = ALG_ERR_FUNC;
  }
  else
  {
    (void )AlgBitNextPowerOfTwo((unsigned int *)&tI0, nX);
    (void )AlgBitNextPowerOfTwo((unsigned int *)&tI1, nY);
    if((tI0 != nX) || (tI1 != nY))
    {
      errNum = ALG_ERR_FUNC;
    }
  }
  if(errNum == ALG_ERR_NONE)
  {
    if(((reBuf = (double *)AlcMalloc(sizeof(double) * nY)) == NULL) ||
       ((imBuf = (double *)AlcMalloc(sizeof(double) * nY)) == NULL))
    {
      errNum = ALG_ERR_MALLOC;
    }
  }
  if(errNum == ALG_ERR_NONE)
  {
    AlgFourReal2D(data0, reBuf, imBuf, nX, nY, cThr);
    AlgFourReal2D(data1, reBuf, imBuf, nX, nY, cThr);
    nX2 = nX / 2;
    nY2 = nY / 2;
    for(idY = 0; idY < nY; ++idY)
    {
      tDP1 = *(data0 + idY) + 1;
      tDP2 = *(data1 + idY) + 1;
      for(idX = 1; idX < nX2; ++idX)
      {
	tD1 = *tDP1;
	tD2 = *(tDP1 + nX2);
	tD3 = *tDP2;
	tD4 = -*(tDP2 + nX2);
	*tDP1 = tD1 * tD3 - tD2 * tD4;
	*(tDP1 + nX2) = tD1 * tD4 + tD2 * tD3;
	++tDP1;
	++tDP2;
      }
    }
    for(idX = 0; idX < nX; idX += nX2)
    {
      for(idY = 1; idY < nY2; ++idY)
      {
	tDP1 = *(data0 + idY) + idX;
	tDP2 = *(data0 + nY2 + idY) + idX;
	tD1 = *tDP1;
	tD2 = *tDP2;
	tD3 = *(*(data1 + idY) + idX);
	tD4 = -*(*(data1 + nY2 + idY) + idX);
	*tDP1 = tD1 * tD3 - tD2 * tD4;
	*tDP2 = tD1 * tD4 + tD2 * tD3;
      }
    }
    **data0 *= **data1;
    **(data0 + nY2) *= **(data1 + nY2);
    *(*data0 + nX2) *= *(*data1 + nX2);
    *(*(data0 + nY2) + nX2) *= *(*(data1 + nY2) + nX2);
    AlgFourRealInv2D(data0, reBuf, imBuf, nX, nY, cThr);
  }
  if(reBuf)
  {
    AlcFree(reBuf);
  }
  if(imBuf)
  {
    AlcFree(imBuf);
  }
  return(errNum);
}

/************************************************************************
* Function:	AlgCrossCorrPeakXY					
* Returns:	void						
* Purpose:	Find the maximum correlation value in the given two
*		dimensional array. Because the correlation data are 
*		stored in wrap-around order and the maximum is known
*		to lie within a limited search range, only the four
*		corners of the array are searched.		
* Global refs:	-						
* Parameters:	int *dstMaxX:		Destination ptr for column
*					coordinate with maximum value.
*		int *dstMaxY:		Destination ptr for line
*					coordinate with maximum value.		
*		double *dstMaxVal	Destination ptr for maximum value.	
*		double **data:		Data to search for maximum.
*		int nX:			Number of colimns in data.
*		int nY:			Number of lines in data.
*		int searchX:		Maximum number of columns to
*					search.
*		int searchY:		Maximum number of lines to
*					search.
************************************************************************/
void		AlgCrossCorrPeakXY(int *dstMaxX, int *dstMaxY,
				   double *dstMaxVal,
				   double **data, int nX, int nY,
				   int searchX, int searchY)
{
  int		yIdx,
		xIdx,
		xMax,
		yMax;
  double	maxVal;
  double	*left,
		*right;

  xMax = 0;
  yMax = 0;
  maxVal = **data;
  for(yIdx = 0; yIdx < searchY; ++yIdx) 	/* Search two bottom corners */
  {
    left = *(data + yIdx);
    right = left + nX - 1;
    for(xIdx = 0; xIdx < searchX; ++xIdx)
    {
      if(*left > maxVal)
      {
	maxVal = *left;
	xMax = xIdx;
	yMax = yIdx;
      }
      if(*right > maxVal)
      {
	maxVal = *right;
	xMax = -(xIdx + 1);
	yMax = yIdx;
      }
      ++left;
      --right;
    }
  }
  for(yIdx = 1; yIdx <= searchY; ++yIdx)	   /* Search two top corners */
  {
    left = *(data + nY - yIdx);
    right = left + nX - 1;
    for(xIdx= 0; xIdx < searchX; ++xIdx)
    {
      if(*left > maxVal)
      {
	maxVal = *left;
	xMax = xIdx;
	yMax = -yIdx;
      }
      if(*right > maxVal)
      {
	maxVal = *right;
	xMax = -(xIdx + 1);
	yMax = -yIdx;
      }
      ++left;
      --right;
    }
  }
  if(dstMaxVal)
  {
    *dstMaxVal = maxVal;
  }
  if(dstMaxX)
  {
    *dstMaxX = xMax;
  }
  if(dstMaxY)
  {
    *dstMaxY = yMax;
  }
}

/************************************************************************
* Function:	AlgCrossCorrPeakY					
* Returns:	void						
* Purpose:	Finds peak value in cross correlation data, only
*		searching the first column.
*		The search is particularly simple because only the
*		first column of the cross correlation data needs to be
*		searched for the angle of rotation.		
*		Data are in wrap around order.			
* Global refs:	-						
* Parameters:	double *dstMaxY:	Destination ptr for line
*					coordinate with maximum value.
*					correlation maximum.	
*		double *dstMaxVal:	Cross correlation maximum.
*		double **data:		Cross correlation data.	
*		int nY:			Number of lines in data array.
************************************************************************/
void		AlgCrossCorrPeakY(int *dstMaxY, double *dstMaxVal,
				   double **data, int nY)
{
  int		idx,
		maxIdx;
  double	maxVal;

  maxIdx = 0;
  maxVal = (**data);
  for(idx = 1; idx < nY; ++idx)
  {
    if(**(data + idx) > maxVal)
    {
      maxVal = **(data + idx);
      maxIdx = idx;
    }
  }
  if(maxIdx > (nY / 2))
  {
    maxIdx -= nY;
  }
  if(dstMaxY)
  {
    *dstMaxY = maxIdx;
  }
  if(dstMaxVal)
  {
    *dstMaxVal = maxVal;
  }
}

#ifdef ALG_CROSSCORR_TEST
int		main(int argc, char *argv[])
{
  int		iX,
		iY,
  		oX,
  		oY,
		nX = 16,
  		nY = 16,
		rep,
		nRep = 1024,
		allRep = 0;
  double	sum;
  double	**data0 = NULL,
  		**data1 = NULL;
  AlgError	errNum = ALG_ERR_NONE;

  AlgRandSeed(time(NULL));
  if((AlcDouble2Malloc(&data0, nY, nX) != ALC_ER_NONE) ||
     (AlcDouble2Malloc(&data1, nY, nX) != ALC_ER_NONE))
  {
    errNum = ALG_ERR_MALLOC;
  }
  else
  {
    for(rep = 0; rep < nRep; ++rep)
    {
      /* iX = (int )(8.0 * (0.5 - AlgRandUniform())); */
      /* iY = (int )(8.0 * (0.5 - AlgRandUniform())); */
      iX = 3;
      iY = -1;
      for(oY = 0; oY < nY; ++oY)
      {
	for(oX = 0; oX < nX; ++oX)
	{
	  *(*(data0 + oY) + oX) = 0.0;
	  *(*(data1 + oY) + oX) = 0.0;
        }
      }
      *(*(data0 + 8 + iY) + 8 + iX) = 1.0;
      *(*(data1 + 8) + 8) = 1.0;
      errNum = AlgCrossCorrelate2D(data0, data1, nX, nY);
      if(errNum == ALG_ERR_NONE)
      {
	sum = 0;
	for(oY = 0; oY < nY; ++oY)
	{
	  for(oX = 0; oX < nX; ++oX)
	  {
	    sum += *(*(data0 + oY) + oX);
	  }
	}
	if(sum < 1.0)
	{
	  sum = 1.0;
	}
	AlgCrossCorrPeakXY(&oX, &oY, NULL, data0, nX, nY, 5, 5);
	if(allRep || (iX != oX) || (iY != oY))
	{
	  (void )fprintf(stderr, "(%d %d) (%d %d)\n", iX, iY, oX, oY);
	  for(oY = 0; oY < nY; ++oY)
	  {
	    for(oX = 0; oX < nX; ++oX)
	    {
	      if(*(*(data0 + oY) + oX) > sum / 2.0)
	      {
	        (void )fprintf(stderr, "#");
	      }
	      else
	      {
	        (void )fprintf(stderr, "+");
	      }
	    }
	    (void )fprintf(stderr, "\n");
	  }
	  (void )fprintf(stderr, "\n");
	}
      }
    }
  }
  return(0);
}
#endif /* ALG_CROSSCORR_TEST */

