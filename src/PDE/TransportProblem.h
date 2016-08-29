// *****************************************************************************
/*!
  \file      src/PDE/TransportProblem.h
  \author    J. Bakosi
  \date      Mon 29 Aug 2016 03:45:02 PM MDT
  \copyright 2012-2015, Jozsef Bakosi, 2016, Los Alamos National Security, LLC.
  \brief     Problem configurations for the advection-diffusion equation
  \details   This file defines policy classes for the advection-diffusion
    partial differential equation, defined in PDE/Transport.h.

    General requirements on advection-diffusion partial differential equation
    problem policy classes:

    - Must define the static function _type()_, returning the enum value of the
      policy option. Example:
      \code{.cpp}
        static ctr::ProblemType type() noexcept {
          return ctr::ProblemType::SHEAR_DIFF;
        }
      \endcode
      which returns the enum value of the option from the underlying option
      class, collecting all possible options for coefficients policies.
*/
// *****************************************************************************
#ifndef TransportProblem_h
#define TransportProblem_h

#include <boost/mpl/vector.hpp>

#include "Types.h"
#include "Inciter/InputDeck/InputDeck.h"
#include "Inciter/Options/Problem.h"

namespace inciter {

//! Transport PDE problem: diffusion of a shear layer
class TransportProblemShearDiff {

  public:
    //! Do error checking on PDE parameters
    //! \param[in] e Equation system index, i.e., which advection-diffusion
    //!   system we operate on among the systems of PDEs
    template< class eq >
    static void errchk( tk::ctr::ncomp_type e, tk::ctr::ncomp_type ncomp )
    {
      const auto& u0 = g_inputdeck.get< tag::param, eq, tag::u0 >()[e];
      ErrChk( ncomp == u0.size(),
        "Wrong number of advection-diffusion PDE parameters 'u0'" );
      const auto& lambda = g_inputdeck.get< tag::param, eq, tag::lambda >()[e];
      ErrChk( ncomp == lambda.size(),
        "Wrong number of advection-diffusion PDE parameters 'lambda'" );
      const auto& diff =
        g_inputdeck.get< tag::param, eq, tag::diffusivity >()[e];
      ErrChk( ncomp == diff.size(),
        "Wrong number of advection-diffusion PDE parameters 'diffusivity'" );
    }

    //! Set initial conditions for dispersion in simple shear flow
    //! \param[in] coord Mesh node coordinates
    //! \param[in,out] unk Array of unknowns
    //! \param[in] e Equation system index, i.e., which advection-diffusion
    //!   system we operate on among the systems of PDEs
    //! \param[in] offset System offset specifying the position of the system of
    //!   PDEs among other systems
    //! \param[in] t Physical time
    template< class eq >
    static void init( const std::array< std::vector< tk::real >, 3 >& coord,
                      tk::MeshNodes& unk,
                      tk::ctr::ncomp_type e,
                      tk::ctr::ncomp_type ncomp,
                      tk::ctr::ncomp_type offset,
                      tk::real t )
    {
      const auto& u0 = g_inputdeck.get< tag::param, eq, tag::u0 >()[e];
      const auto& lambda = g_inputdeck.get< tag::param, eq, tag::lambda >()[e];
      const auto& diff =
        g_inputdeck.get< tag::param, eq, tag::diffusivity >()[e];
      const tk::real X0 = 7200.0;        // x position of source
      const tk::real t0 = g_inputdeck.get< tag::discr, tag::t0 >(); // initial t
      const auto& x = coord[0];
      const auto& y = coord[1];
      for (ncomp_t c=0; c<ncomp; ++c) {
        const auto b = 1.0 + lambda[c]*lambda[c]*t*t/12.0;
        const auto M =
          4.0*M_PI*t0*std::sqrt( 1.0 + lambda[c]*lambda[c]*t0*t0/12.0 );
        for (ncomp_t i=0; i<x.size(); ++i) {
          tk::real a = x[i] - X0 - u0[c]*t - 0.5*lambda[c]*y[i]*t;
          unk( i, c, offset ) =
            M * exp( -a*a/(4.0*M_PI*diff[c]*t*b) - y[i]*y[i]/(4.0*diff[c]*t) )
              / ( 4.0*M_PI*t*std::sqrt(b) );
        }
      }
    }

    //! Assign prescribed shear velocity to nodes of tetrahedron element
    //! \param[in] N Element node indices
    //! \param[in] coord Mesh node coordinates
    //! \param[in] e Equation system index, i.e., which advection-diffusion
    //!   system we operate on among the systems of PDEs
    //! \return Velocity assigned to all vertices of a tetrehedron, size:
    //!   ncomp * ndim * nnode = [ncomp][3][4]
    template< class eq >
    static std::vector< std::array< std::array< tk::real, 4 >, 3 > >
    prescribedVelocity( const std::array< std::size_t, 4 >& N,
                        const std::array< std::vector< tk::real >, 3 >& coord,
                        tk::ctr::ncomp_type e,
                        tk::ctr::ncomp_type ncomp )
    {
      const auto& u0 = g_inputdeck.get< tag::param, eq, tag::u0 >()[e];
      const auto& lambda = g_inputdeck.get< tag::param, eq, tag::lambda >()[e];
      const auto& y = coord[1];
      std::vector< std::array< std::array< tk::real, 4 >, 3 > > vel( ncomp );
      for (ncomp_t c=0; c<ncomp; ++c) {
        std::array< std::array< tk::real, 4 >, 3 > v;
        v[0][0] = u0[c] + lambda[c]*y[N[0]];  v[1][0] = 0.0;  v[2][0] = 0.0;
        v[0][1] = u0[c] + lambda[c]*y[N[1]];  v[1][1] = 0.0;  v[2][1] = 0.0;
        v[0][2] = u0[c] + lambda[c]*y[N[2]];  v[1][2] = 0.0;  v[2][2] = 0.0;
        v[0][3] = u0[c] + lambda[c]*y[N[3]];  v[1][3] = 0.0;  v[2][3] = 0.0;
        vel[c] = std::move(v);
      }
      return vel;
    }

    //! Return the velocity field at cell nodes
    //! \return Array of the four values of the three velocity coordinates
    static std::vector< std::array< tk::real, 4 > >
    velocity( const tk::MeshNodes&,
              const std::array< std::vector< tk::real >, 3 >&,
              const std::array< std::size_t, 4 >& )
    { return { {{0.0, 0.0, 0.0, 0.0}},
               {{0.0, 0.0, 0.0, 0.0}},
               {{0.0, 0.0, 0.0, 0.0}} }; }

    //! Problem type enum accessor
    //! \return Problem type as enum
    static ctr::ProblemType type() noexcept
    { return ctr::ProblemType::SHEAR_DIFF; }
};

//! Transport PDE problem: rotation of Zalesak's slotted cylinder
class TransportProblemSlotCyl {

  public:
    //! Do error checking on PDE parameters
    template< class eq >
    static void errchk( tk::ctr::ncomp_type, tk::ctr::ncomp_type ) {}

    //! Set initial conditions for dispersion in simple shear flow
    template< class eq >
    static void init( const std::array< std::vector< tk::real >, 3 >&,
                      tk::MeshNodes&,
                      tk::ctr::ncomp_type,
                      tk::ctr::ncomp_type,
                      tk::ctr::ncomp_type,
                      tk::real ) {}

    //! Assign prescribed velocity to nodes of tetrahedron element
    //! \return Velocity assigned to all vertices of a tetrehedron, size:
    //!   ncomp * ndim * nnode = [ncomp][3][4]
    template< class eq >
    static std::vector< std::array< std::array< tk::real, 4 >, 3 > >
    prescribedVelocity( const std::array< std::size_t, 4 >&,
                        const std::array< std::vector< tk::real >, 3 >&,
                        tk::ctr::ncomp_type,
                        tk::ctr::ncomp_type ncomp )
    {
      std::vector< std::array< std::array< tk::real, 4 >, 3 > > vel( ncomp );
      for (ncomp_t c=0; c<ncomp; ++c) {
        std::array< std::array< tk::real, 4 >, 3 > v;
        v[0][0] = 0.0;  v[1][0] = 0.0;  v[2][0] = 0.0;
        v[0][1] = 0.0;  v[1][1] = 0.0;  v[2][1] = 0.0;
        v[0][2] = 0.0;  v[1][2] = 0.0;  v[2][2] = 0.0;
        v[0][3] = 0.0;  v[1][3] = 0.0;  v[2][3] = 0.0;
        vel[c] = std::move(v);
      }
      return vel;
    }

    //! Return the velocity field at cell nodes
    //! \return Array of the four values of the three velocity coordinates
    static std::vector< std::array< tk::real, 4 > >
    velocity( const tk::MeshNodes&,
              const std::array< std::vector< tk::real >, 3 >&,
              const std::array< std::size_t, 4 >& )
    { return { {{0.0, 0.0, 0.0, 0.0}},
               {{0.0, 0.0, 0.0, 0.0}},
               {{0.0, 0.0, 0.0, 0.0}} }; }

    static ctr::ProblemType type() noexcept
    { return ctr::ProblemType::SLOT_CYL; }
};

//! List of all advection-diffusion PDE's problem policies
using TransportProblems = boost::mpl::vector< TransportProblemShearDiff
                                            , TransportProblemSlotCyl 
                                            >;

} // inciter::

#endif // TransportProblem_h
