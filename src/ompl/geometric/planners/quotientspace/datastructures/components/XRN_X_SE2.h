#ifndef OMPL_GEOMETRIC_PLANNERS_BUNDLESPACE_BUNDLE_COMPONENT_SE2RN_SE2__
#define OMPL_GEOMETRIC_PLANNERS_BUNDLESPACE_BUNDLE_COMPONENT_SE2RN_SE2__
#include <ompl/geometric/planners/quotientspace/datastructures/components/XRN_X.h>

namespace ompl
{
    namespace geometric
    {
        class BundleSpaceComponent_SE2RN_SE2: public BundleSpaceComponent_XRN_X
        {
            using BaseT = BundleSpaceComponent_XRN_X;
          public:
            BundleSpaceComponent_SE2RN_SE2(
                base::StateSpacePtr BundleSpace,
                base::StateSpacePtr BaseSpace);

            ~BundleSpaceComponent_SE2RN_SE2() override = default;

            virtual void projectBase(
                const ompl::base::State *xBundle,
                ompl::base::State *xBase) const override;

            virtual void mergeStates(
                const ompl::base::State *xBase, 
                const ompl::base::State *xFiber, 
                ompl::base::State *xBundle) const override;


        };
    }
}

#endif

