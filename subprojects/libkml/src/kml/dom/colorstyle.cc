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

// This file contains the implementation of the ColorStyle element.

#include "kml/dom/colorstyle.h"
#include "kml/dom/element.h"
#include "kml/dom/serializer.h"

using kmlbase::Color32;

namespace kmldom {

ColorStyle::ColorStyle()
    : color_(Color32(0xffffffff)),
      has_color_(false),
      colormode_(COLORMODE_NORMAL),
      has_colormode_(false) {
}

ColorStyle::~ColorStyle() {
}

void ColorStyle::AddElement(const ElementPtr& element) {
  switch (element->Type()) {
    case Type_color:
      set_color(Color32(element->get_char_data()));
      break;
    case Type_colorMode:
      has_colormode_ = element->SetEnum(&colormode_);
      break;
    default:
      SubStyle::AddElement(element);
      break;
  }
}

void ColorStyle::Serialize(Serializer& serializer) const {
  SubStyle::Serialize(serializer);
  if (has_color()) {
    serializer.SaveColor(Type_color, get_color());
  }
  if (has_colormode()) {
    serializer.SaveEnum(Type_colorMode, get_colormode());
  }
}

kmldom::KmlDomType ColorStyle::Type() const {
  return Type_ColorStyle;
}

bool ColorStyle::IsA(kmldom::KmlDomType type) const {
  return type == Type_ColorStyle || SubStyle::IsA(type);
}

const kmlbase::Color32& ColorStyle::get_color() const {
  return color_;
}

bool ColorStyle::has_color() const {
  return has_color_;
}

void ColorStyle::set_color(const kmlbase::Color32& color) {
  color_ = color;
  has_color_ = true;
}

void ColorStyle::clear_color() {
  color_ = kmlbase::Color32(0xffffffff);
  has_color_ = false;
}

int ColorStyle::get_colormode() const {
  return colormode_;
}

bool ColorStyle::has_colormode() const {
  return has_colormode_;
}

void ColorStyle::set_colormode(int colormode) {
  colormode_ = colormode;
  has_colormode_ = true;
}

void ColorStyle::clear_colormode() {
  colormode_ = COLORMODE_NORMAL;
  has_colormode_ = false;
}
}  // end namespace kmldom
