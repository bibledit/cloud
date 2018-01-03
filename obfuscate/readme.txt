Installing an obfuscated version of Bibledit Cloud:
* Install Bibledit Cloud as usual.
* Follow the steps in obfuscate/sample.txt.


Creating and installing an obfuscated version of a Bibledit Linux client.
* Follow the information at https://bibledit.org:8081/help/installlinux to install dependencies and get the stock Bibledit Linux client.
* The information says to run the installer. Do not run it yet right now.
* Update file install.sh with the obfuscation information.
  There's four lines of them.
  Remove the number signs to make the obfuscation information active.
  Sample:

    # Bibledit=Scripturedit
    # bibledit=scripturedit
    # Bible=Scripture
    # bible=scripture

* Run the installer as usual.
* Follow the steps in obfuscate/sample.txt.
