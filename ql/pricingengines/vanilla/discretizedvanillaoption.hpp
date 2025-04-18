/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2007 StatPro Italia srl

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

/*! \file discretizedvanillaoption.hpp
    \brief discretized vanilla option
*/

#ifndef quantlib_discretized_vanilla_option_h
#define quantlib_discretized_vanilla_option_h

#include <discretizedasset.hpp>
#include <methods/lattices/bsmlattice.hpp>
#include <instruments/vanillaoption.hpp>

namespace QuantLib {

    class DiscretizedVanillaOption : public DiscretizedAsset {
      public:
        DiscretizedVanillaOption(const VanillaOption::arguments&,
                                 const StochasticProcess& process,
                                 const TimeGrid& grid = TimeGrid());

        void reset(Size size) override;

        std::vector<Time> mandatoryTimes() const override { return stoppingTimes_; }

      protected:
        void postAdjustValuesImpl() override;

      private:
        void applySpecificCondition();
        VanillaOption::arguments arguments_;
        std::vector<Time> stoppingTimes_;
    };

}





#endif
