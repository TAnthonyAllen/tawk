class SymbolType;
class Symbol;
class PLGitem;
/*******************************************************************************
    Simple class to encapsulate a debugging directive
*******************************************************************************/

class Directive
{
public:
SymbolType *type;
Symbol *method;
char *codeMatch;
char *codeToAdd;
PLGitem *line;
struct 
	{
	unsigned int atEnd:1;
	unsigned int atStart:1;
	unsigned int comesBefore:1;
	unsigned int isDirected:1;
	unsigned int within:1;
	};
void emitDirective();
void parseDirective();
};
