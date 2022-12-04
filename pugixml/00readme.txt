Website: https://pugixml.org
Repository: https://github.com/zeux/pugixml

When updating the embedded pugixml code,
be sure to transfer the added code from the old to the new.

This refers to:
1. Warnings suppressions via #pragma's.

2. System config:
// System configuration to check if the system has pugixml.
#include <config.h>
#ifndef HAVE_PUGIXML
...
#endif

