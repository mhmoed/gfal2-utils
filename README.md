# GFAL2 C++ wrapper and tools
**Installation requirements:**
 - [Boost 1.58.0](http://www.boost.org/users/history/version_1_58_0.html)
 - [GFAL2](https://dmc.web.cern.ch/projects/gfal-2/home) development files
 - A C++14-capable compiler

## Tools
### `gfal-find`
Command line tool to search files in SRM/GridFTP/etc. directory trees similar to the `find` command. Allow for searching by shell pattern (`--name`) and file type (`--type`), and provides a long-listing format with the `-l` or `--long` switches.
