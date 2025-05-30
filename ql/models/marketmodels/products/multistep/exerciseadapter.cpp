/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

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

#include <models/marketmodels/products/multistep/exerciseadapter.hpp>

namespace QuantLib {

    ExerciseAdapter::ExerciseAdapter(
                              const Clone<MarketModelExerciseValue>& exercise,
                              Size numberOfProducts)
    : MultiProductMultiStep(exercise->evolution().rateTimes()),
      exercise_(exercise), numberOfProducts_(numberOfProducts),
      isExerciseTime_(exercise->isExerciseTime()) {}

    bool ExerciseAdapter::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelMultiProduct::CashFlow> >&
                                                         generatedCashFlows) {
        std::fill(numberCashFlowsThisStep.begin(),
                  numberCashFlowsThisStep.end(), 0);
        bool done = false;

        exercise_->nextStep(currentState);
        if (isExerciseTime_[currentIndex_]) {
            MarketModelMultiProduct::CashFlow cashflow =
                exercise_->value(currentState);
            numberCashFlowsThisStep[0] = 1;
            generatedCashFlows[0][0] = cashflow;
            done = true;
        }
        ++currentIndex_;
        return done || currentIndex_ == isExerciseTime_.size();
    }

    std::unique_ptr<MarketModelMultiProduct>
    ExerciseAdapter::clone() const {
        return std::unique_ptr<MarketModelMultiProduct>(new ExerciseAdapter(*this));
    }

}

