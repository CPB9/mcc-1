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

#include "kml/dom/style.h"
#include "kml/base/attributes.h"
#include "kml/dom/balloonstyle.h"
#include "kml/dom/iconstyle.h"
#include "kml/dom/kml22.h"
#include "kml/dom/kml_cast.h"
#include "kml/dom/labelstyle.h"
#include "kml/dom/linestyle.h"
#include "kml/dom/liststyle.h"
#include "kml/dom/polystyle.h"
#include "kml/dom/serializer.h"
#include "kml/dom/visitor.h"

using kmlbase::Attributes;

namespace kmldom {

Style::Style() {
}

Style::~Style() {
}

void Style::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_IconStyle:
      set_iconstyle(AsIconStyle(element));
      break;
    case Type_LabelStyle:
      set_labelstyle(AsLabelStyle(element));
      break;
    case Type_LineStyle:
      set_linestyle(AsLineStyle(element));
      break;
    case Type_PolyStyle:
      set_polystyle(AsPolyStyle(element));
      break;
    case Type_BalloonStyle:
      set_balloonstyle(AsBalloonStyle(element));
      break;
    case Type_ListStyle:
      set_liststyle(AsListStyle(element));
      break;
    default:
      StyleSelector::AddElement(element);
      break;
  }
}

void Style::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  StyleSelector::Serialize(serializer);
  if (has_iconstyle()) {
    serializer.SaveElement(get_iconstyle());
  }
  if (has_labelstyle()) {
    serializer.SaveElement(get_labelstyle());
  }
  if (has_linestyle()) {
    serializer.SaveElement(get_linestyle());
  }
  if (has_polystyle()) {
    serializer.SaveElement(get_polystyle());
  }
  if (has_balloonstyle()) {
    serializer.SaveElement(get_balloonstyle());
  }
  if (has_liststyle()) {
    serializer.SaveElement(get_liststyle());
  }
}

void Style::Accept(Visitor* visitor) {
  visitor->VisitStyle(StylePtr(this));
}

void Style::AcceptChildren(VisitorDriver* driver) {
  StyleSelector::AcceptChildren(driver);
  if (has_iconstyle()) {
    driver->Visit(get_iconstyle());
  }
  if (has_labelstyle()) {
    driver->Visit(get_labelstyle());
  }
  if (has_linestyle()) {
    driver->Visit(get_linestyle());
  }
  if (has_polystyle()) {
    driver->Visit(get_polystyle());
  }
  if (has_balloonstyle()) {
    driver->Visit(get_balloonstyle());
  }
  if (has_liststyle()) {
    driver->Visit(get_liststyle());
  }
}

kmldom::KmlDomType Style::Type() const {
  return Type_Style;
}

bool Style::IsA(kmldom::KmlDomType type) const {
  return type == Type_Style || StyleSelector::IsA(type);
}

const IconStylePtr& Style::get_iconstyle() const {
  return iconstyle_;
}

bool Style::has_iconstyle() const {
  return iconstyle_ != nullptr;
}

void Style::set_iconstyle(const IconStylePtr& iconstyle) {
  SetComplexChild(iconstyle, &iconstyle_);
}

void Style::clear_iconstyle() {
  set_iconstyle(NULL);
}

const LabelStylePtr& Style::get_labelstyle() const {
  return labelstyle_;
}

bool Style::has_labelstyle() const {
  return labelstyle_ != nullptr;
}

void Style::set_labelstyle(const LabelStylePtr& labelstyle) {
  SetComplexChild(labelstyle, &labelstyle_);
}

void Style::clear_labelstyle() {
  set_labelstyle(NULL);
}

const LineStylePtr& Style::get_linestyle() const {
  return linestyle_;
}

bool Style::has_linestyle() const {
  return linestyle_ != nullptr;
}

void Style::set_linestyle(const LineStylePtr& linestyle) {
  SetComplexChild(linestyle, &linestyle_);
}

void Style::clear_linestyle() {
  set_linestyle(NULL);
}

const PolyStylePtr& Style::get_polystyle() const {
  return polystyle_;
}

bool Style::has_polystyle() const {
  return polystyle_ != nullptr;
}

void Style::set_polystyle(const PolyStylePtr& polystyle) {
  SetComplexChild(polystyle, &polystyle_);
}

void Style::clear_polystyle() {
  set_polystyle(NULL);
}

const BalloonStylePtr& Style::get_balloonstyle() const {
  return balloonstyle_;
}

bool Style::has_balloonstyle() const {
  return balloonstyle_ != nullptr;
}

void Style::set_balloonstyle(const BalloonStylePtr& balloonstyle) {
  SetComplexChild(balloonstyle, &balloonstyle_);
}

void Style::clear_balloonstyle() {
  set_balloonstyle(NULL);
}

const ListStylePtr& Style::get_liststyle() const {
  return liststyle_;
}

bool Style::has_liststyle() const {
  return liststyle_ != nullptr;
}

void Style::set_liststyle(const ListStylePtr& liststyle) {
  SetComplexChild(liststyle, &liststyle_);
}

void Style::clear_liststyle() {
  set_liststyle(NULL);
}
}  // end namespace kmldom
