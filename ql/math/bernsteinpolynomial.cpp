/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Allen Kuo

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

#include <math/bernsteinpolynomial.hpp>
#include <math/factorial.hpp>
#include <errors.hpp>

namespace QuantLib {

    Real BernsteinPolynomial::get(Natural i,
                                  Natural n,
                                  Real x) {

        Real coeff = Factorial::get(n) /
            (Factorial::get(n-i) * Factorial::get(i));

        return coeff * std::pow(x,int(i)) * std::pow(1.0-x, int(n-i));
    }

}

