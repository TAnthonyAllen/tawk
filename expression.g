
include(expression.act)
/*****************************************************************************
// Expressions (This comment is taken straight from Terence's java grammar)
// Note that most of these Expressions follow the pattern
//   thisLevelExpression :
//       nextHigherPrecedenceExpression
//           (OPERATOR nextHigherPrecedenceExpression)*
// which is a standard recursive definition for a parsing an Expression.
// The operators in java have the following precedences:
//    lowest  (13)  = *= /= %= += -= <<= >>= >>>= &= ^= |=
//            (12)  ?:
//            (11)  ||
//            (10)  &&
//            ( 9)  |
//            ( 8)  ^
//            ( 7)  &
//            ( 6)  == !=
//            ( 5)  < <= > >=
//            ( 4)  << >>
//            ( 3)  +(binary) -(binary)
//            ( 2)  * / %
//            ( 1)  ++ -- +(unary) -(unary)  ~  !  (Type)
//                  []   () (method call)  . (dot -- Identifier qualification)
//                  new   ()  (explicit parenthesis)
//
// the last two are not usually on a precedence chart; I put them in
// to point out that new has a higher precedence than '.', so you
// can validy use
//     new Frame().show()
// 
// Note that the above precedence levels map to the rules below...
// Once you have a precedence chart, writing the appropriate rules as below
// is usually straightfoward

	Note that the deep recursive nesting that appears in the java grammar
	is removed here. Because the current version compiles to C++, don't
	have to worry about precedence. The C++ precedence is in effect in any
	case. Once we compile to an intermediate language, precedence will have
	to be assigned to each operator and the action will have to change to
	make sure that highest level precedence operations happen first. It
	will be much faster than deep recursion.
*****************************************************************************/
CastType	:
				type	= Type
				direct	= Indirection?
				array	= ('[]'+)?
				','?
			;

CastTail	:
				'('
				rest	= CastType+
				')'
			;

CastExpression	:
				direct	= Indirection?
				'('
				type	= CastType
				rest	= CastTail?
				')'
			;

ConditionWord   :
				#doNotGuard
                list    = Conditions  textFollow!&
                ;

ExpressPart	:
                SaveVirtuals
				unaryOp     = UnaryOperator?
				instance    = UnaryExpression
                RangeTail!
                ExpressType?
				express	 = OperationTail*
			;

ExpressTail	:
				operate	 = ('&&' | '||')
				instance = ExpressPart
                FAIL    expressPartFailed
			;

Expression	:
				instance = ExpressPart
                FAIL    expressPartFailed
				express	 = ExpressTail*
			;

ExpressItem	:
				instance = Expression ','?
			;

ExpressList	:
                NoShortcuts
				list	= ExpressItem+
				FAIL	instanceTailFail
			;

Operator	:
				#operatorSet
				operand = Operators
			|
				#logicSet
				comparator = Comparisons
				alphaSet!&
			;

OperationTail	:
				#compareSet
				operate     = Operator
				instance    = UnaryExpression
                FAIL        assignFailed
				question    = Question?
			|
				instance    = Question
            |
                in          = 'in'        textFollow!&
                range       = RangeField
			;

Question	:
				question	= '?'
				trueExp		= Expression
				':'
				falseExp	= Expression
			;

RangeExpression :
                instance    = UnaryExpression
                back        = RangeTail
            ;

RangeField  :
                instance    = Name
            |
                instance    = RangeExpression
            ;

RangeTail :     #rangeSet
                operate     = Ranges
                instance    = Expression
            ;

StringExpression  :
                AllowShortcuts
                instance    = PrintShortcut
            |
                instance    = Expression
            ;

Strings     :
                #[^;]
                item        = StringExpression+
            ;

UnaryOperator	:
				operate = Bump
			|
				operate = [-+!~]+
			;

UnaryExpression :
				operate     = UnaryOperator?
				instance    = ConditionWord
			|
				operate     = UnaryOperator?
				cast        = CastExpression?
				instance    = PrimaryExpression
            |
                AllowShortcuts
                instance    = PrintShortcut
			;
