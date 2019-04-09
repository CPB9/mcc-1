// Copyright 2009, Google Inc. All rights reserved.
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

#include "kml/base/xmlns.h"
#include "kml/base/attributes.h"

namespace kmlbase {

Xmlns::~Xmlns()
{
}

kmlbase::Xmlns* Xmlns::Create(const Attributes& attributes) {
  Xmlns* xmlns = new Xmlns;
  if (xmlns->Parse(attributes)) {
    return xmlns;
  }
  delete xmlns;
  return nullptr;
}

const string& Xmlns::get_default() const {
  return default_;
}

const string Xmlns::GetNamespace(
    const string& prefix) const {
  string name_space;
  if (prefix_map_.get()) {
    prefix_map_->GetValue(prefix, &name_space);
  }
  return name_space;
}

const string Xmlns::GetKey(
    const string& value) const {
  string key;
  if (prefix_map_.get()) {
    prefix_map_->FindKey(value, &key);
  }
  return key;
}

void Xmlns::GetPrefixes(
    std::vector<string>* prefix_vector) const {
  if (prefix_map_.get()) {
    prefix_map_->GetAttrNames(prefix_vector);
  }
}

Xmlns::Xmlns() {
}

bool Xmlns::Parse(const Attributes& attributes) {
  // Create a copy so that we can use non-const SplitByPrefix.
  std::unique_ptr<Attributes> clone(attributes.Clone());
  prefix_map_.reset(clone->SplitByPrefix("xmlns"));
  attributes.GetValue("xmlns", &default_);
  // Return true if there is a default xmlns or if there are any
  // xmlns:prefx="ns" pairs.
  return !default_.empty() || prefix_map_.get();
}
}  // end namespace kmlbase
