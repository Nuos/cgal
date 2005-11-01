#ifndef __EIGEN__
#define __EIGEN__

// piece of code provided by B.Levy
// it is planned to replace it.

template <class FT>
void eigen_semi_definite_symmetric(const FT *mat, 
                                   int n, 
                                   FT *eigen_vec, 
                                   FT *eigen_val) 
{
  static const FT EPS = 0.00001;
  static int MAX_ITER = 100;
  static const FT PPI = 3.14159265358;
  FT *a,*v;
  FT a_norm,a_normEPS,thr,thr_nn;
  int nb_iter = 0;
  int jj;
  int i,j,k,ij,ik,l,m,lm,mq,lq,ll,mm,imv,im,iq,ilv,il,nn;
  int *index;
  FT a_ij,a_lm,a_ll,a_mm,a_im,a_il;
  FT a_lm_2;
  FT v_ilv,v_imv;
  FT x;
  FT sinx,sinx_2,cosx,cosx_2,sincos;
  FT delta;
      
  // Number of entries in mat
      
  nn = (n*(n+1))/2;
      
  // Step 1: Copy mat to a
      
  a = new FT[nn];
      
  for( ij=0; ij<nn; ij++ ) {
      a[ij] = mat[ij];
  }
      
  // Fortran-porting trick: indices for a are between 1 and n
  a--;
      
  // Step 2 : Init diagonalization matrix as the unit matrix
  v = new FT[n*n];
      
  ij = 0;
  for( i=0; i<n; i++ ) {
      for( j=0; j<n; j++ ) {
          if( i==j ) {
              v[ij++] = 1.0;
          } else {
              v[ij++] = 0.0;
          }
      }
  }
      
  // Ugly Fortran-porting trick: indices for v are between 1 and n
  v--;
      
  // Step 3 : compute the weight of the non diagonal terms 
  ij     = 1  ;
  a_norm = 0.0;
  for( i=1; i<=n; i++ ) {
      for( j=1; j<=i; j++ ) {
          if( i!=j ) {
              a_ij    = a[ij];
              a_norm += a_ij*a_ij;
          }
          ij++;
      }
  }
      
  if( a_norm != 0.0 ) {
      
      a_normEPS = a_norm*EPS;
      thr       = a_norm    ;
  
      // Step 4 : rotations
      while( thr > a_normEPS   &&   nb_iter < MAX_ITER ) {
      
          nb_iter++;
          thr_nn = thr / nn;
          
          for( l=1  ; l< n; l++ ) {
              for( m=l+1; m<=n; m++ ) {
                  
                  // compute sinx and cosx 
                  
                  lq   = (l*l-l)/2;
                  mq   = (m*m-m)/2;
                  
                  lm     = l+mq;
                  a_lm   = a[lm];
                  a_lm_2 = a_lm*a_lm;
                  
                  if( a_lm_2 < thr_nn ) {
                      continue ;
                  }
                  
                  ll   = l+lq;
                  mm   = m+mq;
                  a_ll = a[ll];
                  a_mm = a[mm];
                  
                  delta = a_ll - a_mm;
                  
                  if( delta == 0.0 ) {
                      x = - PPI/4 ; 
                  } else {
                      x = - atan( (a_lm+a_lm) / delta ) / 2.0 ;
                  }

                  sinx    = sin(x)   ;
                  cosx    = cos(x)   ;
                  sinx_2  = sinx*sinx;
                  cosx_2  = cosx*cosx;
                  sincos  = sinx*cosx;
                  
                  // rotate L and M columns 
      
                  ilv = n*(l-1);
                  imv = n*(m-1);
                  
                  for( i=1; i<=n;i++ ) {
                      if( (i!=l) && (i!=m) ) {
                          iq = (i*i-i)/2;
                          
                          if( i<m )  { 
                              im = i + mq; 
                          } else {
                              im = m + iq;
                          }
                          a_im = a[im];
                          
                          if( i<l ) {
                              il = i + lq; 
                          } else {
                              il = l + iq;
                          }
                          a_il = a[il];
                          
                          a[il] =  a_il*cosx - a_im*sinx;
                          a[im] =  a_il*sinx + a_im*cosx;
                      }
                      
                      ilv++;
                      imv++;
                      
                      v_ilv = v[ilv];
                      v_imv = v[imv];
                      
                      v[ilv] = cosx*v_ilv - sinx*v_imv;
                      v[imv] = sinx*v_ilv + cosx*v_imv;
                  } 
                  
                  x = a_lm*sincos; x+=x;
                  
                  a[ll] =  a_ll*cosx_2 + a_mm*sinx_2 - x;
                  a[mm] =  a_ll*sinx_2 + a_mm*cosx_2 + x;
                  a[lm] =  0.0;
                  
                  thr = fabs( thr - a_lm_2 );
              }
          }
      }         
  }
      
  // Step 5: index conversion and copy eigen values 
      
  // back from Fortran to C++
  a++;
      
  for( i=0; i<n; i++ ) {
      k = i + (i*(i+1))/2;
      eigen_val[i] = a[k];
  }
    
  delete[] a;
      
  // Step 6: sort the eigen values and eigen vectors 
      
  index = new int[n];
  for( i=0; i<n; i++ ) {
      index[i] = i;
  }
      
  for( i=0; i<(n-1); i++ ) {
      x = eigen_val[i];
      k = i;
          
      for( j=i+1; j<n; j++ ) {
          if( x < eigen_val[j] ) {
              k = j;
              x = eigen_val[j];
          }
      }
          
      eigen_val[k] = eigen_val[i];
      eigen_val[i] = x;
        
      jj       = index[k];
      index[k] = index[i];
      index[i] = jj;
  }


  // Step 7: save the eigen vectors 
  
  v++; // back from Fortran to to C++
      
  ij = 0;
  for( k=0; k<n; k++ ) 
  {
      ik = index[k]*n;
      for( i=0; i<n; i++ ) 
          eigen_vec[ij++] = v[ik++];
  }
  
  delete [] v;
  delete [] index;
  return;
}

#endif
