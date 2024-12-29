/*
Copyright (©) 2003-2025 Teus Benschop.

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


#include <filter/memory.h>
#include <filter/url.h>
#include <filter/string.h>
#ifdef HAVE_MACH_MACH
#include <mach/mach.h>
#endif
#ifdef HAVE_EXECINFO
#include <execinfo.h>
#endif


// Returns the memory available as a percentage of the total system memory.
int filter_memory_percentage_available ()
{
#ifdef HAVE_MACH_MACH
  // macOS.
  struct vm_statistics64 stats;
  mach_port_t host = mach_host_self();
  natural_t count = HOST_VM_INFO64_COUNT;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
  host_statistics64 (host, HOST_VM_INFO64, (host_info64_t)&stats, &count);
#pragma GCC diagnostic pop
  uint64_t active = stats.active_count;
  uint64_t inactive = stats.inactive_count;
  uint64_t wired = stats.wire_count;
  uint64_t speculative = stats.speculative_count;
  uint64_t free = stats.free_count;
  uint64_t total = active + inactive + wired + speculative + free;
  return static_cast<int>(inactive + speculative + free) * 100 / static_cast<int> (total);

#else

  // BSD:
  // https://forums.freebsd.org/threads/38754/
  // http://stackoverflow.com/questions/2513505/how-to-get-available-memory-c-g
  
  // Linux.
  std::string path = "/proc/meminfo";
  if (file_or_dir_exists (path)) {
    std::string meminfo = filter_url_file_get_contents (path);
    size_t pos;
    int memtotal = 0;
    pos = meminfo.find ("MemTotal");
    if (pos != std::string::npos) {
      memtotal = filter::strings::convert_to_int (meminfo.substr (pos + 15));
    }
    int memfree = 0;
    pos = meminfo.find ("MemFree");
    if (pos != std::string::npos) {
      memfree = filter::strings::convert_to_int (meminfo.substr (pos + 15));
    }
    int cached = 0;
    pos = meminfo.find ("Cached");
    if (pos != std::string::npos) {
      cached = filter::strings::convert_to_int (meminfo.substr (pos + 15));
    }
    return (memfree + cached) * 100 / memtotal;
  }
#endif

  // Failed to get available memory: Return something sensible.
  return 50;
}


// Returns how many bytes of memory the app currently uses.
uint64_t filter_memory_total_usage () 
{
#ifdef HAVE_MACH_MACH
  // macOS.
  struct task_basic_info t_info;
  mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
  task_info (mach_task_self(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count);
#pragma GCC diagnostic pop
  // Total usage consists of resident and virtual memory size.
  uint64_t resident_memory = t_info.resident_size;
  // The resident memory is unrealistically high, so can be left out.
  //uint64_t virtual_memory = t_info.virtual_size;
  return resident_memory;
#endif
  return 0;
}


void filter_memory_print_back_trace ()
{
#ifdef HAVE_EXECINFO
  // https://stackoverflow.com/questions/3899870/print-call-stack-in-c-or-c
  // To add linker flag -rdynamic is essential.
  char **strings;
  void *array[1024];
  int size = backtrace(array, 1024);
  strings = backtrace_symbols(array, size);
  for (int i = 0; i < size; i++)
    std::cout << strings[i] << std::endl;
  puts("");
  free(strings);
#endif
}
