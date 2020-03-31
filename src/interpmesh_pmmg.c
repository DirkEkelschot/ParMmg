/* =============================================================================
**  This file is part of the parmmg software package for parallel tetrahedral
**  mesh modification.
**  Copyright (c) Bx INP/Inria/UBordeaux, 2017-
**
**  parmmg is free software: you can redistribute it and/or modify it
**  under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  parmmg is distributed in the hope that it will be useful, but WITHOUT
**  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
**  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
**  License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License and of the GNU General Public License along with parmmg (in
**  files COPYING.LESSER and COPYING). If not, see
**  <http://www.gnu.org/licenses/>. Please read their terms carefully and
**  use this copy of the parmmg distribution only if you accept them.
** =============================================================================
*/

/**
 * \file interpmesh_pmmg.c
 * \brief Interpolate data from a background mesh to the current mesh.
 * \author Cécile Dobrzynski (Bx INP/Inria)
 * \author Algiane Froehly (Inria)
 * \author Nikos Pattakos (Inria)
 * \author Luca Cirrottola (Inria)
 * \version 1
 * \copyright GNU Lesser General Public License.
 */
#include "parmmg.h"
#include "locate_pmmg.h"

/**
 * \param coord pointer to a double array
 * \param barycoord pointer to the point barycentric coordinates in the current
 * tetra
 *
 *  Get barycentric coordinates.
 *
 */
void PMMG_get_baryCoord( double *val, PMMG_baryCoord *phi ) {
  int i;

  for( i = 0; i < 4; i++ )
    val[phi[i].idx] = phi[i].val;

}

/**
 * \param minNew lower bounds of the new box in each space direction
 * \param maxNew upper bounds of the new box in each space direction
 * \param minOld lower bounds of the old box in each space direction
 * \param maxOld upper bounds of the old box in each space direction
 *
 * \return 0 if not intersectiong, 1 otherwise
 *
 *  Check that the intersection of two bounding boxes is not empty.
 *
 */
int PMMG_intersect_boundingBox( double *minNew, double *maxNew,
                                double *minOld, double *maxOld ) {
  int idim;

  for( idim = 0; idim < 3; idim++ ) {
    if( maxNew[idim] <= minOld[idim] )
      return 0;
    if( maxOld[idim] <= minNew[idim] )
      return 0;
  }

  return 1;
}

/**
 * \param mesh pointer to the mesh structure
 * \param pt pointer to the current tetra
 * \param coord pointer to the point coordinates
 * \param faceAreas oriented face areas of the current tetrahedron
 * \param barycoord pointer to the point barycentric coordinates in the current
 * tetra
 *
 * \return 0 if fail, 1 if success
 *
 *  Compute the barycentric coordinates of a given point in a given tetrahedron.
 *
 */
int PMMG_compute_baryCoord( MMG5_pMesh mesh, MMG5_pTetra pt,
                            double *coord, double *faceAreas, PMMG_baryCoord *barycoord ) {
  double *c0,*normal,vol;
  int    ifac;

  /* Retrieve tetra volume */
  vol = pt->qual;

  /* Retrieve face areas and compute barycentric coordinates */
  for( ifac = 0; ifac < 4; ifac++ ) {
    normal = &faceAreas[3*ifac];
    c0 = mesh->point[pt->v[MMG5_idir[ifac][0]]].c;
    barycoord[ifac].val = -( (coord[0]-c0[0])*normal[0] +
                             (coord[1]-c0[1])*normal[1] +
                             (coord[2]-c0[2])*normal[2] )/vol;
    barycoord[ifac].idx = ifac;
  }

  return 1;
}

/**
 * \param a pointer to point barycentric coordinates
 * \param b pointer to point barycentric coordinates
 *
 * \return -1 if (a < b), +1 if (a > b), 0 if equal
 *
 *  Compare the barycentric coordinates of a given point in a given tetrahedron.
 *
 */
int PMMG_compare_baryCoord( const void *a,const void *b ) {
  PMMG_baryCoord *coord_a;
  PMMG_baryCoord *coord_b;

  coord_a = (PMMG_baryCoord *)a;
  coord_b = (PMMG_baryCoord *)b;

  if( coord_a->val > coord_b-> val ) return 1;
  if( coord_a->val < coord_b-> val ) return -1;

  return 0;
}

/**
 * \param mesh pointer to the background mesh structure
 * \param ptr pointer to the tetra to analyze
 * \param ppt pointer to the point to locate
 * \param faceAreas oriented face areas of the current tetrahedra
 * \param barycoord barycentric coordinates of the point to be located
 *
 * \return 1 if found; 0 if not found
 *
 *  Locate a point in a background tetrahedron, and provide its barycentric
 *  coordinates..
 *
 */
int PMMG_locatePointInTetra( MMG5_pMesh mesh, MMG5_pTetra ptr, MMG5_pPoint ppt,
                             double *faceAreas, PMMG_baryCoord *barycoord ) {
  double         vol,eps;
  int            found = 0;

  vol = ptr->qual;
  eps = MMG5_EPS;

  /** Mark tetra */
  ptr->flag = mesh->base;

  /** Get barycentric coordinates and sort them in ascending order */
  PMMG_compute_baryCoord(mesh, ptr, ppt->c, faceAreas, barycoord);
  qsort(barycoord,4,sizeof(PMMG_baryCoord),PMMG_compare_baryCoord);

  /** Exit if inside the element */
  if( barycoord[0].val > -eps ) found = 1;

  return found;
}

/**
 * \param mesh pointer to the background mesh structure
 * \param ppt pointer to the point to locate
 * \param init index of the starting element
 * \param faceAreas oriented face areas of the all tetrahedra in the mesh
 * \param barycoord barycentric coordinates of the point to be located
 *
 * \return ie if positive, index of the target element; if negative, index of
 * the closest element; 0 if not found
 *
 *  Locate a point in a background mesh by traveling the elements adjacency.
 *
 */
int PMMG_locatePoint( MMG5_pMesh mesh, MMG5_pPoint ppt, int init,
                      double *faceAreas, PMMG_baryCoord *barycoord ) {
  MMG5_pTetra    ptr,pt1;
  int            *adja,iel,i,idxTet,step,closestTet;
  double         vol,eps,closestDist;
  static int     mmgWarn0=0,mmgWarn1=0;

  if(!init)
    idxTet = 1;
  else
    idxTet = init;

  step = 0;
  ++mesh->base;
  while(step <= mesh->ne) {
    step++;

    /** Get tetra */
    ptr = &mesh->tetra[idxTet];
    if ( !MG_EOK(ptr) ) continue;

    /** Exit the loop if you find the element */
    if( PMMG_locatePointInTetra( mesh, ptr, ppt,&faceAreas[12*idxTet],
                                 barycoord ) ) break;

    /** Compute new direction */
    adja = &mesh->adja[4*(idxTet-1)+1];
    for( i=0; i<4; i++ ) {
      iel = adja[barycoord[i].idx]/4;

      /* Skip if on boundary */
      if (!iel) continue;

      /* Skip if already marked */
      pt1 = &mesh->tetra[iel];
      if(pt1->flag == mesh->base) continue;

      /* Get next otherwise */
      idxTet = iel;
      break;
    }

    /** Stuck: Start exhaustive research */
    if (i == 4) step = mesh->ne+1;

  }

  /** Boundary hit or cyclic path: Perform exhaustive research */
  if( step == (mesh->ne+1) ) {
    if ( !mmgWarn0 ) {
      mmgWarn0 = 1;
      if ( mesh->info.imprim > PMMG_VERB_DETQUAL ) {
        fprintf(stderr,"\n  ## Warning %s: Cannot locate point,"
                " performing exhaustive research.\n",__func__);
      }
    }

    closestTet = 0;
    closestDist = 1.0e10;
    for( idxTet=1; idxTet<mesh->ne+1; idxTet++ ) {

      /** Get tetra */
      ptr = &mesh->tetra[idxTet];
      if ( !MG_EOK(ptr) ) continue;

      /*¨Skip already analized tetras */
      if( ptr->flag == mesh->base ) continue;

      /** Exit the loop if you find the element */
      if( PMMG_locatePointInTetra( mesh, ptr, ppt,&faceAreas[12*idxTet],
                                   barycoord ) ) break;

      /** Save element index (with negative sign) if it is the closest one */
      vol = ptr->qual;
      if( fabs(barycoord[0].val)*vol < closestDist ) {
        closestDist = fabs(barycoord[0].val)*vol;
        closestTet = -idxTet;
      }

    }

    /** Element not found: Return the closest one with negative sign (if found) */
    if ( idxTet == mesh->ne+1 ) {
      if ( !mmgWarn1 ) {
        mmgWarn1 = 1;
        if ( mesh->info.imprim > PMMG_VERB_VERSION ) {
          fprintf(stderr,"\n  ## Warning %s: Point not located, smallest external volume %e.",
                  __func__,closestDist);
        }
      }
      return closestTet;
    }

  }
  return idxTet;
}

/**
 * \param mesh pointer to the current mesh
 * \param met pointer to the current metrics
 * \param oldMet pointer to the background metrics
 * \param pt pointer to the target background tetrahedron
 * \param ip index of the current point
 * \param phi barycentric coordinates of the point to be interpolated
 *
 * \return 0 if fail, 1 if success
 *
 *  Linearly interpolate point data (metric or solution) on a target background
 *  tetrahedron. This function is analogous to the MMG5_interp4bar_iso()
 *  function.
 */
int PMMG_interp4bar_iso( MMG5_pMesh mesh,MMG5_pSol met,MMG5_pSol oldMet,
                         MMG5_pTetra pt,int ip,double *phi ) {
  int iloc,i,j,ier,iadr;

  assert( met->size == 1 );

  /** Linear interpolation of the squared size */
  iadr = met->size *ip;

  for ( j=0; j<met->size; ++j ) {
    met->m [ iadr + j ] = 0.0;
  }

  for( i=0; i<4; i++ ) {
    for ( j=0; j<met->size; ++j ) {
      /* Barycentric coordinates could be permuted */
      met->m[iadr+j] += phi[i]*oldMet->m[pt->v[i]*met->size+j];
    }
  }

  return 1;
}

/**
 * \param mesh pointer to the current mesh
 * \param met pointer to the current metrics
 * \param oldMet pointer to the background metrics
 * \param pt pointer to the target background tetrahedron
 * \param ip index of the current point
 * \param phi barycentric coordinates of the point to be interpolated
 *
 * \return 0 if fail, 1 if success
 *
 *  Linearly interpolate point the metrics inverse on a target background
 *  tetrahedron.
 *  This function is analogous to the MMG5_interp4barintern() function.
 *
 */
int PMMG_interp4bar_ani( MMG5_pMesh mesh,MMG5_pSol met,MMG5_pSol oldMet,
                         MMG5_pTetra pt,int ip,double *phi ) {
  double dm[4][6],mi[4][6],m[6];
  int    iloc,i,isize,nsize,ier;

  assert( met->size == 6 );
  nsize  = met->size;

  for( i=0; i<4; i++ ) {
    for(isize = 0; isize < nsize; isize++ )
      dm[i][isize] = oldMet->m[nsize*pt->v[i]+isize];
    if( !MMG5_invmat(dm[i],mi[i]) ) return 0;
  }

  /** Linear interpolation of the metrics */
  for( isize = 0; isize < nsize; isize++ ) {
    m[isize] = phi[0]*mi[0][isize] + phi[1]*mi[1][isize] +
      phi[2]*mi[2][isize] + phi[3]*mi[3][isize];
  }

  if( !MMG5_invmat(m,mi[0]) ) return 0;
  for( isize = 0; isize < nsize; isize++ )
    met->m[nsize*ip+isize] = mi[0][isize];

  return 1;
}

/**
 * \param mesh pointer to the current mesh.
 * \param oldMesh pointer to the background mesh.
 * \param met pointer to the current metrics.
 * \param oldMet pointer to the background metrics.
 * \param permNodGlob permutation array for nodes.
 * \param inputMet 1 if user provided metric.
 *
 * \return 0 if fail, 1 if success
 *
 * Copy the data stored in a solution structure of a freezed interface point.
 *
 */
static inline
int PMMG_copySol_point( MMG5_pMesh mesh,MMG5_pMesh oldMesh,
                        MMG5_pSol sol,MMG5_pSol oldSol,int* permNodGlob) {
  MMG5_pPoint    ppt;
  int            isize,nsize,ip;

  nsize   = sol->size;

#warning Luca: when surface adapt will be ready, distinguish BDY from PARBDY
  /** Freezed points: Copy the data stored in solution structure  */
  if ( (!oldMesh->info.renum) || !permNodGlob ) {
    /* No permutation array: simple copy */
    for( ip = 1; ip <= oldMesh->np; ++ip ) {
      ppt = &oldMesh->point[ip];
      if( !MG_VOK(ppt) ) continue;

      /* Remark: ppt->flag is setted multiple times to mesh->base if copySol is
       * called multiple times (for example if we copy both a metric and a
       * solution fields) but it is not a problem since we use the same mesh  */
      ppt->flag = mesh->base;
      if( ppt->tag & MG_REQ ) {
        for( isize = 0; isize<nsize; isize++ ) {
          sol->m[nsize*ip+isize] = oldSol->m[nsize*ip+isize];
        }
      }
    }
  }
  else {
    /* Due to the scotch renumbering we must copy from the old mesh to the
     * new one */
    for( ip = 1; ip <= oldMesh->np; ++ip ) {
      ppt = &oldMesh->point[ip];
      if( !MG_VOK(ppt) ) continue;

      /* Remark: ppt->flag is setted multiple times to mesh->base if copySol is
       * called multiple times (for example if we copy both a metric and a
       * solution fields) but it is not a problem since we use the same mesh  */
      ppt->flag = mesh->base;
      if( ppt->tag & MG_REQ ) {
        for( isize = 0; isize<nsize; isize++ ) {
          sol->m[nsize*permNodGlob[ip]+isize] = oldSol->m[nsize*ip+isize];
        }
      }
    }
  }

  return 1;
}

/**
 * \param mesh pointer to the current mesh.
 * \param oldMesh pointer to the background mesh.
 * \param met pointer to the current metrics.
 * \param oldMet pointer to the background metrics.
 * \param permNodGlob permutation array for nodes.
 *
 * \return 0 if fail, 1 if success
 *
 * Copy the metric of a freezed interface point.
 *
 */
static
int PMMG_copyMetrics_point( MMG5_pMesh mesh,MMG5_pMesh oldMesh,
                            MMG5_pSol met,MMG5_pSol oldMet,int* permNodGlob,
                            unsigned char inputMet ) {
  MMG5_pPoint    ppt;
  int            isize,nsize,ip;
  int            ier;

  if ( !inputMet || mesh->info.hsiz > 0.0 ) return 1;

  ier =  PMMG_copySol_point( mesh, oldMesh,met,oldMet,permNodGlob);

  return ier;
}

/**
 * \param mesh pointer to the current mesh.
 * \param oldMesh pointer to the background mesh.
 * \param field pointer to the current fields.
 * \param oldField pointer to the background fields.
 * \param permNodGlob permutation array for nodes.
 *
 * \return 0 if fail, 1 if success
 *
 * Copy the metric of a freezed interface point.
 *
 */
static
int PMMG_copyFields_point( MMG5_pMesh mesh,MMG5_pMesh oldMesh,
                           MMG5_pSol field,MMG5_pSol oldField,int* permNodGlob) {
  MMG5_pPoint    ppt;
  MMG5_pSol      psl,oldPsl;
  int            isize,nsize,ip,j,ier;

  if ( !mesh->nsols ) return 1;

  for ( j=0; j<mesh->nsols; ++j ) {
    psl    =    field + j;
    oldPsl = oldField + j;
    ier =  PMMG_copySol_point( mesh, oldMesh,psl,oldPsl,permNodGlob);
    if ( !ier ) {
      return 0;
    }
  }

  return 1;
}

/**
 * \param mesh pointer to the current mesh.
 * \param oldMesh pointer to the background mesh.
 * \param met pointer to the current metrics.
 * \param oldMet pointer to the background metrics.
 * \param field pointer to the current fields.
 * \param oldField pointer to the background fields.
 * \param permNodGlob permutation array for nodes.
 * \param inputMet 1 if user provided metric.
 *
 * \return 0 if fail, 1 if success
 *
 * Copy the metric and fields of a freezed interface point.
 *
 */
int PMMG_copyMetricsAndFields_point( MMG5_pMesh mesh ,MMG5_pMesh oldMesh,
                                     MMG5_pSol  met  ,MMG5_pSol  oldMet,
                                     MMG5_pSol  field,MMG5_pSol  oldField,
                                     int* permNodGlob,unsigned char inputMet) {
  int ier;

  ier = PMMG_copyMetrics_point(mesh,oldMesh,met,oldMet,permNodGlob,inputMet);
  if ( !ier ) {
    return 0;
  }

  ier = PMMG_copyFields_point(mesh,oldMesh,field,oldField,permNodGlob);

  return ier;
}

/**
 * \param mesh pointer to the current mesh structure.
 * \param oldMesh pointer to the background mesh structure.
 * \param met pointer to the current metrics structure.
 * \param oldMet pointer to the background metrics structure.
 * \param faceAreas pointer to the array of oriented face areas.
 * \param permNodGlob permutation array of nodes.
 * \param inputMet 1 if user provided metric.
 *
 * \return 0 if fail, 1 if success
 *
 * Interpolate metrics and solution fields for all groups from background
 * to current meshes.
 * For the metric:
 *  Do nothing if no metrics is provided (inputMet == 0), otherwise:
 *  - if the metrics is constant, recompute it;
 *  - else, interpolate the non-constant metrics.
 *
 * For the solution fields: Do nothing if no solution field is provided
 *   (mesh->nsols == 0), interpolate the non-constant field otherwise.
 *
 *  Oriented face areas are pre-computed in this function before proceeding
 *  with the localization.
 *
 */
static
int PMMG_interpMetricsAndFields_mesh( MMG5_pMesh mesh,MMG5_pMesh oldMesh,
                                      MMG5_pSol met,MMG5_pSol oldMet,
                                      MMG5_pSol field,MMG5_pSol oldField,
                                      double *faceAreas,int *permNodGlob,
                                      unsigned char inputMet ) {
  MMG5_pTetra pt;
  MMG5_pPoint ppt;
  MMG5_pSol   psl,oldPsl;
  PMMG_baryCoord barycoord[4];
  double      coord[4],*normal;
  int         ip,istart,ie,ifac,ia,ib,ic,iloc,nsols;
  int         ismet,ier,j;
  static int  mmgWarn=0;

  nsols = mesh->nsols;

  ismet = 1;
  if ( inputMet != 1 ) {
    ismet = 0;
  }
  else  if( mesh->info.hsiz > 0.0 ) {

    /* Compute constant metrics */
    if ( !MMG3D_Set_constantSize(mesh,met) ) return 0;
    ismet = 0;

  }
  if ( (!ismet) && (!nsols) ) {

    /* Nothing to do */
    return 1;
  }


  /** Pre-compute oriented face areas */
  for( ie = 1; ie <= oldMesh->ne; ie++ ) {
    pt = &oldMesh->tetra[ie];
    /* Store tetra volume in the qual field */
    pt->qual = MMG5_orvol( oldMesh->point, pt->v );
    /* Store oriented face normals */
    for( ifac = 0; ifac < 4; ifac++ ) {
      normal = &faceAreas[12*ie+3*ifac];
      ia = pt->v[MMG5_idir[ifac][0]];
      ib = pt->v[MMG5_idir[ifac][1]];
      ic = pt->v[MMG5_idir[ifac][2]];
      ier = MMG5_nonUnitNorPts( oldMesh,ia,ib,ic,normal );
    }
  }


  /** Interpolate metrics and solution fields */
  oldMesh->base = 0;
  for ( ie = 1; ie < oldMesh->ne+1; ie++ ) {
    pt = &oldMesh->tetra[ie];
    if ( !MG_EOK(pt) ) continue;
    pt->flag = oldMesh->base;
  }

  mesh->base++;
  istart = 1;
  for( ie = 1; ie <= mesh->ne; ie++ ) {
    pt = &mesh->tetra[ie];
    if( !MG_EOK(pt) ) continue;
    for( iloc = 0; iloc < 4; iloc++ ) {
      ip = pt->v[iloc];
      ppt = &mesh->point[ip];
      if( !MG_VOK(ppt) ) continue;

      /* Skip already interpolated points */
      if( ppt->flag == mesh->base ) continue;

      if( ppt->tag & MG_REQ ) {
        /* Flag point as interpolated */
        ppt->flag = mesh->base;
        continue; // treated by copyMetricAndField_points
      } else {

        /** Locate point in the old mesh */
        istart = PMMG_locatePoint( oldMesh, ppt, istart,
                                   faceAreas, barycoord );
        if( !istart ) {
          fprintf(stderr,"\n  ## Error: %s:"
                  " point %d not found, coords %e %e %e\n",__func__,
                  ip, mesh->point[ip].c[0],
                  mesh->point[ip].c[1],mesh->point[ip].c[2]);
          return 0;
        } else if( istart < 0 ) {
          if ( !mmgWarn ) {
            mmgWarn = 1;
            if ( mesh->info.imprim > PMMG_VERB_VERSION ) {
              fprintf(stderr,"\n  ## Warning: %s: point %d not"
                      " found, coords %e %e %e\n",__func__,
                      ip, mesh->point[ip].c[0],mesh->point[ip].c[1],
                      mesh->point[ip].c[2]);
            }
          }
          istart = -istart;
        }

        /** Interpolate point metrics */
        PMMG_get_baryCoord( coord, barycoord );
        if ( ismet ) {
          ier = PMMG_interp4bar(mesh,met,oldMet,
                                &oldMesh->tetra[istart],
                                ip,coord);
        }
        if ( mesh->nsols ) {
          for ( j=0; j<mesh->nsols; ++j ) {
            psl    = field + j;
            oldPsl = oldField + j;
            ier = PMMG_interp4bar_iso(mesh,field,oldField,
                                      &oldMesh->tetra[istart],
                                      ip,coord);
          }
        }

        /* Flag point as interpolated */
        ppt->flag = mesh->base;
      }
    }
  }

  return 1;
}

/**
 * \param parmesh pointer to the parmesh structure.
 * \param permNodGlob permutation array of nodes.
 *
 * \return 0 if fail, 1 if success
 *
 *  Interpolate metrics for all groups from background to current meshes.
 *  Do nothing if no metrics is provided (info.inputMet == 0), otherwise:
 *  - if the metrics is constant, recompute it;
 *  - else, interpolate the non-constant metrics.
 *
 */
int PMMG_interpMetricsAndFields( PMMG_pParMesh parmesh,int *permNodGlob ) {
  PMMG_pGrp   grp,oldGrp;
  MMG5_pMesh  mesh,oldMesh;
  MMG5_pSol   met,oldMet,field,oldField;
  double      *faceAreas;
  int         igrp,ier;
  int8_t      allocated;

  /** Loop on current groups */
  ier = 1;
  for( igrp = 0; igrp < parmesh->ngrp; igrp++ ) {

    grp  = &parmesh->listgrp[igrp];
    mesh = grp->mesh;
    met  = grp->met;
    field = grp->field;

    oldGrp  = &parmesh->old_listgrp[igrp];
    oldMesh = oldGrp->mesh;
    oldMet  = oldGrp->met;
    oldField = oldGrp->field;

    /** Pre-allocate oriented face areas */
    allocated = 0;
    if ( mesh->nsols || (( parmesh->info.inputMet == 1 ) && ( mesh->info.hsiz <= 0.0 )) ) {
      ier = 1;
      PMMG_MALLOC( parmesh,faceAreas,12*(oldMesh->ne+1),double,"faceAreas",ier=0 );
      if( !ier ) {
        PMMG_DEL_MEM(parmesh,faceAreas,double,"faceAreas");
        return 0;
      }
      allocated = 1;
    }

    if( !PMMG_interpMetricsAndFields_mesh( mesh, oldMesh, met, oldMet, field, oldField,
                                           faceAreas, permNodGlob, parmesh->info.inputMet ) ) {
      ier = 0;
    }

    /** Deallocate oriented face areas */
    if ( allocated ) {
      PMMG_DEL_MEM(parmesh,faceAreas,double,"faceAreas");
    }
  }

  return ier;
}
