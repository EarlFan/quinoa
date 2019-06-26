// *****************************************************************************
/*!
  \file      src/PDE/Integrate/MultiMatTerms.cpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Functions for computing volume integrals of non-conservative terms
     using DG methods
  \details   This file contains functionality for computing volume integrals of
     non-conservative terms that appear in the multi-material hydrodynamic
     equations, using the discontinuous Galerkin method for various orders
     of numerical representation.
*/
// *****************************************************************************

#include "MultiMatTerms.hpp"
#include "Vector.hpp"
#include "Quadrature.hpp"
#include "EoS/EoS.hpp"
#include "MultiMat/MultiMatIndexing.hpp"

void
tk::nonConservativeInt( ncomp_t system,
                        ncomp_t ncomp,
                        std::size_t nmat,
                        ncomp_t offset,
                        const std::size_t ndof,
                        const std::vector< std::size_t >& inpoel,
                        const UnsMesh::Coords& coord,
                        const Fields& geoElem,
                        const Fields& U,
                        const std::vector< std::vector< tk::real > >&
                          riemannDeriv,
                        const std::vector< std::size_t >& ndofel,
                        Fields& R )
// *****************************************************************************
//  Compute volume integrals for multi-material DG
//! \details This is called for multi-material DG, computing volume integrals of
//!   terms in the volume fraction and energy equations, which do not exist in
//!   the single-material flow formulation (for `CompFlow` DG). For further
//!   details see Pelanti, M., & Shyue, K. M. (2019). A numerical model for
//!   multiphase liquid–vapor–gas flows with interfaces and cavitation.
//!   International Journal of Multiphase Flow, 113, 208-230.
//! \param[in] system Equation system index
//! \param[in] ncomp Number of scalar components in this PDE system
//! \param[in] nmat Number of materials in this PDE system
//! \param[in] offset Offset this PDE system operates from
//! \param[in] ndof Maximum number of degrees of freedom
//! \param[in] inpoel Element-node connectivity
//! \param[in] coord Array of nodal coordinates
//! \param[in] geoElem Element geometry array
//! \param[in] U Solution vector at recent time step
//! \param[in] riemannDeriv Derivatives of partial-pressures and velocities
//!   computed from the Riemann solver for use in the non-conservative terms
//! \param[in] ndofel Vector of local number of degrees of freedome
//! \param[in,out] R Right-hand side vector added to
// *****************************************************************************
{
  using inciter::volfracIdx;
  using inciter::densityIdx;
  using inciter::momentumIdx;
  using inciter::energyIdx;

  IGNORE(system);

  const auto& cx = coord[0];
  const auto& cy = coord[1];
  const auto& cz = coord[2];

  // compute volume integrals
  for (std::size_t e=0; e<U.nunk(); ++e)
  {
    auto ng = tk::NGvol(ndofel[e]);

    // arrays for quadrature points
    std::array< std::vector< real >, 3 > coordgp;
    std::vector< real > wgp;

    coordgp[0].resize( ng );
    coordgp[1].resize( ng );
    coordgp[2].resize( ng );
    wgp.resize( ng );

    GaussQuadratureTet( ng, coordgp, wgp );

    // Extract the element coordinates
    std::array< std::array< real, 3>, 4 > coordel {{
      {{ cx[ inpoel[4*e  ] ], cy[ inpoel[4*e  ] ], cz[ inpoel[4*e  ] ] }},
      {{ cx[ inpoel[4*e+1] ], cy[ inpoel[4*e+1] ], cz[ inpoel[4*e+1] ] }},
      {{ cx[ inpoel[4*e+2] ], cy[ inpoel[4*e+2] ], cz[ inpoel[4*e+2] ] }},
      {{ cx[ inpoel[4*e+3] ], cy[ inpoel[4*e+3] ], cz[ inpoel[4*e+3] ] }}
    }};

    auto jacInv =
            inverseJacobian( coordel[0], coordel[1], coordel[2], coordel[3] );

    // Compute the derivatives of basis function for DG(P1)
    std::array< std::vector<tk::real>, 3 > dBdx;
    if (ndofel[e] > 1)
      dBdx = eval_dBdx_p1( ndofel[e], jacInv );

    // Gaussian quadrature
    for (std::size_t igp=0; igp<ng; ++igp)
    {
      if (ndofel[e] > 4)
        eval_dBdx_p2( igp, coordgp, jacInv, dBdx );

      // Compute the basis function
      auto B =
        eval_basis( ndofel[e], coordgp[0][igp], coordgp[1][igp], coordgp[2][igp] );

      auto wt = wgp[igp] * geoElem(e, 0, 0);

      auto ugp = eval_state( ncomp, offset, ndof, ndofel[e], e, U, B );

      // get bulk properties
      tk::real rhob(0.0);
      for (std::size_t k=0; k<nmat; ++k)
          rhob += ugp[densityIdx(nmat, k)];

      std::array< tk::real, 3 > vel{{ ugp[momentumIdx(nmat, 0)]/rhob,
                                      ugp[momentumIdx(nmat, 1)]/rhob,
                                      ugp[momentumIdx(nmat, 2)]/rhob }};

      std::vector< tk::real > ymat(nmat, 0.0);
      std::array< tk::real, 3 > dap{{0.0, 0.0, 0.0}};
      for (std::size_t k=0; k<nmat; ++k)
      {
        ymat[k] = ugp[densityIdx(nmat, k)]/rhob;

        for (std::size_t idir=0; idir<3; ++idir)
          dap[idir] += riemannDeriv[3*k+idir][e];
      }

      // compute non-conservative terms
      std::vector< tk::real > ncf(ncomp, 0.0);

      for (std::size_t idir=0; idir<3; ++idir)
        ncf[momentumIdx(nmat, idir)] = 0.0;

      for (std::size_t k=0; k<nmat; ++k)
      {
        ncf[densityIdx(nmat, k)] = 0.0;
        ncf[volfracIdx(nmat, k)] = ugp[volfracIdx(nmat, k)]
                                   * riemannDeriv[3*nmat][e];
        for (std::size_t idir=0; idir<3; ++idir)
          ncf[energyIdx(nmat, k)] -= vel[idir] * ( ymat[k]*dap[idir]
                                                - riemannDeriv[3*k+idir][e] );
      }

      update_rhs_ncn( ncomp, offset, ndof, ndofel[e], wt, e, dBdx, ncf, R );
    }
  }
}

void
tk::update_rhs_ncn( ncomp_t ncomp,
                    ncomp_t offset,
                    const std::size_t ndof,
                    const std::size_t ndof_el,
                    const tk::real wt,
                    const std::size_t e,
                    const std::array< std::vector<tk::real>, 3 >& dBdx,
                    const std::vector< tk::real >& ncf,
                    Fields& R )
// *****************************************************************************
//  Update the rhs by adding the non-conservative term integrals
//! \param[in] ncomp Number of scalar components in this PDE system
//! \param[in] offset Offset this PDE system operates from
//! \param[in] ndof Maximum number of degrees of freedom
//! \param[in] ndof_el Number of degrees of freedom for local element
//! \param[in] wt Weight of gauss quadrature point
//! \param[in] e Element index
//! \param[in] dBdx Vector of basis function derivatives
//! \param[in] ncf Vector of non-conservative terms
//! \param[in,out] R Right-hand side vector computed
// *****************************************************************************
{
  //Assert( dBdx[0].size() == ndof_el, "Size mismatch for basis function derivatives" );
  //Assert( dBdx[1].size() == ndof_el, "Size mismatch for basis function derivatives" );
  //Assert( dBdx[2].size() == ndof_el, "Size mismatch for basis function derivatives" );
  Assert( ncf.size() == ncomp, "Size mismatch for non-conservative term" );

  IGNORE(ndof_el);
  IGNORE(dBdx);

  for (ncomp_t c=0; c<ncomp; ++c)
  {
    auto mark = c*ndof;
    R(e, mark, offset) += wt * ncf[c];
  }
}
