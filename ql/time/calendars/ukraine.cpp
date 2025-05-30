/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

#include <time/calendars/ukraine.hpp>

namespace QuantLib {

    Ukraine::Ukraine(Market) {
        // all calendar instances share the same implementation instance
        static ext::shared_ptr<Calendar::Impl> impl(new Ukraine::UseImpl);
        impl_ = impl;
    }

    bool Ukraine::UseImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day (possibly moved to Monday)
            || ((d == 1 || ((d == 2 || d == 3) && w == Monday))
                && m == January)
            // Orthodox Christmas
            || ((d == 7 || ((d == 8 || d == 9) && w == Monday))
                && m == January)
            // Women's Day
            || ((d == 8 || ((d == 9 || d == 10) && w == Monday))
                && m == March)
            // Orthodox Easter Monday
            || (dd == em)
            // Holy Trinity Day
            || (dd == em+49)
            // Workers' Solidarity Days
            || ((d == 1 || d == 2 || (d == 3 && w == Monday)) && m == May)
            // Victory Day
            || ((d == 9 || ((d == 10 || d == 11) && w == Monday)) && m == May)
            // Constitution Day
            || (d == 28 && m == June)
            // Independence Day
            || (d == 24 && m == August)
            // Defender's Day (since 2015)
            || (d == 14 && m == October && y >= 2015))
            return false; // NOLINT(readability-simplify-boolean-expr)
        return true;
    }

}

