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

#ifndef quantlib_nothing_exercise_value_hpp
#define quantlib_nothing_exercise_value_hpp

#include <models/marketmodels/callability/exercisevalue.hpp>
#include <models/marketmodels/evolutiondescription.hpp>

namespace QuantLib {

    class NothingExerciseValue : public MarketModelExerciseValue {
      public:
        //! \todo use Payoff
        NothingExerciseValue(const std::vector<Time>& rateTimes,
                             std::valarray<bool> isExerciseTime = std::valarray<bool>());
        Size numberOfExercises() const override;
        // including any time at which state should be updated
        const EvolutionDescription& evolution() const override;
        std::vector<Time> possibleCashFlowTimes() const override;
        void nextStep(const CurveState&) override;
        void reset() override;
        // whether or not evolution times are exercise times
        std::valarray<bool> isExerciseTime() const override;
        MarketModelMultiProduct::CashFlow value(const CurveState&) const override;
        std::unique_ptr<MarketModelExerciseValue> clone() const override;
      private:
        Size numberOfExercises_;
        std::vector<Time> rateTimes_;
        std::valarray<bool> isExerciseTime_;
        EvolutionDescription evolution_;
        // evolving
        Size currentIndex_ = 0;
        MarketModelMultiProduct::CashFlow cf_;
    };

}


#endif
