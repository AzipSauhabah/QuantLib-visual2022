/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl

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
#include <instruments/compositeinstrument.hpp>
#include <instruments/europeanoption.hpp>
#include <instruments/stock.hpp>
#include <pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <quotes/simplequote.hpp>
#include <time/daycounters/actual360.hpp>

using namespace QuantLib;
using namespace boost::unit_test;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, QuantLib::TopLevelFixture)

BOOST_AUTO_TEST_SUITE(InstrumentTests)

BOOST_AUTO_TEST_CASE(testObservable) {

    BOOST_TEST_MESSAGE("Testing observability of instruments...");

    ext::shared_ptr<SimpleQuote> me1(new SimpleQuote(0.0));
    RelinkableHandle<Quote> h(me1);
    ext::shared_ptr<Instrument> s(new Stock(h));

    Flag f;
    f.registerWith(s);

    s->NPV();
    me1->setValue(3.14);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of instrument change");

    s->NPV();
    f.lower();
    ext::shared_ptr<SimpleQuote> me2(new SimpleQuote(0.0));
    h.linkTo(me2);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of instrument change");

    f.lower();
    s->freeze();
    s->NPV();
    me2->setValue(2.71);
    if (f.isUp())
        BOOST_FAIL("Observer was notified of frozen instrument change");
    s->NPV();
    s->unfreeze();
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of instrument change");
}

BOOST_AUTO_TEST_CASE(testCompositeWhenShiftingDates) {
    BOOST_TEST_MESSAGE(
        "Testing reaction of composite instrument to date changes...");

    Date today = Date::todaysDate();
    DayCounter dc = Actual360();

    ext::shared_ptr<StrikedTypePayoff> payoff(
        new PlainVanillaPayoff(Option::Call, 100.0));
    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(today+30));

    ext::shared_ptr<Instrument> option(new EuropeanOption(payoff, exercise));

    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(0.0, dc);
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(0.01, dc);
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(0.1, dc);

    ext::shared_ptr<BlackScholesMertonProcess> process(
        new BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));
    ext::shared_ptr<PricingEngine> engine(new AnalyticEuropeanEngine(process));

    option->setPricingEngine(engine);

    CompositeInstrument composite;
    composite.add(option);

    Settings::instance().evaluationDate() = today+45;

    if (!composite.isExpired())
        BOOST_FAIL("Composite didn't detect expiration");
    if (composite.NPV() != 0.0)
        BOOST_FAIL("Composite didn't return a null NPV");

    Settings::instance().evaluationDate() = today;

    if (composite.isExpired())
        BOOST_FAIL("Composite didn't detect aliveness");
    if (composite.NPV() == 0.0)
        BOOST_FAIL("Composite didn't recalculate");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
