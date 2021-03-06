// *****************************************************************************
/*!
  \file      src/Inciter/NodeDiagnostics.cpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2020 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     NodeDiagnostics class for collecting nodal diagnostics
  \details   NodeDiagnostics class for collecting nodal diagnostics, e.g.,
    residuals, and various norms of errors while solving partial differential
    equations.
*/
// *****************************************************************************

#include "CGPDE.hpp"
#include "NodeDiagnostics.hpp"
#include "DiagReducer.hpp"
#include "Discretization.hpp"
#include "Inciter/InputDeck/InputDeck.hpp"
#include "Refiner.hpp"

namespace inciter {

extern ctr::InputDeck g_inputdeck;
extern std::vector< CGPDE > g_cgpde;

static CkReduction::reducerType DiagMerger;

} // inciter::

using inciter::NodeDiagnostics;

void
NodeDiagnostics::registerReducers()
// *****************************************************************************
//  Configure Charm++ reduction types
//! \details This routine is supposed to be called from a Charm++ initnode
//!   routine. Since the runtime system executes initnode routines exactly once
//!   on every logical node early on in the Charm++ init sequence, they must be
//!   static as they are called without an object. See also: Section
//!   "Initializations at Program Startup" at in the Charm++ manual
//!   http://charm.cs.illinois.edu/manuals/html/charm++/manual.html.
// *****************************************************************************
{
  DiagMerger = CkReduction::addReducer( mergeDiag );
}

bool
NodeDiagnostics::compute( Discretization& d, const tk::Fields& u ) const
// *****************************************************************************
//  Compute diagnostics, e.g., residuals, norms of errors, etc.
//! \param[in] d Discretization proxy to read from
//! \param[in] u Current solution vector
//! \return True if diagnostics have been computed
//! \details Diagnostics are defined as some norm, e.g., L2 norm, of a quantity,
//!    computed in mesh nodes, A, as ||A||_2 = sqrt[ sum_i(A_i)^2 V_i ],
//!    where the sum is taken over all mesh nodes and V_i is the nodal volume.
//!    We send multiple sets of quantities to the host for aggregation across
//!    the whole mesh. The final aggregated solution will end up in
//!    Transporter::diagnostics(). Aggregation of the partially computed
//!    diagnostics is done via potentially different policies for each field.
//! \see inciter::mergeDiag(), src/Inciter/Diagnostics.h
// *****************************************************************************
{
  // Optionally collect diagnostics and send for aggregation across all workers

  // Query after how many time steps user wants to dump diagnostics
  auto diagfreq = g_inputdeck.get< tag::interval, tag::diag >();

  if ( !((d.It()+1) % diagfreq) ) {     // if remainder, don't dump

    // Store the local IDs of those mesh nodes to which we contribute but do not
    // own, i.e., slave nodes. Ownership here is defined by having a lower chare
    // ID than any other chare that also contributes to the node.

    // Slave mesh node local IDs. Local IDs of those mesh nodes to which we
    // contribute to but do not own. Ownership here is defined by having a lower
    // chare ID than any other chare that also contributes to the node.
    std::unordered_set< std::size_t > slave;

    for (const auto& c : d.NodeCommMap())// for all neighbor chares
      if (d.thisIndex > c.first)        // if our chare ID is larger than theirs
        for (auto i : c.second)         // store local ID in set
          slave.insert( tk::cref_find( d.Lid(), i ) );

    // Diagnostics vector (of vectors) during aggregation. See
    // Inciter/Diagnostics.h.
    std::vector< std::vector< tk::real > >
      diag( NUMDIAG, std::vector< tk::real >( u.nprop(), 0.0 ) );

    const auto& coord = d.Coord();
    const auto& x = coord[0];
    const auto& y = coord[1];
    const auto& z = coord[2];

    // Put in norms sweeping our mesh chunk
    for (std::size_t i=0; i<u.nunk(); ++i)
      if (slave.find(i) == end(slave)) {    // ignore non-owned nodes

        // Compute sum for L2 norm of the numerical solution
        for (std::size_t c=0; c<u.nprop(); ++c)
          diag[L2SOL][c] += u(i,c,0) * u(i,c,0) * d.Vol()[i];

        // Query and collect analytic solution for all components of all PDEs
        // integrated at cell centroids
        std::vector< tk::real > a;
        for (const auto& eq : g_cgpde) {
          auto s = eq.analyticSolution( x[i], y[i], z[i], d.T()+d.Dt() );
          std::move( begin(s), end(s), std::back_inserter(a) );
        }
        Assert( a.size() == u.nprop(), "Size mismatch" );

        // Compute sum for L2 norm of the numerical-analytic solution
        for (std::size_t c=0; c<u.nprop(); ++c)
          diag[L2ERR][c] +=
            (u(i,c,0)-a[c]) * (u(i,c,0)-a[c]) * d.Vol()[i];
        // Compute max for Linf norm of the numerical-analytic solution
        for (std::size_t c=0; c<u.nprop(); ++c) {
          auto err = std::abs( u(i,c,0) - a[c] );
          if (err > diag[LINFERR][c]) diag[LINFERR][c] = err;
        }

      }

    // Append diagnostics vector with metadata on the current time step
    // ITER:: Current iteration count (only the first entry is used)
    // TIME: Current physical time (only the first entry is used)
    // DT: Current physical time step size (only the first entry is used)
    diag[ITER][0] = static_cast< tk::real >( d.It()+1 );
    diag[TIME][0] = d.T() + d.Dt();
    diag[DT][0] = d.Dt();

    // Contribute to diagnostics
    auto stream = serialize( diag );
    d.contribute( stream.first, stream.second.get(), DiagMerger,
      CkCallback(CkIndex_Transporter::diagnostics(nullptr), d.Tr()) );

    return true;        // diagnostics have been computed
  }

  return false;         // diagnostics have not been computed
}
