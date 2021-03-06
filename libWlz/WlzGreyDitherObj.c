#if defined(__GNUC__)
#ident "University of Edinburgh $Id$"
#else
static char _WlzGreyDitherObj_c[] = "University of Edinburgh $Id$";
#endif
/*!
* \file         libWlz/WlzGreyDitherObj.c
* \author       Richard Baldock
* \date         September 2003
* \version      $Id$
* \par
* Address:
*               MRC Human Genetics Unit,
*               MRC Institute of Genetics and Molecular Medicine,
*               University of Edinburgh,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \par
* Copyright (C), [2012],
* The University Court of the University of Edinburgh,
* Old College, Edinburgh, UK.
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be
* useful but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
* PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the Free
* Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA  02110-1301, USA.
* \brief	Makes a dithered object from the given grey-level object.
* \ingroup	WlzValuesUtils
*/

#include <stdlib.h>
#include <Wlz.h>


/* function:     WlzGreyDitherObj    */
/*! 
* \ingroup      WlzValuesUtils
* \brief        Makes a dithered object from the given grey-level Woolz
 object. The destination bits are determined by the number of shades in
 the dithered image (usually 1 bit) and the bit planes to use (typically
 to match the bit-planes of a display mask).
*
* \return       Object with dithered grey values.
* \param    o	Input object.
* \param    destBits	Destination bit planes for dithered values.
* \param    dstErr	Error return.
* \par      Source:
*                WlzGreyDitherObj.c
*/
WlzObject *WlzGreyDitherObj(
  WlzObject	*o,
  unsigned int  destBits,
  WlzErrorNum	*dstErr)
{
  WlzObject	*obj=NULL;
  WlzIntervalWSpace	iwsp1, iwsp2;
  WlzGreyWSpace		gwsp1, gwsp2;
  int		i, j, m, g, G;
  int		n_to;
  int		bit_val[8], permute_val[256];
  int		factor_from, factor_to, mask_start;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  /* check object type - 1 only */
  if( o == NULL ){
    errNum = WLZ_ERR_OBJECT_NULL;
  }
  else {
    switch( o->type ){
    case WLZ_2D_DOMAINOBJ:
      /* check it has a valuetable */
      if( o->values.core == NULL ){
	errNum = WLZ_ERR_VALUES_NULL;
      }
      else if( WlzGreyTableIsTiled(o->values.core->type) ){
	errNum = WLZ_ERR_VALUES_TYPE;
      }
      break;

    case WLZ_3D_DOMAINOBJ:
    case WLZ_TRANS_OBJ:
      errNum = WLZ_ERR_OBJECT_TYPE;
      break;

    case WLZ_EMPTY_OBJ:
      obj = WlzMakeEmpty(&errNum);
      break;

    default:
      errNum = WLZ_ERR_OBJECT_TYPE;
      break;
    }
  }

  if( !obj && (errNum == WLZ_ERR_NONE) ){
    /* count the bits in the mask */
    for(i=1, n_to=0; i <= destBits; i <<= 1 )
    {
      if( i & destBits )
	n_to++;
    }
    
    /* check numbers of bits */
    if( n_to == 0 ){
      errNum = WLZ_ERR_PARAM_DATA;
    }
    else {
      /* make a new object with the same domain */
      if((obj = WlzNewGrey(o, &errNum)) != NULL){
	if( n_to < 8 ){

	  /* set up some factors */
	  factor_from = 255;
	  factor_to = (1<<n_to) - 1;
	  mask_start = (1 << (8-n_to)) - 1;

	  /* set up bit permute lut */
	  for(i=0, j=1, m=destBits; i < n_to; i++)
	  {
	    while( !(m&1) )
	    {
	      m = m >> 1;
	      j = j << 1;
	    }
	    m = m >> 1;
	    bit_val[i] = j;
	  }

	  for(i=0; i < (1<<n_to); i++)
	  {
	    permute_val[i] = 0;
	    for(j=0, m=1; j < n_to; j++, m = m<<1 ){
	      permute_val[i] += (i&m) * bit_val[j];
	    }
	  }

	  for(;i < 256; i++){
	    permute_val[i] = 0;
	  }

	  /* now scan the objects */
	  WlzInitGreyScan(o,&iwsp1,&gwsp1);
	  WlzInitGreyScan(obj,&iwsp2,&gwsp2);
	  while((WlzNextGreyInterval(&iwsp1) == WLZ_ERR_NONE) && 
		(WlzNextGreyInterval(&iwsp2) == WLZ_ERR_NONE ) )
	  {
	    j = iwsp1.linpos;
	    G = rand() & mask_start;
	    switch(  gwsp1.pixeltype )
	    {
	    case WLZ_GREY_INT:
	      for(i=iwsp1.lftpos; i<=iwsp1.rgtpos; i++,
		    gwsp1.u_grintptr.inp++,
		    gwsp2.u_grintptr.inp++ )
	      {
		G += (*gwsp1.u_grintptr.inp) & 255;
		g = (G * factor_to) / factor_from;
		(*gwsp2.u_grintptr.inp) = permute_val[g];
		G -= (g * factor_from) / factor_to;
	      }
	      break;

	    case WLZ_GREY_SHORT:
	      for(i=iwsp1.lftpos; i<=iwsp1.rgtpos; i++,
		    gwsp1.u_grintptr.shp++,
		    gwsp2.u_grintptr.shp++ )
	      {
		G += (*gwsp1.u_grintptr.shp) & 255;
		g = (G * factor_to) / factor_from;
		(*gwsp2.u_grintptr.shp) = (short )(permute_val[g]);
		G -= (g * factor_from) / factor_to;
	      }
	      break;

	    case WLZ_GREY_UBYTE:
	      for(i=iwsp1.lftpos; i<=iwsp1.rgtpos; i++,
		    gwsp1.u_grintptr.ubp++,
		    gwsp2.u_grintptr.ubp++ )
	      {
		G += (*gwsp1.u_grintptr.ubp);
		g = (G * factor_to) / factor_from;
		(*gwsp2.u_grintptr.ubp) = (WlzUByte )(permute_val[g]);
		G -= (g * factor_from) / factor_to;
	      }
	      break;

	    case WLZ_GREY_RGBA: /* RGBA to be done RAB */
	    default:
	      errNum = WLZ_ERR_GREY_TYPE;
	      break;
	    }
	  }
	}
      }
    }
  }
    
  if( dstErr ){
    *dstErr = errNum;
  }
  return obj;
}
