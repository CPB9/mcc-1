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

#include "kml/base/xml_element.h"

namespace kmlbase {

const kmlbase::XmlElement* XmlElement::GetParent() const {
  return parent_;
}

const XmlFile* XmlElement::GetXmlFile() const {
  return xml_file_;
}

kmlbase::XmlnsId XmlElement::get_xmlns() const {
  return xmlns_id_;
}

bool XmlElement::InSameXmlFile(const XmlElementPtr& element) const {
  return element && xml_file_ == element->xml_file_;
}

bool XmlElement::SetXmlFile(const XmlFile* xml_file) {
  if (!xml_file_ && xml_file) {
    xml_file_ = xml_file;
    return true;
  }
  return false;
}

XmlElement::XmlElement()
    : xmlns_id_(XMLNS_NONE), parent_(nullptr), xml_file_(nullptr) {
}

void XmlElement::set_xmlns(kmlbase::XmlnsId xmlns_id) {
  xmlns_id_ = xmlns_id;
}

bool XmlElement::SetParent(const XmlElementPtr& parent) {
  if (!parent_ && parent && InSameXmlFile(parent)) {
    parent_ = parent.get();
    return true;
  }
  return false;
}
}  // end namespace kmlbase
