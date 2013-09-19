//******************************************************************************
/*!
  \file      src/Control/QuinoaKeywords.h
  \author    J. Bakosi
  \date      Thu Sep 19 09:34:37 2013
  \copyright Copyright 2005-2012, Jozsef Bakosi, All rights reserved.
  \brief     Quinoa's keywords
  \details   All keywords recognized by Quinoa's parser
*/
//******************************************************************************
#ifndef QuinoaKeywords_h
#define QuinoaKeywords_h

//! Signal to compiler that we are building a list of keywords. This is used by
//! the inline includes below to make sure they get included in the correct
//! namespace and not polluting the global one.
#define Keywords

namespace quinoa {
namespace grm {
namespace kw {

  using namespace pegtl::ascii;

  // Include base keywords recognized by all parsers
  #include <BaseKeywords.h>

  // Select geometry definition
  //   * Analytic
  using analytic_geometry = pegtl::string<a,n,a,l,y,t,i,c,'_',g,e,o,m,e,t,r,y>;
  //   * Discrete
  using discrete_geometry = pegtl::string<d,i,s,c,r,e,t,e,'_',g,e,o,m,e,t,r,y>;

  // Geometry primitives for analytic geometry definition
  //   * Box
  using box = pegtl::string<b,o,x>;

  // Input filename
  using input = pegtl::string<i,n,p,u,t>;

  // Output filename
  using output = pegtl::string<o,u,t,p,u,t>;

  // PDF filename
  using pdfname = pegtl::string< p,d,f,n,a,m,e >;

  // Glob (i.e. domain-average statistics) filename
  using globname = pegtl::string< g,l,o,b,n,a,m,e >;

  // Statistics filename
  using statname = pegtl::string< s,t,a,t,n,a,m,e >;

  // Select physics:
  //   * Homogeneous material mixing
  using hommix = pegtl::string< h,o,m,m,i,x >;
  //   * Homogeneous hydrodinamics
  using homhydro = pegtl::string< h,o,m,h,y,d,r,o >;
  //   * Homogeneous Rayleigh-Taylor
  using homrt = pegtl::string< h,o,m,r,t >;
  //   * Standalone-particle incompressible Navier-Stokes flow
  using spinsflow = pegtl::string< s,p,i,n,s,f,l,o,w >;

  // Select position model:
  //   * Insviscid model
  using pos_inviscid = pegtl::string< p,o,s,'_',i,n,v,i,s,c,i,d >;
  //   * Viscous model
  using pos_viscous = pegtl::string< p,o,s,'_',v,i,s,c,o,u,s >;

  // Select mass model:
  //   * Beta model
  using mass_beta = pegtl::string< m,a,s,s,'_',b,e,t,a >;

  // Select hydrodynamics model:
  //   * Simplified Langevin model
  using hydro_slm = pegtl::string< h,y,d,r,o,'_',s,l,m >;
  //   * Generalized Langevin model
  using hydro_glm = pegtl::string< h,y,d,r,o,'_',g,l,m >;

  // Select material mix model:
  //   * Interaction by exchange with the mean
  using mix_iem = pegtl::string< m,i,x,'_',i,e,m >;
  //   * Interaction by exchange with the conditional mean
  using mix_iecm = pegtl::string< m,i,x,'_',i,e,c,m >;
  //   * Dirichlet
  using mix_dir = pegtl::string< m,i,x,'_',d,i,r >;
  //   * generalized Dirichlet
  using mix_gendir = pegtl::string< m,i,x,'_',g,e,n,d,i,r >;

  // Select turbulence frequency model:
  //   * Gamma distribution model
  using freq_gamma = pegtl::string< f,r,e,q,'_',g,a,m,m,a >;

  // Number of time steps to take
  using nstep = pegtl::string< n,s,t,e,p >;

  // Terminate time stepping at this value
  using term = pegtl::string< t, e, r, m >;

  // Size of time step
  using dt = pegtl::string< d,t >;

  // Start of position model specification block
  using position = pegtl::string< p,o,s,i,t,i,o,n >;

  // Start of hydrodynamics model specification block
  using hydro = pegtl::string< h,y,d,r,o >;

  // Start of material mix model specification block
  using mix = pegtl::string< m,i,x >;

  // Number of particle position components
  using nposition = pegtl::string< n,p,o,s,i,t,i,o,n >;
  // Number of particle density components
  using ndensity = pegtl::string< n,d,e,n,s,i,t,y >;
  // Number of particle velocity components
  using nvelocity = pegtl::string< n,v,e,l,o,c,i,t,y >;
  // Number of particle scalar components
  using nscalar = pegtl::string< n,s,c,a,l,a,r >;
  // Number of particle turbulence frequency components
  using nfreq = pegtl::string< n,f,r,e,q >;

  // Dirichlet and generalized Dirichlet parameters
  using dir_B = pegtl::string< b >;
  using dir_S = pegtl::string< S >;
  using dir_kappa = pegtl::string< k,a,p,p,a >;
  using gendir_C = pegtl::string< C >;

  // Langevin model parameters
  using SLM_C0 = pegtl::string< C,'0' >;

  // Gamma frequency model parameters
  using freq_gamma_C1 = pegtl::string< C,'1' >;
  using freq_gamma_C2 = pegtl::string< C,'2' >;
  using freq_gamma_C3 = pegtl::string< C,'3' >;
  using freq_gamma_C4 = pegtl::string< C,'4' >;

  // Beta model parameters
  using Beta_At = pegtl::string< A,t >;

  // Quantities
  using transported_scalar = pegtl::string< Y >;
  using transported_scalar_fluctuation = pegtl::string< y >;

  using velocity_x = pegtl::string< U >;
  using velocity_fluctuation_x = pegtl::string< u >;
  using velocity_y = pegtl::string< V >;
  using velocity_fluctuation_y = pegtl::string< v >;
  using velocity_z = pegtl::string< W >;
  using velocity_fluctuation_z = pegtl::string< w >;

  using pressure = pegtl::string< P >;
  using pressure_fluctuation = pegtl::string< p >;
  
  using density = pegtl::string< R >;
  using density_fluctuation = pegtl::string< r >;
  
  // Total number of particles
  using npar = pegtl::string< n,p,a,r >;

  // TTY (screen) output interval
  using ttyi = pegtl::string< t,t,y,i >;

  // Dump (restart file) output interval
  using dmpi = pegtl::string< d,m,p,i >;

  // Statistics output interval
  using stai = pegtl::string< s,t,a,i >;

  // PDF output interval
  using pdfi = pegtl::string< p,d,f,i >;

  // Glob output interval
  using glbi = pegtl::string< g,l,b,i >;

  // Statistics
  using statistics = pegtl::string< s,t,a,t,i,s,t,i,c,s >;

  // Random number generator (RNG) test suite
  using rngtest = pegtl::string< r,n,g,t,e,s,t >;

  // RNG test suite
  using suite = pegtl::string< s,u,i,t,e >;

  // RNG test suites
  using smallcrush = pegtl::string< s,m,a,l,l,c,r,u,s,h >;
  using crush = pegtl::string< c,r,u,s,h >;
  using bigcrush = pegtl::string< b,i,g,c,r,u,s,h >;

  // RNGs
  using rngs = pegtl::string< r,n,g,s >;

  // MKL RNGs
  using mkl_mcg31 = pegtl::string< m,k,l,'_',m,c,g,'3','1' >;
  using mkl_r250 = pegtl::string< m,k,l,'_',r,'2','5','0' >;
  using mkl_mrg32k3a = pegtl::string< m,k,l,'_',m,r,g,'3','2',k,'3',a >;
  using mkl_mcg59 = pegtl::string< m,k,l,'_',m,c,g,'5','9' >;
  using mkl_wh = pegtl::string< m,k,l,'_',w,h >;
  using mkl_mt19937 = pegtl::string< m,k,l,'_',m,t,'1','9','9','3','7' >;
  using mkl_mt2203 = pegtl::string< m,k,l,'_',m,t,'2','2','0','3' >;
  using mkl_sfmt19937 = pegtl::string< m,k,l,'_',s,f,m,t,'1','9','9','3','7' >;
  using mkl_sobol = pegtl::string< m,k,l,'_',s,o,b,o,l >;
  using mkl_niederr = pegtl::string< m,k,l,'_',n,i,e,d,e,r,r >;
  using mkl_iabstract = pegtl::string< m,k,l,'_',i,a,b,s,t,r,a,c,t >;
  using mkl_dabstract = pegtl::string< m,k,l,'_',d,a,b,s,t,r,a,c,t >;
  using mkl_sabstract = pegtl::string< m,k,l,'_',s,a,b,s,t,r,a,c,t >;
  using mkl_nondeterm = pegtl::string< m,k,l,'_',n,o,n,d,e,t,e,r,m >;

} // kw::
} // grm::
} // quinoa::

#endif // QuinoaKeywords_h
