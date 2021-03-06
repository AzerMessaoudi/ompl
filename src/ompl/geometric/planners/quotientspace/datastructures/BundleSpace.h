#ifndef OMPL_GEOMETRIC_PLANNERS_BUNDLESPACE_BUNDLE_
#define OMPL_GEOMETRIC_PLANNERS_BUNDLESPACE_BUNDLE_

#include <ompl/base/Planner.h>
#include "BundleSpaceComponent.h"
#include "BundleSpaceComponentFactory.h"

namespace ompl
{
  //New Notation:
  //
  // Configuration x;
  // Components M
  // Dimension N
  // BundleSpaces Fiber, Bundle, Base;
  //
    namespace geometric
    {
        OMPL_CLASS_FORWARD(BundleSpaceComponent);
    }
    namespace geometric
    {
        /// \brief A single Bundle-space
        class BundleSpace : public ompl::base::Planner
        {
            using BaseT = ompl::base::Planner;

        public:
            /**  \brief Bundle Space contains three OMPL spaces, which we call Bundle, Base and Fiber.

                 Bundle = Base x Fiber is a product space of Base and Fiber and
                      is the main Bundle-space of this class
                 Base is a pointer to the next lower-dimensional Bundle-space and
                 Fiber is the Bundle-space  Bundle / Base

                 We assume that Bundle and Base have been given (as ompl::base::SpaceInformationPtr),
                 and we compute the inverse of the Bundle map, i.e. Fiber = Bundle/Base. */

            BundleSpace(const ompl::base::SpaceInformationPtr &si, BundleSpace *parent_ = nullptr);
            ~BundleSpace();

            /// \brief solve disabled (use MultiBundle::solve)
            /// final prevents subclasses to override
            ompl::base::PlannerStatus solve(const ompl::base::PlannerTerminationCondition &ptc) override final;
            virtual void setProblemDefinition(const ompl::base::ProblemDefinitionPtr &pdef) override;

            virtual void grow() = 0;
            virtual bool getSolution(ompl::base::PathPtr &solution) = 0;

            virtual void sampleFromDatastructure(ompl::base::State *xBase) = 0;
            virtual void sampleFiber(ompl::base::State *xFiber);
            virtual void sampleBundle(ompl::base::State *xRandom);

            virtual bool hasSolution();
            virtual void clear() override;
            virtual void setup() override;

            virtual double getImportance() const = 0;

            /// \brief Allocate State, set entries to Identity/Zero
            ompl::base::State *allocIdentityStateFiber() const;
            ompl::base::State *allocIdentityStateBundle() const;
            ompl::base::State *allocIdentityStateBase() const;
            ompl::base::State *allocIdentityState(ompl::base::StateSpacePtr) const;
            void allocIdentityState(ompl::base::State*, ompl::base::StateSpacePtr) const;

            /// \brief Print Information pertaining to why a state failed being
            /// valid
            void debugInvalidState(const ompl::base::State*);

            /// \brief reset counter for number of levels
            static void resetCounter();

            /// \brief Get SpaceInformationPtr for Fiber
            const ompl::base::SpaceInformationPtr &getFiber() const;
            /// \brief Get SpaceInformationPtr for Bundle
            const ompl::base::SpaceInformationPtr &getBundle() const;
            /// \brief Get SpaceInformationPtr for Base
            const ompl::base::SpaceInformationPtr &getBase() const;

            /// \brief Dimension of Fiber Space
            unsigned int getFiberDimension() const;
            /// \brief Dimension of Base Space
            unsigned int getBaseDimension() const;
            /// \brief Dimension of Bundle Space
            unsigned int getBundleDimension() const;

            const ompl::base::StateSamplerPtr &getFiberSamplerPtr() const;
            const ompl::base::StateSamplerPtr &getBundleSamplerPtr() const;

            /// \brief Parent is a more simplified Bundle-space
            /// (higher in abstraction hierarchy)
            BundleSpace *getParent() const;
            /// \brief Child is a less simplified Bundle-space
            /// (lower in abstraction hierarchy)
            BundleSpace *getChild() const;

            bool hasParent() const;
            bool hasChild() const;

            /// Level in abstraction hierarchy of Bundle-spaces
            unsigned int getLevel() const;
            /// Change abstraction level
            void setLevel(unsigned int);
            /// Type of Bundle-space
            // BundleSubspaceType getType() const;
            /// Set pointer to less simplified Bundle-space
            void setChild(BundleSpace *child_);
            /// Set pointer to more simplified Bundle-space
            void setParent(BundleSpace *parent_);

            /// \brief Bundle Space Projection Operator onto second component
            /// ProjectFiber: Base \times Fiber \rightarrow Fiber
            void projectFiber(
                const ompl::base::State *xBundle, 
                ompl::base::State *xFiber) const;

            /// \brief Bundle Space Projection Operator onto first component
            /// ProjectBase: Base \times Fiber \rightarrow Base
            void projectBase(
                const ompl::base::State *xBundle, 
                ompl::base::State *xBase) const;

            /// Merge a state from Base and Fiber into a state on Bundle (concatenate)
            void mergeStates(
                const ompl::base::State *xBase, 
                const ompl::base::State *xFiber, 
                ompl::base::State *xBundle) const; 

            ompl::base::OptimizationObjectivePtr getOptimizationObjectivePtr() const;

            /// \brief Write class to stream (use as std::cout << *this << std::endl)
            ///  Actual implementation is in void print(std::ostream& out),
            ///  which can be inherited.
            friend std::ostream &operator<<(std::ostream&, const BundleSpace&);

            bool isDynamic() const;

        protected:
            /// Check if Bundle-space is unbounded
            void checkBundleSpaceMeasure(std::string name, 
                const ompl::base::StateSpacePtr space) const;
            void checkBundleSpace() const;
            void MakeFiberSpace();

            std::vector<BundleSpaceComponentPtr> components_;

            /// Internal function implementing actual printing to stream
            virtual void print(std::ostream &out) const;

            ompl::base::SpaceInformationPtr Bundle{nullptr};
            ompl::base::SpaceInformationPtr Base{nullptr};
            ompl::base::SpaceInformationPtr Fiber{nullptr};

            ompl::base::StateSamplerPtr Fiber_sampler_;
            ompl::base::StateSamplerPtr Bundle_sampler_;
            ompl::base::ValidStateSamplerPtr Bundle_valid_sampler_;

            ompl::base::OptimizationObjectivePtr opt_;

            /// A temporary state on Base
            ompl::base::State *xBaseTmp_{nullptr};
            /// A temporary state on Fiber
            ompl::base::State *xFiberTmp_{nullptr};

            static unsigned int counter_;

            /// Identity of space (to keep track of number of Bundle-spaces created)
            unsigned int id_{0};

            /// Level in sequence of Bundle-spaces
            unsigned int level_{0};

            bool hasSolution_{false};
            bool firstRun_{true};

            bool isDynamic_{false};

            BundleSpace *parent_{nullptr};
            BundleSpace *child_{nullptr};

            /** \brief Goal state or goal region */
            ompl::base::Goal *goal_;

            BundleSpaceComponentFactory componentFactory;
        };
    }  // namespace geometric
}  // namespace ompl
#endif
