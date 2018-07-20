#include "RubyUtils/RubyUtils.h"

#include <vector>
#include <SketchUpAPI/sketchup.h>

VALUE AddFaceToModel() {
  SUModelRef model = SU_INVALID;
  SUResult result = SUModelCreate(&model);
  // It's best to always check the return code from the SU function call.
  // Only showing this check once to keep this example short.
  if (result != SU_ERROR_NONE)
    return Qfalse;
  // Get the root entities collection of the model.
  SUEntitiesRef entities = SU_INVALID;
  SUModelGetEntities(model, &entities);
  // Add a face to the model.
  std::vector<SUPoint3D> vertices = { {   0,   0,   0 },
                                      { 100, 100,   0 },
                                      { 100, 100,   0 },
                                      {   0,   0, 100 }, };
  SUFaceRef face = SU_INVALID;
  SUFaceCreateSimple(&face, vertices.data(), vertices.size());
  SUEntitiesAddFaces(entities, 1, &face);
  // Save the in-memory model to a file.
  SUModelSaveToFile(model, "new_model.skp");
  // Release the model when done to avoid memory leaks.
  SUModelRelease(&model);
  return Qtrue;
}

VALUE hello_slapi() {
  SUInitialize(); // Always initialize the API before using it.
  VALUE result = AddFaceToModel();
  SUTerminate(); // Always terminate the API after using it.
  return result;
}

VALUE read_default() {
  VALUE mSketchup = rb_const_get(rb_cObject, rb_intern("Sketchup"));
  // TODO: Use rb_protect to protect against Ruby errors. https://silverhammermba.github.io/emberb/c/#rb_protect
  VALUE value = rb_funcall(mSketchup, rb_intern("read_default"), 3,
      GetRubyInterface("my_dictionary"), GetRubyInterface("my_key"), GetRubyInterface("my_default"));
  if (RB_TYPE_P(value, T_STRING)) {
    auto c_string = StringValuePtr(value);
    // TODO: Do stuff to the C String here.
  }
  return value;
}

// Load this module from Ruby using:
//   require 'SUEX_HelloWorld'
extern "C"
void Init_SUEX_HelloWorld()
{
  VALUE mSUEX_HelloWorld = rb_define_module("SUEX_HelloWorld");
  rb_define_const(mSUEX_HelloWorld, "CEXT_VERSION", GetRubyInterface("1.0.0"));
  rb_define_module_function(mSUEX_HelloWorld, "hello_slapi", VALUEFUNC(hello_slapi), 0);
  rb_define_module_function(mSUEX_HelloWorld, "read_default", VALUEFUNC(read_default), 0);
}
