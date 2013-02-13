/*
 * Test for pthread_timedjoin_np() timing out.
 *
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2012 Pthreads-win32 contributors
 *
 *      Homepage1: http://sourceware.org/pthreads-win32/
 *      Homepage2: http://sourceforge.net/projects/pthreads4w/
 *
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *      http://sources.redhat.com/pthreads-win32/contributors.html
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * --------------------------------------------------------------------------
 *
 * Depends on API functions: pthread_create().
 */

#include "test.h"

void *
func(void * arg)
{
        Sleep(1200);
        return arg;
}

int
main(int argc, char * argv[])
{
        pthread_t id;
        struct timespec abstime;
        void* result = (void*)-1;
        PTW32_STRUCT_TIMEB currSysTime;
        const DWORD NANOSEC_PER_MILLISEC = 1000000;

        assert(pthread_create(&id, NULL, func, (void *)(size_t)999) == 0);

        /*
         * Let thread start before we attempt to join it.
         */
        Sleep(100);

        PTW32_FTIME(&currSysTime);

        abstime.tv_sec = (long)currSysTime.time;
        abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;

        /* Test for pthread_timedjoin_np timeout */
        abstime.tv_sec += 1;
        assert(pthread_timedjoin_np(id, &result, &abstime) == ETIMEDOUT);
        assert((int)(size_t)result == -1);

        /* Test for pthread_tryjoin_np behaviour before thread has exited */
        assert(pthread_tryjoin_np(id, &result) == EBUSY);
        assert((int)(size_t)result == -1);

        Sleep(500);

        /* Test for pthread_tryjoin_np behaviour after thread has exited */
        assert(pthread_tryjoin_np(id, &result) == 0);
        assert((int)(size_t)result == 999);

        /* Success. */
        return 0;
}
