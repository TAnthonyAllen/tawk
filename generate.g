
include(generate.act)
/*****************************************************************************
	Grammar for preprocessing and code generation
*****************************************************************************/
setSKIP
Replacement     :
                    Quote
                |
                    [^,;\n]+
                ;

AliasParameter	:
					parameter   = Name
                    replacedBy  = ('=' replacedBy = Replacement)?
					','?

                |
                    parameter   = Replacement
					','?
				;

AliasParameters :   body    = AliasParameter*
                ;

AliasBody		:
					body    = '(' <> ')'
				;

AliasTarget		:
					type	    = Type
                    indirect    = Indirection?
				|
					target	= Name
					body	= AliasBody?
				;

AliasItem		:
                    name    = Field
					target	= TargetMethod
				|
					'new'
					alias	= Name
                |
                    Comment
                |
					alias	= NameSet
					value	= AliasTarget
				;

ConditionLabel	:
					label	= Name
					[ \t]?
					text	= '\n'}&
				;

DeclareConditions :
					'Conditions'
					ConditionLabel+
				;

Extender        :   name    = Name
                ;

Field           :   name    = Name
                ;

InitializerItem :
                    field       = NameSet
                    function    = NameSet
                ;

OverLoadItem	:
					operate = Operator
                    name    = Name
				|
					'[]'
					assign	= '='?
					name	= Name
				|
					operate	= Bump
					name	= Name
                |
                    newOp   = (operatorSet+)
                    name    = Name
                |
                    '()'
                    name    = Name
				;

TargetMethod    :   target  = Name
                ;

/*****************************************************************************
	Grammar for specifying macros
*****************************************************************************/
MacroElement    :   element     = [^,)]+
                    ','?
                ;

resetSKIP
MacroBit        :
                    bitpart     = [a-zA-z0-9]+
                ;

MacroBodyPart   :
                    other       = MacroBit}
                |
                    rest        = ~.+
                ;

MacroBody       :
                    parts       = MacroBodyPart+
                ;

setSKIP
MacroDelimit    :
                    delimiter = [^a-zA-z0-9;]
                ;

MacroParameters :
                    '('
                    parameters  = MacroElement+
                    ')'
                ;

MacroDefine     :
                    name        = Name
                    parameters  = MacroParameters?
                    MacroDelimit
                    body        = macroDelimiter}
                ;

MacroName       :
                    name = NameSet
                ;

/*****************************************************************************
	Rules for evaluating macros
*****************************************************************************/
Braced      :
                '(' <> ')'
            ;

MacroPart   :
                [^,(]+
            |
                Braced
            ;

MacroArgument   :
                part    = (MacroPart+) ','?
            ;

MacroArgumentList   :
                argument    = MacroArgument+
            ;

CheckMacroParameters    :
                braced  = '(' <> ')'
            ;

CheckMacro  :
				statement   = MacroName
                braced      = CheckMacroParameters?
                ';'?
            ;

MacroBlock  :
				line	= Line*
            ;
resetSKIP
