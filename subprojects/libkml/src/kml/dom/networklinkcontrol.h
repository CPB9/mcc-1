// Copyright 2008, Google Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// This file contains the declaration of the NetworkLinkControl element.

#ifndef KML_DOM_NETWORKLINKCONTROL_H__
#define KML_DOM_NETWORKLINKCONTROL_H__

#include <vector>
#include "kml/base/util.h"
#include "kml/config.h"
#include "kml/dom/element.h"
#include "kml/dom/kml22.h"
#include "kml/dom/kml_ptr.h"

namespace kmldom {

class Visitor;
class VisitorDriver;

// UpdateOperation
// An internal class from which <Create>, <Delete> and <Change> derive. The
// KML XSD uses a choice here which is not readily modeled in C++.
class KML_EXPORT UpdateOperation : public Element {
 public:
  virtual ~UpdateOperation();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 protected:
  // UpdateOperation is abstract.
  UpdateOperation();

 private:
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(UpdateOperation);
};

// <Create>
class KML_EXPORT Create : public UpdateOperation {
 public:
  virtual ~Create();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;
  static KmlDomType ElementType();

  // Create targets containers.
  void add_container(const ContainerPtr& container);

  size_t get_container_array_size() const;

  const ContainerPtr& get_container_array_at(size_t index) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  Create();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  std::vector<ContainerPtr> container_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Create);
};

// <Delete>
class KML_EXPORT Delete : public UpdateOperation {
 public:
  virtual ~Delete();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;
  static KmlDomType ElementType();

  // Delete targets Features.
  void add_feature(const FeaturePtr& feature);

  size_t get_feature_array_size() const;

  const FeaturePtr& get_feature_array_at(size_t index) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  Delete();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  std::vector<FeaturePtr> feature_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Delete);
};

// <Change>
class KML_EXPORT Change : public UpdateOperation {
 public:
  virtual ~Change();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;
  static KmlDomType ElementType();

  // Change targets Objects.
  void add_object(const ObjectPtr& object);

  size_t get_object_array_size() const;

  const ObjectPtr& get_object_array_at(size_t index) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  Change();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  std::vector<ObjectPtr> object_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Change);
};

// <Update>
class KML_EXPORT Update : public BasicElement<Type_Update> {
 public:
  virtual ~Update();

  // <targetHref>
  const string& get_targethref() const;
  bool has_targethref() const;
  void set_targethref(const string& targethref);
  void clear_targethref();

  // <Create>, <Delete> and <Change> elements.
  void add_updateoperation(const UpdateOperationPtr& updateoperation);

  size_t get_updateoperation_array_size() const;

  const UpdateOperationPtr& get_updateoperation_array_at(size_t index) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  Update();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  string targethref_;
  bool has_targethref_;
  std::vector<UpdateOperationPtr> updateoperation_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Update);
};

// <NetworkLinkControl>
class KML_EXPORT NetworkLinkControl
    : public BasicElement<Type_NetworkLinkControl> {
 public:
  virtual ~NetworkLinkControl();

  // <minRefreshPeriod>
  double get_minrefreshperiod() const;
  bool has_minrefreshperiod() const;
  void set_minrefreshperiod(double value);
  void clear_minrefreshperiod();

  // <maxSessionLength>
  double get_maxsessionlength() const;
  bool has_maxsessionlength() const;
  void set_maxsessionlength(double value);
  void clear_maxsessionlength();

  // <cookie>
  const string& get_cookie() const;
  bool has_cookie() const;
  void set_cookie(const string& cookie);
  void clear_cookie();

  // <message>
  const string& get_message() const;
  bool has_message() const;
  void set_message(const string& message);
  void clear_message();

  // <linkName>
  const string& get_linkname() const;
  bool has_linkname() const;
  void set_linkname(const string& linkname);
  void clear_linkname();

  // <linkDescription>
  const string& get_linkdescription() const;
  bool has_linkdescription() const;
  void set_linkdescription(const string& linkdescription);
  void clear_linkdescription();

  // <linkSnippet>
  const LinkSnippetPtr& get_linksnippet() const;
  bool has_linksnippet() const;
  void set_linksnippet(LinkSnippetPtr linksnippet);
  void clear_linksnippet();

  // <expires>
  const string& get_expires() const;
  bool has_expires() const;
  void set_expires(const string& expires);
  void clear_expires();

  // <Update>
  const UpdatePtr& get_update() const;
  bool has_update() const;
  void set_update(const UpdatePtr& update);
  void clear_update();

  // AbstractView
  const AbstractViewPtr& get_abstractview() const;
  bool has_abstractview() const;
  void set_abstractview(const AbstractViewPtr& abstractview);
  void clear_abstractview();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  NetworkLinkControl();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  double minrefreshperiod_;
  bool has_minrefreshperiod_;
  double maxsessionlength_;
  bool has_maxsessionlength_;
  string cookie_;
  bool has_cookie_;
  string message_;
  bool has_message_;
  string linkname_;
  bool has_linkname_;
  string linkdescription_;
  bool has_linkdescription_;
  LinkSnippetPtr linksnippet_;
  string expires_;
  bool has_expires_;
  UpdatePtr update_;
  AbstractViewPtr abstractview_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(NetworkLinkControl);
};

}  // namespace kmldom

#endif  // KML_DOM_NETWORKLINKCONTROL_H__
