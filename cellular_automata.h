/*
 *	Header for cellular_automata.c
 */

// Includes, defines and macros //

#include <stdio.h>
#include <stdlib.h>     // for strtol
#include <stdbool.h>    // for bool datatype
#include <string.h>     // for strcmp, strcat, memcpy
#include <stdlib.h>     // for rand and srand
#include <time.h>       // for time (seeds RNG)
#include <ctype.h>	    // for tolower and isdigit 

// for getting window size. Need to use different methods depending on
// operating system
#if     defined(__unix__) || defined(__APPLE__)
    #include <sys/ioctl.h> 
#elif   defined(_WIN32)
	#include <windows.h>
#endif

#define PIXEL   '#'         // character used as "pixel" in visualization
#define STRMAX  32          // max allowable string length (for filename)
#define MAX_H   10000       // max allowable height
#define COIN    rand() % 2  // generates a 0 or 1


// Type declarations //

typedef enum { NO_OUT, TXT, BMP } Format;

typedef struct {
    
    bool    rand;
    bool    drawit;
    bool    quiet;
    int     rule;
    int     width;
    int     height;
    char    pixel;
    Format  format;
    char    fname[STRMAX];
    int     fgcol;
    int     bgcol;
    int     mult;
    
} Parameters;


// Function declarations //

int     cmd_params(int argc, char *argv[], Parameters *p);
bool    newcell(bool *ruleset, bool left, bool cell, bool right);

// For drawing, saving patterns
void    save_n_draw(bool ruleset[8], bool **pattern, Parameters p);
void    draw(bool ruleset[8], bool *pattern, Parameters p);
void    firstline(bool *pattern, Parameters p);
void    free_array(bool **p, int height);

// For file output
int     do_output(bool **p, Parameters param);
int     check_fname(char *inname, char *outname);
int     outtotxt(bool **pattern, Parameters param);
int     color_consts(char *color);

// General purpose
int     strtoint(char *str);


// Global constants (should all just be strings) //

const char *error_missing = "ERROR: Missing parameter";

const char *help_blurb =

    "Wolfram Elementary Cellular Automaton\n"
    "-------------------------------------\n"
    "\n"
    "PARAMETERS\n"
    "\n"
    "-?\t\tDisplay this help dialog\n"
    "-r ###\t\tSpecify Wolfram rule (0-255)\n"
    "-w ###\t\tWidth of display (in cells)\n"
    "-h ###\t\tHeight of display (defaults to half of width for non-randomized simulations)\n"
    "-R\t\tRandomizes first row of simulation\n"
    "-c @\t\tCustom character for text/terminal output (default is #)\n"
    "-o txt, bmp\tOutput .txt or 24-bit .bmp file\n"
    "-fg x######\tForeground (1) color in bitmap output\n"
    "-bg x######\tBackground (0) color in bitmap output. Like -fg, uses 6-digit RGB hex code\n"
    "-s #\t\tFor bitmaps, multiplies the size per pixel (must be from 1 to 8)\n"
    "-n\t\tDo not draw simulation in terminal before output\n"
    "-f FILENAME\tSupply filename for output (no extension needed)\n"
    "-Q\t\tSuppress confirmation questions\n"
    
    ;
