// *****************************************************************************
/*!
  \file      src/Control/Walker/Options/HydroTimeScales.hpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2020 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Inverse hydrodynamics time scale options
  \details   Inverse hydrodynamics time scale options
*/
// *****************************************************************************
#ifndef HydroTimeScalesOptions_h
#define HydroTimeScalesOptions_h

#include <brigand/sequences/list.hpp>

#include "Toggle.hpp"
#include "Keywords.hpp"
#include "PUPUtil.hpp"
#include "DiffEq/HydroTimeScales.hpp"

namespace walker {
namespace ctr {

//! Inverse hydrodynamics time scale types
enum class HydroTimeScalesType : uint8_t { EQ_A005H=0
                                         , EQ_A005S
                                         , EQ_A005L
                                         , EQ_A05H
                                         , EQ_A05S
                                         , EQ_A05L
                                         , EQ_A075H
                                         , EQ_A075S
                                         , EQ_A075L
                                         };

//! \brief Pack/Unpack HydroTimeScalesType: forward overload to generic enum
//!   class packer
inline void operator|( PUP::er& p, HydroTimeScalesType& e )
{ PUP::pup( p, e ); }

//! HydroTimeScales options: outsource searches to base templated on enum type
class HydroTimeScales : public tk::Toggle< HydroTimeScalesType > {

  public:
    //! Valid expected choices to make them also available at compile-time
    using keywords = brigand::list< kw::eq_A005H
                                  , kw::eq_A005S
                                  , kw::eq_A005L
                                  , kw::eq_A05H
                                  , kw::eq_A05S
                                  , kw::eq_A05L
                                  , kw::eq_A075H
                                  , kw::eq_A075S
                                  , kw::eq_A075L
                                  >;

    //! \brief Options constructor
    //! \details Simply initialize in-line and pass associations to base, which
    //!    will handle client interactions
    explicit HydroTimeScales() :
      tk::Toggle< HydroTimeScalesType >(
        //! Group, i.e., options, name
        "Inverse hydrodynamics time scale",
        //! Enums -> names
        { { HydroTimeScalesType::EQ_A005H, kw::eq_A005H::name() },
          { HydroTimeScalesType::EQ_A005S, kw::eq_A005S::name() },
          { HydroTimeScalesType::EQ_A005L, kw::eq_A005L::name() },
          { HydroTimeScalesType::EQ_A05H, kw::eq_A05H::name() },
          { HydroTimeScalesType::EQ_A05S, kw::eq_A05S::name() },
          { HydroTimeScalesType::EQ_A05L, kw::eq_A05L::name() },
          { HydroTimeScalesType::EQ_A075H, kw::eq_A075H::name() },
          { HydroTimeScalesType::EQ_A075S, kw::eq_A075S::name() },
          { HydroTimeScalesType::EQ_A075L, kw::eq_A075L::name() } },
        //! keywords -> Enums
        {  { kw::eq_A005H::string(), HydroTimeScalesType::EQ_A005H },
           { kw::eq_A005S::string(), HydroTimeScalesType::EQ_A005S },
           { kw::eq_A005L::string(), HydroTimeScalesType::EQ_A005L },
           { kw::eq_A05H::string(), HydroTimeScalesType::EQ_A05H },
           { kw::eq_A05S::string(), HydroTimeScalesType::EQ_A05S },
           { kw::eq_A05L::string(), HydroTimeScalesType::EQ_A05L },
           { kw::eq_A075H::string(), HydroTimeScalesType::EQ_A075H },
           { kw::eq_A075S::string(), HydroTimeScalesType::EQ_A075S },
           { kw::eq_A075L::string(), HydroTimeScalesType::EQ_A075L } } ) {}

    //! \brief Return table based on Enum
    //! \param[in] t Enum value of the option requested
    //! \return tk::Table associated to the option
    tk::Table table( HydroTimeScalesType t ) const {
      if (t == HydroTimeScalesType::EQ_A005H)
        return invhts_eq_A005H;
      else if (t == HydroTimeScalesType::EQ_A005S)
        return invhts_eq_A005S;
      else if (t == HydroTimeScalesType::EQ_A005L)
        return invhts_eq_A005L;
      else if (t == HydroTimeScalesType::EQ_A05H)
        return invhts_eq_A05H;
      else if (t == HydroTimeScalesType::EQ_A05S)
        return invhts_eq_A05S;
      else if (t == HydroTimeScalesType::EQ_A05L)
        return invhts_eq_A05L;
      else if (t == HydroTimeScalesType::EQ_A075H)
        return invhts_eq_A075H;
      else if (t == HydroTimeScalesType::EQ_A075S)
        return invhts_eq_A075S;
      else if (t == HydroTimeScalesType::EQ_A075L)
        return invhts_eq_A075L;
      else Throw( "Inverse hydrodynamics time scale associated to " +
                  std::to_string( static_cast<uint8_t>(t) ) + " not found" );
    }
};

} // ctr::
} // walker::

#endif // HydroTimeScalesOptions_h
