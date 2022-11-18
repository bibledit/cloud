#!/bin/bash

#ls ../mbedtls/*.h > files.txt
#
#while read -r file; do
#
#  echo $file
#  mv $file tmpc
#  echo '#pragma GCC system_header' >> $file
#  cat tmpc >> $file
#  rm tmpc
#
#done < files.txt

ls ../mbedtls/*.c > files.txt
while read -r file; do
  echo $file
  mv $file tmpc
  echo '#pragma clang diagnostic ignored "-Wunknown-warning-option"' >> $file
  cat tmpc >> $file
  rm tmpc
done < files.txt

#find ../mimetic098 -name "*.cxx" > files.txt

#while read -r file; do
#
#  echo $file
#  mv $file tmpc
#  echo '#pragma clang diagnostic ignored "-Wunknown-warning-option"' >> $file
#  echo '#pragma clang diagnostic ignored "-Wimplicit-int-conversion"' >> $file
#  echo '#pragma clang diagnostic ignored "-Wsign-conversion"' >> $file
#  echo '#pragma GCC diagnostic ignored "-Weffc++"' >> $file
#  echo '#pragma GCC diagnostic ignored "-Wconversion"' >> $file
#  echo '#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"' >> $file
#  echo '#pragma GCC diagnostic ignored "-Wold-style-cast"' >> $file
#  echo '#pragma GCC diagnostic ignored "-Wsuggest-override"' >> $file
#  echo '#pragma GCC diagnostic ignored "-Wswitch-default"' >> $file
#  echo '#pragma GCC diagnostic ignored "-Wuseless-cast"' >> $file
#  echo '#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"' >> $file
#  echo '' >> $file
#  cat tmpc >> $file
#  rm tmpc
#
#done < files.txt

#find ../mimetic098 -name "*.h" > files.txt
#
#while read -r file; do
#
#  echo $file
#  mv $file tmpc
#  echo '#pragma GCC system_header' >> $file
#  echo '' >> $file
#  cat tmpc >> $file
#  rm tmpc
#
#done < files.txt

rm files.txt

