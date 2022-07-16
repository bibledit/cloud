#!/bin/bash

cd ../mbedtls
if [ $? -ne 0 ]; then exit; fi

for file in *.c; do
  echo $file
  mv $file tmpc
  echo '#pragma clang diagnostic ignored "-Wimplicit-int-conversion"' >> $file
  echo '#pragma clang diagnostic ignored "-Wsign-conversion"' >> $file
  cat tmpc >> $file
  rm tmpc
done

