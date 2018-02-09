// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once
#include <chrono>
#include <iostream>
#include <string>

#define tic(x) auto x = std::chrono::steady_clock::now()
#define toc(x) std::chrono::duration <double, milli> (std::chrono::steady_clock::now() - x).count()

/// Helper class for TICTOC_SCOPE and TICTOC_BLOCK
class TicTocTimerObject{
private:
    std::chrono::time_point<std::chrono::system_clock> _start;
    std::string _str;
public:
    int i=0;
    TicTocTimerObject(const std::string& str) : _str(str){
        _start = std::chrono::system_clock::now();
    }
    ~TicTocTimerObject(){ 
        double t_ms = std::chrono::duration <double, std::milli> (std::chrono::system_clock::now() - _start).count();
        std::cout << "[" << _str << "] " << t_ms << " ms" << std::endl; 
    }
};

/// Usage:
/// 
/// {
///     ///.... Code to be timed
///     TICTOC_SCOPE(objname, "scope name");
/// } //< prints out [scope name] XXms
/// 
#define TICTOC_SCOPE(objname, string) TicTocTimerObject objname(string)

/// Usage (nothing internally it is just a for loop!):
/// 
/// TIMED_BLOCK(objname, "output string")
/// {
///     //
///     //... Code to be timed
///     //
/// } //< prints out [scope name] XXms
/// 
#define TICTOC_BLOCK(obj, blockName) for (TicTocTimerObject obj(blockName); obj.i < 1; ++obj.i)
