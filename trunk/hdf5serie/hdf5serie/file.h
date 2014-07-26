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
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/shared_ptr.hpp>

namespace boost {
  namespace interprocess {
    class named_semaphore;
    class named_mutex;
    class named_condition;
  }
}

namespace H5 {

  class Dataset;

  class File : public GroupBase {
    friend class Dataset;
    friend class GroupBase;
    public:
      enum FileAccess {
        read,
        write
      };
      File(const boost::filesystem::path &filename, FileAccess type_);
      ~File();
      void reopenAsSWMR();
      static void reopenAllFilesAsSWMR();
      static int getDefaultCompression() { return defaultCompression; }
      static void setDefaultCompression(int comp) { defaultCompression=comp; }
      static int getDefaultChunkSize() { return defaultChunkSize; }
      static void setDefaultChunkSize(int chunk) { defaultChunkSize=chunk; }
      void refresh();
      void flush();

      void flushIfRequested();
      static void flushAllFiles();
      static void flushAllFilesIfRequested();
      void refreshAfterWriterFlush();
      static void refreshAllFiles();
      static void refreshAllFilesAfterWriterFlush();

      struct IPC {
        boost::filesystem::path filename;
        boost::shared_ptr<boost::interprocess::named_semaphore> sem;
        boost::shared_ptr<boost::interprocess::named_mutex> mutex;
        boost::shared_ptr<boost::interprocess::named_condition> cond;
        boost::posix_time::ptime flushRequestTime;
      };
    protected:
      FileAccess type;
      bool isSWMR;
      void close();
      void open();
      static int defaultCompression;
      static int defaultChunkSize;

      static std::set<File*> writerFiles;
      static std::set<File*> readerFiles;
      void requestWriterFlush();
      bool waitForWriterFlush();
      std::string interprocessBasename;
      IPC ipc;
      void addFileToNotifyOnRefresh(const boost::filesystem::path &filename);
      std::vector<IPC> ipcAdd;
  };
}

#endif
