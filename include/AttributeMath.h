#ifndef ATTRIBUTEMATH__H
#define ATTRIBUTEMATH__H

#include <FnGeolib/op/FnGeolibOp.h>
#include <FnGeolibServices/FnGeolibCookInterfaceUtilsService.h>
#include <math.h>

namespace TC_AttributeUtils {

  class AttributeMath : public Foundry::Katana::GeolibOp
  {
    public:

      enum Operation {
        Add, Subtract, Multiply, Divide, Power, Exponent, Min, Max, Mod, Ceil,
        Floor, Round, Abs, ToExposure, ToIntensity
      };

      static void flush() {};
      static void setup(FnKat::GeolibSetupInterface &interface);
      static void cook(FnKat::GeolibCookInterface &interface);

    private:

      static void handleOperation(
          FnAttribute::GroupAttribute &spec,
          FnKat::GeolibCookInterface &interface);


      template <class A>
      static A handleAttr(
          A& srcAttr,
          FnAttribute::GroupAttribute &spec,
          FnKat::GeolibCookInterface &interface)
      {

        if(srcAttr.getNumberOfValues() == 0) { return FnAttribute::NullAttribute(); }

        A modifierAttr;
        FnAttribute::Attribute modifierRawAttr = spec.getChildByName("modifier");

        // First, we need to check if the modifier is a string, in which case
        // it points to another attribute to fetch,  otherwise, its a value
        FnAttribute::StringAttribute modifierStrAttr = modifierRawAttr;
        if(modifierStrAttr.isValid()){

          const std::string modifierAttrName = modifierStrAttr.getValue();
          if(modifierAttrName.empty()) {
            interface.setAttr("error", FnAttribute::StringAttribute(
                  "No modifier attribute name specified"));
            interface.setAttr("type", FnAttribute::StringAttribute("error"));
            interface.stopChildTraversal();
            return FnAttribute::NullAttribute();
          }

          modifierAttr = FnKat::GetGlobalAttr(interface, modifierAttrName);
          if(!modifierAttr.isValid()){
            FnAttribute::GroupAttribute defaults =
            FnGeolibServices::FnGeolibCookInterfaceUtils::cookDaps(interface, "");
            modifierAttr = defaults.getChildByName(modifierAttrName);
          }

        } else {
          modifierAttr = modifierRawAttr;
        }

        if(!modifierAttr.isValid() || modifierAttr.getNumberOfValues() == 0) {
          interface.setAttr("error", FnAttribute::StringAttribute(
                "Modifier attribute is not valid"));
          interface.setAttr("type", FnAttribute::StringAttribute("error"));
          return FnAttribute::NullAttribute();
        }

        // Cool, so now we should have a src, and modifier of the same type
        FnAttribute::IntAttribute modeAttr = spec.getChildByName("operation");
        const Operation mode = (Operation)modeAttr.getValue(0, false);

        // If we have the same number, we process them component wise,
        // otherwise we take the first one and use that
        const int64_t numSrcValues = srcAttr.getNumberOfValues();
        const int64_t numSrcSamples = srcAttr.getNumberOfTimeSamples();
        const int64_t numModifiersValues = modifierAttr.getNumberOfValues();

        // We already checked that we have at least 1 value
        typename A::value_type constModifier = modifierAttr.getValue();
        const typename A::value_type* modifierVals = modifierAttr.getNearestSample(0.0).data();
        const bool useConsntantModifier = (numModifiersValues != numSrcValues);

        std::vector<float> sampleTimes(numSrcSamples);
        std::vector<class A::value_type> newVals;
        newVals.reserve(numSrcValues * numSrcSamples);

        for(int64_t s=0; s<numSrcSamples; ++s) {

          const float sampleTime = srcAttr.getSampleTime(s);
          sampleTimes[s] = sampleTime;

          const typename A::value_type* srcVals = srcAttr.getNearestSample(sampleTime).data();
          for(int64_t i=0; i<numSrcValues; ++i) {

            newVals.push_back(doMath(
              srcVals[i],
              useConsntantModifier ? constModifier : modifierVals[i],
              mode));
          }
        }

        const float* sampleTimesData = sampleTimes.data();
        const typename A::value_type* valsData = newVals.data();

        return A(
          sampleTimesData, sampleTimes.size(),
          &valsData, newVals.size(),
          srcAttr.getTupleSize());
      }
      

      template <typename T>
      static T doMath(T value, T modifier, Operation mode)
      {
        switch( mode ) {

          case Add:
            return value + modifier;

          case Subtract:
            return value - modifier;

          case Multiply:
            return value * modifier;

          case Divide:
            return (modifier != 0.0) ? value/modifier : INFINITY;

          case Power:
            return pow(value, modifier);

          case Exponent:
            return pow(modifier, value);

          case Min:
            return std::min(value, modifier);

          case Max:
            return std::max(value, modifier);

          case Mod:
            return fmod(value, modifier);

          case Floor:
            return floor(value);

          case Ceil:
            return ceil(value);

          case Round:
            return round(value);

          case Abs:
            return fabs(value);

          case ToExposure:
            return (value > 0) ? log(value)/log(2) : -INFINITY ;

          case ToIntensity:
            return pow(2.0, value);

        }

      }
  };

}

#endif
