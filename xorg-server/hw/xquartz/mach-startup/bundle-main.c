/* main.c -- X application launcher
 
 Copyright (c) 2007 Jeremy Huddleston
 Copyright (c) 2007 Apple Inc
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation files
 (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
 HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 
 Except as contained in this notice, the name(s) of the above
 copyright holders shall not be used in advertising or otherwise to
 promote the sale, use or other dealings in this Software without
 prior written authorization. */

#include <CoreFoundation/CoreFoundation.h>
#include <AvailabilityMacros.h>

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/Xlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/un.h>

#include <sys/time.h>
#include <fcntl.h>

#include <mach/mach.h>
#include <mach/mach_error.h>
#include <servers/bootstrap.h>
#include "mach_startup.h"
#include "mach_startupServer.h"

#include "launchd_fd.h"
/* From darwinEvents.c ... but don't want to pull in all the server cruft */
void DarwinListenOnOpenFD(int fd);

extern int noPanoramiXExtension;

#define DEFAULT_CLIENT X11BINDIR "/xterm"
#define DEFAULT_STARTX X11BINDIR "/startx"
#define DEFAULT_SHELL  "/bin/sh"

#ifndef BUILD_DATE
#define BUILD_DATE ""
#endif
#ifndef XSERVER_VERSION
#define XSERVER_VERSION "?"
#endif

const int __crashreporter_info__len = 4096;
const char *__crashreporter_info__base = "X.Org X Server " XSERVER_VERSION " Build Date: " BUILD_DATE;
char __crashreporter_info__buf[4096];
char *__crashreporter_info__ = __crashreporter_info__buf;

static char *launchd_id_prefix = NULL;
static char *server_bootstrap_name = NULL;

#define DEBUG 1

/* This is in quartzStartup.c */
int server_main(int argc, char **argv, char **envp);

static int execute(const char *command);
static char *command_from_prefs(const char *key, const char *default_value);

/*** Pthread Magics ***/
static pthread_t create_thread(void *func, void *arg) {
    pthread_attr_t attr;
    pthread_t tid;
	
    pthread_attr_init (&attr);
    pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
    pthread_create (&tid, &attr, func, arg);
    pthread_attr_destroy (&attr);
	
    return tid;
}

/*** Mach-O IPC Stuffs ***/

union MaxMsgSize {
	union __RequestUnion__mach_startup_subsystem req;
	union __ReplyUnion__mach_startup_subsystem rep; 
};

static mach_port_t checkin_or_register(char *bname) {
    kern_return_t kr;
    mach_port_t mp;

    /* If we're started by launchd or the old mach_init */
    kr = bootstrap_check_in(bootstrap_port, bname, &mp);
    if (kr == KERN_SUCCESS)
        return mp;

    /* We probably were not started by launchd or the old mach_init */
    kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &mp);
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "mach_port_allocate(): %s\n", mach_error_string(kr));
        exit(EXIT_FAILURE);
    }

    kr = mach_port_insert_right(mach_task_self(), mp, mp, MACH_MSG_TYPE_MAKE_SEND);
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "mach_port_insert_right(): %s\n", mach_error_string(kr));
        exit(EXIT_FAILURE);
    }

    kr = bootstrap_register(bootstrap_port, bname, mp);
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "bootstrap_register(): %s\n", mach_error_string(kr));
        exit(EXIT_FAILURE);
    }

    return mp;
}

/*** $DISPLAY handoff ***/
static int accept_fd_handoff(int connected_fd) {
    int launchd_fd;
    
    char databuf[] = "display";
    struct iovec iov[1];
    
    union {
        struct cmsghdr hdr;
        char bytes[CMSG_SPACE(sizeof(int))];
    } buf;
    
    struct msghdr msg;
    struct cmsghdr *cmsg;

    iov[0].iov_base = databuf;
    iov[0].iov_len  = sizeof(databuf);
    
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_control = buf.bytes;
    msg.msg_controllen = sizeof(buf);
    msg.msg_name = 0;
    msg.msg_namelen = 0;
    msg.msg_flags = 0;
    
    cmsg = CMSG_FIRSTHDR (&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    
    msg.msg_controllen = cmsg->cmsg_len;
    
    *((int*)CMSG_DATA(cmsg)) = -1;
    
    if(recvmsg(connected_fd, &msg, 0) < 0) {
        fprintf(stderr, "X11.app: Error receiving $DISPLAY file descriptor.  recvmsg() error: %s\n", strerror(errno));
        return -1;
    }
    
    launchd_fd = *((int*)CMSG_DATA(cmsg));
    
    return launchd_fd;
}

typedef struct {
    int fd;
    string_t filename;
} socket_handoff_t;

/* This thread accepts an incoming connection and hands off the file
 * descriptor for the new connection to accept_fd_handoff()
 */
static void socket_handoff_thread(void *arg) {
    socket_handoff_t *handoff_data = (socket_handoff_t *)arg;
    int launchd_fd = -1;
    int connected_fd;
    unsigned remain;

    /* Now actually get the passed file descriptor from this connection
     * If we encounter an error, keep listening.
     */
    while(launchd_fd == -1) {
        connected_fd = accept(handoff_data->fd, NULL, NULL);
        if(connected_fd == -1) {
            fprintf(stderr, "X11.app: Failed to accept incoming connection on socket (fd=%d): %s\n", handoff_data->fd, strerror(errno));
            sleep(2);
            continue;
        }

        launchd_fd = accept_fd_handoff(connected_fd);
        if(launchd_fd == -1)
            fprintf(stderr, "X11.app: Error receiving $DISPLAY file descriptor, no descriptor received?  Waiting for another connection.\n");

        close(connected_fd);
    }

    close(handoff_data->fd);
    unlink(handoff_data->filename);
    free(handoff_data);
    
    /* TODO: Clean up this race better... giving xinitrc time to run... need to wait for 1.5 branch:
     *
     * From ajax:
     * There's already an internal callback chain for setting selection [in 1.5]
     * ownership.  See the CallSelectionCallback at the bottom of
     * ProcSetSelectionOwner, and xfixes/select.c for an example of how to hook
     * into it.
     */
    
    remain = 3000000;
    fprintf(stderr, "X11.app: Received new $DISPLAY fd: %d ... sleeping to allow xinitrc to catchup.\n", launchd_fd);
    while((remain = usleep(remain)) > 0);
    
    fprintf(stderr, "X11.app Handing off fd to server thread via DarwinListenOnOpenFD(%d)\n", launchd_fd);
    DarwinListenOnOpenFD(launchd_fd);
}

static int create_socket(char *filename_out) {
    struct sockaddr_un servaddr_un;
    struct sockaddr *servaddr;
    socklen_t servaddr_len;
    int ret_fd;
    size_t try, try_max;
    
    for(try=0, try_max=5; try < try_max; try++) {
        tmpnam(filename_out);
        
        /* Setup servaddr_un */
        memset (&servaddr_un, 0, sizeof (struct sockaddr_un));
        servaddr_un.sun_family = AF_UNIX;
        strlcpy(servaddr_un.sun_path, filename_out, sizeof(servaddr_un.sun_path));
        
        servaddr = (struct sockaddr *) &servaddr_un;
        servaddr_len = sizeof(struct sockaddr_un) - sizeof(servaddr_un.sun_path) + strlen(filename_out);
        
        ret_fd = socket(PF_UNIX, SOCK_STREAM, 0);
        if(ret_fd == -1) {
            fprintf(stderr, "X11.app: Failed to create socket (try %d / %d): %s - %s\n", (int)try+1, (int)try_max, filename_out, strerror(errno));
            continue;
        }
        
        if(bind(ret_fd, servaddr, servaddr_len) != 0) {
            fprintf(stderr, "X11.app: Failed to bind socket: %d - %s\n", errno, strerror(errno));
            close(ret_fd);
            return 0;
        }
        
        if(listen(ret_fd, 10) != 0) {
            fprintf(stderr, "X11.app: Failed to listen to socket: %s - %d - %s\n", filename_out, errno, strerror(errno));
            close(ret_fd);
            return 0;
        }
        
#ifdef DEBUG
        fprintf(stderr, "X11.app: Listening on socket for fd handoff:  (%d) %s\n", ret_fd, filename_out);
#endif
        
        return ret_fd;
    }
    
    return 0;
}

static int launchd_socket_handed_off = 0;

kern_return_t do_request_fd_handoff_socket(mach_port_t port, string_t filename) {
    socket_handoff_t *handoff_data;
    
    launchd_socket_handed_off = 1;

    handoff_data = (socket_handoff_t *)calloc(1,sizeof(socket_handoff_t));
    if(!handoff_data) {
        fprintf(stderr, "X11.app: Error allocating memory for handoff_data\n");
        return KERN_FAILURE;
    }

    handoff_data->fd = create_socket(handoff_data->filename);
    if(!handoff_data->fd) {
        free(handoff_data);
        return KERN_FAILURE;
    }

    strlcpy(filename, handoff_data->filename, STRING_T_SIZE);
    
    create_thread(socket_handoff_thread, handoff_data);
    
#ifdef DEBUG
    fprintf(stderr, "X11.app: Thread created for handoff.  Returning success to tell caller to connect and push the fd.\n");
#endif

    return KERN_SUCCESS;
}

kern_return_t do_request_pid(mach_port_t port, int *my_pid) {
    *my_pid = getpid();
    return KERN_SUCCESS;
}

/*** Server Startup ***/
kern_return_t do_start_x11_server(mach_port_t port, string_array_t argv,
                                  mach_msg_type_number_t argvCnt,
                                  string_array_t envp,
                                  mach_msg_type_number_t envpCnt) {
    /* And now back to char ** */
    char **_argv = alloca((argvCnt + 1) * sizeof(char *));
    char **_envp = alloca((envpCnt + 1) * sizeof(char *));
    size_t i;
    
    /* If we didn't get handed a launchd DISPLAY socket, we should
     * unset DISPLAY or we can run into problems with pbproxy
     */
    if(!launchd_socket_handed_off)
        unsetenv("DISPLAY");
    
    if(!_argv || !_envp) {
        return KERN_FAILURE;
    }

    fprintf(stderr, "X11.app: do_start_x11_server(): argc=%d\n", argvCnt);
    for(i=0; i < argvCnt; i++) {
        _argv[i] = argv[i];
        fprintf(stderr, "\targv[%u] = %s\n", (unsigned)i, argv[i]);
    }
    _argv[argvCnt] = NULL;
    
    for(i=0; i < envpCnt; i++) {
        _envp[i] = envp[i];
    }
    _envp[envpCnt] = NULL;
    
    if(server_main(argvCnt, _argv, _envp) == 0)
        return KERN_SUCCESS;
    else
        return KERN_FAILURE;
}

static int startup_trigger(int argc, char **argv, char **envp) {
    Display *display;
    const char *s;
    
    /* Take care of the case where we're called like a normal DDX */
    if(argc > 1 && argv[1][0] == ':') {
        size_t i;
        kern_return_t kr;
        mach_port_t mp;
        string_array_t newenvp;
        string_array_t newargv;

        /* We need to count envp */
        int envpc;
        for(envpc=0; envp[envpc]; envpc++);

        /* We have fixed-size string lengths due to limitations in IPC,
         * so we need to copy our argv and envp.
         */
        newargv = (string_array_t)alloca(argc * sizeof(string_t));
        newenvp = (string_array_t)alloca(envpc * sizeof(string_t));
        
        if(!newargv || !newenvp) {
            fprintf(stderr, "Memory allocation failure\n");
            exit(EXIT_FAILURE);
        }
        
        for(i=0; i < argc; i++) {
            strlcpy(newargv[i], argv[i], STRING_T_SIZE);
        }
        for(i=0; i < envpc; i++) {
            strlcpy(newenvp[i], envp[i], STRING_T_SIZE);
        }

        kr = bootstrap_look_up(bootstrap_port, server_bootstrap_name, &mp);
        if (kr != KERN_SUCCESS) {
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1050
            fprintf(stderr, "bootstrap_look_up(%s): %s\n", server_bootstrap_name, bootstrap_strerror(kr));
#else
            fprintf(stderr, "bootstrap_look_up(%s): %ul\n", server_bootstrap_name, (unsigned long)kr);
#endif
            exit(EXIT_FAILURE);
        }

        kr = start_x11_server(mp, newargv, argc, newenvp, envpc);
        if (kr != KERN_SUCCESS) {
            fprintf(stderr, "start_x11_server: %s\n", mach_error_string(kr));
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    /* If we have a process serial number and it's our only arg, act as if
     * the user double clicked the app bundle: launch app_to_run if possible
     */
    if(argc == 1 || (argc == 2 && !strncmp(argv[1], "-psn_", 5))) {
        /* Now, try to open a display, if so, run the launcher */
        display = XOpenDisplay(NULL);
        if(display) {
            /* Could open the display, start the launcher */
            XCloseDisplay(display);

            return execute(command_from_prefs("app_to_run", DEFAULT_CLIENT));
        }
    }

    /* Start the server */
    if((s = getenv("DISPLAY"))) {
        fprintf(stderr, "X11.app: Could not connect to server (DISPLAY=\"%s\", unsetting).  Starting X server.\n", s);
        unsetenv("DISPLAY");
    } else {
        fprintf(stderr, "X11.app: Could not connect to server (DISPLAY is not set).  Starting X server.\n");
    }
    return execute(command_from_prefs("startx_script", DEFAULT_STARTX));
}

/** Setup the environment we want our child processes to inherit */
static void ensure_path(const char *dir) {
    char buf[1024], *temp;
    
    /* Make sure /usr/X11/bin is in the $PATH */
    temp = getenv("PATH");
    if(temp == NULL || temp[0] == 0) {
        snprintf(buf, sizeof(buf), "/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:%s", dir);
        setenv("PATH", buf, TRUE);
    } else if(strnstr(temp, X11BINDIR, sizeof(temp)) == NULL) {
        snprintf(buf, sizeof(buf), "%s:%s", temp, dir);
        setenv("PATH", buf, TRUE);
    }
}

static void setup_env(void) {
    char *temp;
    const char *pds = NULL;
    const char *disp = getenv("DISPLAY");
    size_t len;

    /* Pass on our prefs domain to startx and its inheritors (mainly for
     * quartz-wm and the Xquartz stub's MachIPC)
     */
    CFBundleRef bundle = CFBundleGetMainBundle();
    if(bundle) {
        CFStringRef pd = CFBundleGetIdentifier(bundle);
        if(pd) {
            pds = CFStringGetCStringPtr(pd, 0);
        }
    }

    /* fallback to hardcoded value if we can't discover it */
    if(!pds) {
        pds = LAUNCHD_ID_PREFIX".X11";
    }

    server_bootstrap_name = malloc(sizeof(char) * (strlen(pds) + 1));
    if(!server_bootstrap_name) {
        fprintf(stderr, "Memory allocation error.\n");
        exit(1);
    }
    strcpy(server_bootstrap_name, pds);
    setenv("X11_PREFS_DOMAIN", server_bootstrap_name, 1);
    
    len = strlen(server_bootstrap_name);
    launchd_id_prefix = malloc(sizeof(char) * (len - 3));
    if(!launchd_id_prefix) {
        fprintf(stderr, "Memory allocation error.\n");
        exit(1);
    }
    strlcpy(launchd_id_prefix, server_bootstrap_name, len - 3);
    
    /* We need to unset DISPLAY if it is not our socket */
    if(disp) {
        /* s = basename(disp) */
        const char *d, *s;
	    for(s = NULL, d = disp; *d; d++) {
            if(*d == '/')
                s = d + 1;
        }

        if(s && *s) {
            temp = (char *)malloc(sizeof(char) * len);
            if(!temp) {
                fprintf(stderr, "Memory allocation error creating space for socket name test.\n");
                exit(1);
            }
            strlcpy(temp, launchd_id_prefix, len);
            strlcat(temp, ":0", len);
            
            if(strcmp(temp, s) != 0) {
                /* If we don't have a match, unset it. */
                unsetenv("DISPLAY");
            }
            free(temp);
        } else {
            /* The DISPLAY environment variable is not formatted like a launchd socket, so reset. */
            unsetenv("DISPLAY");
        }
    }

    /* Make sure PATH is right */
    ensure_path(X11BINDIR);
    
    /* cd $HOME */
    temp = getenv("HOME");
    if(temp != NULL && temp[0] != '\0')
        chdir(temp);
}

/*** Main ***/
int main(int argc, char **argv, char **envp) {
    Bool listenOnly = FALSE;
    int i;
    mach_msg_size_t mxmsgsz = sizeof(union MaxMsgSize) + MAX_TRAILER_SIZE;
    mach_port_t mp;
    kern_return_t kr;

    /* Setup our environment for our children */
    setup_env();
    
    /* The server must not run the PanoramiX operations. */
    noPanoramiXExtension = TRUE;

    /* Setup the initial crasherporter info */
    strlcpy(__crashreporter_info__, __crashreporter_info__base, __crashreporter_info__len);
    
    fprintf(stderr, "X11.app: main(): argc=%d\n", argc);
    for(i=0; i < argc; i++) {
        fprintf(stderr, "\targv[%u] = %s\n", (unsigned)i, argv[i]);
        if(!strcmp(argv[i], "--listenonly")) {
            listenOnly = TRUE;
        }
    }

    mp = checkin_or_register(server_bootstrap_name);
    if(mp == MACH_PORT_NULL) {
        fprintf(stderr, "NULL mach service: %s", server_bootstrap_name);
        return EXIT_FAILURE;
    }
    
    /* Check if we need to do something other than listen, and make another
     * thread handle it.
     */
    if(!listenOnly) {
        pid_t child1, child2;
        int status;

        /* Do the fork-twice trick to avoid having to reap zombies */
        child1 = fork();
        switch (child1) {
            case -1:                                /* error */
                break;

            case 0:                                 /* child1 */
                child2 = fork();

                switch (child2) {
                    int max_files, i;

                    case -1:                            /* error */
                        break;

                    case 0:                             /* child2 */
                        /* close all open files except for standard streams */
                        max_files = sysconf(_SC_OPEN_MAX);
                        for(i = 3; i < max_files; i++)
                            close(i);

                        /* ensure stdin is on /dev/null */
                        close(0);
                        open("/dev/null", O_RDONLY);

                        return startup_trigger(argc, argv, envp);

                    default:                            /* parent (child1) */
                        _exit(0);
                }
                break;

            default:                                /* parent */
              waitpid(child1, &status, 0);
        }
    }
    
    /* Main event loop */
    fprintf(stderr, "Waiting for startup parameters via Mach IPC.\n");
    kr = mach_msg_server(mach_startup_server, mxmsgsz, mp, 0);
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "%s.X11(mp): %s\n", LAUNCHD_ID_PREFIX, mach_error_string(kr));
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

static int execute(const char *command) {
    const char *newargv[4];
    const char **p;
    
    newargv[0] = command_from_prefs("login_shell", DEFAULT_SHELL);
    newargv[1] = "-c";
    newargv[2] = command;
    newargv[3] = NULL;
    
    fprintf(stderr, "X11.app: Launching %s:\n", command);
    for(p=newargv; *p; p++) {
        fprintf(stderr, "\targv[%ld] = %s\n", (long int)(p - newargv), *p);
    }

    execvp (newargv[0], (char * const *) newargv);
    perror ("X11.app: Couldn't exec.");
    return(1);
}

static char *command_from_prefs(const char *key, const char *default_value) {
    char *command = NULL;
    
    CFStringRef cfKey = CFStringCreateWithCString(NULL, key, kCFStringEncodingASCII);
    CFPropertyListRef PlistRef = CFPreferencesCopyAppValue(cfKey, kCFPreferencesCurrentApplication);
    
    if ((PlistRef == NULL) || (CFGetTypeID(PlistRef) != CFStringGetTypeID())) {
        CFStringRef cfDefaultValue = CFStringCreateWithCString(NULL, default_value, kCFStringEncodingASCII);
        int len = strlen(default_value) + 1;

        CFPreferencesSetAppValue(cfKey, cfDefaultValue, kCFPreferencesCurrentApplication);
        CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
        
        command = (char *)malloc(len * sizeof(char));
        if(!command)
            return NULL;
        strcpy(command, default_value);
    } else {
        int len = CFStringGetLength((CFStringRef)PlistRef) + 1;
        command = (char *)malloc(len * sizeof(char));
        if(!command)
            return NULL;
        CFStringGetCString((CFStringRef)PlistRef, command, len,  kCFStringEncodingASCII);
	}
    
    if (PlistRef)
        CFRelease(PlistRef);
    
    return command;
}
