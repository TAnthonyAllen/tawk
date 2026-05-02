
include(parts.act)
/*****************************************************************************
	Odds and ends grammar rules
*****************************************************************************/
Alpha		:   [a-zA-Z0-9_.]+
			;

AssumedString	:   instance = NameSet
			;

Bump		:   bump    = ('++' | '--')
			;

Character	:   instance = (['] ( Escape | [^'] ) ['])
			;

EndComment	:   [^\n]+
			;
		
setSKIP
CaseLabel	:   instance    = Constant      ':'%
            |   instance    = RangeField
			|   instance    = Qualified     ':'%
            |   instance    = Expression
            |   instance    = Name
			;

Case		:
				instance	= 'default:'
			|
				'case'		textFollow!&
                Assuming
				instance	= CaseLabel
                FAIL        caseLabelFail
                ':'
            |
                instance    = Label
			;

ClassAttributes	:
				trait = Attributes	textFollow!&
			|
				'extends'
				type        = Type
            |
                'implements'
                proto       = Type+
            |
                'namespace'
                nSpace      = NameSet
			;

ClassName	:   ClassAttributes!
				path	= Path?
				name	= NameSet&
                temp    = Template?
				dotH	= '.h'?&
			;

Commands	:
				'#' PoundCommand
			|
				SyntaxExtensions
			;

CodePass    :   '-%' comment = '%-'}
            ;

CommentBody	:
				comment = CodePass
			|
				'//' end = EndComment
			|
				'/*' <> '*/'
			|
				'#ifdef' <> '#endif'
			|
				'#define' end = EndComment
			;

Comment		:   comment = CommentBody+
			;

Constant	:
				instance = Number
			|
				instance = Character
			|
				instance = Quote
            |
				instance = 'true'   textFollow!&
			|
            	instance = 'false'  textFollow!&
			;

CodeMatch   :
                body    = Quote
            |
                Directives!
                body    = space{
            ;

DebugDirective  :
                '#'!
                Comment?
                method  = Name
                body    = CodeMatch?
                locate  = Directives?
                active  = 'active'?
                code    = '#;'}
            ;

Directive   :
                type        = Type
                directives  = DebugDirective*
            ;

resetSKIP
Count		:   [0-9]+
			;

EscapeCharacters :
				[nrtbf\"'\\]
			|
				'u'+ [0-9a-fA-F]{4,4} 
			|
				[0-3][0-9]?[0-9]?
			|
				[4-7][0-9]?
			;

Escape		:   '\\'
				EscapeCharacters
			;

FileName	:   path	= Path?
				name	= NameSet
				'.twk'
			;

Format		:   '#'
                [- 0+]?
                width   = [0-9]*
                [*%.0-9a-zA-Z]*
			;

Label		:   name   = Name ':'
			;

NameSet		:   nameStartSet
				nameSet*
			;

MethodNameSet	:
				nameStartSet
				methodNameSet*
			;

setSKIP
MethodName	:   name = MethodNameSet
			;

Name		:   name = NameSet
			;

FieldList	:   'Field' textFollow!&
				name	= [A-Za-z0-9_()*]*
				';'
            |
                'Map;'
			;

PoundCommand	:
				state   = State
				type	= Type?
				level	= Count?
				list	= RuleList?
                field   = FieldList?
            |
                MacroDefine
            |
                Directive
			;

DebugText   :   '='
                upcoming    = Quote
            ;

DebugRule   :
                name        = NameSet+
                upcoming    = DebugText?
            ;

RuleList	:   'Rule'  textFollow!&
				debugRULE	= DebugRule+
				';'
			;

SyntaxExtensions :
				'overload'	textFollow!&
				OverLoadItem+
				';'
			|
				'alias'		textFollow!&
				AliasItem+
				';'
            |
                'extender'  textFollow!&
                Extender+
                ';'
            |
                'initializer' textFollow!&
                InitializerItem+
                ';'
			;

resetSKIP
Number		:
				instance    = ([0-9]+ '.' [0-9]+ ([eE][+-]?[0-9]+)?)
			|
				instance    = (('0'[xX][0-9a-fA-F]+)|([0-9]+))
				isLong	    = [lL]?
			;

Path		:   '/'?
				(Alpha '/')+
			;

Quote		:   string		= '@'?
				instance    = '"'
                body        = '"'}&
			;
