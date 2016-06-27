// Copyright (c) 2016 Tom Cowland
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

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


