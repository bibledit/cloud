#!/bin/bash

# Copyright (©) 2003-2025 Teus Benschop.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


# This script copies the Let's Encrypt certificates into the Bibledit Cloud config directory.
# It were better if Bibledit itself, on startup, could get the certificates.
# But this is not possible since Bibledit does not have enough privileges to read those certificates
# as they have been put by the certbot in the "live" directory.
# Hence this script does the job.


# Let script stop on an error.
set -e


if [ $EUID -gt 0 ]; then
  echo "Run this script as the root user"
  false
fi


CONFIGDIR=`dirname $0`
cd $CONFIGDIR
CONFIGDIR=`pwd`
echo The configuration directory is $CONFIGDIR


# Check whether the configuration directory looks right.
if [ "$(basename -- "$CONFIGDIR")" != "config" ]; then
  echo "Stop because of invalid configuration directory $CONFIGDIR"
  false
fi


# Read the host name from a file it was saved to in a previous run, if the file is there.
HOSTNAME=""
if [ -e hostname.txt ]; then
  HOSTNAME=$(cat hostname.txt)
fi


# If the hostname is not yet given, read it from a Bibledit setting.
if [ "$HOSTNAME" == "" ]; then
  if [ -e ../databases/config/general/site-url ]; then
      URL="$(cat ../databases/config/general/site-url)"
      prot="${URL%%:*}"
      link="${URL#$prot://}"
      domain="${link%%/*}"
      parts=(${domain//:/ })
      HOSTNAME=${parts[0]}
      unset URL
      unset prot
      unset link
      unset domain
      unset parts
  fi
fi


read -p "Enter the host name [$HOSTNAME]: " name
name=${name:-$HOSTNAME}
HOSTNAME=$name
unset name
echo Host name is $HOSTNAME


# Save the host name to file ready for the next run.
echo $HOSTNAME > hostname.txt


# Confirm the directory with Let's Encrypt certificates for the host name.
# Check it exists.
LIVE=/etc/letsencrypt/live/$HOSTNAME
read -p "Enter the directory where the certificates are [$LIVE]: " name
name=${name:-$LIVE}
LIVE=$name
unset name
echo The directory is $LIVE
if ! test -d "$LIVE"; then
    echo "Error: Directory $LIVE with the Let's Encrypt certificates for $HOSTNAME does not exist"
    false
fi


# Copy the certificates into place.
cp $LIVE/cert.pem .
cp $LIVE/chain.pem .
cp $LIVE/fullchain.pem .
cp $LIVE/privkey.pem .
chmod ugo+r *.pem


echo "The Let's Encrypt certificates for $HOSTNAME were copied into place"
echo Restart Bibledit Cloud to have it read the new certificates
echo In any case Bibledit Cloud will read the new certificates within 24 hours.

