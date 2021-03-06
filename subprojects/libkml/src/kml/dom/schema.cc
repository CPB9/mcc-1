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

// This file contains the implementation of the SimpleField and Schema elements.

#include "kml/dom/schema.h"
#include "kml/base/attributes.h"
#include "kml/base/xml_namespaces.h"
#include "kml/dom/kml_cast.h"
#include "kml/dom/serializer.h"
#include "kml/dom/visitor.h"

using kmlbase::Attributes;

namespace kmldom {

// <SimpleField>
SimpleField::SimpleField()
    : has_type_(false), has_name_(false), has_displayname_(false) {
  set_xmlns(kmlbase::XMLNS_KML22);
}

SimpleField::~SimpleField() {
}

static const char kSimpleFieldTypeAttr[] = "type";
static const char kSimpleFieldNameAttr[] = "name";

void SimpleField::ParseAttributes(Attributes* attributes) {
  if (!attributes) {
    return;
  }
  has_type_ = attributes->CutValue(kSimpleFieldTypeAttr, &type_);
  has_name_ = attributes->CutValue(kSimpleFieldNameAttr, &name_);
  AddUnknownAttributes(attributes);
}

void SimpleField::SerializeAttributes(Attributes* attributes) const {
  Element::SerializeAttributes(attributes);
  if (has_type_) {
    attributes->SetValue(kSimpleFieldTypeAttr, type_);
  }
  if (has_name_) {
    attributes->SetValue(kSimpleFieldNameAttr, name_);
  }
}

void SimpleField::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->Type() == Type_displayName) {
    has_displayname_ = element->SetString(&displayname_);
  } else {
    Element::AddElement(element);
  }
}

void SimpleField::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  if (has_displayname()) {
    serializer.SaveFieldById(Type_displayName, get_displayname());
  }
}

void SimpleField::Accept(Visitor* visitor) {
  visitor->VisitSimpleField(SimpleFieldPtr(this));
}

// <GxSimpleArrayField>
GxSimpleArrayField::GxSimpleArrayField() {
  set_xmlns(kmlbase::XMLNS_GX22);
}

GxSimpleArrayField::~GxSimpleArrayField() {
}

void GxSimpleArrayField::Accept(Visitor* visitor) {
  visitor->VisitGxSimpleArrayField(GxSimpleArrayFieldPtr(this));
}

// <Schema>
Schema::Schema() : has_name_(false) {
}

Schema::~Schema() {
}

static const char kSchemaNameAttr[] = "name";

void Schema::ParseAttributes(Attributes* attributes) {
  if (!attributes) {
    return;
  }
  has_name_ = attributes->CutValue(kSchemaNameAttr, &name_);
  Object::ParseAttributes(attributes);
}

void Schema::SerializeAttributes(Attributes* attributes) const {
  Object::SerializeAttributes(attributes);
  if (has_name_) {
    attributes->SetValue(kSchemaNameAttr, name_);
  }
}

void Schema::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_SimpleField:
      add_simplefield(AsSimpleField(element));
      break;
    case Type_GxSimpleArrayField:
      add_gx_simplearrayfield(AsGxSimpleArrayField(element));
      break;
    default:
      Object::AddElement(element);
  }
}

void Schema::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  serializer.SaveElementArray(simplefield_array_);
  serializer.SaveElementArray(gx_simplearrayfield_array_);
}

void Schema::Accept(Visitor* visitor) {
  visitor->VisitSchema(SchemaPtr(this));
}

void Schema::AcceptChildren(VisitorDriver* driver) {
  Object::AcceptChildren(driver);
  Element::AcceptRepeated<SimpleFieldPtr>(&simplefield_array_, driver);
  Element::AcceptRepeated<GxSimpleArrayFieldPtr>(&gx_simplearrayfield_array_,
                                                 driver);
}

const string& SimpleField::get_type() const {
  return type_;
}

bool SimpleField::has_type() const {
  return has_type_;
}

void SimpleField::set_type(const string& value) {
  type_ = value;
  has_type_ = true;
}

void SimpleField::clear_type() {
  type_.clear();
  has_type_ = false;
}

const string& SimpleField::get_name() const {
  return name_;
}

bool SimpleField::has_name() const {
  return has_name_;
}

void SimpleField::set_name(const string& value) {
  name_ = value;
  has_name_ = true;
}

void SimpleField::clear_name() {
  name_.clear();
  has_name_ = false;
}

const string& SimpleField::get_displayname() const {
  return displayname_;
}

bool SimpleField::has_displayname() const {
  return has_displayname_;
}

void SimpleField::set_displayname(const string& value) {
  displayname_ = value;
  has_displayname_ = true;
}

void SimpleField::clear_displayname() {
  displayname_.clear();
  has_displayname_ = false;
}

kmldom::KmlDomType GxSimpleArrayField::Type() const {
  return Type_GxSimpleArrayField;
}

bool GxSimpleArrayField::IsA(kmldom::KmlDomType type) const {
  return type == Type_GxSimpleArrayField || SimpleField::IsA(type);
}

kmldom::KmlDomType Schema::Type() const {
  return Type_Schema;
}

bool Schema::IsA(kmldom::KmlDomType type) const {
  return type == Type_Schema || Object::IsA(type);
}

const string& Schema::get_name() const {
  return name_;
}

bool Schema::has_name() const {
  return has_name_;
}

void Schema::set_name(const string& value) {
  name_ = value;
  has_name_ = true;
}

void Schema::clear_name() {
  name_.clear();
  has_name_ = false;
}

void Schema::add_simplefield(const SimpleFieldPtr& simplefield) {
  AddComplexChild(simplefield, &simplefield_array_);
}

size_t Schema::get_simplefield_array_size() const {
  return simplefield_array_.size();
}

const SimpleFieldPtr& Schema::get_simplefield_array_at(size_t index) const {
  return simplefield_array_[index];
}

void Schema::add_gx_simplearrayfield(
    const GxSimpleArrayFieldPtr& gx_simplearrayfield) {
  AddComplexChild(gx_simplearrayfield, &gx_simplearrayfield_array_);
}

size_t Schema::get_gx_simplearrayfield_array_size() const {
  return gx_simplearrayfield_array_.size();
}

const GxSimpleArrayFieldPtr& Schema::get_gx_simplearrayfield_array_at(
    size_t index) const {
  return gx_simplearrayfield_array_[index];
}
}  // end namespace kmldom
