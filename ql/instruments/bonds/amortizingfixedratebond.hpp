/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Simon Ibbotson

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

/*! \file amortizingfixedratebond.hpp
    \brief amortizing fixed-rate bond
*/

#ifndef quantlib_amortizing_fixed_rate_bond_hpp
#define quantlib_amortizing_fixed_rate_bond_hpp

#include <instruments/bond.hpp>
#include <time/daycounter.hpp>
#include <time/schedule.hpp>
#include <interestrate.hpp>

namespace QuantLib {

    //! amortizing fixed-rate bond
    class AmortizingFixedRateBond : public Bond {
      public:
        AmortizingFixedRateBond(Natural settlementDays,
                                const std::vector<Real>& notionals,
                                Schedule schedule,
                                const std::vector<Rate>& coupons,
                                const DayCounter& accrualDayCounter,
                                BusinessDayConvention paymentConvention = Following,
                                const Date& issueDate = Date(),
                                const Period& exCouponPeriod = Period(),
                                const Calendar& exCouponCalendar = Calendar(),
                                BusinessDayConvention exCouponConvention = Unadjusted,
                                bool exCouponEndOfMonth = false,
                                const std::vector<Real>& redemptions = { 100.0 },
                                Integer paymentLag = 0);

        Frequency frequency() const { return frequency_; }
        const DayCounter& dayCounter() const { return dayCounter_; }
      protected:
        Frequency frequency_;
        DayCounter dayCounter_;
    };

    //! returns a schedule for French amortization
    Schedule sinkingSchedule(const Date& startDate,
                             const Period& bondLength,
                             const Frequency& frequency,
                             const Calendar& paymentCalendar);

    //! returns a sequence of notionals for French amortization
    std::vector<Real> sinkingNotionals(const Period& bondLength,
                                       const Frequency& frequency,
                                       Rate couponRate,
                                       Real initialNotional);

}

#endif
