/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2005, 2006, 2007 StatPro Italia srl

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

#include <qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <auto_link.hpp>
#endif
#include <instruments/vanillaswap.hpp>
#include <instruments/swaption.hpp>
#include <pricingengines/swap/discountingswapengine.hpp>
#include <pricingengines/swaption/treeswaptionengine.hpp>
#include <pricingengines/swaption/jamshidianswaptionengine.hpp>
#include <pricingengines/swaption/g2swaptionengine.hpp>
#include <pricingengines/swaption/fdhullwhiteswaptionengine.hpp>
#include <pricingengines/swaption/fdg2swaptionengine.hpp>
#include <models/shortrate/calibrationhelpers/swaptionhelper.hpp>
#include <models/shortrate/onefactormodels/blackkarasinski.hpp>
#include <math/optimization/levenbergmarquardt.hpp>
#include <indexes/ibor/euribor.hpp>
#include <cashflows/coupon.hpp>
#include <quotes/simplequote.hpp>
#include <termstructures/yield/flatforward.hpp>
#include <time/calendars/target.hpp>
#include <time/daycounters/thirty360.hpp>
#include <utilities/dataformatters.hpp>

#include <iostream>
#include <iomanip>

using namespace QuantLib;

//Number of swaptions to be calibrated to...

Size numRows = 5;
Size numCols = 5;

Integer swapLengths[] = {
      1,     2,     3,     4,     5};
Volatility swaptionVols[] = {
  0.1490, 0.1340, 0.1228, 0.1189, 0.1148,
  0.1290, 0.1201, 0.1146, 0.1108, 0.1040,
  0.1149, 0.1112, 0.1070, 0.1010, 0.0957,
  0.1047, 0.1021, 0.0980, 0.0951, 0.1270,
  0.1000, 0.0950, 0.0900, 0.1230, 0.1160};

void calibrateModel(
          const ext::shared_ptr<ShortRateModel>& model,
          const std::vector<ext::shared_ptr<BlackCalibrationHelper>>& swaptions) {

    std::vector<ext::shared_ptr<CalibrationHelper>> helpers(swaptions.begin(), swaptions.end());
    LevenbergMarquardt om;
    model->calibrate(helpers, om,
                     EndCriteria(400, 100, 1.0e-8, 1.0e-8, 1.0e-8));

    // Output the implied Black volatilities
    for (Size i=0; i<numRows; i++) {
        Size j = numCols - i -1; // 1x5, 2x4, 3x3, 4x2, 5x1
        Size k = i*numCols + j;
        Real npv = swaptions[i]->modelValue();
        Volatility implied = swaptions[i]->impliedVolatility(npv, 1e-4,
                                                             1000, 0.05, 0.50);
        Volatility diff = implied - swaptionVols[k];

        std::cout << i+1 << "x" << swapLengths[j]
                  << std::setprecision(5) << std::noshowpos
                  << ": model " << std::setw(7) << io::volatility(implied)
                  << ", market " << std::setw(7)
                  << io::volatility(swaptionVols[k])
                  << " (" << std::setw(7) << std::showpos
                  << io::volatility(diff) << std::noshowpos << ")\n";
    }
}

int main(int, char* []) {

    try {

        std::cout << std::endl;

        Date todaysDate(15, February, 2002);
        Calendar calendar = TARGET();
        Date settlementDate(19, February, 2002);
        Settings::instance().evaluationDate() = todaysDate;

        // flat yield term structure impling 1x5 swap at 5%
        auto flatRate = ext::make_shared<SimpleQuote>(0.04875825);
        Handle<YieldTermStructure> rhTermStructure(
            ext::make_shared<FlatForward>(
                      settlementDate, Handle<Quote>(flatRate),
                                      Actual365Fixed()));

        // Define the ATM/OTM/ITM swaps
        Frequency fixedLegFrequency = Annual;
        BusinessDayConvention fixedLegConvention = Unadjusted;
        BusinessDayConvention floatingLegConvention = ModifiedFollowing;
        DayCounter fixedLegDayCounter = Thirty360(Thirty360::European);
        Frequency floatingLegFrequency = Semiannual;
        Swap::Type type = Swap::Payer;
        Rate dummyFixedRate = 0.03;
        auto indexSixMonths = ext::make_shared<Euribor6M>(rhTermStructure);

        Date startDate = calendar.advance(settlementDate,1,Years,
                                          floatingLegConvention);
        Date maturity = calendar.advance(startDate,5,Years,
                                         floatingLegConvention);
        Schedule fixedSchedule(startDate,maturity,Period(fixedLegFrequency),
                               calendar,fixedLegConvention,fixedLegConvention,
                               DateGeneration::Forward,false);
        Schedule floatSchedule(startDate,maturity,Period(floatingLegFrequency),
                               calendar,floatingLegConvention,floatingLegConvention,
                               DateGeneration::Forward,false);

        auto swap = ext::make_shared<VanillaSwap>(
            type, 1000.0,
            fixedSchedule, dummyFixedRate, fixedLegDayCounter,
            floatSchedule, indexSixMonths, 0.0,
            indexSixMonths->dayCounter());
        swap->setPricingEngine(ext::make_shared<DiscountingSwapEngine>(rhTermStructure));
        Rate fixedATMRate = swap->fairRate();
        Rate fixedOTMRate = fixedATMRate * 1.2;
        Rate fixedITMRate = fixedATMRate * 0.8;

        auto atmSwap = ext::make_shared<VanillaSwap>(
            type, 1000.0,
            fixedSchedule, fixedATMRate, fixedLegDayCounter,
            floatSchedule, indexSixMonths, 0.0,
            indexSixMonths->dayCounter());
        auto otmSwap = ext::make_shared<VanillaSwap>(
            type, 1000.0,
            fixedSchedule, fixedOTMRate, fixedLegDayCounter,
            floatSchedule, indexSixMonths, 0.0,
            indexSixMonths->dayCounter());
        auto itmSwap = ext::make_shared<VanillaSwap>(
            type, 1000.0,
            fixedSchedule, fixedITMRate, fixedLegDayCounter,
            floatSchedule, indexSixMonths, 0.0,
            indexSixMonths->dayCounter());

        // defining the swaptions to be used in model calibration
        std::vector<Period> swaptionMaturities;
        swaptionMaturities.emplace_back(1, Years);
        swaptionMaturities.emplace_back(2, Years);
        swaptionMaturities.emplace_back(3, Years);
        swaptionMaturities.emplace_back(4, Years);
        swaptionMaturities.emplace_back(5, Years);

        std::vector<ext::shared_ptr<BlackCalibrationHelper>> swaptions;

        // List of times that have to be included in the timegrid
        std::list<Time> times;

        Size i;
        for (i=0; i<numRows; i++) {
            Size j = numCols - i -1; // 1x5, 2x4, 3x3, 4x2, 5x1
            Size k = i*numCols + j;
            auto vol = ext::make_shared<SimpleQuote>(swaptionVols[k]);

            swaptions.push_back(ext::make_shared<SwaptionHelper>(
                               swaptionMaturities[i],
                               Period(swapLengths[j], Years),
                               Handle<Quote>(vol),
                               indexSixMonths,
                               indexSixMonths->tenor(),
                               indexSixMonths->dayCounter(),
                               indexSixMonths->dayCounter(),
                               rhTermStructure));

            swaptions.back()->addTimesTo(times);
        }

        // Building time-grid
        TimeGrid grid(times.begin(), times.end(), 30);


        // defining the models
        auto modelG2 = ext::make_shared<G2>(rhTermStructure);
        auto modelHW = ext::make_shared<HullWhite>(rhTermStructure);
        auto modelHW2 = ext::make_shared<HullWhite>(rhTermStructure);
        auto modelBK = ext::make_shared<BlackKarasinski>(rhTermStructure);


        // model calibrations

        std::cout << "G2 (analytic formulae) calibration" << std::endl;
        for (i=0; i<swaptions.size(); i++)
            swaptions[i]->setPricingEngine(ext::make_shared<G2SwaptionEngine>(modelG2, 6.0, 16));

        calibrateModel(modelG2, swaptions);
        std::cout << "calibrated to:\n"
                  << "a     = " << modelG2->params()[0] << ", "
                  << "sigma = " << modelG2->params()[1] << "\n"
                  << "b     = " << modelG2->params()[2] << ", "
                  << "eta   = " << modelG2->params()[3] << "\n"
                  << "rho   = " << modelG2->params()[4]
                  << std::endl << std::endl;



        std::cout << "Hull-White (analytic formulae) calibration" << std::endl;
        for (i=0; i<swaptions.size(); i++)
            swaptions[i]->setPricingEngine(ext::make_shared<JamshidianSwaptionEngine>(modelHW));

        calibrateModel(modelHW, swaptions);
        std::cout << "calibrated to:\n"
                  << "a = " << modelHW->params()[0] << ", "
                  << "sigma = " << modelHW->params()[1]
                  << std::endl << std::endl;

        std::cout << "Hull-White (numerical) calibration" << std::endl;
        for (i=0; i<swaptions.size(); i++)
            swaptions[i]->setPricingEngine(ext::make_shared<TreeSwaptionEngine>(modelHW2, grid));

        calibrateModel(modelHW2, swaptions);
        std::cout << "calibrated to:\n"
                  << "a = " << modelHW2->params()[0] << ", "
                  << "sigma = " << modelHW2->params()[1]
                  << std::endl << std::endl;

        std::cout << "Black-Karasinski (numerical) calibration" << std::endl;
        for (i=0; i<swaptions.size(); i++)
            swaptions[i]->setPricingEngine(ext::make_shared<TreeSwaptionEngine>(modelBK, grid));

        calibrateModel(modelBK, swaptions);
        std::cout << "calibrated to:\n"
                  << "a = " << modelBK->params()[0] << ", "
                  << "sigma = " << modelBK->params()[1]
                  << std::endl << std::endl;


        // ATM Bermudan swaption pricing

        std::cout << "Payer bermudan swaption "
                  << "struck at " << io::rate(fixedATMRate)
                  << " (ATM)" << std::endl;

        std::vector<Date> bermudanDates;
        const std::vector<ext::shared_ptr<CashFlow>>& leg =
            swap->fixedLeg();
        for (i=0; i<leg.size(); i++) {
            auto coupon = ext::dynamic_pointer_cast<Coupon>(leg[i]);
            bermudanDates.push_back(coupon->accrualStartDate());
        }

        auto bermudanExercise = ext::make_shared<BermudanExercise>(bermudanDates);

        Swaption bermudanSwaption(atmSwap, bermudanExercise);

        // Do the pricing for each model

        // G2 price the European swaption here, it should switch to bermudan
        bermudanSwaption.setPricingEngine(ext::make_shared<TreeSwaptionEngine>(modelG2, 50));
        std::cout << "G2 (tree):      " << bermudanSwaption.NPV() << std::endl;
        bermudanSwaption.setPricingEngine(ext::make_shared<FdG2SwaptionEngine>(modelG2));
        std::cout << "G2 (fdm) :      " << bermudanSwaption.NPV() << std::endl;

        bermudanSwaption.setPricingEngine(ext::make_shared<TreeSwaptionEngine>(modelHW, 50));
        std::cout << "HW (tree):      " << bermudanSwaption.NPV() << std::endl;
        bermudanSwaption.setPricingEngine(ext::make_shared<FdHullWhiteSwaptionEngine>(modelHW));
        std::cout << "HW (fdm) :      " << bermudanSwaption.NPV() << std::endl;

        bermudanSwaption.setPricingEngine(ext::make_shared<TreeSwaptionEngine>(modelHW2, 50));
        std::cout << "HW (num, tree): " << bermudanSwaption.NPV() << std::endl;
        bermudanSwaption.setPricingEngine(ext::make_shared<FdHullWhiteSwaptionEngine>(modelHW2));
        std::cout << "HW (num, fdm) : " << bermudanSwaption.NPV() << std::endl;

        bermudanSwaption.setPricingEngine(ext::make_shared<TreeSwaptionEngine>(modelBK, 50));
        std::cout << "BK:             " << bermudanSwaption.NPV() << std::endl;


        // OTM Bermudan swaption pricing

        std::cout << "Payer bermudan swaption "
                  << "struck at " << io::rate(fixedOTMRate)
                  << " (OTM)" << std::endl;

        Swaption otmBermudanSwaption(otmSwap,bermudanExercise);

        // Do the pricing for each model
        otmBermudanSwaption.setPricingEngine(ext::make_shared<TreeSwaptionEngine>(modelG2, 300));
        std::cout << "G2 (tree):       " << otmBermudanSwaption.NPV()
                  << std::endl;
        otmBermudanSwaption.setPricingEngine(ext::make_shared<FdG2SwaptionEngine>(modelG2));
        std::cout << "G2 (fdm) :       " << otmBermudanSwaption.NPV()
                  << std::endl;

        otmBermudanSwaption.setPricingEngine(ext::make_shared<TreeSwaptionEngine>(modelHW, 50));
        std::cout << "HW (tree):       " << otmBermudanSwaption.NPV()
                  << std::endl;
        otmBermudanSwaption.setPricingEngine(ext::make_shared<FdHullWhiteSwaptionEngine>(modelHW));
        std::cout << "HW (fdm) :       " << otmBermudanSwaption.NPV()
                  << std::endl;

        otmBermudanSwaption.setPricingEngine(ext::make_shared<TreeSwaptionEngine>(modelHW2, 50));
        std::cout << "HW (num, tree):  " << otmBermudanSwaption.NPV()
                  << std::endl;
        otmBermudanSwaption.setPricingEngine(ext::make_shared<FdHullWhiteSwaptionEngine>(modelHW2));
        std::cout << "HW (num, fdm):   " << otmBermudanSwaption.NPV()
                  << std::endl;

        otmBermudanSwaption.setPricingEngine(ext::make_shared<TreeSwaptionEngine>(modelBK, 50));
        std::cout << "BK:              " << otmBermudanSwaption.NPV()
                  << std::endl;


        // ITM Bermudan swaption pricing

        std::cout << "Payer bermudan swaption "
                  << "struck at " << io::rate(fixedITMRate)
                  << " (ITM)" << std::endl;

        Swaption itmBermudanSwaption(itmSwap,bermudanExercise);

        // Do the pricing for each model
        itmBermudanSwaption.setPricingEngine(ext::make_shared<TreeSwaptionEngine>(modelG2, 50));
        std::cout << "G2 (tree):       " << itmBermudanSwaption.NPV()
                  << std::endl;
        itmBermudanSwaption.setPricingEngine(ext::make_shared<FdG2SwaptionEngine>(modelG2));
        std::cout << "G2 (fdm) :       " << itmBermudanSwaption.NPV()
                  << std::endl;

        itmBermudanSwaption.setPricingEngine(ext::make_shared<TreeSwaptionEngine>(modelHW, 50));
        std::cout << "HW (tree):       " << itmBermudanSwaption.NPV()
                  << std::endl;
        itmBermudanSwaption.setPricingEngine(ext::make_shared<FdHullWhiteSwaptionEngine>(modelHW));
        std::cout << "HW (fdm) :       " << itmBermudanSwaption.NPV()
                  << std::endl;

        itmBermudanSwaption.setPricingEngine(ext::make_shared<TreeSwaptionEngine>(modelHW2, 50));
        std::cout << "HW (num, tree):  " << itmBermudanSwaption.NPV()
                  << std::endl;
        itmBermudanSwaption.setPricingEngine(ext::make_shared<FdHullWhiteSwaptionEngine>(modelHW2));
        std::cout << "HW (num, fdm) :  " << itmBermudanSwaption.NPV()
                  << std::endl;

        itmBermudanSwaption.setPricingEngine(ext::make_shared<TreeSwaptionEngine>(modelBK, 50));
        std::cout << "BK:              " << itmBermudanSwaption.NPV()
                  << std::endl;

        return 0;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}

