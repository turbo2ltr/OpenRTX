/***************************************************************************
 *   Copyright (C) 2022 by Federico Amedeo Izzo IU2NUO,                    *
 *                         Niccolò Izzo IU2KIN                             *
 *                         Frederik Saraci IU2NRO                          *
 *                         Silvano Seva IU2KWO                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include <interfaces/filesystem.h>
#include <interfaces/platform.h>
#include <interfaces/graphics.h>
#include <interfaces/delays.h>
#include <interfaces/cps_io.h>
#include <interfaces/gps.h>
#include <threads.h>
#include <openrtx.h>
#include <ui.h>
#include <backup.h>

extern void *dev_task(void *arg);

void openrtx_init()
{
    state.devStatus = STARTUP;

    platform_init();    // Initialize low-level platform drivers
    state_init();       // Initialize radio state
    filesystem_init();  // Initialize filesystem

    gfx_init();         // Initialize display and graphics driver
    kbd_init();         // Initialize keyboard driver
    ui_init();          // Initialize user interface
    #ifdef SCREEN_CONTRAST
    display_setContrast(state.settings.contrast);
    #endif

    // Load codeplug from nonvolatile memory, create a new one in case of failure.
    if(cps_open(NULL) < 0)
    {
        cps_create(NULL);
        if(cps_open(NULL) < 0)
        {
            // Unrecoverable error
            #ifdef PLATFORM_LINUX
            exit(-1);
            #else
            // TODO: implement error handling for non-linux targets
            while(1) ;
            #endif
        }
    }

    // Display splash screen, turn on backlight after a suitable time to
    // hide random pixels during render process
    ui_drawSplashScreen(true);
    gfx_render();
    sleepFor(0u, 30u);
    platform_setBacklightLevel(state.settings.brightness);

    #if defined(GPS_PRESENT)
    // Detect and initialise GPS
    state.gpsDetected = gps_detect(1000);
    if(state.gpsDetected) gps_init(9600);
    #else
    // Keep the splash screen for 1 second
    sleepFor(1u, 0u);
    #endif
}

#if !defined(PLATFORM_LINUX) && !defined(PLATFORM_MOD17)
void _openrtx_backup()
{
    ui_drawBackupScreen();
    gfx_render();
    // Wait 30ms before turning on backlight to hide random pixels on screen
    sleepFor(0u, 30u);
    platform_setBacklightLevel(state.settings.brightness);
    // Wait for backup over serial xmodem
    eflash_dump();
    // Backup completed: Ask user confirmation for flash initialization
    ui_drawFlashInitScreen();
    gfx_render();
    while(true)
    {
        if(platform_getPttStatus() == true)
        {
            int err = filesystem_format();
            // Flash init completed: reboot
            NVIC_SystemReset();
            break;
        }
        if(platform_pwrButtonStatus() == false)
        {
            // User wants to quit: shutdown
            break;
        }
    }
    // Shutdown
    platform_terminate();
    NVIC_SystemReset();
}
#endif

void *openrtx_run()
{
    state.devStatus = RUNNING;

    #if !defined(PLATFORM_LINUX) && !defined(PLATFORM_MOD17)
    // If filesystem initialization failed, enter backup mode (sink state)
    if(state.filesystem_ready == false)
        _openrtx_backup();
    #endif

    // Start the OpenRTX threads
    create_threads();

    // Jump to the device management task
    dev_task(NULL);

    // Device task terminated, complete shutdown sequence
    state_terminate();
    platform_terminate();

    return NULL;
}
