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

#include "kml/xsd/xsd_schema.h"
#include "kml/base/attributes.h"

namespace kmlxsd {

kmlxsd::XsdSchema* XsdSchema::Create(const kmlbase::Attributes& attributes) {
  XsdSchema* xsd_schema = new XsdSchema;
  if (xsd_schema->Parse(attributes)) {
    return xsd_schema;
  }
  delete xsd_schema;
  return nullptr;
}

const string& XsdSchema::get_target_namespace() const {
  return target_namespace_;
}

const string& XsdSchema::get_target_namespace_prefix() const {
  return target_namespace_prefix_;
}

bool XsdSchema::SplitNsName(const string& ns_name,
                            string* name) const {
  size_t prefix_size = target_namespace_prefix_.size();
  if (ns_name.size() > prefix_size + 1 &&
      ns_name.compare(0, prefix_size + 1, target_namespace_prefix_ + ":") ==
          0) {
    if (name) {
      *name = ns_name.substr(prefix_size + 1);
    }
    return true;
  }
  return false;
}

XsdSchema::XsdSchema() {
}

bool XsdSchema::Parse(const kmlbase::Attributes& attributes) {
  attributes.GetString("targetNamespace", &target_namespace_);
  if (target_namespace_.empty()) {
    return false;
  }
  xmlns_.reset(kmlbase::Xmlns::Create(attributes));
  if (!xmlns_.get()) {
    return false;
  }
  // Find the prefix used for the targetNamespace.
  // For example, if xmlns:foo="a:b:c" and targetNamespace="a:b:c" then the
  // prefix we seek is "foo".  A targetNamespace and xmlns:prefix _must_
  // appear in the <schema> for this to be a valid XSD.
  target_namespace_prefix_ = xmlns_->GetKey(target_namespace_);
  return !target_namespace_.empty() && !target_namespace_prefix_.empty();
}

XsdSchema::~XsdSchema() {
}
}  // namespace kmlxsd
