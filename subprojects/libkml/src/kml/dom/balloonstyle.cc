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

// This file contains the implementation of the BalloonStyle element.

#include "kml/dom/balloonstyle.h"
#include "kml/base/attributes.h"
#include "kml/dom/element.h"
#include "kml/dom/serializer.h"
#include "kml/dom/visitor.h"
#include "kml/dom/visitor_driver.h"

using kmlbase::Attributes;
using kmlbase::Color32;

namespace kmldom {

BalloonStyle::BalloonStyle()
    : bgcolor_("ffffffff"),
      has_bgcolor_(false),
      textcolor_("ff000000"),
      has_textcolor_(false),
      has_text_(false),
      displaymode_(DISPLAYMODE_DEFAULT),
      has_displaymode_(false) {
}

BalloonStyle::~BalloonStyle() {
}

void BalloonStyle::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_bgColor:
      set_bgcolor(Color32(element->get_char_data()));
      break;
    case Type_textColor:
      set_textcolor(Color32(element->get_char_data()));
      break;
    case Type_text:
      has_text_ = element->SetString(&text_);
      break;
    case Type_displayMode:
      has_displaymode_ = element->SetEnum(&displaymode_);
      break;
    default:
      SubStyle::AddElement(element);
      break;
  }
}

void BalloonStyle::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  SubStyle::Serialize(serializer);
  if (has_bgcolor()) {
    serializer.SaveColor(Type_bgColor, get_bgcolor());
  }
  if (has_textcolor()) {
    serializer.SaveColor(Type_textColor, get_textcolor());
  }
  if (has_text()) {
    serializer.SaveFieldById(Type_text, get_text());
  }
  if (has_displaymode()) {
    serializer.SaveEnum(Type_displayMode, get_displaymode());
  }
}

void BalloonStyle::Accept(Visitor* visitor) {
  visitor->VisitBalloonStyle(BalloonStylePtr(this));
}

kmldom::KmlDomType BalloonStyle::Type() const {
  return Type_BalloonStyle;
}

bool BalloonStyle::IsA(kmldom::KmlDomType type) const {
  return type == Type_BalloonStyle || SubStyle::IsA(type);
}

const kmlbase::Color32& BalloonStyle::get_bgcolor() const {
  return bgcolor_;
}

bool BalloonStyle::has_bgcolor() const {
  return has_bgcolor_;
}

void BalloonStyle::set_bgcolor(const kmlbase::Color32& bgcolor) {
  bgcolor_ = bgcolor;
  has_bgcolor_ = true;
}

void BalloonStyle::clear_bgcolor() {
  bgcolor_ = kmlbase::Color32(0xffffffff);
  has_bgcolor_ = false;
}

const kmlbase::Color32& BalloonStyle::get_textcolor() const {
  return textcolor_;
}

void BalloonStyle::set_textcolor(const kmlbase::Color32& textcolor) {
  textcolor_ = textcolor;
  has_textcolor_ = true;
}

bool BalloonStyle::has_textcolor() const {
  return has_textcolor_;
}

void BalloonStyle::clear_textcolor() {
  textcolor_ = kmlbase::Color32(0xff000000);
  has_textcolor_ = false;
}

const string& BalloonStyle::get_text() const {
  return text_;
}

bool BalloonStyle::has_text() const {
  return has_text_;
}

void BalloonStyle::set_text(const string& text) {
  text_ = text;
  has_text_ = true;
}

void BalloonStyle::clear_text() {
  text_.clear();
  has_text_ = false;
}

int BalloonStyle::get_displaymode() const {
  return displaymode_;
}

bool BalloonStyle::has_displaymode() const {
  return has_displaymode_;
}

void BalloonStyle::set_displaymode(int displaymode) {
  displaymode_ = displaymode;
  has_displaymode_ = true;
}

void BalloonStyle::clear_displaymode() {
  displaymode_ = DISPLAYMODE_DEFAULT;
  has_displaymode_ = false;
}
}  // end namespace kmldom
