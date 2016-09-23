#include <3ds.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include "common.h"
#include "../3ds_string_utils/source/string_utils.h"
#include "FilePane.h"

using namespace std;

int main(int argc, char **argv)
{
    gfxInitDefault();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// run once begin
    ///
    consoleInit(GFX_BOTTOM, &bottom);

    consoleInit(GFX_TOP, NULL);
    consoleInit(GFX_TOP, &leftpan);
    consoleInit(GFX_TOP, &rightpan);
    consoleInit(GFX_TOP, &console);
    consoleInit(GFX_TOP, &actions);
    consoleInit(GFX_TOP, &context);
    consoleInit(GFX_TOP, &all);

                    //           x   y  w   h
    consoleSetWindow(&menu,      0,  0, 50, 1);
    consoleSetWindow(&leftpan,   0,  1, 25, 26);
    consoleSetWindow(&rightpan, 25,  1, 25, 26);
    consoleSetWindow(&console,   0, 27, 50, 2);
    consoleSetWindow(&actions,   0, 29, 50, 1);
    consoleSetWindow(&context,   5, 1, 20, 20);

    chdir("/");
    FilePane leftFilePane(leftpan, "");
    FilePane rightFilePane(rightpan, "");
    FilePane* active;
    active = &leftFilePane;
    leftFilePane.setActive(true);


    ///
    /// run once end
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    while (aptMainLoop())
    {
        hidScanInput();
        u32 kHeld = 0;// hidKeysHeld();
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START) break; // break in order to return to hbmenu
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// run loop begin
        ///
        if ((kHeld | kDown) & KEY_DOWN){
            active->moveDown();
        }
        if ((kHeld | kDown) & KEY_UP){
            active->moveUp();
        }
        if ((kHeld | kDown) & KEY_DLEFT){
            active->moveTop();
        }
        if ((kHeld | kDown) & KEY_DRIGHT){
            active->moveEnd();
        }
        if (kDown & KEY_A){
            active->enter();
        }
        if (kDown & KEY_B){
            active->updir();
        }
        if (kDown & KEY_L){
            active = &leftFilePane;
            leftFilePane.setActive(true);
            rightFilePane.setActive(false);
        }
        if (kDown & KEY_R){
            active = &rightFilePane;
            rightFilePane.setActive(true);
            leftFilePane.setActive(false);
        }

        ///
        /// run loop end
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // Flush and swap framebuffers
        gfxFlushBuffers();
        gfxSwapBuffers();

        //Wait for VBlank
        gspWaitForVBlank();
    }

    // Exit services
    gfxExit();

    return 0;
}