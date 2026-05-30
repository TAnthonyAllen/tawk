class Tawk;
class Tape;
class PLGitem;
/*******************************************************************************
        This is just a shell class for running tok main - Need to redesign tok
		so that you can have a source file with no class, just globals
*******************************************************************************/

class Tok
{
public:
static Tawk *tawking;
int dummy;
static Tape *globalLinkTape;
static Tawk *testParser;
void run();
};
void assignFailed(PLGitem *iTEM);
void caseLabelFail(PLGitem *iTEM);
void expressPartFailed(PLGitem *iTEM);
void instanceTailFail(PLGitem *iTEM);
int main(int argc, char **argv);
