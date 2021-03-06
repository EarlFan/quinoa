// *****************************************************************************
/*!
  \file      src/RNG/Random123.hpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2020 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Interface to Random123 random number generators
  \details   Interface to Random123 random number generators
*/
// *****************************************************************************
#ifndef Random123_h
#define Random123_h

#include <cstring>
#include <random>
#include <limits>
#include <array>
#include <cfenv>

#include "NoWarning/uniform.hpp"
#include "NoWarning/beta_distribution.hpp"
#include <boost/random/gamma_distribution.hpp>

#include "Exception.hpp"
#include "Keywords.hpp"
#include "Macro.hpp"

namespace tk {

//! Random123-based random number generator used polymorphically with tk::RNG
template< class CBRNG >
class Random123 {

  private:
    static const std::size_t CBRNG_DATA_SIZE = 3;
    using ncomp_t = kw::ncomp::info::expect::type;    
    using ctr_type = typename CBRNG::ctr_type;
    using key_type = typename CBRNG::key_type;
    using value_type = typename CBRNG::ctr_type::value_type;
    using arg_type = std::vector< std::array< value_type, CBRNG_DATA_SIZE > >;

    //! Adaptor to use a std distribution with the Random123 generator
    //! \see C++ concepts: UniformRandomNumberGenerator
    struct Adaptor {
      using result_type = unsigned long;
      Adaptor( CBRNG& r, arg_type& d, int t ) : rng(r), data(d), tid(t) {}
      static constexpr result_type min() { return 0u; }
      static constexpr result_type max() {
        return std::numeric_limits< result_type >::max();
      }
      result_type operator()()
      {
        auto& d = data[ static_cast< std::size_t >( tid ) ];
        d[2] = static_cast< result_type >( tid );
        ctr_type ctr = {{ d[0], d[1] }};      // assemble counter
        key_type key = {{ d[2] }};            // assemble key
        auto res = rng( ctr, key );           // generate
        ctr.incr();
        d[0] = ctr[0];
        d[1] = ctr[1];
        return res[0];
      }
      CBRNG& rng;
      arg_type& data;
      int tid;
    };

  public:
    //! Constructor
    //! \param[in] n Initialize RNG using this many independent streams
    //! \param[in] seed RNG seed
    explicit Random123( uint64_t n = 1, uint64_t seed = 0 ) {
      Assert( n > 0, "Need at least one thread" );
      m_data.resize( n, {{ 0, seed << 32, 0 }} );
    }

    //! Uniform RNG: Generate uniform random numbers
    //! \param[in] tid Thread (or more precisely) stream ID
    //! \param[in] num Number of RNGs to generate
    //! \param[in,out] r Pointer to memory to write the random numbers to
    void uniform( int tid, ncomp_t num, double* r ) const {
      auto& d = m_data[ static_cast< std::size_t >( tid ) ];
      for (ncomp_t i=0; i<num; ++i) {
        d[2] = static_cast< unsigned long >( tid );
        ctr_type ctr = {{ d[0], d[1] }};      // assemble counter
        key_type key = {{ d[2] }};            // assemble key
        auto res = m_rng( ctr, key );         // generate
        r[i] = r123::u01fixedpt< double, value_type >( res[0] );
        ctr.incr();
        d[0] = ctr[0];
        d[1] = ctr[1];
      }
    }

    //! Gaussian RNG: Generate Gaussian random numbers
    //! \param[in] tid Thread (or more precisely stream) ID
    //! \param[in] num Number of RNGs to generate
    //! \param[in,out] r Pointer to memory to write the random numbers to
    //! \details Generating Gaussian random numbers is implemented via an
    //!   adaptor, modeling std::UniformRandomNumberGenerator, outsourcing the
    //!   transformation of uniform random numbers to Gaussian ones, to the
    //!   standard library. The adaptor is instantiated here because a standard
    //!   distribution, such as e.g., std::normal_distribution, generates
    //!   numbers using operator() with no arguments, thus the RNG state and the
    //!   thread ID (this latter only known here) must be stored in the adaptor
    //!   functor's state. Even though creating the adaptor seems like a
    //!   potentially costly operation for every call, using the standard
    //!   library implementation is still faster than a hand-coded
    //!   implementation of the Box-Muller algorithm. Note that libc++ uses a
    //!   cache, as Box-Muller, implemented using the polar algorithm generates
    //!   2 Gaussian numbers for each pair of uniform ones, caching every 2nd.
    void gaussian( int tid, ncomp_t num, double* r ) const {
      Adaptor generator( m_rng, m_data, tid );
      std::normal_distribution<> gauss_dist( 0.0, 1.0 );
      for (ncomp_t i=0; i<num; ++i) r[i] = gauss_dist( generator );
    }

    //! \brief Multi-variate Gaussian RNG: Generate multi-variate Gaussian
    //!    random numbers
    //! \param[in] tid Thread (or more precisely stream) ID
    //! \param[in] num Number of RNGs to generate
    //! \param[in] d Dimension d ( d ≥ 1) of output random vectors
    //! \param[in] mean Mean vector of dimension d
    //! \param[in] cov Lower triangle of covariance matrix, stored as a vector
    //!   of length d(d+1)/2
    //! \param[in,out] r Pointer to memory to write the random numbers to
    //! \warning Not implemented!
    void gaussianmv( [[maybe_unused]] int tid,
                     [[maybe_unused]] ncomp_t num,
                     [[maybe_unused]] ncomp_t d,
                     [[maybe_unused]] const double* const mean,
                     [[maybe_unused]] const double* const cov,
                     [[maybe_unused]] double* r ) const
    {
    }

    //! Beta RNG: Generate beta random numbers
    //! \param[in] tid Thread (or more precisely stream) ID
    //! \param[in] num Number of RNGs to generate
    //! \param[in] p First beta shape parameter
    //! \param[in] q Second beta shape parameter
    //! \param[in] a Beta displacement parameter
    //! \param[in] b Beta scale factor
    //! \param[in,out] r Pointer to memory to write the random numbers to
    //! \details Generating beta-distributed random numbers is implemented via
    //!   an adaptor, modeling boost::UniformRandomNumberGenerator, outsourcing
    //!   the transformation of uniform random numbers to beta-distributed ones,
    //!   to boost::random. The adaptor is instantiated here because a boost
    //!   random number distribution, such as e.g.,
    //!   boost::random::beta_distribution, generates numbers using operator()
    //!   with no arguments, thus the RNG state and the thread ID (this latter
    //!   only known here) must be stored in the adaptor functor's state.
    void beta( int tid, ncomp_t num, double p, double q, double a, double b,
               double* r ) const {
      Adaptor generator( m_rng, m_data, tid );
      boost::random::beta_distribution<> beta_dist( p, q );
      fenv_t fe;
      feholdexcept( &fe );
      for (ncomp_t i=0; i<num; ++i) r[i] = beta_dist( generator ) * b + a;
      feclearexcept( FE_UNDERFLOW );
      feupdateenv( &fe );
    }

    //! Gamma RNG: Generate gamma random numbers
    //! \param[in] tid Thread (or more precisely stream) ID
    //! \param[in] num Number of RNGs to generate
    //! \param[in] a Gamma shape parameter
    //! \param[in] b Gamma scale factor
    //! \param[in,out] r Pointer to memory to write the random numbers to
    //! \details Generating gamma-distributed random numbers is implemented via
    //!   an adaptor, modeling boost::UniformRandomNumberGenerator, outsourcing
    //!   the transformation of uniform random numbers to gamma-distributed
    //!   ones, to boost::random. The adaptor is instantiated here because a
    //!   boost random number distribution, such as e.g.,
    //!   boost::random::gamma_distribution, generates numbers using operator()
    //!   with no arguments, thus the RNG state and the thread ID (this latter
    //!   only known here) must be stored in the adaptor functor's state.
    void gamma( int tid, ncomp_t num, double a, double b, double* r ) const {
      Adaptor generator( m_rng, m_data, tid );
      boost::random::gamma_distribution<> gamma_dist( a, b );
      fenv_t fe;
      feholdexcept( &fe );
      for (ncomp_t i=0; i<num; ++i) r[i] = gamma_dist( generator );
      feclearexcept( FE_UNDERFLOW );
      feupdateenv( &fe );
    }

    //! Accessor to the number of threads we operate on
    uint64_t nthreads() const noexcept { return m_data.size(); }

  private:
    mutable CBRNG m_rng;        //!< Random123 RNG object
    mutable arg_type m_data;    //!< RNG arguments
};

} // tk::

#endif // Random123_h
