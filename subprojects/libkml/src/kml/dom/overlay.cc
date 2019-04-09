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

// This file contains the implementation of the abstract Overlay and concrete
// LatLonBox, GroundOverlay, ViewVolume, ImagePyramid, PhotoOverlay, OverlayXY,
// ScreenXY, RotationXY, Size, and ScreenOverlay elements.

#include "kml/dom/overlay.h"
#include "kml/base/attributes.h"
#include "kml/dom/geometry.h"
#include "kml/dom/kml22.h"
#include "kml/dom/kml_cast.h"
#include "kml/dom/link.h"
#include "kml/dom/serializer.h"
#include "kml/dom/visitor.h"

using kmlbase::Attributes;
using kmlbase::Color32;

namespace kmldom {

Overlay::Overlay()
    : color_(Color32(0xffffffff)),
      has_color_(false),
      draworder_(0),
      has_draworder_(false) {
}

Overlay::~Overlay() {
}

void Overlay::AddElement(const ElementPtr& element) {
  switch (element->Type()) {
    case Type_color:
      set_color(Color32(element->get_char_data()));
      break;
    case Type_drawOrder:
      has_draworder_ = element->SetInt(&draworder_);
      break;
    case Type_Icon:
      set_icon(AsIcon(element));
      break;
    default:
      Feature::AddElement(element);
      break;
  }
}

void Overlay::Serialize(Serializer& serializer) const {
  Feature::Serialize(serializer);
  if (has_color()) {
    serializer.SaveColor(Type_color, get_color());
  }
  if (has_draworder()) {
    serializer.SaveFieldById(Type_drawOrder, get_draworder());
  }
  if (has_icon()) {
    serializer.SaveElement(get_icon());
  }
}

void Overlay::AcceptChildren(VisitorDriver* driver) {
  Feature::AcceptChildren(driver);
  if (has_icon()) {
    driver->Visit(get_icon());
  }
}

LatLonBox::LatLonBox() : rotation_(0.0), has_rotation_(false) {
}

LatLonBox::~LatLonBox() {
}

void LatLonBox::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_rotation:
      has_rotation_ = element->SetDouble(&rotation_);
      break;
    default:
      AbstractLatLonBox::AddElement(element);
      break;
  }
}

void LatLonBox::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  AbstractLatLonBox::Serialize(serializer);
  if (has_rotation()) {
    serializer.SaveFieldById(Type_rotation, get_rotation());
  }
}

void LatLonBox::Accept(Visitor* visitor) {
  visitor->VisitLatLonBox(LatLonBoxPtr(this));
}

GxLatLonQuad::GxLatLonQuad() {
  set_xmlns(kmlbase::XMLNS_GX22);
}

GxLatLonQuad::~GxLatLonQuad() {
}

void GxLatLonQuad::AddElement(const ElementPtr& element) {
  if (CoordinatesPtr coordinates = AsCoordinates(element)) {
    set_coordinates(coordinates);
  } else {
    Object::AddElement(element);
  }
}

void GxLatLonQuad::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Object::Serialize(serializer);
  if (has_coordinates()) {
    serializer.SaveElement(get_coordinates());
  }
}

void GxLatLonQuad::Accept(Visitor* visitor) {
  visitor->VisitGxLatLonQuad(GxLatLonQuadPtr(this));
}

void GxLatLonQuad::AcceptChildren(VisitorDriver* driver) {
  Object::AcceptChildren(driver);
  if (has_coordinates()) {
    driver->Visit(get_coordinates());
  }
}

GroundOverlay::GroundOverlay()
    : altitude_(0.0),
      has_altitude_(false),
      altitudemode_(ALTITUDEMODE_CLAMPTOGROUND),
      has_altitudemode_(false),
      gx_altitudemode_(GX_ALTITUDEMODE_CLAMPTOSEAFLOOR),
      has_gx_altitudemode_(false) {
}

GroundOverlay::~GroundOverlay() {
}

void GroundOverlay::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_altitude:
      has_altitude_ = element->SetDouble(&altitude_);
      break;
    case Type_altitudeMode:
      has_altitudemode_ = element->SetEnum(&altitudemode_);
      break;
    case Type_GxAltitudeMode:
      has_gx_altitudemode_ = element->SetEnum(&gx_altitudemode_);
      break;
    case Type_LatLonBox:
      set_latlonbox(AsLatLonBox(element));
      break;
    case Type_GxLatLonQuad:
      set_gx_latlonquad(AsGxLatLonQuad(element));
      break;
    default:
      Overlay::AddElement(element);
      break;
  }
}

void GroundOverlay::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Overlay::Serialize(serializer);
  if (has_altitude()) {
    serializer.SaveFieldById(Type_altitude, get_altitude());
  }
  if (has_altitudemode()) {
    serializer.SaveEnum(Type_altitudeMode, get_altitudemode());
  }
  if (has_gx_altitudemode()) {
    serializer.SaveEnum(Type_GxAltitudeMode, get_gx_altitudemode());
  }
  if (has_latlonbox()) {
    serializer.SaveElement(get_latlonbox());
  }
  if (has_gx_latlonquad()) {
    serializer.SaveElement(get_gx_latlonquad());
  }
}

void GroundOverlay::Accept(Visitor* visitor) {
  visitor->VisitGroundOverlay(GroundOverlayPtr(this));
}

void GroundOverlay::AcceptChildren(VisitorDriver* driver) {
  Overlay::AcceptChildren(driver);
  if (has_latlonbox()) {
    driver->Visit(get_latlonbox());
  }
  if (has_gx_latlonquad()) {
    driver->Visit(get_gx_latlonquad());
  }
}

ViewVolume::ViewVolume()
    : leftfov_(0.0),
      has_leftfov_(false),
      rightfov_(0.0),
      has_rightfov_(false),
      bottomfov_(0.0),
      has_bottomfov_(false),
      topfov_(0.0),
      has_topfov_(false),
      near_(0.0),
      has_near_(false) {
}

ViewVolume::~ViewVolume() {
}

void ViewVolume::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_leftFov:
      has_leftfov_ = element->SetDouble(&leftfov_);
      break;
    case Type_rightFov:
      has_rightfov_ = element->SetDouble(&rightfov_);
      break;
    case Type_bottomFov:
      has_bottomfov_ = element->SetDouble(&bottomfov_);
      break;
    case Type_topFov:
      has_topfov_ = element->SetDouble(&topfov_);
      break;
    case Type_near:
      has_near_ = element->SetDouble(&near_);
      break;
    default:
      Object::AddElement(element);
      break;
  }
}

void ViewVolume::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Object::Serialize(serializer);
  if (has_leftfov()) {
    serializer.SaveFieldById(Type_leftFov, get_leftfov());
  }
  if (has_rightfov()) {
    serializer.SaveFieldById(Type_rightFov, get_rightfov());
  }
  if (has_bottomfov()) {
    serializer.SaveFieldById(Type_bottomFov, get_bottomfov());
  }
  if (has_topfov()) {
    serializer.SaveFieldById(Type_topFov, get_topfov());
  }
  if (has_near()) {
    serializer.SaveFieldById(Type_near, get_near());
  }
}

void ViewVolume::Accept(Visitor* visitor) {
  visitor->VisitViewVolume(ViewVolumePtr(this));
}

ImagePyramid::ImagePyramid()
    : tilesize_(256),
      has_tilesize_(false),
      maxwidth_(0),
      has_maxwidth_(false),
      maxheight_(0),
      has_maxheight_(false),
      gridorigin_(GRIDORIGIN_LOWERLEFT),
      has_gridorigin_(false) {
}

ImagePyramid::~ImagePyramid() {
}

void ImagePyramid::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_tileSize:
      has_tilesize_ = element->SetInt(&tilesize_);
      break;
    case Type_maxWidth:
      has_maxwidth_ = element->SetInt(&maxwidth_);
      break;
    case Type_maxHeight:
      has_maxheight_ = element->SetInt(&maxheight_);
      break;
    case Type_gridOrigin:
      has_gridorigin_ = element->SetEnum(&gridorigin_);
      break;
    default:
      Object::AddElement(element);
      break;
  }
}

void ImagePyramid::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Object::Serialize(serializer);
  if (has_tilesize()) {
    serializer.SaveFieldById(Type_tileSize, get_tilesize());
  }
  if (has_maxwidth()) {
    serializer.SaveFieldById(Type_maxWidth, get_maxwidth());
  }
  if (has_maxheight()) {
    serializer.SaveFieldById(Type_maxHeight, get_maxheight());
  }
  if (has_gridorigin()) {
    serializer.SaveEnum(Type_gridOrigin, get_gridorigin());
  }
}

void ImagePyramid::Accept(Visitor* visitor) {
  visitor->VisitImagePyramid(ImagePyramidPtr(this));
}

PhotoOverlay::PhotoOverlay()
    : rotation_(0.0),
      has_rotation_(false),
      shape_(SHAPE_RECTANGLE),
      has_shape_(false) {
}

PhotoOverlay::~PhotoOverlay() {
}

void PhotoOverlay::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_rotation:
      has_rotation_ = element->SetDouble(&rotation_);
      break;
    case Type_ViewVolume:
      set_viewvolume(AsViewVolume(element));
      break;
    case Type_ImagePyramid:
      set_imagepyramid(AsImagePyramid(element));
      break;
    case Type_Point:
      set_point(AsPoint(element));
      break;
    case Type_shape:
      has_shape_ = element->SetEnum(&shape_);
      break;
    default:
      Overlay::AddElement(element);
      break;
  }
}

void PhotoOverlay::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Overlay::Serialize(serializer);
  if (has_rotation()) {
    serializer.SaveFieldById(Type_rotation, get_rotation());
  }
  if (has_viewvolume()) {
    serializer.SaveElement(get_viewvolume());
  }
  if (has_imagepyramid()) {
    serializer.SaveElement(get_imagepyramid());
  }
  if (has_point()) {
    serializer.SaveElement(get_point());
  }
  if (has_shape()) {
    serializer.SaveEnum(Type_shape, get_shape());
  }
}

void PhotoOverlay::Accept(Visitor* visitor) {
  visitor->VisitPhotoOverlay(PhotoOverlayPtr(this));
}

void PhotoOverlay::AcceptChildren(VisitorDriver* driver) {
  Overlay::AcceptChildren(driver);
  if (has_viewvolume()) {
    driver->Visit(get_viewvolume());
  }
  if (has_imagepyramid()) {
    driver->Visit(get_imagepyramid());
  }
  if (has_point()) {
    driver->Visit(get_point());
  }
}

OverlayXY::OverlayXY() {
}

OverlayXY::~OverlayXY() {
}

void OverlayXY::Accept(Visitor* visitor) {
  visitor->VisitOverlayXY(OverlayXYPtr(this));
}

ScreenXY::ScreenXY() {
}

ScreenXY::~ScreenXY() {
}

void ScreenXY::Accept(Visitor* visitor) {
  visitor->VisitScreenXY(ScreenXYPtr(this));
}

RotationXY::RotationXY() {
}

RotationXY::~RotationXY() {
}

void RotationXY::Accept(Visitor* visitor) {
  visitor->VisitRotationXY(RotationXYPtr(this));
}

Size::Size() {
}

Size::~Size() {
}

void Size::Accept(Visitor* visitor) {
  visitor->VisitSize(SizePtr(this));
}

ScreenOverlay::ScreenOverlay() : rotation_(0.0), has_rotation_(false) {
}

ScreenOverlay::~ScreenOverlay() {
}

void ScreenOverlay::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_overlayXY:
      set_overlayxy(AsOverlayXY(element));
      break;
    case Type_screenXY:
      set_screenxy(AsScreenXY(element));
      break;
    case Type_rotationXY:
      set_rotationxy(AsRotationXY(element));
      break;
    case Type_size:
      set_size(AsSize(element));
      break;
    case Type_rotation:
      has_rotation_ = element->SetDouble(&rotation_);
      break;
    default:
      Overlay::AddElement(element);
      break;
  }
}

void ScreenOverlay::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Overlay::Serialize(serializer);
  if (has_overlayxy()) {
    serializer.SaveElement(get_overlayxy());
  }
  if (has_screenxy()) {
    serializer.SaveElement(get_screenxy());
  }
  if (has_rotationxy()) {
    serializer.SaveElement(get_rotationxy());
  }
  if (has_size()) {
    serializer.SaveElement(get_size());
  }
  if (has_rotation()) {
    serializer.SaveFieldById(Type_rotation, get_rotation());
  }
}

void ScreenOverlay::Accept(Visitor* visitor) {
  visitor->VisitScreenOverlay(ScreenOverlayPtr(this));
}

void ScreenOverlay::AcceptChildren(VisitorDriver* driver) {
  Overlay::AcceptChildren(driver);
  if (has_overlayxy()) {
    driver->Visit(get_overlayxy());
  }
  if (has_screenxy()) {
    driver->Visit(get_screenxy());
  }
  if (has_rotationxy()) {
    driver->Visit(get_rotationxy());
  }
  if (has_size()) {
    driver->Visit(get_size());
  }
}

kmldom::KmlDomType Overlay::Type() const {
  return Type_Overlay;
}

bool Overlay::IsA(kmldom::KmlDomType type) const {
  return type == Type_Overlay || Feature::IsA(type);
}

const kmlbase::Color32& Overlay::get_color() const {
  return color_;
}

bool Overlay::has_color() const {
  return has_color_;
}

void Overlay::set_color(const kmlbase::Color32& color) {
  color_ = color;
  has_color_ = true;
}

void Overlay::clear_color() {
  color_ = kmlbase::Color32(0xffffffff);
  has_color_ = false;
}

int Overlay::get_draworder() const {
  return draworder_;
}

bool Overlay::has_draworder() const {
  return has_draworder_;
}

void Overlay::set_draworder(int draworder) {
  draworder_ = draworder;
  has_draworder_ = true;
}

void Overlay::clear_draworder() {
  draworder_ = 0;
  has_draworder_ = false;
}

const IconPtr& Overlay::get_icon() const {
  return icon_;
}

bool Overlay::has_icon() const {
  return icon_ != nullptr;
}

void Overlay::set_icon(const IconPtr& icon) {
  SetComplexChild(icon, &icon_);
}

void Overlay::clear_icon() {
  set_icon(NULL);
}

kmldom::KmlDomType LatLonBox::Type() const {
  return Type_LatLonBox;
}

bool LatLonBox::IsA(kmldom::KmlDomType type) const {
  return type == Type_LatLonBox || AbstractLatLonBox::IsA(type);
}

double LatLonBox::get_rotation() const {
  return rotation_;
}

bool LatLonBox::has_rotation() const {
  return has_rotation_;
}

void LatLonBox::set_rotation(double rotation) {
  rotation_ = rotation;
  has_rotation_ = true;
}

void LatLonBox::clear_rotation() {
  rotation_ = 0.0;
  has_rotation_ = false;
}

kmldom::KmlDomType GxLatLonQuad::ElementType() {
  return Type_GxLatLonQuad;
}

kmldom::KmlDomType GxLatLonQuad::Type() const {
  return ElementType();
}

bool GxLatLonQuad::IsA(kmldom::KmlDomType type) const {
  return type == ElementType() || Object::IsA(type);
}

const CoordinatesPtr& GxLatLonQuad::get_coordinates() const {
  return coordinates_;
}

bool GxLatLonQuad::has_coordinates() const {
  return coordinates_ != nullptr;
}

void GxLatLonQuad::set_coordinates(const CoordinatesPtr& coordinates) {
  SetComplexChild(coordinates, &coordinates_);
}

void GxLatLonQuad::clear_coordinates() {
  set_coordinates(NULL);
}

kmldom::KmlDomType GroundOverlay::Type() const {
  return Type_GroundOverlay;
}

bool GroundOverlay::IsA(kmldom::KmlDomType type) const {
  return type == Type_GroundOverlay || Overlay::IsA(type);
}

double GroundOverlay::get_altitude() const {
  return altitude_;
}

bool GroundOverlay::has_altitude() const {
  return has_altitude_;
}

void GroundOverlay::set_altitude(double altitude) {
  altitude_ = altitude;
  has_altitude_ = true;
}

void GroundOverlay::clear_altitude() {
  altitude_ = 0.0;
  has_altitude_ = false;
}

int GroundOverlay::get_altitudemode() const {
  return altitudemode_;
}

bool GroundOverlay::has_altitudemode() const {
  return has_altitudemode_;
}

void GroundOverlay::set_altitudemode(int altitudemode) {
  altitudemode_ = altitudemode;
  has_altitudemode_ = true;
}

void GroundOverlay::clear_altitudemode() {
  altitudemode_ = ALTITUDEMODE_CLAMPTOGROUND;
  has_altitudemode_ = false;
}

int GroundOverlay::get_gx_altitudemode() const {
  return gx_altitudemode_;
}

bool GroundOverlay::has_gx_altitudemode() const {
  return has_gx_altitudemode_;
}

void GroundOverlay::set_gx_altitudemode(int gx_altitudemode) {
  gx_altitudemode_ = gx_altitudemode;
  has_gx_altitudemode_ = true;
}

void GroundOverlay::clear_gx_altitudemode() {
  gx_altitudemode_ = GX_ALTITUDEMODE_CLAMPTOSEAFLOOR;
  has_gx_altitudemode_ = false;
}

const LatLonBoxPtr& GroundOverlay::get_latlonbox() const {
  return latlonbox_;
}

bool GroundOverlay::has_latlonbox() const {
  return latlonbox_ != nullptr;
}

void GroundOverlay::set_latlonbox(const LatLonBoxPtr& latlonbox) {
  SetComplexChild(latlonbox, &latlonbox_);
}

void GroundOverlay::clear_latlonbox() {
  set_latlonbox(NULL);
}

const GxLatLonQuadPtr& GroundOverlay::get_gx_latlonquad() const {
  return gx_latlonquad_;
}

bool GroundOverlay::has_gx_latlonquad() const {
  return gx_latlonquad_ != nullptr;
}

void GroundOverlay::set_gx_latlonquad(const GxLatLonQuadPtr& gx_latlonquad) {
  SetComplexChild(gx_latlonquad, &gx_latlonquad_);
}

void GroundOverlay::clear_gx_latlonquad() {
  set_gx_latlonquad(NULL);
}

kmldom::KmlDomType OverlayXY::Type() const {
  return Type_overlayXY;
}

bool OverlayXY::IsA(kmldom::KmlDomType type) const {
  return type == Type_overlayXY || Vec2::IsA(type);
}

kmldom::KmlDomType ScreenXY::Type() const {
  return Type_screenXY;
}

bool ScreenXY::IsA(kmldom::KmlDomType type) const {
  return type == Type_screenXY || Vec2::IsA(type);
}

kmldom::KmlDomType RotationXY::Type() const {
  return Type_rotationXY;
}

bool RotationXY::IsA(kmldom::KmlDomType type) const {
  return type == Type_rotationXY || Vec2::IsA(type);
}

kmldom::KmlDomType Size::Type() const {
  return Type_size;
}

bool Size::IsA(kmldom::KmlDomType type) const {
  return type == Type_size || Vec2::IsA(type);
}

kmldom::KmlDomType ScreenOverlay::Type() const {
  return Type_ScreenOverlay;
}

bool ScreenOverlay::IsA(kmldom::KmlDomType type) const {
  return type == Type_ScreenOverlay || Overlay::IsA(type);
}

const OverlayXYPtr& ScreenOverlay::get_overlayxy() const {
  return overlayxy_;
}

bool ScreenOverlay::has_overlayxy() const {
  return overlayxy_ != nullptr;
}

void ScreenOverlay::set_overlayxy(const OverlayXYPtr& overlayxy) {
  SetComplexChild(overlayxy, &overlayxy_);
}

void ScreenOverlay::clear_overlayxy() {
  set_overlayxy(NULL);
}

const ScreenXYPtr& ScreenOverlay::get_screenxy() const {
  return screenxy_;
}

bool ScreenOverlay::has_screenxy() const {
  return screenxy_ != nullptr;
}

void ScreenOverlay::set_screenxy(const ScreenXYPtr& screenxy) {
  SetComplexChild(screenxy, &screenxy_);
}

void ScreenOverlay::clear_screenxy() {
  set_screenxy(NULL);
}

const RotationXYPtr& ScreenOverlay::get_rotationxy() const {
  return rotationxy_;
}

bool ScreenOverlay::has_rotationxy() const {
  return rotationxy_ != nullptr;
}

void ScreenOverlay::set_rotationxy(const RotationXYPtr& rotationxy) {
  SetComplexChild(rotationxy, &rotationxy_);
}

void ScreenOverlay::clear_rotationxy() {
  set_rotationxy(NULL);
}

const SizePtr& ScreenOverlay::get_size() const {
  return size_;
}

bool ScreenOverlay::has_size() const {
  return size_ != nullptr;
}

void ScreenOverlay::set_size(const SizePtr& size) {
  SetComplexChild(size, &size_);
}

void ScreenOverlay::clear_size() {
  set_size(NULL);
}

double ScreenOverlay::get_rotation() const {
  return rotation_;
}

bool ScreenOverlay::has_rotation() const {
  return has_rotation_;
}

void ScreenOverlay::set_rotation(double rotation) {
  rotation_ = rotation;
  has_rotation_ = true;
}

void ScreenOverlay::clear_rotation() {
  rotation_ = 0.0;
  has_rotation_ = false;
}

kmldom::KmlDomType ViewVolume::Type() const {
  return Type_ViewVolume;
}

bool ViewVolume::IsA(kmldom::KmlDomType type) const {
  return type == Type_ViewVolume || Object::IsA(type);
}

double ViewVolume::get_leftfov() const {
  return leftfov_;
}

bool ViewVolume::has_leftfov() const {
  return has_leftfov_;
}

void ViewVolume::set_leftfov(double leftfov) {
  leftfov_ = leftfov;
  has_leftfov_ = true;
}

void ViewVolume::clear_leftfov() {
  leftfov_ = 0.0;
  has_leftfov_ = false;
}

double ViewVolume::get_rightfov() const {
  return rightfov_;
}

bool ViewVolume::has_rightfov() const {
  return has_rightfov_;
}

void ViewVolume::set_rightfov(double rightfov) {
  rightfov_ = rightfov;
  has_rightfov_ = true;
}

void ViewVolume::clear_rightfov() {
  rightfov_ = 0.0;
  has_rightfov_ = false;
}

double ViewVolume::get_bottomfov() const {
  return bottomfov_;
}

bool ViewVolume::has_bottomfov() const {
  return has_bottomfov_;
}

void ViewVolume::set_bottomfov(double altitude) {
  bottomfov_ = altitude;
  has_bottomfov_ = true;
}

void ViewVolume::clear_bottomfov() {
  bottomfov_ = 0.0;
  has_bottomfov_ = false;
}

double ViewVolume::get_topfov() const {
  return topfov_;
}

bool ViewVolume::has_topfov() const {
  return has_topfov_;
}

void ViewVolume::set_topfov(double topfov) {
  topfov_ = topfov;
  has_topfov_ = true;
}

void ViewVolume::clear_topfov() {
  topfov_ = 0.0;
  has_topfov_ = false;
}

double ViewVolume::get_near() const {
  return near_;
}

bool ViewVolume::has_near() const {
  return has_near_;
}

void ViewVolume::set_near(double val) {
  near_ = val;
  has_near_ = true;
}

void ViewVolume::clear_near() {
  near_ = 0.0;
  has_near_ = false;
}

kmldom::KmlDomType ImagePyramid::Type() const {
  return Type_ImagePyramid;
}

bool ImagePyramid::IsA(kmldom::KmlDomType type) const {
  return type == Type_ImagePyramid || Object::IsA(type);
}

int ImagePyramid::get_tilesize() const {
  return tilesize_;
}

bool ImagePyramid::has_tilesize() const {
  return has_tilesize_;
}

void ImagePyramid::set_tilesize(int tilesize) {
  tilesize_ = tilesize;
  has_tilesize_ = true;
}

void ImagePyramid::clear_tilesize() {
  tilesize_ = 256;
  has_tilesize_ = false;
}

int ImagePyramid::get_maxwidth() const {
  return maxwidth_;
}

bool ImagePyramid::has_maxwidth() const {
  return has_maxwidth_;
}

void ImagePyramid::set_maxwidth(int maxwidth) {
  maxwidth_ = maxwidth;
  has_maxwidth_ = true;
}

void ImagePyramid::clear_maxwidth() {
  maxwidth_ = 0;
  has_maxwidth_ = false;
}

int ImagePyramid::get_maxheight() const {
  return maxheight_;
}

bool ImagePyramid::has_maxheight() const {
  return has_maxheight_;
}

void ImagePyramid::set_maxheight(int altitude) {
  maxheight_ = altitude;
  has_maxheight_ = true;
}

void ImagePyramid::clear_maxheight() {
  maxheight_ = 0;
  has_maxheight_ = false;
}

int ImagePyramid::get_gridorigin() const {
  return gridorigin_;
}

bool ImagePyramid::has_gridorigin() const {
  return has_gridorigin_;
}

void ImagePyramid::set_gridorigin(int gridorigin) {
  gridorigin_ = gridorigin;
  has_gridorigin_ = true;
}

void ImagePyramid::clear_gridorigin() {
  gridorigin_ = GRIDORIGIN_LOWERLEFT;
  has_gridorigin_ = false;
}

kmldom::KmlDomType PhotoOverlay::Type() const {
  return Type_PhotoOverlay;
}

bool PhotoOverlay::IsA(kmldom::KmlDomType type) const {
  return type == Type_PhotoOverlay || Overlay::IsA(type);
}

double PhotoOverlay::get_rotation() const {
  return rotation_;
}

bool PhotoOverlay::has_rotation() const {
  return has_rotation_;
}

void PhotoOverlay::set_rotation(double rotation) {
  rotation_ = rotation;
  has_rotation_ = true;
}

void PhotoOverlay::clear_rotation() {
  rotation_ = 0.0;
  has_rotation_ = false;
}

const ViewVolumePtr& PhotoOverlay::get_viewvolume() const {
  return viewvolume_;
}

bool PhotoOverlay::has_viewvolume() const {
  return viewvolume_ != nullptr;
}

void PhotoOverlay::set_viewvolume(const ViewVolumePtr& viewvolume) {
  SetComplexChild(viewvolume, &viewvolume_);
}

void PhotoOverlay::clear_viewvolume() {
  set_viewvolume(NULL);
}

const ImagePyramidPtr& PhotoOverlay::get_imagepyramid() const {
  return imagepyramid_;
}

bool PhotoOverlay::has_imagepyramid() const {
  return imagepyramid_ != nullptr;
}

void PhotoOverlay::set_imagepyramid(const ImagePyramidPtr& imagepyramid) {
  SetComplexChild(imagepyramid, &imagepyramid_);
}

void PhotoOverlay::clear_imagepyramid() {
  set_imagepyramid(NULL);
}

const PointPtr& PhotoOverlay::get_point() const {
  return point_;
}

bool PhotoOverlay::has_point() const {
  return point_ != nullptr;
}

void PhotoOverlay::set_point(const PointPtr& point) {
  SetComplexChild(point, &point_);
}

void PhotoOverlay::clear_point() {
  set_point(NULL);
}

int PhotoOverlay::get_shape() const {
  return shape_;
}

bool PhotoOverlay::has_shape() const {
  return has_shape_;
}

void PhotoOverlay::set_shape(int shape) {
  shape_ = shape;
  has_shape_ = true;
}

void PhotoOverlay::clear_shape() {
  shape_ = SHAPE_RECTANGLE;
  has_shape_ = false;
}
}  // end namespace kmldom
