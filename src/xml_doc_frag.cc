// Copyright 2009, Squish Tech, LLC.
#include "xml_doc_frag.h"
#include "xml_element.h"
#include "xml_document.h"
#include "xml_attribute.h"
#include "xml_xpath_context.h"

namespace libxmljs {

#define NAME_SYMBOL     v8::String::NewSymbol("name")
#define CONTENT_SYMBOL  v8::String::NewSymbol("content")

v8::Persistent<v8::FunctionTemplate> XmlDocFrag::constructor_template;

// doc, name, attrs, content, callback
v8::Handle<v8::Value>
XmlDocFrag::New(const v8::Arguments& args) {
  v8::HandleScope scope;
  // was created by BUILD_NODE
  if (args.Length() == 0 || args[0]->StrictEquals(v8::Null()))
      return scope.Close(args.This());

  XmlDocument *document = LibXmlObj::Unwrap<XmlDocument>(args[0]->ToObject());
  v8::String::Utf8Value name(args[1]);

  v8::Handle<v8::Function> callback;

  char* content = NULL;
  if(!args[3]->IsUndefined() && !args[3]->IsNull()) {
      v8::String::Utf8Value content_(args[3]);
      content = strdup(*content_);
  }

  if (args[4]->IsFunction())
    callback = v8::Handle<v8::Function>::Cast(args[4]);

  xmlNode* elem = xmlNewDocNode(document->xml_obj,
                                NULL,
                                (const xmlChar*)*name,
                                (const xmlChar*)content);
  UpdateV8Memory();

  if(content)
      free(content);

  v8::Handle<v8::Object> obj =
      LibXmlObj::GetMaybeBuild<XmlDocFrag, xmlNode>(elem);
  XmlDocFrag *frag = LibXmlObj::Unwrap<XmlDocFrag>(obj);


  if (args[2]->IsObject()) {
    v8::Handle<v8::Object> attributes = args[2]->ToObject();
    v8::Handle<v8::Array> props = attributes->GetPropertyNames();
    for (unsigned int i = 0; i < props->Length(); i++) {
      v8::String::Utf8Value name(
        props->Get(v8::Number::New(i)));
      v8::String::Utf8Value value(
        attributes->Get(props->Get(v8::Number::New(i))));
//      element->set_attr(*name, *value);
    }
  }

  obj->Set(v8::String::NewSymbol("document"), args[0]->ToObject());

  if (*callback && !callback->IsNull()) {
    v8::Handle<v8::Value> argv[1] = { obj };
    *callback->Call(obj, 1, argv);
  }

  return scope.Close(obj);
}

v8::Handle<v8::Value>
XmlDocFrag::Name(const v8::Arguments& args) {
  v8::HandleScope scope;
  XmlDocFrag *frag = LibXmlObj::Unwrap<XmlDocFrag>(args.This());
  assert(frag);

  if (args.Length() == 0)
      return scope.Close(frag->get_name());

  v8::String::Utf8Value name(args[0]->ToString());
  frag->set_name(*name);
  return scope.Close(args.This());
}

v8::Handle<v8::Value>
XmlDocFrag::AddChild(const v8::Arguments& args) {
  v8::HandleScope scope;
  XmlDocFrag *frag = LibXmlObj::Unwrap<XmlDocFrag>(args.This());
  assert(frag);

  XmlElement *child = LibXmlObj::Unwrap<XmlElement>(args[0]->ToObject());
  assert(child);

  child = frag->import_element(child);

  if(child == NULL) {
      LIBXMLJS_THROW_EXCEPTION("Could not add child. Failed to copy node to new Document.");
  }

  frag->add_child(child);
  return scope.Close(LibXmlObj::GetMaybeBuild<XmlElement, xmlNode>(child->xml_obj));
}

v8::Handle<v8::Value>
XmlDocFrag::Find(const v8::Arguments& args) {
  v8::HandleScope scope;
  XmlDocFrag *frag = LibXmlObj::Unwrap<XmlDocFrag>(args.This());
  assert(frag);

  v8::String::Utf8Value xpath(args[0]);

  XmlXpathContext ctxt(frag->xml_obj);

  if (args.Length() == 2) {
    if (args[1]->IsString()) {
      v8::String::Utf8Value uri(args[1]);
      ctxt.register_ns((const xmlChar*)"xmlns", (const xmlChar*)*uri);

    } else if (args[1]->IsObject()) {
      v8::Handle<v8::Object> namespaces = args[1]->ToObject();
      v8::Handle<v8::Array> properties = namespaces->GetPropertyNames();
      for (unsigned int i = 0; i < properties->Length(); i++) {
        v8::Local<v8::String> prop_name = properties->Get(
          v8::Number::New(i))->ToString();
        v8::String::Utf8Value prefix(prop_name);
        v8::String::Utf8Value uri(namespaces->Get(prop_name));
        ctxt.register_ns((const xmlChar*)*prefix, (const xmlChar*)*uri);
      }
    }
  }

  return scope.Close(ctxt.evaluate((const xmlChar*)*xpath));
}

v8::Handle<v8::Value>
XmlDocFrag::Text(const v8::Arguments& args) {
  v8::HandleScope scope;
  XmlDocFrag *frag = LibXmlObj::Unwrap<XmlDocFrag>(args.This());
  assert(frag);

  if (args.Length() == 0) {
    return scope.Close(frag->get_content());
  } else {
    frag->set_content(*v8::String::Utf8Value(args[0]));
  }

  return scope.Close(args.This());
}

v8::Handle<v8::Value>
XmlDocFrag::Child(const v8::Arguments& args) {
  v8::HandleScope scope;
  XmlDocFrag *frag = LibXmlObj::Unwrap<XmlDocFrag>(args.This());
  assert(frag);

  double idx = 1;

  if (args.Length() > 0) {
    if (!args[0]->IsNumber()) {
      LIBXMLJS_THROW_EXCEPTION(
        "Bad argument: must provide #child() with a number");

    } else {
      idx = args[0]->ToNumber()->Value();
    }
  }

  return scope.Close(frag->get_child(idx));
}

v8::Handle<v8::Value>
XmlDocFrag::ChildNodes(const v8::Arguments& args) {
  v8::HandleScope scope;
  XmlDocFrag *frag = LibXmlObj::Unwrap<XmlDocFrag>(args.This());
  assert(frag);

  if (args[0]->IsNumber())
      return scope.Close(frag->get_child(args[0]->ToNumber()->Value()));

  return scope.Close(frag->get_child_nodes());
}

void
XmlDocFrag::set_name(const char* name) {
  xmlNodeSetName(xml_obj, (const xmlChar*)name);
}

v8::Handle<v8::Value>
XmlDocFrag::get_name() {
  return v8::String::New((const char*)xml_obj->name);
}

// TODO(sprsquish) make these work with namespaces
void
XmlDocFrag::add_child(XmlElement* child) {
  xmlAddChild(xml_obj, child->xml_obj);
}

v8::Handle<v8::Value>
XmlDocFrag::get_child(double idx) {
  v8::HandleScope scope;
  double i = 1;
  xmlNode* child = xml_obj->children;

  while (child && i < idx) {
    child = child->next;
    i++;
  }

  if (!child)
    return v8::Null();

  return scope.Close(LibXmlObj::GetMaybeBuild<XmlElement, xmlNode>(child));
}

v8::Handle<v8::Value>
XmlDocFrag::get_child_nodes() {
  v8::HandleScope scope;
  xmlNode* child = xml_obj->children;

  if (!child)
      return scope.Close(v8::Array::New(0));

  xmlNodeSetPtr set = xmlXPathNodeSetCreate(child);

  do {
    xmlXPathNodeSetAdd(set, child);
  } while ((child = child->next));

  v8::Handle<v8::Array> children = v8::Array::New(set->nodeNr);
  for (int i = 0; i < set->nodeNr; ++i) {
    xmlNode *node = set->nodeTab[i];
    children->Set(v8::Number::New(i),
                  LibXmlObj::GetMaybeBuild<XmlElement, xmlNode>(node));
  }

  xmlXPathFreeNodeSet(set);

  return scope.Close(children);
}

void
XmlDocFrag::set_content(const char* content) {
  xmlNodeSetContent(xml_obj, (const xmlChar*)content);
}

v8::Handle<v8::Value>
XmlDocFrag::get_content() {
  xmlChar* content = xmlNodeGetContent(xml_obj);
  if (content) {
    v8::Handle<v8::String> ret_content =
      v8::String::New((const char *)content);
    xmlFree(content);
    return ret_content;
  }

  return v8::String::Empty();
}

XmlElement *
XmlDocFrag::import_element(XmlElement *element) {
    xmlNode *new_child;
    if(xml_obj->doc == element->xml_obj->doc) {
        return element;
    } else {
        new_child = xmlDocCopyNode(element->xml_obj, xml_obj->doc, 1);
        if(new_child == NULL) {
            return NULL;
        }

        xmlUnlinkNode(element->xml_obj);
        
        UpdateV8Memory();

        v8::Handle<v8::Object> obj =
            LibXmlObj::GetMaybeBuild<XmlElement, xmlNode>(new_child);
        return LibXmlObj::Unwrap<XmlElement>(obj);
    }
}

void
XmlDocFrag::Initialize(v8::Handle<v8::Object> target) {
  v8::HandleScope scope;
  v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(New);
  constructor_template = v8::Persistent<v8::FunctionTemplate>::New(t);
  constructor_template->Inherit(XmlNode::constructor_template);
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);

  LXJS_SET_PROTO_METHOD(constructor_template,
                        "addChild",
                        XmlDocFrag::AddChild);

  LXJS_SET_PROTO_METHOD(constructor_template,
                        "child",
                        XmlDocFrag::Child);

  LXJS_SET_PROTO_METHOD(constructor_template,
                        "childNodes",
                        XmlDocFrag::ChildNodes);

  LXJS_SET_PROTO_METHOD(constructor_template,
                        "find",
                        XmlDocFrag::Find);

  LXJS_SET_PROTO_METHOD(constructor_template,
                        "name",
                        XmlDocFrag::Name);

  LXJS_SET_PROTO_METHOD(constructor_template,
                        "text",
                        XmlDocFrag::Text);

  target->Set(v8::String::NewSymbol("DocumentFragment"),
              constructor_template->GetFunction());
}

}  // namespace libxmljs
