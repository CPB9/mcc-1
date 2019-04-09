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

// This file contains the declarations of the SimpleData, SchemaData,
// Data, and ExtendedData elements.

#ifndef KML_DOM_EXTENDEDDATA_H__
#define KML_DOM_EXTENDEDDATA_H__

#include <vector>
#include "kml/base/util.h"
#include "kml/config.h"
#include "kml/dom/element.h"
#include "kml/dom/kml22.h"
#include "kml/dom/kml_ptr.h"
#include "kml/dom/object.h"

namespace kmlbase {
class Attributes;
}

namespace kmldom {

class Visitor;
class VisitorDriver;

// <SimpleData>
class KML_EXPORT SimpleData : public BasicElement<Type_SimpleData> {
 public:
  virtual ~SimpleData();

  // name=
  const string& get_name() const;
  bool has_name() const;
  void set_name(const string& value);
  void clear_name();

  // char data
  const string& get_text() const;
  bool has_text() const;
  void set_text(const string& value);
  void clear_text();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  SimpleData();
  friend class KmlHandler;
  virtual void ParseAttributes(kmlbase::Attributes* attributes);
  virtual void AddElement(const ElementPtr& child);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  virtual void SerializeAttributes(kmlbase::Attributes* attributes) const;
  string name_;
  bool has_name_;
  string text_;
  bool has_text_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(SimpleData);
};

// <gx:SimpleArrayData>
class KML_EXPORT GxSimpleArrayData
    : public BasicElement<Type_GxSimpleArrayData> {
 public:
  virtual ~GxSimpleArrayData();

  // name=
  const string& get_name() const;
  bool has_name() const;
  void set_name(const string& value);
  void clear_name();

  // <gx:value>
  void add_gx_value(const string& value);

  size_t get_gx_value_array_size() const;

  const string& get_gx_value_array_at(size_t index) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  GxSimpleArrayData();
  friend class KmlHandler;
  virtual void ParseAttributes(kmlbase::Attributes* attributes);
  virtual void AddElement(const ElementPtr& child);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  virtual void SerializeAttributes(kmlbase::Attributes* attributes) const;
  string name_;
  bool has_name_;
  std::vector<string> gx_value_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(GxSimpleArrayData);
};

// <SchemaData>
class KML_EXPORT SchemaData : public Object {
 public:
  virtual ~SchemaData();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;
  static KmlDomType ElementType();

  // schemaUrl=
  const string& get_schemaurl() const;
  bool has_schemaurl() const;
  void set_schemaurl(const string& value);
  void clear_schemaurl();

  void add_simpledata(const SimpleDataPtr& simpledata);

  size_t get_simpledata_array_size() const;

  const SimpleDataPtr& get_simpledata_array_at(size_t index) const;

  void add_gx_simplearraydata(const GxSimpleArrayDataPtr& gx_simplearraydata);

  size_t get_gx_simplearraydata_array_size() const;

  const GxSimpleArrayDataPtr& get_gx_simplearraydata_array_at(
      size_t index) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  SchemaData();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  virtual void ParseAttributes(kmlbase::Attributes* attributes);
  friend class ExtendedData;
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  virtual void SerializeAttributes(kmlbase::Attributes* attributes) const;
  string schemaurl_;
  bool has_schemaurl_;
  std::vector<SimpleDataPtr> simpledata_array_;
  std::vector<GxSimpleArrayDataPtr> gx_simplearraydata_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(SchemaData);
};

// <Data>
class KML_EXPORT Data : public Object {
 public:
  virtual ~Data();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;
  static KmlDomType ElementType();

  // name=
  const string& get_name() const;
  bool has_name() const;
  void set_name(const string& value);
  void clear_name();

  // <displayname>
  const string& get_displayname() const;
  bool has_displayname() const;
  void set_displayname(const string& value);
  void clear_displayname();

  // <value>
  const string& get_value() const;
  bool has_value() const;
  void set_value(const string& value);
  void clear_value();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  Data();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  virtual void ParseAttributes(kmlbase::Attributes* attributes);
  friend class ExtendedData;
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  virtual void SerializeAttributes(kmlbase::Attributes* attributes) const;
  string name_;
  bool has_name_;
  string displayname_;
  bool has_displayname_;
  string value_;
  bool has_value_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Data);
};

// <ExtendedData>
class KML_EXPORT ExtendedData : public BasicElement<Type_ExtendedData> {
 public:
  virtual ~ExtendedData();

  // <Data>.
  void add_data(const DataPtr& data);

  size_t get_data_array_size() const;

  const DataPtr& get_data_array_at(size_t index) const;

  // <SchemaData>.
  void add_schemadata(const SchemaDataPtr& schemadata);

  size_t get_schemadata_array_size() const;

  const SchemaDataPtr& get_schemadata_array_at(size_t index) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  ExtendedData();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  std::vector<DataPtr> data_array_;
  std::vector<SchemaDataPtr> schemadata_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(ExtendedData);
};

// <Metadata>
// This element is deprecated in OGC KML 2.2.  New KML should use
// <ExtendedData>.
class KML_EXPORT Metadata : public BasicElement<Type_Metadata> {
 public:
  virtual ~Metadata();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  Metadata();
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
};

}  // end namespace kmldom

#endif  // KML_DOM_EXTENDEDDATA_H__
