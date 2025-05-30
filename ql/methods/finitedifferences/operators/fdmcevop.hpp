/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Klaus Spanderen

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

/*! \file fdmcevop.hpp
    \brief FDM operator for the Constant Elasticity of Variance (CEV) model
*/

#ifndef quantlib_fdm_cev_op_hpp
#define quantlib_fdm_cev_op_hpp

#include <methods/finitedifferences/operators/ninepointlinearop.hpp>
#include <methods/finitedifferences/operators/triplebandlinearop.hpp>
#include <methods/finitedifferences/operators/fdmlinearopcomposite.hpp>

namespace QuantLib {

    //! constant elasticity of variance process (absorbing boundary at f=0)
    /*! \f[
         df_t = \alpha f_t^\beta \mathrm{d}W_t
        \f]
    */

    class FdmMesher;
    class YieldTermStructure;

    class FdmCEVOp : public FdmLinearOpComposite {
      public:

        FdmCEVOp(
            const ext::shared_ptr<FdmMesher>& mesher,
            const ext::shared_ptr<YieldTermStructure>& rTS,
            Real f0, Real alpha, Real beta,
            Size direction);

        Size size() const override;
        void setTime(Time t1, Time t2) override;

        Array apply(const Array& r) const override;
        Array apply_mixed(const Array& r) const override;
        Array apply_direction(Size direction, const Array& r) const override;
        Array solve_splitting(Size direction, const Array& r, Real s) const override;
        Array preconditioner(const Array& r, Real s) const override;

        std::vector<SparseMatrix> toMatrixDecomp() const override;

      private:
        const ext::shared_ptr<YieldTermStructure>& rTS_;
        const Size direction_;
        const TripleBandLinearOp dxxMap_;
        TripleBandLinearOp mapT_;
    };
}

#endif
