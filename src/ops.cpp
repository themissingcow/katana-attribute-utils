// Description: A number of Ops to provide basic Attribute manipulations
// Author: tom
// Date: 2016-06-20

#include <FnGeolib/op/FnGeolibOp.h>
#include <FnPluginSystem/FnPlugin.h>


#include "AttributeMath.h"

using namespace TC_AttributeUtils;

namespace
{
  DEFINE_GEOLIBOP_PLUGIN(AttributeMath)
}

void registerPlugins()
{
    REGISTER_PLUGIN(AttributeMath, "AttributeMathOp", 0, 1);
}

