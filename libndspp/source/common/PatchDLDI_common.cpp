/*
  Copyright (C) 2008 Richard Quirk

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

  This file incorporates work covered by the following copyright and
  permission notice:

    dlditool - Dynamically Linked Disk Interface patch tool
    Copyright (C) 2006  Michael Chisholm (Chishm)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

*/
#include "libnds.h"
#include "PatchDLDI.h"
#include "File.h"

using nds::PatchDLDI;
using nds::File;

typedef signed int addr_t;
typedef unsigned char data_t;

#define FIX_ALL	    0x01
#define FIX_GLUE    0x02
#define FIX_GOT     0x04
#define FIX_BSS     0x08

enum DldiOffsets {
  DO_magicString = 0x00,          // "\xED\xA5\x8D\xBF Chishm"
  DO_magicToken = 0x00,           // 0xBF8DA5ED
  DO_magicShortString = 0x04,     // " Chishm"
  DO_version = 0x0C,
  DO_driverSize = 0x0D,
  DO_fixSections = 0x0E,
  DO_allocatedSpace = 0x0F,

  DO_friendlyName = 0x10,

  DO_text_start = 0x40,           // Data start
  DO_data_end = 0x44,             // Data end
  DO_glue_start = 0x48,           // Interworking glue start    -- Needs address fixing
  DO_glue_end = 0x4C,             // Interworking glue end
  DO_got_start = 0x50,            // GOT start                  -- Needs address fixing
  DO_got_end = 0x54,              // GOT end
  DO_bss_start = 0x58,            // bss start                  -- Needs setting to zero
  DO_bss_end = 0x5C,              // bss end

  // IO_INTERFACE data
  DO_ioType = 0x60,
  DO_features = 0x64,
  DO_startup = 0x68,
  DO_isInserted = 0x6C,
  DO_readSectors = 0x70,
  DO_writeSectors = 0x74,
  DO_clearStatus = 0x78,
  DO_shutdown = 0x7C,
  DO_code = 0x80
};

extern "C"
{
static addr_t readAddr (const data_t *mem, addr_t offset) {
  return ((addr_t*)mem)[offset/sizeof(addr_t)];
}

static void writeAddr (data_t *mem, addr_t offset, addr_t value) {
  ((addr_t*)mem)[offset/sizeof(addr_t)] = value;
}

/*
static void vramcpy (void* dst, const void* src, int len)
{
  u16* dst16 = (u16*)dst;
  u16* src16 = (u16*)src;

  for ( ; len > 0; len -= 2) {
    *dst16++ = *src16++;
  }
}
*/

static addr_t quickFind (const data_t* data, const data_t* search, size_t dataLen, size_t searchLen)
{
  const int* dataChunk = (const int*) data;
  int searchChunk = ((const int*)search)[0];
  addr_t i;
  addr_t dataChunkEnd = (addr_t)(dataLen / sizeof(int));

  for ( i = 0; i < dataChunkEnd; i++) {
    if (dataChunk[i] == searchChunk) {
      if ((i*sizeof(int) + searchLen) > dataLen) {
        return -1;
      }
      if (memcmp (&data[i*sizeof(int)], search, searchLen) == 0) {
        return i*sizeof(int);
      }
    }
  }

  return -1;
}

static const data_t dldiMagicString[] = "\xED\xA5\x8D\xBF Chishm";
#define DEVICE_TYPE_DLDI 0x49444C44
bool dldiPatchBinary (data_t *binData, u32 binSize, data_t * pDH)
{
  addr_t memOffset;        // Offset of DLDI after the file is loaded into memory
  addr_t patchOffset;      // Position of patch destination in the file
  addr_t relocationOffset; // Value added to all offsets within the patch to fix it properly
  addr_t ddmemOffset;      // Original offset used in the DLDI file
  addr_t ddmemStart;       // Start of range that offsets can be in the DLDI file
  addr_t ddmemEnd;         // End of range that offsets can be in the DLDI file
  addr_t ddmemSize;        // Size of range that offsets can be in the DLDI file

  addr_t addrIter;

  data_t *pAH;

  size_t dldiFileSize = 0;

  // Find the DLDI reserved space in the file
  patchOffset = quickFind (binData, dldiMagicString, binSize, sizeof(dldiMagicString));

  if (patchOffset < 0) {
    // does not have a DLDI section
    return false;
  }

  pAH = &(binData[patchOffset]);

  if (*((u32*)(pDH + DO_ioType)) == DEVICE_TYPE_DLDI) {
    // No DLDI patch
    return false;
  }

  if (pDH[DO_driverSize] > pAH[DO_allocatedSpace]) {
    // Not enough space for patch
    return false;
  }

  dldiFileSize = 1 << pDH[DO_driverSize];

  memOffset = readAddr (pAH, DO_text_start);
  if (memOffset == 0) {
    memOffset = readAddr (pAH, DO_startup) - DO_code;
  }
  ddmemOffset = readAddr (pDH, DO_text_start);
  relocationOffset = memOffset - ddmemOffset;
#if 0
  printf("Old driver:          %s\n", &pAH[DO_friendlyName]);
  printf("New driver:          %s\n", &pDH[DO_friendlyName]);
  printf("\n");
  printf("Position in file:    0x%08X\n", patchOffset);
  printf("Position in memory:  0x%08X\n", memOffset);
  printf("Patch base address:  0x%08X\n", ddmemOffset);
  printf("Relocation offset:   0x%08X\n", relocationOffset);
  printf("\n");
#endif
  ddmemStart = readAddr (pDH, DO_text_start);
  ddmemSize = (1 << pDH[DO_driverSize]);
  ddmemEnd = ddmemStart + ddmemSize;

  // Remember how much space is actually reserved
  pDH[DO_allocatedSpace] = pAH[DO_allocatedSpace];
  //data_t allocatedSpace = pAH[DO_allocatedSpace];
  // Copy the DLDI patch into the application
  memcpy (pAH, pDH, dldiFileSize);
  //pAH[DO_allocatedSpace] = allocatedSpace;

  // Fix the section pointers in the header
  writeAddr (pAH, DO_text_start, readAddr (pAH, DO_text_start) + relocationOffset);
  writeAddr (pAH, DO_data_end, readAddr (pAH, DO_data_end) + relocationOffset);
  writeAddr (pAH, DO_glue_start, readAddr (pAH, DO_glue_start) + relocationOffset);
  writeAddr (pAH, DO_glue_end, readAddr (pAH, DO_glue_end) + relocationOffset);
  writeAddr (pAH, DO_got_start, readAddr (pAH, DO_got_start) + relocationOffset);
  writeAddr (pAH, DO_got_end, readAddr (pAH, DO_got_end) + relocationOffset);
  writeAddr (pAH, DO_bss_start, readAddr (pAH, DO_bss_start) + relocationOffset);
  writeAddr (pAH, DO_bss_end, readAddr (pAH, DO_bss_end) + relocationOffset);
  // Fix the function pointers in the header
  writeAddr (pAH, DO_startup, readAddr (pAH, DO_startup) + relocationOffset);
  writeAddr (pAH, DO_isInserted, readAddr (pAH, DO_isInserted) + relocationOffset);
  writeAddr (pAH, DO_readSectors, readAddr (pAH, DO_readSectors) + relocationOffset);
  writeAddr (pAH, DO_writeSectors, readAddr (pAH, DO_writeSectors) + relocationOffset);
  writeAddr (pAH, DO_clearStatus, readAddr (pAH, DO_clearStatus) + relocationOffset);
  writeAddr (pAH, DO_shutdown, readAddr (pAH, DO_shutdown) + relocationOffset);

  // Put the correct DLDI magic string back into the DLDI header
  memcpy (pAH, dldiMagicString, sizeof (dldiMagicString));

  if (pDH[DO_fixSections] & FIX_ALL) {
    // Search through and fix pointers within the data section of the file
    for (addrIter = (readAddr(pDH, DO_text_start) - ddmemStart); addrIter < (readAddr(pDH, DO_data_end) - ddmemStart); addrIter++) {
      if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
        writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
      }
    }
  }

  if (pDH[DO_fixSections] & FIX_GLUE) {
    // Search through and fix pointers within the glue section of the file
    for (addrIter = (readAddr(pDH, DO_glue_start) - ddmemStart); addrIter < (readAddr(pDH, DO_glue_end) - ddmemStart); addrIter++) {
      if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
        writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
      }
    }
  }

  if (pDH[DO_fixSections] & FIX_GOT) {
    // Search through and fix pointers within the Global Offset Table section of the file
    for (addrIter = (readAddr(pDH, DO_got_start) - ddmemStart); addrIter < (readAddr(pDH, DO_got_end) - ddmemStart); addrIter++) {
      if ((ddmemStart <= readAddr(pAH, addrIter)) && (readAddr(pAH, addrIter) < ddmemEnd)) {
        writeAddr (pAH, addrIter, readAddr(pAH, addrIter) + relocationOffset);
      }
    }
  }
  if (pDH[DO_fixSections] & FIX_BSS) {
    // Initialise the BSS to 0
    memset (&pAH[readAddr(pDH, DO_bss_start) - ddmemStart] , 0, readAddr(pDH, DO_bss_end) - readAddr(pDH, DO_bss_start));
  }

  return true;
}
};

namespace nds
{
  class PatchDLDI::PatchDLDI_Impl
  {
    public:
      PatchDLDI_Impl(const char * filename)
      {
        // get the DLDI section of this file.
        m_file.open(filename, "r+");
      }

      bool patch()
      {
        // we only have a smallish buffer to play with.
        if (m_file.is_open())
        {
          // find the position of the patch area, and situate it so that there is enough space either side
          const unsigned char * p = PatchDLDI::dldiPatch();
          if (not p)
            return 0;
          unsigned short * dst = PatchDLDI::buffer();
          addr_t pos;
          int r;
          do {
            FILE * fp = (FILE*)m_file.file();
            int where = ftell(fp);
            static const int bufferSize(1024*128);
            r = m_file.read((char*)dst, bufferSize);
            pos = quickFind((const data_t*)dst, dldiMagicString, r, sizeof(dldiMagicString));
            if (pos >= 0)
            {
              if ((pos + p[DO_driverSize]) > bufferSize)
              {
                // oops - the data is going to fall off the end.
                // rewind a bit and retry
                int offset = pos - p[DO_driverSize];
                if (where < offset)
                {
                  nds::PatchDLDI::freeBuffer(dst);
                  return false;
                }
                fseek(fp, where - offset, SEEK_SET);
                r = m_file.read((char*)dst, bufferSize);
              }

              if (dldiPatchBinary((data_t*)dst, r, (data_t*)p))
              {
                // copy dst back into the file.
                fseek(fp, where, SEEK_SET);
                m_file.write((const char*)dst, r);
                m_file.close();
                nds::PatchDLDI::freeBuffer(dst);
                return true;
              }
              else
              {
                nds::PatchDLDI::freeBuffer(dst);
                return false;
              }
            }
          }
          while (r > 0 and pos < 0);
          nds::PatchDLDI::freeBuffer(dst);
        }

        return false;
      }

    private:
      File m_file;
  };
}

PatchDLDI::PatchDLDI(const char * filename):
  m_impl(new PatchDLDI_Impl(filename))
{
}

bool PatchDLDI::patch()
{
  return m_impl->patch();
}
