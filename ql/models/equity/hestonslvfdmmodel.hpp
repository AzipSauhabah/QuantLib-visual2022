/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Göttker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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

/*! \file hestonslvfdmmodel.hpp
    \brief Heston stochastic local volatility model
*/

#ifndef quantlib_heston_slv_model_hpp
#define quantlib_heston_slv_model_hpp

#include <handle.hpp>
#include <patterns/lazyobject.hpp>
#include <patterns/observable.hpp>
#include <methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <methods/finitedifferences/utilities/fdmhestongreensfct.hpp>

#include <list>

namespace QuantLib {

class SimpleQuote;
    class HestonModel;
    class LocalVolTermStructure;

    struct HestonSLVFokkerPlanckFdmParams {
         Size xGrid, vGrid;
         Size tMaxStepsPerYear, tMinStepsPerYear;
         Real tStepNumberDecay;

        // Rannacher smoothing steps at the beginning
         Size nRannacherTimeSteps;

         Size predictionCorretionSteps;

        // local volatility forward equation
         Real x0Density;
         Real localVolEpsProb;
         Size maxIntegrationIterations;

        // variance mesher definition
         Real vLowerEps, vUpperEps, vMin;
         Real v0Density, vLowerBoundDensity, vUpperBoundDensity;

        // do not calculate leverage function if prob is smaller than eps
         Real leverageFctPropEps;

        // algorithm to get to the start configuration at time point one
         FdmHestonGreensFct::Algorithm greensAlgorithm;
         FdmSquareRootFwdOp::TransformationType trafoType;

        // define finite difference scheme
         FdmSchemeDesc schemeDesc;
    };

    class HestonSLVFDMModel : public LazyObject {
      public:
        HestonSLVFDMModel(Handle<LocalVolTermStructure> localVol,
                          Handle<HestonModel> hestonModel,
                          const Date& endDate,
                          HestonSLVFokkerPlanckFdmParams params,
                          bool logging = false,
                          std::vector<Date> mandatoryDates = std::vector<Date>(),
                          Real mixingFactor = 1.0);

        ext::shared_ptr<HestonProcess> hestonProcess() const;
        ext::shared_ptr<LocalVolTermStructure> localVol() const;
        ext::shared_ptr<LocalVolTermStructure> leverageFunction() const;

        struct LogEntry {
            const Time t;
            const ext::shared_ptr<Array> prob;
            const ext::shared_ptr<FdmMesherComposite> mesher;
        };

        const std::list<LogEntry>& logEntries() const;

      protected:
        void performCalculations() const override;

        const Handle<LocalVolTermStructure> localVol_;
        const Handle<HestonModel> hestonModel_;
        const Date endDate_;
        const HestonSLVFokkerPlanckFdmParams params_;
        const std::vector<Date> mandatoryDates_;
        const Real mixingFactor_;

        mutable ext::shared_ptr<LocalVolTermStructure> leverageFunction_;

        const bool logging_;
        mutable std::list<LogEntry> logEntries_;
    };
}


#endif

