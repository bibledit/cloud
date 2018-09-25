#!/bin/bash

# Copyright (©) 2003-2017 Teus Benschop.

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


# Definitions for what to obfuscate.
# Bibledit=Scripturedit
# bibledit=scripturedit
# Bible=Scripture
# bible=scripture


# Do not update anything below this line.


if [ "$Bibledit" = "" ]
then
Bibledit=Bibledit
fi
if [ "$bibledit" = "" ]
then
bibledit=bibledit
fi
if [ "$Bible" = "" ]
then
Bible=Bible
fi
if [ "$bible" = "" ]
then
bible=bible
fi


SCRIPTPATH=`readlink -f "$0"`
echo Running script $SCRIPTPATH


# Some distro's cannot run $ su.
UNAME=`uname -a`
echo -n "Installing ${Bibledit} on "
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

# Obfuscation information.
Bibledit=$1
bibledit=$2
Bible=$3
bible=$4

echo Updating the software sources...
which apt > /dev/null
if [ $? -eq 0 ]; then
apt update
fi

# One could think of running $ apt upgrade now.
# But this could lead to more problems than it would resolve...
# So, the script does not run that.

echo Installing the software ${Bibledit} relies on...

which apt > /dev/null
if [ $? -eq 0 ]
then
echo Installing dependencies through apt...
# On Debian and derivates it is sufficient to use the --yes switch only.
# The package manager apt is also found on openSUSE, and there is also needs --assume-yes.
apt --yes --assume-yes install build-essential
apt --yes --assume-yes install autoconf
apt --yes --assume-yes install automake
apt --yes --assume-yes install autoconf-archive
apt --yes --assume-yes install git
apt --yes --assume-yes install zip
apt --yes --assume-yes install pkgconf
apt --yes --assume-yes install libcurl4-openssl-dev
apt --yes --assume-yes install libssl-dev
apt --yes --assume-yes install libatspi2.0-dev
apt --yes --assume-yes install libgtk-3-dev
apt --yes --assume-yes install libwebkit2gtk-3.0-dev
apt --yes --assume-yes install libwebkit2gtk-4.0-dev
fi

# Fedora.
which dnf > /dev/null
if [ $? -eq 0 ]
then
echo Installing dependencies through dnf...
dnf --assumeyes install autoconf
dnf --assumeyes install automake
dnf --assumeyes install autoconf-archive
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
yum --assumeyes install autoconf
yum --assumeyes install automake
yum --assumeyes install autoconf-archive
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
zypper -n --non-interactive --no-gpg-checks install autoconf
zypper -n --non-interactive --no-gpg-checks install automake
zypper -n --non-interactive --no-gpg-checks install autoconf-archive
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


echo Creating the script to start ${bibledit}
rm -f /usr/bin/${bibledit}
echo #!/bin/bash >> /usr/bin/${bibledit}
echo cd  >> /usr/bin/${bibledit}
echo cd ${bibledit} >> /usr/bin/${bibledit}
echo "./${bibledit} 2>&1 | grep -v WARNING | tr -d 012" >> /usr/bin/${bibledit}
chmod +x /usr/bin/${bibledit}

echo Install launcher to start ${Bibledit}
# It does that here at this stage in the script, because here it has root privileges.
wget https://raw.githubusercontent.com/bibledit/linux/master/bibledit.desktop -O /usr/share/applications/${bibledit}.desktop
echo Updating ${bibledit}.desktop
# The .desktop file has the following lines, among others:
# Name=Bibledit
# Comment=Bible Editor
# Exec=bibledit
# Update those.
sed -i.bak "s/Bibledit/${Bibledit}/g" /usr/share/applications/${bibledit}.desktop
if [ $? -ne 0 ]; then exit; fi
sed -i.bak "s/Bible/${Bible}/g" /usr/share/applications/${bibledit}.desktop
if [ $? -ne 0 ]; then exit; fi
sed -i.bak "s/bibledit/${bibledit}/g" /usr/share/applications/${bibledit}.desktop
if [ $? -ne 0 ]; then exit; fi
rm /usr/share/applications/*.bak
# Install it.
desktop-file-install /usr/share/applications/${bibledit}.desktop
wget https://raw.githubusercontent.com/bibledit/linux/master/bbe512x512.png -O /usr/share/icons/bbe512x512.png

# Act as if the script ran successfully, no matter whether it really did.
exit 0

scriptblock
# This is the end of the script to run with root privileges.


# Make the script executable to be ran with root privileges.
chmod +x install2.sh

# Conditionally run $ su.
if [ $RUNSU -ne 0 ]; then
echo Please provide the password for the root user and press Enter
su -c ./install2.sh -- -- "${Bibledit}" "${bibledit}" "${Bible}" "${bible}"
fi

EXIT_CODE=$?
# If $ su did not run, run $ sudo.
if [ $RUNSU -eq 0 ]; then
EXIT_CODE=1
fi
# If $ su ran, but failed, run $ sudo.
if [ $EXIT_CODE != 0 ]; then

echo Please provide the password for the administrative user and press Enter:
sudo ./install2.sh "${Bibledit}" "${bibledit}" "${Bible}" "${bible}"
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
rm -f .local/share/applications/${bibledit}.desktop


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


# Create the directory and unpack the tarball there.
mkdir -p ${bibledit}
tar xf $TARBALL -C ${bibledit} --strip-components=1
if [ $? -ne 0 ]
then
echo Failed to unpack ${Bibledit}
rm $TARBALL
exit
fi


# The working directory.
cd ${bibledit}


# Remove bits from any older build that might cause crashes in the new build.
find . -name "*.o" -delete


echo Changing the program name and installation location to ${bibledit}/${Bibledit}
sed -i.bak "s/bibledit/$bibledit/g" configure.ac
if [ $? -ne 0 ]; then exit; fi
sed -i.bak "s/Bibledit/$Bibledit/g" configure.ac
if [ $? -ne 0 ]; then exit; fi
sed -i.bak "s/bin_PROGRAMS = bibledit/bin_PROGRAMS = ${bibledit}/g" Makefile.am
if [ $? -ne 0 ]; then exit; fi
sed -i.bak "s/bibledit_SOURCES/${bibledit}_SOURCES/g" Makefile.am
if [ $? -ne 0 ]; then exit; fi
sed -i.bak "s/bibledit_LDADD/${bibledit}_LDADD/g" Makefile.am
if [ $? -ne 0 ]; then exit; fi
# Remove backup file(s).
rm *.bak


# Remove the internationalization file.
# It is not needed.
rm locale/bibledit.pot
if [ $? -ne 0 ]; then exit; fi


if [ "$bible" != "bible" ]
then
echo Renaming the bibles folder where to store the Bibles to ${bible}s
cp -r bibles ${bible}s
if [ $? -ne 0 ]; then exit; fi
rm -rf bibles
if [ $? -ne 0 ]; then exit; fi
# Update the references to this folder in the code.
sed -i.bak "s/\"bibles\"/\"${bible}s\"/g" database/bibles.cpp setup/logic.cpp
if [ $? -ne 0 ]; then exit; fi
# Remove backup file(s).
rm database/*.bak
rm setup/*.bak
fi


if [ "$bible" != "bible" ]
then
echo Renaming the databases/config/bible folder where to store the Bibles configuration data
cp -r databases/config/bible databases/config/${bible}
if [ $? -ne 0 ]; then exit; fi
rm -rf databases/config/bible
if [ $? -ne 0 ]; then exit; fi
# Update the references to this folder in the code.
sed -i.bak "s/\"bible\"/\"${bible}\"/g" database/config/bible.cpp
if [ $? -ne 0 ]; then exit; fi
# Remove backup file(s).
rm database/config/*.bak
fi


echo Removing the man file as not needed in this situation
rm -f man/bibledit.1
sed -i.bak "/man_MANS/g" Makefile.am
# Remove backup file(s).
rm *.bak


if [ "$Bibledit" != "Bibledit" ]
then
echo Updating title of $Bibledit
sed -i.bak "s/\"Bibledit\"/\"${Bibledit}\"/g" executable/bibledit.cpp
if [ $? -ne 0 ]; then exit; fi
# Remove backup file(s).
find . -name "*.bak" -delete
fi


if [ "$bibledit" != "bibledit" ]
then
echo Updating data directory for $bibledit
sed -i.bak "s/\"bibledit\"/\"${bibledit}\"/g" executable/bibledit.cpp
if [ $? -ne 0 ]; then exit; fi
# Remove backup file(s).
find . -name "*.bak" -delete
fi


# Change any files with the fragment "Bible" in them to "Scripture".
# At the time of writing this script, there was only one file.
# The wildcard ; matches any directory.
# The wildcard * is the well-known one.
# The #1 matches the first wildcard, #2 the second wildcard, and so on.
# mmv ";*Bible*" "#1#2$Bible#3"
# if [ $? -ne 0 ]; then exit; fi


# Deal with "freebible.html" and where it is called.
# mmv ";*freebible*" "#1#2free$bible#3"
# if [ $? -ne 0 ]; then exit; fi
# find . -name "*.html" -print0 | xargs -0 sed -i.bak -e "s/freebible/free$bible/g"
# if [ $? -ne 0 ]; then exit; fi


# Deal with "bibledit.xpm" and where it is called.
# mmv ";*bibledit.xpm*" "#1#2$bibledit.xpm#3"
# if [ $? -ne 0 ]; then exit; fi
# sed -i.bak "s/bibledit.xpm/$bibledit.xpm/g" Makefile.am executable/bibledit.cpp
# if [ $? -ne 0 ]; then exit; fi


# Deal with "bibledit.png" and where it is called.
# mmv ";*bibledit.png*" "#1#2$bibledit.png#3"
# if [ $? -ne 0 ]; then exit; fi
# sed -i.bak "s/bibledit.png/$bibledit.png/g" Makefile.am setup/index.html
# if [ $? -ne 0 ]; then exit; fi


# Deal with "quickbible.html" and where it is called.
# mmv ";*quickbible.html*" "#1#2quick${bible}.html#3"
# if [ $? -ne 0 ]; then exit; fi


./reconfigure
if [ $? -ne 0 ]; then exit; fi
./configure
if [ $? -ne 0 ]; then exit; fi
make clean
if [ $? -ne 0 ]; then exit; fi
make
if [ $? -ne 0 ]; then exit; fi


if [ "$bibledit" != "bibledit" ]
then
echo Removing existing bibledit binaries and desktop file
rm -f bibledit
rm -f libbibledit.a
rm -f bibledit.desktop
echo Removing Sample Bible
rm -rf bibles
rm -f databases/sample.sqlite
fi


# Remove the script, so people cannot reuse it.
# Reusing scripts have given problems in the past as newer scripts were different.
rm $SCRIPTPATH

echo If there were no errors, ${Bibledit} should be working now.
echo --
echo To start ${Bibledit}, open a terminal, and type:
echo ${bibledit}
echo and press Enter.
