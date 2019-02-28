// *****************************************************************************
/*!
  \file      src/PDE/Integrate/Initialize.h
  \copyright 2016-2018, Los Alamos National Security, LLC.
  \brief     Functions for initialization of system of PDEs in DG methods
  \details   This file contains functionality for setting initial conditions
     and evaluating known solutions used in discontinuous Galerkin methods for
     various orders of numerical representation.
*/
// *****************************************************************************
#ifndef Initialize_h
#define Initialize_h

#include "Types.h"
#include "Fields.h"
#include "UnsMesh.h"
#include "FunctionPrototypes.h"

namespace tk {

//! Initalize a PDE system for DG(P0)
void
initializeP0( ncomp_t system,
              ncomp_t ncomp,
              ncomp_t offset,
              const std::vector< std::size_t >& inpoel,
              const UnsMesh::Coords& coord,
              const SolutionFn& solution,
              Fields& unk,
              real t,
              const std::size_t nielem );

//! Initalize a PDE system for DG(P1)
void
initializeP1( ncomp_t system,
              ncomp_t ncomp,
              ncomp_t offset,
              const Fields& L,
              const std::vector< std::size_t >& inpoel,
              const UnsMesh::Coords& coord,
              const SolutionFn& solution,
              Fields& unk,
              real t,
              const std::size_t nielem );

//! Initalize a PDE system for DG(P2)
void
initializeP2( ncomp_t system,
              ncomp_t ncomp,
              ncomp_t offset,
              const Fields& L,
              const std::vector< std::size_t >& inpoel,
              const UnsMesh::Coords& coord,
              const SolutionFn& solution,
              Fields& unk,
              real t,
              const std::size_t nielem );

//! Initalize a system of PDEs for discontinous Galerkin methods
void
initialize( ncomp_t system,
            ncomp_t ncomp,
            ncomp_t offset,
            const Fields& L,
            const std::vector< std::size_t >& inpoel,
            const UnsMesh::Coords& coord,
            const SolutionFn& solution,
            Fields& unk,
            real t,
            const std::size_t nielem );

} // tk::

#endif // Initialize_h