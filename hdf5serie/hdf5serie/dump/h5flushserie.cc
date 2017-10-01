/* Copyright (C) 2014 Markus Friedrich
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

#include <config.h>
#include <cassert>
#include <cfenv>
#include <iostream>
#include <hdf5serie/file.h>
#include <boost/filesystem.hpp>

using namespace std;
using namespace H5;
using namespace boost::filesystem;

int main(int argc, char *argv[]) {
#ifndef _WIN32
  assert(feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW)!=-1);
#endif

  try {
    if(argc==1 || (argc==2 && argv[1]==string("-h"))) {
      cout<<"Usage: "<<argv[0]<<" [-h| <file> ...]"<<endl;
      cout<<endl;
      cout<<"Flush all HDF5Serie files given as arguments."<<endl;
      cout<<"Waits until all files are flushed successfully or a timeout occures."<<endl;
      cout<<"Outputs for each file provided as argument, in order, 0 or 1:"<<endl;
      cout<<"0 = no refresh of this file needed, since the flush was not successfull or no new data are available"<<endl;
      cout<<"1 = refresh this file, since new data is available"<<endl;
      return 0;
    }

    vector<shared_ptr<File> > h5File;
    for(int i=1; i<argc; ++i)
      h5File.push_back(make_shared<File>(argv[i], File::read));

    for(vector<shared_ptr<File> >::iterator it=h5File.begin(); it!=h5File.end(); ++it)
      (*it)->requestWriterFlush();
    vector<bool> refreshNeeded;
    refreshNeeded.reserve(h5File.size());
    for(vector<shared_ptr<File> >::iterator it=h5File.begin(); it!=h5File.end(); ++it)
      cout<<(*it)->waitForWriterFlush()<<endl;

    return 0;
  }
  catch(const std::exception &ex) {
    cout<<"Exception:"<<endl
        <<ex.what()<<endl;
    return 1;
  }
  catch(...) {
    cout<<"Unknown exception"<<endl;
    return 1;
  }
}
