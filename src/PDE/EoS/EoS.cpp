// *****************************************************************************
/*!
  \file      src/PDE/EoS/EoS.cpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Equation of state class
  \details   This file defines functions for equations of state for the
    compressible flow equations.
*/
// *****************************************************************************

#include "EoS.hpp"
#include "Inciter/InputDeck/InputDeck.hpp"

namespace inciter {

extern ctr::InputDeck g_inputdeck;

} // inciter::

tk::real inciter::eos_pressure( ncomp_t system,
                                tk::real rho,
                                tk::real rhou,
                                tk::real rhov,
                                tk::real rhow,
                                tk::real rhoE )
// *****************************************************************************
//  \brief Calculate pressure from the material density, momentum and total energy
//    using the stiffened-gas equation of state
//! \param[in] system Equation system index
//! \param[in] rho Material density
//! \param[in] rhou X-momentum
//! \param[in] rhov Y-momentum
//! \param[in] rhow Z-momentum
//! \param[in] rhoE Material total energy
//! \return Material pressure calculated using the stiffened-gas EoS
// *****************************************************************************
{
  // query input deck to get gamma, p_c
  auto g = g_inputdeck.get< tag::param, tag::compflow, tag::gamma >()[ system ];
  tk::real p_c(0.0);

  tk::real pressure = (rhoE - 0.5 * (rhou*rhou +
                                     rhov*rhov +
                                     rhow*rhow) / rho - p_c)* (g-1.0) - p_c;
  return pressure;
}

tk::real inciter::eos_soundspeed( ncomp_t system, tk::real rho, tk::real pr )
// *****************************************************************************
//  Calculate speed of sound from the material density and material pressure
//! \param[in] system Equation system index
//! \param[in] rho Material density
//! \param[in] pr Material pressure
//! \return Material speed of sound using the stiffened-gas EoS
// *****************************************************************************
{
  // query input deck to get gamma, p_c
  auto g = g_inputdeck.get< tag::param, tag::compflow, tag::gamma >()[ system ];
  tk::real p_c(0.0);

  tk::real a = std::sqrt( g * (pr+p_c) / rho );
  return a;
}

tk::real inciter::eos_totalenergy( ncomp_t system,
                                   tk::real rho,
                                   tk::real rhou,
                                   tk::real rhov,
                                   tk::real rhow,
                                   tk::real pr )
// *****************************************************************************
//  \brief Calculate material specific total energy from the material density, momentum
//    and material pressure
//  using the stiffened-gas equation of state
//! \param[in] system Equation system index
//! \param[in] rho Material density
//! \param[in] rhou X-momentum
//! \param[in] rhov Y-momentum
//! \param[in] rhow Z-momentum
//! \param[in] pr Material pressure
//! \return Material specific total energy using the stiffened-gas EoS
// *****************************************************************************
{
  // query input deck to get gamma, p_c
  auto g = g_inputdeck.get< tag::param, tag::compflow, tag::gamma >()[ system ];
  tk::real p_c(0.0);

  tk::real rhoE = (pr + p_c) / (g-1.0) + 0.5 * (rhou*rhou +
                                                rhov*rhov +
                                                rhow*rhow) / rho + p_c;
  return rhoE;
}
