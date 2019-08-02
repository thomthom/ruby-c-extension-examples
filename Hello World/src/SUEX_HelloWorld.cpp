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

VALUE get_scene_names() {
  auto model = GetActiveModel();

  VALUE names = rb_ary_new();

  size_t num_scenes = 0;
  SU(SUModelGetNumScenes(model, &num_scenes));
  
  std::vector<SUSceneRef> scenes(num_scenes, SU_INVALID);
  SU(SUModelGetScenes(model, num_scenes, scenes.data(), &num_scenes));

  for (const auto& scene : scenes) {
    SUStringRef name_ref = SU_INVALID;
    SU(SUStringCreate(&name_ref));
    SU(SUSceneGetName(scene, &name_ref));
    auto name = GetString(name_ref);
    rb_ary_push(names, GetRubyInterface(name.c_str()));
    SU(SUStringRelease(&name_ref));
  }

  return names;
}

VALUE symbols_test() {
  SUMaterialRef material = SU_INVALID;
  SUMaterialGetColorizeType(material, nullptr);

  SUTextureRef texture = SU_INVALID;
  SUTextureWriteOriginalToFile(texture, nullptr);

  SUModelRef model = SU_INVALID;
  size_t count = 0;
  SUModelGetNumImageDefinitions(model, nullptr);

  SUModelGetImageDefinitions(model, count, nullptr, nullptr);

  return Qnil;
}

VALUE get_selection_hack() {
  VALUE mSketchup = rb_const_get(rb_cObject, rb_intern("Sketchup"));
  VALUE model = rb_funcall(mSketchup, rb_intern("active_model"), 0);

  VALUE selection = rb_funcall(model, rb_intern("selection"), 0);
  size_t sel_size = NUM2SIZET(rb_funcall(selection, rb_intern("size"), 0));

  VALUE ids = rb_ary_new_capa(static_cast<long>(sel_size));

  std::vector<SUEntityRef> entities(sel_size, SU_INVALID);
  for (size_t i = 0; i < sel_size; i++) {
    VALUE entity = rb_funcall(selection, rb_intern("at"), 1, SIZET2NUM(i));
    assert(TYPE(entity) == T_DATA);
    void* data = DATA_PTR(entity);
    entities[i].ptr = data;
    assert(SUIsValid(entities[i]));
    int32_t id = 0;
    SU(SUEntityGetID(entities[i], &id));
    rb_ary_push(ids, INT2NUM(id));
  }

  return ids;
}


// Load this module from Ruby using:
//   require 'SUEX_HelloWorld'
//   SUEX_HelloWorld.is_section_active?
//   SUEX_HelloWorld.is_face_complex?
//   SUEX_HelloWorld.num_triangles
//   SUEX_HelloWorld.scene_names
//   SUEX_HelloWorld.get_selection
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

  rb_define_module_function(mSUEX_HelloWorld, "scene_names",
                            VALUEFUNC(get_scene_names), 0);

  rb_define_module_function(mSUEX_HelloWorld, "symbols",
                            VALUEFUNC(symbols_test), 0);

  rb_define_module_function(mSUEX_HelloWorld, "get_selection",
                            VALUEFUNC(get_selection_hack), 0);
}
