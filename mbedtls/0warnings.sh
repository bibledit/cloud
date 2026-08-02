#!/bin/bash

ls ./*.h > files.txt
while read -r file; do
  echo "$file"
  mv "$file" tmpc
  echo '#pragma GCC system_header' >> "$file"
  cat tmpc >> "$file"
  rm tmpc
done < files.txt

ls ./*.c > files.txt
while read -r file; do
  echo "$file"
  mv "$file" tmpc
  echo '#pragma clang diagnostic ignored "-Wunknown-warning-option"' >> "$file"
  echo '#pragma GCC diagnostic ignored "-Wuseless-cast"' >> "$file"
  cat tmpc >> "$file"
  rm tmpc
done < files.txt

rm files.txt

