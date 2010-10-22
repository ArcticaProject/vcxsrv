/*  This file is part of mhmake.
 *
 *  Copyright (C) 2001-2010 marha@sourceforge.net
 *
 *  Mhmake is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Mhmake is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Mhmake.  If not, see <http://www.gnu.org/licenses/>.
*/

/* $Rev$ */

#ifndef __REFPTR_H__
#define __REFPTR_H__

#ifdef _DEBUG
#define PRINTF(arg) if (g_PrintLexYacc) printf arg
#else
#define PRINTF(arg)
#endif

template <class T> class iterstack: public stack<T>
{
  public:
    typedef typename deque<T>::iterator iterator;
    typedef typename deque<T>::reverse_iterator reverse_iterator;

    iterator begin()
    {
      return stack<T>::c.begin();
    }
    iterator end()
    {
      return stack<T>::c.end();
    }
    reverse_iterator rbegin()
    {
      return stack<T>::c.rbegin();
    }
    reverse_iterator rend()
    {
      return stack<T>::c.rend();
    }
};

struct refbase
{
  int m_Count;
  refbase()
  {
    m_Count=1;
  }
};

// Template class T needs to be derived from refbase;
template <class T> class refptr
{
  T *m_RefPtr;
public:
  refptr()
  {
    m_RefPtr=NULL;
  }
  refptr(T *Ptr) {
    m_RefPtr=Ptr;
  }
  refptr(const refptr<T> &RefPtr)
  {
    m_RefPtr=RefPtr.m_RefPtr;
    if (m_RefPtr)
      m_RefPtr->m_Count++;
  }
  ~refptr()
  {
    if (m_RefPtr && !--m_RefPtr->m_Count)
    {
      delete m_RefPtr;
    }
  }
  refptr<T> &operator=(const refptr<T>& Src)
  {
    if (Src.m_RefPtr!=m_RefPtr)
    {
      this->~refptr();
      new (this) refptr<T>(Src);
    }
    return *this;
  }

  refptr<T> &operator=(T *pPtr) // Assumes that T has reference count 1 and has no other users
  {
    #if defined(_DEBUG) && defined(_MSC_VER)
    if (pPtr && pPtr->m_Count!=1)
      DebugBreak();
    #endif
    this->~refptr();
    m_RefPtr=pPtr;
    return *this;
  }

  T & operator*() const
  {
    return *m_RefPtr;
  }
  T * operator->() const
  {
    return m_RefPtr;
  }
  operator T*() const
  {
    return m_RefPtr;
  }
};

#endif

