AM_CFLAGS = $(libapfstool_CFLAGS) $(libgeneral_CFLAGS) $(libra1nsn0w_CFLAGS)
AM_CXXFLAGS = $(AM_CFLAGS)
AM_LDFLAGS = $(libapfstool_LIBS) $(libgeneral_LIBS) $(libra1nsn0w_LIBS)

lib_LTLIBRARIES = libramdisk_patcher_ra1nsn0w_plugin.la

libramdisk_patcher_ra1nsn0w_plugin_la_CPPFLAGS = $(AM_CFLAGS)
libramdisk_patcher_ra1nsn0w_plugin_la_LIBADD = $(AM_LDFLAGS)
libramdisk_patcher_ra1nsn0w_plugin_la_SOURCES = plugin.cpp \
                                                RPObj.cpp \
                                                RPRamdisk.cpp \
                                                utils.cpp
