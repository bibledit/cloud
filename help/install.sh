#!/bin/bash

# Copyright (©) 2003-2016 Teus Benschop.

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


SCRIPTPATH=`readlink -f "$0"`
echo Running script $SCRIPTPATH


# Some distro's cannot run $ su.
UNAME=`uname -a`
echo -n "Installing Bibledit on "
echo $UNAME
RUNSU=1;
echo "$UNAME" | grep -q Ubuntu
EXIT_CODE=$?
if [ $EXIT_CODE -eq 0 ]; then
RUNSU=0;
fi


# Create a script with commands to run with root privileges.
cat > install2.sh <<'scriptblock'

#!/bin/bash

clear
echo Updating the software sources...
which apt-get > /dev/null
if [ $? -eq 0 ]; then
apt-get update
fi

echo Installing the software Bibledit relies on...

which apt-get > /dev/null
if [ $? -eq 0 ]
then
echo Installing dependencies through apt-get...
# On Debian and derivates it is sufficient to use the --yes switch only.
# The package manager apt-get is also found on openSUSE, and there is also needs --assume-yes. 
apt-get --yes --assume-yes install build-essential
apt-get --yes --assume-yes install git
apt-get --yes --assume-yes install zip
apt-get --yes --assume-yes install pkgconf
apt-get --yes --assume-yes install libcurl4-openssl-dev
apt-get --yes --assume-yes install libssl-dev
apt-get --yes --assume-yes install libatspi2.0-dev
apt-get --yes --assume-yes install libgtk-3-dev
apt-get --yes --assume-yes install libwebkit2gtk-3.0-dev
apt-get --yes --assume-yes install libwebkit2gtk-4.0-dev
fi

# Fedora.
which dnf > /dev/null
if [ $? -eq 0 ]
then
echo Installing dependencies through dnf...
dnf --assumeyes install gcc-c++
dnf --assumeyes install git
dnf --assumeyes install zip
dnf --assumeyes install pkgconfig
dnf --assumeyes install libcurl-devel
dnf --assumeyes install openssl-devel
dnf --assumeyes install gtk3-devel
dnf --assumeyes install webkitgtk4-devel
fi

# CentOS
which yum > /dev/null
if [ $? -eq 0 ]
then
echo Installing dependencies through yum...
yum --assumeyes install gcc-c++
yum --assumeyes install git
yum --assumeyes install zip
yum --assumeyes install pkgconfig
yum --assumeyes install libcurl-devel
yum --assumeyes install openssl-devel
yum --assumeyes install gtk3-devel
yum --assumeyes install webkitgtk3-devel
yum --assumeyes install libwebkit2gtk-devel
fi

# openSUSE
which zypper > /dev/null
if [ $? -eq 0 ]
then
echo Installing dependencies through zypper...
zypper -n --non-interactive --no-gpg-checks install gcc-c++
zypper -n --non-interactive --no-gpg-checks install git
zypper -n --non-interactive --no-gpg-checks install zip
zypper -n --non-interactive --no-gpg-checks install pkg-config
zypper -n --non-interactive --no-gpg-checks install libcurl-devel
zypper -n --non-interactive --no-gpg-checks install libopenssl-devel
zypper -n --non-interactive --no-gpg-checks install cairo-devel
zypper -n --non-interactive --no-gpg-checks install gtk3-devel
zypper -n --non-interactive --no-gpg-checks install webkit2gtk3-devel
fi

echo Creating the script to start bibledit.
rm -f /usr/bin/bibledit
echo #!/bin/bash >> /usr/bin/bibledit
echo cd  >> /usr/bin/bibledit
echo cd bibledit >> /usr/bin/bibledit
echo './bibledit 2>&1 | grep -v WARNING | tr -d 012' >> /usr/bin/bibledit
chmod +x /usr/bin/bibledit

echo Install launcher to start bibledit.
# It does that here at this stage in the script, because here it has root privileges.
wget https://raw.githubusercontent.com/bibledit/linux/master/bibledit.desktop -O /usr/share/applications/bibledit.desktop
desktop-file-install /usr/share/applications/bibledit.desktop
wget https://raw.githubusercontent.com/bibledit/linux/master/bibledit.png -O /usr/share/icons/bibledit.png

# Act as if the script ran successfully, no matter whether it really did.
exit 0

scriptblock
# This is the end of the script to run with root privileges.


# Make the script executable to be ran with root privileges.
chmod +x install2.sh

# Conditionally run $ su.
if [ $RUNSU -ne 0 ]; then
echo Please provide the password for the root user and press Enter
su -c ./install2.sh
fi

EXIT_CODE=$?
# If $ su did not run, run $ sudo.
if [ $RUNSU -eq 0 ]; then
EXIT_CODE=1
fi
# If $ su ran, but failed, run $ sudo.
if [ $EXIT_CODE != 0 ]; then

echo Please provide the password for the administrative user and press Enter:
sudo ./install2.sh
EXIT_CODE=$?
if [ $EXIT_CODE != 0 ]; then
exit
fi

fi


# Remove the script with commands to run with root privileges.
rm install2.sh


# Remove any possible local launcher.
# The reason is that a local launcher takes precendence over a system-wide one.
cd
rm -f .local/share/applications/bibledit.desktop


cd
rm -f index.html
wget http://bibledit.org/linux -q -O index.html
if [ $? -ne 0 ]
then
echo Failed to list tarballs
exit
fi
cat index.html | grep "bibledit-" | grep -o '<a href=['"'"'"][^"'"'"']*['"'"'"]' | sed -e 's/^<a href=["'"'"']//' -e 's/["'"'"']$//' | tail -n 1 > tarball.txt
rm index.html
TARBALL=`cat tarball.txt`
rm tarball.txt
rm -f $TARBALL.*
wget --continue --tries=100 http://bibledit.org/linux/$TARBALL
if [ $? -ne 0 ]
then
echo Failed to download Bibledit
exit
fi

mkdir -p bibledit
tar xf $TARBALL -C bibledit --strip-components=1
if [ $? -ne 0 ]
then
echo Failed to unpack Bibledit
rm $TARBALL
exit
fi

cd bibledit
# Remove bits from any older build that might cause crashes in the new build.
find . -name "*.o" -delete
./configure
if [ $? -ne 0 ]
then
echo Failed to configure Bibledit
exit
fi
make clean
make --jobs=4
if [ $? -ne 0 ]
then
echo Failed to build Bibledit
exit
fi

# Remove the script, so people cannot reuse it.
# Reusing scripts have given problems in the past as newer scripts were different.
rm $SCRIPTPATH

echo If there were no errors, Bibledit should be working now.
echo --
echo To start Bibledit, open a terminal, and type:
echo bibledit
echo and press Enter.
