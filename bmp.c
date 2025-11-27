/*
 *  Subprograms, includes, structs, definitions etc. relevant to bitmap drawing for Wolfram
 *
 *  Info on .bmp file format obtained from
 *  https://web.archive.org/web/20080912171714/http://www.fortunecity.com/skyscraper/windows/364/bmpffrmt.html
 *  https://web.archive.org/web/20190818002821/https://ricardolovelace.com/creating-bitmap-images-with-c-on-windows.html
 */

/*  BITMAP DATA STRUCTURES
 *  These are the two file headers mandatory to each bmp file, as well as a color structure.
 *  They are strictly defined in size and order of fields, thus the careful distinction
 *  between short and long ints here.
 */

#define DPI     96
#define PPM     DPI * 39.37
#define MIN_BMP 54

// Bitmap file headers are already in windows.h.  In Unix-based systems I must define these manually.
#if		defined(__unix__) || defined(__APPLE__)

    // File header (14 bytes)

    typedef struct {

        unsigned char   bfType[2];    // must be set to "BM" to declare that this is a .bmp file
        int             bfSize;       // file size in bytes
        short           bfReserved1;  // must always be 0
        short           bfReserved2;  // also must always be 0
        unsigned int    bfOffBits;    // offset from beginning of file to bmp data. Should be 54

    } __attribute__((packed))         // needed to prevent GCC from padding variables by size
        BITMAPFILEHEADER;

    // Information header (40 bytes)

    typedef struct {

        unsigned int    biSize;           // size of BITMAPINFOHEADER structure in bytes
        unsigned int    biWidth;          // width of image (pixels)
        unsigned int    biHeight;         // height of image (pixels)
        short           biPlanes;         // number of planes of target device (must be 0 or 1)
        short           biBitCount;       // # of bits per pixel (1, 4, 8, 16, 24 or 32)
        unsigned int    biCompression;    // type of compression; 0 = no compression
        unsigned int    biSizeImage;      // size of image data (bytes). 0 if no compression
        unsigned int    biXPelsPerMeter;  // horizontal pixels per meter (usually 0)
        unsigned int    biYPelsPerMeter;  // vertical pixels per meter
        unsigned int    biClrUsed;        // # of colors in bmp. If 0, calculate with biBitCount
        unsigned int    biClrImportant;   // # of colors that are "important" to bmp.
                                          // If 0, all colors are important
    } __attribute__((packed))
        BITMAPINFOHEADER;
		
#endif


// Color information
// Note that bitmaps, like a lot of other stuff apparently, hold their color info backwards;
// in other words, the colors are stored in BGR order instead of RGB. This struct doesn't
// need to reflect this, of course, but it might as well

typedef struct {

    unsigned char blue;
    unsigned char green;
    unsigned char red;

} bgr_data;


// bmp specific function declarations

int         outtobmp(bool **pattern, Parameters p);
void        initbmpheaders(BITMAPFILEHEADER *bfh, BITMAPINFOHEADER *bih, int width, int height, int mult);
bgr_data    do_color(unsigned int color);


/*  FUNCTIONS  */

// outtobmp
// main function for outputting to bitmap file

int outtobmp(bool **pattern, Parameters p)
{
    if(p.width*p.mult <= MIN_BMP) {
        printf("ERROR: Bitmap too small\n");
        return 1;
    }

    FILE *fp;   // file pointer

    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;

    bgr_data Zero = do_color(p.bgcol);
    bgr_data One  = do_color(p.fgcol);

    initbmpheaders(&bfh, &bih, p.width, p.height, p.mult);    // initialize values in bmp headers

    // open file for writing (b flag forces binary writing, which will improve portability
    // of output image)
    if((fp = fopen(p.fname, "w")) == NULL) {
        printf("ERROR: Failed to create file %s\n", p.fname);
        return 1;
    }

    // write headers to bmp file
    // fwrite(ptr to elements to be written, size per element (bytes), # of elements, filestream)
    fwrite(&bfh, 1, sizeof bfh, fp);
    fwrite(&bih, 1, sizeof bih, fp);

    // draw out pixels. Bitmaps are drawn upside down for some reason (though not reversed)
    // so the y axis must start from the bottom up
    for(int y=p.height-1; y>=0; y--)
        for(int j=0; j<p.mult; j++)
            for(int x=0; x<p.width; x++) {
                bgr_data bgr;

                // set color to be used for pixel
                if(pattern[y][x]) bgr = One; else bgr = Zero;

                // place color into 3-byte BGR array
                unsigned char color[3] = { bgr.blue, bgr.green, bgr.red };

                // ...and write those bytes to the pixel!
                for(int i=0; i<p.mult; i++)
                    fwrite(color, 1, sizeof color, fp);
        }

    if(fclose(fp)) {
        printf("ERROR: Failed to close file stream\n");
        return 1;
    }

    return 0;
}


// initbmp
// initialize bitmap header values. Most of these are constants which is convenient

void initbmpheaders(BITMAPFILEHEADER *bfh, BITMAPINFOHEADER *bih, int width, int height, int mult)
{
    int img_size = width * height;

    int header_size = sizeof *bfh + sizeof *bih;    //should always equal 54

    int file_size = header_size + (img_size*4)*(mult^2);    // allocates 4 bytes per pixel plus size of
                                                            // file header for overall file size
    // initialize file header
	
	#if		defined(__unix__) || defined(__APPLE__)
		memcpy(bfh->bfType, "BM", 2);
	#elif	defined(_WIN32)
		bfh->bfType = 0x4d42;	// 0x42 = "B", 0x4d = "M"  
	#endif
    bfh->bfSize      = file_size;
    bfh->bfReserved1 = 0;
    bfh->bfReserved2 = 0;
    bfh->bfOffBits   = header_size;         // start drawing pixels at this byte

    // initialize info header
    bih->biSize          = sizeof *bih;
    bih->biWidth         = width * mult;
    bih->biHeight        = height * mult;
    bih->biPlanes        = 1;
    bih->biBitCount      = 24;              // 24 bit bitmap
    bih->biCompression   = 0;
    bih->biSizeImage     = file_size;
    bih->biXPelsPerMeter = PPM;
    bih->biYPelsPerMeter = PPM;
    bih->biClrUsed       = 0;
    bih->biClrImportant  = 0;
}


// do_color
// turns 6-digit hex code into BGR color struct for bitmap to use

bgr_data do_color(unsigned int color)
{
    bgr_data bgr;

    bgr.blue    = color & 0x0000FF;
    bgr.green   = (color & 0x00FF00) >> 8;
    bgr.red     = (color & 0xFF0000) >> 16;

    return bgr;
}
