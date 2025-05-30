/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2022 Marcin Rybacki

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
#include <time/calendars/target.hpp>
#include <time/daycounters/actualactual.hpp>
#include <instruments/bondforward.hpp>
#include <instruments/bonds/fixedratebond.hpp>
#include <pricingengines/bond/discountingbondengine.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(BondForwardTests)

struct CommonVars {
    // common data
    Date today;
    RelinkableHandle<YieldTermStructure> curveHandle;

    // setup
    CommonVars() {
        today = Date(7, March, 2022);
        Settings::instance().evaluationDate() = today;

        curveHandle.linkTo(flatRate(today, 0.0004977, Actual365Fixed()));
    }
};

ext::shared_ptr<Bond> buildBond(const Date &issue, 
                                const Date &maturity, 
                                Rate cpn) {
    Schedule sch(issue, maturity, Period(Annual), TARGET(), Following, Following,
                 DateGeneration::Backward, false);

    return ext::make_shared<FixedRateBond>(2, 1.e5, sch, std::vector<Rate>(1, cpn),
                                           ActualActual(ActualActual::ISDA));
}

ext::shared_ptr<BondForward> buildBondForward(const ext::shared_ptr<Bond>& underlying,
                                              const Handle<YieldTermStructure> &handle,
                                              const Date& delivery, 
                                              Position::Type type) {
    auto valueDt = handle->referenceDate();
    return ext::make_shared<BondForward>(valueDt, delivery, type, 0.0, 2,
                                         ActualActual(ActualActual::ISDA), 
                                         TARGET(), Following, underlying, handle, handle);
}


BOOST_AUTO_TEST_CASE(testFuturesPriceReplication) {
    BOOST_TEST_MESSAGE("Testing futures price replication...");

    CommonVars vars;

    Real tolerance = 1.0e-2;

    Date issue(15, August, 2015);
    Date maturity(15, August, 2046);
    Rate cpn = 0.025;

    auto bnd = buildBond(issue, maturity, cpn);
    auto pricer = ext::make_shared<DiscountingBondEngine>(vars.curveHandle);
    bnd->setPricingEngine(pricer);

    Date delivery(10, March, 2022);
    Real conversionFactor = 0.76871;
    auto bndFwd = buildBondForward(bnd, vars.curveHandle, delivery, Position::Long);

    auto futuresPrice = bndFwd->cleanForwardPrice() / conversionFactor;
    auto expectedFuturesPrice = 207.47;

    if (std::fabs(futuresPrice - expectedFuturesPrice) > tolerance)
        BOOST_ERROR("unable to replicate bond futures price\n"
                    << std::setprecision(5) << "    calculated:    " << futuresPrice << "\n"
                    << "    expected:    " << expectedFuturesPrice << "\n");
}

BOOST_AUTO_TEST_CASE(testCleanForwardPriceReplication) {
    BOOST_TEST_MESSAGE("Testing clean forward price replication...");

    CommonVars vars;

    Real tolerance = 1.0e-2;

    Date issue(15, August, 2015);
    Date maturity(15, August, 2046);
    Rate cpn = 0.025;

    auto bnd = buildBond(issue, maturity, cpn);
    auto pricer = ext::make_shared<DiscountingBondEngine>(vars.curveHandle);
    bnd->setPricingEngine(pricer);

    Date delivery(10, March, 2022);
    auto bndFwd = buildBondForward(bnd, vars.curveHandle, delivery, Position::Long);

    auto fwdCleanPrice = bndFwd->cleanForwardPrice();
    auto expectedFwdCleanPrice = bndFwd->forwardValue() - bnd->accruedAmount(delivery);

    if (std::fabs(fwdCleanPrice - expectedFwdCleanPrice) > tolerance)
        BOOST_ERROR("unable to replicate clean forward price\n"
                    << std::setprecision(5) << "    calculated:    " << fwdCleanPrice << "\n"
                    << "    expected:    " << expectedFwdCleanPrice << "\n");
}

BOOST_AUTO_TEST_CASE(testThatForwardValueIsEqualToSpotValueIfNoIncome) {
    BOOST_TEST_MESSAGE(
        "Testing that forward value is equal to spot value if no income...");

    CommonVars vars;

    Real tolerance = 1.0e-2;

    Date issue(15, August, 2015);
    Date maturity(15, August, 2046);
    Rate cpn = 0.025;

    auto bnd = buildBond(issue, maturity, cpn);
    auto pricer = ext::make_shared<DiscountingBondEngine>(vars.curveHandle);
    bnd->setPricingEngine(pricer);

    Date delivery(10, March, 2022);
    auto bndFwd = buildBondForward(bnd, vars.curveHandle, delivery, Position::Long);

    auto bndFwdValue = bndFwd->forwardValue();
    auto underlyingDirtyPrice = bnd->dirtyPrice();

    if (std::fabs(bndFwdValue - underlyingDirtyPrice) > tolerance)
        BOOST_ERROR("unable to match the dirty price \n"
                    << std::setprecision(5) << "    bond forward:    " << bndFwdValue << "\n"
                    << "    underlying bond:    " << underlyingDirtyPrice << "\n");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
