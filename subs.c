/*    
 *    Subprograms for cellular_automata.c
 */
 

// newcell
// determines what each cell will be based on the three immediately above it

bool newcell(bool *ruleset, bool left, bool cell, bool right)
{
    unsigned char threesome = 0;    // threesome holds the three above cells, and
                                    // should always be between 000 and 111 (0 and 7)

    threesome |= right;             // bit to right of center cell
    threesome |= (int)(cell << 1);  // center cell (the one above selected)
    threesome |= (int)(left << 2);  // bit to right of center cell

    // determines what new cell will be based on already-defined matching rule
    for(int i=0b000; i<=0b111; i++)
        if(i == threesome)
            return ruleset[i];

    return 0;   // this should never run
}


// firstline
// Draws/saves first line of array

void firstline(bool *pattern, Parameters p)
{
    for(int x=0; x<p.width; x++) {
        if(!p.rand)
            pattern[x] = (x == p.width/2) ? 1 : 0; // set middle cell only to 1
        else
            pattern[x] = COIN;                      // if randomized, randomize each cell
        if(p.drawit)
            printf("%c", pattern[x] ? p.pixel : ' ');
    }
    if(p.drawit)
        printf("\n");   
}


// draw
// Recursive function for drawing pattern with single-row array. Uses a lot less memory than
// save_n_draw since only two rows at a time are allocated

void draw(bool ruleset[8], bool *lastrow, Parameters p)
{
    static long y = 1;  
    bool nextrow[p.width];

    if(lastrow == NULL) {
        lastrow = malloc(p.width * sizeof (bool) );
        firstline(lastrow, p);
    }

    for(int x=0; x<p.width; x++) {
        bool left, right;

        left    = (x>0)         ? lastrow[x-1] : lastrow[p.width-1];
        right   = (x<p.width-1) ? lastrow[x+1] : lastrow[0];
        
        nextrow[x] = newcell(ruleset, left, lastrow[x], right);
        printf("%c", nextrow[x] ? p.pixel : ' ');
    }

    if(y == 1) free(lastrow);

    printf("\n");
    if(++y<p.height)
        draw(ruleset, nextrow, p);
}


// save_n_draw
// Saves entire pattern to a 2D array so that it can be used again later and saved to
// plain-text or BMP output

void save_n_draw(bool ruleset[8], bool **pattern, Parameters p)
{
    firstline(pattern[0], p);

    // fill rest of pattern table
    for(int y=1; y<p.height; y++) {       // by row
        for(int x=0; x<p.width; x++) {    // by cell 
            bool left, right;

            // simulation "wraps around"
            left    = (x>0)         ? pattern[y-1][x-1] : pattern[y-1][p.width-1];
            right   = (x<p.width-1) ? pattern[y-1][x+1] : pattern[y-1][0];

            pattern[y][x] = newcell(ruleset, left, pattern[y-1][x], right);
            if(p.drawit)
                printf("%c", pattern[y][x] ? p.pixel : ' ');
        }
        if(p.drawit)
            printf("\n");
    }
}


// free_array
// frees memory allocated to 2D array

void free_array(bool **p, int height)
{
    for(int y=0; y<height; y++)
        free(p[y]);
    free(p);
}


// do_output
// Handles getting filename from user. Actual file output is passed to other subprograms. Returns
// 1 if there is an error

int do_output(bool **pattern, Parameters p)
{
    if(p.fname[0] == '\0') {
        printf("What filename to use? (Leave blank for \"rule%d%s\") ", p.rule, p.rand ? "_rand" : "");

        //getchar();  // some hacky shit to absorb return keystroke

        fgets(p.fname, STRMAX, stdin);

        // default filename
        if(p.fname[0] == '\n')
            snprintf(p.fname, sizeof p.fname, "./rule%d%s", p.rule, p.rand ? "_rand" : "");

        // parse out user filename
        else
            if(check_fname(p.fname, &p.fname[0]) ) {
                printf("ERROR: Invalid filename %s\n", p.fname);
                return 1;
            }
    }
    strcat(p.fname, (p.format == TXT) ? ".txt" : ".bmp");

    switch(p.format) {
        case TXT:
        if(outtotxt(pattern, p) )  // outputs to text file
            return 1;           // ends program if text output fails for any reason
        break;

        case BMP:
        if(outtobmp(pattern, p) ) // outputs to bitmap file
            return 1;                   // as above, ends if output fails
        break;

        default:
        printf("ERROR: Unrecognized file format\n");    // this should never run but might 
        return 1;                                       // as well be thorough
    }

    printf("\nSaved to %s\n", p.fname);
    free_array(pattern, p.height);    

    return 0;
}


// check_fname
// makes sure filename is valid, returns 1 if not

int check_fname(char *inname, char *outname)
{
    int i = 0;
    outname[i] = inname[i];

    do {   
        switch(outname[i]) {
            case '/': case '.': case '\\': case ';': case ':':
                return 1;
            default:
                break;
        }
        i++;
        
    } while((outname[i]=inname[i]) != '\n' && inname[i] != '\0');   // advance to return character    
     
    outname[i] = '\0';                                              // and replace it with a null  
      
    return 0; 
}


// outtotxt
// outputs saved array to text file, frees array. Returns 1 if there's an error

int outtotxt(bool **pattern, Parameters p)
{
    FILE *fp;    // pointer to file

    if((fp = fopen(p.fname, "w") ) == NULL) {
        printf("ERROR: Failed to create file %s\n", p.fname);
        return 1;
    }

    for(int y=0; y<p.height; y++) {
        for(int x=0; x<p.width; x++)
            fprintf(fp, "%c", pattern[y][x] ? p.pixel : ' ');
        fprintf(fp, "\n");
    }

    if(fclose(fp) ) {
        printf("ERROR: Failed to close file stream\n");
        return 1;
    }
    return 0;
}


// cmd_params
// processes command line parameters. Returns 1 if there is an error

int cmd_params(int argc, char *argv[], Parameters *p)
{
    for(int i=1; i<argc; i++)
    
        // -h: shows help dialog and ends program
        if(!strcmp(argv[i], "-?")) {
            printf("\n%s\n", help_blurb);
            return 1;
        } 

        // -R: Randomize flag. Top row is randomized instead of single center cell
        else if(!strcmp(argv[i], "-R"))
            p->rand = 1;

        // -n: Supresses drawing to shell
        else if(!strcmp(argv[i], "-n") )
            p->drawit = 0;
            
        // -Q: don't ask to save to BMP
        else if(!strcmp(argv[i], "-Q") )
            p->quiet = 1;
        
        // -r ###: user provides rule
        else if(!strcmp(argv[i], "-r")) {
            if(i+1 == argc) {
                printf("%s\n", error_missing);
                return 1;
            }
            p->rule = strtoint(&argv[++i][0]);
            if(p->rule < 0 || p->rule > 255) {
                printf("ERROR: Rule out of range or invalid\n");
                return 1;
            }
        } 

        // -w ###: width of display
        else if(!strcmp(argv[i], "-w")) {
            if(i+1 == argc) {
                printf("%s\n", error_missing);
                return 1;
            }
            p->width = strtoint(&argv[++i][0]);
            if(p->width < 0) {
                printf("ERROR: Width out of range or invalid\n");
                return 1;
            }

        } 

        // -h ###: height of display
        else if(!strcmp(argv[i], "-h") ) {
            if(i+1 == argc) {
                printf("%s For help dialog type \"-?\"\n", error_missing);
                return 1;
            }
            p->height = strtoint(&argv[++i][0]);
            if(p->height < 1) {
                printf("ERROR: Height out of range or invalid\n");
                return 1;
            }
        }

        // -c @: single character used as "pixel" in display
        else if(!strcmp(argv[i], "-c") ) {
            if(i+1 == argc) {
                printf("%s\n", error_missing);
                return 1;
            }
            if(strlen(argv[++i]) > 1) {
                printf("ERROR: -c takes single character\n");
                return 1;
            }
            p->pixel = argv[i][0];
        }

        // -o: output result to specified format
        else if(!strcmp(argv[i], "-o") ) {
            if(i+1 == argc) {
                printf("%s\n", error_missing);
                return 1;
            }
            if(!strcmp(argv[++i], "txt") )
                p->format = TXT;
                
            else if (!strcmp(argv[i], "bmp") )
                p->format = BMP;

             else {
                printf("ERROR: Invalid output format %s\n", argv[i]);
                return 1;
            }
        }
        
        // -fg COLOR: set foreground color, supplying either hex code or CGA color name
        else if(!strcmp(argv[i], "-fg") ) {
            if(i+1 == argc) {
                printf("%s\n", error_missing);
                return 1;
            }
            if(argv[++i][0] == 'x')
                p->fgcol = (int)strtol(&argv[i][1], NULL, 16);
            else
                if( (p->fgcol = color_consts(argv[i])) == -1) {
                    printf("ERROR: Invalid color\n");
                    return 1;
                }
        }

        // -bg COLOR: same as above but for background color
        else if(!strcmp(argv[i], "-bg") ) {
            if(i+1 == argc) {
                printf("%s\n", error_missing);
                return 1;
            }
            if(argv[++i][0] == 'x') 
                p->bgcol = (int)strtol(&argv[i][1], NULL, 16);
            else
                if( (p->bgcol = color_consts(argv[i])) == -1) {
                    printf("ERROR: Invalid color\n");
                    return 1;
                }
        }

        // -s #: for bitmaps, how many pixels square each cell should be (1-8)
        else if(!strcmp(argv[i], "-s") ) {
            if(i+1 == argc) {
                printf("%s\n", error_missing);
                return 1;
            }
            p->mult = strtoint(&argv[++i][0]);
            if(p->mult <=0 || p->mult > 8) {
                printf("ERROR: stretch out of range or invalid\n");
                return 1;
            }
        }

        // -f FILENAME: supply output filename in command line
        else if(!strcmp(argv[i], "-f") ) {
            if(i+1 == argc) {
                printf("%s\n", error_missing);
                return 1;
            }
            if(strlen(argv[++i]) < STRMAX)
                if(check_fname(argv[i], p->fname) ) {
                    printf("ERROR: Invalid filename %s\n", p->fname);
                    return 1;
                }
        }
        
        else {
            printf("ERROR: Invalid parameter %s\n", argv[i]);
            return 1;
        }

    return 0;
}


// color_consts
// checks if string provided matches one of 16 CGA colors, returns -1 if not

int color_consts(char *color)
{
    const int BLACK     = 0x000000;
    const int DARKGRAY  = 0x555555;
    const int DARKBLUE  = 0x0000AA;
    const int BLUE      = 0x5555FF;
    const int DARKGREEN = 0x00AA00;
    const int GREEN     = 0x55FF55;
    const int DARKCYAN  = 0x00AAAA;
    const int CYAN      = 0x55FFFF;
    const int DARKRED   = 0xAA0000;
    const int RED       = 0xFF5555;
    const int DARKPRPL  = 0xAA00AA;
    const int PURPLE    = 0xFF55FF;
    const int BROWN     = 0xAAAA00;
    const int YELLOW    = 0xFFFF55;
    const int GRAY      = 0xAAAAAA;
    const int WHITE     = 0xFFFFFF;
    
         if(!strcmp(color, "black") )      return BLACK;
    else if(!strcmp(color, "darkgray") )   return DARKGRAY;
    else if(!strcmp(color, "darkblue") )   return DARKBLUE;
    else if(!strcmp(color, "blue") )       return BLUE;
    else if(!strcmp(color, "darkgreen") )  return DARKGREEN;
    else if(!strcmp(color, "green") )      return GREEN;
    else if(!strcmp(color, "darkcyan") )   return DARKCYAN;
    else if(!strcmp(color, "cyan") )       return CYAN;
    else if(!strcmp(color, "darkred") )    return DARKRED;
    else if(!strcmp(color, "red") )        return RED;
    else if(!strcmp(color, "darkpurple") ) return DARKPRPL;
    else if(!strcmp(color, "purple") )     return PURPLE;
    else if(!strcmp(color, "brown") )      return BROWN;
    else if(!strcmp(color, "yellow") )     return YELLOW;
    else if(!strcmp(color, "gray") )       return GRAY;
    else if(!strcmp(color, "white") )      return WHITE;
    
    else return -1;
}


// strtoint
// converts string to integer, return -1 if not a valid int

int strtoint(char *str)
{
    int x = 0;
    int len = strlen(str);
    for(int i=0; i<len; i++) {
        if(!isdigit(str[i])) return -1;
        
        x = x * 10 + (str[i] - '0');
    }
    return x;
}
