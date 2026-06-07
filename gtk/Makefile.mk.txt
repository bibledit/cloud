applicationdir = $(datadir)/applications
application_DATA = bibledit.desktop

iconsdir = $(datadir)/icons
icons_DATA = bbe48x48.xpm bbe512x512.png

## It should be sufficient to put the icon for the .desktop file
## in the icons folder only,
## but a Debian package somehow does not install those.
## That is why the icon is also put in the pixmaps folder.

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = bbe48x48.xpm bbe512x512.png

appstreamdir = $(datadir)/metainfo/
appstream_DATA = bibledit.appdata.xml

