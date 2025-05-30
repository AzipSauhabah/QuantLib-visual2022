/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2018 Matthias Lungwitz

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

/*! \file pribor.hpp
\brief %PRIBOR rate
*/

#ifndef quantlib_pribor_hpp
#define quantlib_pribor_hpp

#include <indexes/iborindex.hpp>
#include <time/calendars/czechrepublic.hpp>
#include <time/daycounters/actual360.hpp>
#include <currencies/europe.hpp>

namespace QuantLib {

	//! %PRIBOR rate
	/*! Prague Interbank Offered Rate fixed by CFBF.

	Conventions are taken from
	OpenGamma "Interest Rate Instruments and Market Conventions
	Guide" as well as
	https://cfbf.cz/wp-content/uploads/2018/02/pribor-rules.pdf

	\warning Roll convention and EoM not yet checked.
	*/
	class Pribor : public IborIndex {
	public:
		Pribor(const Period& tenor,
               const Handle<YieldTermStructure>& h = {})
			: IborIndex("PRIBOR", tenor, (tenor == 1 * Days ? 0 : 2), CZKCurrency(),
				CzechRepublic(), ModifiedFollowing, false,
				Actual360(), h) {}
	};

}


#endif
