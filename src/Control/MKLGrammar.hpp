// *****************************************************************************
/*!
  \file      src/Control/MKLGrammar.hpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2020 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Intel MKL-related grammar
  \details   This file defines Intel Math Kernel Library related grammar,
    (re-)used by several executables.
*/
// *****************************************************************************
#ifndef MKLGrammar_h
#define MKLGrammar_h

#ifdef HAS_MKL
  #include "Options/MKLGaussianMethod.hpp"
  #include "Options/MKLGaussianMVMethod.hpp"
  #include "Options/MKLUniformMethod.hpp"
#endif

namespace tk {
//! Toolkit, grammar definition for Intel's Math Kernel Library
namespace mkl {

  using namespace tao;

  //! \brief rng: match any one of the MKL random number generators
  template< template< class > class use >
  struct rng :
         pegtl::sor< typename use< kw::mkl_mcg31 >::pegtl_string,
                     typename use< kw::mkl_r250 >::pegtl_string,
                     typename use< kw::mkl_mrg32k3a >::pegtl_string,
                     typename use< kw::mkl_mcg59 >::pegtl_string,
                     typename use< kw::mkl_wh >::pegtl_string,
                     typename use< kw::mkl_mt19937 >::pegtl_string,
                     typename use< kw::mkl_mt2203 >::pegtl_string,
                     typename use< kw::mkl_sfmt19937 >::pegtl_string,
                     typename use< kw::mkl_sobol >::pegtl_string,
                     typename use< kw::mkl_niederr >::pegtl_string,
                     //typename use< kw::mkl_iabstract >::pegtl_string,
                     //typename use< kw::mkl_dabstract >::pegtl_string,
                     //typename use< kw::mkl_sabstract >::pegtl_string,
                     typename use< kw::mkl_nondeterm >::pegtl_string > {};

  //! \brief Match and set MKL RNG seed
  template< template< class > class use, typename sel,
            typename vec, typename... tags >
  struct seed :
         tk::grm::process< use< kw::seed >,
                           tk::grm::insert_seed< sel, vec, tags... > > {};

  //! \brief Match and set MKL uniform method algorithm
  template< template< class > class use, typename sel,
            typename vec, typename... tags >
  struct uniform_method :
         grm::rng_option< use,
                          use< kw::uniform_method >,
                          ctr::MKLUniformMethod,
                          tag::uniform_method,
                          sel, vec, tags... > {};
     
  //! \brief Match and set MKL Gaussian method algorithm
  template< template< class > class use, typename sel,
            typename vec, typename... tags >
  struct gaussian_method :
         grm::rng_option< use,
                          use< kw::gaussian_method >,
                          ctr::MKLGaussianMethod,
                          tag::gaussian_method,
                          sel, vec, tags... > {};
     
  //! \brief Match and set MKL multi-variate Gaussian method algorithm
  template< template< class > class use, typename sel,
            typename vec, typename... tags >
  struct gaussianmv_method :
         grm::rng_option< use,
                          use< kw::gaussianmv_method >,
                          ctr::MKLGaussianMVMethod,
                          tag::gaussianmv_method,
                          sel, vec, tags... > {};

  //! \brief Match and set MKL beta method algorithm
  template< template< class > class use, typename sel,
            typename vec, typename... tags >
  struct beta_method :
         grm::rng_option< use,
                          use< kw::beta_method >,
                          ctr::MKLBetaMethod,
                          tag::beta_method,
                          sel, vec, tags... > {};

  //! \brief Match and set MKL gamma method algorithm
  template< template< class > class use, typename sel,
            typename vec, typename... tags >
  struct gamma_method :
         grm::rng_option< use,
                          use< kw::gamma_method >,
                          ctr::MKLGammaMethod,
                          tag::gamma_method,
                          sel, vec, tags... > {};

  //! \brief Match MKL RNGs in an rngs ... end block
  //! \see walker::deck::rngs
  template< template< class > class use, typename sel,
            typename vec, typename... tags >
  struct rngs :
         pegtl::if_must<
           tk::grm::scan< rng< use >,
                          tk::grm::store_back_option< use,
                                                      ctr::RNG,
                                                      sel, vec > >,
           tk::grm::block< kw::end,
                           seed< use, sel, vec, tags... >,
                           uniform_method< use, sel, vec, tags... >,
                           gaussian_method< use, sel, vec, tags... >,
                           gaussianmv_method< use, sel, vec, tags... >,
                           beta_method< use, sel, vec, tags... >,
                           gamma_method< use, sel, vec, tags... > > >
  {};

} // mkl::
} // tk::

#endif // MKLGrammar_h
