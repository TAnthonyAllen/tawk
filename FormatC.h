class BlockTok;
class Buffer;
class Instance;
class SymbolType;
class Symbol;
class Stak;
class Directive;
class Statement;
class Expression;
/*******************************************************************************
        Formats tok code in ObjectiveC or C++ or C (C++ w/methods global) format
*******************************************************************************/

class FormatC
{
public:
BlockTok *staticBlock;
Buffer *buffer;
Buffer *errorBuffer;
Buffer *forwardBuffer;
Buffer *headerBuffer;
Buffer *includeText;
Buffer *junkBuffer;
Buffer *structBuffer;
Instance *stringEncoder;
char *currentNameSpace;
char *filename;
SymbolType *currentType;
Symbol *ocStringConverter;
Symbol *enclosingMethod;
Stak *mStak;
int suppressedDeclarations;
Directive *pendingDirective;
Directive *endDirective;
Directive *startDirective;
struct 
	{
	unsigned int argumentIsOC:1;
	unsigned int jitting:1;
	unsigned int makeOCfile:1;
	unsigned int processingGlobalMethods:1;
	unsigned int writingParameters:1;
	};
FormatC();
void addNameSpace(char *name);
void checkInitialize(Instance *instance);
void checkSort(SymbolType *type);
void checkStatement(Statement *statement);
void checkTypedefs();
void close();
void convert(Expression *express);
void declare(BlockTok *block);
int declare(Symbol *symbol, int flag, int width);
void declare(Instance *instance, int forDotH);
void declare(Expression *expression);
void declareBody(SymbolType *type);
void declareClass(SymbolType *type);
void declareHeaders(SymbolType *type);
void declareMethods(SymbolType *type, int flag);
void declareStructure(SymbolType *type);
void declareTail(Instance *instance, int flag);
void flagType(SymbolType *type);
void forwardClass(SymbolType *type);
void indent();
void indentComment(Instance *instance);
Instance *initialStatement(Symbol *symbol);
void initialize(Symbol *method);
Instance *makeOCstring(Instance *instance);
void printCode();
void printQualified(Instance *instance);
int printQualified(Instance *instance, int ocParent, int ocMethodRef);
int printQualified(Symbol *symbol, int hasParent, int methodRef);
char *toString(Instance *instance);
void writeBlock(BlockTok *block);
void writeExpression(Expression *expression);
void writeIndirect(Instance *instance);
void writeInstance(Instance *instance);
void writeLambda(Statement *statement);
void writeOCsignature(Symbol *method);
void writeParameterType(Symbol *symbol);
void writeParameters(Instance *instance);
void writeSignature(Symbol *method, int flag);
void writeStatement(Statement *statement);
};
