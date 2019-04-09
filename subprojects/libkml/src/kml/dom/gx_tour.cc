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

// This file contains the implementation of the <gx:Tour> and related elements.

#include "kml/dom/gx_tour.h"
#include "kml/base/attributes.h"
#include "kml/base/xml_namespaces.h"
#include "kml/dom/abstractview.h"
#include "kml/dom/kml_cast.h"
#include "kml/dom/networklinkcontrol.h"
#include "kml/dom/serializer.h"
#include "kml/dom/visitor.h"

using kmlbase::Attributes;

namespace kmldom {

// <gx:Tour>

GxTour::GxTour() {
  set_xmlns(kmlbase::XMLNS_GX22);
}

GxTour::~GxTour() {
}

void GxTour::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->Type() == Type_GxPlaylist) {
    set_gx_playlist(AsGxPlaylist(element));
  } else {
    Feature::AddElement(element);
  }
}

void GxTour::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Feature::Serialize(serializer);
  if (has_gx_playlist()) {
    serializer.SaveElement(get_gx_playlist());
  }
}

void GxTour::Accept(Visitor* visitor) {
  visitor->VisitGxTour(GxTourPtr(this));
}

void GxTour::AcceptChildren(VisitorDriver* driver) {
  Feature::AcceptChildren(driver);
  if (has_gx_playlist()) {
    driver->Visit(get_gx_playlist());
  }
}

// <gx:Playlist>

GxPlaylist::GxPlaylist() {
  set_xmlns(kmlbase::XMLNS_GX22);
}

GxPlaylist::~GxPlaylist() {
}

void GxPlaylist::add_gx_tourprimitive(
    const GxTourPrimitivePtr& gx_tourprimitive) {
  gx_tourprimitive_array_.push_back(gx_tourprimitive);
}

size_t GxPlaylist::get_gx_tourprimitive_array_size() const {
  return gx_tourprimitive_array_.size();
}

const GxTourPrimitivePtr& GxPlaylist::get_gx_tourprimitive_array_at(
    size_t index) const {
  return gx_tourprimitive_array_[index];
}

void GxPlaylist::AddElement(const ElementPtr& element) {
  if (GxTourPrimitivePtr gx_tourprimitive = AsGxTourPrimitive(element)) {
    add_gx_tourprimitive(gx_tourprimitive);
  } else {
    Element::AddElement(element);
  }
}

void GxPlaylist::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  for (size_t i = 0; i < get_gx_tourprimitive_array_size(); ++i) {
    serializer.SaveElementGroup(get_gx_tourprimitive_array_at(i),
                                Type_GxTourPrimitive);
  }
}

void GxPlaylist::Accept(Visitor* visitor) {
  visitor->VisitGxPlaylist(GxPlaylistPtr(this));
}

void GxPlaylist::AcceptChildren(VisitorDriver* driver) {
  Object::AcceptChildren(driver);
  Element::AcceptRepeated<GxTourPrimitivePtr>(&gx_tourprimitive_array_, driver);
}

// TourPrimitiveCommon

void GxTourPrimitiveCommon::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->Type() == Type_GxDuration) {
    has_gx_duration_ = element->SetDouble(&gx_duration_);
    return;
  }
  GxTourPrimitive::AddElement(element);
}

void GxTourPrimitiveCommon::Serialize(Serializer& serializer) const {
  if (has_gx_duration()) {
    serializer.SaveFieldById(Type_GxDuration, get_gx_duration());
  }
}

// <gx:AnimatedUpdate>

GxAnimatedUpdate::GxAnimatedUpdate() {
  set_xmlns(kmlbase::XMLNS_GX22);
}

GxAnimatedUpdate::~GxAnimatedUpdate() {
}

void GxAnimatedUpdate::AddElement(const ElementPtr& element) {
  if (UpdatePtr update = AsUpdate(element)) {
    set_update(update);
    return;
  }
  GxTourPrimitiveCommon::AddElement(element);
}

void GxAnimatedUpdate::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  GxTourPrimitiveCommon::Serialize(serializer);
  if (has_update()) {
    serializer.SaveElement(get_update());
  }
}

void GxAnimatedUpdate::Accept(Visitor* visitor) {
  visitor->VisitGxAnimatedUpdate(GxAnimatedUpdatePtr(this));
}

void GxAnimatedUpdate::AcceptChildren(VisitorDriver* driver) {
  GxTourPrimitiveCommon::AcceptChildren(driver);
  if (has_update()) {
    driver->Visit(get_update());
  }
}

// <gx:FlyTo>

GxFlyTo::GxFlyTo()
    : gx_flytomode_(GX_FLYTOMODE_BOUNCE), has_gx_flytomode_(false) {
  set_xmlns(kmlbase::XMLNS_GX22);
}

GxFlyTo::~GxFlyTo() {
}

void GxFlyTo::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->Type() == Type_GxFlyToMode) {
    has_gx_flytomode_ = element->SetEnum(&gx_flytomode_);
    return;
  }
  if (AbstractViewPtr abstractview = AsAbstractView(element)) {
    set_abstractview(abstractview);
    return;
  }
  GxTourPrimitiveCommon::AddElement(element);
}

void GxFlyTo::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  GxTourPrimitiveCommon::Serialize(serializer);
  if (has_gx_flytomode()) {
    serializer.SaveEnum(Type_GxFlyToMode, get_gx_flytomode());
  }
  if (has_abstractview()) {
    serializer.SaveElement(get_abstractview());
  }
}

void GxFlyTo::Accept(Visitor* visitor) {
  visitor->VisitGxFlyTo(GxFlyToPtr(this));
}

void GxFlyTo::AcceptChildren(VisitorDriver* driver) {
  GxTourPrimitiveCommon::AcceptChildren(driver);
  if (has_abstractview()) {
    driver->Visit(get_abstractview());
  }
}

// <gx:Wait>

GxWait::GxWait() {
  set_xmlns(kmlbase::XMLNS_GX22);
}

GxWait::~GxWait() {
}

void GxWait::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  GxTourPrimitiveCommon::Serialize(serializer);
}

void GxWait::Accept(Visitor* visitor) {
  visitor->VisitGxWait(GxWaitPtr(this));
}

// <gx:SoundCue>

GxSoundCue::GxSoundCue() : has_href_(false) {
  set_xmlns(kmlbase::XMLNS_GX22);
}

GxSoundCue::~GxSoundCue() {
}

void GxSoundCue::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->Type() == Type_href) {
    has_href_ = element->SetString(&href_);
    return;
  }
  GxTourPrimitive::AddElement(element);
}

void GxSoundCue::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  GxTourPrimitive::Serialize(serializer);
  if (has_href()) {
    serializer.SaveFieldById(Type_href, get_href());
  }
}

void GxSoundCue::Accept(Visitor* visitor) {
  visitor->VisitGxSoundCue(GxSoundCuePtr(this));
}

// <gx:TourControl>

GxTourControl::GxTourControl()
    : gx_playmode_(GX_PLAYMODE_PAUSE), has_gx_playmode_(false) {
  set_xmlns(kmlbase::XMLNS_GX22);
}

GxTourControl::~GxTourControl() {
}

void GxTourControl::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->Type() == Type_GxPlayMode) {
    has_gx_playmode_ = element->SetEnum(&gx_playmode_);
    return;
  }
  GxTourPrimitive::AddElement(element);
}

void GxTourControl::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  GxTourPrimitive::Serialize(serializer);
  if (has_gx_playmode()) {
    serializer.SaveEnum(Type_GxPlayMode, get_gx_playmode());
  }
}

void GxTourControl::Accept(Visitor* visitor) {
  visitor->VisitGxTourControl(GxTourControlPtr(this));
}

kmldom::KmlDomType GxTour::ElementType() {
  return Type_GxTour;
}

kmldom::KmlDomType GxTour::Type() const {
  return ElementType();
}

bool GxTour::IsA(kmldom::KmlDomType type) const {
  return type == ElementType() || Feature::IsA(type);
}

const GxPlaylistPtr& GxTour::get_gx_playlist() const {
  return gx_playlist_;
}

bool GxTour::has_gx_playlist() const {
  return gx_playlist_ != nullptr;
}

void GxTour::set_gx_playlist(const GxPlaylistPtr& gx_playlist) {
  SetComplexChild(gx_playlist, &gx_playlist_);
}

void GxTour::clear_gx_playlist() {
  set_gx_playlist(NULL);
}

kmldom::KmlDomType GxPlaylist::ElementType() {
  return Type_GxPlaylist;
}

kmldom::KmlDomType GxPlaylist::Type() const {
  return ElementType();
}

bool GxPlaylist::IsA(kmldom::KmlDomType type) const {
  return type == ElementType() || Object::IsA(type);
}

kmldom::KmlDomType GxTourPrimitive::ElementType() {
  return static_cast<KmlDomType>(Type_GxTourPrimitive);
}

kmldom::KmlDomType GxTourPrimitive::Type() const {
  return ElementType();
}

bool GxTourPrimitive::IsA(kmldom::KmlDomType type) const {
  return type == ElementType() || Object::IsA(type);
}

GxTourPrimitive::GxTourPrimitive() {
}

double GxTourPrimitiveCommon::get_gx_duration() const {
  return gx_duration_;
}

bool GxTourPrimitiveCommon::has_gx_duration() const {
  return has_gx_duration_;
}

void GxTourPrimitiveCommon::set_gx_duration(double gx_duration) {
  gx_duration_ = gx_duration;
  has_gx_duration_ = true;
}

void GxTourPrimitiveCommon::clear_gx_duration() {
  gx_duration_ = 0.0;
  has_gx_duration_ = false;
}

GxTourPrimitiveCommon::GxTourPrimitiveCommon()
    : has_gx_duration_(false), gx_duration_(0.0) {
}

kmldom::KmlDomType GxAnimatedUpdate::ElementType() {
  return Type_GxAnimatedUpdate;
}

kmldom::KmlDomType GxAnimatedUpdate::Type() const {
  return ElementType();
}

bool GxAnimatedUpdate::IsA(kmldom::KmlDomType type) const {
  return type == ElementType() || GxTourPrimitive::IsA(type);
}

const UpdatePtr& GxAnimatedUpdate::get_update() const {
  return update_;
}

bool GxAnimatedUpdate::has_update() const {
  return update_ != nullptr;
}

void GxAnimatedUpdate::set_update(const UpdatePtr& update) {
  SetComplexChild(update, &update_);
}

void GxAnimatedUpdate::clear_update() {
  set_update(NULL);
}

kmldom::KmlDomType GxFlyTo::ElementType() {
  return Type_GxFlyTo;
}

kmldom::KmlDomType GxFlyTo::Type() const {
  return ElementType();
}

bool GxFlyTo::IsA(kmldom::KmlDomType type) const {
  return type == ElementType() || GxTourPrimitive::IsA(type);
}

int GxFlyTo::get_gx_flytomode() const {
  return gx_flytomode_;
}

bool GxFlyTo::has_gx_flytomode() const {
  return has_gx_flytomode_;
}

void GxFlyTo::set_gx_flytomode(int value) {
  gx_flytomode_ = value;
  has_gx_flytomode_ = true;
}

void GxFlyTo::clear_gx_flytomode() {
  gx_flytomode_ = kmldom::GX_FLYTOMODE_BOUNCE;
  has_gx_flytomode_ = false;
}

const AbstractViewPtr& GxFlyTo::get_abstractview() const {
  return abstractview_;
}

bool GxFlyTo::has_abstractview() const {
  return abstractview_ != nullptr;
}

void GxFlyTo::set_abstractview(const AbstractViewPtr& abstractview) {
  SetComplexChild(abstractview, &abstractview_);
}

void GxFlyTo::clear_abstractview() {
  set_abstractview(NULL);
}

kmldom::KmlDomType GxWait::ElementType() {
  return Type_GxWait;
}

kmldom::KmlDomType GxWait::Type() const {
  return ElementType();
}

bool GxWait::IsA(kmldom::KmlDomType type) const {
  return type == ElementType() || GxTourPrimitive::IsA(type);
}

kmldom::KmlDomType GxSoundCue::ElementType() {
  return Type_GxSoundCue;
}

kmldom::KmlDomType GxSoundCue::Type() const {
  return ElementType();
}

bool GxSoundCue::IsA(kmldom::KmlDomType type) const {
  return type == ElementType() || GxTourPrimitive::IsA(type);
}

const string& GxSoundCue::get_href() const {
  return href_;
}

bool GxSoundCue::has_href() const {
  return has_href_;
}

void GxSoundCue::set_href(const string& href) {
  href_ = href;
  has_href_ = true;
}

void GxSoundCue::clear_href() {
  href_.clear();
  has_href_ = false;
}

kmldom::KmlDomType GxTourControl::ElementType() {
  return Type_GxTourControl;
}

kmldom::KmlDomType GxTourControl::Type() const {
  return ElementType();
}

bool GxTourControl::IsA(kmldom::KmlDomType type) const {
  return type == ElementType() || GxTourPrimitive::IsA(type);
}

int GxTourControl::get_gx_playmode() const {
  return gx_playmode_;
}

bool GxTourControl::has_gx_playmode() const {
  return has_gx_playmode_;
}

void GxTourControl::set_gx_playmode(int value) {
  gx_playmode_ = value;
  has_gx_playmode_ = true;
}

void GxTourControl::clear_gx_playmode() {
  gx_playmode_ = GX_PLAYMODE_PAUSE;
  has_gx_playmode_ = false;
}
}  // end namespace kmldom
