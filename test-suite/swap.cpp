/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2007 StatPro Italia srl

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
#include <instruments/vanillaswap.hpp>
#include <pricingengines/swap/discountingswapengine.hpp>
#include <termstructures/yield/flatforward.hpp>
#include <time/calendars/nullcalendar.hpp>
#include <time/daycounters/thirty360.hpp>
#include <time/daycounters/actual365fixed.hpp>
#include <time/daycounters/simpledaycounter.hpp>
#include <time/schedule.hpp>
#include <indexes/ibor/euribor.hpp>
#include <cashflows/iborcoupon.hpp>
#include <cashflows/cashflowvectors.hpp>
#include <termstructures/volatility/optionlet/constantoptionletvol.hpp>
#include <utilities/dataformatters.hpp>
#include <cashflows/cashflows.hpp>
#include <cashflows/couponpricer.hpp>
#include <currencies/europe.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(SwapTests)

struct CommonVars {
    // global data
    Date today, settlement;
    Swap::Type type;
    Real nominal;
    Calendar calendar;
    BusinessDayConvention fixedConvention, floatingConvention;
    Frequency fixedFrequency, floatingFrequency;
    DayCounter fixedDayCount;
    ext::shared_ptr<IborIndex> index;
    Natural settlementDays;
    RelinkableHandle<YieldTermStructure> termStructure;

    // utilities
    ext::shared_ptr<VanillaSwap>
    makeSwap(Integer length, Rate fixedRate, Spread floatingSpread, DateGeneration::Rule rule = DateGeneration::Forward) const {
        Date maturity = calendar.advance(settlement,length,Years,
                                         floatingConvention);
        Schedule fixedSchedule(settlement,maturity,Period(fixedFrequency),
                               calendar,fixedConvention,fixedConvention, rule, false);
        Schedule floatSchedule(settlement,maturity,
                               Period(floatingFrequency),
                               calendar,floatingConvention,
                               floatingConvention, rule, false);
        ext::shared_ptr<VanillaSwap> swap(
                new VanillaSwap(type, nominal,
                                fixedSchedule, fixedRate, fixedDayCount,
                                floatSchedule, index, floatingSpread,
                                index->dayCounter()));
        swap->setPricingEngine(ext::shared_ptr<PricingEngine>(
                                  new DiscountingSwapEngine(termStructure)));
        return swap;
    }

    CommonVars() {
        type = Swap::Payer;
        settlementDays = 2;
        nominal = 100.0;
        fixedConvention = Unadjusted;
        floatingConvention = ModifiedFollowing;
        fixedFrequency = Annual;
        floatingFrequency = Semiannual;
        fixedDayCount = Thirty360(Thirty360::BondBasis);
        index = ext::shared_ptr<IborIndex>(new
                Euribor(Period(floatingFrequency), termStructure));
        calendar = index->fixingCalendar();
        today = calendar.adjust(Settings::instance().evaluationDate());
        settlement = calendar.advance(today,settlementDays,Days);
        termStructure.linkTo(flatRate(settlement,0.05,Actual365Fixed()));
    }
};


BOOST_AUTO_TEST_CASE(testFairRate) {

    BOOST_TEST_MESSAGE("Testing vanilla-swap calculation of fair fixed rate...");

    CommonVars vars;

    Integer lengths[] = { 1, 2, 5, 10, 20 };
    Spread spreads[] = { -0.001, -0.01, 0.0, 0.01, 0.001 };

    for (int& length : lengths) {
        for (Real spread : spreads) {

            ext::shared_ptr<VanillaSwap> swap = vars.makeSwap(length, 0.0, spread);
            swap = vars.makeSwap(length, swap->fairRate(), spread);
            if (std::fabs(swap->NPV()) > 1.0e-10) {
                BOOST_ERROR("recalculating with implied rate:\n"
                            << std::setprecision(2) << "    length: " << length << " years\n"
                            << "    floating spread: " << io::rate(spread) << "\n"
                            << "    swap value: " << swap->NPV());
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testFairSpread) {

    BOOST_TEST_MESSAGE("Testing vanilla-swap calculation of "
                       "fair floating spread...");

    CommonVars vars;

    Integer lengths[] = { 1, 2, 5, 10, 20 };
    Rate rates[] = { 0.04, 0.05, 0.06, 0.07 };

    for (int& length : lengths) {
        for (Real j : rates) {

            ext::shared_ptr<VanillaSwap> swap = vars.makeSwap(length, j, 0.0);
            swap = vars.makeSwap(length, j, swap->fairSpread());
            if (std::fabs(swap->NPV()) > 1.0e-10) {
                BOOST_ERROR("recalculating with implied spread:\n"
                            << std::setprecision(2) << "    length: " << length << " years\n"
                            << "    fixed rate: " << io::rate(j) << "\n"
                            << "    swap value: " << swap->NPV());
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testRateDependency) {

    BOOST_TEST_MESSAGE("Testing vanilla-swap dependency on fixed rate...");

    CommonVars vars;

    Integer lengths[] = { 1, 2, 5, 10, 20 };
    Spread spreads[] = { -0.001, -0.01, 0.0, 0.01, 0.001 };
    Rate rates[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };

    for (int& length : lengths) {
        for (Real spread : spreads) {
            // store the results for different rates...
            std::vector<Real> swap_values;
            for (Real rate : rates) {
                ext::shared_ptr<VanillaSwap> swap = vars.makeSwap(length, rate, spread);
                swap_values.push_back(swap->NPV());
            }
            // and check that they go the right way
            auto it = std::adjacent_find(swap_values.begin(), swap_values.end(), std::less<>());
            if (it != swap_values.end()) {
                Size n = it - swap_values.begin();
                BOOST_ERROR("NPV is increasing with the fixed rate in a swap: \n"
                            << "    length: " << length << " years\n"
                            << "    value:  " << swap_values[n]
                            << " paying fixed rate: " << io::rate(rates[n]) << "\n"
                            << "    value:  " << swap_values[n + 1]
                            << " paying fixed rate: " << io::rate(rates[n + 1]));
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testSpreadDependency) {

    BOOST_TEST_MESSAGE("Testing vanilla-swap dependency on floating spread...");

    CommonVars vars;

    Integer lengths[] = { 1, 2, 5, 10, 20 };
    Rate rates[] = { 0.04, 0.05, 0.06, 0.07 };
    Spread spreads[] = { -0.01, -0.002, -0.001, 0.0, 0.001, 0.002, 0.01 };

    for (int& length : lengths) {
        for (Real j : rates) {
            // store the results for different spreads...
            std::vector<Real> swap_values;
            for (Real spread : spreads) {
                ext::shared_ptr<VanillaSwap> swap = vars.makeSwap(length, j, spread);
                swap_values.push_back(swap->NPV());
            }
            // and check that they go the right way
            auto it =
                std::adjacent_find(swap_values.begin(), swap_values.end(), std::greater<>());
            if (it != swap_values.end()) {
                Size n = it - swap_values.begin();
                BOOST_ERROR("NPV is decreasing with the floating spread in a swap: \n"
                            << "    length: " << length << " years\n"
                            << "    value:  " << swap_values[n]
                            << " receiving spread: " << io::rate(spreads[n]) << "\n"
                            << "    value:  " << swap_values[n + 1]
                            << " receiving spread: " << io::rate(spreads[n + 1]));
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testInArrears) {

    BOOST_TEST_MESSAGE("Testing in-arrears swap calculation...");

    CommonVars vars;

    /* See Hull, 4th ed., page 550
       Note: the calculation in the book is wrong (work out the
       adjustment and you'll get 0.05 + 0.000115 T1)
    */

    Date maturity = vars.today + 5*Years;
    Calendar calendar = NullCalendar();
    Schedule schedule(vars.today, maturity,Period(Annual),calendar,
                      Following,Following,
                      DateGeneration::Forward,false);
    DayCounter dayCounter = SimpleDayCounter();
    std::vector<Real> nominals(1, 100000000.0);
    ext::shared_ptr<IborIndex> index(new IborIndex("dummy", 1*Years, 0,
                                             EURCurrency(), calendar,
                                             Following, false, dayCounter,
                                             vars.termStructure));
    Rate oneYear = 0.05;
    Rate r = std::log(1.0+oneYear);
    vars.termStructure.linkTo(flatRate(vars.today,r,dayCounter));


    std::vector<Rate> coupons(1, oneYear);
    Leg fixedLeg = FixedRateLeg(schedule)
        .withNotionals(nominals)
        .withCouponRates(coupons, dayCounter);

    std::vector<Real> gearings;
    std::vector<Rate> spreads;
    Natural fixingDays = 0;

    Volatility capletVolatility = 0.22;
    Handle<OptionletVolatilityStructure> vol(
        ext::shared_ptr<OptionletVolatilityStructure>(new
            ConstantOptionletVolatility(vars.today, NullCalendar(), Following,
                                        capletVolatility, dayCounter)));
    ext::shared_ptr<IborCouponPricer> pricer(new
        BlackIborCouponPricer(vol));

    Leg floatingLeg = IborLeg(schedule, index)
        .withNotionals(nominals)
        .withPaymentDayCounter(dayCounter)
        .withFixingDays(fixingDays)
        .withGearings(gearings)
        .withSpreads(spreads)
        .inArrears();
    setCouponPricer(floatingLeg, pricer);

    Swap swap(floatingLeg,fixedLeg);
    swap.setPricingEngine(ext::shared_ptr<PricingEngine>(
                              new DiscountingSwapEngine(vars.termStructure)));

    Decimal storedValue = -144813.0;
    Real tolerance = 1.0;

    if (std::fabs(swap.NPV()-storedValue) > tolerance)
        BOOST_ERROR("Wrong NPV calculation:\n"
                    << "    expected:   " << storedValue << "\n"
                    << "    calculated: " << swap.NPV());
}

BOOST_AUTO_TEST_CASE(testCachedValue) {

    BOOST_TEST_MESSAGE("Testing vanilla-swap calculation against cached value...");

    bool usingAtParCoupons = IborCoupon::Settings::instance().usingAtParCoupons();

    CommonVars vars;

    vars.today = Date(17,June,2002);
    Settings::instance().evaluationDate() = vars.today;
    vars.settlement =
        vars.calendar.advance(vars.today,vars.settlementDays,Days);
    vars.termStructure.linkTo(flatRate(vars.settlement,0.05,Actual365Fixed()));

    ext::shared_ptr<VanillaSwap> swap = vars.makeSwap(10, 0.06, 0.001);

    if (swap->numberOfLegs() != 2)
        BOOST_ERROR("failed to return correct number of legs:\n"
                    << std::fixed << std::setprecision(12)
                    << "    calculated: " << swap->numberOfLegs() << "\n"
                    << "    expected:   " << 2);

    Real cachedNPV = usingAtParCoupons ? -5.872863313209 : -5.872342992212;

    if (std::fabs(swap->NPV()-cachedNPV) > 1.0e-11)
        BOOST_ERROR("failed to reproduce cached swap value:\n"
                    << std::fixed << std::setprecision(12)
                    << "    calculated: " << swap->NPV() << "\n"
                    << "    expected:   " << cachedNPV);
}

BOOST_AUTO_TEST_CASE(testThirdWednesdayAdjustment) {

    BOOST_TEST_MESSAGE("Testing third-Wednesday adjustment...");

    CommonVars vars;

    ext::shared_ptr<VanillaSwap> swap = vars.makeSwap(1, 0.0, -0.001, DateGeneration::ThirdWednesdayInclusive);

    if (swap->floatingSchedule().startDate() != Date(16, September, 2015)) {
        BOOST_ERROR("Wrong Start Date " << swap->floatingSchedule().startDate());
    }

     if (swap->floatingSchedule().endDate() != Date(21, September, 2016)) {
        BOOST_ERROR("Wrong End Date " << swap->floatingSchedule().endDate());
    }
}

BOOST_AUTO_TEST_CASE(testNotifications) {
    BOOST_TEST_MESSAGE("Testing cash-flow notifications for vanilla swap...");

    CommonVars vars;

    Date spot = vars.calendar.advance(vars.today, 2*Days);
    Real nominal = 100000.0;

    Schedule schedule = MakeSchedule()
        .from(spot)
        .to(vars.calendar.advance(spot, 2*Years))
        .withCalendar(vars.calendar)
        .withFrequency(Semiannual);

    RelinkableHandle<YieldTermStructure> forecast_handle;
    forecast_handle.linkTo(flatRate(0.02, Actual365Fixed()));

    RelinkableHandle<YieldTermStructure> discount_handle;
    discount_handle.linkTo(flatRate(0.02, Actual365Fixed()));
    
    auto index = ext::make_shared<Euribor6M>(forecast_handle);
    
    auto swap = ext::make_shared<VanillaSwap>(Swap::Payer,
                                              nominal,
                                              schedule,
                                              0.03,
                                              Actual365Fixed(),
                                              schedule,
                                              index,
                                              0.0,
                                              Actual365Fixed());
    swap->setPricingEngine(ext::make_shared<DiscountingSwapEngine>(discount_handle));
    swap->NPV();

    Flag flag;
    flag.registerWith(swap);
    flag.lower();

    forecast_handle.linkTo(flatRate(0.03, Actual365Fixed()));

    if (!flag.isUp())
        BOOST_FAIL("swap was not notified of curve change");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
