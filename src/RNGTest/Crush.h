//******************************************************************************
/*!
  \file      src/RNGTest/Crush.h
  \author    J. Bakosi
  \date      Wed Apr 23 13:41:36 2014
  \copyright Copyright 2005-2012, Jozsef Bakosi, All rights reserved.
  \brief     Crush battery
  \details   Crush battery
*/
//******************************************************************************
#ifndef Crush_h
#define Crush_h

#include <TestU01Util.h>
#include <TestU01Suite.h>

namespace rngtest {

//! Crush : TestU01Suite
class Crush : public TestU01Suite {

  public:
    //! Constructor
    explicit Crush(const Base& base);

    //! Destructor
    ~Crush() noexcept override = default;

    //! Add statistical tests to battery
    void addTests( std::size_t id,
                   const tk::ctr::RNGType& rng,
                   const Gen01Ptr& gen );

  private:
    //! Don't permit copy constructor
    Crush(const Crush&) = delete;
    //! Don't permit copy assigment
    Crush& operator=(const Crush&) = delete;
    //! Don't permit move constructor
    Crush(Crush&&) = delete;
    //! Don't permit move assigment
    Crush& operator=(Crush&&) = delete;
};

} // rngtest::

#endif // Crush_h