include(declare.act)
/*****************************************************************************
	Grammar for specifying declarations
*****************************************************************************/
setSKIP
ArrayInitializer :
				'{'
				instance = Initializer*
				'}'
			;

Bits		:
				':'
				length	= Number
			;

ButtonArray	:
				'['
				button	= Name+
				']'
			;
			
DeclareItem	:
				item	= MethodHead
                instance = MethodInitializer?
				','?
				Comment?
            |
                item    = Name
                '('
                argument = Expression
                ')'
			|
				item	= ItemHead
				assign	= ItemInitializer?
                initialize  = ':'?
				','?
				Comment?
			;

DeclareType	:
                #doNotGuard
				type	= Type
			;

Declaration	:
                #doNotGuard
				outlet	= 'outlet'?
				modify	= Linkage*
				type	= DeclareType
				declare	= DeclareItem+
			|
				declare = Structure
			|
				DeclareConditions
			;

Indirection	:
				direct	= [*&^]+
			;

InitExpression :
                instance = Expression
            |
                instance = RangeField
			;

Initializer	:
				instance = ExpressList
				','?
			|
				instance = ArrayInitializer
				','?
			;

ItemInitializer :
				'='
				SetObject?
				instance = (ArrayInitializer | InitExpression)
			;

ItemArray	:
				'[' [0-9]* ']'
			;

ItemHead	:
				direct	= Indirection?
				name	= Name
				array	= ItemArray*
				bits	= Bits?
			;

LambdaName	:
				name = NameSet
			;

MethodInitializer :
                '='
                instance    = FieldExpression
            ;

StructureItem	:
				item	= Declaration
				';'?
			|
				name	= Name
				bits	= Bits?
				buttons	= ButtonArray?
				','?
			;

StructureType :
				type	= Type
			|
				type	= Name
			;

StructureBody :
				label	= StructureType
				'{'
				entry	= StructureItem+
				'}'
				field	= DeclareItem*
			|
				entry	= StructureItem+
			;

Structure	:
				kind	= Structures textFollow!&
				body	= StructureBody
			;

Template    :
                '<>'&
            |
                '<'& <> '>'
            ;

TypeName    :
				type	= NameSet
            ;

Type		:
                #doNotGuard
                hasConst    = 'const'?
				noSign	    = 'unsigned'?
				type	    = TypeName
                temp        = Template?
			;

TypeList	:
				instance = Type
				','?
			;
