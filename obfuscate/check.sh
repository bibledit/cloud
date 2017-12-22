#!/bin/sh

# Checks for certain words whether they occur in the filenames.

# Do not look in the source code because it is to be removed after build.
find . -name "*bible*" ! -name "*.cpp" ! -name "*.h"
find . -name "*Bible*"
