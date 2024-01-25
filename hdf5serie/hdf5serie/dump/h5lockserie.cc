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

#include <config.h>
#include <clocale>
#include <cfenv>
#include <boost/program_options.hpp>
#include <iostream>
#include <hdf5serie/file.h>
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

using namespace std;
using namespace H5;
namespace po = boost::program_options;

int main(int argc, char* argv[]) {
#ifdef _WIN32
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
#else
  assert(feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW)!=-1);
#endif
  setlocale(LC_ALL, "C");

  try {
    // positional filename options (1 more more times
    po::positional_options_description posArg;
    posArg.add("filename", -1);
    // hidden options to store positional arguments
    po::options_description hiddenOpts("");
    hiddenOpts.add_options()
      ("filename", po::value<vector<string>>(), "")
    ;

    // available options
    po::options_description opts("Options");
    opts.add_options()
      ("help,h", "Produce this help message")
      ("dump"  , "Dump shared memory content (default if no other option given). !!!Note that the mutex is NOT locked for this operation!!!")
      ("remove", "Remove the shared memory !!!The shared memory is removed EVEN if it is used by any other process!!!")
    ;

    // parse arguments and store in vm
    po::options_description allOpts("");
    allOpts.add(opts).add(hiddenOpts);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(allOpts).positional(posArg).run(), vm);
    po::notify(vm);    
    
    // help text
    if(vm.count("help")) {
      cout<<"Apply action to the shared memory associated with a HDF5Serie HDF5 file."<<endl;
      cout<<"Required positional option:"<<endl;
      cout<<"  filename              The filename of the HDF5 file (can be given more than ones)"<<endl;
      cout<<opts<<endl;
      return 0;
    }
    
    // check options
    if(!vm.count("filename") || vm["filename"].as<vector<string>>().empty()) {
      cout<<"At least one positional option filename is required, see -h.\n";
      return 0;
    }
    if(vm.count("dump") && vm.count("remove")) {
      cout<<"The options --dump and --remove are mutally exclusive, see -h.\n";
      return 0;
    }

    // run given task
    for(auto &fn : vm["filename"].as<vector<string>>()) {
      try {
        if(vm.count("remove"))
          // remove shared memory
          File::removeSharedMemory(fn);
        else
          // dump shared memory
          File::dumpSharedMemory(fn);
      }
      catch(exception &ex) {
        cout<<"The following files created a exception:"<<endl;
        cout<<ex.what()<<endl;
        cout<<"filename: "<<fn<<endl;
      }
      catch(...) {
        cout<<"Unknown exception while checking the following file:"<<endl;
        cout<<"filename: "<<fn<<endl;
      }
    }
  }
  catch(exception &ex) {
    cerr<<ex.what()<<endl;
    return 1;
  }
  catch(...) {
    cerr<<"Unknown exception"<<endl;
    return 1;
  }

  return 0;
}
