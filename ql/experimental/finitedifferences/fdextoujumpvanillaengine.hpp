/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file fdextoujumpvanillaengine.hpp
    \brief Finite Differences Ornstein Uhlenbeck plus exponential jumps engine 
           for vanilla options
*/

#ifndef quantlib_fd_simple_ou_jump_swing_engine_hpp
#define quantlib_fd_simple_ou_jump_swing_engine_hpp

#include <pricingengine.hpp>
#include <instruments/vanillaoption.hpp>
#include <methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <experimental/finitedifferences/fdmextoujumpmodelinnervalue.hpp>

namespace QuantLib {

    class YieldTermStructure;
    class ExtOUWithJumpsProcess;

    class FdExtOUJumpVanillaEngine
        : public GenericEngine<VanillaOption::arguments,
                               VanillaOption::results> {
      public:
        typedef FdmExtOUJumpModelInnerValue::Shape Shape;
        FdExtOUJumpVanillaEngine(ext::shared_ptr<ExtOUWithJumpsProcess> p,
                                 ext::shared_ptr<YieldTermStructure> rTS,
                                 Size tGrid = 50,
                                 Size xGrid = 200,
                                 Size yGrid = 50,
                                 ext::shared_ptr<Shape> shape = ext::shared_ptr<Shape>(),
                                 const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer());

        void calculate() const override;

      private:
        const ext::shared_ptr<ExtOUWithJumpsProcess> process_;
        const ext::shared_ptr<YieldTermStructure> rTS_;
        const ext::shared_ptr<Shape> shape_;
        const Size tGrid_, xGrid_, yGrid_;
        const FdmSchemeDesc schemeDesc_;
    };
}

#endif
