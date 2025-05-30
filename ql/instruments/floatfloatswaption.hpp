/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013, 2018 Peter Caspers

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

/*! \file floatfloatswaption.hpp
    \brief floatfloatswaption class
*/

#ifndef quantlib_instruments_floatfloatswaption_hpp
#define quantlib_instruments_floatfloatswaption_hpp

#include <instruments/swaption.hpp>
#include <instruments/floatfloatswap.hpp>
#include <pricingengines/swaption/basketgeneratingengine.hpp>
#include <termstructures/yieldtermstructure.hpp>
#include <termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <models/calibrationhelper.hpp>

namespace QuantLib {

    //! floatfloat swaption class
    /*! \ingroup instruments
    */

    class FloatFloatSwaption : public Option {
      public:
        class arguments;
        class engine;
        FloatFloatSwaption(ext::shared_ptr<FloatFloatSwap> swap,
                           const ext::shared_ptr<Exercise>& exercise,
                           Settlement::Type delivery = Settlement::Physical,
                           Settlement::Method settlementMethod = Settlement::PhysicalOTC);
        //! \name Instrument interface
        //@{
        bool isExpired() const override;
        void setupArguments(PricingEngine::arguments*) const override;
        //@}
        //! \name Inspectors
        //@{
        Settlement::Type settlementType() const { return settlementType_; }
        Settlement::Method settlementMethod() const {
            return settlementMethod_;
        }
        Swap::Type type() const { return swap_->type(); }
        const ext::shared_ptr<FloatFloatSwap> &underlyingSwap() const {
            return swap_;
        }
        //@}
        std::vector<ext::shared_ptr<BlackCalibrationHelper>>
        calibrationBasket(const ext::shared_ptr<SwapIndex>& standardSwapBase,
                          const ext::shared_ptr<SwaptionVolatilityStructure>& swaptionVolatility,
                          BasketGeneratingEngine::CalibrationBasketType basketType =
                              BasketGeneratingEngine::MaturityStrikeByDeltaGamma) const;

      private:
        // arguments
        ext::shared_ptr<FloatFloatSwap> swap_;
        Settlement::Type settlementType_;
        Settlement::Method settlementMethod_;
    };

    //! %Arguments for cms swaption calculation
    class FloatFloatSwaption::arguments : public FloatFloatSwap::arguments,
                                          public Option::arguments {
      public:
        arguments() = default;
        ext::shared_ptr<FloatFloatSwap> swap;
        Settlement::Type settlementType;
        Settlement::Method settlementMethod;
        void validate() const override;
    };

    //! base class for cms swaption engines
    class FloatFloatSwaption::engine
        : public GenericEngine<FloatFloatSwaption::arguments,
                               FloatFloatSwaption::results> {};
}

#endif
