
#include "RubyUtils/RubyUtils.h"


VALUE current_thread() {
  VALUE thread = rb_thread_current();
  return rb_obj_id(thread);
}

VALUE current_fiber() {
  VALUE fiber = rb_fiber_current();
  return rb_obj_id(fiber);
}


VALUE hello_world() {
  return GetRubyInterface("Hello World!");
}

// Load this module from Ruby using:
//   require 'SUEX_HelloWorld'
extern "C"
void Init_SUEX_HelloWorld()
{
  VALUE mSUEX_HelloWorld = rb_define_module("SUEX_HelloWorld");
  rb_define_const(mSUEX_HelloWorld, "CEXT_VERSION", GetRubyInterface("1.0.0"));
  rb_define_module_function(mSUEX_HelloWorld, "hello_world", VALUEFUNC(hello_world), 0);
  rb_define_module_function(mSUEX_HelloWorld, "current_thread", VALUEFUNC(current_thread), 0);
  rb_define_module_function(mSUEX_HelloWorld, "current_fiber", VALUEFUNC(current_fiber), 0);
}
