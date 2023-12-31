AC_PREREQ(2.59)
AC_INIT(alsa-plugins, 1.0.29)
AM_INIT_AUTOMAKE
AC_PREFIX_DEFAULT(/usr)

AM_MAINTAINER_MODE([enable])

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_PROG_INSTALL
AC_DISABLE_STATIC
AM_PROG_LIBTOOL
AC_HEADER_STDC

CC_NOUNDEFINED

PKG_CHECK_MODULES(ALSA, alsa >= 1.0.11)
AC_CHECK_LIB(asound, snd_pcm_ioplug_create,,
	     AC_ERROR([*** libasound has no external plugin SDK]), -ldl)

AC_ARG_ENABLE([jack],
      AS_HELP_STRING([--disable-jack], [Disable building of JACK plugin]))

if test "x$enable_jack" != "xno"; then
  PKG_CHECK_MODULES(JACK, jack >= 0.98, [HAVE_JACK=yes], [HAVE_JACK=no])
fi
AM_CONDITIONAL(HAVE_JACK, test x$HAVE_JACK = xyes)

AC_ARG_ENABLE([pulseaudio],
      AS_HELP_STRING([--disable-pulseaudio], [Disable building of pulseaudio plugin]))

if test "x$enable_pulseaudio" != "xno"; then
  PKG_CHECK_MODULES(pulseaudio, [libpulse >= 0.9.11], [HAVE_PULSE=yes], [HAVE_PULSE=no])
fi
AM_CONDITIONAL(HAVE_PULSE, test x$HAVE_PULSE = xyes)

AC_ARG_ENABLE([samplerate],
      AS_HELP_STRING([--disable-samplerate], [Disable building of samplerate plugin]))

if test "x$enable_samplerate" != "xno"; then
  PKG_CHECK_MODULES(samplerate, [samplerate], [HAVE_SAMPLERATE=yes], [HAVE_SAMPLERATE=no])
fi
AM_CONDITIONAL(HAVE_SAMPLERATE, test x$HAVE_SAMPLERATE = xyes)

AC_ARG_ENABLE([maemo-plugin],
	AS_HELP_STRING([--enable-maemo-plugin],
		[Build Maemo DSP plugin]),
	[HAVE_MAEMO_PLUGIN="$enableval"], [HAVE_MAEMO_PLUGIN="no"])
AM_CONDITIONAL(HAVE_MAEMO_PLUGIN, test x$HAVE_MAEMO_PLUGIN = xyes)

AC_ARG_ENABLE([maemo-resource-manager],
	AS_HELP_STRING([--enable-maemo-resource-manager],
		[Use Maemo resource manager]),
	[use_maemo_rm="$enableval"], [use_maemo_rm="no"])

if test "$use_maemo_rm" = "yes"; then
        PKG_CHECK_MODULES(DBUS, [dbus-1], [HAVE_DBUS=yes], [HAVE_DBUS=no])

        if test "$HAVE_DBUS" = "yes"; then
                AC_DEFINE(USE_RESOURCE_MANAGER, 1,"Use dbus server as a resouce manager")
        else
        	AC_ERROR([You must have D-Bus installed if you want to enable the Maemo resource manager])
        fi
fi

AC_ARG_ENABLE([avcodec],
      AS_HELP_STRING([--disable-avcodec], [Don't build plugins depending on avcodec (a52)]))

if test "x$enable_avcodec" != "xno"; then
  PKG_CHECK_MODULES(AVCODEC, [libavcodec libavutil], [HAVE_AVCODEC=yes], [HAVE_AVCODEC=no])
fi

if test "x$HAVE_AVCODEC" = "xno"; then
  AC_ARG_WITH([avcodec-includedir],
	AS_HELP_STRING([--with-avcodec-includedir=dir],
		[AVcodec include directory]),
	[AVCODEC_CFLAGS="-I$withval"], [AVCODEC_CFLAGS=""])
  AC_ARG_WITH([avcodec-libdir],
	AS_HELP_STRING([--with-avcodec-libdir=dir],
		[AVcodec library directory]),
	[AVCODEC_LIBS="-L$withval"], [AVCODEC_LIBS=""])

  CFLAGS_saved="$CFLAGS"
  LDFLAGS_saved="$LDFLAGS"
  CFLAGS="$CFLAGS $AVCODEC_CFLAGS"
  LDFLAGS="$LDFLAGS $AVCODEC_LIBS"
  AVCODEC_LIBS="$AVCODEC_LIBS -lavcodec"
  AC_CHECK_LIB([avcodec], [avcodec_open], [HAVE_AVCODEC=yes], [HAVE_AVCODEC=no])
  CFLAGS="$CFLAGS_saved"
  LDFLAGS="$LDFLAGS_saved"
fi

if test $HAVE_AVCODEC = yes; then
  AVCODEC_HEADER=""
  AC_CHECK_HEADER([ffmpeg/avcodec.h], [AVCODEC_HEADER='<ffmpeg/avcodec.h>'])
  if test -z "$AVCODEC_HEADER"; then
    AC_CHECK_HEADER([libavcodec/avcodec.h], [AVCODEC_HEADER='<libavcodec/avcodec.h>'])
  fi
  if test -z "$AVCODEC_HEADER"; then
    HAVE_AVCODEC=no
  fi
fi

AM_CONDITIONAL(HAVE_AVCODEC, test x$HAVE_AVCODEC = xyes)
AC_SUBST(AVCODEC_CFLAGS)
AC_SUBST(AVCODEC_LIBS)
AC_SUBST(AVCODEC_HEADER)

PKG_CHECK_MODULES(speexdsp, [speexdsp >= 1.2], [HAVE_SPEEXDSP="yes"], [HAVE_SPEEXDSP=""])
AM_CONDITIONAL(HAVE_SPEEXDSP, test "$HAVE_SPEEXDSP" = "yes")

AC_ARG_WITH([speex],
	AS_HELP_STRING([--with-speex={builtin|lib|no}],
		[build speex resampler (built-in code, link with external lib, or no build)]),
	[PPH=$withval], [PPH="lib"])

USE_LIBSPEEX=""
if test "$PPH" = "lib"; then
	if test "$HAVE_SPEEXDSP" = "yes"; then
		AC_CHECK_LIB([speexdsp], [speex_resampler_init],
			[USE_LIBSPEEX="yes"], [USE_LIBSPEEX=""])
	fi
	if test "$USE_LIBSPEEX" = "yes"; then
		AC_DEFINE(USE_LIBSPEEX, 1, "Link with libspeex for resampler")
	else
		echo "No libspeex with resampler unit; use built-in code"
		PPH="builtin"
	fi
fi

AM_CONDITIONAL(HAVE_PPH, test "$PPH" = "builtin" -o "$PPH" = "lib")
AM_CONDITIONAL(USE_LIBSPEEX, test "$PPH" = "lib")

test "x$prefix" = xNONE && prefix=$ac_default_prefix
test "x$exec_prefix" = xNONE && exec_prefix=$prefix

dnl ALSA plugin directory
AC_ARG_WITH(plugindir,
    AS_HELP_STRING([--with-plugindir=dir],
	[path where ALSA plugin files are stored]),
    plugindir="$withval", plugindir="")
if test -z "$plugindir"; then
    eval dir="$libdir"
    case "$dir" in
    /*) ;;
    *) dir="$dir"
    esac
    plugindir="$dir/alsa-lib"
fi
AC_DEFINE_UNQUOTED(ALSA_PLUGIN_DIR, "$plugindir", [directory containing ALSA add-on modules])
ALSA_PLUGIN_DIR="$plugindir"
AC_SUBST(ALSA_PLUGIN_DIR)

dnl ALSA data directory
AC_ARG_WITH(alsadatadir,
    AS_HELP_STRING([--with-alsadatadir=dir],
	[path where ALSA data files are stored]),
    alsadatadir="$withval", alsadatadir="")
if test -z "$alsadatadir"; then
    eval dir="$datadir"
    case "$dir" in
    /*) ;;
    *) dir="$dir"
    esac
    alsadatadir="$dir/alsa"
fi
AC_DEFINE_UNQUOTED(ALSA_DATA_DIR, "$alsadatadir", [directory containing ALSA data files])
ALSA_DATA_DIR="$alsadatadir"
AC_SUBST(ALSA_DATA_DIR)

SAVE_PLUGINS_VERSION

AC_OUTPUT([
	Makefile
	oss/Makefile
	pph/Makefile
	jack/Makefile
	pulse/Makefile
	mix/Makefile
	rate/Makefile
	a52/Makefile
	rate-lavc/Makefile
	maemo/Makefile
	doc/Makefile
	usb_stream/Makefile
	speex/Makefile
	arcam-av/Makefile
])

dnl Show the build conditions

echo
echo "Plugin directory: $ALSA_PLUGIN_DIR"
echo "ALSA_CFLAGS: $ALSA_CFLAGS"
echo "ALSA_LIBS: $ALSA_LIBS"
echo "JACK plugin:        $HAVE_JACK"
if test "$HAVE_JACK" = "yes"; then
  echo "  JACK_CFLAGS: $JACK_CFLAGS"
  echo "  JACK_LIBS: $JACK_LIBS"
fi
echo "Pulseaudio plugin:  $HAVE_PULSE"
if test "$HAVE_PULSE" = "yes"; then
  echo "  pulseaudio_CFLAGS: $pulseaudio_CFLAGS"
  echo "  pulseaudio_LIBS: $pulseaudio_LIBS"
fi
echo "Samplerate plugin:  $HAVE_SAMPLERATE"
if test "$HAVE_SAMPLERATE" = "yes"; then
  echo "  samplerate_CFLAGS: $samplerate_CFLAGS"
  echo "  samplerate_LIBS: $samplerate_LIBS"
fi
echo "Maemo plugin:       $HAVE_MAEMO_PLUGIN"
echo "  Using Osso resource manager: $use_maemo_rm"
echo "A52, lavc plugins:  $HAVE_AVCODEC"
if test "$HAVE_AVCODEC" = "yes"; then
  echo "  AVCODEC_CFLAGS: $AVCODEC_CFLAGS"
  echo "  AVCODEC_LIBS: $AVCODEC_LIBS"
  echo "  AVCODEC_HEADER: $AVCODEC_HEADER"
fi
echo "Speex rate plugin:  $PPH"
echo "Speex preprocess plugin:  $HAVE_SPEEXDSP"
if test "$HAVE_SPEEX" = "yes"; then
  echo "  speexdsp_CFLAGS: $speexdsp_CFLAGS"
  echo "  speexdsp_LIBS: $speexdsp_LIBS"
fi
