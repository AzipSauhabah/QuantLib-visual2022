/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file mceuropeanhestonengine.hpp
    \brief Monte Carlo Heston-model engine for European options
*/

#ifndef quantlib_mc_european_heston_engine_hpp
#define quantlib_mc_european_heston_engine_hpp

#include <pricingengines/vanilla/mcvanillaengine.hpp>
#include <processes/hestonprocess.hpp>
#include <utility>

namespace QuantLib {

    //! Monte Carlo Heston-model engine for European options
    /*! \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              reproducing results available in web/literature
    */
    template <class RNG = PseudoRandom,
              class S = Statistics, class P = HestonProcess>
    class MCEuropeanHestonEngine
        : public MCVanillaEngine<MultiVariate,RNG,S> {
      public:
        typedef typename MCVanillaEngine<MultiVariate,RNG,S>::path_pricer_type
            path_pricer_type;
        MCEuropeanHestonEngine(const ext::shared_ptr<P>&,
                               Size timeSteps,
                               Size timeStepsPerYear,
                               bool antitheticVariate,
                               Size requiredSamples,
                               Real requiredTolerance,
                               Size maxSamples,
                               BigNatural seed);
      protected:
        ext::shared_ptr<path_pricer_type> pathPricer() const override;
    };

    //! Monte Carlo Heston European engine factory
    template <class RNG = PseudoRandom,
              class S = Statistics, class P = HestonProcess>
    class MakeMCEuropeanHestonEngine {
      public:
        explicit MakeMCEuropeanHestonEngine(ext::shared_ptr<P>);
        // named parameters
        MakeMCEuropeanHestonEngine& withSteps(Size steps);
        MakeMCEuropeanHestonEngine& withStepsPerYear(Size steps);
        MakeMCEuropeanHestonEngine& withSamples(Size samples);
        MakeMCEuropeanHestonEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCEuropeanHestonEngine& withMaxSamples(Size samples);
        MakeMCEuropeanHestonEngine& withSeed(BigNatural seed);
        MakeMCEuropeanHestonEngine& withAntitheticVariate(bool b = true);
        // conversion to pricing engine
        operator ext::shared_ptr<PricingEngine>() const;
      private:
        ext::shared_ptr<P> process_;
        bool antithetic_ = false;
        Size steps_, stepsPerYear_, samples_, maxSamples_;
        Real tolerance_;
        BigNatural seed_ = 0;
    };


    class EuropeanHestonPathPricer : public PathPricer<MultiPath> {
      public:
        EuropeanHestonPathPricer(Option::Type type,
                                 Real strike,
                                 DiscountFactor discount);
        Real operator()(const MultiPath& Multipath) const override;

      private:
        PlainVanillaPayoff payoff_;
        DiscountFactor discount_;
    };


    // template definitions

    template <class RNG, class S, class P>
    MCEuropeanHestonEngine<RNG, S, P>::MCEuropeanHestonEngine(
                const ext::shared_ptr<P>& process,
                Size timeSteps, Size timeStepsPerYear, bool antitheticVariate,
                Size requiredSamples, Real requiredTolerance,
                Size maxSamples, BigNatural seed)
    : MCVanillaEngine<MultiVariate,RNG,S>(process, timeSteps, timeStepsPerYear,
                                          false, antitheticVariate, false,
                                          requiredSamples, requiredTolerance,
                                          maxSamples, seed) {}


    template <class RNG, class S, class P>
    ext::shared_ptr<
        typename MCEuropeanHestonEngine<RNG,S,P>::path_pricer_type>
    MCEuropeanHestonEngine<RNG,S,P>::pathPricer() const {

        ext::shared_ptr<PlainVanillaPayoff> payoff(
                  ext::dynamic_pointer_cast<PlainVanillaPayoff>(
                                                    this->arguments_.payoff));
        QL_REQUIRE(payoff, "non-plain payoff given");

        ext::shared_ptr<P> process =
            ext::dynamic_pointer_cast<P>(this->process_);
        QL_REQUIRE(process, "Heston like process required");

        return ext::shared_ptr<
            typename MCEuropeanHestonEngine<RNG,S,P>::path_pricer_type>(
                   new EuropeanHestonPathPricer(
                                        payoff->optionType(),
                                        payoff->strike(),
                                        process->riskFreeRate()->discount(
                                                   this->timeGrid().back())));
    }


    template <class RNG, class S, class P>
    inline MakeMCEuropeanHestonEngine<RNG, S, P>::MakeMCEuropeanHestonEngine(
        ext::shared_ptr<P> process)
    : process_(std::move(process)), steps_(Null<Size>()), stepsPerYear_(Null<Size>()),
      samples_(Null<Size>()), maxSamples_(Null<Size>()), tolerance_(Null<Real>()) {}

    template <class RNG, class S,class P>
    inline MakeMCEuropeanHestonEngine<RNG,S,P>&
    MakeMCEuropeanHestonEngine<RNG,S,P>::withSteps(Size steps) {
        QL_REQUIRE(stepsPerYear_ == Null<Size>(),
                   "number of steps per year already set");
        steps_ = steps;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCEuropeanHestonEngine<RNG,S,P>&
    MakeMCEuropeanHestonEngine<RNG,S,P>::withStepsPerYear(Size steps) {
        QL_REQUIRE(steps_ == Null<Size>(),
                   "number of steps already set");
        stepsPerYear_ = steps;
        return *this;
    }

    template <class RNG, class S,class P>
    inline MakeMCEuropeanHestonEngine<RNG,S,P>&
    MakeMCEuropeanHestonEngine<RNG,S,P>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCEuropeanHestonEngine<RNG,S,P>&
    MakeMCEuropeanHestonEngine<RNG,S,P>::withAbsoluteTolerance(Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCEuropeanHestonEngine<RNG,S,P>&
    MakeMCEuropeanHestonEngine<RNG,S,P>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCEuropeanHestonEngine<RNG,S,P>&
    MakeMCEuropeanHestonEngine<RNG,S,P>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S, class P>
    inline MakeMCEuropeanHestonEngine<RNG,S,P>&
    MakeMCEuropeanHestonEngine<RNG,S,P>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S, class P>
    inline
    MakeMCEuropeanHestonEngine<RNG,S,P>::
    operator ext::shared_ptr<PricingEngine>() const {
        QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
                   "number of steps not given");
        return ext::shared_ptr<PricingEngine>(
               new MCEuropeanHestonEngine<RNG,S,P>(process_,
                                                   steps_,
                                                   stepsPerYear_,
                                                   antithetic_,
                                                   samples_, tolerance_,
                                                   maxSamples_,
                                                   seed_));
    }



    inline EuropeanHestonPathPricer::EuropeanHestonPathPricer(
                                                 Option::Type type,
                                                 Real strike,
                                                 DiscountFactor discount)
    : payoff_(type, strike), discount_(discount) {
        QL_REQUIRE(strike>=0.0,
                   "strike less than zero not allowed");
    }

    inline Real EuropeanHestonPathPricer::operator()(
                                           const MultiPath& multiPath) const {
        const Path& path = multiPath[0];
        const Size n = multiPath.pathSize();
        QL_REQUIRE(n>0, "the path cannot be empty");

        return payoff_(path.back()) * discount_;
    }

}


#endif
