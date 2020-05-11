#ifndef OMPL_GEOMETRIC_PLANNERS_BUNDLESPACE_PROPAGATORS_DYNAMIC__
#define OMPL_GEOMETRIC_PLANNERS_BUNDLESPACE_PROPAGATORS_DYNAMIC__

#include <ompl/geometric/planners/multilevel/datastructures/propagators/BundleSpacePropagator.h>
#include <ompl/control/Control.h>
#include <ompl/control/SpaceInformation.h>
#include <ompl/control/StatePropagator.h>
#include <ompl/control/DirectedControlSampler.h>
#include <ompl/control/ControlSampler.h>
#include <ompl/util/RandomNumbers.h>

namespace ompl
{
  namespace control
  {
    OMPL_CLASS_FORWARD(DirectedControlSampler);
  }

  namespace geometric
  {

    class BundleSpacePropagatorDynamic: public BundleSpacePropagator
    {
        using BaseT = BundleSpacePropagator;

      public:

        BundleSpacePropagatorDynamic() = delete;
        BundleSpacePropagatorDynamic(BundleSpaceGraph*); 

        virtual ~BundleSpacePropagatorDynamic() override;

        virtual bool steer( 
            const Configuration *from, 
            const Configuration *to, 
            Configuration *result) override;

        const control::Control* getLastControl();

      protected:
        RNG rng_;
        int numberOfControlSamples{10};
        double propStepSize;
        int controlDuration{2};

        control::SpaceInformation *siC_{nullptr};

        control::Control* controlRandom_{nullptr};

        base::State* stateRandom_;

        control::StatePropagatorPtr prop_;

        // control::SimpleDirectedControlSamplerPtr controlSampler_;

        control::DirectedControlSamplerPtr controlSampler_;



    };
  }
}


#endif