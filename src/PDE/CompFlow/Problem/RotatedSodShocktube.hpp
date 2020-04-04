// *****************************************************************************
/*!
  \file      src/PDE/CompFlow/Problem/RotatedSodShocktube.hpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2020 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Problem configuration for rotated Sod's shock-tube
  \details   This file defines a policy class for the compressible flow
    equations, defined in PDE/CompFlow/CompFlow.h. See PDE/CompFlow/Problems.h
    for general requirements on Problem policy classes for CompFlow.
*/
// *****************************************************************************
#ifndef CompFlowProblemRotatedSodShocktube_h
#define CompFlowProblemRotatedSodShocktube_h

#include "Types.hpp"
#include "SodShocktube.hpp"

namespace inciter {

//! CompFlow system of PDEs problem: rotated Sod shock-tube
//! \see G. A. Sod. A Survey of Several Finite Difference Methods for Systems of
//!   Nonlinear Hyperbolic Conservation Laws. J. Comput. Phys., 27:1–31, 1978.
class CompFlowProblemRotatedSodShocktube : public CompFlowProblemSodShocktube {

  public:
    //! Evaluate analytical solution at (x,y,0) for all components
    static tk::SolutionFn::result_type
    solution( ncomp_t system, ncomp_t ncomp, tk::real x, tk::real y, tk::real z,
              tk::real t, int& );

    //! Return problem type
    static ctr::ProblemType type() noexcept
    { return ctr::ProblemType::ROTATED_SOD_SHOCKTUBE; }
};

} // inciter::

#endif // CompFlowProblemRotatedSodShocktube_h
