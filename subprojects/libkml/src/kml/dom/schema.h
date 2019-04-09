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

#ifndef KML_DOM_SCHEMA_H__
#define KML_DOM_SCHEMA_H__

#include <vector>
#include "kml/base/util.h"
#include "kml/config.h"
#include "kml/dom/element.h"
#include "kml/dom/kml22.h"
#include "kml/dom/object.h"

namespace kmlbase {
class Attributes;
}

namespace kmldom {

class Visitor;
class VisitorDriver;

// <SimpleField>
class KML_EXPORT SimpleField : public BasicElement<Type_SimpleField> {
 public:
  virtual ~SimpleField();

  const string& get_type() const;
  bool has_type() const;
  void set_type(const string& value);
  void clear_type();

  const string& get_name() const;
  bool has_name() const;
  void set_name(const string& value);
  void clear_name();

  const string& get_displayname() const;
  bool has_displayname() const;
  void set_displayname(const string& value);
  void clear_displayname();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 protected:
  SimpleField();
  virtual void AddElement(const ElementPtr& element);
  virtual void ParseAttributes(kmlbase::Attributes* attributes);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  virtual void SerializeAttributes(kmlbase::Attributes* attributes) const;

 private:
  friend class KmlFactory;
  friend class KmlHandler;
  string type_;
  bool has_type_;
  string name_;
  bool has_name_;
  string displayname_;
  bool has_displayname_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(SimpleField);
};

// <gx:SimpleArrayField>
class KML_EXPORT GxSimpleArrayField : public SimpleField {
 public:
  virtual ~GxSimpleArrayField();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  GxSimpleArrayField();
  friend class KmlHandler;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(GxSimpleArrayField);
};

// <Schema>
// Note: in the XSD Schema is not an Object. We inherit from Object here
// so it appears in the parsed object map and is easily accessible.
class KML_EXPORT Schema : public Object {
 public:
  virtual ~Schema();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  const string& get_name() const;
  bool has_name() const;
  void set_name(const string& value);
  void clear_name();

  void add_simplefield(const SimpleFieldPtr& simplefield);

  size_t get_simplefield_array_size() const;

  const SimpleFieldPtr& get_simplefield_array_at(size_t index) const;

  void add_gx_simplearrayfield(
      const GxSimpleArrayFieldPtr& gx_simplearrayfield);

  size_t get_gx_simplearrayfield_array_size() const;

  const GxSimpleArrayFieldPtr& get_gx_simplearrayfield_array_at(
      size_t index) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  Schema();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  virtual void ParseAttributes(kmlbase::Attributes* attributes);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  virtual void SerializeAttributes(kmlbase::Attributes* attributes) const;
  string name_;
  bool has_name_;
  std::vector<SimpleFieldPtr> simplefield_array_;
  std::vector<GxSimpleArrayFieldPtr> gx_simplearrayfield_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Schema);
};

}  // namespace kmldom

#endif  // KML_DOM_SCHEMA_H__
