/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2017 Peter Caspers
 Copyright (C) 2017 Oleg Kulkov

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file usdlibor.hpp
    \brief %USD %LIBOR rate
*/

#ifndef quantlib_usd_libor_hpp
#define quantlib_usd_libor_hpp

#include <indexes/ibor/libor.hpp>
#include <time/calendars/unitedstates.hpp>
#include <time/daycounters/actual360.hpp>
#include <currencies/america.hpp>

namespace QuantLib {

    //! %USD %LIBOR rate
    /*! US Dollar LIBOR fixed by ICE.

        See <https://www.theice.com/marketdata/reports/170>.
    */
    class USDLibor : public Libor {
      public:
        USDLibor(const Period& tenor,
                 const Handle<YieldTermStructure>& h = {})
        : Libor("USDLibor", tenor,
                2,
                USDCurrency(),
                UnitedStates(UnitedStates::LiborImpact),
                Actual360(), h) {}
    };

    //! base class for the one day deposit ICE %USD %LIBOR indexes
    class DailyTenorUSDLibor : public DailyTenorLibor {
      public:
        DailyTenorUSDLibor(Natural settlementDays,
                           const Handle<YieldTermStructure>& h = {})
        : DailyTenorLibor("USDLibor", settlementDays,
                          USDCurrency(),
                          UnitedStates(UnitedStates::LiborImpact),
                          Actual360(), h) {}
    };

    //! Overnight %USD %Libor index
    class USDLiborON : public DailyTenorUSDLibor {
      public:
        explicit USDLiborON(const Handle<YieldTermStructure>& h = {})
        : DailyTenorUSDLibor(0, h) {}
    };
}

#endif
