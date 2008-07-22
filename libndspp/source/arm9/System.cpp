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
#include <nds/system.h>

VoidFunctionPointer nds::System::s_callback(0);
bool s_awoke = false;

void nds::System::recvSleepMessage()
{
  s_awoke = true;
}

static void sleep_fifo()
{
  while (!(REG_IPC_FIFO_CR & IPC_FIFO_RECV_EMPTY))
  {
    u32 value = REG_IPC_FIFO_RX;
    if (value == nds::System::SLEEP_MESSAGE)
      nds::System::recvSleepMessage();
  }
}

void nds::System::setupSleepWatchdog()
{
    irqSet(IRQ_FIFO_NOT_EMPTY, sleep_fifo);
    irqEnable(IRQ_FIFO_NOT_EMPTY);
    REG_IPC_FIFO_CR = IPC_FIFO_SEND_CLEAR | IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ;
}

void nds::System::checkSleep()
{
  if (keysHeld() & KEY_LID)
  {
    powerOFF(POWER_LCD);
    unsigned long oldIE = REG_IE;
    // remove current interrupts
    irqDisable((IRQ_MASK)oldIE);
    REG_IME = 0;
    if (s_callback)
    {
      // switch off Wifi, etc.
      s_callback();
    }
    s_awoke = false;
    // tell the arm7 to snooze. it'll send a new interrupt to wake us up
    REG_IPC_FIFO_CR = IPC_FIFO_SEND_CLEAR | IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ;
    REG_IPC_FIFO_TX = SLEEP_MESSAGE;
    irqEnable(IRQ_FIFO_NOT_EMPTY);
    while (!s_awoke)
      swiWaitForIRQ();
    // wait for vblank before powering on the LCD
    while (REG_VCOUNT!=0);
    while (REG_VCOUNT==0);
    while (REG_VCOUNT!=0);
    powerON(POWER_LCD);
    irqEnable((IRQ_MASK)(oldIE));
  }
}

const char * nds::System::uname()
{
  return "Nintendo DS";
}

int nds::System::language()
{
  return PersonalData->_user_data.language;
}

void nds::System::registerSleepFunction(VoidFunctionPointer fn)
{
  s_callback = fn;
}
