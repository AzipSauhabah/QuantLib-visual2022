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

/*! \file localvolrndcalculator.hpp
    \brief local volatility risk neutral terminal density calculation
*/

#ifndef quantlib_local_vol_rnd_calculator_hpp
#define quantlib_local_vol_rnd_calculator_hpp

#include <math/matrix.hpp>
#include <quotes/simplequote.hpp>
#include <patterns/lazyobject.hpp>
#include <termstructures/yieldtermstructure.hpp>
#include <methods/finitedifferences/meshers/fdm1dmesher.hpp>
#include <methods/finitedifferences/utilities/riskneutraldensitycalculator.hpp>

#include <vector>

namespace QuantLib {
	class TimeGrid;
	class Fdm1dMesher;
	class Interpolation;
	class LocalVolTermStructure;

	class LocalVolRNDCalculator : public RiskNeutralDensityCalculator,
								  public LazyObject {
	  public:
            LocalVolRNDCalculator(ext::shared_ptr<Quote> spot,
                                  ext::shared_ptr<YieldTermStructure> rTS,
                                  ext::shared_ptr<YieldTermStructure> qTS,
                                  const ext::shared_ptr<LocalVolTermStructure>& localVol,
                                  Size xGrid = 101,
                                  Size tGrid = 51,
                                  Real x0Density = 0.1,
                                  Real localVolProbEps = 1e-6,
                                  Size maxIter = 10000,
                                  Time gaussianStepSize = -Null<Time>());

            LocalVolRNDCalculator(ext::shared_ptr<Quote> spot,
                                  ext::shared_ptr<YieldTermStructure> rTS,
                                  ext::shared_ptr<YieldTermStructure> qTS,
                                  ext::shared_ptr<LocalVolTermStructure> localVol,
                                  const ext::shared_ptr<TimeGrid>& timeGrid,
                                  Size xGrid = 101,
                                  Real x0Density = 0.1,
                                  Real eps = 1e-6,
                                  Size maxIter = 10000,
                                  Time gaussianStepSize = -Null<Time>());

            Real pdf(Real x, Time t) const override;
            Real cdf(Real x, Time t) const override;
            Real invcdf(Real p, Time t) const override;

            ext::shared_ptr<TimeGrid> timeGrid() const;
            ext::shared_ptr<Fdm1dMesher> mesher(Time t) const;
            std::vector<Size> rescaleTimeSteps() const;

	  protected:
            void performCalculations() const override;

          private:
		Real probabilityInterpolation(Size idx, Real x) const;
		Array rescalePDF(const Array& x, const Array& p) const;


		const Size xGrid_, tGrid_;
		const Real x0Density_;
		const Real localVolProbEps_;
		const Size maxIter_;
		const Time gaussianStepSize_;
		const ext::shared_ptr<Quote> spot_;
		const ext::shared_ptr<LocalVolTermStructure> localVol_;
		const ext::shared_ptr<YieldTermStructure> rTS_;
		const ext::shared_ptr<YieldTermStructure> qTS_;
		const ext::shared_ptr<TimeGrid> timeGrid_;
		mutable std::vector<ext::shared_ptr<Fdm1dMesher> > xm_;
		const ext::shared_ptr<Matrix> pm_;
		mutable std::vector<Size> rescaleTimeSteps_;
		mutable std::vector<ext::shared_ptr<Interpolation> > pFct_;
	};
}

#endif
