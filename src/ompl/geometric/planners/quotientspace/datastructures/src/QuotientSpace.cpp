/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2019, University of Stuttgart
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the University of Stuttgart nor the names
*     of its contributors may be used to endorse or promote products
*     derived from this software without specific prior written
*     permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/* Author: Andreas Orthey */
#include <ompl/geometric/planners/quotientspace/datastructures/QuotientSpace.h>

#include <ompl/base/objectives/PathLengthOptimizationObjective.h>
#include <ompl/base/goals/GoalSampleableRegion.h>
#include <ompl/base/spaces/SO2StateSpace.h>
#include <ompl/base/spaces/SO3StateSpace.h>
#include <ompl/base/spaces/SE2StateSpace.h>
#include <ompl/base/spaces/SE3StateSpace.h>

namespace ob = ompl::base;
namespace og = ompl::geometric;
using namespace ob;
using namespace og;


const unsigned int verbose{0};

QuotientSpace::QuotientSpace(const ob::SpaceInformationPtr &si, QuotientSpace *parent_)
  : ob::Planner(si, "QuotientSpace"), Q1(si), parent_(parent_)
{
    id_ = counter_++;

    if (parent_ != nullptr)
        parent_->setChild(this);  // need to be able to traverse down the tree

    const StateSpacePtr Q1_space = Q1->getStateSpace();

    OMPL_DEVMSG1("--- QuotientSpace %d", id_);

    if (parent_ == nullptr)
    {
        OMPL_DEVMSG1("ATOMIC_RN dimension: %d measure: %f", Q1_space->getDimension(), Q1_space->getMeasure());
        type_ = ATOMIC_RN;
    }
    else
    {
        Q0 = parent_->getSpaceInformation();
        const StateSpacePtr Q0_space = Q0->getStateSpace();
        // X1 = Q1 / Q0
        const StateSpacePtr X1_space = computeQuotientSpace(Q1_space, Q0_space);

        if (X1_space != nullptr)
        {
            X1 = std::make_shared<SpaceInformation>(X1_space);
            X1_sampler_ = X1->allocStateSampler();
            if (Q0_space->getDimension() + X1_space->getDimension() != Q1_space->getDimension())
            {
                OMPL_ERROR("QuotientSpace Dimensions are wrong.");
                exit(0);
            }
            OMPL_DEVMSG1("Q0 dimension: %d measure: %f", Q0_space->getDimension(), Q0_space->getMeasure());
            OMPL_DEVMSG1("X1 dimension: %d measure: %f", X1_space->getDimension(), X1_space->getMeasure());
            OMPL_DEVMSG1("Q1 dimension: %d measure: %f", Q1_space->getDimension(), Q1_space->getMeasure());
            if ((Q0_space->getMeasure() <= 0) || (X1_space->getMeasure() <= 0) || (Q1_space->getMeasure() <= 0))
            {
                OMPL_ERROR("Zero-measure QuotientSpace detected.");
                exit(0);
            }
            if (!X1_sampler_)
            {
                X1_sampler_ = X1->allocStateSampler();
            }
            checkSpaceHasFiniteMeasure(X1_space);
        }
        else
        {
            OMPL_DEVMSG1("Q0 dimension: %d measure: %f", Q0_space->getDimension(), Q0_space->getMeasure());
            OMPL_DEVMSG1("Q1 dimension: %d measure: %f", Q1_space->getDimension(), Q1_space->getMeasure());
        }
        checkSpaceHasFiniteMeasure(Q0_space);
    }
    checkSpaceHasFiniteMeasure(Q1_space);

    if (!Q1_valid_sampler_)
    {
        Q1_valid_sampler_ = Q1->allocValidStateSampler();
    }
    if (!Q1_sampler_)
    {
        Q1_sampler_ = Q1->allocStateSampler();
    }
    if (parent_ != nullptr)
    {
        s_Q0_tmp_ = Q0->allocState();
        if (X1_dimension_ > 0)
            s_X1_tmp_ = X1->allocState();
    }
}
QuotientSpace::~QuotientSpace()
{
    if (parent_ != nullptr)
    {
        if (s_Q0_tmp_)
            Q0->freeState(s_Q0_tmp_);
        if (X1 && s_X1_tmp_)
            X1->freeState(s_X1_tmp_);
    }
}

void QuotientSpace::setup()
{
    BaseT::setup();
    hasSolution_ = false;
    firstRun_ = true;
}

void QuotientSpace::clear()
{
    BaseT::clear();
    totalNumberOfSamples_ = 0;
    totalNumberOfFeasibleSamples_ = 0;

    hasSolution_ = false;
    firstRun_ = true;
    if (parent_ == nullptr && X1_dimension_ > 0)
        X1_sampler_.reset();

    pdef_->clearSolutionPaths();
}

void QuotientSpace::checkSpaceHasFiniteMeasure(const ob::StateSpacePtr space) const
{
    if (space->getMeasure() >= std::numeric_limits<double>::infinity())
    {
        OMPL_ERROR("QuotientSpace has no bounds");
        const StateSpacePtr Q0_space = Q0->getStateSpace();
        const StateSpacePtr Q1_space = Q1->getStateSpace();
        OMPL_ERROR("Q0 dimension: %d measure: %f", Q0_space->getDimension(), Q0_space->getMeasure());
        OMPL_ERROR("Q1 dimension: %d measure: %f", Q1_space->getDimension(), Q1_space->getMeasure());
        if (X1 != nullptr)
        {
            const StateSpacePtr X1_space = X1->getStateSpace();
            OMPL_ERROR("X1 dimension: %d measure: %f", X1_space->getDimension(), X1_space->getMeasure());
        }
        exit(0);
    }
}

ob::PlannerStatus QuotientSpace::solve(const ob::PlannerTerminationCondition &ptc)
{
    (void)ptc;
    OMPL_ERROR("A Quotient-Space cannot be solved alone. Use class MultiQuotient to solve Quotient-Spaces.");
    exit(1);
}


void QuotientSpace::setProblemDefinition(const ob::ProblemDefinitionPtr &pdef)
{
    BaseT::setProblemDefinition(pdef);

    if (pdef_->hasOptimizationObjective())
    {
        opt_ = pdef_->getOptimizationObjective();
    }
    else
    {
        opt_ = std::make_shared<ob::PathLengthOptimizationObjective>(si_);
    }
}

unsigned int QuotientSpace::counter_ = 0;
void QuotientSpace::resetCounter()
{
    QuotientSpace::counter_ = 0;
}

const StateSpacePtr QuotientSpace::computeQuotientSpace(const StateSpacePtr Q1, const StateSpacePtr Q0)
{
    type_ = identifyQuotientSpaceType(Q1, Q0);

    StateSpacePtr X1{nullptr};
    Q1_dimension_ = Q1->getDimension();
    Q0_dimension_ = Q0->getDimension();

    if (Q0_dimension_ == 0 || Q1_dimension_ == 0)
    {
        OMPL_ERROR("Detected Zero-dimensional QuotientSpace.");
        OMPL_ERROR("Q0 has dimension %d.", Q0_dimension_);
        OMPL_ERROR("Q1 has dimension %d.", Q1_dimension_);
        exit(0);
    }

    switch (type_)
    {
        case IDENTITY_SPACE_RN:
        case IDENTITY_SPACE_SE2:
        case IDENTITY_SPACE_SE2RN:
        case IDENTITY_SPACE_SO2RN:
        case IDENTITY_SPACE_SE3:
        case IDENTITY_SPACE_SE3RN:
        {
            X1_dimension_ = 0;
            break;
        }
        case RN_RM:
        {
            unsigned int N = Q1_dimension_ - Q0_dimension_;
            X1 = std::make_shared<ob::RealVectorStateSpace>(N);
            X1_dimension_ = N;

            RealVectorBounds Q1_bounds = std::static_pointer_cast<ob::RealVectorStateSpace>(Q1)->getBounds();
            std::vector<double> low;
            low.resize(N);
            std::vector<double> high;
            high.resize(N);
            RealVectorBounds X1_bounds(N);
            for (unsigned int k = 0; k < N; k++)
            {
                X1_bounds.setLow(k, Q1_bounds.low.at(k + Q0_dimension_));
                X1_bounds.setHigh(k, Q1_bounds.high.at(k + Q0_dimension_));
            }
            std::static_pointer_cast<ob::RealVectorStateSpace>(X1)->setBounds(X1_bounds);

            break;
        }
        case SE2_R2:
        {
            X1_dimension_ = 1;
            X1 = std::make_shared<ob::SO2StateSpace>();
            break;
        }
        case SE3_R3:
        {
            X1_dimension_ = 3;
            X1 = std::make_shared<ob::SO3StateSpace>();
            break;
        }
        case SE2RN_SE2:
        case SE3RN_SE3:
        case SO2RN_SO2:
        {
            ob::CompoundStateSpace *Q1_compound = Q1->as<ob::CompoundStateSpace>();
            const std::vector<StateSpacePtr> Q1_decomposed = Q1_compound->getSubspaces();

            X1_dimension_ = Q1_decomposed.at(1)->getDimension();

            X1 = std::make_shared<ob::RealVectorStateSpace>(X1_dimension_);
            std::static_pointer_cast<ob::RealVectorStateSpace>(X1)->setBounds(
                std::static_pointer_cast<ob::RealVectorStateSpace>(Q1_decomposed.at(1))->getBounds());

            break;
        }
        case SE2RN_R2:
        {
            ob::CompoundStateSpace *Q1_compound = Q1->as<ob::CompoundStateSpace>();
            const std::vector<StateSpacePtr> Q1_decomposed = Q1_compound->getSubspaces();
            const std::vector<StateSpacePtr> Q1_SE2_decomposed =
                Q1_decomposed.at(0)->as<ob::CompoundStateSpace>()->getSubspaces();

            const ob::RealVectorStateSpace *Q1_RN = Q1_decomposed.at(1)->as<ob::RealVectorStateSpace>();
            unsigned int N = Q1_RN->getDimension();

            ob::StateSpacePtr SO2(new ob::SO2StateSpace());
            ob::StateSpacePtr RN(new ob::RealVectorStateSpace(N));
            RN->as<ob::RealVectorStateSpace>()->setBounds(Q1_RN->getBounds());

            X1 = SO2 + RN;
            X1_dimension_ = 1 + N;
            break;
        }
        case SE3RN_R3:
        {
            ob::CompoundStateSpace *Q1_compound = Q1->as<ob::CompoundStateSpace>();
            const std::vector<StateSpacePtr> Q1_decomposed = Q1_compound->getSubspaces();
            const std::vector<StateSpacePtr> Q1_SE3_decomposed =
                Q1_decomposed.at(0)->as<ob::CompoundStateSpace>()->getSubspaces();

            // const ob::SE3StateSpace *Q1_SE3 = Q1_SE3_decomposed.at(0)->as<ob::SE3StateSpace>();
            // const ob::SO3StateSpace *Q1_SO3 = Q1_SE3_decomposed.at(1)->as<ob::SO3StateSpace>();
            const ob::RealVectorStateSpace *Q1_RN = Q1_decomposed.at(1)->as<ob::RealVectorStateSpace>();
            unsigned int N = Q1_RN->getDimension();

            ob::StateSpacePtr SO3(new ob::SO3StateSpace());
            ob::StateSpacePtr RN(new ob::RealVectorStateSpace(N));
            RN->as<ob::RealVectorStateSpace>()->setBounds(Q1_RN->getBounds());

            X1 = SO3 + RN;
            X1_dimension_ = 3 + N;
            break;
        }
        case SE2RN_SE2RM:
        case SO2RN_SO2RM:
        case SE3RN_SE3RM:
        {
            ob::CompoundStateSpace *Q1_compound = Q1->as<ob::CompoundStateSpace>();
            const std::vector<StateSpacePtr> Q1_decomposed = Q1_compound->getSubspaces();
            ob::CompoundStateSpace *Q0_compound = Q0->as<ob::CompoundStateSpace>();
            const std::vector<StateSpacePtr> Q0_decomposed = Q0_compound->getSubspaces();

            unsigned int N = Q1_decomposed.at(1)->getDimension();
            unsigned int M = Q0_decomposed.at(1)->getDimension();
            X1_dimension_ = N - M;
            X1 = std::make_shared<ob::RealVectorStateSpace>(X1_dimension_);

            RealVectorBounds Q1_bounds =
                std::static_pointer_cast<ob::RealVectorStateSpace>(Q1_decomposed.at(1))->getBounds();
            std::vector<double> low;
            low.resize(X1_dimension_);
            std::vector<double> high;
            high.resize(X1_dimension_);
            RealVectorBounds X1_bounds(X1_dimension_);
            for (unsigned int k = 0; k < X1_dimension_; k++)
            {
                X1_bounds.setLow(k, Q1_bounds.low.at(k + M));
                X1_bounds.setHigh(k, Q1_bounds.high.at(k + M));
            }
            std::static_pointer_cast<ob::RealVectorStateSpace>(X1)->setBounds(X1_bounds);
            break;
        }
        default:
        {
            OMPL_ERROR("Unknown QuotientSpace type: %d", type_);
            exit(0);
        }
    }
    return X1;
}

QuotientSpace::QuotientSpaceType QuotientSpace::identifyQuotientSpaceType(const StateSpacePtr Q1, const StateSpacePtr Q0)
{
    //
    // We can currently handle 11 types of quotient-space mappings
    // 
    //   (1) Q1 Rn     , Q0 Rm     [0<m<=n]  => X1 = R(n-m) \union {\emptyset}
    //   (2) Q1 SE2    , Q0 R2               => X1 = SO2
    //   (3) Q1 SE3    , Q0 R3               => X1 = SO3
    //
    //   (4) Q1 SE3xRn , Q0 SE3              => X1 = Rn
    //   (5) Q1 SE3xRn , Q0 R3               => X1 = SO3xRn
    //   (6) Q1 SE3xRn , Q0 SE3xRm [0<m<=n ] => X1 = R(n-m) \union {\emptyset}
    //
    //   (7) Q1 SE2xRn , Q0 SE2              => X1 = Rn
    //   (8) Q1 SE2xRn , Q0 R2               => X1 = SO2xRN
    //   (9) Q1 SE2xRn , Q0 SE2xRm [0<m<=n ] => X1 = R(n-m) \union {\emptyset}
    //
    //  (10) Q1 SO2xRn , Q0 SO2              => X1 = Rn
    //  (11) Q1 SO2xRn , Q0 SO2xRm [0<m<=n ] => X1 = R(n-m) \union {\emptyset}

    if (!Q1->isCompound())
    {
        ///##############################################################################/
        //------------------ non-compound cases:
        ///##############################################################################/
        //
        //------------------ (1) Q1 = Rn, Q0 = Rm, 0<m<n, X1 = R(n-m)
        if (Q1->getType() == ob::STATE_SPACE_REAL_VECTOR)
        {
            unsigned int n = Q1->getDimension();
            if (Q0->getType() == ob::STATE_SPACE_REAL_VECTOR)
            {
                unsigned int m = Q0->getDimension();
                if (n > m && m > 0)
                {
                    type_ = RN_RM;
                }
                else
                {
                    if (n == m && m > 0)
                    {
                        type_ = IDENTITY_SPACE_RN;
                    }
                    else
                    {
                        
                        OMPL_ERROR("Not allowed: dimensionality needs to be monotonically increasing.");
                        OMPL_ERROR("We require n >= m > 0 but have n=%d >= m=%d > 0", n, m);
                        exit(0);
                    }
                }
            }
            else
            {
                OMPL_ERROR( "Q1 is R^%d but Q0 type %d is not handled.", n, Q0->getType());
                exit(0);
            }
        }
        else
        {
            OMPL_ERROR("Q1 is non-compound state, but its type %d is not handled.", Q1->getType());
            exit(0);
        }
    }
    else
    {
        ///##############################################################################/
        //------------------ compound cases:
        ///##############################################################################/
        //
        //------------------ (2) Q1 = SE2, Q0 = R2, X1 = SO2
        ///##############################################################################/
        if (Q1->getType() == ob::STATE_SPACE_SE2)
        {
            if (Q0->getType() == ob::STATE_SPACE_REAL_VECTOR)
            {
                if (Q0->getDimension() == 2)
                {
                    type_ = SE2_R2;
                }
                else
                {
                    OMPL_ERROR("Q1 is SE2 but Q0 type %d is of dimension %d", Q0->getType(), Q0->getDimension());
                    exit(0);
                }
            }
            else
            {
                if (Q0->getType() == ob::STATE_SPACE_SE2)
                {
                    type_ = IDENTITY_SPACE_SE2;
                }
                else
                {
                    OMPL_ERROR("Q1 is SE2 but Q0 type %d is not handled.", Q0->getType());
                    exit(0);
                }
            }
        }
        //------------------ (3) Q1 = SE3, Q0 = R3, X1 = SO3
        ///##############################################################################/
        else if (Q1->getType() == ob::STATE_SPACE_SE3)
        {
            if (Q0->getType() == ob::STATE_SPACE_REAL_VECTOR)
            {
                if (Q0->getDimension() == 3)
                {
                    type_ = SE3_R3;
                }
                else
                {
                    OMPL_ERROR("Q1 is SE3 but Q0 type %d is of dimension %d.", Q0->getType(), Q0->getDimension());
                    exit(0);
                }
            }
            else
            {
                if (Q0->getType() == ob::STATE_SPACE_SE3)
                {
                    type_ = IDENTITY_SPACE_SE3;
                }
                else
                {
                    OMPL_ERROR("Q1 is SE2 but Q0 type %d is not handled.", Q0->getType());
                    exit(0);
                }
                OMPL_ERROR("Q1 is SE3 but Q0 type %d is not handled.", Q0->getType());
                exit(0);
            }
        }
        ///##############################################################################/
        else
        {
            ob::CompoundStateSpace *Q1_compound = Q1->as<ob::CompoundStateSpace>();
            const std::vector<StateSpacePtr> Q1_decomposed = Q1_compound->getSubspaces();
            unsigned int Q1_subspaces = Q1_decomposed.size();
            if (Q1_subspaces == 2)
            {
                if (Q1_decomposed.at(0)->getType() == ob::STATE_SPACE_SE3 &&
                    Q1_decomposed.at(1)->getType() == ob::STATE_SPACE_REAL_VECTOR)
                {
                    unsigned int n = Q1_decomposed.at(1)->getDimension();
                    if (Q0->getType() == ob::STATE_SPACE_SE3)
                    {
                        //------------------ (4) Q1 = SE3xRn, Q0 = SE3, X1 = Rn
                        ///##############################################################################/
                        type_ = SE3RN_SE3;
                    }
                    else if (Q0->getType() == ob::STATE_SPACE_REAL_VECTOR)
                    {
                        //------------------ (5) Q1 = SE3xRn, Q0 = R3, X1 = SO3xRN
                        ///##############################################################################/
                        unsigned int m = Q0->getDimension();
                        if (m == 3)
                        {
                            type_ = SE3RN_R3;
                        }
                        else
                        {
                            OMPL_ERROR("Not allowed. Q0 needs to be 3-dimensional but is %d dimensional", m);
                            exit(0);
                        }
                    }
                    else
                    {
                        //------------------ (6) Q1 = SE3xRn, Q0 = SE3xRm, X1 = R(n-m)
                        ///##############################################################################/
                        ob::CompoundStateSpace *Q0_compound = Q0->as<ob::CompoundStateSpace>();
                        const std::vector<StateSpacePtr> Q0_decomposed = Q0_compound->getSubspaces();
                        unsigned int Q0_subspaces = Q0_decomposed.size();
                        if (Q0_subspaces == 2)
                        {
                            if (Q1_decomposed.at(0)->getType() == ob::STATE_SPACE_SE3 &&
                                Q1_decomposed.at(1)->getType() == ob::STATE_SPACE_REAL_VECTOR)
                            {
                                unsigned int m = Q0_decomposed.at(1)->getDimension();
                                if (m < n && m > 0)
                                {
                                    type_ = SE3RN_SE3RM;
                                }
                                else
                                {
                                    if(m == n){
                                        type_ = IDENTITY_SPACE_SE3RN;
                                    }
                                    else
                                    {
                                        OMPL_ERROR("We require n >= m > 0, but have n=%d >= m=%d > 0.", n, m);
                                        exit(0);
                                    }
                                }
                            }
                        }
                        else
                        {
                            OMPL_ERROR("State compound with %d subspaces not handled.", Q0_subspaces);
                            exit(0);
                        }
                    }
                }
                else
                {
                    if (Q1_decomposed.at(0)->getType() == ob::STATE_SPACE_SE2 &&
                        Q1_decomposed.at(1)->getType() == ob::STATE_SPACE_REAL_VECTOR)
                    {
                        unsigned int n = Q1_decomposed.at(1)->getDimension();
                        if (Q0->getType() == ob::STATE_SPACE_SE2)
                        {
                            //------------------ (7) Q1 = SE2xRn, Q0 = SE2, X1 = Rn
                            ///##############################################################################/
                            type_ = SE2RN_SE2;
                        }
                        else if (Q0->getType() == ob::STATE_SPACE_REAL_VECTOR)
                        {
                            //------------------ (8) Q1 = SE2xRn, Q0 = R2, X1 = SO2xRN
                            ///##############################################################################/
                            unsigned int m = Q0->getDimension();
                            if (m == 2)
                            {
                                type_ = SE2RN_R2;
                            }
                            else
                            {
                                OMPL_ERROR("Not allowed. Q0 needs to be 2-dimensional but is %d dimensional", m);
                                exit(0);
                            }
                        }
                        else
                        {
                            //------------------ (9) Q1 = SE2xRn, Q0 = SE2xRm, X1 = R(n-m)
                            ///##############################################################################/
                            ob::CompoundStateSpace *Q0_compound = Q0->as<ob::CompoundStateSpace>();
                            const std::vector<StateSpacePtr> Q0_decomposed = Q0_compound->getSubspaces();
                            unsigned int Q0_subspaces = Q0_decomposed.size();
                            if (Q0_subspaces == 2)
                            {
                                if (Q1_decomposed.at(0)->getType() == ob::STATE_SPACE_SE2 &&
                                    Q1_decomposed.at(1)->getType() == ob::STATE_SPACE_REAL_VECTOR)
                                {
                                    unsigned int m = Q0_decomposed.at(1)->getDimension();
                                    if (m < n && m > 0)
                                    {
                                        type_ = SE2RN_SE2RM;
                                    }
                                    else
                                    {
                                        if(m == n){
                                            type_ = IDENTITY_SPACE_SE2RN;
                                        }
                                        else
                                        {
                                            OMPL_ERROR("We require n >= m > 0, but have n=%d >= m=%d > 0.", n, m);
                                            exit(0);
                                        }
                                    }
                                }else{

                                }
                            }
                            else
                            {
                                OMPL_ERROR("QO is compound with %d subspaces, but we only handle 2.", Q0_subspaces);
                                exit(0);
                            }
                        }
										}else if(Q1_decomposed.at(0)->getType() == base::STATE_SPACE_SO2 
												&& Q1_decomposed.at(1)->getType() == base::STATE_SPACE_REAL_VECTOR){
												if( Q0->getType() == base::STATE_SPACE_SO2 ){
                            //------------------ (10) Q1 = SO2xRn, Q0 = SO2, X1 = Rn
                            ///##############################################################################/
                            type_ = SO2RN_SO2;
                        }else{
                            //------------------ (11) Q1 = SO2xRn, Q0 = SO2xRm, X1 = R(n-m)
                            ///##############################################################################/
                            if(Q0->isCompound())
                            {
                                ob::CompoundStateSpace *Q0_compound = Q0->as<ob::CompoundStateSpace>();
                                const std::vector<StateSpacePtr> Q0_decomposed = Q0_compound->getSubspaces();
                                unsigned int Q0_subspaces = Q0_decomposed.size();
                                if (Q0_subspaces == 2)
                                {
                                    if (Q1_decomposed.at(0)->getType() == ob::STATE_SPACE_SO2 &&
                                        Q1_decomposed.at(1)->getType() == ob::STATE_SPACE_REAL_VECTOR)
                                    {
                                        unsigned int n = Q1_decomposed.at(1)->getDimension();
                                        unsigned int m = Q0_decomposed.at(1)->getDimension();
                                        if (m < n && m > 0){
                                            type_ = SO2RN_SO2RM;
                                        }else
                                        {
                                            if (m == n){
                                                type_ = IDENTITY_SPACE_SO2RN;
                                            }else
                                            {
                                                OMPL_ERROR("We require n >= m > 0 but have n=%d >= m=%d > 0.", n, m);
                                                exit(0);
                                            }
                                        }
                                    }else{
                                        OMPL_ERROR("Cannot project onto type %d.", Q1->getType());
                                        exit(0);
                                    }
                                }else{
                                    OMPL_ERROR("Q0 has %d subspaces. We can handle only 2.", Q0_subspaces);
                                    exit(0);
                                }
                            }else{
                                OMPL_ERROR("Cannot project onto type %d.", Q0->getType());
                                exit(0);
                            }
												}
                    }else
                    {
                        OMPL_ERROR("State compound %d and %d not recognized.", 
                            Q1_decomposed.at(0)->getType(),
                            Q1_decomposed.at(1)->getType());
                        exit(0);
                    }
                }
            }
            else
            {
                OMPL_ERROR("Q1 has %d subspaces, but we only support 2.", Q1_subspaces);
                exit(0);
            }
        }
    }
    return type_;
}

void QuotientSpace::mergeStates(const ob::State *qQ0, const ob::State *qX1, ob::State *qQ1) const
{
    ////input : qQ0 \in Q0, qX1 \in X1
    ////output: qQ1 = qQ0 \circ qX1 \in Q1
    const StateSpacePtr Q1_space = Q1->getStateSpace();
    const StateSpacePtr X1_space = X1->getStateSpace();
    const StateSpacePtr Q0_space = parent_->getSpaceInformation()->getStateSpace();

    switch (type_)
    {
        case IDENTITY_SPACE_RN:
        case IDENTITY_SPACE_SE2:
        case IDENTITY_SPACE_SE2RN:
        case IDENTITY_SPACE_SO2RN:
        case IDENTITY_SPACE_SE3:
        case IDENTITY_SPACE_SE3RN:
        {
            OMPL_ERROR("Cannot merge states for Identity space");
            exit(0);
        }
        case RN_RM:
        {
            ob::RealVectorStateSpace::StateType *sQ1 = qQ1->as<RealVectorStateSpace::StateType>();
            const ob::RealVectorStateSpace::StateType *sQ0 = qQ0->as<RealVectorStateSpace::StateType>();
            const ob::RealVectorStateSpace::StateType *sX1 = qX1->as<RealVectorStateSpace::StateType>();

            for (unsigned int k = 0; k < Q0_dimension_; k++)
            {
                sQ1->values[k] = sQ0->values[k];
            }
            for (unsigned int k = Q0_dimension_; k < Q1_dimension_; k++)
            {
                sQ1->values[k] = sX1->values[k - Q0_dimension_];
            }
            break;
        }
        case SE2_R2:
        {
            ob::SE2StateSpace::StateType *sQ1 = qQ1->as<SE2StateSpace::StateType>();
            const ob::RealVectorStateSpace::StateType *sQ0 = qQ0->as<RealVectorStateSpace::StateType>();
            const ob::SO2StateSpace::StateType *sX1 = qX1->as<SO2StateSpace::StateType>();

            sQ1->setXY(sQ0->values[0], sQ0->values[1]);
            sQ1->setYaw(sX1->value);

            break;
        }
        case SE3_R3:
        {
            ob::SE3StateSpace::StateType *sQ1 = qQ1->as<SE3StateSpace::StateType>();
            ob::SO3StateSpace::StateType *sQ1_rotation = &sQ1->rotation();

            const ob::RealVectorStateSpace::StateType *sQ0 = qQ0->as<RealVectorStateSpace::StateType>();
            const ob::SO3StateSpace::StateType *sX1 = qX1->as<SO3StateSpace::StateType>();

            sQ1->setXYZ(sQ0->values[0], sQ0->values[1], sQ0->values[2]);

            sQ1_rotation->x = sX1->x;
            sQ1_rotation->y = sX1->y;
            sQ1_rotation->z = sX1->z;
            sQ1_rotation->w = sX1->w;

            break;
        }
        case SE3RN_R3:
        {
            ob::SE3StateSpace::StateType *sQ1_SE3 = qQ1->as<ob::CompoundState>()->as<SE3StateSpace::StateType>(0);
            ob::SO3StateSpace::StateType *sQ1_SO3 = &sQ1_SE3->rotation();
            ob::RealVectorStateSpace::StateType *sQ1_RN =
                qQ1->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            const ob::RealVectorStateSpace::StateType *sQ0 = qQ0->as<RealVectorStateSpace::StateType>();
            const ob::SO3StateSpace::StateType *sX1_SO3 =
                qX1->as<ob::CompoundState>()->as<ob::SO3StateSpace::StateType>(0);
            const ob::RealVectorStateSpace::StateType *sX1_RN =
                qX1->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            sQ1_SE3->setXYZ(sQ0->values[0], sQ0->values[1], sQ0->values[2]);
            sQ1_SO3->x = sX1_SO3->x;
            sQ1_SO3->y = sX1_SO3->y;
            sQ1_SO3->z = sX1_SO3->z;
            sQ1_SO3->w = sX1_SO3->w;

            for (unsigned int k = 0; k < X1_dimension_ - 3; k++)
            {
                sQ1_RN->values[k] = sX1_RN->values[k];
            }

            break;
        }
        case SE2RN_SE2:
        {
            ob::SE2StateSpace::StateType *sQ1_SE2 = qQ1->as<ob::CompoundState>()->as<SE2StateSpace::StateType>(0);
            ob::RealVectorStateSpace::StateType *sQ1_RN =
                qQ1->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            const ob::SE2StateSpace::StateType *sQ0 = qQ0->as<SE2StateSpace::StateType>();
            const ob::RealVectorStateSpace::StateType *sX1 = qX1->as<RealVectorStateSpace::StateType>();

            sQ1_SE2->setX(sQ0->getX());
            sQ1_SE2->setY(sQ0->getY());
            sQ1_SE2->setYaw(sQ0->getYaw());

            for (unsigned int k = 0; k < X1_dimension_; k++)
            {
                sQ1_RN->values[k] = sX1->values[k];
            }
            break;
        }
        case SO2RN_SO2:
        {
            ob::SO2StateSpace::StateType *sQ1_SO2 = qQ1->as<ob::CompoundState>()->as<SO2StateSpace::StateType>(0);
            ob::RealVectorStateSpace::StateType *sQ1_RN = qQ1->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            const ob::SO2StateSpace::StateType *sQ0 = qQ0->as<SO2StateSpace::StateType>();
            const ob::RealVectorStateSpace::StateType *sX1 = qX1->as<RealVectorStateSpace::StateType>();

            sQ1_SO2->value = sQ0->value;

            for(unsigned k = 0; k < X1_dimension_; k++){
              sQ1_RN->values[k] = sX1->values[k];
            }
            break;
        }
        case SO2RN_SO2RM:
        {
            ob::SO2StateSpace::StateType *sQ1_SO2 = qQ1->as<ob::CompoundState>()->as<SO2StateSpace::StateType>(0);
            ob::RealVectorStateSpace::StateType *sQ1_RN =
                qQ1->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            const ob::SO2StateSpace::StateType *sQ0_SO2 = qQ0->as<ob::CompoundState>()->as<SO2StateSpace::StateType>(0);
            const ob::RealVectorStateSpace::StateType *sQ0_RM =
                qQ0->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            const ob::RealVectorStateSpace::StateType *sX1 = qX1->as<RealVectorStateSpace::StateType>();

            sQ1_SO2->value = sQ0_SO2->value;

            unsigned int M = Q1_dimension_ - X1_dimension_ - 1;
            unsigned int N = X1_dimension_;

            for (unsigned int k = 0; k < M; k++)
            {
                sQ1_RN->values[k] = sQ0_RM->values[k];
            }
            for (unsigned int k = M; k < M + N; k++)
            {
                sQ1_RN->values[k] = sX1->values[k - M];
            }
            break;
        }

        case SE2RN_R2:
        {
            ob::SE2StateSpace::StateType *sQ1_SE2 = qQ1->as<ob::CompoundState>()->as<SE2StateSpace::StateType>(0);
            ob::RealVectorStateSpace::StateType *sQ1_RN =
                qQ1->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            const ob::RealVectorStateSpace::StateType *sQ0 = qQ0->as<RealVectorStateSpace::StateType>();
            const ob::SO2StateSpace::StateType *sX1_SO2 =
                qX1->as<ob::CompoundState>()->as<ob::SO2StateSpace::StateType>(0);
            const ob::RealVectorStateSpace::StateType *sX1_RN =
                qX1->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            sQ1_SE2->setX(sQ0->values[0]);
            sQ1_SE2->setY(sQ0->values[1]);
            sQ1_SE2->setYaw(sX1_SO2->value);

            for (unsigned int k = 0; k < X1_dimension_ - 1; k++)
            {
                sQ1_RN->values[k] = sX1_RN->values[k];
            }
            break;
        }
        case SE2RN_SE2RM:
        {
            ob::SE2StateSpace::StateType *sQ1_SE2 = qQ1->as<ob::CompoundState>()->as<SE2StateSpace::StateType>(0);
            ob::RealVectorStateSpace::StateType *sQ1_RN =
                qQ1->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            const ob::SE2StateSpace::StateType *sQ0_SE2 = qQ0->as<ob::CompoundState>()->as<SE2StateSpace::StateType>(0);
            const ob::RealVectorStateSpace::StateType *sQ0_RM =
                qQ0->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            const ob::RealVectorStateSpace::StateType *sX1 = qX1->as<RealVectorStateSpace::StateType>();

            sQ1_SE2->setX(sQ0_SE2->getX());
            sQ1_SE2->setY(sQ0_SE2->getY());
            sQ1_SE2->setYaw(sQ0_SE2->getYaw());

            //[X Y YAW] [1...M-1][M...N-1]
            // SE2               RN
            unsigned int M = Q1_dimension_ - X1_dimension_ - 3;
            unsigned int N = X1_dimension_;

            for (unsigned int k = 0; k < M; k++)
            {
                sQ1_RN->values[k] = sQ0_RM->values[k];
            }
            for (unsigned int k = M; k < M + N; k++)
            {
                sQ1_RN->values[k] = sX1->values[k - M];
            }
            break;
        }
        case SE3RN_SE3:
        {
            ob::SE3StateSpace::StateType *sQ1_SE3 = qQ1->as<ob::CompoundState>()->as<SE3StateSpace::StateType>(0);
            ob::SO3StateSpace::StateType *sQ1_SE3_rotation = &sQ1_SE3->rotation();
            ob::RealVectorStateSpace::StateType *sQ1_RN =
                qQ1->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            const ob::SE3StateSpace::StateType *sQ0 = qQ0->as<SE3StateSpace::StateType>();
            const ob::SO3StateSpace::StateType *sQ0_rotation = &sQ0->rotation();
            const ob::RealVectorStateSpace::StateType *sX1 = qX1->as<RealVectorStateSpace::StateType>();

            sQ1_SE3->setXYZ(sQ0->getX(), sQ0->getY(), sQ0->getZ());
            sQ1_SE3_rotation->x = sQ0_rotation->x;
            sQ1_SE3_rotation->y = sQ0_rotation->y;
            sQ1_SE3_rotation->z = sQ0_rotation->z;
            sQ1_SE3_rotation->w = sQ0_rotation->w;

            for (unsigned int k = 0; k < X1_dimension_; k++)
            {
                sQ1_RN->values[k] = sX1->values[k];
            }

            break;
        }
        case SE3RN_SE3RM:
        {
            ob::SE3StateSpace::StateType *sQ1_SE3 = qQ1->as<ob::CompoundState>()->as<SE3StateSpace::StateType>(0);
            ob::SO3StateSpace::StateType *sQ1_SE3_rotation = &sQ1_SE3->rotation();
            ob::RealVectorStateSpace::StateType *sQ1_RN =
                qQ1->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            const ob::SE3StateSpace::StateType *sQ0_SE3 = qQ0->as<ob::CompoundState>()->as<SE3StateSpace::StateType>(0);
            const ob::SO3StateSpace::StateType *sQ0_SE3_rotation = &sQ0_SE3->rotation();
            const ob::RealVectorStateSpace::StateType *sQ0_RM =
                qQ0->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            const ob::RealVectorStateSpace::StateType *sX1 = qX1->as<RealVectorStateSpace::StateType>();

            sQ1_SE3->setXYZ(sQ0_SE3->getX(), sQ0_SE3->getY(), sQ0_SE3->getZ());
            sQ1_SE3_rotation->x = sQ0_SE3_rotation->x;
            sQ1_SE3_rotation->y = sQ0_SE3_rotation->y;
            sQ1_SE3_rotation->z = sQ0_SE3_rotation->z;
            sQ1_SE3_rotation->w = sQ0_SE3_rotation->w;

            //[X Y Z YAW PITCH ROLL] [1...M-1][M...N-1]
            // SE3                                        RN
            unsigned int M = Q1_dimension_ - X1_dimension_ - 6;
            unsigned int N = X1_dimension_;

            for (unsigned int k = 0; k < M; k++)
            {
                sQ1_RN->values[k] = sQ0_RM->values[k];
            }
            for (unsigned int k = M; k < M + N; k++)
            {
                sQ1_RN->values[k] = sX1->values[k - M];
            }
            break;
        }
        default:
        {
            OMPL_ERROR("Cannot merge states");
            OMPL_ERROR("Type %d not implemented.", type_);
            exit(0);
        }
    }
}
void QuotientSpace::projectX1Subspace(const ob::State *q, ob::State *qX1) const
{
    switch (type_)
    {
        case RN_RM:
        {
            const ob::RealVectorStateSpace::StateType *sQ1 = q->as<RealVectorStateSpace::StateType>();
            ob::RealVectorStateSpace::StateType *sX1 = qX1->as<RealVectorStateSpace::StateType>();

            for (unsigned int k = Q0_dimension_; k < Q1_dimension_; k++)
            {
                sX1->values[k - Q0_dimension_] = sQ1->values[k];
            }
            break;
        }
        case SE2_R2:
        {
            const ob::SE2StateSpace::StateType *sQ1 = q->as<SE2StateSpace::StateType>();
            ob::SO2StateSpace::StateType *sX1 = qX1->as<SO2StateSpace::StateType>();
            sX1->value = sQ1->getYaw();
            break;
        }
        case SE3_R3:
        {
            const ob::SE3StateSpace::StateType *sQ1 = q->as<SE3StateSpace::StateType>();
            const ob::SO3StateSpace::StateType *sQ1_SO3 = &sQ1->rotation();

            ob::SO3StateSpace::StateType *sX1_SO3 = qX1->as<SO3StateSpace::StateType>();

            sX1_SO3->x = sQ1_SO3->x;
            sX1_SO3->y = sQ1_SO3->y;
            sX1_SO3->z = sQ1_SO3->z;
            sX1_SO3->w = sQ1_SO3->w;

            break;
        }
        case SE3RN_R3:
        {
            const ob::SE3StateSpace::StateType *sQ1_SE3 = q->as<ob::CompoundState>()->as<SE3StateSpace::StateType>(0);
            const ob::SO3StateSpace::StateType *sQ1_SO3 = &sQ1_SE3->rotation();
            const ob::RealVectorStateSpace::StateType *sQ1_RN =
                q->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            ob::SO3StateSpace::StateType *sX1_SO3 = qX1->as<ob::CompoundState>()->as<SO3StateSpace::StateType>(0);
            ob::RealVectorStateSpace::StateType *sX1_RN =
                qX1->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            sX1_SO3->x = sQ1_SO3->x;
            sX1_SO3->y = sQ1_SO3->y;
            sX1_SO3->z = sQ1_SO3->z;
            sX1_SO3->w = sQ1_SO3->w;
            for (unsigned int k = 0; k < X1_dimension_ - 3; k++)
            {
                sX1_RN->values[k] = sQ1_RN->values[k];
            }

            break;
        }
        case SE2RN_R2:
        {
            const ob::SE2StateSpace::StateType *sQ1_SE2 = q->as<ob::CompoundState>()->as<SE2StateSpace::StateType>(0);
            const ob::RealVectorStateSpace::StateType *sQ1_RN =
                q->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            ob::SO2StateSpace::StateType *sX1_SO2 = qX1->as<ob::CompoundState>()->as<ob::SO2StateSpace::StateType>(0);
            ob::RealVectorStateSpace::StateType *sX1_RN =
                qX1->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            sX1_SO2->value = sQ1_SE2->getYaw();
            for (unsigned int k = 0; k < X1_dimension_ - 1; k++)
            {
                sX1_RN->values[k] = sQ1_RN->values[k];
            }
            break;
        }
        case SE2RN_SE2RM:
        case SO2RN_SO2RM:
        {
            const ob::RealVectorStateSpace::StateType *sQ1_RN =
                q->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            const ob::RealVectorStateSpace::StateType *sX1 = qX1->as<RealVectorStateSpace::StateType>();

            unsigned int N = Q1_dimension_ - X1_dimension_ - 3;
            for (unsigned int k = N; k < Q1_dimension_ - 3; k++)
            {
                sX1->values[k - N] = sQ1_RN->values[k];
            }
            break;
        }
        case SE2RN_SE2:
        case SE3RN_SE3:
        case SO2RN_SO2:
        {
            const ob::RealVectorStateSpace::StateType *sQ1_RN =
                q->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);
            ob::RealVectorStateSpace::StateType *sX1 = qX1->as<RealVectorStateSpace::StateType>();

            for (unsigned int k = 0; k < X1_dimension_; k++)
            {
                sX1->values[k] = sQ1_RN->values[k];
            }

            break;
        }
        case SE3RN_SE3RM:
        {
            const ob::RealVectorStateSpace::StateType *sQ1_RN =
                q->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            const ob::RealVectorStateSpace::StateType *sX1 = qX1->as<RealVectorStateSpace::StateType>();

            unsigned int N = Q1_dimension_ - X1_dimension_ - 6;
            for (unsigned int k = N; k < Q1_dimension_ - 6; k++)
            {
                sX1->values[k - N] = sQ1_RN->values[k];
            }
            break;
        }
        default:
        {
            OMPL_ERROR("Cannot project onto X1.");
            OMPL_ERROR("Type %d not implemented.", type_);
            exit(0);
        }
    }
}

void QuotientSpace::projectQ0Subspace(const ob::State *q, ob::State *qQ0) const
{
    switch (type_)
    {
        case IDENTITY_SPACE_RN:
        case IDENTITY_SPACE_SE2:
        case IDENTITY_SPACE_SE2RN:
        case IDENTITY_SPACE_SO2RN:
        case IDENTITY_SPACE_SE3:
        case IDENTITY_SPACE_SE3RN:
        {
            // Identity function
            Q1->getStateSpace()->copyState(qQ0, q);
            break;
        }
        case RN_RM:
        {
            const ob::RealVectorStateSpace::StateType *sQ1 = q->as<RealVectorStateSpace::StateType>();
            ob::RealVectorStateSpace::StateType *sQ0 = qQ0->as<RealVectorStateSpace::StateType>();

            for (unsigned int k = 0; k < Q0_dimension_; k++)
            {
                sQ0->values[k] = sQ1->values[k];
            }
            break;
        }
        case SE2_R2:
        {
            const ob::SE2StateSpace::StateType *sQ1 = q->as<SE2StateSpace::StateType>();
            ob::RealVectorStateSpace::StateType *sQ0 = qQ0->as<RealVectorStateSpace::StateType>();
            sQ0->values[0] = sQ1->getX();
            sQ0->values[1] = sQ1->getY();
            break;
        }
        case SE2RN_R2:
        {
            const ob::SE2StateSpace::StateType *sQ1 = q->as<ob::CompoundState>()->as<SE2StateSpace::StateType>(0);
            ob::RealVectorStateSpace::StateType *sQ0 = qQ0->as<RealVectorStateSpace::StateType>();
            sQ0->values[0] = sQ1->getX();
            sQ0->values[1] = sQ1->getY();
            break;
        }
        case SE2RN_SE2:
        {
            const ob::SE2StateSpace::StateType *sQ1_SE2 = q->as<ob::CompoundState>()->as<SE2StateSpace::StateType>(0);
            ob::SE2StateSpace::StateType *sQ0_SE2 = qQ0->as<SE2StateSpace::StateType>();

            sQ0_SE2->setX(sQ1_SE2->getX());
            sQ0_SE2->setY(sQ1_SE2->getY());
            sQ0_SE2->setYaw(sQ1_SE2->getYaw());

            break;
        }
        case SO2RN_SO2:
        {
          const ob::SO2StateSpace::StateType *sQ1_SO2 = q->as<ob::CompoundState>()->as<SO2StateSpace::StateType>(0);
          ob::SO2StateSpace::StateType *sQ0_SO2 = qQ0->as<SO2StateSpace::StateType>();

          sQ0_SO2->value = sQ1_SO2->value;

          break;
        }
        case SO2RN_SO2RM:
        {
            const ob::SO2StateSpace::StateType *sQ1_SO2 = q->as<ob::CompoundState>()->as<SO2StateSpace::StateType>(0);
            const ob::RealVectorStateSpace::StateType *sQ1_RN =
                q->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            ob::SO2StateSpace::StateType *sQ0_SO2 = qQ0->as<ob::CompoundState>()->as<SO2StateSpace::StateType>(0);
            ob::RealVectorStateSpace::StateType *sQ0_RM =
                qQ0->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            sQ0_SO2->value = sQ1_SO2->value;

            for (unsigned int k = 0; k < Q0_dimension_ - 1; k++)
            {
                sQ0_RM->values[k] = sQ1_RN->values[k];
            }
            break;
        }

        case SE2RN_SE2RM:
        {
            const ob::SE2StateSpace::StateType *sQ1_SE2 = q->as<ob::CompoundState>()->as<SE2StateSpace::StateType>(0);
            const ob::RealVectorStateSpace::StateType *sQ1_RN =
                q->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            ob::SE2StateSpace::StateType *sQ0_SE2 = qQ0->as<ob::CompoundState>()->as<SE2StateSpace::StateType>(0);
            ob::RealVectorStateSpace::StateType *sQ0_RN =
                qQ0->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            sQ0_SE2->setX(sQ1_SE2->getX());
            sQ0_SE2->setY(sQ1_SE2->getY());
            sQ0_SE2->setYaw(sQ1_SE2->getYaw());

            for (unsigned int k = 0; k < Q0_dimension_ - 3; k++)
            {
                sQ0_RN->values[k] = sQ1_RN->values[k];
            }
            break;
        }
        case SE3_R3:
        {
            const ob::SE3StateSpace::StateType *sQ1 = q->as<SE3StateSpace::StateType>();
            ob::RealVectorStateSpace::StateType *sQ0 = qQ0->as<RealVectorStateSpace::StateType>();

            sQ0->values[0] = sQ1->getX();
            sQ0->values[1] = sQ1->getY();
            sQ0->values[2] = sQ1->getZ();

            break;
        }
        case SE3RN_R3:
        {
            const ob::SE3StateSpace::StateType *sQ1_SE3 = q->as<ob::CompoundState>()->as<SE3StateSpace::StateType>(0);
            ob::RealVectorStateSpace::StateType *sQ0 = qQ0->as<RealVectorStateSpace::StateType>();

            sQ0->values[0] = sQ1_SE3->getX();
            sQ0->values[1] = sQ1_SE3->getY();
            sQ0->values[2] = sQ1_SE3->getZ();

            break;
        }
        case SE3RN_SE3:
        {
            const ob::SE3StateSpace::StateType *sQ1_SE3 = q->as<ob::CompoundState>()->as<SE3StateSpace::StateType>(0);
            const ob::SO3StateSpace::StateType *sQ1_SE3_rotation = &sQ1_SE3->rotation();

            ob::SE3StateSpace::StateType *sQ0 = qQ0->as<SE3StateSpace::StateType>();
            ob::SO3StateSpace::StateType *sQ0_rotation = &sQ0->rotation();

            sQ0->setXYZ(sQ1_SE3->getX(), sQ1_SE3->getY(), sQ1_SE3->getZ());
            sQ0_rotation->x = sQ1_SE3_rotation->x;
            sQ0_rotation->y = sQ1_SE3_rotation->y;
            sQ0_rotation->z = sQ1_SE3_rotation->z;
            sQ0_rotation->w = sQ1_SE3_rotation->w;

            break;
        }
        case SE3RN_SE3RM:
        {
            const ob::SE3StateSpace::StateType *sQ1_SE3 = q->as<ob::CompoundState>()->as<SE3StateSpace::StateType>(0);
            const ob::SO3StateSpace::StateType *sQ1_SE3_rotation = &sQ1_SE3->rotation();
            const ob::RealVectorStateSpace::StateType *sQ1_RN =
                q->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            ob::SE3StateSpace::StateType *sQ0_SE3 = qQ0->as<ob::CompoundState>()->as<SE3StateSpace::StateType>(0);
            ob::SO3StateSpace::StateType *sQ0_rotation = &sQ0_SE3->rotation();
            ob::RealVectorStateSpace::StateType *sQ0_RN =
                qQ0->as<ob::CompoundState>()->as<RealVectorStateSpace::StateType>(1);

            sQ0_SE3->setXYZ(sQ1_SE3->getX(), sQ1_SE3->getY(), sQ1_SE3->getZ());
            sQ0_rotation->x = sQ1_SE3_rotation->x;
            sQ0_rotation->y = sQ1_SE3_rotation->y;
            sQ0_rotation->z = sQ1_SE3_rotation->z;
            sQ0_rotation->w = sQ1_SE3_rotation->w;

            for (unsigned int k = 0; k < Q0_dimension_ - 6; k++)
            {
                sQ0_RN->values[k] = sQ1_RN->values[k];
            }
            break;
        }
        default:
        {
            OMPL_ERROR("Cannot project onto Q0.");
            OMPL_ERROR("Type %d not implemented.", type_);
            exit(1);
        }
    }
}

const ob::SpaceInformationPtr &QuotientSpace::getX1() const
{
    return X1;
}
const ob::SpaceInformationPtr &QuotientSpace::getQ1() const
{
    return Q1;
}
const ob::SpaceInformationPtr &QuotientSpace::getQ0() const
{
    return Q0;
}
unsigned int QuotientSpace::getX1Dimension() const
{
    return X1_dimension_;
}
unsigned int QuotientSpace::getQ1Dimension() const
{
    return Q1->getStateDimension();
}
unsigned int QuotientSpace::getDimension() const
{
    return getQ1Dimension();
}
unsigned int QuotientSpace::getQ0Dimension() const
{
    return Q0_dimension_;
}
const ob::StateSamplerPtr &QuotientSpace::getX1SamplerPtr() const
{
    return X1_sampler_;
}
const ob::StateSamplerPtr &QuotientSpace::getQ1SamplerPtr() const
{
    return Q1_sampler_;
}

bool QuotientSpace::hasSolution()
{
    if (!hasSolution_)
    {
        ob::PathPtr path;
        hasSolution_ = getSolution(path);
    }
    return hasSolution_;
}
unsigned int QuotientSpace::getTotalNumberOfSamples() const
{
    return totalNumberOfSamples_;
}
unsigned int QuotientSpace::getTotalNumberOfFeasibleSamples() const
{
    return totalNumberOfFeasibleSamples_;
}
QuotientSpace *QuotientSpace::getParent() const
{
    return parent_;
}
QuotientSpace *QuotientSpace::getChild() const
{
    return child_;
}
void QuotientSpace::setChild(QuotientSpace *child)
{
    child_ = child;
}
void QuotientSpace::setParent(QuotientSpace *parent)
{
    parent_ = parent;
}
unsigned int QuotientSpace::getLevel() const
{
    return level_;
}
void QuotientSpace::setLevel(unsigned int level)
{
    level_ = level;
}
QuotientSpace::QuotientSpaceType QuotientSpace::getType() const
{
    return type_;
}
ob::OptimizationObjectivePtr QuotientSpace::getOptimizationObjectivePtr() const
{
    return opt_;
}

bool QuotientSpace::sampleQuotient(ob::State *q_random)
{
    Q1_sampler_->sampleUniform(q_random);
    return true;
}

bool QuotientSpace::sample(ob::State *q_random)
{
    bool valid = false;
    if (parent_ == nullptr)
    {
        // return Q1_valid_sampler->sample(q_random);
        Q1_sampler_->sampleUniform(q_random);
        valid = Q1->isValid(q_random);
    }
    else
    {
        if (X1_dimension_ > 0)
        {
            // Adjusted sampling function: Sampling in G0 x X1
            X1_sampler_->sampleUniform(s_X1_tmp_);
            parent_->sampleQuotient(s_Q0_tmp_);
            mergeStates(s_Q0_tmp_, s_X1_tmp_, q_random);
        }
        else
        {
            parent_->sampleQuotient(q_random);
        }
        valid = Q1->isValid(q_random);
    }
    totalNumberOfSamples_++;
    if (valid)
    {
        totalNumberOfFeasibleSamples_++;
    }

    return valid;
}

double QuotientSpace::getImportance() const
{
    double N = (double)totalNumberOfSamples_;
    return 1.0 / (N + 1);
}

void QuotientSpace::print(std::ostream &out) const
{
    out << "[QuotientSpace: id" << id_ << " |lvl" << level_ << "] ";
    unsigned int sublevel = std::max(1U, level_);
    if (parent_ == nullptr)
    {
        out << "X" << sublevel << "=Q" << sublevel << ": ";
        if (Q1->getStateSpace()->getType() == ob::STATE_SPACE_SE2)
        {
            out << "SE(2)";
        }
        else if (Q1->getStateSpace()->getType() == ob::STATE_SPACE_SE3)
        {
            out << "SE(3)";
        }
        else if (Q1->getStateSpace()->getType() == ob::STATE_SPACE_REAL_VECTOR)
        {
            out << "R^" << Q1->getStateDimension();
        }
        else
        {
            out << "unknown";
        }
    }
    else
    {
        out << "X" << sublevel << "=Q" << sublevel << ": ";
        switch (type_)
        {
            case QuotientSpace::IDENTITY_SPACE_RN:
            {
                out << "R^" << Q0_dimension_ 
                  << " | Q" << level_ + 1 
                  << ": R^" << Q1_dimension_;
                break;
            }
            case QuotientSpace::IDENTITY_SPACE_SE2:
            {
                out << "SE(2)" << " | Q" << level_ + 1 << ": SE(2)";
                break;
            }
            case QuotientSpace::IDENTITY_SPACE_SE2RN:
            {
                out << "SE(2)xR^" << Q0_dimension_ 
                  << " | Q" << level_ + 1 
                  << ": SE(2)xR^" << Q1_dimension_;
                break;
            }
            case QuotientSpace::IDENTITY_SPACE_SO2RN:
            {
                out << "SO(2)xR^" << Q0_dimension_ 
                  << " | Q" << level_ + 1 
                  << ": SO(2)xR^" << Q1_dimension_;
                break;
            }
            case QuotientSpace::IDENTITY_SPACE_SE3:
            {
                out << "SE(3)" << " | Q" << level_ + 1 << ": SE(3)";
                break;
            }
            case QuotientSpace::IDENTITY_SPACE_SE3RN:
            {
                out << "SE(3)xR^" << Q0_dimension_ 
                  << " | Q" << level_ + 1 
                  << ": SE(3)xR^" << Q1_dimension_;
                break;
            }
            case QuotientSpace::RN_RM:
            {
                out << "R^" << Q0_dimension_ << " | Q" << level_ + 1 << ": R^" << Q1_dimension_ << " | X" << level_ + 1
                    << ": R^" << Q1_dimension_ - Q0_dimension_;
                break;
            }
            case QuotientSpace::SE2_R2:
            {
                out << "R^2 | Q" << level_ + 1 << ": SE(2) | X" << level_ + 1 << ": SO(2)";
                break;
            }
            case QuotientSpace::SE3_R3:
            {
                out << "R^3 | Q" << level_ + 1 << ": SE(3) | X" << level_ + 1 << ": SO(3)";
                break;
            }
            case QuotientSpace::SE2RN_SE2:
            {
                out << "SE(2) | Q" << level_ + 1 << ": SE(2)xR^" << X1_dimension_ << " | X" << level_ + 1 << ": R^"
                    << X1_dimension_;
                break;
            }
            case QuotientSpace::SO2RN_SO2:
            {
                out << "SO(2) | Q" << level_ + 1 << ": SO(2)xR^" << X1_dimension_ << " | X" << level_+1 << ": R^" << X1_dimension_;
                break;
            }
            case QuotientSpace::SE3RN_SE3:
            {
                out << "SE(3) | Q" << level_ + 1 << ": SE(3)xR^" << X1_dimension_ << " | X" << level_ + 1 << ": R^"
                    << X1_dimension_;
                break;
            }
            case QuotientSpace::SE2RN_SE2RM:
            {
                out << "SE(2)xR^" << Q0_dimension_ - 3 << " | Q" << level_ + 1 << ": SE(2)xR^" << Q1_dimension_ - 3
                    << " | X" << level_ + 1 << ": R^" << X1_dimension_;
                break;
            }
            case QuotientSpace::SO2RN_SO2RM:
            {
                out << "SO(2)xR^" << Q0_dimension_ - 1 << " | Q" << level_ + 1 << ": SO(2)xR^" << Q1_dimension_ - 1
                    << " | X" << level_ + 1 << ": R^" << X1_dimension_;
                break;
            }
            case QuotientSpace::SE3RN_SE3RM:
            {
                out << "SE(3)xR^" << Q0_dimension_ - 6 << " | Q" << level_ + 1 << ": SE(3)xR^" << Q1_dimension_ - 6
                    << " | X" << level_ + 1 << ": R^" << X1_dimension_;
                break;
            }
            default:
            {
                out << "unknown type_: " << type_;
            }
        }
    }
    out << " [Importance:" << getImportance() << "]";
}

namespace ompl
{
    namespace geometric
    {
        std::ostream &operator<<(std::ostream &out, const QuotientSpace &quotient_)
        {
            quotient_.print(out);
            return out;
        }
    }
}
