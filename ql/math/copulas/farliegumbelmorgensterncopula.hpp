/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Marek Glowacki

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

/*! \file farliegumbelmorgensterncopula.hpp
    \brief Farlie-Gumbel-Morgenstern copula
*/

#ifndef quantlib_math_Farlie_Gumbel_Morgenstern_copula_h
#define quantlib_math_Farlie_Gumbel_Morgenstern_copula_h

#include <types.hpp>
#include <functional>

namespace QuantLib {

    //! Farlie-Gumbel-Morgenstern copula
    class FarlieGumbelMorgensternCopula {
      public:
        FarlieGumbelMorgensternCopula(Real theta);
        Real operator()(Real x, Real y) const;
      private:
        Real theta_;
    };
    
}

#endif
