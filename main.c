/* Keegan Covey (C) 2019 
 *
 * Draws an elementary cellular automaton using Wolfram rule
 *
 *          Example: RULE 30 (00011110)
 *        111|110|101|100|011|010|001|000
 *         0 | 0 | 0 | 1 | 1 | 1 | 1 | 0     
 */

#include "cellular_automata.h"
#include "bmp.c"
#include "subs.c"

int main(int argc, char *argv[])
{
    bool ruleset[8];

    Parameters param;

    param.rand      = 0;
    param.quiet     = 0;
    param.drawit    = 1;
    param.rule      = -1;
    param.width		= -1;
    param.height    = 0;
    param.pixel		= PIXEL;
    param.format    = NO_OUT;
    param.fgcol		= 0x000000;
    param.bgcol		= 0xffffff;
    param.mult		= 1;
    param.fname[0]  = '\0';

	// Unix and Windows have their own function calls for getting the screen buffer size
	#if		defined(__unix__) || defined(__APPLE__)

		struct winsize max;
		ioctl(0, TIOCGWINSZ, &max);    // get window size at time of execution
		int maxwin = max.ws_col;
		
	#elif   defined(_WIN32)

		//int maxwin = 50;
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		int maxwin = csbi.srWindow.Right - csbi.srWindow.Left+1;
		
	#endif

    // process command line parameters. End execution if parameter returns 1 (ie an
    // incorrect parameter has been given)
    if(cmd_params(argc, argv, &param))
        return 0;

    // Get rule and display width from user, if they weren't already specified in command 

    if(param.rule < 0) {
        printf("Provide rule number (0-255)");
        do {
            printf(">");
            scanf("%d", &param.rule);
        } while(param.rule < 0 || param.rule > 255);
    }
    if(param.width == -1) {
        printf("Provide width of sample (type 0 for window width)");
        do {
            printf(">");
            scanf("%d", &param.width);
        } while(param.width < 0);
    }

    // fill ruleset table for each bit of rule value
    for(int i=0, rule = param.rule; i<8; i++) {
        ruleset[i] = rule & 1;    // flag bit on if it's a 1 in rule number
        rule >>= 1;               // advance next bit into 1's place
    }

    if(!param.width)             	// if width is unspecified, defaults to window width
        param.width = maxwin;

    if(!param.height || !param.rand)    // height defaults to half of width. This will always be
        param.height = param.width/2;	// the case if pattern is not randomized

    if(param.rand)                  	// seed RNG if randomized is set
        srand(time(0));

    if(!param.format) {
        if(param.width > maxwin) {
            printf("Specified width is wider than window! Continue? >");
            if(tolower(getchar()) != 'y')
                return 0; 
        }
        draw(ruleset, NULL, param);
    } else        // otherwise, save array to memory and then output it to specified file format
    {
        if(param.width > maxwin && param.drawit) {
            printf("Specified width is wider than window! Draw anyway? (If not, we can still save to %s) >", 
                                                                 (param.format == TXT) ? "text" : "bitmap");
            if(tolower( getchar() ) != 'y') param.drawit = 0;
            getchar();
        }

        // allocate 2-dimensional array for storing pattern
        bool **p = NULL;
        p = malloc(param.height * sizeof *p);
        for(int y=0; y<param.height; y++)
            p[y] = malloc(param.width * sizeof *p[y]); 

        save_n_draw(ruleset, p, param);    // assign 0 or 1 values to every cell in array

        if(!param.quiet) {
            printf("Save to %s (Y/N)? ", (param.format == TXT) ? "text" : "bitmap");
            if(tolower( getchar() ) != 'y') {
                free_array(p, param.height);
                return 0;
            }
            getchar();
        }

        if(do_output(p, param)) return 1;
    }
	
	return 0;
}
