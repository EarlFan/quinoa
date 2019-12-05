// *****************************************************************************
/*!
  \file      src/PDE/Transport/CGTransport.hpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Scalar transport using continous Galerkin discretization
  \details   This file implements the physics operators governing transported
     scalars using continuous Galerkin discretization.
*/
// *****************************************************************************
#ifndef CGTransport_h
#define CGTransport_h

#include <vector>
#include <array>
#include <limits>
#include <cmath>
#include <unordered_set>
#include <unordered_map>

#include "Exception.hpp"
#include "Vector.hpp"
#include "DerivedData.hpp"
#include "Around.hpp"
#include "Reconstruction.hpp"
#include "Inciter/InputDeck/InputDeck.hpp"

namespace inciter {

extern ctr::InputDeck g_inputdeck;

namespace cg {

//! \brief Transport equation used polymorphically with tk::CGPDE
//! \details The template argument(s) specify policies and are used to configure
//!   the behavior of the class. The policies are:
//!   - Physics - physics configuration, see PDE/Transport/Physics/CG.h
//!   - Problem - problem configuration, see PDE/Transport/Problem.h
//! \note The default physics is CGAdvection, set in
//!    inciter::deck::check_transport()
template< class Physics, class Problem >
class Transport {

  private:
    using ncomp_t = kw::ncomp::info::expect::type;

  public:
    //! Constructor
    //! \param[in] c Equation system index (among multiple systems configured)
    explicit Transport( ncomp_t c ) :
      m_physics( Physics() ),
      m_problem( Problem() ),
      m_system( c ),
      m_ncomp(
        g_inputdeck.get< tag::component >().get< tag::transport >().at(c) ),
      m_offset(
        g_inputdeck.get< tag::component >().offset< tag::transport >(c) )
    {
      m_problem.errchk( m_system, m_ncomp );
    }

    //! Initalize the transport equations using problem policy
    //! \param[in] coord Mesh node coordinates
    //! \param[in,out] unk Array of unknowns
    //! \param[in] t Physical time
    void initialize( const std::array< std::vector< tk::real >, 3 >& coord,
                     tk::Fields& unk,
                     tk::real t ) const
    {
      Assert( coord[0].size() == unk.nunk(), "Size mismatch" );
      const auto& x = coord[0];
      const auto& y = coord[1];
      const auto& z = coord[2];
      for (ncomp_t i=0; i<x.size(); ++i) {
        const auto s =
          Problem::solution( m_system, m_ncomp, x[i], y[i], z[i], t );
        for (ncomp_t c=0; c<m_ncomp; ++c)
          unk( i, c, m_offset ) = s[c];
      }
    }

    //! Return analytic solution (if defined by Problem) at xi, yi, zi, t
    //! \param[in] xi X-coordinate
    //! \param[in] yi Y-coordinate
    //! \param[in] zi Z-coordinate
    //! \param[in] t Physical time
    //! \return Vector of analytic solution at given location and time
    std::vector< tk::real >
    analyticSolution( tk::real xi, tk::real yi, tk::real zi, tk::real t ) const
    {
      auto s = Problem::solution( m_system, m_ncomp, xi, yi, zi, t );
      return std::vector< tk::real >( begin(s), end(s) );
    }

    //! Compute nodal gradients of primitive variables for ALECG
    //! \param[in] coord Mesh node coordinates
    //! \param[in] inpoel Mesh element connectivity
    //! \param[in] U Solution vector at recent time step
    //! \param[in,out] G Nodal gradients of primitive variables
    void grad( const std::array< std::vector< tk::real >, 3 >& coord,
               const std::vector< std::size_t >& inpoel,
               const tk::Fields& U,
               tk::Fields& G ) const
    {
      Assert( U.nunk() == coord[0].size(), "Number of unknowns in solution "
              "vector at recent time step incorrect" );
      Assert( G.nunk() == coord[0].size(),
              "Number of unknowns in gradient vector incorrect" );
      Assert( G.nprop() == m_ncomp*3,
              "Number of components in gradient vector incorrect" );

      const auto& x = coord[0];
      const auto& y = coord[1];
      const auto& z = coord[2];

      // compute gradients of primitive variables in points
      G.fill( 0.0 );

      for (std::size_t e=0; e<inpoel.size()/4; ++e) {
        // access node IDs
        const std::array< std::size_t, 4 >
          N{{ inpoel[e*4+0], inpoel[e*4+1], inpoel[e*4+2], inpoel[e*4+3] }};
        // compute element Jacobi determinant
        const std::array< tk::real, 3 >
          ba{{ x[N[1]]-x[N[0]], y[N[1]]-y[N[0]], z[N[1]]-z[N[0]] }},
          ca{{ x[N[2]]-x[N[0]], y[N[2]]-y[N[0]], z[N[2]]-z[N[0]] }},
          da{{ x[N[3]]-x[N[0]], y[N[3]]-y[N[0]], z[N[3]]-z[N[0]] }};
        const auto J = tk::triple( ba, ca, da );        // J = 6V
        Assert( J > 0, "Element Jacobian non-positive" );
        // shape function derivatives, nnode*ndim [4][3]
        std::array< std::array< tk::real, 3 >, 4 > grad;
        grad[1] = tk::crossdiv( ca, da, J );
        grad[2] = tk::crossdiv( da, ba, J );
        grad[3] = tk::crossdiv( ba, ca, J );
        for (std::size_t i=0; i<3; ++i)
          grad[0][i] = -grad[1][i]-grad[2][i]-grad[3][i];
        // access solution at element nodes
        std::vector< std::array< tk::real, 4 > > u( m_ncomp );
        for (ncomp_t c=0; c<m_ncomp; ++c) u[c] = U.extract( c, m_offset, N );
        // scatter-add gradient contributions to points
        auto J24 = J/24.0;
        for (std::size_t a=0; a<4; ++a)
          for (std::size_t b=0; b<4; ++b)
            for (std::size_t j=0; j<3; ++j)
              for (std::size_t c=0; c<m_ncomp; ++c)
                G(N[a],c*3+j,0) += J24 * grad[b][j] * u[c][b];
      }
    }

    //! Compute right hand side for ALECG
    //! \param[in] coord Mesh node coordinates
    //! \param[in] inpoel Mesh element connectivity
    //! \param[in] esued Elements surrounding edges
    //! \param[in] psup Points surrounding points
    //! \param[in] triinpoel Boundary triangle face connecitivity
    //! \param[in] gid Local->global node id map
    //! \param[in] norm Dual-face normals associated to edges
    //! \param[in] G Nodal gradients
    //! \param[in] U Solution vector at recent time step
    //! \param[in,out] R Right-hand side vector computed
    void rhs( tk::real,
              const std::array< std::vector< tk::real >, 3 >&  coord,
              const std::vector< std::size_t >& inpoel,
              const std::unordered_map< tk::UnsMesh::Edge,
                      std::vector< std::size_t >, tk::UnsMesh::Hash<2>,
                      tk::UnsMesh::Eq<2> >& esued,
              const std::pair< std::vector< std::size_t >,
                               std::vector< std::size_t > >& psup,
              const std::vector< std::size_t >& triinpoel,
              const std::vector< std::size_t >& gid,
              const std::unordered_map< tk::UnsMesh::Edge,
                      std::array< tk::real, 3 >,
                      tk::UnsMesh::Hash<2>, tk::UnsMesh::Eq<2> >& norm,
              const tk::Fields& G,
              const tk::Fields& U,
              tk::Fields& R ) const
    {
      Assert( G.nunk() == coord[0].size(),
              "Number of unknowns in gradient vector incorrect" );
      Assert( G.nprop() == m_ncomp*3,
              "Number of components in gradient vector incorrect" );
      Assert( U.nunk() == coord[0].size(), "Number of unknowns in solution "
              "vector at recent time step incorrect" );
      Assert( R.nunk() == coord[0].size(),
              "Number of unknowns and/or number of components in right-hand "
              "side vector incorrect" );

      const auto& x = coord[0];
      const auto& y = coord[1];
      const auto& z = coord[2];

      // zero right hand side for all components
      for (ncomp_t c=0; c<m_ncomp; ++c) R.fill( c, m_offset, 0.0 );

      // access pointer to right hand side at component and offset
      std::vector< const tk::real* > r( m_ncomp );
      for (ncomp_t c=0; c<m_ncomp; ++c) r[c] = R.cptr( c, m_offset );

      // for verification only, will go away once correct
      tk::Fields V( U.nunk(), 3 );
      V.fill( 0.0 );

      // domain-edge integral
      for (std::size_t p=0; p<U.nunk(); ++p) {  // for each point p
        for (auto q : tk::Around(psup,p)) {     // for each edge p-q
          // access elements surrounding edge p-q
          const auto& surr_elements = tk::cref_find(esued,{p,q});
          // access and orient dual-face normals for edge p-q
          auto n = tk::cref_find( norm, {gid[p],gid[q]} );
          if (gid[p] > gid[q]) { n[0] = -n[0]; n[1] = -n[1]; n[2] = -n[2]; }
          // compute primitive variables at edge-end points (for Transport,
          // these are the same as the conserved variables)
          std::array< std::vector< tk::real >, 2 >
            ru{ std::vector<tk::real>(m_ncomp,0.0),
                std::vector<tk::real>(m_ncomp,0.0) };
          for (std::size_t c=0; c<m_ncomp; ++c) {
            ru[0][c] = U(p,c,m_offset);
            ru[1][c] = U(q,c,m_offset);
          }
          // compute MUSCL reconstruction in edge-end points
          tk::muscl( {p,q}, coord, G, ru );
          // evaluate prescribed velocity
          auto v =
            Problem::prescribedVelocity( m_system, m_ncomp, x[p], y[p], z[p] );
          // compute domain integral
          for (auto e : surr_elements) {
            // access node IDs
            const std::array< std::size_t, 4 >
                N{ inpoel[e*4+0], inpoel[e*4+1], inpoel[e*4+2], inpoel[e*4+3] };
            // compute element Jacobi determinant
            const std::array< tk::real, 3 >
              ba{{ x[N[1]]-x[N[0]], y[N[1]]-y[N[0]], z[N[1]]-z[N[0]] }},
              ca{{ x[N[2]]-x[N[0]], y[N[2]]-y[N[0]], z[N[2]]-z[N[0]] }},
              da{{ x[N[3]]-x[N[0]], y[N[3]]-y[N[0]], z[N[3]]-z[N[0]] }};
            const auto J = tk::triple( ba, ca, da );        // J = 6V
            Assert( J > 0, "Element Jacobian non-positive" );
            // shape function derivatives, nnode*ndim [4][3]
            std::array< std::array< tk::real, 3 >, 4 > grad;
            grad[1] = tk::crossdiv( ca, da, J );
            grad[2] = tk::crossdiv( da, ba, J );
            grad[3] = tk::crossdiv( ba, ca, J );
            for (std::size_t i=0; i<3; ++i)
              grad[0][i] = -grad[1][i]-grad[2][i]-grad[3][i];
            // sum flux contributions to nodes
            auto J48 = J/48.0;
            for (const auto& [a,b] : tk::lpoed) {
              auto s = tk::orient( {N[a],N[b]}, {p,q} );
              for (std::size_t j=0; j<3; ++j) {
                for (std::size_t c=0; c<m_ncomp; ++c) {
                  R.var(r[c],p) -= J48 * s * (grad[a][j] - grad[b][j])
                                 * (v[c][j]*(ru[0][c] + ru[1][c])
                                    - tk::dot(v[c],n)*(ru[1][c] - ru[0][c]));

                }
                V(p,j,0) -= 2.0*J48 * s * (grad[a][j] - grad[b][j]);
              }
            }
          }
        }
      }

      // test 2*sum_{vw in v} D_i^{vw} = 0 for interior points (this only makes
      // sense in serial)
      //std::unordered_set< std::size_t > bp(begin(triinpoel), end(triinpoel));
      //for (std::size_t p=0; p<coord[0].size(); ++p)
      //  if (bp.find(p) == end(bp))
      //    for (std::size_t j=0; j<3; ++j)
      //      if (std::abs(V(p,j,0)) > 1.0e-15)
      //        std::cout << 'd';

      // boundary integrals
      for (std::size_t e=0; e<triinpoel.size()/3; ++e) {
        // access node IDs
        const std::array< std::size_t, 3 >
          N{ triinpoel[e*3+0], triinpoel[e*3+1], triinpoel[e*3+2] };
        // node coordinates
        std::array< tk::real, 3 > xp{ x[N[0]], x[N[1]], x[N[2]] },
                                  yp{ y[N[0]], y[N[1]], y[N[2]] },
                                  zp{ z[N[0]], z[N[1]], z[N[2]] };
        // compute face area
        auto A = tk::area( xp, yp, zp );
        // compute face normal
        auto n = tk::normal( xp, yp, zp );
        // access solution at element nodes
        std::vector< std::array< tk::real, 3 > > u( m_ncomp );
        for (ncomp_t c=0; c<m_ncomp; ++c) u[c] = U.extract( c, m_offset, N );
        // sum boundary integral contributions to boundary nodes
        for (std::size_t a=0; a<3; ++a) {
          for (std::size_t j=0; j<3; ++j) {
            for (std::size_t c=0; c<m_ncomp; ++c) {
              for (std::size_t b=0; b<3; ++b)
                R.var(r[c],N[a]) -= A/12.0 * n[j] * (u[c][a] + u[c][b]);
              R.var(r[c],N[a]) += A/6.0 * n[j] * u[c][a];
            }
            for (std::size_t b=0; b<3; ++b)
              V(N[a],j,0) -= 2.0*A/12.0 * n[j];
            V(N[a],j,0) += A/6.0 * n[j];
          }
        }
      }

      // test 2*sum_{vw in v} D_i^{vw} + 2*sum_{vw in v} B_i^{vw} + B_i^v = 0
      // for boundary points (this only makes sense in serial)
      //for (std::size_t p=0; p<coord[0].size(); ++p)
      //  if (bp.find(p) != end(bp))
      //    for (std::size_t j=0; j<3; ++j)
      //      if (std::abs(V(p,j,0)) > 1.0e-15)
      //        std::cout << 'b';
    }
      
    //! Compute right hand side for DiagCG (CG-FCT)
    //! \param[in] deltat Size of time step
    //! \param[in] coord Mesh node coordinates
    //! \param[in] inpoel Mesh element connectivity
    //! \param[in] U Solution vector at recent time step
    //! \param[in,out] Ue Element-centered solution vector at intermediate step
    //!    (used here internally as a scratch array)
    //! \param[in,out] R Right-hand side vector computed
    void rhs( tk::real,
              tk::real deltat,
              const std::array< std::vector< tk::real >, 3 >& coord,
              const std::vector< std::size_t >& inpoel,
              const tk::Fields& U,
              tk::Fields& Ue,
              tk::Fields& R ) const
    {
      using tag::transport;
      Assert( U.nunk() == coord[0].size(), "Number of unknowns in solution "
              "vector at recent time step incorrect" );
      Assert( R.nunk() == coord[0].size(),
              "Number of unknowns in right-hand side vector incorrect" );

      const auto& x = coord[0];
      const auto& y = coord[1];
      const auto& z = coord[2];

      // 1st stage: update element values from node values (gather-add)
      for (std::size_t e=0; e<inpoel.size()/4; ++e) {

        // access node IDs
        const std::array< std::size_t, 4 > N{{ inpoel[e*4+0], inpoel[e*4+1],
                                               inpoel[e*4+2], inpoel[e*4+3] }};
        // compute element Jacobi determinant
        const std::array< tk::real, 3 >
          ba{{ x[N[1]]-x[N[0]], y[N[1]]-y[N[0]], z[N[1]]-z[N[0]] }},
          ca{{ x[N[2]]-x[N[0]], y[N[2]]-y[N[0]], z[N[2]]-z[N[0]] }},
          da{{ x[N[3]]-x[N[0]], y[N[3]]-y[N[0]], z[N[3]]-z[N[0]] }};
        const auto J = tk::triple( ba, ca, da );        // J = 6V
        Assert( J > 0, "Element Jacobian non-positive" );

        // shape function derivatives, nnode*ndim [4][3]
        std::array< std::array< tk::real, 3 >, 4 > grad;
        grad[1] = tk::crossdiv( ca, da, J );
        grad[2] = tk::crossdiv( da, ba, J );
        grad[3] = tk::crossdiv( ba, ca, J );
        for (std::size_t i=0; i<3; ++i)
          grad[0][i] = -grad[1][i]-grad[2][i]-grad[3][i];

        // access solution at element nodes
        std::vector< std::array< tk::real, 4 > > u( m_ncomp );
        for (ncomp_t c=0; c<m_ncomp; ++c) u[c] = U.extract( c, m_offset, N );
        // access solution at elements
        std::vector< const tk::real* > ue( m_ncomp );
        for (ncomp_t c=0; c<m_ncomp; ++c) ue[c] = Ue.cptr( c, m_offset );

        // sum nodal averages to element
        for (ncomp_t c=0; c<m_ncomp; ++c) {
          Ue.var(ue[c],e) = 0.0;
          for (std::size_t a=0; a<4; ++a)
            Ue.var(ue[c],e) += u[c][a]/4.0;
        }

        // get prescribed velocity
        const std::array< std::vector<std::array<tk::real,3>>, 4 > vel{{
          Problem::prescribedVelocity( m_system, m_ncomp,
                                       x[N[0]], y[N[0]], z[N[0]] ),
          Problem::prescribedVelocity( m_system, m_ncomp,
                                       x[N[1]], y[N[1]], z[N[1]] ),
          Problem::prescribedVelocity( m_system, m_ncomp,
                                       x[N[2]], y[N[2]], z[N[2]] ),
          Problem::prescribedVelocity( m_system, m_ncomp,
                                       x[N[3]], y[N[3]], z[N[3]] ) }};

        // sum flux (advection) contributions to element
        tk::real d = deltat/2.0;
        for (std::size_t c=0; c<m_ncomp; ++c)
          for (std::size_t j=0; j<3; ++j)
            for (std::size_t a=0; a<4; ++a)
              Ue.var(ue[c],e) -= d * grad[a][j] * vel[a][c][j]*u[c][a];

      }


      // zero right hand side for all components
      for (ncomp_t c=0; c<m_ncomp; ++c) R.fill( c, m_offset, 0.0 );

      // 2nd stage: form rhs from element values (scatter-add)
      for (std::size_t e=0; e<inpoel.size()/4; ++e) {

        // access node IDs
        const std::array< std::size_t, 4 > N{{ inpoel[e*4+0], inpoel[e*4+1],
                                               inpoel[e*4+2], inpoel[e*4+3] }};
        // compute element Jacobi determinant
        const std::array< tk::real, 3 >
          ba{{ x[N[1]]-x[N[0]], y[N[1]]-y[N[0]], z[N[1]]-z[N[0]] }},
          ca{{ x[N[2]]-x[N[0]], y[N[2]]-y[N[0]], z[N[2]]-z[N[0]] }},
          da{{ x[N[3]]-x[N[0]], y[N[3]]-y[N[0]], z[N[3]]-z[N[0]] }};
        const auto J = tk::triple( ba, ca, da );        // J = 6V
        Assert( J > 0, "Element Jacobian non-positive" );

        // shape function derivatives, nnode*ndim [4][3]
        std::array< std::array< tk::real, 3 >, 4 > grad;
        grad[1] = tk::crossdiv( ca, da, J );
        grad[2] = tk::crossdiv( da, ba, J );
        grad[3] = tk::crossdiv( ba, ca, J );
        for (std::size_t i=0; i<3; ++i)
          grad[0][i] = -grad[1][i]-grad[2][i]-grad[3][i];

        // access solution at elements
        std::vector< tk::real > ue( m_ncomp );
        for (ncomp_t c=0; c<m_ncomp; ++c) ue[c] = Ue( e, c, m_offset );
        // access pointer to right hand side at component and offset
        std::vector< const tk::real* > r( m_ncomp );
        for (ncomp_t c=0; c<m_ncomp; ++c) r[c] = R.cptr( c, m_offset );
        // access solution at nodes of element
        std::vector< std::array< tk::real, 4 > > u( m_ncomp );
        for (ncomp_t c=0; c<m_ncomp; ++c) u[c] = U.extract( c, m_offset, N );

        // get prescribed velocity
        auto xc = (x[N[0]] + x[N[1]] + x[N[2]] + x[N[3]]) / 4.0;
        auto yc = (y[N[0]] + y[N[1]] + y[N[2]] + y[N[3]]) / 4.0;
        auto zc = (z[N[0]] + z[N[1]] + z[N[2]] + z[N[3]]) / 4.0;
        const auto vel =
          Problem::prescribedVelocity( m_system, m_ncomp, xc, yc, zc );

        // scatter-add flux contributions to rhs at nodes
        tk::real d = deltat * J/6.0;
        for (std::size_t c=0; c<m_ncomp; ++c)
          for (std::size_t j=0; j<3; ++j)
            for (std::size_t a=0; a<4; ++a)
              R.var(r[c],N[a]) += d * grad[a][j] * vel[c][j]*ue[c];

        // add (optional) diffusion contribution to right hand side
        m_physics.diffusionRhs( m_system, m_ncomp, deltat, J, grad,
                                N, u, r, R );

      }
    }

    //! Compute the minimum time step size
    //! \param[in] U Solution vector at recent time step
    //! \param[in] coord Mesh node coordinates
    //! \param[in] inpoel Mesh element connectivity
    //! \return Minimum time step size
    tk::real dt( const std::array< std::vector< tk::real >, 3 >& coord,
                 const std::vector< std::size_t >& inpoel,
                 const tk::Fields& U ) const
    {
      using tag::transport;
      Assert( U.nunk() == coord[0].size(), "Number of unknowns in solution "
              "vector at recent time step incorrect" );
      const auto& x = coord[0];
      const auto& y = coord[1];
      const auto& z = coord[2];
      // compute the minimum dt across all elements we own
      tk::real mindt = std::numeric_limits< tk::real >::max();
      for (std::size_t e=0; e<inpoel.size()/4; ++e) {
        const std::array< std::size_t, 4 > N{{ inpoel[e*4+0], inpoel[e*4+1],
                                               inpoel[e*4+2], inpoel[e*4+3] }};
        // compute cubic root of element volume as the characteristic length
        const std::array< tk::real, 3 >
          ba{{ x[N[1]]-x[N[0]], y[N[1]]-y[N[0]], z[N[1]]-z[N[0]] }},
          ca{{ x[N[2]]-x[N[0]], y[N[2]]-y[N[0]], z[N[2]]-z[N[0]] }},
          da{{ x[N[3]]-x[N[0]], y[N[3]]-y[N[0]], z[N[3]]-z[N[0]] }};
        const auto L = std::cbrt( tk::triple( ba, ca, da ) / 6.0 );
        // access solution at element nodes at recent time step
        std::vector< std::array< tk::real, 4 > > u( m_ncomp );
        for (ncomp_t c=0; c<m_ncomp; ++c) u[c] = U.extract( c, m_offset, N );
        // get velocity for problem
        const std::array< std::vector<std::array<tk::real,3>>, 4 > vel{{
          Problem::prescribedVelocity( m_system, m_ncomp,
                                       x[N[0]], y[N[0]], z[N[0]] ),
          Problem::prescribedVelocity( m_system, m_ncomp,
                                       x[N[1]], y[N[1]], z[N[1]] ),
          Problem::prescribedVelocity( m_system, m_ncomp,
                                       x[N[2]], y[N[2]], z[N[2]] ),
          Problem::prescribedVelocity( m_system, m_ncomp,
                                       x[N[3]], y[N[3]], z[N[3]] ) }};
        // compute the maximum length of the characteristic velocity (advection
        // velocity) across the four element nodes
        tk::real maxvel = 0.0;
        for (ncomp_t c=0; c<m_ncomp; ++c)
          for (std::size_t i=0; i<4; ++i) {
            auto v = std::sqrt( tk::dot( vel[i][c], vel[i][c] ) );
            if (v > maxvel) maxvel = v;
          }
        // compute element dt for the advection
        auto advection_dt = L / maxvel;
        // compute element dt based on diffusion
        auto diffusion_dt = m_physics.diffusion_dt( m_system, m_ncomp, L, u );
        // compute minimum element dt
        auto elemdt = std::min( advection_dt, diffusion_dt );
        // find minimum dt across all elements
        if (elemdt < mindt) mindt = elemdt;
      }
      return mindt;
    }

    //! \brief Query all side set IDs the user has configured for all components
    //!   in this PDE system
    //! \param[in,out] conf Set of unique side set IDs to add to
    void side( std::unordered_set< int >& conf ) const
    { m_problem.side( conf ); }

    //! \brief Query Dirichlet boundary condition value on a given side set for
    //!    all components in this PDE system
    //! \param[in] t Physical time
    //! \param[in] deltat Time step size
    //! \param[in] ss Pair of side set ID and list of node IDs on the side set
    //! \param[in] coord Mesh node coordinates
    //! \return Vector of pairs of bool and boundary condition value associated
    //!   to mesh node IDs at which Dirichlet boundary conditions are set. Note
    //!   that instead of the actual boundary condition value, we return the
    //!   increment between t+dt and t, since that is what the solution requires
    //!   as we solve for the soution increments and not the solution itself.
    std::map< std::size_t, std::vector< std::pair<bool,tk::real> > >
    dirbc( tk::real t,
           tk::real deltat,
           const std::pair< const int, std::vector< std::size_t > >& ss,
           const std::array< std::vector< tk::real >, 3 >& coord ) const
    {
      using tag::param; using tag::transport; using tag::bcdir;
      using NodeBC = std::vector< std::pair< bool, tk::real > >;
      std::map< std::size_t, NodeBC > bc;
      const auto& ubc = g_inputdeck.get< param, transport, bcdir >();
      if (!ubc.empty()) {
        Assert( ubc.size() > m_system, "Indexing out of Dirichlet BC eq-vector" );
        const auto& x = coord[0];
        const auto& y = coord[1];
        const auto& z = coord[2];
        for (const auto& b : ubc[m_system])
          if (std::stoi(b) == ss.first)
            for (auto n : ss.second) {
              Assert( x.size() > n, "Indexing out of coordinate array" );
              const auto s = m_problem.solinc( m_system, m_ncomp,
                                               x[n], y[n], z[n], t, deltat );
              auto& nbc = bc[n] = NodeBC( m_ncomp );
              for (ncomp_t c=0; c<m_ncomp; ++c)
                nbc[c] = { true, s[c] };
            }
      }
      return bc;
    }

    //! Set symmetry boundary conditions at nodes
    void
    symbc( tk::Fields&,
           const std::unordered_map<std::size_t,std::array<tk::real,4>>& )
    const {}

    //! Query nodes at which symmetry boundary conditions are set
    void
    symbcnodes( const std::map< int, std::vector< std::size_t > >&,
                const std::vector< std::size_t >&,
                std::unordered_set< std::size_t >& ) const {}

    //! Return field names to be output to file
    //! \return Vector of strings labelling fields output in file
    //! \details This functions should be written in conjunction with
    //!   fieldOutput(), which provides the vector of fields to be output
    std::vector< std::string > fieldNames() const {
      std::vector< std::string > n;
      const auto& depvar =
        g_inputdeck.get< tag::param, tag::transport, tag::depvar >().
        at(m_system);
      // will output numerical solution for all components
      for (ncomp_t c=0; c<m_ncomp; ++c)
        n.push_back( depvar + std::to_string(c) + "_numerical" );
      // will output analytic solution for all components
      for (ncomp_t c=0; c<m_ncomp; ++c)
        n.push_back( depvar + std::to_string(c) + "_analytic" );
      // will output error for all components
      for (ncomp_t c=0; c<m_ncomp; ++c)
        n.push_back( depvar + std::to_string(c) + "_error" );
      return n;
    }

    //! Return field output going to file
    //! \param[in] t Physical time
    //! \param[in] V Total mesh volume
    //! \param[in] coord Mesh node coordinates
    //! \param[in] v Nodal volumes
    //! \param[in,out] U Solution vector at recent time step
    //! \return Vector of vectors to be output to file
    //! \details This functions should be written in conjunction with names(),
    //!   which provides the vector of field names
    //! \note U is overwritten
    std::vector< std::vector< tk::real > >
    fieldOutput( tk::real t,
                 tk::real V,
                 const std::array< std::vector< tk::real >, 3 >& coord,
                 const std::vector< tk::real >& v,
                 tk::Fields& U ) const
    {
      std::vector< std::vector< tk::real > > out;
      // will output numerical solution for all components
      auto E = U;
      for (ncomp_t c=0; c<m_ncomp; ++c)
        out.push_back( U.extract( c, m_offset ) );
      // evaluate analytic solution at time t
      initialize( coord, U, t );
      // will output analytic solution for all components
      for (ncomp_t c=0; c<m_ncomp; ++c)
        out.push_back( U.extract( c, m_offset ) );
      // will output error for all components
      for (ncomp_t c=0; c<m_ncomp; ++c) {
        auto u = U.extract( c, m_offset );
        auto e = E.extract( c, m_offset );
        Assert( u.size() == e.size(), "Size mismatch" );
        Assert( u.size() == v.size(), "Size mismatch" );
        for (std::size_t i=0; i<u.size(); ++i)
          e[i] = std::pow( e[i] - u[i], 2.0 ) * v[i] / V;
        out.push_back( e );
      }
      return out;
    }

    //! Return names of integral variables to be output to diagnostics file
    //! \return Vector of strings labelling integral variables output
    std::vector< std::string > names() const {
      std::vector< std::string > n;
      const auto& depvar =
        g_inputdeck.get< tag::param, tag::transport, tag::depvar >().
        at(m_system);
      // construct the name of the numerical solution for all components
      for (ncomp_t c=0; c<m_ncomp; ++c)
        n.push_back( depvar + std::to_string(c) );
      return n;
    }

  private:
    const Physics m_physics;            //!< Physics policy
    const Problem m_problem;            //!< Problem policy
    const ncomp_t m_system;             //!< Equation system index
    const ncomp_t m_ncomp;              //!< Number of components in this PDE
    const ncomp_t m_offset;             //!< Offset this PDE operates from
};

} // cg::
} // inciter::

#endif // Transport_h
