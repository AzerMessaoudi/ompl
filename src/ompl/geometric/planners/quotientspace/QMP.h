/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2020, University of Stuttgart
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

/* Author: Andreas Orthey, Sohaib Akbar */

#ifndef OMPL_GEOMETRIC_PLANNERS_QUOTIENTSPACE_QMP_
#define OMPL_GEOMETRIC_PLANNERS_QUOTIENTSPACE_QMP_
#include <ompl/geometric/planners/quotientspace/datastructures/BundleSpaceSequence.h>
#include <ompl/geometric/planners/quotientspace/algorithms/QMPImpl.h>

namespace ompl
{
    namespace geometric
    {
        /**
             @anchor QMP
             @par Short description
             A motion planning algorithm computes the motion
             of a robot by computing a path through its configuration space.
             To improve the runtime of motion planning algorithms, we
             propose to nest robots in each other, creating a nested quotient-
             space decomposition of the configuration space. Based on this
             decomposition we define a new roadmap-based motion planning
             algorithm called the Quotient-space roadMap Planner (QMP).
             The algorithm starts growing a graph on the lowest dimensional
             quotient space, switches to the next quotient space once a
             valid path has been found, and keeps updating the graphs
             on each quotient space simultaneously until a valid path in
             the configuration space has been found. We show that this
             algorithm is probabilistically complete and outperforms a set
             of state-of-the-art algorithms implemented in the open motion
             planning library (OMPL).
             @par External documentation
             A. Orthey, A. Escande and E. Yoshida,
             Quotient-Space Motion Planning,
             in <em>International Conference on Intelligent Robots and Systems</em>, 2018,
             [[PDF]](https://arxiv.org/abs/1807.09468)
        */

        /** \brief Quotient-space roadMap Planner (QMP) Algorithm */
        typedef ompl::geometric::BundleSpaceSequence<ompl::geometric::QMPImpl> QMP;

    }  // namespace geometric
}  // namespace ompl

#endif
