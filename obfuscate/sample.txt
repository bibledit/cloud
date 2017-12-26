# Sample configuration file for obfuscating the user interface.

# Lines that start with a number sign (#) are comments.

# Obfuscating will take effect after completing the following steps:
# 1. Copying this file sample.txt to texts.txt.
# 2. Updating file texts.txt.
# 3. Restarting Bibledit.

# Search and replace is case sensitive.

# Longer strings get replaced before shorter ones.

# Words to search for can be obfuscated themselves.
# The following are recognized:
# "Bbe" searches for "Bibledit", and "bb" for "bible", and "Bb" for "Bible".

# The following will change the name of Bibledit to Scriptedit:
Bbe=Scriptedit

# More examples:
bb=volume
Bb=Volume
Genesis=Startup
Proverbs=Oneliners

# Uncomment the following line to hide the Bible resources.
HideBibleResources

