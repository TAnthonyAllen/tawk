include includes

%%
insert(Tawk.rtn)
include(Tawk.act)
/*****************************************************************************
	This grammar was originally derived from a java grammar
		
	Java features tossed
		package
		inner classes
		inner interface
		got rid of commas in lists
		final becomes finally
		abstract
		protected
		public (this is the default, thank you)
		native
		threadsafe
		volatile
		transient
		class option on a new statement
Debug
    Block Line
;
*****************************************************************************/
Rules
	Block Body Bump CastExpression CastType CodePass Comment ConditionList Constant
    Declaration DotH Escape Expression ExpressItem ExpressList ExpressPart Extends
    Field FieldBody3 IfBody Indirection Inheritance Initializer InstanceBody
    ItemHead ItemInitializer Label Line Method MethodHead MethodInitializer
    MethodParameters Modify Name NameSet New NewArray NotQuote NotQuote2 Number Operator
    OperationTail Parameter ParameterItem ParameterList Path PoundCommand
    PrimaryExpression PrintItem PrintSet PrintShortcut Qualified Question Quote
    RangeExpression RangeField RangeTail ResetType RuleList SecondaryExpression
    Statement StatementBody Strings StringExpression SyntaxExtensions Target
    TargetMethod Template Throw Try Type TypeList UnaryExpression UnaryOperator
;
Set	alphaSet		[A-Za-z_] ;
Set	nameStartSet	[A-Za-z_@] ;
Set	commentSet		[-/#] ;
Set	compareSet		[-!/|%^?:&*<>+=glen] ;
Set	compareFollow	[-+ \t\n\r('\"*&!] ;
Set	logicSet		[glen] ;
Set	methodSet		[A-Za-z0-9_*&(),@] ;
Set methodNameSet	[A-Za-z0-9_:] ;
Set	nameSet			[A-Za-z0-9_] ;
Set	operatorSet		[-!/~|%^?:&*<>+=ei] ;
Set rangeSet        [<>.] ;
Set	singleQuote		['] ;
Set space			[ \t\n] ;
Set stringOP        [-+=] ;
Set	textFollow		[A-Za-z0-9_] ;
Set typesSet ;

include(keywords.g)
include(generate.g)
include(parts.g)
include(declare.g)
include(expression.g)
setSKIP
BlockStart	:
				brace	= '{'
			;

Block		:
				start	= BlockStart
				line	= Line*
				'}'
				FAIL	'Expected } or statement'
			;

Body		:
				Commands
            |
				body	= Method
			|
				body	= Declaration
				';'
				FAIL	'Expected a semi-colon'
			|
				#commentSet
				Comment
			|
                Include3
			;

Catch		:
				'catch'
				except		= Parameter?
				statement	= Statement
				ResetType?
			;

ClassHeading	:
				'class'
				nom			= ClassName
				attributes	= ClassAttributes*
			|
				externalRef = 'external'?
				structure	= Structure
				';'?
			|
				'external'
                kind = (kind = Structures textFollow!&)?
				nom			= ClassName*
				attributes	= ClassAttributes*
				';'?
			;

ClassBlockStart :
				'{'
			;

ClassBlock	:
				ClassBlockStart
				Body*
				'}'
				FAIL	'Expected }, static, or type'
			;

Directivise :
                line    = Line+
            ;

Divert		:
				Inheritance+
			;

Else		:
				'else'		textFollow!&
				statement	= Statement
				ResetType?
			|
				'or'		textFollow!&
				statement	= IfBody
			;

Extends		:
				ClassHeading
				ClassBlock?
			|
				#commentSet
				Comment
			|
				DeclareConditions
				';'
			;

Fielding    :   name    = Name
            ;

FieldComponent  :
                name    = Fielding
                body    = InstanceBody?
            |
                name    = TypeName
                body    = InstanceBody
            ;

FieldBody	:
				prefix	= Bump?
                part    = FieldComponent
			|
				name    = New
			|
				'('
				name    = Expression
				')'
			;

FieldExpression :
				cast	= CastExpression?
				direct	= Indirection?
				instance = Qualified
			;

Final		:
				'finally'
				statement	= Statement
				ResetType?
			;

ForOption	:
				instance	= '('
				initial		= ExpressList? ';'
				FAIL		'Expected expression list or ;'
				condition	= Expression? ';'
				FAIL		'Expected expression or ;'
				increment	= ExpressList?
				')'
				FAIL		'Expected expression list or )'
			|
				instance	= Expression
				name		= ( 'on' name = Name )?
			;

IfBody		:
				instance	= Expression
				Comment?
				action		= Statement
				ResetType?
				otherwise	= Else?
			;

Include		:
				include	= '#include' <> '\n'&
			|
				include	= '#import' <> '\n'&
			|
				('include' | 'import')
				[ \t]+&
				include	= '\n'}&
			;

setSKIP
Imports		:
				Include
			|
				Commands
			;

Inheritance	:
				Extends
			|
				Imports
			|
				method	= Body+
			|
				error	= EndComment
			;
			
InstanceTail	:
				array = NewArray+
			|
				'('
				ResetType
                NoShortcuts
				expression	= ParameterList?
				')'
				FAIL	instanceTailFail
			;

InstanceBody	:
				body	= InstanceTail
			;

Lambda		:
				function	= LambdaName
				'='
				body		= Block
			|
				function	= MethodType
				body		= Block
			;

Line		:
				'use'       textFollow!&
				target      = FieldExpression
			|
                '}'!&
				statement   = Statement
				ResetType?
            |
                'label'
                name        = Name+
                ';'
                FAIL        'Label declaration expected an ending ;'
			;

LineByLine  :
                #[A-Za-z0-9/(*]
                line        = Statement+
            ;

MethodParameters	:
				'('
				parameter   = Parameter*
				ellipsis    = '...'?
				')'
			;

MethodHead	:
				direct		= Indirection?
				function    = MethodName?
				head		= MethodParameters
			;

MethodType	:
				modify		= Linkage*
				type		= Type
				methodHead	= MethodHead
				'{'%
			;

Method		:
				method	= MethodType
				block	= Block
			;

New			:
				instance = 'new'	textFollow!&
				type	= Type?
				body	= InstanceBody?
				initial	= ArrayInitializer?
			;

NewArray	:
				'['
				instance  = Expression?
				']'
			;

Parameter	:
				type	= Type
				item	= ParameterItem*
				','?
			;

ParameterItem	:
				name	= MethodHead
				','?
			|
				direct	= Indirection?
				name	= Name
				array	= '[]'*
				','?
			|
				name	= '[]'+
				','?
			|
				name	= Indirection
				','?
			;

ParameterList	:
				expression	= ExpressItem+
			|
				expression	= TypeList+
			;

PrimaryExpression :
				instance = Constant
			|
				instance = SecondaryExpression
			|
				instance = FieldExpression
            |
                instance = AssumedString
			;

PrintTarget	:
				'('&
				instance = FieldExpression?
				')'
			;

PrintShortcut	:
				instance = [,:`]
			;

PrintItem	:
				instance = PrintShortcut
			|
				instance = Expression
				format	 = Format?
			;

PrintTo     :
                'to'
                instance    = FieldExpression
            ;

PrintCommand	:
				printer = 'print'	textFollow!&
				target  = PrintTarget?&
			|
				stdPrint    = 'cout'	textFollow!&
			|
				stdPrint    = 'cerr'	textFollow!&
			;

Print		:
				start       = PrintCommand
				arguments   = PrintItem*
                output      = PrintTo?
			;

QualifyStart :
				name = 'this'		textFollow!&
			|
				field = FieldBody
			;

QualifyTail	:
				'.'
				field	= FieldBody
			;

QualifyType	:
				type	= Type
				'.'
			;

Qualified	:
				type	= QualifyType?
				field	= QualifyStart
				rest	= QualifyTail*
				postfix	= Bump?
			;

SecondaryExpression :
				instance = 'null'   textFollow!&
			|
				instance = 'sizeof'
				'('
				Type
				pointer = '*'*
				')'
			;

Statement	:
                statement   = CheckMacro
            |
                statement   = StatementBody
            ;

StatementBody   :
				#commentSet
				statement	= CommentBody
			|
				statement	= Block
			|
				'if'						textFollow!&
				statement	= IfBody
			|
				statement	= 'return'		textFollow!&
				instance	= Expression?
				';'
			|
				'for'						textFollow!&
                Iterating?
				instance	= ForOption
				statement	= Statement
			|
				statement	= Print
				';'
			|
				'while'						textFollow!&
                Iterating?
				instance	= Expression
				Comment?
				statement	= Statement
			|
				statement	= Case
			|
				statement	= 'break'		textFollow!&
				';'
			|
				statement	= 'continue'	textFollow!&
				';'
			|
				statement	= 'goto'		textFollow!&
				direct      = Indirection?
				field		= Target
				';'
			|
                statement   = Switch
				Comment?
				block		= Block
				FAIL	'Switch expected a block'
			|
				Commands
			|
				statement	= ';'
			|
				statement	= 'delete'
				array		= '[]'?
				instance    = Qualified
				';'
			|
				'do'						textFollow!&
                Iterating?
				statement	= Statement
				ResetType
				'while'						textFollow!&
				instance	= Expression
				';'
			|
				statement	= Throw
				';'
			|
				statement	= Try
			|
				statement	= Declaration ';'
			|
				statement	= Expression ';'
			|
				statement	= Lambda
			;

Start		:   Inheritance+
			;

Switch      :   statement   = 'switch'    textFollow!&
				name        = FieldBody3?
				FAIL	'Switch expected expression in parentheses'
            ;

Target      :   field       = Qualified
            |   field       = Name
            ;

Throw		:   'throw'
				express		= Expression
			;

Try			:   'try'
				statement	= Statement
				ResetType
				catch		= Catch*
				end			= Final?
				;
%%

void assignFailed(PLGtester t)
{
Tawk    tOK = (Tawk)testParser;
    if assigning
        {
        assigning = false;
        cout "Assigning turned off":;
        }
}

void caseLabelFail(PLGtester t)
{
Tawk    tOK = (Tawk)testParser;
    assuming = false;
}

void expressPartFailed(PLGtester t)
{
Tawk    tOK = (Tawk)testParser;
   popVirtuals();
}

void instanceTailFail(PLGtester t)
{
Tawk    tOK = (Tawk)testParser;
    noShortcuts = false;
    printErrorMessage();
}
