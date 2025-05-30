/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2009 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2003 RiskMap srl
 Copyright (C) 2015 Maddalena Zanzi
 Copyright (c) 2015 Klaus Spanderen
 Copyright (C) 2020 Leonardo Arcari
 Copyright (C) 2020 Kline s.r.l.

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <time/date.hpp>
#include <time/timeunit.hpp>
#include <time/imm.hpp>
#include <time/ecb.hpp>
#include <time/asx.hpp>
#include <utilities/dataparsers.hpp>

#include <sstream>
#include <unordered_set>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(DateTests)

BOOST_AUTO_TEST_CASE(ecbIsECBcode) {
    BOOST_TEST_MESSAGE("Testing ECB codes for validity...");

    BOOST_TEST(ECB::isECBcode("JAN00"));
    BOOST_TEST(ECB::isECBcode("FEB78"));
    BOOST_TEST(ECB::isECBcode("mar58"));
    BOOST_TEST(ECB::isECBcode("aPr99"));

    BOOST_TEST(!ECB::isECBcode(""));
    BOOST_TEST(!ECB::isECBcode("JUNE99"));
    BOOST_TEST(!ECB::isECBcode("JUN1999"));
    BOOST_TEST(!ECB::isECBcode("JUNE"));
    BOOST_TEST(!ECB::isECBcode("JUNE1999"));
    BOOST_TEST(!ECB::isECBcode("1999"));
}

BOOST_AUTO_TEST_CASE(ecbDates) {
    BOOST_TEST_MESSAGE("Testing ECB dates...");

    const std::set<Date> knownDates = ECB::knownDates();
    BOOST_TEST(!knownDates.empty(),
                   "empty ECB date vector");

    const Size n = ECB::nextDates(Date::minDate()).size();
    BOOST_TEST(n == knownDates.size(),
                   "nextDates(minDate) returns "  << n <<
                   " instead of " << knownDates.size() << " dates");

    Date previousEcbDate = Date::minDate();
    for (const Date& currentEcbDate : knownDates) {
        if (!ECB::isECBdate(currentEcbDate))
            BOOST_FAIL(currentEcbDate << " fails isECBdate check");

        const Date ecbDateMinusOne = currentEcbDate-1;
        if (ECB::isECBdate(ecbDateMinusOne))
            BOOST_FAIL(ecbDateMinusOne << " fails isECBdate check");

        if (ECB::nextDate(ecbDateMinusOne) != currentEcbDate)
            BOOST_FAIL("next ECB date following " << ecbDateMinusOne <<
                       " must be " << currentEcbDate);

        if (ECB::nextDate(previousEcbDate) != currentEcbDate)
            BOOST_FAIL("next ECB date following " << previousEcbDate <<
                       " must be " << currentEcbDate);

        previousEcbDate = currentEcbDate;
    }

    const Date knownDate = *knownDates.begin();
    ECB::removeDate(knownDate);
    BOOST_TEST(!ECB::isECBdate(knownDate),
                   "unable to remove an ECB date");
    ECB::addDate(knownDate);
    BOOST_TEST(ECB::isECBdate(knownDate),
                   "unable to add an ECB date");
}

BOOST_AUTO_TEST_CASE(ecbGetDateFromCode) {
    BOOST_TEST_MESSAGE("Testing conversion of ECB codes to dates...");

    const Date ref2000((Day)1, January, (Year)2000);
    BOOST_TEST(ECB::date("JAN05", ref2000) == Date((Day)19, January,  (Year)2005));
    BOOST_TEST(ECB::date("FEB06", ref2000) == Date((Day) 8, February, (Year)2006));
    BOOST_TEST(ECB::date("MAR07", ref2000) == Date((Day)14, March,    (Year)2007));
    BOOST_TEST(ECB::date("APR08", ref2000) == Date((Day)16, April,    (Year)2008));
    BOOST_TEST(ECB::date("JUN09", ref2000) == Date((Day)10, June,     (Year)2009));
    BOOST_TEST(ECB::date("JUL10") == Date((Day)14, July,      (Year)2010));
    BOOST_TEST(ECB::date("AUG11") == Date((Day)10, August,    (Year)2011));
    BOOST_TEST(ECB::date("SEP12") == Date((Day)12, September, (Year)2012));
    BOOST_TEST(ECB::date("OCT13") == Date((Day) 9, October,   (Year)2013));
    BOOST_TEST(ECB::date("NOV14") == Date((Day)12, November,  (Year)2014));
    BOOST_TEST(ECB::date("DEC15") == Date((Day) 9, December,  (Year)2015));
}

BOOST_AUTO_TEST_CASE(ecbGetCodeFromDate) {
    BOOST_TEST_MESSAGE("Testing creation of ECB code from a given date...");

    BOOST_TEST("JAN06" == ECB::code(Date((Day)18, January,  (Year)2006)));
    BOOST_TEST("MAR10" == ECB::code(Date((Day)10, March,    (Year)2010)));
    BOOST_TEST("NOV17" == ECB::code(Date((Day) 1, November, (Year)2017)));
}

BOOST_AUTO_TEST_CASE(ecbNextCode) {
    BOOST_TEST_MESSAGE("Testing calculation of the next ECB code from a given code...");

    BOOST_TEST("FEB06" == ECB::nextCode("JAN06"));
    BOOST_TEST("MAR10" == ECB::nextCode("FeB10"));
    BOOST_TEST("NOV17" == ECB::nextCode("OCT17"));
    BOOST_TEST("JAN18" == ECB::nextCode("dEC17"));
    BOOST_TEST("JAN00" == ECB::nextCode("dec99"));
}

BOOST_AUTO_TEST_CASE(immDates) {
    BOOST_TEST_MESSAGE("Testing IMM dates...");

    const std::string IMMcodes[] = {
        "F0", "G0", "H0", "J0", "K0", "M0", "N0", "Q0", "U0", "V0", "X0", "Z0",
        "F1", "G1", "H1", "J1", "K1", "M1", "N1", "Q1", "U1", "V1", "X1", "Z1",
        "F2", "G2", "H2", "J2", "K2", "M2", "N2", "Q2", "U2", "V2", "X2", "Z2",
        "F3", "G3", "H3", "J3", "K3", "M3", "N3", "Q3", "U3", "V3", "X3", "Z3",
        "F4", "G4", "H4", "J4", "K4", "M4", "N4", "Q4", "U4", "V4", "X4", "Z4",
        "F5", "G5", "H5", "J5", "K5", "M5", "N5", "Q5", "U5", "V5", "X5", "Z5",
        "F6", "G6", "H6", "J6", "K6", "M6", "N6", "Q6", "U6", "V6", "X6", "Z6",
        "F7", "G7", "H7", "J7", "K7", "M7", "N7", "Q7", "U7", "V7", "X7", "Z7",
        "F8", "G8", "H8", "J8", "K8", "M8", "N8", "Q8", "U8", "V8", "X8", "Z8",
        "F9", "G9", "H9", "J9", "K9", "M9", "N9", "Q9", "U9", "V9", "X9", "Z9"
    };

    Date counter = { 1, January, 2000 };
    Date last = { 1, January, 2040 };
    Date imm;

    while (counter<=last) {
        imm = IMM::nextDate(counter, false);

        // check that imm is greater than counter
        if (imm<=counter)
            BOOST_FAIL(imm.weekday() << " " << imm
                       << " is not greater than "
                       << counter.weekday() << " " << counter);

        // check that imm is an IMM date
        if (!IMM::isIMMdate(imm, false))
            BOOST_FAIL(imm.weekday() << " " << imm
                       << " is not an IMM date (calculated from "
                       << counter.weekday() << " " << counter << ")");

        // check that imm is <= to the next IMM date in the main cycle
        if (imm>IMM::nextDate(counter, true))
            BOOST_FAIL(imm.weekday() << " " << imm
                       << " is not less than or equal to the next future in the main cycle "
                       << IMM::nextDate(counter, true));

        // check that for every date IMMdate is the inverse of IMMcode
        if (IMM::date(IMM::code(imm), counter) != imm)
            BOOST_FAIL(IMM::code(imm)
                       << " at calendar day " << counter
                       << " is not the IMM code matching " << imm);

        // check that for every date the 120 IMM codes refer to future dates
        for (int i=0; i<40; ++i) {
            if (IMM::date(IMMcodes[i], counter)<counter)
                BOOST_FAIL(IMM::date(IMMcodes[i], counter)
                           << " is wrong for " << IMMcodes[i]
                           << " at reference date " << counter);
        }

        counter = counter + 1;
    }
}

BOOST_AUTO_TEST_CASE(asxDates) {
    BOOST_TEST_MESSAGE("Testing ASX dates...");

    const std::string ASXcodes[] = {
        "F0", "G0", "H0", "J0", "K0", "M0", "N0", "Q0", "U0", "V0", "X0", "Z0",
        "F1", "G1", "H1", "J1", "K1", "M1", "N1", "Q1", "U1", "V1", "X1", "Z1",
        "F2", "G2", "H2", "J2", "K2", "M2", "N2", "Q2", "U2", "V2", "X2", "Z2",
        "F3", "G3", "H3", "J3", "K3", "M3", "N3", "Q3", "U3", "V3", "X3", "Z3",
        "F4", "G4", "H4", "J4", "K4", "M4", "N4", "Q4", "U4", "V4", "X4", "Z4",
        "F5", "G5", "H5", "J5", "K5", "M5", "N5", "Q5", "U5", "V5", "X5", "Z5",
        "F6", "G6", "H6", "J6", "K6", "M6", "N6", "Q6", "U6", "V6", "X6", "Z6",
        "F7", "G7", "H7", "J7", "K7", "M7", "N7", "Q7", "U7", "V7", "X7", "Z7",
        "F8", "G8", "H8", "J8", "K8", "M8", "N8", "Q8", "U8", "V8", "X8", "Z8",
        "F9", "G9", "H9", "J9", "K9", "M9", "N9", "Q9", "U9", "V9", "X9", "Z9"
    };

    for (Date counter(1, January, 2000); counter <= Date(1,January, 2040); ++counter) {
        const Date asx = ASX::nextDate(counter, false);

        // check that asx is greater than counter
        if (asx <= counter)
            BOOST_FAIL(asx.weekday() << " " << asx
                       << " is not greater than "
                       << counter.weekday() << " " << counter);

        // check that asx is an ASX date
        if (!ASX::isASXdate(asx, false))
            BOOST_FAIL(asx.weekday() << " " << asx
                       << " is not an ASX date (calculated from "
                       << counter.weekday() << " " << counter << ")");

        // check that asx is <= to the next ASX date in the main cycle
        if (asx > ASX::nextDate(counter, true))
            BOOST_FAIL(asx.weekday() << " " << asx
                       << " is not less than or equal to the next future in the main cycle "
                       << ASX::nextDate(counter, true));

        // check that for every date ASXdate is the inverse of ASXcode
        if (ASX::date(ASX::code(asx), counter) != asx)
            BOOST_FAIL(ASX::code(asx)
                       << " at calendar day " << counter
                       << " is not the ASX code matching " << asx);

        // check that for every date the 120 ASX codes refer to future dates
        for (const auto& ASXcode : ASXcodes) {
            if (ASX::date(ASXcode, counter) < counter)
                BOOST_FAIL(ASX::date(ASXcode, counter) << " is wrong for " << ASXcode
                           << " at reference date " << counter);
        }
    }
}


BOOST_AUTO_TEST_CASE(asxDatesSpecific) {
    BOOST_TEST_MESSAGE("Testing ASX functionality with specific dates...");

    // isASXdate
    {
        // date is ASX date depending on mainCycle
        const Date date((Day)12, January, (Year)2024);
        BOOST_ASSERT(date.weekday() == Friday);

        // check mainCycle
        BOOST_TEST(ASX::isASXdate(date, /*mainCycle*/false));
        BOOST_TEST(!ASX::isASXdate(date, /*mainCycle*/true));
    }

    // nextDate from code + ref date
    BOOST_TEST(Date((Day)8, February, (Year)2002)
        == ASX::nextDate("F2", /*mainCycle*/false, Date((Day)1, January, (Year)2000)));

    BOOST_TEST(Date((Day)9, June, (Year)2023)
        == ASX::nextDate("K3", /*mainCycle*/true, Date((Day)1, January, (Year)2014)));

    // nextCode
    BOOST_TEST("F4" == ASX::nextCode(Date((Day)1, January, (Year)2024), /*mainCycle*/false));
    BOOST_TEST("G4" == ASX::nextCode(Date((Day)15, January, (Year)2024), /*mainCycle*/false));
    BOOST_TEST("H4" == ASX::nextCode(Date((Day)15, January, (Year)2024), /*mainCycle*/true));

    BOOST_TEST("G4" == ASX::nextCode("F4", /*mainCycle*/false, Date((Day)1, January, (Year)2020)));
    BOOST_TEST("H5" == ASX::nextCode("Z4", /*mainCycle*/true, Date((Day)1, January, (Year)2020)));
}

BOOST_AUTO_TEST_CASE(testConsistency) {

    BOOST_TEST_MESSAGE("Testing dates...");

    Date::serial_type minDate = Date::minDate().serialNumber()+1,
                      maxDate = Date::maxDate().serialNumber();

    Date::serial_type dyold = Date(minDate-1).dayOfYear(),
                      dold  = Date(minDate-1).dayOfMonth(),
                      mold  = Date(minDate-1).month(),
                      yold  = Date(minDate-1).year(),
                      wdold = Date(minDate-1).weekday();

    for (Date::serial_type i=minDate; i<=maxDate; i++) {
        Date t(i);
        Date::serial_type serial = t.serialNumber();

        // check serial number consistency
        if (serial != i)
            BOOST_FAIL("inconsistent serial number:\n"
                       << "    original:      " << i << "\n"
                       << "    date:          " << t << "\n"
                       << "    serial number: " << serial);

        Integer dy = t.dayOfYear(),
                d  = t.dayOfMonth(),
                m  = t.month(),
                y  = t.year(),
                wd = t.weekday();

        // check if skipping any date
        if (!((dy == dyold+1) ||
              (dy == 1 && dyold == 365 && !Date::isLeap(yold)) ||
              (dy == 1 && dyold == 366 && Date::isLeap(yold))))
            BOOST_FAIL("wrong day of year increment: \n"
                       << "    date: " << t << "\n"
                       << "    day of year: " << dy << "\n"
                       << "    previous:    " << dyold);
        dyold = dy;

        if (!((d == dold+1 && m == mold   && y == yold) ||
              (d == 1      && m == mold+1 && y == yold) ||
              (d == 1      && m == 1      && y == yold+1)))
            BOOST_FAIL("wrong day,month,year increment: \n"
                       << "    date: " << t << "\n"
                       << "    day,month,year: "
                       << d << "," << Integer(m) << "," << y << "\n"
                       << "    previous:       "
                       << dold<< "," << Integer(mold) << "," << yold);
        dold = d; mold = m; yold = y;

        // check month definition
        if (m < 1 || m > 12)
            BOOST_FAIL("invalid month: \n"
                       << "    date:  " << t << "\n"
                       << "    month: " << Integer(m));

        // check day definition
        if (d < 1)
            BOOST_FAIL("invalid day of month: \n"
                       << "    date:  " << t << "\n"
                       << "    day: " << d);
        if (!((m == 1  && d <= 31) ||
              (m == 2  && d <= 28) ||
              (m == 2  && d == 29 && Date::isLeap(y)) ||
              (m == 3  && d <= 31) ||
              (m == 4  && d <= 30) ||
              (m == 5  && d <= 31) ||
              (m == 6  && d <= 30) ||
              (m == 7  && d <= 31) ||
              (m == 8  && d <= 31) ||
              (m == 9  && d <= 30) ||
              (m == 10 && d <= 31) ||
              (m == 11 && d <= 30) ||
              (m == 12 && d <= 31)))
            BOOST_FAIL("invalid day of month: \n"
                       << "    date:  " << t << "\n"
                       << "    day: " << d);

        // check weekday definition
        if (!((Integer(wd) == Integer(wdold+1)) ||
              (Integer(wd) == 1 && Integer(wdold) == 7)))
            BOOST_FAIL("invalid weekday: \n"
                       << "    date:  " << t << "\n"
                       << "    weekday:  " << Integer(wd) << "\n"
                       << "    previous: " << Integer(wdold));
        wdold = wd;

        // create the same date with a different constructor
        Date s(d,Month(m),y);
        // check serial number consistency
        serial = s.serialNumber();
        if (serial != i)
            BOOST_FAIL("inconsistent serial number:\n"
                       << "    date:          " << t << "\n"
                       << "    serial number: " << i << "\n"
                       << "    cloned date:   " <<  s << "\n"
                       << "    serial number: " << serial);
    }

}

BOOST_AUTO_TEST_CASE(isoDates) {
    BOOST_TEST_MESSAGE("Testing ISO dates...");
    std::string input_date("2006-01-15");
    Date d = DateParser::parseISO(input_date);
    if (d.dayOfMonth() != 15 ||
        d.month() != January ||
        d.year() != 2006) {
        BOOST_FAIL("Iso date failed\n"
                   << " input date:    " << input_date << "\n"
                   << " day of month:  " << d.dayOfMonth() << "\n"
                   << " month:         " << d.month() << "\n"
                   << " year:          " << d.year());
    }
}

#ifndef QL_PATCH_SOLARIS
BOOST_AUTO_TEST_CASE(parseDates) {
    BOOST_TEST_MESSAGE("Testing parsing of dates...");

    std::string input_date("2006-01-15");
    Date d = DateParser::parseFormatted(input_date, "%Y-%m-%d");
    if (d != Date(15, January, 2006)) {
        BOOST_FAIL("date parsing failed\n"
                   << " input date:  " << input_date << "\n"
                   << " parsed date: " << d);
    }

    input_date = "12/02/2012";
    d = DateParser::parseFormatted(input_date, "%m/%d/%Y");
    if (d != Date(2, December, 2012)) {
        BOOST_FAIL("date parsing failed\n"
                   << " input date:  " << input_date << "\n"
                   << " parsed date: " << d);
    }
    d = DateParser::parseFormatted(input_date, "%d/%m/%Y");
    if (d != Date(12, February, 2012)) {
        BOOST_FAIL("date parsing failed\n"
                   << " input date:  " << input_date << "\n"
                   << " parsed date: " << d);
    }

    input_date = "20011002";
    d = DateParser::parseFormatted(input_date, "%Y%m%d");
    if (d != Date(2, October, 2001)) {
        BOOST_FAIL("date parsing failed\n"
                   << " input date:  " << input_date << "\n"
                   << " parsed date: " << d);
    }
}
#endif

#ifdef QL_HIGH_RESOLUTION_DATE
BOOST_AUTO_TEST_CASE(intraday) {

    BOOST_TEST_MESSAGE("Testing intraday information of dates...");

    const Date d1 = Date(12, February, 2015, 10, 45, 12, 1234, 76253);

    BOOST_CHECK_MESSAGE(d1.year() == 2015, "failed to reproduce year");
    BOOST_CHECK_MESSAGE(d1.month() == February, "failed to reproduce month");
    BOOST_CHECK_MESSAGE(d1.dayOfMonth() == 12, "failed to reproduce day");
    BOOST_CHECK_MESSAGE(d1.hours() == 10, "failed to reproduce hour of day");
    BOOST_CHECK_MESSAGE(d1.minutes() == 45,
        "failed to reproduce minute of hour");
    BOOST_CHECK_MESSAGE(d1.seconds() == 13,
        "failed to reproduce second of minute");

    if (Date::ticksPerSecond() == 1000)
        BOOST_CHECK_MESSAGE(d1.fractionOfSecond() == 0.234,
            "failed to reproduce fraction of second");
    else if (Date::ticksPerSecond() >= 1000000)
        BOOST_CHECK_MESSAGE(d1.fractionOfSecond() == (234000 + 76253)/1000000.0,
        "failed to reproduce fraction of second");

    if (Date::ticksPerSecond() >= 1000)
        BOOST_CHECK_MESSAGE(d1.milliseconds() == 234 + 76,
            "failed to reproduce number of milliseconds");

    if (Date::ticksPerSecond() >= 1000000)
        BOOST_CHECK_MESSAGE(d1.microseconds() == 253,
            "failed to reproduce number of microseconds");

    const Date d2 = Date(28, February, 2015, 50, 165, 476, 1234, 253);
    BOOST_CHECK_MESSAGE(d2.year() == 2015, "failed to reproduce year");
    BOOST_CHECK_MESSAGE(d2.month() == March, "failed to reproduce month");
    BOOST_CHECK_MESSAGE(d2.dayOfMonth() == 2, "failed to reproduce day");
    BOOST_CHECK_MESSAGE(d2.hours() == 4, "failed to reproduce hour of day");
    BOOST_CHECK_MESSAGE(d2.minutes() == 52,
        "failed to reproduce minute of hour");
    BOOST_CHECK_MESSAGE(d2.seconds() == 57,
        "failed to reproduce second of minute");

    if (Date::ticksPerSecond() >= 1000)
        BOOST_CHECK_MESSAGE(d2.milliseconds() == 234,
            "failed to reproduce number of milliseconds");
    if (Date::ticksPerSecond() >= 1000000)
        BOOST_CHECK_MESSAGE(d2.microseconds() == 253,
            "failed to reproduce number of microseconds");

    std::ostringstream s;
    s << io::iso_datetime(Date(7, February, 2015, 1, 4, 2, 3, 4));

    BOOST_CHECK_MESSAGE(s.str() == std::string("2015-02-07T01:04:02,003004"),
        "datetime to string failed to reproduce expected result");

    const Date d3 = Date(10, April, 2023, 11, 43, 13, 234, 253);

    BOOST_CHECK_MESSAGE(d3 + Period(23, Hours) ==
            Date(11, April, 2023, 10, 43, 13, 234, 253), "failed to add hours");

    BOOST_CHECK_MESSAGE(d3 + Period(2, Minutes) ==
            Date(10, April, 2023, 11, 45, 13, 234, 253), "failed to add minutes");

    BOOST_CHECK_MESSAGE(d3 + Period(-2, Seconds) ==
            Date(10, April, 2023, 11, 43, 11, 234, 253), "failed to add seconds");

    BOOST_CHECK_MESSAGE(d3 + Period(-20, Milliseconds) ==
            Date(10, April, 2023, 11, 43, 13, 214, 253), "failed to add milliseconds");

    BOOST_CHECK_MESSAGE(d3 + Period(20, Microseconds) ==
            Date(10, April, 2023, 11, 43, 13, 234, 273), "failed to add microseconds");

}
#endif

BOOST_AUTO_TEST_CASE(canHash) {
    BOOST_TEST_MESSAGE("Testing hashing of dates...");

    Date start_date = Date(1, Jan, 2020);
    int nb_tests = 500;

    std::hash<Date> hasher;

    // Check hash values
    for (int i = 0; i < nb_tests; ++i) {
        for (int j = 0; j < nb_tests; ++j) {
            Date lhs = start_date + i;
            Date rhs = start_date + j;

            if (lhs == rhs && hasher(lhs) != hasher(rhs)) {
                BOOST_FAIL("Equal dates are expected to have same hash value\n"
                           << "rhs = " << lhs << '\n'
                           << "lhs = " << rhs << '\n'
                           << "hash(lhs) = " << hasher(lhs) << '\n'
                           << "hash(rhs) = " << hasher(rhs) << '\n');
            }

            if (lhs != rhs && hasher(lhs) == hasher(rhs)) {
                BOOST_FAIL("Different dates are expected to have different hash value\n"
                           << "rhs = " << lhs << '\n'
                           << "lhs = " << rhs << '\n'
                           << "hash(lhs) = " << hasher(lhs) << '\n'
                           << "hash(rhs) = " << hasher(rhs) << '\n');
            }
        }
    }

    // Check if Date can be used as unordered_set key
    std::unordered_set<Date> set;
    set.insert(start_date);

    if (set.count(start_date) == 0) {
        BOOST_FAIL("Expected to find date " << start_date << " in unordered_set\n");
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
