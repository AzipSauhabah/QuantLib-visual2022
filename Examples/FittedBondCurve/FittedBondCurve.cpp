/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2007 Allen Kuo
 Copyright (C) 2015 Andres Hernandez

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

/*  This example shows how to fit a term structure to a set of bonds
    using four different fitting methodologies. Though fitting is most
    useful for large numbers of bonds with non-smooth yield tenor
    structures, for comparison purposes, relatively smooth bond yields
    are fit here and compared to known solutions (par coupons), or
    results generated from the bootstrap fitting method.
*/

#include <qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <auto_link.hpp>
#endif
#include <termstructures/yield/fittedbonddiscountcurve.hpp>
#include <termstructures/yield/piecewiseyieldcurve.hpp>
#include <termstructures/yield/flatforward.hpp>
#include <termstructures/yield/bondhelpers.hpp>
#include <termstructures/yield/nonlinearfittingmethods.hpp>
#include <pricingengines/bond/bondfunctions.hpp>
#include <time/calendars/target.hpp>
#include <time/daycounters/simpledaycounter.hpp>

#include <iostream>
#include <iomanip>

using namespace std;
using namespace QuantLib;

// par-rate approximation
Rate parRate(const YieldTermStructure& yts,
             const std::vector<Date>& dates,
             const DayCounter& resultDayCounter) {
    QL_REQUIRE(dates.size() >= 2, "at least two dates are required");
    Real sum = 0.0;
    Time dt;
    for (Size i=1; i<dates.size(); ++i) {
        dt = resultDayCounter.yearFraction(dates[i-1], dates[i]);
        QL_REQUIRE(dt>=0.0, "unsorted dates");
        sum += yts.discount(dates[i]) * dt;
    }
    Real result = yts.discount(dates.front()) - yts.discount(dates.back());
    return result/sum;
}

void printOutput(const std::string& tag,
                 const ext::shared_ptr<FittedBondDiscountCurve>& curve) {
    cout << tag << endl;
    cout << "reference date : "
         << curve->referenceDate()
         << endl;
    cout << "number of iterations : "
         << curve->fitResults().numberOfIterations()
         << endl
         << endl;
}


int main(int, char* []) {

    try {

        const Size numberOfBonds = 15;
        Real cleanPrice[numberOfBonds];

        for (Real& i : cleanPrice) {
            i = 100.0;
        }

        std::vector< ext::shared_ptr<SimpleQuote>> quote;
        for (Real i : cleanPrice) {
            quote.push_back(ext::make_shared<SimpleQuote>(i));
        }

        RelinkableHandle<Quote> quoteHandle[numberOfBonds];
        for (Size i=0; i<numberOfBonds; i++) {
            quoteHandle[i].linkTo(quote[i]);
        }

        Integer lengths[] = { 2, 4, 6, 8, 10, 12, 14, 16,
                              18, 20, 22, 24, 26, 28, 30 };
        Real coupons[] = { 0.0200, 0.0225, 0.0250, 0.0275, 0.0300,
                           0.0325, 0.0350, 0.0375, 0.0400, 0.0425,
                           0.0450, 0.0475, 0.0500, 0.0525, 0.0550 };

        Frequency frequency = Annual;
        DayCounter dc = SimpleDayCounter();
        BusinessDayConvention accrualConvention = ModifiedFollowing;
        BusinessDayConvention convention = ModifiedFollowing;
        Real redemption = 100.0;

        Calendar calendar = TARGET();
        Date today = calendar.adjust(Date::todaysDate());
        Date origToday = today;
        Settings::instance().evaluationDate() = today;

        // changing bondSettlementDays=3 increases calculation
        // time of exponentialsplines fitting method
        Natural bondSettlementDays = 0;
        Natural curveSettlementDays = 0;

        Date bondSettlementDate = calendar.advance(today, bondSettlementDays*Days);

        cout << endl;
        cout << "Today's date: " << today << endl;
        cout << "Bonds' settlement date: " << bondSettlementDate << endl;
        cout << "Calculating fit for 15 bonds....." << endl << endl;

        std::vector<ext::shared_ptr<BondHelper>> instrumentsA;
        std::vector<ext::shared_ptr<RateHelper>> instrumentsB;

        for (Size j=0; j<std::size(lengths); j++) {

            Date maturity = calendar.advance(bondSettlementDate, lengths[j]*Years);

            Schedule schedule(bondSettlementDate, maturity, Period(frequency),
                              calendar, accrualConvention, accrualConvention,
                              DateGeneration::Backward, false);

            auto helperA = ext::make_shared<FixedRateBondHelper>(quoteHandle[j],
                                             bondSettlementDays,
                                             100.0,
                                             schedule,
                                             std::vector<Rate>(1,coupons[j]),
                                             dc,
                                             convention,
                                             redemption);

            auto helperB = ext::make_shared<FixedRateBondHelper>(quoteHandle[j],
                                             bondSettlementDays,
                                             100.0,
                                             schedule,
                                             std::vector<Rate>(1, coupons[j]),
                                             dc,
                                             convention,
                                             redemption);
            instrumentsA.push_back(helperA);
            instrumentsB.push_back(helperB);
        }


        bool constrainAtZero = true;
        Real tolerance = 1.0e-10;
        Size max = 5000;

        auto ts0 = ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(curveSettlementDays,
                                                          calendar,
                                                          instrumentsB,
                                                          dc);

        ExponentialSplinesFitting exponentialSplines(constrainAtZero);

        auto ts1 = ext::make_shared<FittedBondDiscountCurve>(curveSettlementDays,
                                              calendar,
                                              instrumentsA,
                                              dc,
                                              exponentialSplines,
                                              tolerance,
                                              max);

        printOutput("(a) exponential splines", ts1);


        SimplePolynomialFitting simplePolynomial(3, constrainAtZero);

        auto ts2 = ext::make_shared<FittedBondDiscountCurve>(curveSettlementDays,
                                                calendar,
                                                instrumentsA,
                                                dc,
                                                simplePolynomial,
                                                tolerance,
                                                max);

        printOutput("(b) simple polynomial", ts2);


        NelsonSiegelFitting nelsonSiegel;

        auto ts3 = ext::make_shared<FittedBondDiscountCurve>(curveSettlementDays,
                                                    calendar,
                                                    instrumentsA,
                                                    dc,
                                                    nelsonSiegel,
                                                    tolerance,
                                                    max);

        printOutput("(c) Nelson-Siegel", ts3);


        // a cubic bspline curve with 11 knot points, implies
        // n=6 (constrained problem) basis functions

        Time knots[] =  { -30.0, -20.0,  0.0,  5.0, 10.0, 15.0,
                           20.0,  25.0, 30.0, 40.0, 50.0 };

        std::vector<Time> knotVector;
        for (Real& knot : knots) {
            knotVector.push_back(knot);
        }

        CubicBSplinesFitting cubicBSplines(knotVector, constrainAtZero);

        auto ts4 = ext::make_shared<FittedBondDiscountCurve>(curveSettlementDays,
                                                   calendar,
                                                   instrumentsA,
                                                   dc,
                                                   cubicBSplines,
                                                   tolerance,
                                                   max);

        printOutput("(d) cubic B-splines", ts4);

        SvenssonFitting svensson;

        auto ts5 = ext::make_shared<FittedBondDiscountCurve>(curveSettlementDays,
                                                    calendar,
                                                    instrumentsA,
                                                    dc,
                                                    svensson,
                                                    tolerance,
                                                    max);

        printOutput("(e) Svensson", ts5);

        Handle<YieldTermStructure> discountCurve(
            ext::make_shared<FlatForward>(
                                    curveSettlementDays, calendar, 0.01, dc));
        SpreadFittingMethod nelsonSiegelSpread(
                                    ext::make_shared<NelsonSiegelFitting>(),
                                    discountCurve);

        auto ts6 = ext::make_shared<FittedBondDiscountCurve>(curveSettlementDays,
                                                    calendar,
                                                    instrumentsA,
                                                    dc,
                                                    nelsonSiegelSpread,
                                                    tolerance,
                                                    max);

        printOutput("(f) Nelson-Siegel spread", ts6);

        //Fixed kappa, and 7 coefficients
        ExponentialSplinesFitting exponentialSplinesFixed(constrainAtZero,7,0.02);

        auto ts7 = ext::make_shared<FittedBondDiscountCurve>(curveSettlementDays,
                                                    calendar, 
                                                    instrumentsA, 
                                                    dc, 
                                                    exponentialSplinesFixed, 
                                                    tolerance, 
                                                    max);

        printOutput("(g) exponential splines, fixed kappa", ts7);

        cout << "Output par rates for each curve. In this case, "
             << endl
             << "par rates should equal coupons for these par bonds."
             << endl
             << endl;

        cout << setw(6) << "tenor" << " | "
             << setw(6) << "coupon" << " | "
             << setw(6) << "bstrap" << " | "
             << setw(6) << "(a)" << " | "
             << setw(6) << "(b)" << " | "
             << setw(6) << "(c)" << " | "
             << setw(6) << "(d)" << " | "
             << setw(6) << "(e)" << " | "
             << setw(6) << "(f)" << " | "
             << setw(6) << "(g)" << endl;

        for (Size i=0; i<instrumentsA.size(); i++) {

            std::vector<ext::shared_ptr<CashFlow>> cfs =
                instrumentsA[i]->bond()->cashflows();

            Size cfSize = instrumentsA[i]->bond()->cashflows().size();
            std::vector<Date> keyDates;
            keyDates.push_back(bondSettlementDate);

            for (Size j=0; j<cfSize-1; j++) {
                if (!cfs[j]->hasOccurred(bondSettlementDate, false)) {
                    Date myDate =  cfs[j]->date();
                    keyDates.push_back(myDate);
                }
            }

            Real tenor = dc.yearFraction(today, cfs[cfSize-1]->date());

            cout << setw(6) << fixed << setprecision(3) << tenor << " | "
                 << setw(6) << fixed << setprecision(3)
                 << 100.*coupons[i] << " | "
                 // piecewise bootstrap
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts0,keyDates,dc) << " | "
                 // exponential splines
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts1,keyDates,dc) << " | "
                 // simple polynomial
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts2,keyDates,dc) << " | "
                 // Nelson-Siegel
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts3,keyDates,dc) << " | "
                 // cubic bsplines
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts4,keyDates,dc) << " | "
                 // Svensson
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts5,keyDates,dc)  << " | "
                 // Nelson-Siegel Spread
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts6,keyDates,dc) << " | "
                 // Exponential, fixed kappa
                 << setw(6) << fixed << setprecision(3) 
                 << 100. *parRate(*ts7, keyDates, dc) << endl;
        }

        cout << endl << endl << endl;
        cout << "Now add 23 months to today. Par rates should be "  << endl
             << "automatically recalculated because today's date "  << endl
             << "changes.  Par rates will NOT equal coupons (YTM "  << endl
             << "will, with the correct compounding), but the "     << endl
             << "piecewise yield curve par rates can be used as "   << endl
             << "a benchmark for correct par rates."
             << endl
             << endl;

        today = calendar.advance(origToday,23,Months,convention);
        Settings::instance().evaluationDate() = today;
        bondSettlementDate = calendar.advance(today, bondSettlementDays*Days);

        printOutput("(a) exponential splines", ts1);

        printOutput("(b) simple polynomial", ts2);

        printOutput("(c) Nelson-Siegel", ts3);

        printOutput("(d) cubic B-splines", ts4);

        printOutput("(e) Svensson", ts5);

        printOutput("(f) Nelson-Siegel spread", ts6);

        printOutput("(g) exponential spline, fixed kappa", ts7);

        cout << endl
             << endl;


        cout << setw(6) << "tenor" << " | "
             << setw(6) << "coupon" << " | "
             << setw(6) << "bstrap" << " | "
             << setw(6) << "(a)" << " | "
             << setw(6) << "(b)" << " | "
             << setw(6) << "(c)" << " | "
             << setw(6) << "(d)" << " | "
             << setw(6) << "(e)" << " | "
             << setw(6) << "(f)" << " | "
             << setw(6) << "(g)" << endl;

        for (Size i=0; i<instrumentsA.size(); i++) {

            std::vector<ext::shared_ptr<CashFlow>> cfs =
                instrumentsA[i]->bond()->cashflows();

            Size cfSize = instrumentsA[i]->bond()->cashflows().size();
            std::vector<Date> keyDates;
            keyDates.push_back(bondSettlementDate);

            for (Size j=0; j<cfSize-1; j++) {
                if (!cfs[j]->hasOccurred(bondSettlementDate, false)) {
                    Date myDate =  cfs[j]->date();
                    keyDates.push_back(myDate);
                }
            }

            Real tenor = dc.yearFraction(today, cfs[cfSize-1]->date());

            cout << setw(6) << fixed << setprecision(3) << tenor << " | "
                 << setw(6) << fixed << setprecision(3)
                 << 100.*coupons[i] << " | "
                 // piecewise bootstrap
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts0,keyDates,dc) << " | "
                 // exponential splines
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts1,keyDates,dc) << " | "
                 // simple polynomial
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts2,keyDates,dc) << " | "
                 // Nelson-Siegel
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts3,keyDates,dc) << " | "
                 // cubic bsplines
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts4,keyDates,dc) << " | "
                 // Svensson
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts5,keyDates,dc) << " | "
                 // Nelson-Siegel Spread
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts6,keyDates,dc) << " | "
                 // exponential, fixed kappa
                 << setw(6) << fixed << setprecision(3) 
                 << 100. * parRate(*ts7, keyDates, dc) << endl;
        }

        cout << endl << endl << endl;
        cout << "Now add one more month, for a total of two years " << endl
             << "from the original date. The first instrument is "  << endl
             << "now expired and par rates should again equal "     << endl
             << "coupon values, since clean prices did not change."
             << endl
             << endl;

        instrumentsA.erase(instrumentsA.begin(),
                           instrumentsA.begin()+1);
        instrumentsB.erase(instrumentsB.begin(),
                           instrumentsB.begin()+1);

        today = calendar.advance(origToday,24,Months,convention);
        Settings::instance().evaluationDate() = today;
        bondSettlementDate = calendar.advance(today, bondSettlementDays*Days);

        auto ts00 = ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(curveSettlementDays,
                                                          calendar,
                                                          instrumentsB,
                                                          dc);

        auto ts11 = ext::make_shared<FittedBondDiscountCurve>(curveSettlementDays,
                                              calendar,
                                              instrumentsA,
                                              dc,
                                              exponentialSplines,
                                              tolerance,
                                              max);

        printOutput("(a) exponential splines", ts11);


        auto ts22 = ext::make_shared<FittedBondDiscountCurve>(curveSettlementDays,
                                                calendar,
                                                instrumentsA,
                                                dc,
                                                simplePolynomial,
                                                tolerance,
                                                max);

        printOutput("(b) simple polynomial", ts22);


        auto ts33 = ext::make_shared<FittedBondDiscountCurve>(curveSettlementDays,
                                                    calendar,
                                                    instrumentsA,
                                                    dc,
                                                    nelsonSiegel,
                                                    tolerance,
                                                    max);

        printOutput("(c) Nelson-Siegel", ts33);


        auto ts44 = ext::make_shared<FittedBondDiscountCurve>(curveSettlementDays,
                                                   calendar,
                                                   instrumentsA,
                                                   dc,
                                                   cubicBSplines,
                                                   tolerance,
                                                   max);

        printOutput("(d) cubic B-splines", ts44);

        auto ts55 = ext::make_shared<FittedBondDiscountCurve>(curveSettlementDays,
                                                   calendar,
                                                   instrumentsA,
                                                   dc,
                                                   svensson,
                                                   tolerance,
                                                   max);

        printOutput("(e) Svensson", ts55);

        auto ts66 = ext::make_shared<FittedBondDiscountCurve>(curveSettlementDays,
                                                    calendar,
                                                    instrumentsA,
                                                    dc,
                                                    nelsonSiegelSpread,
                                                    tolerance,
                                                    max);

        printOutput("(f) Nelson-Siegel spread", ts66);

        auto ts77 = ext::make_shared<FittedBondDiscountCurve>(curveSettlementDays, 
                                                    calendar, 
                                                    instrumentsA, 
                                                    dc, 
                                                    exponentialSplinesFixed, 
                                                    tolerance, 
                                                    max);

        printOutput("(g) exponential, fixed kappa", ts77);

        cout << setw(6) << "tenor" << " | "
             << setw(6) << "coupon" << " | "
             << setw(6) << "bstrap" << " | "
             << setw(6) << "(a)" << " | "
             << setw(6) << "(b)" << " | "
             << setw(6) << "(c)" << " | "
             << setw(6) << "(d)" << " | "
             << setw(6) << "(e)" << " | "
             << setw(6) << "(f)" << " | "
             << setw(6) << "(g)" << endl;

        for (Size i=0; i<instrumentsA.size(); i++) {

            std::vector<ext::shared_ptr<CashFlow>> cfs =
                instrumentsA[i]->bond()->cashflows();

            Size cfSize = instrumentsA[i]->bond()->cashflows().size();
            std::vector<Date> keyDates;
            keyDates.push_back(bondSettlementDate);

            for (Size j=0; j<cfSize-1; j++) {
                if (!cfs[j]->hasOccurred(bondSettlementDate, false)) {
                    Date myDate =  cfs[j]->date();
                    keyDates.push_back(myDate);
                }
            }

            Real tenor = dc.yearFraction(today, cfs[cfSize-1]->date());

            cout << setw(6) << fixed << setprecision(3) << tenor << " | "
                 << setw(6) << fixed << setprecision(3)
                 << 100.*coupons[i+1] << " | "
                 // piecewise bootstrap
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts00,keyDates,dc) << " | "
                 // exponential splines
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts11,keyDates,dc) << " | "
                 // simple polynomial
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts22,keyDates,dc) << " | "
                 // Nelson-Siegel
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts33,keyDates,dc) << " | "
                 // cubic bsplines
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts44,keyDates,dc) << " | "
                 // Svensson
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts55,keyDates,dc) << " | "
                 // Nelson-Siegel Spread
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts66,keyDates,dc) << " | "
                 // exponential, fixed kappa
                 << setw(6) << fixed << setprecision(3) 
                 << 100. *parRate(*ts77, keyDates, dc) << endl;
        }


        cout << endl << endl << endl;
        cout << "Now decrease prices by a small amount, corresponding"  << endl
             << "to a theoretical five basis point parallel + shift of" << endl
             << "the yield curve. Because bond quotes change, the new " << endl
             << "par rates should be recalculated automatically."
             << endl
             << endl;

        for (Size k=0; k<std::size(lengths)-1; k++) {

            Real P = instrumentsA[k]->quote()->value();
            const Bond& b = *instrumentsA[k]->bond();
            Rate ytm = BondFunctions::yield(b, {P, Bond::Price::Clean},
                                            dc, Compounded, frequency,
                                            today);
            Time dur = BondFunctions::duration(b, ytm,
                                               dc, Compounded, frequency,
                                               Duration::Modified,
                                               today);

            const Real bpsChange = 5.;
            // dP = -dur * P * dY
            Real deltaP = -dur * P * (bpsChange/10000.);
            quote[k+1]->setValue(P + deltaP);
        }


        cout << setw(6) << "tenor" << " | "
             << setw(6) << "coupon" << " | "
             << setw(6) << "bstrap" << " | "
             << setw(6) << "(a)" << " | "
             << setw(6) << "(b)" << " | "
             << setw(6) << "(c)" << " | "
             << setw(6) << "(d)" << " | "
             << setw(6) << "(e)" << " | "
             << setw(6) << "(f)" << " | "
             << setw(6) << "(g)" << endl;

        for (Size i=0; i<instrumentsA.size(); i++) {

            std::vector<ext::shared_ptr<CashFlow>> cfs =
                instrumentsA[i]->bond()->cashflows();

            Size cfSize = instrumentsA[i]->bond()->cashflows().size();
            std::vector<Date> keyDates;
            keyDates.push_back(bondSettlementDate);

            for (Size j=0; j<cfSize-1; j++) {
                if (!cfs[j]->hasOccurred(bondSettlementDate, false)) {
                    Date myDate =  cfs[j]->date();
                    keyDates.push_back(myDate);
                }
            }

            Real tenor = dc.yearFraction(today, cfs[cfSize-1]->date());

            cout << setw(6) << fixed << setprecision(3) << tenor << " | "
                 << setw(6) << fixed << setprecision(3)
                 << 100.*coupons[i+1] << " | "
                 // piecewise bootstrap
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts00,keyDates,dc) << " | "
                 // exponential splines
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts11,keyDates,dc) << " | "
                 // simple polynomial
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts22,keyDates,dc) << " | "
                 // Nelson-Siegel
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts33,keyDates,dc) << " | "
                 // cubic bsplines
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts44,keyDates,dc) << " | "
                 // Svensson
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts55,keyDates,dc) << " | "
                 // Nelson-Siegel Spread
                 << setw(6) << fixed << setprecision(3)
                 << 100.*parRate(*ts66,keyDates,dc) << " | "
                 // exponential spline, fixed kappa
                 << setw(6) << fixed << setprecision(3) 
                 << 100. *parRate(*ts77, keyDates, dc) << endl;
        }

        return 0;

    } catch (std::exception& e) {
        cerr << e.what() << endl;
        return 1;
    } catch (...) {
        cerr << "unknown error" << endl;
        return 1;
    }

}

