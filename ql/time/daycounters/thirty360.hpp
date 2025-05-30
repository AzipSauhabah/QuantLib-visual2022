/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2018 Alexey Indiryakov

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

/*! \file thirty360.hpp
    \brief 30/360 day counters
*/

#ifndef quantlib_thirty360_day_counter_h
#define quantlib_thirty360_day_counter_h

#include <time/daycounter.hpp>

namespace QuantLib {

    //! 30/360 day count convention
    /*! The 30/360 day count can be calculated according to a
        number of conventions.

        US convention: if the starting date is the 31st of a month or
        the last day of February, it becomes equal to the 30th of the
        same month.  If the ending date is the 31st of a month and the
        starting date is the 30th or 31th of a month, the ending date
        becomes equal to the 30th.  If the ending date is the last of
        February and the starting date is also the last of February,
        the ending date becomes equal to the 30th.
        Also known as "30/360" or "360/360".

        Bond Basis convention: if the starting date is the 31st of a
        month, it becomes equal to the 30th of the same month.
        If the ending date is the 31st of a month and the starting
        date is the 30th or 31th of a month, the ending date
        also becomes equal to the 30th of the month.
        Also known as "US (ISMA)".

        European convention: starting dates or ending dates that
        occur on the 31st of a month become equal to the 30th of the
        same month.
        Also known as "30E/360", or "Eurobond Basis".

        Italian convention: starting dates or ending dates that
        occur on February and are greater than 27 become equal to 30
        for computational sake.

        ISDA convention: starting or ending dates on the 31st of the
        month become equal to 30; starting dates or ending dates that
        occur on the last day of February also become equal to 30,
        except for the termination date.  Also known as "30E/360
        ISDA", "30/360 ISDA", or "30/360 German".

        NASD convention: if the starting date is the 31st of a
        month, it becomes equal to the 30th of the same month.
        If the ending date is the 31st of a month and the starting
        date is earlier than the 30th of a month, the ending date
        becomes equal to the 1st of the next month, otherwise the
        ending date becomes equal to the 30th of the same month.

        \ingroup daycounters
    */
    class Thirty360 : public DayCounter {
      public:
        enum Convention {
            USA,
            BondBasis,
            European,
            EurobondBasis,
            Italian,
            German,
            ISMA,
            ISDA,
            NASD
        };
      private:
        class Thirty360_Impl : public DayCounter::Impl {
          public:
            Time yearFraction(const Date& d1, const Date& d2,
                              const Date&, const Date&) const override {
                return dayCount(d1,d2)/360.0;
            }
        };
        class US_Impl final : public Thirty360_Impl {
          public:
            std::string name() const override { return std::string("30/360 (US)"); }
            Date::serial_type dayCount(const Date& d1, const Date& d2) const override;
        };
        class ISMA_Impl final : public Thirty360_Impl {
          public:
            std::string name() const override { return std::string("30/360 (Bond Basis)"); }
            Date::serial_type dayCount(const Date& d1, const Date& d2) const override;
        };
        class EU_Impl final : public Thirty360_Impl {
          public:
            std::string name() const override { return std::string("30E/360 (Eurobond Basis)"); }
            Date::serial_type dayCount(const Date& d1, const Date& d2) const override;
        };
        class IT_Impl final : public Thirty360_Impl {
          public:
            std::string name() const override { return std::string("30/360 (Italian)"); }
            Date::serial_type dayCount(const Date& d1, const Date& d2) const override;
        };
        class ISDA_Impl final : public Thirty360_Impl {
          public:
            explicit ISDA_Impl(const Date& terminationDate)
            : terminationDate_(terminationDate) {}
            std::string name() const override { return std::string("30E/360 (ISDA)"); }
            Date::serial_type dayCount(const Date& d1, const Date& d2) const override;
          private:
            Date terminationDate_;
        };
        class NASD_Impl final : public Thirty360_Impl {
          public:
            std::string name() const override { return std::string("30/360 (NASD)"); }
            Date::serial_type dayCount(const Date& d1, const Date& d2) const override;
        };
        static ext::shared_ptr<DayCounter::Impl>
        implementation(Convention c, const Date& terminationDate);
      public:
        explicit Thirty360(Convention c, const Date& terminationDate = Date())
        : DayCounter(implementation(c, terminationDate)) {}
    };

}

#endif
