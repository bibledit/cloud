/*
 Copyright (©) 2003-2016 Teus Benschop.
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


// Config.h to be included.
#include <config.h>
#include <config/config.h>
#include <library/locks.h>
#ifndef HAVE_CLIENT
#include <openssl/crypto.h>
#include <pthread.h>


#define MUTEX_TYPE       pthread_mutex_t
#define MUTEX_SETUP(x)   pthread_mutex_init(&(x), NULL)
#define MUTEX_CLEANUP(x) pthread_mutex_destroy(&(x))
#define MUTEX_LOCK(x)    pthread_mutex_lock(&(x))
#define MUTEX_UNLOCK(x)  pthread_mutex_unlock(&(x))
#define THREAD_ID        pthread_self(  )


/* This array will store all of the mutexes available to OpenSSL. */
static MUTEX_TYPE *mutex_buf = NULL;


static void locking_function(int mode, int n, const char * file, int line)
{
  if (mode & CRYPTO_LOCK)
    MUTEX_LOCK(mutex_buf[n]);
  else
    MUTEX_UNLOCK(mutex_buf[n]);
  if (file && line) {}
}


static unsigned long id_function(void)
{
  return ((unsigned long)THREAD_ID);
}


#endif


void thread_setup ()
{
#ifndef HAVE_CLIENT
  mutex_buf = malloc (CRYPTO_num_locks () * sizeof(MUTEX_TYPE));
  if (!mutex_buf) return;
  int i;
  for (i = 0;  i < CRYPTO_num_locks ();  i++)
    MUTEX_SETUP (mutex_buf[i]);
  CRYPTO_set_id_callback (id_function);
  CRYPTO_set_locking_callback (locking_function);
#endif
}


void thread_cleanup ()
{
#ifndef HAVE_CLIENT
  if (!mutex_buf) return;
  CRYPTO_set_id_callback (NULL);
  CRYPTO_set_locking_callback (NULL);
  int i;
  for (i = 0;  i < CRYPTO_num_locks ();  i++)
    MUTEX_CLEANUP (mutex_buf[i]);
  free (mutex_buf);
  mutex_buf = NULL;
#endif
}
