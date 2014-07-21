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
 *   friedrich.at.gc@googlemail.com
 *
 */

#ifndef _HDF5SERIE_FILE_H_
#define _HDF5SERIE_FILE_H_

#include <hdf5serie/group.h>
#include <boost/filesystem.hpp>

namespace H5 {

  class Dataset;

  class File : public GroupBase {
    friend class Dataset;
    public:
      enum FileAccess {
        read,
        write
      };
      File(const boost::filesystem::path &filename, FileAccess type_);
      ~File();
      void reopenAsSWMR();
      static int getDefaultCompression() { return defaultCompression; }
      static void setDefaultCompression(int comp) { defaultCompression=comp; }
      static int getDefaultChunkSize() { return defaultChunkSize; }
      static void setDefaultChunkSize(int chunk) { defaultChunkSize=chunk; }
      void refresh();
      void flush();
      static void flushAllFiles(bool onlyIfRequrestedBySignal=false);
    protected:
      boost::filesystem::path pidFilename;
      pid_t writerPID;
      bool writerExists;
      FileAccess type;
      bool isSWMR;
      void close();
      void open();
      static int defaultCompression;
      static int defaultChunkSize;
      static std::set<File*> writerFiles;
      static void sigUsr2Handler(int);
      static bool flushAllFilesRequested;
  };
}

#endif
