BEGIN {
D["PACKAGE_NAME"]=" \"bibledit\""
D["PACKAGE_TARNAME"]=" \"bibledit\""
D["PACKAGE_VERSION"]=" \"5.0.990\""
D["PACKAGE_STRING"]=" \"bibledit 5.0.990\""
D["PACKAGE_BUGREPORT"]=" \"http://bibledit.org\""
D["PACKAGE_URL"]=" \"\""
D["PACKAGE"]=" \"bibledit\""
D["VERSION"]=" \"5.0.990\""
D["HAVE_CXX17"]=" 1"
D["HAVE_STDIO_H"]=" 1"
D["HAVE_STDLIB_H"]=" 1"
D["HAVE_STRING_H"]=" 1"
D["HAVE_INTTYPES_H"]=" 1"
D["HAVE_STDINT_H"]=" 1"
D["HAVE_STRINGS_H"]=" 1"
D["HAVE_SYS_STAT_H"]=" 1"
D["HAVE_SYS_TYPES_H"]=" 1"
D["HAVE_UNISTD_H"]=" 1"
D["STDC_HEADERS"]=" 1"
D["HAVE_PTHREAD"]=" 1"
D["HAVE_LIBPROC"]=" 1"
D["HAVE_EXECINFO"]=" 1"
D["HAVE_SYS_SYSCTL"]=" 1"
D["HAVE_MACH_MACH"]=" 1"
D["PACKAGE_DATA_DIR"]=" \"/usr/share/bibledit\""
D["PACKAGE_PREFIX_DIR"]=" \"NONE\""
D["HAVE_STOI"]=" 1"
D["HAVE_ICU"]=" 1"
D["HAVE_UTF8PROC"]=" 1"
  for (key in D) D_is_set[key] = 1
  FS = ""
}
/^[\t ]*#[\t ]*(define|undef)[\t ]+[_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ][_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789]*([\t (]|$)/ {
  line = $ 0
  split(line, arg, " ")
  if (arg[1] == "#") {
    defundef = arg[2]
    mac1 = arg[3]
  } else {
    defundef = substr(arg[1], 2)
    mac1 = arg[2]
  }
  split(mac1, mac2, "(") #)
  macro = mac2[1]
  prefix = substr(line, 1, index(line, defundef) - 1)
  if (D_is_set[macro]) {
    # Preserve the white space surrounding the "#".
    print prefix "define", macro P[macro] D[macro]
    next
  } else {
    # Replace #undef with comments.  This is necessary, for example,
    # in the case of _POSIX_SOURCE, which is predefined and required
    # on some systems where configure will not decide to define it.
    if (defundef == "undef") {
      print "/*", prefix defundef, macro, "*/"
      next
    }
  }
}
{ print }
