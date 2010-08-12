// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_DOC_FRAG_H_
#define SRC_XML_DOC_FRAG_H_

#include "libxmljs.h"
#include "xml_node.h"
#include "xml_element.h"

namespace libxmljs {

class XmlDocFrag : public XmlNode {
  public:

  explicit XmlDocFrag(xmlNode* node) : XmlNode(node) {}

  static void Initialize(v8::Handle<v8::Object> target);
  static v8::Persistent<v8::FunctionTemplate> constructor_template;

  protected:

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> Name(const v8::Arguments& args);
  static v8::Handle<v8::Value> Find(const v8::Arguments& args);
  static v8::Handle<v8::Value> Text(const v8::Arguments& args);
  static v8::Handle<v8::Value> Path(const v8::Arguments& args);
  static v8::Handle<v8::Value> Child(const v8::Arguments& args);
  static v8::Handle<v8::Value> ChildNodes(const v8::Arguments& args);
  static v8::Handle<v8::Value> AddChild(const v8::Arguments& args);

  void set_name(const char* name);
  v8::Handle<v8::Value> get_name();
  v8::Handle<v8::Value> get_child(double idx);
  v8::Handle<v8::Value> get_child_nodes();
  v8::Handle<v8::Value> get_path();
  v8::Handle<v8::Value> clone(bool deep);
  void add_child(XmlElement* child);
  void set_content(const char* content);
  v8::Handle<v8::Value> get_content();
  XmlElement *import_element(XmlElement* element);
};

}  // namespace libxmljs

#endif  // SRC_XML_DOC_FRAG_H_
