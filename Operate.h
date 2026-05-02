class KeyTable;
class Instance;
/*******************************************************************************
        This encapsulates an operator, providing precedence rating and pointer
        to the verbs code.
*******************************************************************************/

class Operate
{
public:
char *op;
int rank;
void (*method)(Instance *);
struct 
	{
	unsigned int assign:1;
	unsigned int call:1;
	unsigned int comparison:4;
	unsigned int conjunction:1;
	unsigned int overload:1;
	unsigned int pointing:1;
	unsigned int question:1;
	unsigned int isRange:1;
	unsigned int unary:1;
	};
#define eql(button) (button == 1)
#define ltn(button) (button == 2)
#define lte(button) (button == 3)
#define gtn(button) (button == 4)
#define gte(button) (button == 5)
#define neq(button) (button == 6)
static KeyTable *verbs;
Operate(char *v);
Operate(char *v, int p);
int compare(char *v);
};
