/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file genericThread.h
 * @author drose
 * @date 2011-11-09
 */

#ifndef GENERICTHREAD_H
#define GENERICTHREAD_H

#include "pandabase.h"
#include "thread.h"

#ifndef CPPPARSER
#include <functional>

/**
 * A generic thread type that allows calling a C-style thread function without
 * having to subclass.
 */
class EXPCL_PANDA_PIPELINE GenericThread : public Thread {
public:
  typedef void ThreadFunc(void *user_data);

  GenericThread(const std::string &name, const std::string &sync_name);
  GenericThread(const std::string &name, const std::string &sync_name, ThreadFunc *function, void *user_data);
  GenericThread(const std::string &name, const std::string &sync_name, std::function<void()> function);

  INLINE void set_function(std::function<void()> function);
  INLINE const std::function<void()> &get_function() const;

protected:
  virtual void thread_main();

private:
  std::function<void()> _function;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Thread::init_type();
    register_type(_type_handle, "GenericThread",
                  Thread::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "genericThread.I"

#endif  // CPPPARSER

#endif  // GENERICTHREAD_H
