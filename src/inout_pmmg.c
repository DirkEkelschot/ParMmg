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
 * \file inout_pmmg.c
 * \brief io for the parmmg software
 * \author Algiane Froehly (InriaSoft)
 * \version 1
 * \date 07 2018
 * \copyright GNU Lesser General Public License.
 *
 * input/outputs for parmmg.
 *
 */

#include "parmmg.h"

int PMMG_loadCommunicators( PMMG_pParMesh parmesh,FILE* inm,int bin ) {
  MMG5_pTetra pt;
  MMG5_pPrism pp;
  MMG5_pTria  pt1;
  MMG5_pQuad  pq1;
  MMG5_pEdge  pa;
  MMG5_pPoint ppt;
  int         API_mode,icomm,ier;
  int         n_face_comm,n_node_comm;
  int         *nitem_face_comm,*nitem_node_comm;
  int         *color_face,*color_node;
  int         **idx_face_loc,**idx_face_glo;
  int         **idx_node_loc,**idx_node_glo;
  double      *norm,*n,dd;
  float       fc;
  long        posfaces,posnodes;
  int         iswp;
  int         binch,bdim,bpos,i,k,ip,idn;
  int         *ina;
  char        *ptr;
  char        chaine[MMG5_FILESTR_LGTH],strskip[MMG5_FILESTR_LGTH];

  posfaces = posnodes = 0;
  n_face_comm = n_node_comm = 0;
  iswp = 0;
  ina = NULL;


  if (!bin) {
    strcpy(chaine,"D");
    while(fscanf(inm,"%127s",&chaine[0])!=EOF && strncmp(chaine,"End",strlen("End")) ) {
      if ( chaine[0] == '#' ) {
        fgets(strskip,MMG5_FILESTR_LGTH,inm);
        continue;
      }

      if(!strncmp(chaine,"ParallelTriangles",strlen("ParallelTriangles"))) {
        MMG_FSCANF(inm,"%d",&n_face_comm);
        posfaces = ftell(inm);
        continue;
      } else if(!strncmp(chaine,"ParallelVertices",strlen("ParallelVertices"))) {
        MMG_FSCANF(inm,"%d",&n_node_comm);
        posnodes = ftell(inm);
        continue;
      }
    }
  } else { //binary file
//    bdim = 0;
//    MMG_FREAD(&mesh->ver,MMG5_SW,1,inm);
//    iswp=0;
//    if(mesh->ver==16777216)
//      iswp=1;
//    else if(mesh->ver!=1) {
//      fprintf(stderr,"BAD FILE ENCODING\n");
//    }
//    MMG_FREAD(&mesh->ver,MMG5_SW,1,inm);
//    if(iswp) mesh->ver = MMG5_swapbin(mesh->ver);
//    while(fread(&binch,MMG5_SW,1,inm)!=0 && binch!=54 ) {
//      if(iswp) binch=MMG5_swapbin(binch);
//      if(binch==54) break;
//      if(!bdim && binch==3) {  //Dimension
//        MMG_FREAD(&bdim,MMG5_SW,1,inm);  //NulPos=>20
//        if(iswp) bdim=MMG5_swapbin(bdim);
//        MMG_FREAD(&bdim,MMG5_SW,1,inm);
//        if(iswp) bdim=MMG5_swapbin(bdim);
//        mesh->dim = bdim;
//        if(bdim!=3) {
//          fprintf(stderr,"BAD MESH DIMENSION : %d\n",mesh->dim);
//          fprintf(stderr," Exit program.\n");
//          return -1;
//        }
//        continue;
//      } else if(!mesh->npi && binch==4) {  //Vertices
//        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
//        if(iswp) bpos=MMG5_swapbin(bpos);
//        MMG_FREAD(&mesh->npi,MMG5_SW,1,inm);
//        if(iswp) mesh->npi=MMG5_swapbin(mesh->npi);
//        posnp = ftell(inm);
//        rewind(inm);
//        fseek(inm,bpos,SEEK_SET);
//        continue;
//      } else if(binch==15) {  //RequiredVertices
//        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
//        if(iswp) bpos=MMG5_swapbin(bpos);
//        MMG_FREAD(&npreq,MMG5_SW,1,inm);
//        if(iswp) npreq=MMG5_swapbin(npreq);
//        posnpreq = ftell(inm);
//        rewind(inm);
//        fseek(inm,bpos,SEEK_SET);
//        continue;
//      } else if(!mesh->nti && binch==6) {//Triangles
//        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
//        if(iswp) bpos=MMG5_swapbin(bpos);
//        MMG_FREAD(&mesh->nti,MMG5_SW,1,inm);
//        if(iswp) mesh->nti=MMG5_swapbin(mesh->nti);
//        posnt = ftell(inm);
//        rewind(inm);
//        fseek(inm,bpos,SEEK_SET);
//        continue;
//      } else if(binch==17) {  //RequiredTriangles
//        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
//        if(iswp) bpos=MMG5_swapbin(bpos);
//        MMG_FREAD(&ntreq,MMG5_SW,1,inm);
//        if(iswp) ntreq=MMG5_swapbin(ntreq);
//        posntreq = ftell(inm);
//        rewind(inm);
//        fseek(inm,bpos,SEEK_SET);
//        continue;
//      }
//      else if(!mesh->nquad && binch==7) {//Quadrilaterals
//        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
//        if(iswp) bpos=MMG5_swapbin(bpos);
//        MMG_FREAD(&mesh->nquad,MMG5_SW,1,inm);
//        if(iswp) mesh->nquad=MMG5_swapbin(mesh->nquad);
//        posnq = ftell(inm);
//        rewind(inm);
//        fseek(inm,bpos,SEEK_SET);
//        continue;
//      } else if(binch==18) {  //RequiredQuadrilaterals
//        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
//        if(iswp) bpos=MMG5_swapbin(bpos);
//        MMG_FREAD(&nqreq,MMG5_SW,1,inm);
//        if(iswp) nqreq=MMG5_swapbin(nqreq);
//        posnqreq = ftell(inm);
//        rewind(inm);
//        fseek(inm,bpos,SEEK_SET);
//        continue;
//      } else if(!mesh->nei && binch==8) {//Tetra
//        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
//        if(iswp) bpos=MMG5_swapbin(bpos);
//        MMG_FREAD(&mesh->nei,MMG5_SW,1,inm);
//        if(iswp) mesh->nei=MMG5_swapbin(mesh->nei);
//        posne = ftell(inm);
//        rewind(inm);
//        fseek(inm,bpos,SEEK_SET);
//        continue;
//      } else if(!mesh->nprism && binch==9) {//Prism
//        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
//        if(iswp) bpos=MMG5_swapbin(bpos);
//        MMG_FREAD(&mesh->nprism,MMG5_SW,1,inm);
//        if(iswp) mesh->nprism=MMG5_swapbin(mesh->nprism);
//        posnprism = ftell(inm);
//        rewind(inm);
//        fseek(inm,bpos,SEEK_SET);
//        continue;
//      } else if(binch==12) {  //RequiredTetra
//        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
//        if(iswp) bpos=MMG5_swapbin(bpos);
//        MMG_FREAD(&nereq,MMG5_SW,1,inm);
//        if(iswp) nereq=MMG5_swapbin(nereq);
//        posnereq = ftell(inm);
//        rewind(inm);
//        fseek(inm,bpos,SEEK_SET);
//        continue;
//      } else if(!ncor && binch==13) { //Corners
//        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
//        if(iswp) bpos=MMG5_swapbin(bpos);
//        MMG_FREAD(&ncor,MMG5_SW,1,inm);
//        if(iswp) ncor=MMG5_swapbin(ncor);
//        posncor = ftell(inm);
//        rewind(inm);
//        fseek(inm,bpos,SEEK_SET);
//        continue;
//      } else if(!mesh->nai && binch==5) { //Edges
//        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
//        if(iswp) bpos=MMG5_swapbin(bpos);
//        MMG_FREAD(&mesh->nai,MMG5_SW,1,inm);
//        if(iswp) mesh->nai=MMG5_swapbin(mesh->nai);
//        posned = ftell(inm);
//        rewind(inm);
//        fseek(inm,bpos,SEEK_SET);
//        continue;
//      } else if(binch==16) {  //RequiredEdges
//        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
//        if(iswp) bpos=MMG5_swapbin(bpos);
//        MMG_FREAD(&nedreq,MMG5_SW,1,inm);
//        if(iswp) nedreq=MMG5_swapbin(nedreq);
//        posnedreq = ftell(inm);
//        rewind(inm);
//        fseek(inm,bpos,SEEK_SET);
//        continue;
//      }  else if(binch==14) {  //Ridges
//        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
//        if(iswp) bpos=MMG5_swapbin(bpos);
//        MMG_FREAD(&nr,MMG5_SW,1,inm);
//        if(iswp) nr=MMG5_swapbin(nr);
//        posnr = ftell(inm);
//        rewind(inm);
//        fseek(inm,bpos,SEEK_SET);
//        continue;
//      } else if(!ng && binch==60) {  //Normals
//        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
//        if(iswp) bpos=MMG5_swapbin(bpos);
//        MMG_FREAD(&ng,MMG5_SW,1,inm);
//        if(iswp) ng=MMG5_swapbin(ng);
//        posnormal = ftell(inm);
//        rewind(inm);
//        fseek(inm,bpos,SEEK_SET);
//        continue;
//      } else if(binch==20) {  //NormalAtVertices
//        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
//        if(iswp) bpos=MMG5_swapbin(bpos);
//        MMG_FREAD(&mesh->nc1,MMG5_SW,1,inm);
//        if(iswp) mesh->nc1=MMG5_swapbin(mesh->nc1);
//        posnc1 = ftell(inm);
//        rewind(inm);
//        fseek(inm,bpos,SEEK_SET);
//        continue;
//      } else {
//        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
//        if(iswp) bpos=MMG5_swapbin(bpos);
//
//        rewind(inm);
//        fseek(inm,bpos,SEEK_SET);
//      }
//    }
  }

  /* memory allocation */
  PMMG_CALLOC(parmesh,color_face,n_face_comm,int,
                  "color_face",return 0);
  PMMG_CALLOC(parmesh,idx_face_loc,n_face_comm,int*,
                  "idx_face_loc pointer",return 0);
  PMMG_CALLOC(parmesh,idx_face_glo,n_face_comm,int*,
                  "idx_face_glo pointer",return 0);

  rewind(inm);
  /* get parallel faces */
  if(n_face_comm) {
    rewind(inm);
    fseek(inm,posfaces,SEEK_SET);
    /* Read nb of items */
    if(!bin) {
      for( icomm = 0; icomm < n_face_comm; icomm++ ) {
        MMG_FSCANF(inm,"%d",&nitem_face_comm[icomm]);
      }
    }
    else {
      for( icomm = 0; icomm < n_face_comm; icomm++ ) {
        MMG_FREAD(&nitem_face_comm[icomm],MMG5_SW,1,inm);
        if(iswp) nitem_face_comm[icomm]=MMG5_swapbin(nitem_face_comm[icomm]);
      }
    }
    /* Read colors */
    if(!bin) {
      for( icomm = 0; icomm < n_face_comm; icomm++ ) {
        MMG_FSCANF(inm,"%d",&color_face[icomm]);
      }
    }
    else {
      for( icomm = 0; icomm < n_face_comm; icomm++ ) {
        MMG_FREAD(&color_face[icomm],MMG5_SW,1,inm);
        if(iswp) color_face[icomm]=MMG5_swapbin(color_face[icomm]);
      }
    }
    /* Allocate indices arrays */
    for( icomm = 0; icomm < n_face_comm; icomm++ ) {
      PMMG_CALLOC(parmesh,idx_face_loc[icomm],nitem_face_comm[icomm],int,
                  "idx_face_loc",return 0);
      PMMG_CALLOC(parmesh,idx_face_glo[icomm],nitem_face_comm[icomm],int,
                  "idx_face_glo",return 0);
    }
    /* Read indices */
    if(!bin) {
      for( icomm = 0; icomm < n_face_comm; icomm++ ) {
        for( i = 0; i < nitem_face_comm[icomm]; i++ ) {
          MMG_FSCANF(inm,"%d %d",&idx_face_loc[icomm][i],&idx_face_glo[icomm][i]);
        }
      }
    } else {
      for( icomm = 0; icomm < n_face_comm; icomm++ ) {
        for( i = 0; i < nitem_face_comm[icomm]; i++ ) {
          MMG_FREAD(&idx_face_loc[icomm][i],MMG5_SW,1,inm);
          if(iswp) idx_face_loc[icomm][i]=MMG5_swapbin(idx_face_loc[icomm][i]);
          MMG_FREAD(&idx_face_glo[icomm][i],MMG5_SW,1,inm);
          if(iswp) idx_face_glo[icomm][i]=MMG5_swapbin(idx_face_glo[icomm][i]);
        }
      }
    }
  }


  if( n_face_comm )
    API_mode = PMMG_APIDISTRIB_faces;
  else if( n_node_comm )
    API_mode = PMMG_APIDISTRIB_nodes;
  else {
    fprintf(stderr,"### Error: No parallel communicators provided!\n");
    return 0;
  }

  /* Set API mode */
  if( !PMMG_Set_iparameter( parmesh, PMMG_IPARAM_APImode, API_mode ) ) {
    MPI_Finalize();
    exit(EXIT_FAILURE);
  };


  /* Set triangles or nodes interfaces depending on API mode */
  switch( API_mode ) {

    case PMMG_APIDISTRIB_faces :

      /* Set the number of interfaces */
      ier = PMMG_Set_numberOfFaceCommunicators(parmesh, n_face_comm);

      /* Loop on each interface (proc pair) seen by the current rank) */
      for( icomm = 0; icomm < n_face_comm; icomm++ ) {

        /* Set nb. of entities on interface and rank of the outward proc */
        ier = PMMG_Set_ithFaceCommunicatorSize(parmesh, icomm,
                                               color_face[icomm],
                                               nitem_face_comm[icomm]);

        /* Set local and global index for each entity on the interface */
        ier = PMMG_Set_ithFaceCommunicator_faces(parmesh, icomm,
                                                 idx_face_loc[icomm],
                                                 idx_face_glo[icomm], 1 );
      }
      break;

    case PMMG_APIDISTRIB_nodes :

      /* Set the number of interfaces */
      ier = PMMG_Set_numberOfNodeCommunicators(parmesh, n_node_comm);

      /* Loop on each interface (proc pair) seen by the current rank) */
      for( icomm = 0; icomm < n_node_comm; icomm++ ) {

        /* Set nb. of entities on interface and rank of the outward proc */
        ier = PMMG_Set_ithNodeCommunicatorSize(parmesh, icomm,
                                               color_node[icomm],
                                               nitem_node_comm[icomm]);

        /* Set local and global index for each entity on the interface */
        ier = PMMG_Set_ithNodeCommunicator_nodes(parmesh, icomm,
                                                 idx_node_loc[icomm],
                                                 idx_node_glo[icomm], 1 );
      }
      break;
  }


  PMMG_DEL_MEM(parmesh,nitem_face_comm,int,"nitem_face_comm");
  PMMG_DEL_MEM(parmesh,color_face,int,"color_face");
  for( icomm = 0; icomm < n_face_comm; icomm++ ) {
    PMMG_DEL_MEM(parmesh,idx_face_loc[icomm],int,"idx_face_loc");
    PMMG_DEL_MEM(parmesh,idx_face_glo[icomm],int,"idx_face_glo");
  }
  PMMG_DEL_MEM(parmesh,idx_face_loc,int*,"idx_face_loc pointer");
  PMMG_DEL_MEM(parmesh,idx_face_glo,int*,"idx_face_glo pointer");

  return 1;

}

int PMMG_loadMesh_distributed(PMMG_pParMesh parmesh,const char *filename) {
  MMG5_pMesh  mesh;
  FILE*       inm;
  int         bin,ier;
  char        *data;

  if ( parmesh->ngrp != 1 ) {
    fprintf(stderr,"  ## Error: %s: you must have exactly 1 group in you parmesh.",
            __func__);
    return 0;
  }
  mesh = parmesh->listgrp[0].mesh;

  /* Set mmg verbosity to the max between the Parmmg verbosity and the mmg verbosity */
  assert ( mesh->info.imprim == parmesh->info.mmg_imprim );
  mesh->info.imprim = MG_MAX ( parmesh->info.imprim, mesh->info.imprim );


  ier = MMG3D_openMesh(mesh,filename,&inm,&bin);
  if( ier < 1 ) return ier;
  ier = MMG3D_loadMesh_opened(mesh,inm,bin);
  if( ier < 1 ) return ier;

  fclose(inm);

  /* Restore the mmg verbosity to its initial value */
  mesh->info.imprim = parmesh->info.mmg_imprim;

  if ( 1 != ier ) return 0;

  return 1;
}

int PMMG_loadMesh_centralized(PMMG_pParMesh parmesh,const char *filename) {
  MMG5_pMesh mesh;
  int        ier;

  if ( parmesh->myrank!=parmesh->info.root ) {
    return 1;
  }

  if ( parmesh->ngrp != 1 ) {
    fprintf(stderr,"  ## Error: %s: you must have exactly 1 group in you parmesh.",
            __func__);
    return 0;
  }
  mesh = parmesh->listgrp[0].mesh;

  /* Set mmg verbosity to the max between the Parmmg verbosity and the mmg verbosity */
  assert ( mesh->info.imprim == parmesh->info.mmg_imprim );
  mesh->info.imprim = MG_MAX ( parmesh->info.imprim, mesh->info.imprim );

  ier = MMG3D_loadMesh(mesh,filename);

  /* Restore the mmg verbosity to its initial value */
  mesh->info.imprim = parmesh->info.mmg_imprim;

  if ( 1 != ier ) return 0;

  return 1;
}

int PMMG_loadMet_centralized(PMMG_pParMesh parmesh,const char *filename) {
  MMG5_pMesh mesh;
  MMG5_pSol  met;
  int        ier;

  if ( parmesh->myrank!=parmesh->info.root ) {
    return 1;
  }

  if ( parmesh->ngrp != 1 ) {
    fprintf(stderr,"  ## Error: %s: you must have exactly 1 group in you parmesh.",
            __func__);
    return 0;
  }
  mesh = parmesh->listgrp[0].mesh;
  met  = parmesh->listgrp[0].met;

  /* Set mmg verbosity to the max between the Parmmg verbosity and the mmg verbosity */
  assert ( mesh->info.imprim == parmesh->info.mmg_imprim );
  mesh->info.imprim = MG_MAX ( parmesh->info.imprim, mesh->info.imprim );

  ier = MMG3D_loadSol(mesh,met,filename);

  /* Restore the mmg verbosity to its initial value */
  mesh->info.imprim = parmesh->info.mmg_imprim;

  return ier;
}

int PMMG_loadLs_centralized(PMMG_pParMesh parmesh,const char *filename) {
  MMG5_pMesh mesh;
  MMG5_pSol  ls;
  int        ier;

  if ( parmesh->myrank!=parmesh->info.root ) {
    return 1;
  }

  if ( parmesh->ngrp != 1 ) {
    fprintf(stderr,"  ## Error: %s: you must have exactly 1 group in you parmesh.",
            __func__);
    return 0;
  }
  mesh = parmesh->listgrp[0].mesh;
  ls   = parmesh->listgrp[0].sol;

  /* Set mmg verbosity to the max between the Parmmg verbosity and the mmg verbosity */
  assert ( mesh->info.imprim == parmesh->info.mmg_imprim );
  mesh->info.imprim = MG_MAX ( parmesh->info.imprim, mesh->info.imprim );

  ier = MMG3D_loadSol(mesh,ls,filename);

  /* Restore the mmg verbosity to its initial value */
  mesh->info.imprim = parmesh->info.mmg_imprim;

  return ier;
}

int PMMG_loadDisp_centralized(PMMG_pParMesh parmesh,const char *filename) {
  MMG5_pMesh mesh;
  MMG5_pSol  disp;
  int        ier;

  if ( parmesh->myrank!=parmesh->info.root ) {
    return 1;
  }

  if ( parmesh->ngrp != 1 ) {
    fprintf(stderr,"  ## Error: %s: you must have exactly 1 group in you parmesh.",
            __func__);
    return 0;
  }
  mesh = parmesh->listgrp[0].mesh;
  disp = parmesh->listgrp[0].disp;

  /* Set mmg verbosity to the max between the Parmmg verbosity and the mmg verbosity */
  assert ( mesh->info.imprim == parmesh->info.mmg_imprim );
  mesh->info.imprim = MG_MAX ( parmesh->info.imprim, mesh->info.imprim );

  ier = MMG3D_loadSol(mesh,disp,filename);

  /* Restore the mmg verbosity to its initial value */
  mesh->info.imprim = parmesh->info.mmg_imprim;

  return ier;
}

int PMMG_loadSol_centralized(PMMG_pParMesh parmesh,const char *filename) {
  MMG5_pMesh mesh;
  MMG5_pSol  sol;
  int        ier;
  const char *namein;

  if ( parmesh->myrank!=parmesh->info.root ) {
    return 1;
  }

  if ( parmesh->ngrp != 1 ) {
    fprintf(stderr,"  ## Error: %s: you must have exactly 1 group in you parmesh.",
            __func__);
    return 0;
  }
  mesh = parmesh->listgrp[0].mesh;

  /* For each mode: pointer over the solution structure to load */
  if ( mesh->info.lag >= 0 ) {
    sol = parmesh->listgrp[0].disp;
  }
  else if ( mesh->info.iso ) {
    sol = parmesh->listgrp[0].sol;
  }
  else {
    sol = parmesh->listgrp[0].met;
  }

  if ( !filename ) {
    namein = sol->namein;
  }
  else {
    namein = filename;
  }
  assert ( namein );

  /* Set mmg verbosity to the max between the Parmmg verbosity and the mmg verbosity */
  assert ( mesh->info.imprim == parmesh->info.mmg_imprim );
  mesh->info.imprim = MG_MAX ( parmesh->info.imprim, mesh->info.imprim );

  ier = MMG3D_loadSol(mesh,sol,namein);

  /* Restore the mmg verbosity to its initial value */
  mesh->info.imprim = parmesh->info.mmg_imprim;

  return ier;
}

int PMMG_loadAllSols_centralized(PMMG_pParMesh parmesh,const char *filename) {
  MMG5_pMesh mesh;
  MMG5_pSol  sol;
  int        ier;

  if ( parmesh->myrank!=parmesh->info.root ) {
    return 1;
  }

  if ( parmesh->ngrp != 1 ) {
    fprintf(stderr,"  ## Error: %s: you must have exactly 1 group in you parmesh.",
            __func__);
    return 0;
  }
  mesh = parmesh->listgrp[0].mesh;
  sol  = parmesh->listgrp[0].sol;

  /* Set mmg verbosity to the max between the Parmmg verbosity and the mmg verbosity */
  assert ( mesh->info.imprim == parmesh->info.mmg_imprim );
  mesh->info.imprim = MG_MAX ( parmesh->info.imprim, mesh->info.imprim );

  ier = MMG3D_loadAllSols(mesh,&sol,filename);

  /* Restore the mmg verbosity to its initial value */
  mesh->info.imprim = parmesh->info.mmg_imprim;

  return ier;

}

int PMMG_saveMesh_centralized(PMMG_pParMesh parmesh,const char *filename) {
  MMG5_pMesh mesh;
  int        ier;

  if ( parmesh->myrank!=parmesh->info.root ) {
    return 1;
  }

  if ( parmesh->ngrp != 1 ) {
    fprintf(stderr,"  ## Error: %s: you must have exactly 1 group in you parmesh.",
            __func__);
    return 0;
  }
  mesh = parmesh->listgrp[0].mesh;

  /* Set mmg verbosity to the max between the Parmmg verbosity and the mmg verbosity */
  assert ( mesh->info.imprim == parmesh->info.mmg_imprim );
  mesh->info.imprim = MG_MAX ( parmesh->info.imprim, mesh->info.imprim );

  ier = MMG3D_saveMesh(mesh,filename);

  /* Restore the mmg verbosity to its initial value */
  mesh->info.imprim = parmesh->info.mmg_imprim;

  return ier;
}

int PMMG_saveMet_centralized(PMMG_pParMesh parmesh,const char *filename) {
  MMG5_pMesh mesh;
  MMG5_pSol  met;
  int        ier;

  if ( parmesh->myrank!=parmesh->info.root ) {
    return 1;
  }

  if ( parmesh->ngrp != 1 ) {
    fprintf(stderr,"  ## Error: %s: you must have exactly 1 group in you parmesh.",
            __func__);
    return 0;
  }
  mesh = parmesh->listgrp[0].mesh;
  met  = parmesh->listgrp[0].met;

  /* Set mmg verbosity to the max between the Parmmg verbosity and the mmg verbosity */
  assert ( mesh->info.imprim == parmesh->info.mmg_imprim );
  mesh->info.imprim = MG_MAX ( parmesh->info.imprim, mesh->info.imprim );

  ier =  MMG3D_saveSol(mesh,met,filename);

  /* Restore the mmg verbosity to its initial value */
  mesh->info.imprim = parmesh->info.mmg_imprim;

  return ier;
}

int PMMG_saveAllSols_centralized(PMMG_pParMesh parmesh,const char *filename) {
  MMG5_pMesh mesh;
  MMG5_pSol  sol;
  int        ier;

  if ( parmesh->myrank!=parmesh->info.root ) {
    return 1;
  }

  if ( parmesh->ngrp != 1 ) {
    fprintf(stderr,"  ## Error: %s: you must have exactly 1 group in you parmesh.",
            __func__);
    return 0;
  }
  mesh = parmesh->listgrp[0].mesh;
  sol  = parmesh->listgrp[0].sol;

  /* Set mmg verbosity to the max between the Parmmg verbosity and the mmg verbosity */
  assert ( mesh->info.imprim == parmesh->info.mmg_imprim );
  mesh->info.imprim = MG_MAX ( parmesh->info.imprim, mesh->info.imprim );

  ier = MMG3D_saveAllSols(mesh,&sol,filename);

  /* Restore the mmg verbosity to its initial value */
  mesh->info.imprim = parmesh->info.mmg_imprim;

  return ier;
}
