/* Copyright (C) 2009 Markus Friedrich
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact:
 *   mafriedrich@users.berlios.de
 *
 */

#ifndef _HDF5SERIE_FILESERIE_H_
#define _HDF5SERIE_FILESERIE_H_

#include <H5Cpp.h>
#include <list>

namespace H5 {

  class FileSerie : public H5File {
    private:
      static std::list<FileSerie*> openedFile;
      static bool flushOnes;
      static void sigUSR2Handler(int);
      static int defaultCompression;
      static int defaultChunkSize;
    public:
      FileSerie(const char *name, unsigned int flags,
                const FileCreatPropList &create_plist=FileCreatPropList::DEFAULT,
                const FileAccPropList &access_plist=FileAccPropList::DEFAULT);
      FileSerie(const H5std_string &name, unsigned int flags,
                const FileCreatPropList &create_plist=FileCreatPropList::DEFAULT,
                const FileAccPropList &access_plist=FileAccPropList::DEFAULT);
      ~FileSerie();
      void close();
      void openFile(const H5std_string &name, unsigned int flags, const FileAccPropList &access_plist=FileAccPropList::DEFAULT);
      void openFile(const char *name, unsigned int flags, const FileAccPropList &access_plist=FileAccPropList::DEFAULT);
      static void flushAllFiles();
      static bool getFlushOnes() { return flushOnes; }
      static void deletePIDFiles();

      static int getDefaultCompression() { return defaultCompression; }
      static void setDefaultCompression(int comp) { defaultCompression=comp; }
      static int getDefaultChunkSize() { return defaultChunkSize; }
      static void setDefaultChunkSize(int chunk) { defaultChunkSize=chunk; }
  };
}

#endif
