/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file poland.hpp
    \brief Polish calendar
*/

#ifndef quantlib_polish_calendar_hpp
#define quantlib_polish_calendar_hpp

#include <time/calendar.hpp>

namespace QuantLib {

    //! Polish calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>Easter Monday</li>
        <li>Corpus Christi</li>
        <li>New Year's Day, January 1st</li>
        <li>Epiphany, January 6th (since 2011)</li>
        <li>May Day, May 1st</li>
        <li>Constitution Day, May 3rd</li>
        <li>Assumption of the Blessed Virgin Mary, August 15th</li>
        <li>All Saints Day, November 1st</li>
        <li>Independence Day, November 11th</li>
        <li>Christmas, December 25th</li>
        <li>2nd Day of Christmas, December 26th</li>
        </ul>

        \ingroup calendars
    */
    class Poland : public Calendar {
      private:
        class SettlementImpl : public Calendar::WesternImpl {
          public:
            std::string name() const override { return "Poland Settlement"; }
            bool isBusinessDay(const Date&) const override;
        };
        class WseImpl final : public SettlementImpl {
          public:
            std::string name() const override { return "Warsaw stock exchange"; }
            bool isBusinessDay(const Date&) const override;
        };
      public:
        //! PL calendars
        enum Market { Settlement,  //!< Settlement calendar
                      WSE,         //!< Warsaw stock exchange calendar
        };

        explicit Poland(Market market = Settlement);
    };

}


#endif
