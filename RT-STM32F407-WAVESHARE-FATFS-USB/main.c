/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "ch_test.h"

#include "chprintf.h"
#include "shell.h"

#include "usb_msd.h"
#include "ff.h"



/*===========================================================================*/
/* FatFs related.                                                            */
/*===========================================================================*/

/**
 * @brief FS object.
 */
static FATFS SDC_FS;

/* FS mounted and ready.*/
static bool fs_ready = FALSE;

#define RED_ON palClearPad(GPIOF, GPIOF_PIN8);






/*
 * LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palTogglePad(GPIOF, GPIOF_PIN9);
    chThdSleepMilliseconds(fs_ready ? 125 : 500);
  }
}

uint8_t mmc_scrachpad[512];
SDCConfig cfg =
{
 mmc_scrachpad,
 SDC_MODE_4BIT
};

/*
 * Application entry point.
 */
int main(void)
{

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   * - lwIP subsystem initialization using the default configuration.
   */
  halInit();
  chSysInit();

  /*
   * Activates the serial driver 6 and SDC driver 1 using default
   * configuration.
   */
  sdStart(&SD6, NULL);
  /*
   * SD card insertion monitor.
   */

  sdcStart(&SDCD1, &cfg);
  if (sdcConnect(&SDCD1) == HAL_FAILED)
  {
    RED_ON;
  };

  FRESULT err = f_mount(&SDC_FS, "/", 1);
  if (err != FR_OK)
  {
    RED_ON;
    fs_ready = FALSE;
  }
  else
  {
    fs_ready = true;
  }

  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);


  init_msd_driver();


  /*
   * Normal main() thread activity, handling SD card events and shell
   * start/exit.
   */

  while (true)
  {

    chThdSleepMilliseconds(500);

  }
}
