# Attribute Utils

## AttributeMath

It’s always bugged my how long winded it is to apply relative changes to numeric attributes, this gets even harder if they’re shader parameters and so may be set to default values. Writing an OpScript for it always seems like overkill.

### Basic features:

Apply a bunch of mathematical operators to attribute values
Source the ‘modifier’ value from another attribute.
Apply changes to local, global and default value holding attributes.
Store the new value under a different name.
For arrays, you can either have a constant modifier, or a per-element modifier.
For the technically minded, the actual Op can support multiple operations at once, just the UI was hard, so the Node only defines one operation.

### Current Limitations:

If using another attribute to source the modifier value, it has to be the same type as the destination attribute (ie: you can’t use a float to multiply a double, or int, etc…)
Not had extensive testing, let me know if i’ve messed up the maths somewhere.
Operations:

* Add
* Subtract
* Multiply
* Divide
* Power (value ^ modifier)
* Exponent (modifier ^ value)
* Min
* Max
* Mod
* Ceil
* Floor
* Round
* Abs
* Intensity to Exposure
* Exposure to Intensity

## License

This code is licenced under the MIT licence.