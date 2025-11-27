
# Script to generate one bitmap for every rule of the Wolfram cellular automaton

import subprocess   # for running Wolfram in command line with parameters
import random       # for RNG

############################
### FUNCTION DEFINITIONS ###
############################


# color_hex
# converts dict of 3 color values into hex string

def color_hex(vals):
    
    color = ""
    for i in vals.values():
        color = color + hex(i).lstrip("0x")
        
    return "x"+color


# rand_cols
# generates random color hex strings

def rand_cols():

    # assign foreground and background values to dictionaries
    fgvals = {
        "red" : random.randint(0,256),
        "green" : random.randint(0,256),
        "blue" : random.randint(0,256)
    }
    bgvals = {
        "red" : random.randint(0,256),
        "green" : random.randint(0,256),
        "blue" : random.randint(0,256)
    }
    
    # ensure (loosely) that foreground color is lighter
    if sum(fgvals.values() ) < sum(bgvals.values() ):
        temp = fgvals.copy()
        fgvals = bgvals.copy()
        bgvals = temp.copy()
    
    # convert dictionaries to hex strings    
    fgcol = color_hex(fgvals)
    bgcol = color_hex(bgvals)
    
    return fgcol, bgcol


# cga_cols
# choose a random CGA color for foreground and background

def cga_cols():
    
    col_list = ["black", "darkgray", "darkblue", "darkred", "darkpurple", "darkgreen", "brown", "darkcyan", 
                "blue", "purple", "red", "green", "gray", "cyan", "yellow", "white"]
    
    fgcol = ""
    bgcol = ""
    
    while fgcol == bgcol:
        fgcol = random.choice(col_list)
        bgcol = random.choice(col_list)
    
    return fgcol, bgcol


#####################
### MAIN FUNCTION ###
#####################

# Getting user input

width = int(input("Width of swatch:"))
if width < 0 or width > 2000:
    print("ERROR: Width out of range")
    quit()

height = int(input("Height of swatch:"))
if height < 0 or height > 2000:
    print("ERROR: Height out of range")
    quit()

stretch = int(input("Stretch value:"))
if stretch < 0 or stretch > 8:
    print("ERROR: Stretch out of range")
    quit()
    
# loop through rules 0 to 255
for i in range(256):

    fname = "wolf_swatch_" + str(i).zfill(3)    # generate filename with rule number appended
    
    #fgcol, bgcol = rand_cols()
    fgcol, bgcol = cga_cols()

    # attempt to actually run Wolfram using above parameters
    try:
        subprocess.call(["./wolfram", "-Q", "-n", "-R", "-o","bmp", "-s",str(stretch), "-w",str(width), "-h",str(height), "-r",str(i), "-fg",fgcol, "-bg",bgcol, "-f",fname])
    except:
        print("\nERROR: Failed to execute wolfram. Aborting.")
        break

