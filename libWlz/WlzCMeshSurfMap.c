#if defined(__GNUC__)
#ident "MRC HGU $Id$"
#else
#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma ident "MRC HGU $Id$"
#else
static char _WlzCMeshSurfMap_c[] = "MRC HGU $Id$";
#endif
#endif
/*!
* \file         WlzCMeshSurfMap.c
* \author       Bill Hill
* \date         May 2010
* \version      $Id$
* \par
* Address:
*               MRC Human Genetics Unit,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \par
* Copyright (C) 2010 Medical research Council, UK.
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
* \brief	Functions for computing surface mappings that are based
* 		on conformal transformations.
* \ingroup	WlzTransform
*/

#include<Wlz.h>
#include <stdlib.h>

static int			WlzCMeshSurfMapIdxCmpFn(
				  const void *p0,
				  const void *p1);
static WlzGMModel 		*WlzCMeshToGMModel2D(
				  WlzObject *mObj,
				  int disp,
				  WlzErrorNum *dstErr);
static WlzGMModel 		*WlzCMeshToGMModel2D5(
				  WlzObject *mObj,
				  int disp,
				  WlzErrorNum *dstErr);

/*!
* \return	New mesh object with displacements set or NULL on error.
* \ingroup	WlzTransform
* \brief	Computes a least squares conformal transformation which
* 		maps the source surface to a destination plane with z
* 		coordinate zero. See WlzCMeshCompSurfMapConformalIdx().
* \param	inObj			Input conforming mesh object which
* 					must be of type WLZ_CMESH_2D5.
* \param	nDV			Number of destination vertices.
* \param	dV			Destination vertices.
* \param	nSV			Number of destination vertices
* 					which must be the same as nDV.
* \param	sV			Source vertices.
* \param	dstErr			Woolz error code, may be NULL.
*/
WlzObject	*WlzCMeshCompSurfMapConformal(WlzObject *inObj,
				int nDV, WlzDVertex3 *dV,
				int nSV, WlzDVertex3 *sV,
				WlzErrorNum *dstErr)
{
  int		*nodTb = NULL;
  WlzCMesh2D5	*mesh;
  WlzValues	val;
  WlzObject	*rtnObj = NULL;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  val.core = NULL;
  if(inObj == NULL)
  {
    errNum = WLZ_ERR_OBJECT_NULL;
  }
  else if(inObj->type != WLZ_CMESH_2D5)
  {
    errNum = WLZ_ERR_OBJECT_TYPE;
  }
  else if(inObj->domain.core == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else if(inObj->domain.core->type != WLZ_CMESH_2D5)
  {
    errNum = WLZ_ERR_DOMAIN_TYPE;
  }
  else if((nDV < 1) || (nDV != nSV) || (dV == NULL) || (sV == NULL))
  {
    errNum = WLZ_ERR_PARAM_DATA;
  }
  else
  {
    mesh = inObj->domain.cm2d5;
    if((mesh->res.nod.numEnt < 3) || (mesh->res.elm.numEnt < 1))
    {
      errNum = WLZ_ERR_DOMAIN_DATA;
    }
    else if((nodTb = (int *)AlcMalloc(sizeof(int) * nSV)) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
    else
    {
      if(WlzCMeshMatchNNodIdx2D5(mesh, nSV, sV, nodTb) != nDV)
      {
        errNum = WLZ_ERR_DOMAIN_DATA;
      }
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    rtnObj = WlzCMeshCompSurfMapConformalIdx(mesh, nDV, dV, nodTb, &errNum);
  }
  AlcFree(nodTb);
  if(dstErr != NULL)
  {
    *dstErr = errNum;
  }
  return(rtnObj);
}

/*!
* \return	New mesh object with displacements set or NULL on error.
* \ingroup	WlzTransform
* \brief	Computes a least squares conformal transformation which
* 		maps the source surface to a destination plane with z
* 		coordinate zero.
* 		TODO
* \param	mesh			Input conforming mesh which must be
* 					of type WLZ_CMESH_2D5.
* \param	nPN			Number of pinned nodes.
* \param	dPV			Destination coordinates of the
* 					pinned nodes. All z components
* 					should be equal and usualy set to
* 					zero. The coordinates must correspond
* 					to the indices of pIdx.
* \param	pIdx			Indices of the pinned nodes
* 					which must all be valid. The indices
* 					must be sorted soo that the indices
* 					increase monotonically.
* \param	dstErr			Woolz error code, may be NULL.
*/
WlzObject	*WlzCMeshCompSurfMapConformalIdx(WlzCMesh2D5 *mesh,
				int nP, WlzDVertex3 *dPV, int *pIdx,
				WlzErrorNum *dstErr)
{
  int		mE,
  		nE,
		mN,
		nN;
  int		*pIdxSortTb = NULL,
     		*pIdxIdxTb = NULL,
  		*eIdxTb = NULL,
  		*nIdxTb = NULL,
		*pIdxSorted = NULL;
  double	*bM = NULL,
  		*bUM = NULL,
		*xM = NULL;
  double	**aM = NULL,
  		**bPM = NULL;
  WlzObject	*rtnObj = NULL;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(mesh == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else if(mesh->type != WLZ_CMESH_2D5)
  {
    errNum = WLZ_ERR_DOMAIN_TYPE;
  }
  else if((nP < 2) || (mesh->res.nod.numEnt < nP) ||
          (mesh->res.elm.numEnt < 1))
  {
    errNum = WLZ_ERR_DOMAIN_DATA;
  }
  /* Allocate matrices, element and node index look up tables. */
  if(errNum == WLZ_ERR_NONE)
  {
    mE = mesh->res.elm.maxEnt;
    mN = mesh->res.nod.maxEnt;
    nN = mesh->res.nod.numEnt;
    nE = mesh->res.elm.numEnt;
    if((AlcDouble2Calloc(&aM, nE * 2, nN * 2) != ALC_ER_NONE) ||
       (AlcDouble2Calloc(&bPM, nE * 2, nP * 2) != ALC_ER_NONE) ||
       ((bUM = AlcMalloc(sizeof(double) * nP * 2)) == NULL) ||
       ((bM = AlcMalloc(sizeof(double) * nE * 2)) == NULL) ||
       ((xM = AlcMalloc(sizeof(double) * nN * 2)) == NULL) ||
       ((pIdxSortTb = AlcMalloc(sizeof(int) * nP)) == NULL) ||
       ((pIdxIdxTb = AlcMalloc(sizeof(int) * mN)) == NULL) ||
       ((pIdxSorted = AlcMalloc(sizeof(int) * nP)) == NULL) ||
       ((eIdxTb = AlcMalloc(sizeof(int) * mE)) == NULL) ||
       ((nIdxTb = AlcMalloc(sizeof(int) * mN)) == NULL))
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    int		idE,
    		idN;

    for(idN = 0; idN < nP; ++idN)
    {
      pIdxIdxTb[pIdx[idN]] = idN;    /* LUT from node index to given arrays. */
      pIdxSortTb[idN] = idN;  /* LUT for pinned nodes sorted by index value. */
    }
    (void )AlgHeapSortIdx(pIdx, pIdxSortTb, nP, AlgHeapSortCmpIdxIFn);
    for(idN = 0; idN < nP; ++idN)
    {
      pIdxSorted[idN] = pIdx[pIdxSortTb[idN]];
    }
    /* Fill in the element and node index tables. */
    (void )WlzCMeshSetNodIdxTbl2D5(mesh, nIdxTb);
    (void )WlzCMeshSetElmIdxTbl2D5(mesh, eIdxTb);
    /* Compute matrix values. */
    for(idE = 0; idE < mesh->res.elm.maxEnt; ++idE)
    {
      WlzCMeshElm2D5 *elm;

      elm = (WlzCMeshElm2D5 *)AlcVectorItemGet(mesh->res.elm.vec, idE);
      if(elm->idx >= 0)
      {
        int  	idN;
	double	a2;
	WlzDVertex3 v[3],
		    p[3];
	WlzCMeshNod2D5 *nod[3];

	/* Get element vertives. */
	nod[0] = WLZ_CMESH_ELM2D5_GET_NODE_0(elm); p[0] = nod[0]->pos;
	nod[1] = WLZ_CMESH_ELM2D5_GET_NODE_1(elm); p[1] = nod[1]->pos;
	nod[2] = WLZ_CMESH_ELM2D5_GET_NODE_2(elm); p[2] = nod[2]->pos;
	/* Compute useful vectors and lengths. */
	WLZ_VTX_3_SUB(v[0], p[1], p[0]);
	WLZ_VTX_3_SUB(v[1], p[2], p[0]);
	WLZ_VTX_3_CROSS(v[2], v[0], v[1]);
	a2 = WLZ_VTX_3_LENGTH(v[2]);
	if(a2 < WLZ_MESH_TOLERANCE)
	{
	  /* This should never occur! */
	  errNum = WLZ_ERR_DOMAIN_DATA;
	  break;
	}
	else
	{
	  int    idT;
	  double d,
	  	 l0,
	  	 l2;
	  double wR[3],
		 wI[3];
	  WlzDVertex2 q2;
	  WlzDVertex3 u[3]; /* Basis vectors within the plane of the current
	                       triangular element. */

	  d = 1.0 / sqrt(a2);
	  l0 = WLZ_VTX_3_LENGTH(v[0]);
	  l2 = a2; /* WLZ_VTX_3_LENGTH(v[2]) */
	  idT = eIdxTb[elm->idx];
	  /* Compute the orthonormal basis vectors for this element. */
	  WLZ_VTX_3_SCALE(u[0], v[0], 1.0 / l0);
	  WLZ_VTX_3_SCALE(u[2], v[2], 1.0 / l2);
	  WLZ_VTX_3_CROSS(u[1], u[2], u[0]);
	  q2.vtX = WLZ_VTX_3_DOT(v[1], u[0]);
	  q2.vtY = WLZ_VTX_3_DOT(v[1], u[1]);
	  wR[0] = d * (q2.vtX - l0);
	  wI[0] = d * q2.vtY; 
	  wR[1] = d * -q2.vtX;
	  wI[1] = d * -q2.vtY;
	  wR[2] = d * l0;
	  wI[2] = 0.0;
	  for(idN = 0; idN < 3; ++idN)
	  {
	    int idV;
	    int *idPP;

	    idV = nIdxTb[nod[idN]->idx];
	    if((idPP = bsearch(&(nod[idN]->idx), pIdxSorted, nP, sizeof(int),
	                       WlzCMeshSurfMapIdxCmpFn)) != NULL)
	    {
	      int idQ,
	      	  idP;

	      /* Node is pinned. */
	      idQ = idPP - pIdxSorted;      /* Index into table pinned node. */
	      idP = pIdxIdxTb[nod[idN]->idx];
	      bPM[idT     ][idQ     ] =  wR[idN];
	      bPM[idT + nE][idQ     ] = -wI[idN];
	      bPM[idT     ][idQ + nP] =  wI[idN];
	      bPM[idT + nE][idQ + nP] =  wR[idN];
	      bUM[idQ     ] = dPV[idP].vtX;
	      bUM[idQ + nP] = dPV[idP].vtY;
	    }
	    else
	    {
	      /* Node is free. */
	      aM[idT     ][idV     ] =  wR[idN];
	      aM[idT + nE][idV     ] = -wI[idN];
	      aM[idT     ][idV + nN] =  wI[idN];
	      aM[idT + nE][idV + nN] =  wR[idN];
	    }
	  }
	}
      }
    }
  }
  AlcFree(pIdxSortTb);
  AlcFree(pIdxIdxTb);
  /* Compute bM and solve for mapped vertices. */
  if(errNum == WLZ_ERR_NONE)
  {
    AlgMatrixVectorMul(bM, ALG_MATRIX_RECT, bPM, bUM, nE * 2, nP * 2);
    /* AlgMatrixScale(&bM, &bM, -1.0, 1, nE * 2); */
#ifdef WLZ_CMESH_SM_DEBUG
    (void )fprintf(stderr, "WlzCMeshCompSurfMapConformalIdx() aM =\n[\n");
    (void )AlcDouble2WriteAsci(stderr, aM, nE * 2, nN * 2);
    (void )fprintf(stderr, "]\n");
    (void )fprintf(stderr, "WlzCMeshCompSurfMapConformalIdx() bM =\n[\n");
    (void )AlcDouble1WriteAsci(stderr, bM, nE * 2);
    (void )fprintf(stderr, "]\n");
#endif 
    errNum = WlzErrorFromAlg(
	     AlgMatrixSolveLSQR(ALG_MATRIX_RECT, aM, nE * 2, nN * 2, bM, xM,
	                        1.0e-10, 1.0e-10, 1.0e-10, 1000, 0,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL));
  }
  AlcFree(bUM);
  AlcFree(eIdxTb);
  Alc2Free((void **)bPM);
  Alc2Free((void **)aM);
  /* Create return object. */
  if(errNum == WLZ_ERR_NONE)
  {
    WlzDomain	dom;
    WlzValues	val;

#ifdef WLZ_CMESH_SM_DEBUG
    (void )fprintf(stderr, "WlzCMeshCompSurfMapConformalIdx() xM =\n[\n");
    (void )AlcDouble1WriteAsci(stderr, xM, nN * 2);
    (void )fprintf(stderr, "]\n");
#endif 
    dom.cm2d5 = mesh;
    val.core = NULL;
    rtnObj = WlzMakeMain(WLZ_CMESH_2D5, dom, val, NULL, NULL, &errNum);
  }
  /* Allocate indexed values. */
  if(errNum == WLZ_ERR_NONE)
  {
    int		dim = 3;
    WlzValues	val;

    val.x = WlzMakeIndexedValues(rtnObj, 1, &dim, WLZ_GREY_DOUBLE,
                                 WLZ_VALUE_ATTACH_NOD, &errNum);
    rtnObj->values = WlzAssignValues(val, NULL);
  }
  /* Set displacements for the indexed values. */
  if(errNum == WLZ_ERR_NONE)
  {
    int		idN;
    double 	*dsp;
    WlzIndexedValues *ixv;

    ixv = rtnObj->values.x;
    for(idN = 0; idN < mesh->res.nod.maxEnt; ++idN)
    {
      WlzCMeshNod2D5 *nod;

      nod = (WlzCMeshNod2D5 *)AlcVectorItemGet(mesh->res.nod.vec, idN);
      if(nod->idx >= 0)
      {
	int	idV;

	dsp = (double *)WlzIndexedValueGet(ixv, nod->idx);
	idV = nIdxTb[nod->idx];
	if(bsearch(&(nod->idx), pIdxSorted, nP, sizeof(int),
	           WlzCMeshSurfMapIdxCmpFn) != NULL)
	{
	  dsp[0] = 0.0;
	  dsp[1] = 0.0;
	}
	else
	{
	  dsp[0] = -(xM[idV     ] + nod->pos.vtX);
	  dsp[1] = -(xM[idV + nN] + nod->pos.vtY);
	}
	dsp[2] = -(nod->pos.vtZ);
      }
    }
  }
  AlcFree(xM);
  AlcFree(bM);
  AlcFree(nIdxTb);
  AlcFree(pIdxSorted);
  if(dstErr != NULL)
  {
    *dstErr = errNum;
  }
  return(rtnObj);
}

/*!
* \return	Comparison value for qsort().
* \ingroup	WlzMesh
* \brief	Called by qsort() to sort integers into acending order.
* \param	p0			Pointer to first int.
* \param	p1			Pointer to second int.
*/
static int	WlzCMeshSurfMapIdxCmpFn(const void *p0, const void *p1)
{
  int		*i0,
  		*i1;

  i0 = (int *)p0;
  i1 = (int *)p1;
  return(*i0 - *i1);
}

/*!
* \return       New contour object corresponding to the given mesh.
* \ingroup      WlzMesh
* \brief        Creates a contour corresponding to the given conforming mesh
*               which must be a 2D5 mesh, ie a surface.
* \param        mObj                    Given conforming mesh.
* \param        disp                    Non zero if the mesh displacements
*                                       are to be applied.
* \param        dstErr                  Destination error pointer, may be NULL.
*/
WlzObject       *WlzCMeshToContour(WlzObject *mObj, int disp,
                                   WlzErrorNum *dstErr)
{
  WlzDomain dom;
  WlzValues val;
  WlzObject     *cObj = NULL;
  WlzErrorNum   errNum = WLZ_ERR_NONE;

  dom.core = NULL;
  val.core = NULL;
  if(mObj == NULL)
  {
    errNum = WLZ_ERR_OBJECT_NULL;
  }
  else if(mObj->domain.core == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    dom.ctr = WlzMakeContour(&errNum);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    dom.ctr->model = WlzAssignGMModel(WlzCMeshToGMModel(mObj, disp, &errNum),
    			              NULL);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    cObj = WlzMakeMain(WLZ_CONTOUR, dom, val, NULL, NULL, &errNum);
  }
  if(errNum != WLZ_ERR_NONE)
  {
    (void )WlzFreeContour(dom.ctr);
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(cObj);
}

/*!
* \return       New geometric model corresponding to the given mesh.
* \ingroup      WlzMesh
* \brief        Creates a geometric model corresponding to the given
* 		conforming mesh which must be either a 2D or 2D5 mesh,
* 		ie a surface. The resulting model will have either
* 		have type WLZ_GMMOD_3D (from 2D5) or WLZ_GMMOD_2D (from
* 		2D).
* \param        mObj                    Given conforming mesh.
* \param        disp                    Non zero if the mesh displacements
*                                       are to be applied.
* \param        dstErr                  Destination error pointer, may be NULL.
*/
WlzGMModel	*WlzCMeshToGMModel(WlzObject *mObj, int disp,
				   WlzErrorNum *dstErr)
{
  WlzGMModel	*model = NULL;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(mObj == NULL)
  {
    errNum = WLZ_ERR_OBJECT_NULL;
  }
  else if(mObj->domain.core == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else if(disp != 0)
  {
    if(mObj->values.core == NULL)
    {
      errNum = WLZ_ERR_VALUES_NULL;
    }
    else if(mObj->values.core->type != WLZ_INDEXED_VALUES)
    {
      errNum = WLZ_ERR_VALUES_TYPE;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    switch(mObj->type)
    {
      case WLZ_CMESH_2D:
	model = WlzCMeshToGMModel2D(mObj, disp, &errNum);
        break;
      case WLZ_CMESH_2D5:
	model = WlzCMeshToGMModel2D5(mObj, disp, &errNum);
        break;
      default:
        errNum = WLZ_ERR_OBJECT_TYPE;
	break;
    }
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(model);
}

/*!
* \return	New 2D conforming mesh object.
* \ingroup	WlzMesh
* \brief	Creates a new 2D conforming mesh object by flattening the given
* 		2D5 conforming mesh object. This is done by applying the
* 		2D5 object's indexed values which are assumed to be valid
* 		displacements to a plane. See WlzCMeshCompSurfMapConformal().
* \param	gObj			Given 2D5 conforming mesh object
* 					with valid displacements.
* \param	dstErr			Destination error pointer, may be NULL.
*/
WlzObject	*WlzCMeshFlatten2D5(WlzObject *gObj, WlzErrorNum *dstErr)
{
  WlzCMesh2D5	*gMesh;
  WlzCMesh2D	*rMesh = NULL;
  WlzObject	*rObj = NULL;
  WlzIndexedValues *gIxv;
  WlzErrorNum   errNum = WLZ_ERR_NONE;

  if(gObj == NULL)
  {
    errNum = WLZ_ERR_OBJECT_NULL;
  }
  else if(gObj->type != WLZ_CMESH_2D5)
  {
    errNum = WLZ_ERR_OBJECT_TYPE;
  }
  else if((gMesh = gObj->domain.cm2d5) == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  if((gMesh->res.nod.numEnt < 3) || (gMesh->res.elm.numEnt < 1))
  {
    errNum = WLZ_ERR_DOMAIN_DATA;
  }
  else if((gIxv = gObj->values.x) == NULL)
  {
    errNum = WLZ_ERR_VALUES_NULL;
  }
  else if((gIxv->rank != 1) || (gIxv->dim[0] < 2))
  {
    errNum = WLZ_ERR_VALUES_DATA;
  }
  else
  {
    rMesh = WlzCMeshNew2D(&errNum);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    if((AlcVectorExtendAndGet(rMesh->res.nod.vec,
                              gMesh->res.nod.maxEnt) == NULL) ||
       (AlcVectorExtendAndGet(rMesh->res.elm.vec,
                              gMesh->res.elm.maxEnt) == NULL))
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if(errNum == WLZ_ERR_NONE)                
  {
    int	idN;

    for(idN = 0; idN < gMesh->res.nod.maxEnt; ++idN)
    {
      WlzCMeshNod2D  *rNod;
      WlzCMeshNod2D5 *gNod;

      gNod = (WlzCMeshNod2D5 *)AlcVectorItemGet(gMesh->res.nod.vec, idN);
      if(gNod->idx >= 0)
      {
	double	*dsp;
	WlzDVertex2 pos;

	dsp = (double *)WlzIndexedValueGet(gIxv, gNod->idx);
	pos.vtX = gNod->pos.vtX + dsp[0];
	pos.vtY = gNod->pos.vtY + dsp[1];
        rNod = WlzCMeshNewNod2D(rMesh, pos, NULL);
      }
      else
      {
	/* Insert matching invalid mesh node. */
        rNod = (WlzCMeshNod2D *)AlcVectorItemGet(rMesh->res.nod.vec, idN);
	rNod->idx = -1;
	++(rMesh->res.nod.nextIdx);
      }
    }
    WlzCMeshUpdateBBox2D(rMesh);
    errNum = WlzCMeshReassignGridCells2D(rMesh, rMesh->res.nod.numEnt);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    int	idE;

    for(idE = 0; idE < gMesh->res.elm.maxEnt; ++idE)
    {
      WlzCMeshElm2D *rElm;
      WlzCMeshElm2D5 *gElm;

      gElm = (WlzCMeshElm2D5 *)AlcVectorItemGet(gMesh->res.elm.vec, idE);
      if(gElm->idx >= 0)
      {
	int	idN;
        WlzCMeshNod2D *nod[3];

	for(idN = 0; idN < 3; ++idN)
	{
	  nod[idN] = (WlzCMeshNod2D *)AlcVectorItemGet(rMesh->res.nod.vec,
						       gElm->edu[idN].nod->idx);
	}
	(void )WlzCMeshNewElm2D(rMesh, nod[0], nod[1], nod[2], 1, &errNum);
	if(errNum != WLZ_ERR_NONE)
	{
	  break;
	}
      }
      else
      {
	/* Insert matching invalid mesh element. */
        rElm = (WlzCMeshElm2D *)AlcVectorItemGet(rMesh->res.elm.vec, idE);
	rElm->idx = -1;
	++(rMesh->res.elm.nextIdx);
      }
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    errNum = WlzCMeshReassignGridCells2D(rMesh, rMesh->res.nod.numEnt);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    WlzDomain dom;
    WlzValues val;

    dom.cm2 = rMesh;
    val.core = NULL;
    WlzCMeshUpdateMaxSqEdgLen2D(rMesh);
    rObj = WlzMakeMain(WLZ_CMESH_2D, dom, val, NULL, NULL, &errNum);
  }
  if(errNum != WLZ_ERR_NONE)
  {
    if(rObj != NULL)
    {
      (void )WlzFreeObj(rObj);
      rObj = NULL;
    }
    else if(rMesh != NULL)
    {
      (void )WlzCMeshFree2D(rMesh);
      rMesh = NULL;
    }
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(rObj);
}

/*!
* \return       New geometric model corresponding to the given mesh.
* \ingroup      WlzMesh
* \brief        Creates a geometric model corresponding to the given
* 		conforming mesh which is assumed to be a 2D mesh.
* 		The resulting model will be a WLZ_GMMOD_2D model.
* \param        mObj                    Given conforming mesh.
* \param        disp                    Non zero if the mesh displacements
*                                       are to be applied.
* \param        dstErr                  Destination error pointer, may be NULL.
*/
static WlzGMModel *WlzCMeshToGMModel2D(WlzObject *mObj, int disp,
				       WlzErrorNum *dstErr)
{
  int		nBkSz,
  		nHTSz,
		nNod = 0;
  WlzCMesh2D	*mesh;
  WlzGMModel	*model = NULL;
  WlzIndexedValues *ixv;
  WlzErrorNum	errNum = WLZ_ERR_NONE;
  const int	minBkSz = 1024,
  		minHTSz = 1024;

  mesh = mObj->domain.cm2;
  if(((nNod = mesh->res.nod.numEnt) < 3) || (mesh->res.elm.numEnt < 1))
  {
    errNum = WLZ_ERR_DOMAIN_DATA;
  }
  else if(disp != 0)
  {
    if((ixv = mObj->values.x) == NULL)
    {
      errNum = WLZ_ERR_VALUES_NULL;
    }
    else if((ixv->rank != 1) || (ixv->dim[0] < 2))
    {
      errNum = WLZ_ERR_VALUES_DATA;
    }
  }
  /* Create a new geometric model. */
  if(errNum == WLZ_ERR_NONE)        
  {
    if((nBkSz = nNod / 16) < minBkSz)
    {
      nBkSz = minBkSz;
    }
    if((nHTSz = nNod / 4) < minHTSz)
    {
      nHTSz = minHTSz;
    }
    model = WlzGMModelNew(WLZ_GMMOD_2D, nBkSz, nHTSz, &errNum);
  }
  /* Add the simplices (edges) to the model. */
  if(errNum == WLZ_ERR_NONE)
  {
    int		idE;

    for(idE = 0; idE < mesh->res.elm.maxEnt; ++idE)
    {
      WlzCMeshElm2D *elm;
      
      elm = (WlzCMeshElm2D *)AlcVectorItemGet(mesh->res.elm.vec, idE);
      if(elm->idx >= 0)
      {
	int	idN;
	WlzDVertex2 pos[4];

	for(idN = 0; idN < 3; ++idN)
	{
	  pos[idN] = elm->edu[idN].nod->pos;
	  if(disp)
	  {
	    double *dsp;
	    
	    dsp = (double *)WlzIndexedValueGet(ixv, elm->edu[idN].nod->idx);
	    pos[idN].vtX += dsp[0];
	    pos[idN].vtY += dsp[1];
	  }
	}
	pos[3] = pos[0];
	for(idN = 0; idN < 3; ++idN)  
	{
	  errNum = WlzGMModelConstructSimplex2D(model, pos + idN);
	  if(errNum != WLZ_ERR_NONE)
	  {
	    break;
	  }
	}
	if(errNum != WLZ_ERR_NONE)
	{
	  break;
	}
      }
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    errNum = WlzGMModelRehashVHT(model, 0);
  }
  if(errNum != WLZ_ERR_NONE)
  {
    (void )WlzGMModelFree(model);
    model = NULL;
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(model);
}

/*!
* \return       New geometric model corresponding to the given mesh.
* \ingroup      WlzMesh
* \brief        Creates a geometric model corresponding to the given
* 		conforming mesh which is assumed to be a 2D5 mesh.
* 		The resulting model will be a WLZ_GMMOD_3D model.
* \param        mObj                    Given conforming mesh.
* \param        disp                    Non zero if the mesh displacements
*                                       are to be applied.
* \param        dstErr                  Destination error pointer, may be NULL.
*/
static WlzGMModel *WlzCMeshToGMModel2D5(WlzObject *mObj, int disp,
				        WlzErrorNum *dstErr)
{
  int		nBkSz,
  		nHTSz,
		nNod = 0;
  WlzCMesh2D5	*mesh;
  WlzGMModel	*model = NULL;
  WlzIndexedValues *ixv;
  WlzErrorNum	errNum = WLZ_ERR_NONE;
  const int	minBkSz = 1024,
  		minHTSz = 1024;

  ixv = mObj->values.x;
  mesh = mObj->domain.cm2d5;
  if(((nNod = mesh->res.nod.numEnt) < 3) || (mesh->res.elm.numEnt < 1))
  {
    errNum = WLZ_ERR_DOMAIN_DATA;
  }
  else if(ixv == NULL)
  {
    disp = 0;
  }
  else if((ixv->rank != 1) || (ixv->dim[0] < 3))
  {
    errNum = WLZ_ERR_VALUES_DATA;
  }
  /* Create a new geometric model. */
  if(errNum == WLZ_ERR_NONE)        
  {
    if((nBkSz = nNod / 16) < minBkSz)
    {
      nBkSz = minBkSz;
    }
    if((nHTSz = nNod / 4) < minHTSz)
    {
      nHTSz = minHTSz;
    }
    model = WlzGMModelNew(WLZ_GMMOD_3D, nBkSz, nHTSz, &errNum);
  }
  /* Add the simplices (faces) to the model. */
  if(errNum == WLZ_ERR_NONE)
  {
    int		idE;

    for(idE = 0; idE < mesh->res.elm.maxEnt; ++idE)
    {
      WlzCMeshElm2D5 *elm;
      
      elm = (WlzCMeshElm2D5 *)AlcVectorItemGet(mesh->res.elm.vec, idE);
      if(elm->idx >= 0)
      {
	int	idN;
	WlzDVertex3 pos[3];

	for(idN = 0; idN < 3; ++idN)
	{
	  pos[idN] = elm->edu[idN].nod->pos;
	}
	if(disp)
	{
	  for(idN = 0; idN < 3; ++idN)
	  {
	    double *dsp;
	    
	    dsp = (double *)WlzIndexedValueGet(ixv, elm->edu[idN].nod->idx);
	    pos[idN].vtX += dsp[0];
	    pos[idN].vtY += dsp[1];
	    pos[idN].vtZ += dsp[2];
	  }
	}
        errNum = WlzGMModelConstructSimplex3D(model, pos);
	if(errNum != WLZ_ERR_NONE)
	{
	  break;
	}
      }
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    errNum = WlzGMModelRehashVHT(model, 0);
  }
  if(errNum != WLZ_ERR_NONE)
  {
    (void )WlzGMModelFree(model);
    model = NULL;
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(model);
}