#if defined(__GNUC__)
#ident "MRC HGU $Id$"
#else
#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma ident "MRC HGU $Id$"
#else
static char _WlzFreeSpace_c[] = "MRC HGU $Id$";
#endif
#endif
/*!
* \file         libWlz/WlzFreeSpace.c
* \author       Bill Hill, Richard Baldock
* \date         March 1999
* \version      $Id$
* \par
* Address:
*               MRC Human Genetics Unit,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \par
* Copyright (C) 2005 Medical research Council, UK.
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
* \brief	Functions for freeing objects and their components.
* \ingroup	WlzAllocation
* \todo         -
* \bug          None known.
*/

#include <stdlib.h>
#include <Wlz.h>

/* function:     WlzFreeObj    */
/*! 
* \ingroup      WlzAllocation
* \brief        Free space allocated to a woolz object.
*
* \return       Error number, values: WLZ_ERR_NONE, WLZ_ERR_MEM_FREE
* \param    obj	Object to be freed.
* \par      Source:
*                WlzFreeSpace.c
*/
WlzErrorNum WlzFreeObj(WlzObject *obj)
{
  WlzCompoundArray	*ca = (WlzCompoundArray *) obj;
  WlzErrorNum		errNum = WLZ_ERR_NONE;
  int 			i;

  WLZ_DBG((WLZ_DBG_ALLOC|WLZ_DBG_LVL_FN|WLZ_DBG_LVL_1),
  	  ("WlzFreeObj FE 0x%lx\n",
	   (unsigned long )obj));

  /* check the object pointer and linkcount */
  if (obj == NULL){
    return( WLZ_ERR_NONE );
  }

  if( WlzUnlink(&(obj->linkcount), &errNum) ){    /* Check linkcount */

    switch( obj->type ){

    case WLZ_2D_DOMAINOBJ:
      WLZ_DBG((WLZ_DBG_ALLOC|WLZ_DBG_LVL_1),
	      ("WlzFreeObj 01 0x%lx WLZ_2D_DOMAINOBJ "
	       "0x%lx %d 0x%lx %d 0x%lx\n",
	       (unsigned long )obj,
	       (unsigned long )(obj->domain.i),
	       (obj->domain.i?obj->domain.i->linkcount: 0),
	       (unsigned long )(obj->values.v),
	       ((obj->values.v)? obj->values.v->linkcount: 0),
	       (unsigned long )(obj->plist)));
      if( WlzFreeDomain(obj->domain) ||
	  WlzFreeValueTb(obj->values.v) ||
	  WlzFreePropertyList(obj->plist) ||
	  WlzFreeObj(obj->assoc) ){
	errNum = WLZ_ERR_MEM_FREE;
      }
      break;

    case WLZ_3D_DOMAINOBJ:
      WLZ_DBG((WLZ_DBG_ALLOC|WLZ_DBG_LVL_1),
      	      ("WlzFreeObj 02 0x%lx WLZ_3D_DOMAINOBJ 0x%lx, "
	       "%d 0x%lx %d 0x%lx\n",
	       (unsigned long )obj, (unsigned long )(obj->domain.i),
	       (obj->domain.p?obj->domain.p->linkcount: 0),
	       (unsigned long )(obj->values.vox),
	       (obj->values.vox?obj->values.vox->linkcount: 0),
	       (unsigned long )(obj->plist)));
      if( WlzFreePlaneDomain(obj->domain.p) ||
	  WlzFreeVoxelValueTb(obj->values.vox) ||
	  WlzFreePropertyList(obj->plist) ||
	  WlzFreeObj(obj->assoc) ){
	errNum = WLZ_ERR_MEM_FREE;
      }
      break;

    case WLZ_TRANS_OBJ:
      WLZ_DBG((WLZ_DBG_ALLOC|WLZ_DBG_LVL_1),
      	      ("WlzFreeObj 03 0x%lx WLZ_TRANS_OBJ 0x%lx, "
	       "%d 0x%lx %d 0x%lx\n",
	       (unsigned long )obj,
	       (unsigned long )(obj->domain.t),
	       ((obj->domain.t)?(obj->domain.t)->linkcount: 0),
	       (unsigned long )(obj->values.obj),
	       ((obj->values.obj)?(obj->values.obj)->linkcount: 0),
	       (unsigned long )(obj->plist)));
      if( WlzFreeAffineTransform(obj->domain.t) ||
	  WlzFreeObj(obj->values.obj) ||
	  WlzFreePropertyList(obj->plist) ||
	  WlzFreeObj(obj->assoc) ){
	errNum = WLZ_ERR_MEM_FREE;
      }
      break;

    case WLZ_2D_POLYGON:
      WLZ_DBG((WLZ_DBG_ALLOC|WLZ_DBG_LVL_1),
      	      ("WlzFreeObj 05 0x%lx WLZ_2D_POLYGON 0x%lx\n",
	       (unsigned long )obj, (unsigned long )(obj->domain.poly)));
      errNum = WlzFreePolyDmn(obj->domain.poly);
      break;

    case WLZ_BOUNDLIST:
      WLZ_DBG((WLZ_DBG_ALLOC|WLZ_DBG_LVL_1),
      	      ("WlzFreeObj 06 0x%lx WLZ_BOUNDLIST 0x%lx\n",
	       (unsigned long )obj, (unsigned long )(obj->domain.b)));
      errNum = WlzFreeBoundList(obj->domain.b);
      break;

    case WLZ_CONTOUR:
      WLZ_DBG((WLZ_DBG_ALLOC|WLZ_DBG_LVL_1),
      	      ("WlzFreeObj 06 0x%lx WLZ_CONTOUR 0x%lx\n",
	       (unsigned long )obj, (unsigned long )(obj->domain.ctr)));
      errNum = WlzFreeContour(obj->domain.ctr);
      break;

    case WLZ_CONV_HULL:
      WLZ_DBG((WLZ_DBG_ALLOC|WLZ_DBG_LVL_1),
      	      ("WlzFreeObj 07 0x%lx WLZ_CONV_HULL 0x%lx 0x%lx\n",
	       (unsigned long )obj,
	       (unsigned long )(obj->domain.poly),
	       (unsigned long)(obj->values.c)));
      if( WlzFreePolyDmn(obj->domain.poly) ||
	  WlzFreeConvHull(obj->values.c) ){
	errNum = WLZ_ERR_MEM_FREE;
      }
      break;

    case WLZ_HISTOGRAM:
      WLZ_DBG((WLZ_DBG_ALLOC|WLZ_DBG_LVL_1),
      	      ("WlzFreeObj 08 0x%lx WLZ_CONV_HULL 0x%lx\n",
	       (unsigned long )obj, (unsigned long )(obj->domain.hist)));
      errNum = WlzFreeDomain(obj->domain);
      break;

    case WLZ_RECTANGLE:
      WLZ_DBG((WLZ_DBG_ALLOC|WLZ_DBG_LVL_1),
      	      ("WlzFreeObj 09 0x%lx WLZ_RECTANGLE 0x%lx\n",
	       (unsigned long )obj, (unsigned long )(obj->domain.r)));
      errNum = WlzFreeDomain(obj->domain);
      break;

    case WLZ_AFFINE_TRANS:
      WLZ_DBG((WLZ_DBG_ALLOC|WLZ_DBG_LVL_1),
      	      ("WlzFreeObj 12 0x%lx WLZ_AFFINE_TRANS\n",
	       (unsigned long )obj));
      errNum = WlzFreeAffineTransform(obj->domain.t);
      break;

    case WLZ_COMPOUND_ARR_1:
      WLZ_DBG((WLZ_DBG_ALLOC|WLZ_DBG_LVL_1),
      	      ("WlzFreeObj 15 0x%lx WLZ_COMPOUND_ARR_1\n",
	       (unsigned long )ca));
      for (i=0; i<ca->n; i++){
	if( WlzFreeObj(ca->o[i]) != WLZ_ERR_NONE ){
	  errNum = WLZ_ERR_MEM_FREE;
	}
      }
      break;

    case WLZ_COMPOUND_ARR_2:
      WLZ_DBG((WLZ_DBG_ALLOC|WLZ_DBG_LVL_1),
      	      ("WlzFreeObj 16 0x%lx WLZ_COMPOUND_ARR_2\n",
	       (unsigned long )ca));
      for (i=0; i<ca->n; i++){
	if( WlzFreeObj(ca->o[i]) != WLZ_ERR_NONE ){
	  errNum = WLZ_ERR_MEM_FREE;
	}
      }
      break;

    case WLZ_PROPERTY_OBJ:
      WLZ_DBG((WLZ_DBG_ALLOC|WLZ_DBG_LVL_1),
      	      ("WlzFreeObj 17 0x%lx WLZ_PROPERTY_OBJ\n",
	       (unsigned long )obj));
      errNum = WlzFreePropertyList(obj->plist);
      break;

    case WLZ_EMPTY_OBJ:
      WLZ_DBG((WLZ_DBG_ALLOC|WLZ_DBG_LVL_1),
      	      ("WlzFreeObj 18 0x%lx WLZ_EMPTY_OBJ\n",
	       (unsigned long )obj));
      break;

    default:
      WLZ_DBG((WLZ_DBG_ALLOC|WLZ_DBG_LVL_1),
      	      ("WlzFreeObj 18 0x%lx %d\n",
	       (unsigned long )obj, (int )(obj->type)));
      errNum = WLZ_ERR_OBJECT_TYPE;
      break;

    }    /* End of switch */

    AlcFree((void *) obj);
  }

  return( errNum );
}

/* function:     WlzFreeIntervalDomain    */
/*! 
* \ingroup      WlzAllocation
* \brief        Free an interval domain - convenience link to
 WlzFreeDomain()
*
* \return       Error number, values: from WlzFreeDomain().
* \param    idom	interval domain pointer to be freed.
* \par      Source:
*                WlzFreeSpace.c
*/
WlzErrorNum WlzFreeIntervalDomain(WlzIntervalDomain *idom)
{
  WlzDomain	domain;

  domain.i = idom;

  return( WlzFreeDomain(domain) );
}

/* function:     WlzFreeHistogramDomain    */
/*! 
* \ingroup      WlzAllocation
* \brief        Free a histogram domain.
*
* \return       Error number, values: from WlzFreeDomain().
* \param    hist	Histogram domain to be freed.
* \par      Source:
*                WlzFreeSpace.c
*/
WlzErrorNum WlzFreeHistogramDomain(WlzHistogramDomain *hist)
{
  WlzDomain	domain;

  domain.hist  = hist;

  return( WlzFreeDomain(domain) );
}

/* function:     WlzFreeDomain    */
/*! 
* \ingroup      WlzAllocation
* \brief        Free a domain structure of any type. All domain
structures must have a type, linkcount and freeptr by
which, if set, all allocated space can be freed.
*
* \return       Error number, values: WLZ_ERR_NONE, WLZ_ERR_MEM_FREE.
* \param    domain	Domain union to be freed.
* \par      Source:
*                WlzFreeSpace.c
*/
WlzErrorNum WlzFreeDomain(WlzDomain domain)
{
  WlzErrorNum errNum=WLZ_ERR_NONE;
  AlcErrno		alcErrNum=ALC_ER_NONE;

  /* check the object pointer and linkcount */
  if (domain.core == NULL){
    return( WLZ_ERR_NONE );
  }

  if( WlzUnlink(&(domain.core->linkcount), &errNum) ){

    if (domain.core->freeptr != NULL){
      alcErrNum = AlcFreeStackFree(domain.core->freeptr);
    }

    AlcFree((void *) domain.core);
  }

  if( alcErrNum != ALC_ER_NONE ){
    errNum = WLZ_ERR_MEM_FREE;
  }
  return errNum;
}

/* function:     WlzFreePlaneDomain    */
/*! 
* \ingroup      WlzAllocation
* \brief        Free a planedomain
*
* \return       Error number, values: WLZ_ERR_NONE and errors from
 WlzFreeAffineTransform(), WlzFreeDomain(), WlzFreeBoundList(),
 WlzFreePolyDmn().
* \param    planedm	Pointer to planedomain structure to be freed.
* \par      Source:
*                WlzFreeSpace.c
*/
WlzErrorNum WlzFreePlaneDomain(WlzPlaneDomain *planedm)
{
  WlzDomain	*domains;
  int 		nplanes;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  /* check the object pointer and linkcount */
  if (planedm == NULL){
    return( WLZ_ERR_NONE );
  }

  if( WlzUnlink(&(planedm->linkcount), &errNum) ){

    nplanes = planedm->lastpl - planedm->plane1 + 1;
    domains = planedm->domains;

    switch( planedm->type ){

    case WLZ_PLANEDOMAIN_DOMAIN:
      while( nplanes-- ){
	errNum |= WlzFreeDomain( *domains );
	domains++;
      }
      break;

    case WLZ_PLANEDOMAIN_POLYGON:
      while( nplanes-- ){
	errNum |= WlzFreePolyDmn((*domains).poly);
	domains++;
      }
      break;

    case WLZ_PLANEDOMAIN_BOUNDLIST:
      while( nplanes-- ){
	errNum |= WlzFreeBoundList((*domains).b);
	domains++;
      }
      break;

    case WLZ_PLANEDOMAIN_HISTOGRAM:
      while( nplanes-- ){
	errNum |= WlzFreeDomain(*domains);
	domains++;
      }
      break;

    case WLZ_PLANEDOMAIN_AFFINE:
      while( nplanes-- ){
	errNum |= WlzFreeAffineTransform((*domains).t);
	domains++;
      }
      break;

    default:
      return( WLZ_ERR_PLANEDOMAIN_TYPE );

    }
    AlcFree((void *) planedm->domains);
    AlcFree((void *) planedm);
  }

  return( errNum );
}

/* function:     WlzFreeValueTb    */
/*! 
* \ingroup      WlzAllocation
* \brief        Convenience routine to free a ragged rect valuetable.
*
* \return       Error number, values: WLZ_ERR_NONE and from WlzFreeValues().
* \param    vdmn	Value domain to be freed.
* \par      Source:
*                WlzFreeSpace.c
*/
WlzErrorNum WlzFreeValueTb(WlzRagRValues *vdmn)
{       
  WlzValues	values;

  /* check the object pointer and linkcount */
  if (vdmn == NULL){
    return( WLZ_ERR_NONE );
  }

  values.v = vdmn;

  return( WlzFreeValues( values ) );
}

/* function:     WlzFreeValues    */
/*! 
* \ingroup      WlzAllocation
* \brief        Free a values structure, currently only WlzRagRValues
and WlzRectValues DO NOT call this function with any
other values structure types!
*
* \return       Error number, values: WLZ_ERR_NONE, WLZ_ERR_VALUES_DATA.
* \param    values	Values union to be freed.
* \par      Source:
*                WlzFreeSpace.c
*/
WlzErrorNum WlzFreeValues(WlzValues values)
{
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  /* check the object pointer and linkcount */
  if (values.v == NULL){
    return( WLZ_ERR_NONE );
  }

  if( WlzUnlink(&(values.v->linkcount), &errNum) ){
			
    /* if there is a freeptr then free it */
    if (values.v->freeptr != NULL){

      /* it is illegal for a table to point to itself */
      if( values.v->original_table.v != NULL ){
	return( WLZ_ERR_VALUES_DATA );
      }
      (void )AlcFreeStackFree(values.v->freeptr);

    }
    
    if( values.v->original_table.v ){
      errNum = WlzFreeValues( values.v->original_table );
    }

    AlcFree((void *) values.v);
  }

  return( errNum );
}

/* function:     WlzFreeVoxelValueTb    */
/*! 
* \ingroup      WlzAllocation
* \brief        Free a voxel value table
*
* \return       Error number, values: WLZ_ERR_NONE,
 WLZ_ERR_VOXELVALUES_TYPE and from WlzFreeValues().
* \param    voxtab	
* \par      Source:
*                WlzFreeSpace.c
*/
WlzErrorNum WlzFreeVoxelValueTb(WlzVoxelValues *voxtab)
{
  WlzValues	*values;
  int 		nplanes;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  /* check the object pointer and linkcount */
  if (voxtab == NULL){
    return( WLZ_ERR_NONE );
  }

  /* check the type */
  if( voxtab->type != WLZ_VOXELVALUETABLE_GREY ){
    return WLZ_ERR_VOXELVALUES_TYPE;
  }

  if( WlzUnlink(&(voxtab->linkcount), &errNum) ){

    nplanes = voxtab->lastpl - voxtab->plane1 + 1;
    values = voxtab->values;
    while( nplanes-- ){
      errNum |= WlzFreeValues(*values);
      values++;
    }
    AlcFree((char *) voxtab);
  }

  return( errNum );
}

/* function:     WlzFreeConvHull    */
/*! 
* \ingroup      WlzAllocation
* \brief        Free convex hull values.
*
* \return       Error number, values: WLZ_ERR_NONE and from WlzUnlink().
* \param    c	Pointer to convex hull values to be  freed.
* \par      Source:
*                WlzFreeSpace.c
*/
WlzErrorNum WlzFreeConvHull(WlzConvHullValues *c)
{
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  /* check the object pointer and linkcount */
  if (c == NULL){
    return( WLZ_ERR_NONE );
  }

  if( WlzUnlink(&(c->linkcount), &errNum) ){
    AlcFree((void *) c);
  }

  return errNum;
}

/* function:     WlzFreePolyDmn    */
/*! 
* \ingroup      WlzAllocation
* \brief        Free a polygon domain.
*
* \return       Error number, values: WLZ_ERR_NONE and from WlzUnlink().
* \param    poly	Polygon domain to be freed.
* \par      Source:
*                WlzFreeSpace.c
*/
WlzErrorNum WlzFreePolyDmn(WlzPolygonDomain *poly)
{
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  /* check the object pointer and linkcount */
  if (poly == NULL){
    return( WLZ_ERR_NONE );
  }

  if( WlzUnlink(&(poly->linkcount), &errNum) ){
    AlcFree((void *) poly);
  }

  return errNum;
}

/* function:     WlzFreeBoundList    */
/*! 
* \ingroup      WlzAllocation
* \brief        Recursively free a boundary list.
*
* \return       Error number, values: WLZ_ERR_NONE and from WlzUnlink().
* \param    b	Boundary list structure to be freed (note this will call WlzFreeBoundList recursively).
* \par      Source:
*                WlzFreeSpace.c
*/
WlzErrorNum WlzFreeBoundList(WlzBoundList *b)
{
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  /* check the object pointer and linkcount */
  if (b == NULL){
    return( WLZ_ERR_NONE );
  }

  if( WlzUnlink(&(b->linkcount), &errNum) ){
    errNum |= WlzFreePolyDmn(b->poly);
    errNum |= WlzFreeBoundList(b->next);
    errNum |= WlzFreeBoundList(b->down);
    AlcFree((void *) b);
  }

  return( errNum );
}

/* function:     WlzFree3DWarpTrans    */
/*! 
* \ingroup      WlzAllocation
* \brief        Free a 3D warp transform.
*
* \return       Error number, values: WLZ_ERR_NONE and from WlzFreePlaneDomain().
* \param    obj	3D warp transform object to be freed.
* \par      Source:
*                WlzFreeSpace.c
*/
WlzErrorNum	WlzFree3DWarpTrans(Wlz3DWarpTrans *obj)
{
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(obj == NULL)
  {
     errNum = WLZ_ERR_OBJECT_NULL;
  }
  else
  {
    if(obj->intptdoms)
    {
      (void )AlcFree(obj->intptdoms);
    }
    if(obj->pdom)
    {
      errNum = WlzFreePlaneDomain(obj->pdom);
    }
    (void )AlcFree(obj);
  }
  return(errNum);
}

/* function:     WlzFreeContour    */
/*! 
* \ingroup      WlzAllocation
* \brief        Free's a WlzContour data structure.
*
* \return       Error number, values: WLZ_ERR_NONE, WLZ_ERR_DOMAIN_NULL, WLZ_ERR_DOMAIN_TYPE and from WlzUnlink().
* \param    ctr	Contour to be freed.
* \par      Source:
*                WlzFreeSpace.c
*/
WlzErrorNum	WlzFreeContour(WlzContour *ctr)
{
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(ctr == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else if(ctr->type != WLZ_CONTOUR)
  {
    errNum = WLZ_ERR_DOMAIN_TYPE;
  }
  else
  {
    if(WlzUnlink(&(ctr->linkcount), &errNum))
    {
      if(ctr->model && WlzUnlink(&(ctr->model->linkcount), &errNum))
      {
        (void )WlzGMModelFree(ctr->model);
      }
      AlcFree((void *)ctr);
    }
  }
  return(errNum);
}

