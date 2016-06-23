
def registerAttributeMathOp():

    from Katana import Nodes3DAPI
    from Katana import FnAttribute, FnGeolibServices

    def buildOpChain( node, interface ):

        interface.setMinRequiredInputs(1)

        graphState = interface.getGraphState()
        frameTime = graphState.getTime()

        cel = node.getParameter('CEL').getValue(frameTime)

        attrName = node.getParameter('attributeName').getValue(frameTime)
        attrType = node.getParameter('attributeType').getValue(frameTime)

        operation = node.getParameter('operation').getValue(frameTime)

        operationGb = FnAttribute.GroupBuilder()
        operationGb.set("attributeName", FnAttribute.StringAttribute(attrName))
        operationGb.set("operation", FnAttribute.IntAttribute(int(operation)))

        modifierSource = node.getParameter('modifierSource').getValue(frameTime)
        modifierValue = node.getParameter('modifierValue').getValue(frameTime)
        modifierAttr = node.getParameter('modifierAttribute').getValue(frameTime)

        destinationAttr = node.getParameter('options.destinationAttribute').getValue(frameTime)

        if modifierSource == "value":

          if attrType == "float":
            operationGb.set("modifier", FnAttribute.FloatAttribute(float(modifierValue)))
          elif attrType == "double":
            operationGb.set("modifier", FnAttribute.DoubleAttribute(float(modifierValue)))
          else:
            operationGb.set("modifier", FnAttribute.IntAttribute(int(modifierValue)))

        elif modifierSource == "array":

          p = node.getParameter('modifierArray')
          vals = [c.getValue(frameTime) for c in p.getChildren()]

          if attrType == "float":
            operationGb.set("modifier", FnAttribute.FloatAttribute([float(v) for v in vals]))
          elif attrType == "double":
            operationGb.set("modifier", FnAttribute.DoubleAttribute([float(v) for v in vals]))
          else:
            operationGb.set("modifier", FnAttribute.IntAttribute([int(v) for v in vals]))

        else:
            operationGb.set("modifier", FnAttribute.StringAttribute(modifierAttr))

        if destinationAttr:
          operationGb.set("destinationAttr", FnAttribute.StringAttribute(destinationAttr))

        argsGb = FnAttribute.GroupBuilder()
        argsGb.set("system", graphState.getOpSystemArgs())

        argsGb.set("CEL", FnAttribute.StringAttribute(cel))
        argsGb.set("operations.o1", operationGb.build())

        applyToGlobals = node.getParameter('options.applyToGlobalValues').getValue(frameTime)
        applyToDefaults = node.getParameter('options.applyToDefaultValues').getValue(frameTime)

        argsGb.set("applyToGlobalValues", FnAttribute.IntAttribute(applyToGlobals))
        argsGb.set("applyToDefaultValues", FnAttribute.IntAttribute(applyToDefaults))

        interface.appendOp('AttributeMathOp', argsGb.build())


    # Here we need to define the parameters for the node, and register the op
    # chain creation callback function

    nodeBuilder = Nodes3DAPI.NodeTypeBuilder( "AttributeMath" )
    nodeBuilder.setInputPortNames( ("in",) )

    # Parameters can be described by a group attribute
    paramGb = FnAttribute.GroupBuilder()
    paramGb.set("CEL", FnAttribute.StringAttribute('') )
    paramGb.set("attributeName", FnAttribute.StringAttribute('') )
    paramGb.set("attributeType", FnAttribute.StringAttribute('float'))
    paramGb.set("operation", FnAttribute.IntAttribute(0))
    paramGb.set("modifierSource", FnAttribute.StringAttribute('value') )
    paramGb.set("modifierValue", FnAttribute.DoubleAttribute(1.0) )
    # We split this out - as otherwise we have to specify a default value with
    # more than one element (so that an Array param is created instead of a
    # Scalar param) - many cases a single value is enough, and the dynamic
    # array widget gets a bit confusing
    paramGb.set("modifierArray", FnAttribute.DoubleAttribute([1.0,1.0,1.0]) )
    paramGb.set("modifierAttribute", FnAttribute.StringAttribute('') )

    paramGb.set("options.applyToGlobalValues", FnAttribute.IntAttribute(0) )
    paramGb.set("options.applyToDefaultValues", FnAttribute.IntAttribute(0) )
    paramGb.set("options.destinationAttribute", FnAttribute.StringAttribute('') )

    nodeBuilder.setParametersTemplateAttr( paramGb.build() )

    nodeBuilder.setHintsForNode( {
      'widget': 'attributeset'
    } )

    nodeBuilder.setHintsForParameter( "CEL", {
        'widget':'cel'
    } )

    nodeBuilder.setHintsForParameter( "operation", {
        'widget':'mapper',
        'options': {
          'add': 0, 'subtract': 1, 'multiply': 2, 'divide' : 3,
          'power': 4, 'exponent': 5, 'min': 6, 'max': 7, 'mod': 8,
          'ceil' : 9, 'floor': 10, 'round' : 11, 'abs': 12,
          'intensity to exposure':13, 'exposure to intensity': 14
        },
        'options__order': ('add', 'subtract', 'multiply', 'divide', 'power',
          'exponent', 'min', 'max', 'mod', 'ceil', 'floor', 'round', 'abs',
          'intensity to exposure', 'exposure to intensity' )
    } )

    nodeBuilder.setHintsForParameter( "attributeType", {
        'widget':'popup',
        'options':['float', 'double', 'integer'],
        'help':
          "This must match the data type of the attribute to be modified. "+\
          "If you're not sure, use the 'Drop Attribute Here' widget."
    } )

    nodeBuilder.setHintsForParameter( "modifierSource", {
        'widget':'popup',
        'options':['value', 'array', 'attribute'],
        'conditionalVisPath': '../operation',
        'conditionalVisValue': 9,
        'conditionalVisOp': 'lessThan',
        'help':
           "Value/Array: A constant modifier, specified below. "+\
           "Attribute: The modifier value is taken from the named attribute "+\
           "at the same location. NOTE: This must be of the same type as the "+\
           "target attribute."
    } )

    nodeBuilder.setHintsForParameter( "modifierValue", {
      'conditionalVisOps': {
        'conditionalVisOp': 'and',
        'conditionalVisLeft': 'conditionalVis1',
        'conditionalVisRight': 'conditionalVis2',
        'conditionalVis1Path': '../modifierSource',
        'conditionalVis1Value': 'value',
        'conditionalVis1Op': 'equalTo',
        'conditionalVis2Path': '../operation',
        'conditionalVis2Value': 9,
        'conditionalVis2Op': 'lessThan',
      }
    } )

    nodeBuilder.setHintsForParameter( "modifierArray", {
      'conditionalVisOps': {
        'conditionalVisOp': 'and',
        'conditionalVisLeft': 'conditionalVis1',
        'conditionalVisRight': 'conditionalVis2',
        'conditionalVis1Path': '../modifierSource',
        'conditionalVis1Value': 'array',
        'conditionalVis1Op': 'equalTo',
        'conditionalVis2Path': '../operation',
        'conditionalVis2Value': 9,
        'conditionalVis2Op': 'lessThan'
      },
      'widget':'dynamicArray'
    } )

    nodeBuilder.setHintsForParameter( "modifierAttribute", {
      'conditionalVisOps': {
        'conditionalVisOp': 'and',
        'conditionalVisLeft': 'conditionalVis1',
        'conditionalVisRight': 'conditionalVis2',
        'conditionalVis1Path': '../modifierSource',
        'conditionalVis1Value': 'attribute',
        'conditionalVis1Op': 'equalTo',
        'conditionalVis2Path': '../operation',
        'conditionalVis2Value': 9,
        'conditionalVis2Op': 'lessThan'
      },
      'help':
          "An attribute name at the location being processed, to be used "+\
          "as the modifier value"
    } )


    nodeBuilder.setHintsForParameter( "options.applyToGlobalValues", {
        'widget':'checkBox',
        'help':"When set, global values will also be modified"
    } )

    nodeBuilder.setHintsForParameter( "options.applyToDefaultValues", {
        'widget':'checkBox',
        'help':"When set, default values will also be modified"
    } )

    nodeBuilder.setBuildOpChainFnc( buildOpChain )
    nodeBuilder.build()


registerAttributeMathOp()

