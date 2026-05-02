class Tawk;
class Tape;
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
void run();
};
int main(int argc, char **argv);
