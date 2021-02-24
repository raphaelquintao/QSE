## QSE
Search Engine written for academic purpose.

* Implemented.
    * Indexer
    * Search
    * Web Interface
* Not fully implemented.
    * Nothing!
    
---

## Requirements
* C++11 - MINGW64(Windows), CLANG(MacOS) or GCC(Linux)
* [Cmake](http://www.cmake.org/)
* [Boost](http://www.boost.org/)
* [cpp-netlib](http://cpp-netlib.org/)
* [Htmlcxx](http://htmlcxx.sourceforge.net)
* [Tidy-HTML5](https://github.com/htacg/tidy-html5)


## Usage - Indexer
`QSE-Indexer.exe -d foo/bar/ -n corpus [-s] [-a] [-r] [-t] [-v] [-h]`

Parameter | Description |
----------:|:-----------|
   -d  | Collection base directory
   -n  | Database name
   -s  | Max raw data read before flush, default 10MB.
   -a  | Indicates that directory contains raw html files, do not set this option means that directory contains data in binary file and contains index.txt as reference.
   -r  | Read collection recursively, use only if using `-a`
   -t  | Use Tidy as HTML parser, drastically increase time and memory usage. Default HTMLCXX, faster but worse.
   -v  | Version and About
   -h  | Help 