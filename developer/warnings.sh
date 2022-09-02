#!/bin/bash

#ls ../mbedtls/*.c > files.txt
#
#while read -r file; do
#
#  echo $file
#  mv $file tmpc
#  echo '#pragma clang diagnostic ignored "-Wimplicit-int-conversion"' >> $file
#  echo '#pragma clang diagnostic ignored "-Wsign-conversion"' >> $file
#  echo '#pragma GCC diagnostic ignored "-Wconversion"' >> $file
#  cat tmpc >> $file
#  rm tmpc
#
#done < files.txt

find ../mimetic098 -name "*.cxx" > files.txt

while read -r file; do

  echo $file
  mv $file tmpc
  echo '#pragma clang diagnostic ignored "-Wimplicit-int-conversion"' >> $file
  echo '#pragma clang diagnostic ignored "-Wsign-conversion"' >> $file
  echo '#pragma GCC diagnostic ignored "-Weffc++"' >> $file
  echo '#pragma GCC diagnostic ignored "-Wconversion"' >> $file
  echo '#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"' >> $file
  echo '#pragma GCC diagnostic ignored "-Wold-style-cast"' >> $file
  echo '#pragma GCC diagnostic ignored "-Wsuggest-override"' >> $file
  echo '#pragma GCC diagnostic ignored "-Wswitch-default"' >> $file
  echo '' >> $file
  cat tmpc >> $file
  rm tmpc

done < files.txt

rm files.txt

