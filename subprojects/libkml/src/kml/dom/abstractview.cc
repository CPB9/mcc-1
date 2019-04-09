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

// This file contains the implementation of the LookAt and Camera elements.

#include "kml/dom/abstractview.h"
#include "kml/base/attributes.h"
#include "kml/dom/gx_timeprimitive.h"
#include "kml/dom/kml_cast.h"
#include "kml/dom/serializer.h"
#include "kml/dom/visitor.h"
#include "kml/dom/visitor_driver.h"

using kmlbase::Attributes;

namespace kmldom {

// AbstractView
void AbstractView::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->IsA(Type_TimePrimitive)) {
    set_gx_timeprimitive(AsTimePrimitive(element));
    return;
  }
  Object::AddElement(element);
}

void AbstractView::Serialize(Serializer& serializer) const {
  if (has_gx_timeprimitive()) {
    serializer.SaveElementGroup(get_gx_timeprimitive(), Type_TimePrimitive);
  }
}

void AbstractView::AcceptChildren(VisitorDriver* driver) {
  Object::AcceptChildren(driver);
  if (has_gx_timeprimitive()) {
    driver->Visit(get_gx_timeprimitive());
  }
}

// AbstractViewCommon
AbstractViewCommon::AbstractViewCommon()
    : longitude_(0.0),
      has_longitude_(false),
      latitude_(0.0),
      has_latitude_(false),
      altitude_(0.0),
      has_altitude_(false),
      heading_(0.0),
      has_heading_(false),
      tilt_(0.0),
      has_tilt_(false),
      altitudemode_(ALTITUDEMODE_CLAMPTOGROUND),
      has_altitudemode_(false),
      gx_altitudemode_(GX_ALTITUDEMODE_CLAMPTOSEAFLOOR),
      has_gx_altitudemode_(false) {
}

void AbstractViewCommon::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_longitude:
      has_longitude_ = element->SetDouble(&longitude_);
      break;
    case Type_latitude:
      has_latitude_ = element->SetDouble(&latitude_);
      break;
    case Type_altitude:
      has_altitude_ = element->SetDouble(&altitude_);
      break;
    case Type_heading:
      has_heading_ = element->SetDouble(&heading_);
      break;
    case Type_tilt:
      has_tilt_ = element->SetDouble(&tilt_);
      break;
    case Type_altitudeMode:
      has_altitudemode_ = element->SetEnum(&altitudemode_);
      break;
    case Type_GxAltitudeMode:
      has_gx_altitudemode_ = element->SetEnum(&gx_altitudemode_);
      break;
    default:
      AbstractView::AddElement(element);
      break;
  }
}

void AbstractViewCommon::SerializeBeforeR(Serializer& serializer) const {
  AbstractView::Serialize(serializer);
  if (has_longitude()) {
    serializer.SaveFieldById(Type_longitude, get_longitude());
  }
  if (has_latitude()) {
    serializer.SaveFieldById(Type_latitude, get_latitude());
  }
  if (has_altitude()) {
    serializer.SaveFieldById(Type_altitude, get_altitude());
  }
  if (has_heading()) {
    serializer.SaveFieldById(Type_heading, get_heading());
  }
  if (has_tilt()) {
    serializer.SaveFieldById(Type_tilt, get_tilt());
  }
}

void AbstractViewCommon::SerializeAfterR(Serializer& serializer) const {
  if (has_altitudemode()) {
    serializer.SaveEnum(Type_altitudeMode, get_altitudemode());
  }
  if (has_gx_altitudemode()) {
    serializer.SaveEnum(Type_GxAltitudeMode, get_gx_altitudemode());
  }
}

// <LookAt>
LookAt::LookAt() : range_(0.0), has_range_(false) {
}

void LookAt::AddElement(const ElementPtr& element) {
  if (element && element->Type() == Type_range) {
    has_range_ = element->SetDouble(&range_);
  } else {
    AbstractViewCommon::AddElement(element);
  }
}

void LookAt::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  AbstractViewCommon::SerializeBeforeR(serializer);
  if (has_range()) {
    serializer.SaveFieldById(Type_range, get_range());
  }
  AbstractViewCommon::SerializeAfterR(serializer);
}

void LookAt::Accept(Visitor* visitor) {
  visitor->VisitLookAt(LookAtPtr(this));
}

// <Camera>
Camera::Camera() : roll_(0.0), has_roll_(false) {
}

void Camera::AddElement(const ElementPtr& element) {
  if (element && element->Type() == Type_roll) {
    has_roll_ = element->SetDouble(&roll_);
  } else {
    AbstractViewCommon::AddElement(element);
  }
}

void Camera::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  AbstractViewCommon::SerializeBeforeR(serializer);
  if (has_roll()) {
    serializer.SaveFieldById(Type_roll, get_roll());
  }
  AbstractViewCommon::SerializeAfterR(serializer);
}

void Camera::Accept(Visitor* visitor) {
  visitor->VisitCamera(CameraPtr(this));
}

kmldom::KmlDomType AbstractView::Type() const {
  return Type_AbstractView;
}

bool AbstractView::IsA(kmldom::KmlDomType type) const {
  return type == Type_AbstractView || Object::IsA(type);
}

const TimePrimitivePtr& AbstractView::get_gx_timeprimitive() const {
  return gx_timeprimitive_;
}

void AbstractView::set_gx_timeprimitive(
    const TimePrimitivePtr& gx_timeprimitive) {
  SetComplexChild(gx_timeprimitive, &gx_timeprimitive_);
}

bool AbstractView::has_gx_timeprimitive() const {
  return gx_timeprimitive_ != nullptr;
}

void AbstractView::clear_gx_timeprimitive() {
  set_gx_timeprimitive(NULL);
}

AbstractView::AbstractView() {
}

kmldom::KmlDomType LookAt::Type() const {
  return Type_LookAt;
}

bool LookAt::IsA(kmldom::KmlDomType type) const {
  return type == Type_LookAt || AbstractView::IsA(type);
}

LookAt::~LookAt() {
}

Camera::~Camera() {
}

kmldom::KmlDomType Camera::Type() const {
  return Type_Camera;
}

bool Camera::IsA(kmldom::KmlDomType type) const {
  return type == Type_Camera || AbstractView::IsA(type);
}

void Camera::clear_roll() {
  roll_ = 0.0;
  has_roll_ = false;
}

bool Camera::has_roll() const {
  return has_roll_;
}

double Camera::get_roll() const {
  return roll_;
}

void Camera::set_roll(double roll) {
  roll_ = roll;
  has_roll_ = true;
}

void LookAt::clear_range() {
  range_ = 0.0;
  has_range_ = false;
}

void LookAt::set_range(double range) {
  range_ = range;
  has_range_ = true;
}

bool LookAt::has_range() const {
  return has_range_;
}

double LookAt::get_range() const {
  return range_;
}

void AbstractViewCommon::clear_gx_altitudemode() {
  gx_altitudemode_ = GX_ALTITUDEMODE_CLAMPTOSEAFLOOR;
  has_gx_altitudemode_ = false;
}

void AbstractViewCommon::set_gx_altitudemode(int gx_altitudemode) {
  gx_altitudemode_ = gx_altitudemode;
  has_gx_altitudemode_ = true;
}

bool AbstractViewCommon::has_gx_altitudemode() const {
  return has_gx_altitudemode_;
}

int AbstractViewCommon::get_gx_altitudemode() const {
  return gx_altitudemode_;
}

void AbstractViewCommon::clear_altitudemode() {
  altitudemode_ = ALTITUDEMODE_CLAMPTOGROUND;
  has_altitudemode_ = false;
}

bool AbstractViewCommon::has_altitudemode() const {
  return has_altitudemode_;
}

void AbstractViewCommon::set_altitudemode(int altitudemode) {
  altitudemode_ = altitudemode;
  has_altitudemode_ = true;
}

int AbstractViewCommon::get_altitudemode() const {
  return altitudemode_;
}

void AbstractViewCommon::clear_tilt() {
  tilt_ = 0.0;
  has_tilt_ = false;
}

void AbstractViewCommon::set_tilt(double tilt) {
  tilt_ = tilt;
  has_tilt_ = true;
}

bool AbstractViewCommon::has_tilt() const {
  return has_tilt_;
}

double AbstractViewCommon::get_tilt() const {
  return tilt_;
}

void AbstractViewCommon::clear_heading() {
  heading_ = 0.0;
  has_heading_ = false;
}

void AbstractViewCommon::set_heading(double heading) {
  heading_ = heading;
  has_heading_ = true;
}

bool AbstractViewCommon::has_heading() const {
  return has_heading_;
}

double AbstractViewCommon::get_heading() const {
  return heading_;
}

void AbstractViewCommon::clear_altitude() {
  altitude_ = 0.0;
  has_altitude_ = false;
}

void AbstractViewCommon::set_altitude(double altitude) {
  altitude_ = altitude;
  has_altitude_ = true;
}

bool AbstractViewCommon::has_altitude() const {
  return has_altitude_;
}

double AbstractViewCommon::get_altitude() const {
  return altitude_;
}

void AbstractViewCommon::clear_latitude() {
  latitude_ = 0.0;
  has_latitude_ = false;
}

void AbstractViewCommon::set_latitude(double latitude) {
  latitude_ = latitude;
  has_latitude_ = true;
}

bool AbstractViewCommon::has_latitude() const {
  return has_latitude_;
}

double AbstractViewCommon::get_latitude() const {
  return latitude_;
}

void AbstractViewCommon::clear_longitude() {
  longitude_ = 0.0;
  has_longitude_ = false;
}

void AbstractViewCommon::set_longitude(double longitude) {
  longitude_ = longitude;
  has_longitude_ = true;
}

bool AbstractViewCommon::has_longitude() const {
  return has_longitude_;
}

double AbstractViewCommon::get_longitude() const {
  return longitude_;
}

AbstractView::~AbstractView() {
}

AbstractViewCommon::~AbstractViewCommon() {
}
}  // end namespace kmldom
