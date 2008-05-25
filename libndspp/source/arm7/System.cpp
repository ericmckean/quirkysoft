/*
  Copyright (C) 2007,2008 Richard Quirk

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "System.h"
#include <nds.h>

VoidFunctionPointer nds::System::s_callback(0);
bool s_needToSleep(false);

void nds::System::recvSleepMessage()
{
  s_needToSleep = true;
}

static void lidOpenHandler()
{
  writePowerManagement(PM_CONTROL_REG,
      PM_SOUND_AMP |
      PM_BACKLIGHT_BOTTOM |
      PM_BACKLIGHT_TOP |
      PM_LED_ON);
}

static void arm7_fifo()
{
  while (!(REG_IPC_FIFO_CR & IPC_FIFO_RECV_EMPTY))
  {
    u32 msg = REG_IPC_FIFO_RX;
    if (msg == nds::System::SLEEP_MESSAGE)
    {
      nds::System::recvSleepMessage();
    }
  }
}


void nds::System::setupSleepWatchdog()
{
  irqSet(IRQ_FIFO_NOT_EMPTY, arm7_fifo);
  irqEnable(IRQ_FIFO_NOT_EMPTY);
  REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ;
}

void nds::System::checkSleep()
{
  // if the lid is down:
  if (s_needToSleep)
  {
    s_needToSleep = false;
    // make the light flicker, mostly off
    writePowerManagement(PM_CONTROL_REG, PM_LED_CONTROL(1));
    // save previous interrupts
    unsigned long oldIE = REG_IE;
    // remove current interrupts, set the lid open interrupt handler
    REG_IME = 0;
    irqSet(IRQ_LID, lidOpenHandler);
    irqDisable((IRQ_MASK)(oldIE));
    swiChangeSoundBias(0, 0x200);
    powerOFF(POWER_SOUND);
    // set the lid open interrupt on
    irqEnable(IRQ_LID);
    REG_IME = 1;

    if (s_callback)
    {
      s_callback();
    }

    swiSleep();

    // restore interrupts
    irqDisable(IRQ_LID);
    irqEnable((IRQ_MASK)(oldIE));

    // now sleep is over
    powerON(POWER_SOUND);
    swiChangeSoundBias(1, 0x200);
    REG_IPC_FIFO_TX = SLEEP_MESSAGE;
  }
}

void nds::System::registerSleepFunction(VoidFunctionPointer fn)
{
  s_callback = fn;
}
