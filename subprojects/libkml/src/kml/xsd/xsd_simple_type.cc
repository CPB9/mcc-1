
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

#include "kml/xsd/xsd_simple_type.h"
#include "kml/base/attributes.h"

namespace kmlxsd {

kmlxsd::XsdSimpleType* XsdSimpleType::Create(
    const kmlbase::Attributes& attributes) {
  string name;
  if (attributes.GetString("name", &name)) {
    return new XsdSimpleType(name);
  }
  return nullptr;
}

XsdSimpleTypePtr XsdSimpleType::AsSimpleType(const XsdTypePtr& xsd_type) {
  if (xsd_type && xsd_type->get_xsd_type_id() == XSD_TYPE_SIMPLE) {
    return kmlbase::static_pointer_cast<XsdSimpleType>(xsd_type);
  }
  return nullptr;
}

kmlxsd::XsdType::XsdTypeEnum XsdSimpleType::get_xsd_type_id() const {
  return XSD_TYPE_SIMPLE;
}

bool XsdSimpleType::is_complex() const {
  return false;
}

const string XsdSimpleType::get_name() const {
  return name_;
}

const string XsdSimpleType::get_base() const {
  return restriction_base_;
}

void XsdSimpleType::set_restriction_base(const string& base) {
  restriction_base_ = base;
}

const string& XsdSimpleType::get_restriction_base() const {
  return restriction_base_;
}

void XsdSimpleType::add_enumeration(const string& value) {
  enumeration_.push_back(value);
}

size_t XsdSimpleType::get_enumeration_size() const {
  return enumeration_.empty() ? 0 : enumeration_.size();
}

const string& XsdSimpleType::get_enumeration_at(
    size_t index) const {
  return enumeration_[index];
}

bool XsdSimpleType::IsEnumeration() const {
  return restriction_base_ == "string" && !enumeration_.empty();
}

XsdSimpleType::~XsdSimpleType() {
}

XsdSimpleType::XsdSimpleType(const string& name) : name_(name) {
}
}  // namespace kmlxsd
