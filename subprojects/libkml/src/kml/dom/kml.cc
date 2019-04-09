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

#include "kml/dom/kml.h"
#include "kml/base/attributes.h"
#include "kml/base/xml_namespaces.h"
#include "kml/dom/element.h"
#include "kml/dom/feature.h"
#include "kml/dom/kml_cast.h"
#include "kml/dom/networklinkcontrol.h"
#include "kml/dom/serializer.h"
#include "kml/dom/visitor.h"

using kmlbase::Attributes;

namespace kmldom {

Kml::Kml() : has_hint_(false) {
  set_xmlns(kmlbase::XMLNS_KML22);
}

Kml::~Kml() {
}

static const char kHint[] = "hint";

void Kml::ParseAttributes(Attributes* attributes) {
  if (!attributes) {
    return;
  }
  has_hint_ = attributes->CutValue(kHint, &hint_);
  AddUnknownAttributes(attributes);
}

void Kml::SerializeAttributes(Attributes* attributes) const {
  Element::SerializeAttributes(attributes);
  if (has_hint_) {
    attributes->SetValue(kHint, hint_);
  }
}

void Kml::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->IsA(Type_Feature)) {
    set_feature(AsFeature(element));
  } else if (element->Type() == Type_NetworkLinkControl) {
    set_networklinkcontrol(AsNetworkLinkControl(element));
  } else {
    Element::AddElement(element);
  }
}

void Kml::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  if (has_networklinkcontrol()) {
    serializer.SaveElement(get_networklinkcontrol());
  }
  if (has_feature()) {
    serializer.SaveElementGroup(get_feature(), Type_Feature);
  }
}

void Kml::Accept(Visitor* visitor) {
  visitor->VisitKml(KmlPtr(this));
}

void Kml::AcceptChildren(VisitorDriver* driver) {
  Element::AcceptChildren(driver);
  if (has_networklinkcontrol()) {
    driver->Visit(get_networklinkcontrol());
  }
  if (has_feature()) {
    driver->Visit(get_feature());
  }
}

const string& Kml::get_hint() {
  return hint_;
}

bool Kml::has_hint() const {
  return has_hint_;
}

void Kml::set_hint(const string& hint) {
  hint_ = hint;
  has_hint_ = true;
}

void Kml::clear_hint() {
  hint_.clear();
  has_hint_ = false;
}

const NetworkLinkControlPtr& Kml::get_networklinkcontrol() const {
  return networklinkcontrol_;
}

bool Kml::has_networklinkcontrol() const {
  return networklinkcontrol_ != nullptr;
}

void Kml::set_networklinkcontrol(
    const NetworkLinkControlPtr& networklinkcontrol) {
  SetComplexChild(networklinkcontrol, &networklinkcontrol_);
}

void Kml::clear_networklinkcontrol() {
  set_networklinkcontrol(NULL);
}

const FeaturePtr& Kml::get_feature() const {
  return feature_;
}

bool Kml::has_feature() const {
  return feature_ != nullptr;
}

void Kml::set_feature(const FeaturePtr& feature) {
  SetComplexChild(feature, &feature_);
}

void Kml::clear_feature() {
  set_feature(NULL);
}
}  // end namespace kmldom
