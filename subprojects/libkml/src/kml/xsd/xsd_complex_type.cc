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

// This file contains the implementation of the XsdComplexType class.

#include "kml/xsd/xsd_complex_type.h"
#include "kml/base/attributes.h"

namespace kmlxsd {

XsdComplexType::XsdComplexType(const string& name) : name_(name) {
}
// private
bool XsdComplexType::ParseAttributes(const kmlbase::Attributes& attributes) {
  // <xs:complexType name="FooType">.  Returns false if there's no name=.
  return attributes.GetString("name", &name_);
}

kmlxsd::XsdComplexType* XsdComplexType::Create(
    const kmlbase::Attributes& attributes) {
  string name;
  if (attributes.GetString("name", &name)) {
    return new XsdComplexType(name);
  }
  return nullptr;
}

XsdComplexTypePtr XsdComplexType::AsComplexType(
    const kmlxsd::XsdTypePtr& xsd_type) {
  if (xsd_type && xsd_type->get_xsd_type_id() == XSD_TYPE_COMPLEX) {
    return kmlbase::static_pointer_cast<XsdComplexType>(xsd_type);
  }
  return nullptr;
}

kmlxsd::XsdType::XsdTypeEnum XsdComplexType::get_xsd_type_id() const {
  return XSD_TYPE_COMPLEX;
}

bool XsdComplexType::is_complex() const {
  return true;
}

const string XsdComplexType::get_name() const {
  return name_;
}

const string XsdComplexType::get_base() const {
  return extension_base_;
}

void XsdComplexType::set_extension_base(
    const string& extension_base) {
  extension_base_ = extension_base;
}

const string& XsdComplexType::get_extension_base() const {
  return extension_base_;
}

bool XsdComplexType::has_extension_base() const {
  return !extension_base_.empty();
}

void XsdComplexType::add_element(const XsdElementPtr& element) {
  sequence_.push_back(element);
}

size_t XsdComplexType::get_sequence_size() const {
  return sequence_.size();
}

const XsdElementPtr XsdComplexType::get_sequence_at(size_t index) const {
  return sequence_[index];
}

XsdComplexType::~XsdComplexType() {
}
}  // end namespace kmlxsd
