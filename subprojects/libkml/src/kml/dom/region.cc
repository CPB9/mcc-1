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

// This file contains the implementation of the LatLonAltBox, Lod
// and Region elements.

#include "kml/dom/region.h"
#include "kml/base/attributes.h"
#include "kml/dom/abstractlatlonbox.h"
#include "kml/dom/kml_cast.h"
#include "kml/dom/serializer.h"
#include "kml/dom/visitor.h"

using kmlbase::Attributes;

namespace kmldom {

LatLonAltBox::LatLonAltBox()
    : minaltitude_(0.0),
      has_minaltitude_(false),
      maxaltitude_(0.0),
      has_maxaltitude_(false),
      altitudemode_(ALTITUDEMODE_CLAMPTOGROUND),
      has_altitudemode_(false),
      gx_altitudemode_(GX_ALTITUDEMODE_CLAMPTOSEAFLOOR),
      has_gx_altitudemode_(false) {
}

LatLonAltBox::~LatLonAltBox() {
}

void LatLonAltBox::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_minAltitude:
      has_minaltitude_ = element->SetDouble(&minaltitude_);
      break;
    case Type_maxAltitude:
      has_maxaltitude_ = element->SetDouble(&maxaltitude_);
      break;
    case Type_altitudeMode:
      has_altitudemode_ = element->SetEnum(&altitudemode_);
      break;
    case Type_GxAltitudeMode:
      has_gx_altitudemode_ = element->SetEnum(&gx_altitudemode_);
      break;
    default:
      AbstractLatLonBox::AddElement(element);
      break;
  }
}

void LatLonAltBox::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  AbstractLatLonBox::Serialize(serializer);
  if (has_minaltitude()) {
    serializer.SaveFieldById(Type_minAltitude, get_minaltitude());
  }
  if (has_maxaltitude()) {
    serializer.SaveFieldById(Type_maxAltitude, get_maxaltitude());
  }
  if (has_altitudemode()) {
    serializer.SaveEnum(Type_altitudeMode, get_altitudemode());
  }
  if (has_gx_altitudemode()) {
    serializer.SaveEnum(Type_GxAltitudeMode, get_gx_altitudemode());
  }
}

void LatLonAltBox::Accept(Visitor* visitor) {
  visitor->VisitLatLonAltBox(LatLonAltBoxPtr(this));
}

Lod::Lod()
    : minlodpixels_(0.0),
      has_minlodpixels_(false),
      maxlodpixels_(-1.0),
      has_maxlodpixels_(false),
      minfadeextent_(0.0),
      has_minfadeextent_(false),
      maxfadeextent_(0.0),
      has_maxfadeextent_(false) {
}

Lod::~Lod() {
}

void Lod::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_minLodPixels:
      has_minlodpixels_ = element->SetDouble(&minlodpixels_);
      break;
    case Type_maxLodPixels:
      has_maxlodpixels_ = element->SetDouble(&maxlodpixels_);
      break;
    case Type_minFadeExtent:
      has_minfadeextent_ = element->SetDouble(&minfadeextent_);
      break;
    case Type_maxFadeExtent:
      has_maxfadeextent_ = element->SetDouble(&maxfadeextent_);
      break;
    default:
      Object::AddElement(element);
      break;
  }
}

void Lod::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Object::Serialize(serializer);
  if (has_minlodpixels()) {
    serializer.SaveFieldById(Type_minLodPixels, get_minlodpixels());
  }
  if (has_maxlodpixels()) {
    serializer.SaveFieldById(Type_maxLodPixels, get_maxlodpixels());
  }
  if (has_minfadeextent()) {
    serializer.SaveFieldById(Type_minFadeExtent, get_minfadeextent());
  }
  if (has_maxfadeextent()) {
    serializer.SaveFieldById(Type_maxFadeExtent, get_maxfadeextent());
  }
}

void Lod::Accept(Visitor* visitor) {
  visitor->VisitLod(LodPtr(this));
}

Region::Region() {
}

Region::~Region() {
}

void Region::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_LatLonAltBox:
      set_latlonaltbox(AsLatLonAltBox(element));
      break;
    case Type_Lod:
      set_lod(AsLod(element));
      break;
    default:
      Object::AddElement(element);
  }
}

void Region::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Object::Serialize(serializer);
  if (has_latlonaltbox()) {
    serializer.SaveElement(get_latlonaltbox());
  }
  if (has_lod()) {
    serializer.SaveElement(get_lod());
  }
}

void Region::Accept(Visitor* visitor) {
  visitor->VisitRegion(RegionPtr(this));
}

void Region::AcceptChildren(VisitorDriver* driver) {
  Object::AcceptChildren(driver);
  if (has_latlonaltbox()) {
    driver->Visit(get_latlonaltbox());
  }
  if (has_lod()) {
    driver->Visit(get_lod());
  }
}

kmldom::KmlDomType LatLonAltBox::Type() const {
  return Type_LatLonAltBox;
}

bool LatLonAltBox::IsA(kmldom::KmlDomType type) const {
  return type == Type_LatLonAltBox || AbstractLatLonBox::IsA(type);
}

double LatLonAltBox::get_minaltitude() const {
  return minaltitude_;
}

bool LatLonAltBox::has_minaltitude() const {
  return has_minaltitude_;
}

void LatLonAltBox::set_minaltitude(double minaltitude) {
  minaltitude_ = minaltitude;
  has_minaltitude_ = true;
}

void LatLonAltBox::clear_minaltitude() {
  minaltitude_ = 0.0;
  has_minaltitude_ = false;
}

double LatLonAltBox::get_maxaltitude() const {
  return maxaltitude_;
}

bool LatLonAltBox::has_maxaltitude() const {
  return has_maxaltitude_;
}

void LatLonAltBox::set_maxaltitude(double maxaltitude) {
  maxaltitude_ = maxaltitude;
  has_maxaltitude_ = true;
}

void LatLonAltBox::clear_maxaltitude() {
  maxaltitude_ = 0.0;
  has_maxaltitude_ = false;
}

int LatLonAltBox::get_altitudemode() const {
  return altitudemode_;
}

void LatLonAltBox::set_altitudemode(int altitudemode) {
  altitudemode_ = altitudemode;
  has_altitudemode_ = true;
}

bool LatLonAltBox::has_altitudemode() const {
  return has_altitudemode_;
}

void LatLonAltBox::clear_altitudemode() {
  altitudemode_ = ALTITUDEMODE_CLAMPTOGROUND;
  has_altitudemode_ = false;
}

int LatLonAltBox::get_gx_altitudemode() const {
  return gx_altitudemode_;
}

bool LatLonAltBox::has_gx_altitudemode() const {
  return has_gx_altitudemode_;
}

void LatLonAltBox::set_gx_altitudemode(int gx_altitudemode) {
  gx_altitudemode_ = gx_altitudemode;
  has_gx_altitudemode_ = true;
}

void LatLonAltBox::clear_gx_altitudemode() {
  gx_altitudemode_ = GX_ALTITUDEMODE_CLAMPTOSEAFLOOR;
  has_gx_altitudemode_ = false;
}

kmldom::KmlDomType Lod::Type() const {
  return Type_Lod;
}

bool Lod::IsA(kmldom::KmlDomType type) const {
  return type == Type_Lod || Object::IsA(type);
}

double Lod::get_minlodpixels() const {
  return minlodpixels_;
}

bool Lod::has_minlodpixels() const {
  return has_minlodpixels_;
}

void Lod::set_minlodpixels(double minlodpixels) {
  minlodpixels_ = minlodpixels;
  has_minlodpixels_ = true;
}

void Lod::clear_minlodpixels() {
  minlodpixels_ = 0.0;
  has_minlodpixels_ = false;
}

double Lod::get_maxlodpixels() const {
  return maxlodpixels_;
}

bool Lod::has_maxlodpixels() const {
  return has_maxlodpixels_;
}

void Lod::set_maxlodpixels(double minlodpixels) {
  maxlodpixels_ = minlodpixels;
  has_maxlodpixels_ = true;
}

void Lod::clear_maxlodpixels() {
  maxlodpixels_ = -1.0;
  has_maxlodpixels_ = false;
}

double Lod::get_minfadeextent() const {
  return minfadeextent_;
}

bool Lod::has_minfadeextent() const {
  return has_minfadeextent_;
}

void Lod::set_minfadeextent(double minlodpixels) {
  minfadeextent_ = minlodpixels;
  has_minfadeextent_ = true;
}

void Lod::clear_minfadeextent() {
  minfadeextent_ = 0.0;
  has_minfadeextent_ = false;
}

double Lod::get_maxfadeextent() const {
  return maxfadeextent_;
}

bool Lod::has_maxfadeextent() const {
  return has_maxfadeextent_;
}

void Lod::set_maxfadeextent(double maxlodpixels) {
  maxfadeextent_ = maxlodpixels;
  has_maxfadeextent_ = true;
}

void Lod::clear_maxfadeextent() {
  maxfadeextent_ = 0.0;
  has_maxfadeextent_ = false;
}

kmldom::KmlDomType Region::Type() const {
  return Type_Region;
}

bool Region::IsA(kmldom::KmlDomType type) const {
  return type == Type_Region || Object::IsA(type);
}

const LatLonAltBoxPtr& Region::get_latlonaltbox() const {
  return latlonaltbox_;
}

bool Region::has_latlonaltbox() const {
  return latlonaltbox_ != nullptr;
}

void Region::set_latlonaltbox(const LatLonAltBoxPtr& latlonaltbox) {
  SetComplexChild(latlonaltbox, &latlonaltbox_);
}

void Region::clear_latlonaltbox() {
  set_latlonaltbox(NULL);
}

const LodPtr& Region::get_lod() const {
  return lod_;
}

bool Region::has_lod() const {
  return lod_ != nullptr;
}

void Region::set_lod(const LodPtr& lod) {
  SetComplexChild(lod, &lod_);
}

void Region::clear_lod() {
  set_lod(NULL);
}
}  // end namespace kmldom
