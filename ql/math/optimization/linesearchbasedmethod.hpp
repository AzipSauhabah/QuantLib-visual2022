/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2009 Frédéric Degraeve

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

/*! \file linesearchbasedmethod.hpp
    \brief Abstract optimization method class
*/

#ifndef quantlib_line_search_based_optimization_method_h
#define quantlib_line_search_based_optimization_method_h

#include <math/optimization/method.hpp>
#include <math/array.hpp>
#include <shared_ptr.hpp>

namespace QuantLib {

    class LineSearch;

    //! Line search based method
    class LineSearchBasedMethod : public OptimizationMethod {
      public:
        explicit LineSearchBasedMethod(
            ext::shared_ptr<LineSearch> lSearch = ext::shared_ptr<LineSearch>());
        ~LineSearchBasedMethod() override = default;

        EndCriteria::Type minimize(Problem& P, const EndCriteria& endCriteria) override;

      protected:
        //! computes the new search direction
        virtual Array getUpdatedDirection(const Problem &P,
                                          Real gold2,
                                          const Array& gradient) = 0;
        //! line search
        ext::shared_ptr<LineSearch> lineSearch_;
    };

}

#endif
