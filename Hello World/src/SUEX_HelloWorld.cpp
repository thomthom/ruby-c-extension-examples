#include "RubyUtils/RubyUtils.h"

#include <assert.h>
#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include "SketchUpAPI/sketchup.h"
#else
#import <SketchUpAPI/sketchup.h>
#endif


#define SU(api_function_call) {\
SUResult su_api_result = api_function_call;\
assert(SU_ERROR_NONE == su_api_result);\
}\

#define refute(condition) assert(!(condition))


//extern "C" {
//SU_RESULT SUApplicationGetActiveModel(SUModelRef* model);
//}


std::string GetString(const SUStringRef& string) {
  size_t length = 0;
  SU(SUStringGetUTF8Length(string, &length));
  std::vector<char> buffer(length + 1);
  size_t out_length = 0;
  SU(SUStringGetUTF8(string, length + 1, buffer.data(), &out_length));
  assert(out_length == length);
  return std::string(begin(buffer), end(buffer));
}

SUModelRef GetActiveModel() {
  //VALUE sketchup = rb_define_module("Sketchup");
  //VALUE model = rb_funcall(sketchup, rb_intern("active_model"), 0);
  //VALUE address = rb_funcall(model, rb_intern("skpdoc"), 1, Qtrue);
  //return SUModelFromExisting(NUM2SIZET(address));
  SUModelRef model = SU_INVALID;
  SU(SUApplicationGetActiveModel(&model));
  return model;
}


VALUE hello_world() {
  return GetRubyInterface("Hello World!");
}

VALUE is_section_active() {
  auto model = GetActiveModel();
  
  SUEntitiesRef entities = SU_INVALID;
  SU(SUModelGetEntities(model, &entities));
  
  size_t num_sections = 0;
  SU(SUEntitiesGetNumSectionPlanes(entities, &num_sections));
  
  std::vector<SUSectionPlaneRef> sections(num_sections, SU_INVALID);
  if (num_sections > 0) {
    SU(SUEntitiesGetSectionPlanes(entities, num_sections, sections.data(), &num_sections));
  }
  
  if (sections.size() > 0) {
    auto section = sections[0];
    bool is_active = false;
    SUSectionPlaneIsActive(section, &is_active);
    return is_active ? Qtrue : Qfalse;
  }
  
  return Qnil;
}

VALUE is_face_complex() {
  auto model = GetActiveModel();

  SUEntitiesRef root_entities = SU_INVALID;
  SU(SUModelGetEntities(model, &root_entities));

  size_t num_groups = 0;
  SU(SUEntitiesGetNumGroups(root_entities, &num_groups));
  if (num_groups == 0)
    return Qnil;

  SUGroupRef group = SU_INVALID;
  SU(SUEntitiesGetGroups(root_entities, 1, &group, &num_groups));

  SUEntitiesRef entities = SU_INVALID;
  SU(SUGroupGetEntities(group, &entities));

  size_t num_faces = 0;
  SU(SUEntitiesGetNumFaces(entities, &num_faces));

  std::vector<SUFaceRef> faces(num_faces, SU_INVALID);
  if (num_faces > 0) {
    SU(SUEntitiesGetFaces(entities, num_faces, faces.data(), &num_faces));
  }

  if (faces.size() > 0) {
    auto face = faces[0];
    bool is_complex = false;
    SUFaceIsComplex(face, &is_complex);
    return is_complex ? Qtrue : Qfalse;
  }

  return Qnil;
}

VALUE num_triangles() {
  auto model = GetActiveModel();

  SUEntitiesRef root_entities = SU_INVALID;
  SU(SUModelGetEntities(model, &root_entities));

  size_t num_groups = 0;
  SU(SUEntitiesGetNumGroups(root_entities, &num_groups));
  if (num_groups == 0)
    return Qnil;

  SUGroupRef group = SU_INVALID;
  SU(SUEntitiesGetGroups(root_entities, 1, &group, &num_groups));

  SUEntitiesRef entities = SU_INVALID;
  SU(SUGroupGetEntities(group, &entities));

  size_t num_faces = 0;
  SU(SUEntitiesGetNumFaces(entities, &num_faces));

  std::vector<SUFaceRef> faces(num_faces, SU_INVALID);
  if (num_faces > 0) {
    SU(SUEntitiesGetFaces(entities, num_faces, faces.data(), &num_faces));
  }

  if (faces.size() > 0) {
    auto face = faces[0];

    SUMeshHelperRef mesh = SU_INVALID;
    SU(SUMeshHelperCreate(&mesh, face));

    size_t num_triangles = 0;
    SU(SUMeshHelperGetNumTriangles(mesh, &num_triangles));

    return SIZET2NUM(num_triangles);
  }

  return Qnil;
}

// Load this module from Ruby using:
//   require 'SUEX_HelloWorld'
//   SUEX_HelloWorld.is_section_active?
//   SUEX_HelloWorld.is_face_complex?
//   SUEX_HelloWorld.num_triangles
extern "C"
void Init_SUEX_HelloWorld()
{
  VALUE mSUEX_HelloWorld = rb_define_module("SUEX_HelloWorld");
  
  rb_define_const(mSUEX_HelloWorld, "CEXT_VERSION", GetRubyInterface("1.0.0"));
  
  rb_define_module_function(mSUEX_HelloWorld, "hello_world",
    VALUEFUNC(hello_world), 0);
  
  rb_define_module_function(mSUEX_HelloWorld, "is_section_active?",
                            VALUEFUNC(is_section_active), 0);

  rb_define_module_function(mSUEX_HelloWorld, "is_face_complex?",
                            VALUEFUNC(is_face_complex), 0);

  rb_define_module_function(mSUEX_HelloWorld, "num_triangles",
                            VALUEFUNC(num_triangles), 0);
}
