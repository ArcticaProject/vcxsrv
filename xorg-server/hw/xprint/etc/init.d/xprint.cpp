XCOMM!/bin/sh
XCOMM
XCOMM Copyright 2002-2004 by Roland Mainz <roland.mainz@nrubsig.org>.
XCOMM
XCOMM This script manages the Xprint server side

XCOMM Basic support for IRIX-style "chkconfig"
XCOMM chkconfig: 2345 61 61
XCOMM description: Startup/shutdown script for Xprint server(s)

XCOMM Basic support for the Linux Standard Base Specification 1.0.0
XCOMM (Note: The Should-Start/Stop lines are there so that this works in the
XCOMM future, when the LSB adopts these. The X-UnitedLinux lines are there 
XCOMM so that it works right now.)
XCOMM## BEGIN INIT INFO
XCOMM Provides: xprint
XCOMM Required-Start: $local_fs $remote_fs $syslog $network
XCOMM Required-Stop: $local_fs $remote_fs $syslog
XCOMM Should-Start: cups lpd xfs
XCOMM Should-Stop:  cups lpd xfs
XCOMM X-UnitedLinux-Should-Start: cups lpd xfs
XCOMM X-UnitedLinux-Should-Stop:  cups lpd xfs
XCOMM Default-Start: 3 5
XCOMM Default-Stop: 0 1 2 6
XCOMM Description: Startup/shutdown script for Xprint server(s)
XCOMM## END INIT INFO

#undef sun
#undef unix

XCOMM###########################################################################
XCOMM
XCOMM This script has three main tasks:
XCOMM 1. Start Xprint servers ("Xprt") at boot time.
XCOMM 2. Shutdown Xprint servers when the machine is being shut down.
XCOMM 3. Query the list of available printers.
XCOMM
XCOMM Additional tasks are:
XCOMM 4. Restart ('restart'|'force-reload') and conditional restart
XCOMM    ('condrestart'/'try-restart') for Linux support
XCOMM 5. Wrapping of application call with setting XPSERVERLIST ('wrapper')
XCOMM
XCOMM Usage:
XCOMM - Start Xprint server(s) manually:
XCOMM   % /etc/init.d/xprint start
XCOMM
XCOMM - Stop Xprint server(s) manually:
XCOMM   % /etc/init.d/xprint stop
XCOMM
XCOMM - Populate $XPSERVERLIST env var (for example as part of a global
XCOMM   login script like /etc/profile or per-user login scripts like
XCOMM   $HOME/.profile (sh/ksh/bash))
XCOMM   % XPSERVERLIST="`/etc/init.d/xprint get_xpserverlist`"
XCOMM   % export XPSERVERLIST
XCOMM
XCOMM Installation:
XCOMM   Copy this script to /etc/init.d/xprint and make sure that it is
XCOMM   executable. If your installation is LSB-compliant, then run
XCOMM   % /usr/lib/lsb/install_initd /etc/init.d/xprint
XCOMM   to start the service on startup. Otherwise, manually create links
XCOMM   to the matching run levels.
XCOMM   Examples:
XCOMM     - Solaris 2.7/2.8/2.9:
XCOMM       % cp xprint /etc/init.d/xprint
XCOMM       % chmod a+rx /etc/init.d/xprint
XCOMM       % ln /etc/init.d/xprint /etc/rc0.d/K38xprint
XCOMM       % ln /etc/init.d/xprint /etc/rc1.d/K38xprint
XCOMM       % ln /etc/init.d/xprint /etc/rc2.d/S81xprint
XCOMM       % ln /etc/init.d/xprint /etc/rcS.d/K38xprint
XCOMM
XCOMM     - SuSE Linux 7.3
XCOMM       % cp xprint /etc/init.d/xprint
XCOMM       % chmod a+rx /etc/init.d/xprint
XCOMM       % ln -s ../xprint /etc/init.d/rc3.d/K13xprint
XCOMM       % ln -s ../xprint /etc/init.d/rc3.d/S12xprint
XCOMM       % ln -s ../xprint /etc/init.d/rc5.d/K13xprint
XCOMM       % ln -s ../xprint /etc/init.d/rc5.d/S12xprint
XCOMM       % ln -s ../xprint /etc/init.d/rc2.d/K13xprint
XCOMM       % ln -s ../xprint /etc/init.d/rc2.d/S12xprint
XCOMM
XCOMM     - SuSE Linux 6.4:
XCOMM       % cp xprint /sbin/init.d/xprint
XCOMM       % chmod a+rx /sbin/init.d/xprint
XCOMM       % ln -s ../xprint /sbin/init.d/rc2.d/K20xprint
XCOMM       % ln -s ../xprint /sbin/init.d/rc2.d/S20xprint
XCOMM       % ln -s ../xprint /sbin/init.d/rc3.d/K20xprint
XCOMM       % ln -s ../xprint /sbin/init.d/rc3.d/S20xprint
XCOMM
XCOMM   Notes: 
XCOMM   - The Xprint servers must be started _after_ the print
XCOMM     spooler or the server(s) may refuse to start
XCOMM   - The script should be readable by all users to ensure that they
XCOMM     can use the "get_xpserverlist"-option
XCOMM
XCOMM Custom configuration:
XCOMM   - Edit the function setup_config() in this script to match your needs
XCOMM
XCOMM Known bugs/ToDo/Notes:
XCOMM   - The shell script assumes that a Xserver can be reached via 
XCOMM     "hostname:displaynum" where "hostname" is obtained from
XCOMM     "/usr/bin/hostname". It may happen that a kernel firewall
XCOMM     blocks an X connection on the same host (e.g. client && Xserver 
XCOMM     are running on the same host).
XCOMM     Suggested fix: Fix the firewall config.
XCOMM     Suggested workaround: Edit this script and replace the use of 
XCOMM     /usr/bin/hostname with "echo 'localhost'".
XCOMM
XCOMM###########################################################################
XCOMM


XCOMM###########################################################################

fatal_error()
{
    echo "${0}: ## FATAL ERROR: ${1}" 1>&2
    exit 1
}

error_echo()
{
    echo "${0}: ## ERROR: ${1}" 1>&2
}

warning_echo()
{
    echo "${0}: ## WARNING: ${1}" 1>&2
}

verbose_echo()
{
    echo "${0}: ${1}"
}

msg()
{
    echo "${1}"
}

XCOMM###########################################################################

#ifndef OS_LINUX
XCOMM Force use of a POSIX conformant sh 
XCOMM (Solaris /sbin/sh is plain Bourne shell)
[ "$1" != "posix_sh_forced" -a -x /bin/ksh  ]            && exec /bin/ksh  "$0" posix_sh_forced "$@"
[ "$1" != "posix_sh_forced" -a -x /bin/bash ]            && exec /bin/bash --norc --noprofile "$0" posix_sh_forced "$@"
[ "$1" != "posix_sh_forced" -a -x /usr/local/bin/ksh ]   && exec /usr/local/bin/ksh "$0" posix_sh_forced "$@"
[ "$1" != "posix_sh_forced" -a -x /usr/local/bin/bash ]  && exec /usr/local/bin/bash --norc --noprofile "$0" posix_sh_forced "$@"
if [ "$1" != "posix_sh_forced" ] ; then
    echo "${0}: ## FATAL ERROR: No POSIX-shell found."  1>&2
    exit 1
fi

shift # Drop "posix_sh_forced"
#endif /* !OS_LINUX */

XCOMM#debug
XCOMM set -x

XCOMM Change current dir to a location which is writeable by everyone
cd /tmp

XCOMM Clear some environment variables
unset TEMP TMPDIR SCREENDIR

XCOMM Set search path for commands
export PATH=/usr/bin:/bin:/usr/sbin:/sbin
#ifdef OS_SOLARIS
export PATH=/usr/xpg4/bin:${PATH}
#endif

XCOMM# Try to figure-out where X11 was installed
#if defined(OS_SOLARIS)
XPROJECTROOT=/usr/openwin
export OPENWINHOME=/usr/openwin
#elif defined(OS_AIX)
XPROJECTROOT=/usr/lpp/X11
#else
#if defined(ProjectRoot)
[ -d ProjectRoot/bin ]	&& XPROJECTROOT=ProjectRoot
#endif
[ -d /usr/X11/bin ]     && XPROJECTROOT=/usr/X11
[ -d /usr/X11R6/bin ]   && XPROJECTROOT=/usr/X11R6
#endif
XPCUSTOMGLUE=DEF_XPCUSTOMGLUE # This is used for customizing this script
export XPROJECTROOT XPCUSTOMGLUE

if [ -z "${XPROJECTROOT}" ] ; then
    fatal_error "Unknown XProjectRoot."
fi

XCOMM Set the location of the Xprt binary we want to use.
XPRT_BIN="${XPROJECTROOT}/bin/Xprt"

XCOMM Set the location of the global file where we store the locations 
XCOMM of the system-wide servers
if [ -d /var/run ] ; then
    XP_GLOBAL_SERVERS=/var/run/Xprint_servers
else
    XP_GLOBAL_SERVERS=/tmp/.Xprint_servers
fi

XCOMM ${LOGNAME} will not work if user su'ed into another account
export CURRLOGNAME="$(id -u -n)"

XCOMM Set location where we want to store the list of Xprint servers managed
XCOMM by this user
XCOMM - If we start as "root" use the global file
XCOMM - If we start as normal user use a per-user file

if [ "${CURRLOGNAME}" != "root" -a "${CURRLOGNAME}" != "" ] ; then
    XP_PER_USER_SERVERS="/tmp/.Xprint_servers_${CURRLOGNAME}"
    XP_SERVERS="${XP_PER_USER_SERVERS}"
else
    XP_SERVERS="${XP_GLOBAL_SERVERS}"
fi

XCOMM Set umask that owner can r/w all files and everyone else can read them
umask 022

XCOMM Bump limit for per-process open files to ensure that Xprt can open many many fonts
ulimit -n 1024

XCOMM###########################################################################

XCOMM Get list of fonts for a given display
get_fontlist_from_display()
{
    ${XPROJECTROOT}/bin/xset -display "${1}" q | 
      awk "/Font Path:/ { i=1 ; next } i==1 { print \$0 ; i=0 }" | 
        fontpath2fontlist
}

XCOMM Get list from a fontserver config
get_fontlist_from_xfs_config()
{
    if [ ! -r "${1}" ] ; then
        return 0
    fi
      
    (        
      cat "${1}" |
      while read -r i ; do
          for val in $i; do
              case $val in
                \#*)  break ;;
                ?*=*) key="${val%%=*}" ;;
                =*)   key="${tok}" ;;
                *)    [ "${key}" = "catalogue" -a "${tok}" != "" ] && echo "${tok}" ;;
              esac
              tok="${val#*=}"
          done
      done
    ) | tr "," "\n" | fontpath2fontlist
}

get_fontlist_from_all_xfs_configs()
{
    get_fontlist_from_xfs_config "/etc/openwin/fs/fontserver.cfg"
    get_fontlist_from_xfs_config "/usr/openwin/lib/X11/fontserver.cfg"
    get_fontlist_from_xfs_config "/etc/X11/fs-xtt/config"
    get_fontlist_from_xfs_config "/etc/X11/fs/config"
    get_fontlist_from_xfs_config "/etc/X11/xfs/config"
    get_fontlist_from_xfs_config "${XPROJECTROOT}/lib/X11/fs/config"
}

get_fontlist_from_xf86config()
{
    srcxconf=""

    XCOMM see xorg.conf(5x) manual page for the list of locations used here    
    [ "${srcxconf}" = "" -a -f "/etc/X11/xorg.conf"  ]                      && srcxconf="/etc/X11/xorg.conf"
    [ "${srcxconf}" = "" -a -f "/usr/X11R6/etc/X11/xorg.conf" ]             && srcxconf="/usr/X11R6/etc/X11/xorg.conf"
    [ "${srcxconf}" = "" -a -f "/etc/X11/xorg.conf-4" ]                     && srcxconf="/etc/X11/xorg.conf-4"
    [ "${srcxconf}" = "" -a -f "/etc/X11/xorg.conf" ]                       && srcxconf="/etc/X11/xorg.conf"
    [ "${srcxconf}" = "" -a -f "/etc/xorg.conf" ]                           && srcxconf="/etc/xorg.conf"
    [ "${srcxconf}" = "" -a -f "/usr/X11R6/etc/X11/xorg.conf.${hostname}" ] && srcxconf="/usr/X11R6/etc/X11/xorg.conf.${hostname}"
    [ "${srcxconf}" = "" -a -f "/usr/X11R6/etc/X11/xorg.conf-4" ]           && srcxconf="/usr/X11R6/etc/X11/xorg.conf-4"
    [ "${srcxconf}" = "" -a -f "/usr/X11R6/etc/X11/xorg.conf" ]             && srcxconf="/usr/X11R6/etc/X11/xorg.conf"
    [ "${srcxconf}" = "" -a -f "/usr/X11R6/lib/X11/xorg.conf.${hostname}" ] && srcxconf="/usr/X11R6/lib/X11/xorg.conf.${hostname}"
    [ "${srcxconf}" = "" -a -f "/usr/X11R6/lib/X11/xorg.conf-4" ]           && srcxconf="/usr/X11R6/lib/X11/xorg.conf-4"
    [ "${srcxconf}" = "" -a -f "/usr/X11R6/lib/X11/xorg.conf" ]             && srcxconf="/usr/X11R6/lib/X11/xorg.conf"

    XCOMM Xfree86 locations
    [ "${srcxconf}" = "" -a -f "/etc/X11/XF86Config-4" ] && srcxconf="/etc/X11/XF86Config-4"
    [ "${srcxconf}" = "" -a -f "/etc/X11/XF86Config" ]   && srcxconf="/etc/X11/XF86Config"


    if [ "${srcxconf}" = "" ] ; then
        return 0
    fi

    currsection=""
    cat "${srcxconf}" |
    while read i1 i2 i3 i4 ; do
        # Strip "\"" from I2
        i2="${i2#\"}" ; i2="${i2%\"}"

        case "${i1}" in
            \#*)
                continue
                ;;
            'Section')
                currsection="${i2}"
                ;;
            'EndSection')
                currsection=""
                ;;
            'FontPath')
                [ "$currsection" = "Files" ] && echo "${i2%:unscaled}"
                ;;
        esac  
    done | egrep -v -i "tcp/|tcp4/|tcp6/|unix/"
    
    return 0
}

get_fontlist_from_defoma()
{
    # Include Debian defoma font directory where relevant
    if [ -d "/var/lib/defoma/x-ttcidfont-conf.d/dirs" ] ; then        
        find "/var/lib/defoma/x-ttcidfont-conf.d/dirs" -name fonts.dir |
          while read i ; do echo "${i%/fonts.dir}" ; done
    fi
}

XCOMM Get list of system fonts
get_system_fontlist()
{
#if defined(OS_SOLARIS)
          ## List the standard X11 fonts
          # echo "${XPROJECTROOT}/lib/X11/fonts/F3/"
          # echo "${XPROJECTROOT}/lib/X11/fonts/F3bitmaps/"
          echo "${XPROJECTROOT}/lib/X11/fonts/Type1/"
          echo "${XPROJECTROOT}/lib/X11/fonts/Type1/outline/"
          # We cannot use /usr/openwin/lib/X11/fonts/Type1/sun/ - see
          # http://xprint.mozdev.org/bugs/show_bug.cgi?id=5726
          # ("Xprint doesn't start under Solaris 2.9 due *.ps files in /usr/openwin/lib/X11/fonts/Type1/sun/fonts.dir")
          #echo "${XPROJECTROOT}/lib/X11/fonts/Type1/sun/"
          echo "${XPROJECTROOT}/lib/X11/fonts/TrueType/"
          echo "${XPROJECTROOT}/lib/X11/fonts/Speedo/"
          echo "${XPROJECTROOT}/lib/X11/fonts/misc/"
          echo "${XPROJECTROOT}/lib/X11/fonts/75dpi/"
          echo "${XPROJECTROOT}/lib/X11/fonts/100dpi/"
  
          ## List all fonts in all locales installed on this machine
          cat /usr/openwin/lib/locale/''*/OWfontpath | tr "," "\n" | sort -u
#elif defined(OS_LINUX)
          (
            get_fontlist_from_defoma

            get_fontlist_from_xf86config
            
            # Getting font paths from XFS is mainly required for compatibilty to RedHat
            get_fontlist_from_all_xfs_configs
                   
            ## List all fonts in all locales installed on this machine
            (
              [ -d "/usr/share/fonts" ] && find /usr/share/fonts -name fonts.dir
              find "${XPROJECTROOT}/lib/X11/fonts" -name fonts.dir 
            ) | 
              while read i ; do echo "${i%/fonts.dir}" ; done
          ) | sort -u
#else
          ## List the standard X11 fonts
          # (AIX should be handled like Solaris but I did not found a way to
          # enumerate all fonts in all installed locales without scanning the
          # dirs manually)
          echo "${XPROJECTROOT}/lib/X11/fonts/Type1/"
          echo "${XPROJECTROOT}/lib/X11/fonts/TrueType/"
          echo "${XPROJECTROOT}/lib/X11/fonts/TTF/"
          echo "${XPROJECTROOT}/lib/X11/fonts/Speedo/"
          echo "${XPROJECTROOT}/lib/X11/fonts/misc/"
          echo "${XPROJECTROOT}/lib/X11/fonts/75dpi/"
          echo "${XPROJECTROOT}/lib/X11/fonts/100dpi/"
          echo "${XPROJECTROOT}/lib/X11/fonts/"
#endif
}

XCOMM Filter fonts per given extended regular expressions
XCOMM (and make sure we filter any model-config fonts - they are managed by Xprt internally)
filter_fontlist()
{
    egrep -- "${1}" | fgrep -v "/models/" | egrep -v -- "${2}"
}

XCOMM Filter font paths with unsupported font types
XCOMM (such as CID fonts)
filter_unsupported_fonts()
{
    egrep -v -i "/cid(/$|$)|/csl(/$|$)"
}

XCOMM Validate fontpath
XCOMM Only return valid font path entries (note that these entries may
XCOMM include non-file stuff like font server references)
validate_fontlist()
{
    while read i ; do       
        case "${i}" in
            # Check if font path entry is a font server...
            tcp/''*|tcp4/''*|tcp6/''*|unix/''*)
                echo "${i}"
                ;;
            # ... if not check if the path is accessible
            # and has a valid "fonts.dir" index
            *)
                [ -f "${i}/fonts.dir" ] && echo "${i}"
                ;;
        esac
    done
}

XCOMM Build a comma-seperated list of fonts (font path) from a list of fonts
fontlist2fontpath()
{
    fp=""
    read fp;
    while read i ; do
        fp="${fp},${i}"
    done
    
    echo "$fp"
}

XCOMM Build a list (one item per line) of fonts from a font path
fontpath2fontlist()
{
    while read i ; do
        echo "${i}" | tr "," "\n"
    done
}

XCOMM Sort scaleable fonts (PS Type1 and TrueType) first in a font list
sort_scaleable_fonts_first()
{
    i="$(fontlist2fontpath)"
    # First list PS Type1 fonts...
    echo "${i}" | fontpath2fontlist | fgrep "/Type1"
    # ... then TrueType fonts ...
    echo "${i}" | fontpath2fontlist | egrep -i "/TrueType|/TT(/$|$)|/TTF(/$|$)"
    # ... then all others
    echo "${i}" | fontpath2fontlist | egrep -v -i "/Type1|/TrueType|/TT(/$|$)|/TTF(/$|$)"
}

XCOMM Check if a X display is used by a Xserver or not
XCOMM Known bugs:
XCOMM - there is no way in plain bourne shell or bash (see comment about ksh93
XCOMM   below) to test if a Xserver sits only on a socket and not on a pipe
XCOMM - some Xserver's don't cleanup their stuff in /tmp on exit which may end
XCOMM   in the problem that we don't detect possible free display numbers
XCOMM   (one problem is that only ksh93 can do stuff like 
XCOMM   "cat </dev/tcp/0.0.0.0/6001")
CheckIfDisplayIsInUse()
{
    id=$1
    
    [ -r "/tmp/.X${id}-lock"     ] && return 0;
    [ -r "/tmp/.X11-unix/X${id}" ] && return 0;
    [ -r "/tmp/.X11-pipe/X${id}" ] && return 0;
    
    return 1;
}

lastdisplaynumreturned_store=/tmp/.Xp_last_display_returned_by_findfreexdisplaynum_${RANDOM}_${RANDOM}

XCOMM Try to find a free display number
FindFreeXDisplayNum()
{
    if [ -r "${lastdisplaynumreturned_store}" ] ; then
      i="$(cat "${lastdisplaynumreturned_store}")"
    else
      i=32 # start at display 32
    fi
    
    while [ $i -lt 127 ] ; do
        i=$(($i + 1))

        if CheckIfDisplayIsInUse $i ; then
            true
        else
            echo "$i"
            echo "$i" >"${lastdisplaynumreturned_store}"
            return 0
        fi
    done

    # Using "magic" value of 189 here due lack of a better solution
    echo "189"
    echo "189" >"${lastdisplaynumreturned_store}"    
    return 0
}

XCOMM Check if a process exists or not
pexists()
{
    [ "$1" = "" ] && return 1;

    # Use of /proc would be better but not all platforms (like AIX) have procfs
    [ "$(ps -p $1 | fgrep $1)" != "" ] && return 0;
    return 1
}

XCOMM Check if there is a spooler running...
is_spooler_running()
{
    # This covers Linux lpd, CUPS, Solaris and AIX 4.x - but what about
    # AIX 5.x, HP-UX and IRIX ?

    [ "$(ps -A | egrep 'lpd|lpsched|cupsd|qdaemon' | fgrep -v 'grep')" != "" ] && return 0;
    return 1
}

XCOMM Wait until the spooler system has been started (but not more than 30secs)
wait_for_print_spooler()
{
    for i in 1 2 3 4 5 6 7 8 9 10 ; do
        is_spooler_running && return 0;
        sleep 3
    done
    
    return 0
}

lock_server_registry()
{
    lock_counter=0 # counts in 1/100s
    waiting_for_lock_msg_send="false"
    while ! mkdir "${XP_SERVERS}.lock" 2>/dev/null ; do
        # print notice after 2 seconds
        if [ ${lock_counter} -gt 200 -a "${waiting_for_lock_msg_send}" != "true" ] ; then
            echo "${0}: waiting for lock(=${XP_SERVERS}.lock)..."
            waiting_for_lock_msg_send="true"
        fi

        # tread the lock as "broken" after 20 seconds
        if [ ${lock_counter} -gt 2000 ] ; then
            echo "${0}: WARNING: lock timeout for lock(=${XP_SERVERS}.lock)."
            return 0
        fi
                
        if [ -x /bin/usleep ] ; then
            /bin/usleep 200000
            lock_counter=$((${lock_counter} + 20)) # 20/100s
        else
            sleep 1
            lock_counter=$((${lock_counter} + 100)) # 100/100s
        fi
    done
}

unlock_server_registry()
{
    rmdir "${XP_SERVERS}.lock"
}

XCOMM Kill dead registry locks (silently!)
kill_dead_registry_locks()
{
    rm -Rf "${XP_SERVERS}.lock"
}

XCOMM Start Xprint servers
start_servers()
{
    # Write registry "intro" ...
    lock_server_registry
    echo "# Xprint server list"                               >>"${XP_SERVERS}"
    echo "# File is for private use for ${0}."                >>"${XP_SERVERS}"
    echo "# Do not edit, rely on the content or file format." >>"${XP_SERVERS}"
    unlock_server_registry

    hostname="$(hostname)"

    default_fontpath="$(get_system_fontlist | fontlist2fontpath)"
    default_fontpath_acceptpattern=".*";
    default_fontpath_rejectpattern="_No_Match_"; # Match nothing   
    
    curr=0
    while [ $curr -lt $num_xpstart ] ; do
        if [ "${xpstart_remote_server[$curr]}" != "" ] ; then
            # Remote Xprt, just put the entry into the registry
            lock_server_registry
            echo "display=${xpstart_remote_server[$curr]}" >>"${XP_SERVERS}"
            unlock_server_registry
        else
            # Run block in seperate process to avoid that changes to the
            # xpstart_* variables affect the next cycle
            (
              # Use defaults if there are no special options
              [ "${xpstart_fontpath[$curr]}"               = "" ] &&               xpstart_fontpath[$curr]="${default_fontpath}";
              [ "${xpstart_fontpath_acceptpattern[$curr]}" = "" ] && xpstart_fontpath_acceptpattern[$curr]="$default_fontpath_acceptpattern";
              [ "${xpstart_fontpath_rejectpattern[$curr]}" = "" ] && xpstart_fontpath_rejectpattern[$curr]="$default_fontpath_rejectpattern";
              [ "${xpstart_displayid[$curr]}"              = "" ] &&              xpstart_displayid[$curr]="$(FindFreeXDisplayNum)"       
              [ "${xpstart_logger[$curr]}"                 = "" ] &&                 xpstart_logger[$curr]="logger -p lpr.notice -t Xprt_${xpstart_displayid[$curr]}";
              [ "${xpstart_logfile[$curr]}"                = "" ] &&                xpstart_logfile[$curr]="/dev/null";
              [ "${xpstart_xprt_binary[$curr]}"            = "" ] &&            xpstart_xprt_binary[$curr]="${XPRT_BIN}";
              if [ "${xpstart_xprt_binary[$curr]}" = "/usr/openwin/bin/Xprt" -o "$(uname -s)" = "SunOS" ] ; then
                  # Solaris /usr/openwin/bin/Xprt does not support "-nolisten tcp"
                  # yet nor is it possible to run a Xserver on a unix socket only
                  # in Solaris since access to the unix domain sockets in
                  # /tmp/.X11-pipe and /tmp/.X11-unix is restricted to applications
                  # with group-id "root" (e.g. the Xprt server would need to be
                  # setgid "root" that plain users can start it listening on a unix
                  # socket only)
                  [ "${xpstart_options[$curr]}" = "" ] && xpstart_options[$curr]="-ac -pn"
              else
                  [ "${xpstart_options[$curr]}" = "" ] && xpstart_options[$curr]="-ac -pn -nolisten tcp"
              fi

              # Check if the Xprt binary is available                                     
              if [ ! -x "${xpstart_xprt_binary[$curr]}" ] ; then                                            
                  error_echo "Can't find \"${xpstart_xprt_binary[$curr]}\"."                                  
                  exit 1 # exit block                                                               
              fi                                                                          

              # Verify and set location of font encodings directory file
              if [ "${xpstart_font_encodings_dir[$curr]}" = "" ] ; then
                  if [ -f "${XPROJECTROOT}/lib/X11/fonts/xf86encodings/encodings.dir" ] ; then
                      xpstart_font_encodings_dir[$curr]="${XPROJECTROOT}/lib/X11/fonts/xf86encodings/encodings.dir"
                  else
                      xpstart_font_encodings_dir[$curr]="${XPROJECTROOT}/lib/X11/fonts/encodings/encodings.dir";
                  fi
              fi

              unset FONT_ENCODINGS_DIRECTORY
              if [ ! -f "${xpstart_font_encodings_dir[$curr]}" ] ; then
                  warning_echo "Can't find \"${xpstart_font_encodings_dir[$curr]}\", TrueType font support may not work."
              fi

              export FONT_ENCODINGS_DIRECTORY="${xpstart_font_encodings_dir[$curr]}"

              # Generate font path (containing only valid font path elements)
              # from input font path and filter expressions
              curr_fp=$(echo "${xpstart_fontpath[$curr]}" | 
                        fontpath2fontlist |
                        filter_fontlist "${xpstart_fontpath_acceptpattern[$curr]}" "${xpstart_fontpath_rejectpattern[$curr]}" | 
                        filter_unsupported_fonts |
                        sort_scaleable_fonts_first | 
                        validate_fontlist | 
                        fontlist2fontpath)

              # Set Xserver auditing level option
              unset curr_audit
              if [ "${xpstart_auditlevel[$curr]}" != "" ] ; then
                  curr_audit="-audit ${xpstart_auditlevel[$curr]}"
              fi

              # Set Xprt -XpFile option
              unset curr_xpfile
              if [ "${xpstart_xpfile[$curr]}" != "" ] ; then
                  curr_xpfile="-XpFile ${xpstart_xpfile[$curr]}"
              fi

              # Set custom XPCONFIGDIR (if there is one)
              unset XPCONFIGDIR
              if [ "${xpstart_xpconfigdir[$curr]}" != "" ] ; then
                  export XPCONFIGDIR="${xpstart_xpconfigdir[$curr]}"
              fi

              # If someone uses "-nolisten tcp" make sure we do not add a hostname to force local transport
              if [ "$(echo "${xpstart_options[$curr]}" | egrep "nolisten.*tcp")" != "" ] ; then
                  xp_display=":${xpstart_displayid[$curr]}"
              else
                  xp_display="${hostname}:${xpstart_displayid[$curr]}"
              fi
              
              (
                (
                  "${xpstart_xprt_binary[$curr]}" \
                      ${xpstart_options[$curr]} \
                      ${curr_xpfile} ${curr_audit} \
                      -fp ${curr_fp} \
                      :${xpstart_displayid[$curr]} &
                  server_pid="$!"
 
                  # Append the new server to the registry
                  lock_server_registry
                  echo "display=${xp_display} display_id=${xpstart_displayid[$curr]} pid=${server_pid}" >>"${XP_SERVERS}"
                  unlock_server_registry
                  
                  wait
                  echo "Xprint server pid=${server_pid} done, exitcode=$?."
                  
                  # Remove the dead server from the registry
                  # (only if the registry still exists - if /etc/init.d/xprint stop" gets called the registry
                  # will be removed - and we should not re-create it afterwards...)
                  lock_server_registry
                  if [ -f "${XP_SERVERS}" ] ; then
                      x="$(cat "${XP_SERVERS}")" # Store content of file "${XP_SERVERS}" in var "x"
                      echo "${x}" | fgrep -v "display_id=${xpstart_displayid[$curr]} pid=${server_pid}" >"${XP_SERVERS}"
                  fi
                  unlock_server_registry                 
                ) 2>&1 | while read i ; do echo "$i" | tee -a "${xpstart_logfile[$curr]}" | ${xpstart_logger[$curr]} ; done
              ) &
            )
        fi
        
        curr=$(($curr + 1))
    done
    
    # Remove tmp. file created by |FindFreeXDisplayNum()|
    rm -f "${lastdisplaynumreturned_store}"

    # Done.
    lock_server_registry
    echo "# EOF." >>"${XP_SERVERS}"
    unlock_server_registry
    return 0
}


XCOMM Convenience function to check setup and start Xprt server(s)
do_start()
{
    if [ -f "${XP_SERVERS}" ] ; then
        numservers="$(do_get_xpserverlist | wc -l)"
        if [ ${numservers} -gt 0 ] ; then
            verbose_echo "Xprint servers are already running."
            return 0
        else
            verbose_echo "Old server registry found, cleaning-up..."
            do_stop
        fi
    fi
    
    # Check if we can write the registry file
    touch "${XP_SERVERS}" 2>/dev/null
    if [ ! -f "${XP_SERVERS}" ] ; then
        error_echo "Cannot create \"${XP_SERVERS}\"."
        # exit code 4 = user had insufficient privilege (LSB)
        exit 4
    fi

    if ! setup_config ; then
        error_echo "setup_config failed."
        exit 1
    fi

    # Provide two paths here - one which simply starts the Xprt servers,
    # assuming that there is a print spooler already running (or that 
    # ${XPCONFIG}/C/print/Xprinters provides static print queue entries
    # (like for the PSspooldir print model)) and a 2nd path which
    # explicitly checks if the print queue daemons are running
    if true ; then
        msg "Starting Xprint servers: Xprt."
        start_servers
    else 
        # Continue in the background if there is no spooler running yet (that
        # we don't hold off the boot process nor run in a possible race-condition
        # when /etc/init.d/lpd was not called yet but the runlevel script waits
        # for us to finish first ...
        if is_spooler_running ; then
            msg "Starting Xprint servers: Xprt."
            start_servers
        else
            msg "Starting Xprint servers (in the background): Xprt."
            (wait_for_print_spooler ; start_servers) &
            sleep 5
        fi
    fi
    
    if [ "${CURRLOGNAME}" = "root" -a -d /var/lock/subsys/ ] ; then
        touch /var/lock/subsys/xprint
    fi
}

XCOMM Convenience function to stop Xprt server(s)
do_stop()
{
    msg "Stopping Xprint servers: Xprt."

    lock_server_registry
    if [ -f "${XP_SERVERS}" ] ; then       
        reglist="$(cat "${XP_SERVERS}")"
        rm -f "${XP_SERVERS}"
    fi
    unlock_server_registry
    
    if [ "${reglist}" != "" ] ; then
        echo "${reglist}" |
          grep "^display=.*:.* pid=[0-9]*$" |
          while read i ; do
              (
                eval ${i}
                if pexists ${pid} ; then 
                    kill ${pid}
                fi

                # Remove the X sockets/pipes which are not in use anymore
                # (It would be better if the Xservers would cleanup this
                # automatically, but most Xservers do not do that... ;-(
                # Note that this will not work on Solaris where applications
                # must run with groupid="root" if they want to write into
                # /tmp/.X11-unix/ and/or /tmp/.X11-pipe/)
                if [ "${display_id}" != "" ] ; then
                    rm -f "/tmp/.X${display_id}-lock"     2>/dev/null
                    rm -f "/tmp/.X11-unix/X${display_id}" 2>/dev/null
                    rm -f "/tmp/.X11-pipe/X${display_id}" 2>/dev/null
                fi 
              )
          done
    fi

    if [ "${CURRLOGNAME}" = "root" -a -d /var/lock/subsys/ ] ; then
        rm -f /var/lock/subsys/xprint
    fi
    
    # Remove any outstanding (dead) locks and cleanup
    rm -f "${XP_SERVERS}"
    kill_dead_registry_locks
}

XCOMM Convenience function to obtain a list of available Xprint servers
do_get_xpserverlist()
{
    if [ -f "${XP_PER_USER_SERVERS}" -o -f "${XP_GLOBAL_SERVERS}" ] ; then
        xpserverlist=$(
          # Enumerate both per-user and global servers (in that order)
          (
            [ -f "${XP_PER_USER_SERVERS}" ]  && cat "${XP_PER_USER_SERVERS}"
            [ -f "${XP_GLOBAL_SERVERS}"   ]  && cat "${XP_GLOBAL_SERVERS}"
          ) |
            egrep "^display=.*:.* pid=[0-9]*$|^display=.*:[0-9]*$" |
              while read i ; do
                  (
                    pid="none"
                    eval ${i}
                    # Check if the Xprt process exists (if possible)
                    if [ "$pid" != "none" ] ; then
                        if pexists ${pid} ; then
                            echo ${display}
                        fi
                    else
                        echo ${display}
                    fi
                  )
              done | tr "\n" " "
          )
        # Only produce output if we have some entries...
        [ "${xpserverlist}" != "" ] && echo "${xpserverlist}"
    fi
}

do_restart()
{
    msg "Restarting Xprint server(s): Xprt."
    do_stop
    sleep 1
    do_start
}

do_diag()
{
    echo "##### Diag start $(date)."
    
    # General info
    echo "## General info start."
    (
      echo "PATH=\"${PATH}\""
      echo "TZ=\"${TZ}\""
      echo "LANG=\"${LANG}\""
      echo "uname -a=\"$(uname -a)\""
      echo "uname -s=\"$(uname -s)\""
      echo "uname -p=\"$(uname -p)\""
      echo "uname -i=\"$(uname -i)\""
      echo "uname -m=\"$(uname -m)\""
      echo "has /etc/SuSE-release ... $([ -f "/etc/SuSE-release" ] && echo "yes" || echo "no")"
      echo "has /etc/redhat-release ... $([ -f "/etc/redhat-release" ] && echo "yes" || echo "no")"
      echo "has /etc/debian_version ... $([ -f "/etc/debian_version" ] && echo "yes" || echo "no")"
      echo "how many Xprt servers are running ...$(ps -ef | fgrep Xprt | fgrep -v "grep" | wc -l)"
    ) 2>&1 | while read i ; do echo "  $i" ; done
    echo "## General info end."
    
    # Testing font paths
    echo "## Testing font paths start."
    (
      get_system_fontlist |
          filter_unsupported_fonts |
          sort_scaleable_fonts_first | 
          validate_fontlist | while read d ; do
          echo "#### Testing \"${d}\" ..."
          if [ ! -d "$d" ] ; then
              echo "# Error: $d does not exists."
              continue
          fi
          if [ ! -r "$d" ] ; then
              echo "# Error: $d not readable."
              continue
          fi
          if [ ! -f "${d}/fonts.dir" ] ; then
              echo "# Error: ${d}/fonts.dir not found."
              continue
          else
              if [ ! -r "${d}/fonts.dir" ] ; then
                  echo "# Error: ${d}/fonts.dir not readable."
                  continue
              fi
          fi
          if [ -f "${d}/fonts.alias" ] ; then
              if [ ! -r "${d}/fonts.alias" ] ; then
                  echo "# Error: ${d}/fonts.alias not readable."
              fi
          fi

          if [ "$(cat "${d}/fonts.dir" | fgrep 'cursor')" != "" ] ; then
              echo "${d}/fonts.dir has cursor font."
          fi
          if [ "$(cat "${d}/fonts.dir" | fgrep 'fixed')" != "" ] ; then
              echo "${d}/fonts.dir has fixed font."
          fi

          if [ -r "${d}/fonts.alias" ] ; then 
              if [ "$(cat "${d}/fonts.alias" | fgrep 'cursor')" != "" ] ; then
                  echo "${d}/fonts.alias has cursor font."
              fi
              if [ "$(cat "${d}/fonts.alias" | fgrep 'fixed')" != "" ] ; then
                  echo "${d}/fonts.alias has fixed font."
              fi
          fi

          linenum=0
          cat "${d}/fonts.dir" | while read i1 i2 i3 i4 ; do
              linenum=$((${linenum} + 1))
              [ ${linenum} -eq 1 ] && continue

              if [ ! -f "${d}/${i1}" ] ; then
                  echo "ERROR: ${d}/fonts.dir line ${linenum} has non-exististant font \"${i1}\" (=\"${i1} ${i2} ${i3} ${i4}\")"
              fi
          done
      done
    ) 2>&1 | while read i ; do echo "  $i" ; done
    echo "## Testing font paths end."
    
    echo "##### Diag End $(date)."
}

XCOMM Set platform-defaults for setup_config()
setup_config_defaults()
{
    curr_num_xpstart="${1}"
    
    #### Defaults for Linux/Solaris
    # Start Xprt using builtin XPCONFIGDIR at a free display numer
    # (Solaris(=SunOS5.x)'s /usr/openwin/bin/Xprt supports TrueType fonts,
    # therefore we don't need to filter them)
                      xpstart_fontpath[${curr_num_xpstart}]="";
        xpstart_fontpath_acceptpattern[${curr_num_xpstart}]=".*";
        xpstart_fontpath_rejectpattern[${curr_num_xpstart}]="/Speedo|/F3bitmaps|/F3";
                     xpstart_displayid[${curr_num_xpstart}]="";
                   xpstart_xpconfigdir[${curr_num_xpstart}]="";
                        xpstart_xpfile[${curr_num_xpstart}]="";
                    xpstart_auditlevel[${curr_num_xpstart}]="4";
                       xpstart_options[${curr_num_xpstart}]="";
                        xpstart_logger[${curr_num_xpstart}]="";
    # Check whether we have /dev/stderr (needed for old AIX + old Debian)
    if [ -w "/dev/stderr" ] ; then
                       xpstart_logfile[${curr_num_xpstart}]="/dev/stderr";
    else
                       xpstart_logfile[${curr_num_xpstart}]="/dev/tty";
    fi
                   xpstart_xprt_binary[${curr_num_xpstart}]="";

    # Custom rules for the GISWxprintglue package on Solaris
    # (which uses Solaris's /usr/openwin/bin/Xprt but a custom config)
    if [ "${XPCUSTOMGLUE}" = "GISWxprintglue" ] ; then
        xpstart_xpconfigdir[${curr_num_xpstart}]="/opt/GISWxprintglue/server/etc/XpConfig"
        xpstart_xprt_binary[${curr_num_xpstart}]="/usr/openwin/bin/Xprt"
    fi
    # Custom rules for the GISWxprint package on Solaris
    # (which uses both it's own Xprt and a custom config)
    if [ "${XPCUSTOMGLUE}" = "GISWxprint" ] ; then
        xpstart_xpconfigdir[${curr_num_xpstart}]="/opt/GISWxprint/server/etc/XpConfig"
        xpstart_xprt_binary[${curr_num_xpstart}]="/opt/GISWxprint/bin/Xprt"
        xpstart_font_encodings_dir[${curr_num_xpstart}]="/opt/GISWxprint/lib/X11/fonts/encodings/encodings.dir"
    fi
    
    #######################################################
    ###
    ### Debian Xprint package default configuration
    ###
    if [ "${XPCUSTOMGLUE}" = "DebianGlue" ] ; then
        # Set XPCONFIGDIR=/usr/share/Xprint/xserver
        xpstart_xpconfigdir[${curr_num_xpstart}]="/usr/share/Xprint/xserver";

        # Use fixed display ID (":64"), or else all client programs will have to be
        # restarted simply to update XPSERVERLIST to the latest ID when upgrading,
        # which would be a nightmare.
        xpstart_displayid[${curr_num_xpstart}]=64;

        # Do not send any messages to console
        xpstart_logfile[${curr_num_xpstart}]="/dev/null";

        # By default use binary provided by Debian's "xprt-xprintorg" package
        # (=/usr/bin/Xprt), otherwise leave blank (e.g. use script's default
        # (=/usr/X11R6/bin/Xprt))
        if [ -x "/usr/bin/Xprt" ] ; then
            xpstart_xprt_binary[${curr_num_xpstart}]="/usr/bin/Xprt";
        fi
    fi
    ###
    ### End Debian default configuration
    ###
    #######################################################
}

fetch_etc_initd_xprint_envvars()
{
    curr_num_xpstart="${1}"

    ## Process some $ETC_INITD_XPRINT_* vars after all which may be used by
    # a user to override the hardcoded values here when starting Xprt per-user
    # (a more flexible way is to provide an own setup config script in
    # "~./Xprint_per_user_startup" - see above)
    if [ "${ETC_INITD_XPRINT_XPRT_PATH}" != "" ] ; then
        xpstart_xprt_binary[${curr_num_xpstart}]="${ETC_INITD_XPRINT_XPRT_PATH}"
    fi
    if [ "${ETC_INITD_XPRINT_XPCONFIGDIR}" != "" ] ; then
        xpstart_xpconfigdir[${curr_num_xpstart}]="${ETC_INITD_XPRINT_XPCONFIGDIR}"
    fi
    if [ "${ETC_INITD_XPRINT_XPFILE}" != "" ] ; then
        xpstart_xpfile[${curr_num_xpstart}]="${ETC_INITD_XPRINT_XPFILE}"
    fi
    if [ "${ETC_INITD_XPRINT_LOGFILE}" != "" ] ; then
        xpstart_logfile[${curr_num_xpstart}]="${ETC_INITD_XPRINT_LOGFILE}"
    fi
    if [ "${ETC_INITD_XPRINT_DISPLAYID}" != "" ] ; then
        xpstart_displayid[${curr_num_xpstart}]="${ETC_INITD_XPRINT_DISPLAYID}"
    fi
    if [ "${ETC_INITD_XPRINT_FONTPATH}" != "" ] ; then
        xpstart_fontpath[${curr_num_xpstart}]="${ETC_INITD_XPRINT_FONTPATH}"
    fi
    if [ "${ETC_INITD_XPRINT_XPRT_OPTIONS}" != "" ] ; then
        xpstart_options[${curr_num_xpstart}]="${ETC_INITD_XPRINT_XPRT_OPTIONS}"
    fi
    if [ "${ETC_INITD_XPRINT_AUDITLEVEL}" != "" ] ; then
        xpstart_auditlevel[${curr_num_xpstart}]="${ETC_INITD_XPRINT_AUDITLEVEL}"
    fi
    if [ "${ETC_INITD_XPRINT_XF86ENCODINGSDIR}" != "" ] ; then
        xpstart_font_encodings_dir[${curr_num_xpstart}]="${ETC_INITD_XPRINT_XF86ENCODINGSDIR}"
    fi
}

XCOMM###########################################################################
XCOMM setup_config() sets the configuration parameters used to start one
XCOMM or more Xprint servers ("Xprt").
XCOMM The following variables are used:
XCOMM - "num_xpstart" - number of servers to start
XCOMM - "xpstart_fontpath[index]" - custom font path. Leave blank if you want
XCOMM   the platform-specific default
XCOMM - "xpstart_fontpath_acceptpattern[index]" - extended regular expression
XCOMM   (see egrep(1)) used to filter the font path - items only pass this
XCOMM   filter if they match the pattern (leave blank if you want to filter
XCOMM   nothing)
XCOMM - "xpstart_fontpath_rejectpattern[index]" - extended regular expression
XCOMM   (see egrep(1)) used to filter the font path - items only pass this
XCOMM   filter if they do not match the pattern (leave blank if you want to
XCOMM   filter nothing)
XCOMM - "xpstart_font_encodings_dir[index]" - location of "encodings.dir".
XCOMM   Leave blank to use the default.
XCOMM - "xpstart_displayid[index]" - display id to use for the Xprint server
XCOMM   (leave blank to choose the next available free display id)
XCOMM - "xpstart_xpconfigdir[index]" - value for custom XPCONFIGDIR (leave blank
XCOMM   if you don not want that that XPCONFIGDIR is set at Xprt startup)
XCOMM - "xpstart_xpfile[index]" - value used for Xprt's "-XpFile" option (leave
XCOMM   blank if you do not want to set this option)
XCOMM - "xpstart_auditlevel[index]" - set Xserver auditing level (leave blank to
XCOMM   use no auditing)
XCOMM - "xpstart_options[index]" - set further Xprt options (leave blank to set
XCOMM   no further options)
XCOMM - "xpstart_logger[index]" - utility which gets stderr/stdout messages from
XCOMM   Xprt and sends them to a logging daemon. Leave blank to use /usr/bin/logger
XCOMM   to send such messages to the lpr.notice syslog)
XCOMM - "xpstart_logfile[index]" - log file to append stderr/stdout messages from
XCOMM   Xprt to. Leave blank to send messages to /dev/null
XCOMM - "xpstart_xprt_binary[index]" - set custom Xprt binary (leave blank to use
XCOMM   the platform-specifc default)
setup_config()
{
    num_xpstart=0;
    
    if [ "${ETC_INITD_XPRINT_CUSTOM_SETUP_CONFIG}" != "" ] ; then
        user_cfg="${ETC_INITD_XPRINT_CUSTOM_SETUP_CONFIG}"
    else
        user_cfg="${HOME}/.Xprint_per_user_startup"
    fi
    
    # Source per-user ~/.Xprint_per_user_startup file if there is one
    # (and do not use the script's defaults below)
    if [ -r "${user_cfg}" ] ; then
        # Define API version which should be checked by ${HOME}/.Xprint_per_user_startup
        # ${HOME}/.Xprint_per_user_startup should bail-out if the version differ
        etc_initd_xprint_api_version=2
        
        # Source per-user settings script
        . "${user_cfg}"
        
        # done with setting the config for per-user Xprt instances
        return 0;
    else
        # Use /etc/init.d/xprint's builtin config
        # Each entry should start with |setup_config_defaults| to pull the
        # platform defaults and finish with |num_xpstart=$(($num_xpstart + 1))|
        # to end the entry
        
        # Set platform-defaults
        setup_config_defaults "${num_xpstart}"

        ## -- snip --

        # Admins can put their stuff "in" here...

        ## -- snip --

        # Override script's builtin values with those a user may set via the
        # $ETC_INIITD_XPRINT_* env vars
        fetch_etc_initd_xprint_envvars "${num_xpstart}"

        num_xpstart=$((${num_xpstart} + 1))

        return 0;
    fi

    #### Sample 1:
    # # Start Xprt on a free display ID with custom XPCONFIGDIR and without
    # # Speedo and TrueType fonts
    #    xpstart_fontpath_rejectpattern[$num_xpstart]="/Speedo|/TrueType|/TT(/$|$)|/TTF(/$|$)";
    #               xpstart_xpconfigdir[$num_xpstart]="/home/gisburn/cwork/Xprint/Xprt_config/XpConfig";
    #                xpstart_auditlevel[$num_xpstart]="4";
    #                   xpstart_options[$num_xpstart]="-ac -pn";
    #num_xpstart=$(($num_xpstart + 1))


    #### Sample 2: 
    # # Start Xprt without TrueType fonts on a display 55 with custom
    # # XPCONFIGDIR
    #    xpstart_fontpath_rejectpattern[$num_xpstart]="/TrueType|/TT(/$|$)|/TTF(/$|$)";
    #                 xpstart_displayid[$num_xpstart]=55;
    #               xpstart_xpconfigdir[$num_xpstart]="/home/gisburn/cwork/Xprint/Xprt_config/XpConfig";
    #                xpstart_auditlevel[$num_xpstart]=4;
    #                   xpstart_options[$num_xpstart]="-ac -pn";
    #num_xpstart=$(($num_xpstart + 1))

    #### Sample 3: 
    # # Start Xprt without TrueType fonts on a display 56 with custom
    # # XPCONFIGDIR and alternate "Xprinters" file
    #    xpstart_fontpath_rejectpattern[$num_xpstart]="/TrueType|/TT(/$|$)|/TTF(/$|$)";
    #                 xpstart_displayid[$num_xpstart]=56;
    #               xpstart_xpconfigdir[$num_xpstart]="/etc/XpConfig/default";
    #                    xpstart_xpfile[$num_xpstart]="/etc/XpConfig/default/Xprinters_test2"
    #                xpstart_auditlevel[$num_xpstart]="4";
    #                   xpstart_options[$num_xpstart]="-ac -pn";
    #               xpstart_xprt_binary[$num_xpstart]="";
    #num_xpstart=$(($num_xpstart + 1))

    #### Sample 4:
    # # Start Xprt with Solaris ISO-8859-7 (greek(="el") locale) fonts on
    # # display 57
    #                  xpstart_fontpath[$num_xpstart]="/usr/openwin/lib/locale/iso_8859_7/X11/fonts/75dpi,/usr/openwin/lib/locale/iso_8859_7/X11/fonts/Type1,/usr/openwin/lib/X11/fonts/misc/";
    #    xpstart_fontpath_acceptpattern[$num_xpstart]="";
    #    xpstart_fontpath_rejectpattern[$num_xpstart]="_No_Match_";
    #                 xpstart_displayid[$num_xpstart]="57";
    #                xpstart_auditlevel[$num_xpstart]="4";
    #                   xpstart_options[$num_xpstart]="-ac -pn";
    #num_xpstart=$(($num_xpstart + 1))

    #### Sample 5:
    # # Start Xprt with the font list of an existing Xserver (excluding Speedo fonts) on
    # # display 58
    # # Note that this only works within a X session. At system boot time
    # # there will be no $DISPLAY to fetch the information from!!
    #                  xpstart_fontpath[$num_xpstart]="$(get_fontlist_from_display ${DISPLAY} | fontlist2fontpath)";
    #    xpstart_fontpath_acceptpattern[$num_xpstart]="";
    #    xpstart_fontpath_rejectpattern[$num_xpstart]="";
    #                 xpstart_displayid[$num_xpstart]="58";
    #               xpstart_xpconfigdir[$num_xpstart]="";
    #                xpstart_auditlevel[$num_xpstart]="4";
    #                   xpstart_options[$num_xpstart]="-ac -pn";
    #               xpstart_xprt_binary[$num_xpstart]="";
    #num_xpstart=$(($num_xpstart + 1))

    #### Sample 6:
    # # List remote Xprt's here 
    # # (note that there is no test to check whether these DISPLAYs are valid!)
    #             xpstart_remote_server[$num_xpstart]="sera:12"    ; num_xpstart=$(($num_xpstart + 1))
    #             xpstart_remote_server[$num_xpstart]="gandalf:19" ; num_xpstart=$(($num_xpstart + 1))   
}

XCOMM###########################################################################

XCOMM Main
case "$1" in
    ## Start Xprint servers
    'start')
        do_start
        ;;

    ## Stop Xprint servers
    # Note that this does _not_ kill Xprt instances started using this script 
    # by non-root users
    'stop')
        do_stop
        ;;

    ## Restart Xprint servers
    'restart'|'force-reload')
        do_restart
        ;;

    ## Reload configuration without stopping and restarting
    'reload')
        # not supported
        msg "reload not supported, use 'restart' or 'force-reload'"
        exit 3
        ;;

    ## Restart Xprint only if it is already running
    'condrestart'|'try-restart')
        # only restart if it is already running
        [ -f /var/lock/subsys/xprint ] && do_restart || :
        ;;
    
    ## Get list of all Xprint servers for this user
    # (incl. per-user and system-wide instances)    
    'get_xpserverlist')
        do_get_xpserverlist
        ;;

    ## Get status of Xprint servers, RedHat-style
    'status')
        x="$(do_get_xpserverlist)"
        if [ "${x}" != "" ] ; then
            msg "Xprint (${x}) is running..."
            exit 0
        else
            msg "Xprint is stopped"
            exit 3
        fi 
        ;;
        
    ## Wrapper
    'wrapper')
        cmd="${2}"
        [ "${cmd}" = "" ] && fatal_error "No command given."
        shift ; shift
        export XPSERVERLIST="$(do_get_xpserverlist)"
        [ "${XPSERVERLIST}" = "" ] && fatal_error "No Xprint servers found."
        exec "${cmd}" "$@"
        ;;

    ## Wrapper for "xplsprinters"
    'lsprinters')
        [                  "${ETC_INITD_XPRINT_XPLSPRINTERS_PATH}" != "" ] && cmd="${ETC_INITD_XPRINT_XPLSPRINTERS_PATH}"
        [ "${cmd}" = "" -a "${XPCUSTOMGLUE}" = "GISWxprintglue"          ] && cmd="/opt/GISWxprintglue/bin/xplsprinters"
        [ "${cmd}" = "" -a "${XPCUSTOMGLUE}" = "GISWxprint"              ] && cmd="/opt/GISWxprint/bin/xplsprinters"
        [ "${cmd}" = "" -a "${XPROJECTROOT}" != ""                       ] && cmd="${XPROJECTROOT}/bin/xplsprinters"
        [ "${cmd}" = ""                                                  ] && cmd="xplsprinters"

        shift
        export XPSERVERLIST="$(do_get_xpserverlist)"
        [ "${XPSERVERLIST}" = "" ] && fatal_error "No Xprint servers found."
        exec "${cmd}" "$@"
        ;;

    ## Diagnostics
    'diag')
        do_diag
        ;;
    
    ## Print usage
    *)
        msg "Usage: $0 { start | stop | restart | reload | force-reload | status | condrestart | try-restart | wrapper | lsprinters | get_xpserverlist | diag }"
        exit 2
esac
exit 0

XCOMM EOF.
