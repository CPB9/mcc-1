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

// This file contains the implementation of the abstract Vec2 element.

#include "kml/dom/vec2.h"
#include "kml/base/attributes.h"
#include "kml/dom/serializer.h"
#include "kml/dom/visitor.h"
#include "kml/dom/xsd.h"

using kmlbase::Attributes;

namespace kmldom {

Vec2::Vec2()
    : x_(1.0),
      has_x_(false),
      y_(1.0),
      has_y_(false),
      xunits_(UNITS_FRACTION),
      has_xunits_(false),
      yunits_(UNITS_FRACTION),
      has_yunits_(false) {
}

Vec2::~Vec2() {
}

static const char kX[] = "x";
static const char kY[] = "y";
static const char kXUnits[] = "xunits";
static const char kYUnits[] = "yunits";

// Look up the given attribute and convert it to an enum val of the given
// enum type.  For example, attr_name="xunits" and enum_type=Type_units.
// This matches the behavior in kmldom::Field::SetEnum.
// NOTE: This would be a general purpose method if any other KML elements
// had attributes of enum type.
static bool CutEnumAttr(Attributes* attributes, const string attr_name,
                        int enum_type, int* enum_val) {
  string attr_val;
  if (attributes->CutValue(attr_name, &attr_val)) {
    int val = Xsd::GetSchema()->EnumId(enum_type, attr_val);
    if (val != -1) {
      *enum_val = val;
      return true;
    }
  }
  return false;
}

void Vec2::clear_yunits() {
  yunits_ = false;
  has_yunits_ = false;
}

void Vec2::set_yunits(int value) {
  yunits_ = value;
  has_yunits_ = true;
}

bool Vec2::has_yunits() const {
  return has_yunits_;
}

int Vec2::get_yunits() const {
  return yunits_;
}

void Vec2::clear_xunits() {
  xunits_ = false;
  has_xunits_ = false;
}

void Vec2::set_xunits(int value) {
  xunits_ = value;
  has_xunits_ = true;
}

bool Vec2::has_xunits() const {
  return has_xunits_;
}

int Vec2::get_xunits() const {
  return xunits_;
}

void Vec2::clear_y() {
  y_ = 1.0;
  has_y_ = false;
}

void Vec2::set_y(double value) {
  y_ = value;
  has_y_ = true;
}

bool Vec2::has_y() const {
  return has_y_;
}

double Vec2::get_y() const {
  return y_;
}

void Vec2::clear_x() {
  x_ = 1.0;
  has_x_ = false;
}

void Vec2::set_x(double value) {
  x_ = value;
  has_x_ = true;
}

bool Vec2::has_x() const {
  return has_x_;
}

double Vec2::get_x() const {
  return x_;
}

bool Vec2::IsA(kmldom::KmlDomType type) const {
  return type == Type_Vec2;
}

kmldom::KmlDomType Vec2::Type() const {
  return Type_Vec2;
}

void Vec2::ParseAttributes(Attributes* attributes) {
  if (!attributes) {
    return;
  }
  has_x_ = attributes->CutValue(kX, &x_);
  has_y_ = attributes->CutValue(kY, &y_);
  has_xunits_ = CutEnumAttr(attributes, kXUnits, Type_units, &xunits_);
  has_yunits_ = CutEnumAttr(attributes, kYUnits, Type_units, &yunits_);
  AddUnknownAttributes(attributes);
}

void Vec2::SerializeAttributes(Attributes* attributes) const {
  Element::SerializeAttributes(attributes);
  if (has_x_) {
    attributes->SetValue(kX, x_);
  }
  if (has_y_) {
    attributes->SetValue(kY, y_);
  }
  if (has_xunits_) {
    attributes->SetValue(kXUnits,
                         Xsd::GetSchema()->EnumValue(Type_units, xunits_));
  }
  if (has_yunits_) {
    attributes->SetValue(kYUnits,
                         Xsd::GetSchema()->EnumValue(Type_units, yunits_));
  }
}

void Vec2::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
}

void Vec2::Accept(Visitor* visitor) {
  visitor->VisitVec2(Vec2Ptr(this));
}

}  // end namespace kmldom
