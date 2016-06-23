// Description: It does math to attributes
// Author: tom
// Date: 2016-01-25

#include "AttributeMath.h"

#include <iostream>
#include <string>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>


using namespace TC_AttributeUtils;


void AttributeMath::setup(FnKat::GeolibSetupInterface &interface)
{
  interface.setThreading(FnKat::GeolibSetupInterface::ThreadModeConcurrent);
}

void AttributeMath::cook(FnKat::GeolibCookInterface &interface)
{
  FnAttribute::GroupAttribute operations = interface.getOpArg("operations");
  if(!operations.isValid()) {
    interface.stopChildTraversal();
    return;
  }

  FnAttribute::StringAttribute celAttr = interface.getOpArg("CEL");
  if( celAttr.isValid() ) {

    FnGeolibServices::FnGeolibCookInterfaceUtils::MatchesCELInfo match;
    FnGeolibServices::FnGeolibCookInterfaceUtils::matchesCEL(
        match, interface, celAttr);

    if(!match.matches) {
      if(!match.canMatchChildren) {
        interface.stopChildTraversal();
      }
      return;
    }
  }

  const int64_t numChildren = operations.getNumberOfChildren();
  for(int64_t i=0; i<numChildren; ++i) {

    FnAttribute::GroupAttribute spec = operations.getChildByIndex(i);

    if(spec.isValid()) {
      handleOperation(spec, interface);
    }
  }
}


void AttributeMath::handleOperation(
    FnAttribute::GroupAttribute &spec,
    FnKat::GeolibCookInterface &interface)
{
  FnAttribute::StringAttribute srcNameAttr = spec.getChildByName("attributeName");
  if(!srcNameAttr.isValid()) { return; }

  const std::string srcName = srcNameAttr.getValue();
  if(srcName.empty()) { return; }

  FnAttribute::DataAttribute srcAttr = interface.getAttr(srcName);

  if(!srcAttr.isValid()) {
    // see if we should apply to globals
    FnAttribute::IntAttribute applyGlobals = interface.getOpArg("applyToGlobalValues");
    if(applyGlobals.getValue(0, false) != 0) {
      srcAttr = FnKat::GetGlobalAttr(interface, srcName);
    }
  }

  if(!srcAttr.isValid()) {
    // see if we should apply to defaults
    FnAttribute::IntAttribute applyDefaults= interface.getOpArg("applyToDefaultValues");
    if(applyDefaults.getValue(0, false) != 0) {

      FnAttribute::GroupAttribute defaults =
        FnGeolibServices::FnGeolibCookInterfaceUtils::cookDaps(interface, "");

      srcAttr = defaults.getChildByName(srcName);
    }
  }

  if(!srcAttr.isValid()) {
    return;
  }


  FnAttribute::DataAttribute finalAttr;

  // We need to support ints, floats and doubles,
  FnAttribute::FloatAttribute asFloatAttr = srcAttr;
  if(asFloatAttr.isValid()) {
    finalAttr = handleAttr(asFloatAttr, spec, interface);
  } else {
    FnAttribute::DoubleAttribute asDoubleAttr = srcAttr;
    if(asDoubleAttr.isValid()) {
      finalAttr = handleAttr(asDoubleAttr, spec, interface);
    } else {
      FnAttribute::IntAttribute asIntAttr = srcAttr;
      if(asIntAttr.isValid()) {
        finalAttr = handleAttr(asIntAttr, spec, interface);
      }
    }
  }

  if(!finalAttr.isValid()){
    return;
  }

  // this is way more fun
  FnAttribute::StringAttribute destNameAttr = spec.getChildByName("destinationAttr");
  std::string destAttrName = destNameAttr.getValue(srcName, false);
  if(destAttrName == "") { destAttrName = srcName; }

  interface.setAttr(destAttrName, finalAttr);
}


