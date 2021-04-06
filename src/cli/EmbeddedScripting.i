#ifndef EMBEDDED_SCRIPTING_I
#define EMBEDDED_SCRIPTING_I

%module EmbeddedScripting

%include <std_string.i>

// DLM: including these files causes a crash, I don't know why
//%include <utilities/core/CommonInclude.i>
//%import <utilities/core/CommonImport.i>
//%import <utilities/Utilities.i>

%{
  #include <embedded_files.hxx>
  #include "EmbeddedHelp.hpp"
%}

//%ignore embedded_files::fileNames;

namespace embedded_files {

  %typemap(out) const std::vector<std::string> &fileNames {
    $result = rb_ary_new2((int)$1->size());
    int i = 0;

    std::vector<std::string>::const_iterator it = $1->begin();
    std::vector<std::string>::const_iterator ie = $1->end();

    for ( ; it != ie; ++it) {
      VALUE v = rb_str_new2(it->c_str());
      rb_ary_push($result, v);
    }
    rb_obj_freeze($result);  // treat as immutable result
    return $result;
  }
}

%include <embedded_files.hxx>
%include "EmbeddedHelp.hpp"

#endif // EMBEDDED_SCRIPTING_I
