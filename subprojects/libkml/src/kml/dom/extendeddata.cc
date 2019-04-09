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

// This file contains the implementation of the ExtendedData, SimpleData,
// SchemaData and Data element.

#include "kml/dom/extendeddata.h"
#include "kml/base/attributes.h"
#include "kml/base/xml_namespaces.h"
#include "kml/dom/kml_cast.h"
#include "kml/dom/kml_ptr.h"
#include "kml/dom/serializer.h"
#include "kml/dom/visitor.h"
#include "kml/dom/xsd.h"

using kmlbase::Attributes;

namespace kmldom {

// <SimpleData>
SimpleData::SimpleData() : has_name_(false), has_text_(false) {
  set_xmlns(kmlbase::XMLNS_KML22);
}

SimpleData::~SimpleData() {
}

static const char kSimpleDataName[] = "name";

void SimpleData::ParseAttributes(Attributes* attributes) {
  if (!attributes) {
    return;
  }
  has_name_ = attributes->CutValue(kSimpleDataName, &name_);
  AddUnknownAttributes(attributes);
}

void SimpleData::SerializeAttributes(Attributes* attributes) const {
  Element::SerializeAttributes(attributes);
  if (has_name_) {
    attributes->SetValue(kSimpleDataName, name_);
  }
}

// SimpleData needs to parse its own character data (like Snippet).
void SimpleData::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->Type() == Type_SimpleData) {
    set_text(get_char_data());
  } else {
    // We have a known KML element inside <SimpleData> and need to store it.
    Element::AddElement(element);
  }
}

void SimpleData::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  if (has_text()) {
    serializer.SaveContent(text_, true);
  }
}

void SimpleData::Accept(Visitor* visitor) {
  visitor->VisitSimpleData(SimpleDataPtr(this));
}
///////////////////////////////////////////////
// <GxSimpleArrayData>
GxSimpleArrayData::GxSimpleArrayData() : has_name_(false) {
  set_xmlns(kmlbase::XMLNS_GX22);
}

GxSimpleArrayData::~GxSimpleArrayData() {
}

static const char kGxSimpleArrayDataName[] = "name";

void GxSimpleArrayData::ParseAttributes(Attributes* attributes) {
  if (!attributes) {
    return;
  }
  has_name_ = attributes->CutValue(kGxSimpleArrayDataName, &name_);
  AddUnknownAttributes(attributes);
}

void GxSimpleArrayData::SerializeAttributes(Attributes* attributes) const {
  Element::SerializeAttributes(attributes);
  if (has_name_) {
    attributes->SetValue(kGxSimpleArrayDataName, name_);
  }
}

void GxSimpleArrayData::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->Type() == Type_GxValue) {
    add_gx_value(element->get_char_data());
  } else {
    Element::AddElement(element);
  }
}

void GxSimpleArrayData::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  for (size_t i = 0; i < gx_value_array_.size(); i++) {
    serializer.SaveFieldById(Type_GxValue, get_gx_value_array_at(i));
  }
}

void GxSimpleArrayData::Accept(Visitor* visitor) {
  visitor->VisitGxSimpleArrayData(GxSimpleArrayDataPtr(this));
}

// <SchemaData>
SchemaData::SchemaData() : has_schemaurl_(false) {
  set_xmlns(kmlbase::XMLNS_KML22);
}

SchemaData::~SchemaData() {
  // simpledata_array_'s destructor calls the destructor of each SimpleDataPtr
  // releasing the reference and potentially freeing the SimpleData storage.
}

static const char kSchemaUrl[] = "schemaUrl";

void SchemaData::ParseAttributes(Attributes* attributes) {
  if (!attributes) {
    return;
  }
  has_schemaurl_ = attributes->CutValue(kSchemaUrl, &schemaurl_);
  Object::ParseAttributes(attributes);
}

void SchemaData::SerializeAttributes(Attributes* attributes) const {
  Object::SerializeAttributes(attributes);
  if (has_schemaurl_) {
    attributes->SetValue(kSchemaUrl, schemaurl_);
  }
}

void SchemaData::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_SimpleData:
      add_simpledata(AsSimpleData(element));
      break;
    case Type_GxSimpleArrayData:
      add_gx_simplearraydata(AsGxSimpleArrayData(element));
      break;
    default:
      Object::AddElement(element);
  }
}

void SchemaData::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Object::Serialize(serializer);
  serializer.SaveElementArray(simpledata_array_);
  serializer.SaveElementArray(gx_simplearraydata_array_);
}

void SchemaData::Accept(Visitor* visitor) {
  visitor->VisitSchemaData(SchemaDataPtr(this));
}

void SchemaData::AcceptChildren(VisitorDriver* driver) {
  Object::AcceptChildren(driver);
  Element::AcceptRepeated<SimpleDataPtr>(&simpledata_array_, driver);
  Element::AcceptRepeated<GxSimpleArrayDataPtr>(&gx_simplearraydata_array_,
                                                driver);
}

// <Data>
Data::Data() : has_name_(false), has_displayname_(false), has_value_(false) {
  set_xmlns(kmlbase::XMLNS_KML22);
}

Data::~Data() {
}

static const char kDataName[] = "name";

void Data::ParseAttributes(Attributes* attributes) {
  if (!attributes) {
    return;
  }
  has_name_ = attributes->CutValue(kDataName, &name_);
  Object::ParseAttributes(attributes);
}

void Data::SerializeAttributes(Attributes* attributes) const {
  Object::SerializeAttributes(attributes);
  if (has_name_) {
    attributes->SetValue(kDataName, name_);
  }
}

void Data::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->Type() == Type_displayName) {
    has_displayname_ = element->SetString(&displayname_);
  } else if (element->Type() == Type_value) {
    has_value_ = element->SetString(&value_);
  } else {
    Object::AddElement(element);
  }
}

void Data::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Object::Serialize(serializer);
  if (has_displayname()) {
    serializer.SaveFieldById(Type_displayName, get_displayname());
  }
  if (has_value()) {
    serializer.SaveFieldById(Type_value, get_value());
  }
}

void Data::Accept(Visitor* visitor) {
  visitor->VisitData(DataPtr(this));
}

// <ExtendedData>
ExtendedData::ExtendedData() {
  set_xmlns(kmlbase::XMLNS_KML22);
}

ExtendedData::~ExtendedData() {
  // data_array_'s and schemadata_array_'s destructors call the destructor of
  // each DataPtr and SchemaDataPtr, releasing the references and potentially
  // freeing the SchemaData and Data storage.
}

void ExtendedData::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (DataPtr data = AsData(element)) {
    add_data(data);
  } else if (SchemaDataPtr schemadata = AsSchemaData(element)) {
    add_schemadata(schemadata);
  } else {
    Element::AddElement(element);
  }
}

void ExtendedData::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  serializer.SaveElementArray(data_array_);
  serializer.SaveElementArray(schemadata_array_);
}

void ExtendedData::Accept(Visitor* visitor) {
  visitor->VisitExtendedData(ExtendedDataPtr(this));
}

void ExtendedData::AcceptChildren(VisitorDriver* driver) {
  Element::AcceptChildren(driver);
  Element::AcceptRepeated<DataPtr>(&data_array_, driver);
  Element::AcceptRepeated<SchemaDataPtr>(&schemadata_array_, driver);
}

// <Metadata>
Metadata::Metadata() {
  set_xmlns(kmlbase::XMLNS_KML22);
}

Metadata::~Metadata() {
}

void Metadata::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
}

void Metadata::Accept(Visitor* visitor) {
  visitor->VisitMetadata(MetadataPtr(this));
}

const string& SimpleData::get_name() const {
  return name_;
}

bool SimpleData::has_name() const {
  return has_name_;
}

void SimpleData::set_name(const string& value) {
  name_ = value;
  has_name_ = true;
}

void SimpleData::clear_name() {
  name_.clear();
  has_name_ = false;
}

const string& SimpleData::get_text() const {
  return text_;
}

bool SimpleData::has_text() const {
  return has_text_;
}

void SimpleData::set_text(const string& value) {
  text_ = value;
  has_text_ = true;
}

void SimpleData::clear_text() {
  text_.clear();
  has_text_ = false;
}

const string& GxSimpleArrayData::get_name() const {
  return name_;
}

bool GxSimpleArrayData::has_name() const {
  return has_name_;
}

void GxSimpleArrayData::set_name(const string& value) {
  name_ = value;
  has_name_ = true;
}

void GxSimpleArrayData::clear_name() {
  name_.clear();
  has_name_ = false;
}

void GxSimpleArrayData::add_gx_value(const string& value) {
  gx_value_array_.push_back(value);
}

size_t GxSimpleArrayData::get_gx_value_array_size() const {
  return gx_value_array_.size();
}

const string& GxSimpleArrayData::get_gx_value_array_at(
    size_t index) const {
  return gx_value_array_[index];
}

kmldom::KmlDomType SchemaData::Type() const {
  return ElementType();
}

bool SchemaData::IsA(kmldom::KmlDomType type) const {
  return type == ElementType() || Object::IsA(type);
}

kmldom::KmlDomType SchemaData::ElementType() {
  return Type_SchemaData;
}

const string& SchemaData::get_schemaurl() const {
  return schemaurl_;
}

bool SchemaData::has_schemaurl() const {
  return has_schemaurl_;
}

void SchemaData::set_schemaurl(const string& value) {
  schemaurl_ = value;
  has_schemaurl_ = true;
}

void SchemaData::clear_schemaurl() {
  schemaurl_.clear();
  has_schemaurl_ = false;
}

void SchemaData::add_simpledata(const SimpleDataPtr& simpledata) {
  AddComplexChild(simpledata, &simpledata_array_);
}

size_t SchemaData::get_simpledata_array_size() const {
  return simpledata_array_.size();
}

const SimpleDataPtr& SchemaData::get_simpledata_array_at(size_t index) const {
  return simpledata_array_[index];
}

void SchemaData::add_gx_simplearraydata(
    const GxSimpleArrayDataPtr& gx_simplearraydata) {
  AddComplexChild(gx_simplearraydata, &gx_simplearraydata_array_);
}

size_t SchemaData::get_gx_simplearraydata_array_size() const {
  return gx_simplearraydata_array_.size();
}

const GxSimpleArrayDataPtr& SchemaData::get_gx_simplearraydata_array_at(
    size_t index) const {
  return gx_simplearraydata_array_[index];
}

kmldom::KmlDomType Data::Type() const {
  return ElementType();
}

bool Data::IsA(kmldom::KmlDomType type) const {
  return type == ElementType() || Object::IsA(type);
}

kmldom::KmlDomType Data::ElementType() {
  return Type_Data;
}

const string& Data::get_name() const {
  return name_;
}

bool Data::has_name() const {
  return has_name_;
}

void Data::set_name(const string& value) {
  name_ = value;
  has_name_ = true;
}

void Data::clear_name() {
  name_.clear();
  has_name_ = false;
}

const string& Data::get_displayname() const {
  return displayname_;
}

bool Data::has_displayname() const {
  return has_displayname_;
}

void Data::set_displayname(const string& value) {
  displayname_ = value;
  has_displayname_ = true;
}

void Data::clear_displayname() {
  displayname_.clear();
  has_displayname_ = false;
}

const string& Data::get_value() const {
  return value_;
}

bool Data::has_value() const {
  return has_value_;
}

void Data::set_value(const string& value) {
  value_ = value;
  has_value_ = true;
}

void Data::clear_value() {
  value_.clear();
  has_value_ = false;
}

void ExtendedData::add_data(const DataPtr& data) {
  AddComplexChild(data, &data_array_);
}

size_t ExtendedData::get_data_array_size() const {
  return data_array_.size();
}

const DataPtr& ExtendedData::get_data_array_at(size_t index) const {
  return data_array_[index];
}

void ExtendedData::add_schemadata(const SchemaDataPtr& schemadata) {
  AddComplexChild(schemadata, &schemadata_array_);
}

size_t ExtendedData::get_schemadata_array_size() const {
  return schemadata_array_.size();
}

const SchemaDataPtr& ExtendedData::get_schemadata_array_at(size_t index) const {
  return schemadata_array_[index];
}
}  // end namespace kmldom
