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

#ifndef _HDF5SERIE_OPTIONS_H_
#define _HDF5SERIE_OPTIONS_H_

#include "file.h"

namespace H5 {

  struct Options {
    int fixedStrSize = -1;
    int compression = File::getDefaultCompression();
    int chunkSize = File::getDefaultChunkSize();
    int cacheSize = File::getDefaultCacheSize();
    Options& _fixedStrSize(int v) { fixedStrSize = v; return *this; }
    Options& _compression(int v) { compression = v; return *this; }
    Options& _chunkSize(int v) { chunkSize = v; return *this; }
    Options& _cacheSize(int v) { cacheSize = v; return *this; }
  };

}

#endif
