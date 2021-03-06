#include <stdio.h>
#include "position_locator.hpp"

#if !FPGA
#include <iostream>
#include <fstream>
#include <cstdlib>
using namespace std;
fstream infile("cue_rawPosition.txt", ios_base::in);
#endif

#if FPGA
#include "xparameters.h"
volatile unsigned int *position_locator = (unsigned int*) XPAR_POSITION_LOCATOR_0_S00_AXI_BASEADDR;
#endif

#if !POS_LOC_IGNORE

void initPositionLocator()
{
    #if FPGA
	// (MAX_RED, MIN_RED, MAX_GREEN, MIN_GREEN, MAX_BLUE, MIN_BLUE)
    position_locator[1] = 0x00ff7070;
    #endif
}


unsigned int pollPositionLocator()
{
    unsigned int position_value;
    do {
        #if FPGA
        position_value = position_locator[0];
        #elif !FPGA
        // read position from textfile
        if (!infile.eof()) {
            infile >> std::hex >> position_value;
        } else {
            std::cout << "End of file reached" << std::endl;
            std::exit(1);
        }

        std::cout << "Reading HexValue: " << std::hex << position_value << std::endl;
        #endif
    } while (!POSLOC_isValid(position_value));

    return position_value;
}

bool positionIsOnTable(int pos_x, int pos_y)
{
    #if !FPGA
    cout << "Is on table: " <<
      ( pos_x >= POSLOC_tableTop_X &&
	    pos_x <= POSLOC_tableBot_X &&
	    pos_y >= POSLOC_tableTop_Y &&
	    pos_y <= POSLOC_tableBot_Y) << std::endl;
    #endif
    return( pos_x >= POSLOC_tableTop_X &&
	    pos_x <= POSLOC_tableBot_X &&
	    pos_y >= POSLOC_tableTop_Y &&
	    pos_y <= POSLOC_tableBot_Y);
}

void convertCameraCordinateToTable(int *x, int *y)
{
    // The scale factor is shifted up by 5 bits,
    // Shift the result down by 5 after multiplying.
    *x = ((*x-POSLOC_tableTop_X) * POSLOC_tableScale_X) >> POSLOC_shiftRequired;
    *y = ((*y-POSLOC_tableTop_Y) * POSLOC_tableScale_Y) >> POSLOC_shiftRequired;
    #if !FPGA
    cout << "converted x position: " << std::dec << *x << std::endl;
    cout << "converted y position: " << std::dec << *y << std::endl;
    #endif

    // The position locator treats the top as y=0, while the
    //	physics library treats the top as y=480*NORMAL.
    //	Therefore, flip the value.
    //	NOT NECESSARY? (if the tft draws it flipped again)
    //*y = NORTH_WALL - *y;
}

#endif  // POS_LOC_IGNORE
