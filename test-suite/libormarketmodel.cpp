/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Klaus Spanderen

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

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <indexes/ibor/euribor.hpp>
#include <instruments/capfloor.hpp>
#include <instruments/vanillaswap.hpp>
#include <legacy/libormarketmodels/lfmcovarproxy.hpp>
#include <legacy/libormarketmodels/lfmhullwhiteparam.hpp>
#include <legacy/libormarketmodels/lfmswaptionengine.hpp>
#include <legacy/libormarketmodels/liborforwardmodel.hpp>
#include <legacy/libormarketmodels/lmexpcorrmodel.hpp>
#include <legacy/libormarketmodels/lmextlinexpvolmodel.hpp>
#include <legacy/libormarketmodels/lmfixedvolmodel.hpp>
#include <legacy/libormarketmodels/lmlinexpcorrmodel.hpp>
#include <math/optimization/levenbergmarquardt.hpp>
#include <math/randomnumbers/rngtraits.hpp>
#include <math/statistics/generalstatistics.hpp>
#include <methods/montecarlo/multipathgenerator.hpp>
#include <models/shortrate/calibrationhelpers/caphelper.hpp>
#include <models/shortrate/calibrationhelpers/swaptionhelper.hpp>
#include <pricingengines/capfloor/analyticcapfloorengine.hpp>
#include <pricingengines/capfloor/blackcapfloorengine.hpp>
#include <pricingengines/swap/discountingswapengine.hpp>
#include <quotes/simplequote.hpp>
#include <termstructures/volatility/optionlet/capletvariancecurve.hpp>
#include <termstructures/yield/zerocurve.hpp>
#include <time/daycounters/actual360.hpp>
#include <time/schedule.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)  // tests for deprecated classes

BOOST_AUTO_TEST_SUITE(LiborMarketModelTests)

ext::shared_ptr<IborIndex> makeIndex(std::vector<Date> dates, const std::vector<Rate>& rates) {
    DayCounter dayCounter = Actual360();

    RelinkableHandle<YieldTermStructure> termStructure;

    ext::shared_ptr<IborIndex> index(new Euribor6M(termStructure));

    Date todaysDate =
        index->fixingCalendar().adjust(Date(4,September,2005));
    Settings::instance().evaluationDate() = todaysDate;

    dates[0] = index->fixingCalendar().advance(todaysDate,
                                               index->fixingDays(), Days);

    termStructure.linkTo(ext::shared_ptr<YieldTermStructure>(
                                    new ZeroCurve(dates, rates, dayCounter)));

    return index;
}


ext::shared_ptr<IborIndex> makeIndex() {
    std::vector<Date> dates = {{4,September,2005}, {4,September,2018}};
    std::vector<Rate> rates = {0.039, 0.041};

    return makeIndex(dates, rates);
}


ext::shared_ptr<OptionletVolatilityStructure>
makeCapVolCurve(const Date& todaysDate) {
    Volatility vols[] = {14.40, 17.15, 16.81, 16.64, 16.17,
                         15.78, 15.40, 15.21, 14.86};

    std::vector<Date> dates;
    std::vector<Volatility> capletVols;
    ext::shared_ptr<LiborForwardModelProcess> process(
                               new LiborForwardModelProcess(10, makeIndex()));

    for (Size i=0; i < 9; ++i) {
        capletVols.push_back(vols[i]/100);
        dates.push_back(process->fixingDates()[i+1]);
    }

    return ext::make_shared<CapletVarianceCurve>(
                         todaysDate, dates,
                         capletVols, Actual360());
}


BOOST_AUTO_TEST_CASE(testSimpleCovarianceModels) {
    BOOST_TEST_MESSAGE("Testing simple covariance models...");

    const Size size = 10;
    const Real tolerance = 1e-14;
    Size i;

    ext::shared_ptr<LmCorrelationModel> corrModel(
                                new LmExponentialCorrelationModel(size, 0.1));

    Matrix recon = corrModel->correlation(0.0)
        - corrModel->pseudoSqrt(0.0)*transpose(corrModel->pseudoSqrt(0.0));

    for (i=0; i<size; ++i) {
        for (Size j=0; j<size; ++j) {
            if (std::fabs(recon[i][j]) > tolerance)
                BOOST_ERROR("Failed to reproduce correlation matrix"
                            << "\n    calculated: " << recon[i][j]
                            << "\n    expected:   " << 0);
        }
    }

    std::vector<Time> fixingTimes(size);
    for (i=0; i<size; ++i) {
        fixingTimes[i] = 0.5*i;
    }

    const Real a=0.2;
    const Real b=0.1;
    const Real c=2.1;
    const Real d=0.3;

    ext::shared_ptr<LmVolatilityModel> volaModel(
             new LmLinearExponentialVolatilityModel(fixingTimes, a, b, c, d));

    ext::shared_ptr<LfmCovarianceProxy> covarProxy(
                                new LfmCovarianceProxy(volaModel, corrModel));

    ext::shared_ptr<LiborForwardModelProcess> process(
                             new LiborForwardModelProcess(size, makeIndex()));

    ext::shared_ptr<LiborForwardModel> liborModel(
                        new LiborForwardModel(process, volaModel, corrModel));

    for (Real t=0; t<4.6; t+=0.31) {
        recon = covarProxy->covariance(t)
            - covarProxy->diffusion(t)*transpose(covarProxy->diffusion(t));

        for (Size i=0; i<size; ++i) {
            for (Size j=0; j<size; ++j) {
                if (std::fabs(recon[i][j]) > tolerance)
                    BOOST_ERROR("Failed to reproduce correlation matrix"
                                << "\n    calculated: " << recon[i][j]
                                << "\n    expected:   " << 0);
            }
        }

        Array volatility = volaModel->volatility(t);

        for (Size k=0; k<size; ++k) {
            Real expected = 0;
            if (static_cast<Real>(k) > 2 * t) {
                const Real T = fixingTimes[k];
                expected=(a*(T-t)+d)*std::exp(-b*(T-t)) + c;
            }

            if (std::fabs(expected - volatility[k]) > tolerance)
                BOOST_ERROR("Failed to reproduce volatities"
                            << "\n    calculated: " << volatility[k]
                            << "\n    expected:   " << expected);
        }
    }
}

BOOST_AUTO_TEST_CASE(testCapletPricing) {
    BOOST_TEST_MESSAGE("Testing caplet pricing...");

    bool usingAtParCoupons  = IborCoupon::Settings::instance().usingAtParCoupons();

    const Size size = 10;
    Real tolerance = usingAtParCoupons ? 1e-12 : 1e-5;

    ext::shared_ptr<IborIndex> index = makeIndex();
    ext::shared_ptr<LiborForwardModelProcess> process(
        new LiborForwardModelProcess(size, index));

    // set-up pricing engine
    const ext::shared_ptr<OptionletVolatilityStructure> capVolCurve =
        makeCapVolCurve(Settings::instance().evaluationDate());

    Array variances = LfmHullWhiteParameterization(process, capVolCurve)
        .covariance(0.0).diagonal();

    ext::shared_ptr<LmVolatilityModel> volaModel(
        new LmFixedVolatilityModel(Sqrt(variances),
                                   process->fixingTimes()));

    ext::shared_ptr<LmCorrelationModel> corrModel(
                                new LmExponentialCorrelationModel(size, 0.3));

    ext::shared_ptr<AffineModel> model(
                        new LiborForwardModel(process, volaModel, corrModel));

    const Handle<YieldTermStructure> termStructure =
        process->index()->forwardingTermStructure();

    ext::shared_ptr<AnalyticCapFloorEngine> engine1(
                            new AnalyticCapFloorEngine(model, termStructure));

    auto cap1 = Cap(process->cashFlows(),
                    std::vector<Rate>(size, 0.04));
    cap1.setPricingEngine(engine1);

    const Real expected = 0.015853935178;
    const Real calculated = cap1.NPV();

    if (std::fabs(expected - calculated) > tolerance)
        BOOST_ERROR("Failed to reproduce npv"
                    << "\n    calculated: " << calculated
                    << "\n    expected:   " << expected);
}

BOOST_AUTO_TEST_CASE(testCalibration, *precondition(if_speed(Fast))) {
    BOOST_TEST_MESSAGE("Testing calibration of a Libor forward model...");

    const Size size = 14;
    const Real tolerance = 8e-3;

    Volatility capVols[] = {0.145708,0.158465,0.166248,0.168672,
                            0.169007,0.167956,0.166261,0.164239,
                            0.162082,0.159923,0.157781,0.155745,
                            0.153776,0.151950,0.150189,0.148582,
                            0.147034,0.145598,0.144248};

    Volatility swaptionVols[] = {0.170595, 0.166844, 0.158306, 0.147444,
                                 0.136930, 0.126833, 0.118135, 0.175963,
                                 0.166359, 0.155203, 0.143712, 0.132769,
                                 0.122947, 0.114310, 0.174455, 0.162265,
                                 0.150539, 0.138734, 0.128215, 0.118470,
                                 0.110540, 0.169780, 0.156860, 0.144821,
                                 0.133537, 0.123167, 0.114363, 0.106500,
                                 0.164521, 0.151223, 0.139670, 0.128632,
                                 0.119123, 0.110330, 0.103114, 0.158956,
                                 0.146036, 0.134555, 0.124393, 0.115038,
                                 0.106996, 0.100064};

    ext::shared_ptr<IborIndex> index = makeIndex();
    ext::shared_ptr<LiborForwardModelProcess> process(
        new LiborForwardModelProcess(size, index));
    Handle<YieldTermStructure> termStructure = index->forwardingTermStructure();

    // set-up the model
    ext::shared_ptr<LmVolatilityModel> volaModel(
                    new LmExtLinearExponentialVolModel(process->fixingTimes(),
                                                       0.5,0.6,0.1,0.1));

    ext::shared_ptr<LmCorrelationModel> corrModel(
                     new LmLinearExponentialCorrelationModel(size, 0.5, 0.8));

    ext::shared_ptr<LiborForwardModel> model(
                        new LiborForwardModel(process, volaModel, corrModel));

    Size swapVolIndex = 0;
    DayCounter dayCounter=index->forwardingTermStructure()->dayCounter();

    // set-up calibration helper
    std::vector<ext::shared_ptr<CalibrationHelper> > calibrationHelpers;

    Size i;
    for (i=2; i < size; ++i) {
        const Period maturity = i*index->tenor();
        Handle<Quote> capVol(
            ext::shared_ptr<Quote>(new SimpleQuote(capVols[i-2])));

        auto caphelper =
            ext::make_shared<CapHelper>(maturity, capVol, index, Annual,
                                        index->dayCounter(), true, termStructure,
                                        BlackCalibrationHelper::ImpliedVolError);

        caphelper->setPricingEngine(ext::shared_ptr<PricingEngine>(
                           new AnalyticCapFloorEngine(model, termStructure)));

        calibrationHelpers.push_back(caphelper);

        if (i<= size/2) {
            // add a few swaptions to test swaption calibration as well
            for (Size j=1; j <= size/2; ++j) {
                const Period len = j*index->tenor();
                Handle<Quote> swaptionVol(
                    ext::shared_ptr<Quote>(
                        new SimpleQuote(swaptionVols[swapVolIndex++])));

                auto swaptionHelper =
                    ext::make_shared<SwaptionHelper>(maturity, len, swaptionVol, index,
                                                     index->tenor(), dayCounter,
                                                     index->dayCounter(),
                                                     termStructure,
                                                     BlackCalibrationHelper::ImpliedVolError);

                swaptionHelper->setPricingEngine(
                     ext::shared_ptr<PricingEngine>(
                                 new LfmSwaptionEngine(model,termStructure)));

                calibrationHelpers.push_back(swaptionHelper);
            }
        }
    }

    LevenbergMarquardt om(1e-6, 1e-6, 1e-6);
    model->calibrate(calibrationHelpers, om, EndCriteria(2000, 100, 1e-6, 1e-6, 1e-6));

    // measure the calibration error
    Real calculated = 0.0;
    for (i=0; i<calibrationHelpers.size(); ++i) {
        Real diff = calibrationHelpers[i]->calibrationError();
        calculated += diff*diff;
    }

    if (std::sqrt(calculated) > tolerance)
        BOOST_ERROR("Failed to calibrate libor forward model"
                    << "\n    calculated diff: " << std::sqrt(calculated)
                    << "\n    expected : smaller than  " << tolerance);
}

BOOST_AUTO_TEST_CASE(testSwaptionPricing) {
    BOOST_TEST_MESSAGE("Testing forward swap and swaption pricing...");

    bool usingAtParCoupons = IborCoupon::Settings::instance().usingAtParCoupons();

    const Size size  = 10;
    const Size steps = 8*size;

    Real tolerance = usingAtParCoupons ? 1e-12 : 1e-6;

    std::vector<Date> dates = {{4,September,2005}, {4,September,2011}};
    std::vector<Rate> rates = {0.04, 0.08};

    ext::shared_ptr<IborIndex> index = makeIndex(dates, rates);

    ext::shared_ptr<LiborForwardModelProcess> process(
                                   new LiborForwardModelProcess(size, index));

    ext::shared_ptr<LmCorrelationModel> corrModel(
                                new LmExponentialCorrelationModel(size, 0.5));

    ext::shared_ptr<LmVolatilityModel> volaModel(
        new LmLinearExponentialVolatilityModel(process->fixingTimes(),
                                               0.291, 1.483, 0.116, 0.00001));

   // set-up pricing engine
    process->setCovarParam(ext::shared_ptr<LfmCovarianceParameterization>(
                               new LfmCovarianceProxy(volaModel, corrModel)));

    // set-up a small Monte-Carlo simulation to price swations
    typedef PseudoRandom::rsg_type rsg_type;
    typedef MultiPathGenerator<rsg_type>::sample_type sample_type;

    std::vector<Time> tmp = process->fixingTimes();
    TimeGrid grid(tmp.begin(), tmp.end(), steps);

    Size i;
    std::vector<Size> location;
    for (i=0; i < tmp.size(); ++i) {
        location.push_back(
                      std::find(grid.begin(),grid.end(),tmp[i])-grid.begin());
    }

    rsg_type rsg = PseudoRandom::make_sequence_generator(
                       process->factors()*(grid.size()-1),
                       BigNatural(42));

    const Size nrTrails = 5000;
    MultiPathGenerator<rsg_type> generator(process, grid, rsg, false);

    ext::shared_ptr<LiborForwardModel>
        liborModel(new LiborForwardModel(process, volaModel, corrModel));

    Calendar calendar = index->fixingCalendar();
    DayCounter dayCounter = index->forwardingTermStructure()->dayCounter();
    BusinessDayConvention convention = index->businessDayConvention();

    Date settlement  = index->forwardingTermStructure()->referenceDate();

    for (i=1; i < size; ++i) {
        for (Size j=1; j <= size-i; ++j) {
            Date fwdStart    = settlement + Period(6*i, Months);
            Date fwdMaturity = fwdStart + Period(6*j, Months);

            Schedule schedule(fwdStart, fwdMaturity, index->tenor(), calendar,
                               convention, convention, DateGeneration::Forward, false);

            Rate swapRate  = 0.0404;
            ext::shared_ptr<VanillaSwap> forwardSwap(
                new VanillaSwap(Swap::Receiver, 1.0,
                                schedule, swapRate, dayCounter,
                                schedule, index, 0.0, index->dayCounter()));
            forwardSwap->setPricingEngine(ext::shared_ptr<PricingEngine>(
                new DiscountingSwapEngine(index->forwardingTermStructure())));

            // check forward pricing first
            const Real expected = forwardSwap->fairRate();
            const Real calculated = liborModel->S_0(i-1,i+j-1);

            if (std::fabs(expected - calculated) > tolerance)
                BOOST_ERROR("Failed to reproduce fair forward swap rate"
                            << "\n    calculated: " << calculated
                            << "\n    expected:   " << expected);

            swapRate = forwardSwap->fairRate();
            forwardSwap = ext::make_shared<VanillaSwap>(
                                Swap::Receiver, 1.0,
                                schedule, swapRate, dayCounter,
                                schedule, index, 0.0, index->dayCounter());
            forwardSwap->setPricingEngine(ext::shared_ptr<PricingEngine>(
                new DiscountingSwapEngine(index->forwardingTermStructure())));

            if (i == j && i<=size/2) {
                ext::shared_ptr<PricingEngine> engine(
                     new LfmSwaptionEngine(liborModel,
                                           index->forwardingTermStructure()));
                ext::shared_ptr<Exercise> exercise(
                    new EuropeanExercise(process->fixingDates()[i]));

                auto swaption = ext::make_shared<Swaption>(forwardSwap, exercise);
                swaption->setPricingEngine(engine);

                GeneralStatistics stat;

                for (Size n=0; n<nrTrails; ++n) {
                    sample_type path = (n % 2) != 0U ? generator.antithetic() : generator.next();

                    std::vector<Rate> rates(size);
                    for (Size k=0; k<process->size(); ++k) {
                        rates[k] = path.value[k][location[i]];
                    }
                    std::vector<DiscountFactor> dis =
                        process->discountBond(rates);

                    Real npv=0.0;
                    for (Size m=i; m < i+j; ++m) {
                        npv += (swapRate - rates[m])
                               * (  process->accrualEndTimes()[m]
                                  - process->accrualStartTimes()[m])*dis[m];
                    }
                    stat.add(std::max(npv, 0.0));
                }

                if (std::fabs(swaption->NPV() - stat.mean())
                    > stat.errorEstimate()*2.35)
                    BOOST_ERROR("Failed to reproduce swaption npv"
                                << "\n    calculated: " << stat.mean()
                                << "\n    expected:   " << swaption->NPV());
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
