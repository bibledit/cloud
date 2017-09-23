/*
Copyright (©) 2003-2017 Teus Benschop.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


// System configuration.
#include <config.h>


// Basic C headers.
#include <cstdlib>
#ifndef HAVE_WINDOWS
#include <unistd.h>
#include <utime.h>
#include <dirent.h>
#endif
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>


// C headers in sub folders.
#include <sys/stat.h>
#ifndef HAVE_WINDOWS
#include <sys/time.h>
#endif
#ifdef HAVE_WINDOWS
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/param.h>
#endif
#include <sys/types.h>


// C++ headers.
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <algorithm>
#include <set>
#include <chrono>
#include <iomanip>
#include <stdexcept>
#include <thread>
#include <cmath>
#include <mutex>
#include <numeric>
#include <random>
#include <limits>


// Headers dependencies.
#include <sqlite/sqlite3.h>


// Bibledit configuration.
#include <config/logic.h>
#include <config/config.h>


using namespace std;
