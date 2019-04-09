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

// This file contains the implementation of the abstract element Geometry
// and the concrete elements coordinates, Point, LineString, LinearRing,
// outerBoundaryIs, innerBoundaryIs and Polygon.

#include "kml/dom/geometry.h"
#include <ctype.h>
#include <stdlib.h>
#include "kml/base/attributes.h"
#include "kml/base/missing/strtod.h"
#include "kml/base/xml_namespaces.h"
#include "kml/dom/element.h"
#include "kml/dom/extendeddata.h"
#include "kml/dom/kml22.h"
#include "kml/dom/kml_cast.h"
#include "kml/dom/kml_ptr.h"
#include "kml/dom/link.h"  // Remove when model.h is repaired.
#include "kml/dom/object.h"
#include "kml/dom/serializer.h"
#include "kml/dom/visitor.h"

using kmlbase::Attributes;
using kmlbase::Vec3;

namespace kmldom {

Coordinates::Coordinates() {
  set_xmlns(kmlbase::XMLNS_KML22);
}

Coordinates::~Coordinates() {
}

// This parses off a Vec3 from the given string and returns a pointer
// to the end of chars consumed.  The purpose is for use in the inner loop
// of the overall parse of a <coordinates> string.  This handles both 2d and
// 3d points and handles any form of whitespace around the commas between
// coordinates.  The general formula is commas between 2 or 3 coordinates
// with any form of whitespace permitted around the commas and whitespace
// between tuples including before the first and after the last.
// Here are some example 3d cstr's which all set the Vec3(1.1, 2.2, 3.3)
// Comma separated coords, space only between tuples:
// "1.1,2.2,3.3 4.4,5.5,6.6"
// Comma separated coords, leading whitespace:
// " 1.1,2.2,3.3 4.4,5.5,6.6"
// "\n 1.1,2.2,3.3 4.4,5.5,6.6"
// Whitespace between coordinates:
// "\t1.1 , 2.2 , 3.3  4.4 , 5.5,6.6"
// Here are some 2d cstr's which all parse to Vec3(1.1, 2.2, 0.0).
// Note that lack of altitude is represented as altitude = 0.0.
// (Actual use of the altitude typeically depends on altitudeMode).
// No spaces. Comma separated as might be common for Point:
// "1.1,2.2"
// A couple of tuples with comma-separated coordinates and single space
// separatting the tuples as might be used in LineString:
// "1.1,2.2 4.4,5.5"
// Leading newlines and tabs as might created by a pretty printer:
// "\n\t1.1, 2.2\t\t4.4, 5.5\n"
// Bad separators are simply discarded and we move to the next comma. A string
// like this: "1.1*2.2,3,3" will become "1.1,3.3,0.0". This precisely matches
// the precent for parsing of bad coordinate strings set by Google Earth.
bool Coordinates::ParseVec3(const char* cstr, char** nextp, Vec3* vec) {
  if (!cstr || !vec) {  // Not much to do w/o input or output.
    return false;
  }
  bool done = false;
  char* endp = const_cast<char*>(cstr);

  // Ignore any commas at the start of our scan. This will cause this:
  // <coordinates>1,2,3,4,5</coordinates> to be treated as:
  // <coordinates>1,2,3 4,5</coordinates>, which is how Google Earth treats
  // the misuse of commas as separators.
  if (*endp == ',') {
    ++endp;
  }

  // Longitude first.  strtod() eats leading whitespace.
  vec->set(0, kml_strtod(endp, &endp));
  if (endp) {
    // Latitude next.
    while (isspace(*endp) || *endp != ',') {
      // We check here to make sure the parse is sane. If we've been passed
      // an invalid coordinate string, this loop will reach the null
      // terminator. If we see it, we set the nextp pointer to the end and
      // return which will let Coordinates::Parse know that it's finished.
      if (*endp == '\0') {
        *nextp = endp;
        return done;
      }
      // Eat whitespace between double and comma.
      ++endp;
    }
    vec->set(1, kml_strtod(endp + 1, &endp));
    done = true;  // Need at least lon,lat to be valid.

    // If no altitude set to 0
    while (isspace(*endp)) {  // Eat whitespace between double and comma.
      ++endp;
    }
    if (*endp == ',') {
      // Note that this sets altitude only if an altitude is supplied.
      vec->set(2, kml_strtod(endp + 1, &endp));
    }
  }
  if (nextp) {
    while (isspace(*endp)) {  // Eat the remaining whitespace before return.
      ++endp;
    }
    *nextp = endp;
  }
  return done;
}

// The char_data is everything between <coordinates> elements including
// leading and trailing whitespace.
void Coordinates::Parse(const string& char_data) {
  const char* cstr = char_data.c_str();
  const char* endp = cstr + char_data.size();
  char* next = const_cast<char*>(cstr);
  while (next != endp) {
    Vec3 vec;
    if (ParseVec3(next, &next, &vec)) {
      coordinates_array_.push_back(vec);
    }
  }
}

// Coordinates essentially parses itself.
void Coordinates::AddElement(const ElementPtr& element) {
  Parse(get_char_data());
}

void Coordinates::Serialize(Serializer& serializer) const {
  Attributes dummy;
  serializer.BeginById(Type(), dummy);
  serializer.BeginElementArray(Type(), coordinates_array_.size());
  for (size_t i = 0; i < coordinates_array_.size(); ++i) {
    serializer.SaveVec3(coordinates_array_[i]);
  }
  serializer.EndElementArray(Type_coordinates);
  serializer.End();
}

void Coordinates::Accept(Visitor* visitor) {
  visitor->VisitCoordinates(CoordinatesPtr(this));
}

Geometry::Geometry() {
}

Geometry::~Geometry() {
}

AltitudeGeometryCommon::AltitudeGeometryCommon()
    : altitudemode_(ALTITUDEMODE_CLAMPTOGROUND),
      has_altitudemode_(false),
      gx_altitudemode_(GX_ALTITUDEMODE_CLAMPTOSEAFLOOR),
      has_gx_altitudemode_(false) {
}

AltitudeGeometryCommon::~AltitudeGeometryCommon() {
}

void AltitudeGeometryCommon::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_altitudeMode:
      has_altitudemode_ = element->SetEnum(&altitudemode_);
      return;
    case Type_GxAltitudeMode:
      has_gx_altitudemode_ = element->SetEnum(&gx_altitudemode_);
      return;
    default:
      Geometry::AddElement(element);
  }
}

ExtrudeGeometryCommon::ExtrudeGeometryCommon()
    : extrude_(false), has_extrude_(false) {
}

ExtrudeGeometryCommon::~ExtrudeGeometryCommon() {
}

void ExtrudeGeometryCommon::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->Type() == Type_extrude) {
    has_extrude_ = element->SetBool(&extrude_);
    return;
  }
  AltitudeGeometryCommon::AddElement(element);
}

CoordinatesGeometryCommon::CoordinatesGeometryCommon() {
}

CoordinatesGeometryCommon::~CoordinatesGeometryCommon() {
}

void CoordinatesGeometryCommon::AddElement(const ElementPtr& element) {
  if (CoordinatesPtr coordinates = AsCoordinates(element)) {
    set_coordinates(coordinates);
  } else {
    ExtrudeGeometryCommon::AddElement(element);
  }
}

void CoordinatesGeometryCommon::AcceptChildren(VisitorDriver* driver) {
  ExtrudeGeometryCommon::AcceptChildren(driver);
  if (has_coordinates()) {
    driver->Visit(get_coordinates());
  }
}

Point::Point() {
}

Point::~Point() {
}

void Point::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Geometry::Serialize(serializer);
  if (has_extrude()) {
    serializer.SaveFieldById(Type_extrude, get_extrude());
  }
  if (has_altitudemode()) {
    serializer.SaveEnum(Type_altitudeMode, get_altitudemode());
  }
  if (has_gx_altitudemode()) {
    serializer.SaveEnum(Type_GxAltitudeMode, get_gx_altitudemode());
  }
  if (has_coordinates()) {
    serializer.SaveElement(get_coordinates());
  }
}

void Point::Accept(Visitor* visitor) {
  visitor->VisitPoint(PointPtr(this));
}

LineCommon::LineCommon() : tessellate_(false), has_tessellate_(false) {
}

LineCommon::~LineCommon() {
}

void LineCommon::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->Type() == Type_tessellate) {
    has_tessellate_ = element->SetBool(&tessellate_);
    return;
  }
  CoordinatesGeometryCommon::AddElement(element);
}

void LineCommon::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Geometry::Serialize(serializer);
  if (has_extrude()) {
    serializer.SaveFieldById(Type_extrude, get_extrude());
  }
  if (has_tessellate()) {
    serializer.SaveFieldById(Type_tessellate, get_tessellate());
  }
  if (has_altitudemode()) {
    serializer.SaveEnum(Type_altitudeMode, get_altitudemode());
  }
  if (has_gx_altitudemode()) {
    serializer.SaveEnum(Type_GxAltitudeMode, get_gx_altitudemode());
  }
  if (has_coordinates()) {
    serializer.SaveElement(get_coordinates());
  }
}

LineString::LineString() {
}

LineString::~LineString() {
}

void LineString::Accept(Visitor* visitor) {
  visitor->VisitLineString(LineStringPtr(this));
}

LinearRing::LinearRing() {
}

LinearRing::~LinearRing() {
}

void LinearRing::Accept(Visitor* visitor) {
  visitor->VisitLinearRing(LinearRingPtr(this));
}

BoundaryCommon::BoundaryCommon() {
}

BoundaryCommon::~BoundaryCommon() {
}

void BoundaryCommon::AddElement(const ElementPtr& element) {
  if (LinearRingPtr linearring = AsLinearRing(element)) {
    set_linearring(linearring);
  } else {
    Element::AddElement(element);
  }
}

void BoundaryCommon::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  if (has_linearring()) {
    serializer.SaveElement(get_linearring());
  }
}

void BoundaryCommon::AcceptChildren(VisitorDriver* driver) {
  Element::AcceptChildren(driver);
  if (has_linearring()) {
    driver->Visit(get_linearring());
  }
}

OuterBoundaryIs::OuterBoundaryIs() {
}

OuterBoundaryIs::~OuterBoundaryIs() {
}

void OuterBoundaryIs::Accept(Visitor* visitor) {
  visitor->VisitOuterBoundaryIs(OuterBoundaryIsPtr(this));
}

InnerBoundaryIs::InnerBoundaryIs() {
}

InnerBoundaryIs::~InnerBoundaryIs() {
}

void InnerBoundaryIs::Accept(Visitor* visitor) {
  visitor->VisitInnerBoundaryIs(InnerBoundaryIsPtr(this));
}

Polygon::Polygon() : tessellate_(false), has_tessellate_(false) {
}

Polygon::~Polygon() {
}

void Polygon::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_tessellate:
      has_tessellate_ = element->SetBool(&tessellate_);
      break;
    case Type_outerBoundaryIs:
      set_outerboundaryis(AsOuterBoundaryIs(element));
      break;
    case Type_innerBoundaryIs:
      add_innerboundaryis(AsInnerBoundaryIs(element));
      break;
    default:
      ExtrudeGeometryCommon::AddElement(element);
  }
}

void Polygon::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Geometry::Serialize(serializer);
  if (has_extrude()) {
    serializer.SaveFieldById(Type_extrude, get_extrude());
  }
  if (has_tessellate()) {
    serializer.SaveFieldById(Type_tessellate, get_tessellate());
  }
  if (has_altitudemode()) {
    serializer.SaveEnum(Type_altitudeMode, get_altitudemode());
  }
  if (has_gx_altitudemode()) {
    serializer.SaveEnum(Type_GxAltitudeMode, get_gx_altitudemode());
  }
  if (has_outerboundaryis()) {
    serializer.SaveElement(get_outerboundaryis());
  }
  serializer.SaveElementArray(innerboundaryis_array_);
}

void Polygon::Accept(Visitor* visitor) {
  visitor->VisitPolygon(PolygonPtr(this));
}

void Polygon::AcceptChildren(VisitorDriver* driver) {
  ExtrudeGeometryCommon::AcceptChildren(driver);
  if (has_outerboundaryis()) {
    driver->Visit(get_outerboundaryis());
  }
  Element::AcceptRepeated<InnerBoundaryIsPtr>(&innerboundaryis_array_, driver);
}

MultiGeometry::MultiGeometry() {
}

MultiGeometry::~MultiGeometry() {
}

void MultiGeometry::add_geometry(const GeometryPtr& geometry) {
  AddComplexChild(geometry, &geometry_array_);
}

void MultiGeometry::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->IsA(Type_Geometry)) {
    add_geometry(AsGeometry(element));
    return;
  }
  Geometry::AddElement(element);
}

void MultiGeometry::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Geometry::Serialize(serializer);
  serializer.SaveElementGroupArray(geometry_array_, Type_Geometry);
}

void MultiGeometry::Accept(Visitor* visitor) {
  visitor->VisitMultiGeometry(MultiGeometryPtr(this));
}

void MultiGeometry::AcceptChildren(VisitorDriver* driver) {
  Geometry::AcceptChildren(driver);
  Element::AcceptRepeated<GeometryPtr>(&geometry_array_, driver);
}

GxTrack::GxTrack() {
  set_xmlns(kmlbase::XMLNS_GX22);
}

GxTrack::~GxTrack() {
}

void GxTrack::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_when:
      add_when(element->get_char_data());
      break;
    case Type_GxAngles:
      Parse(element->get_char_data(), &gx_angles_array_);
      break;
    case Type_GxCoord:
      Parse(element->get_char_data(), &gx_coord_array_);
      break;
    case Type_Model:
      set_model(AsModel(element));
      break;
    case Type_ExtendedData:
      set_extendeddata(AsExtendedData(element));
      break;
    default:
      AltitudeGeometryCommon::AddElement(element);
  }
}

void GxTrack::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Geometry::Serialize(serializer);
  if (has_altitudemode()) {
    serializer.SaveEnum(Type_altitudeMode, get_altitudemode());
  }
  if (has_gx_altitudemode()) {
    serializer.SaveEnum(Type_GxAltitudeMode, get_gx_altitudemode());
  }
  for (size_t i = 0; i < when_array_.size(); i++) {
    serializer.SaveStringFieldById(Type_when, when_array_[i]);
  }
  const Attributes dummy;
  for (size_t i = 0; i < gx_coord_array_.size(); i++) {
    serializer.SaveSimpleVec3(Type_GxCoord, gx_coord_array_.at(i), " ");
  }
  for (size_t i = 0; i < gx_angles_array_.size(); i++) {
    serializer.SaveSimpleVec3(Type_GxAngles, gx_angles_array_.at(i), " ");
  }
  if (has_model()) {
    serializer.SaveElement(get_model());
  }
  if (has_extendeddata()) {
    serializer.SaveElement(get_extendeddata());
  }
}

void GxTrack::Accept(Visitor* visitor) {
  visitor->VisitGxTrack(GxTrackPtr(this));
}

void GxTrack::AcceptChildren(VisitorDriver* driver) {
  AltitudeGeometryCommon::AcceptChildren(driver);
  if (has_model()) {
    driver->Visit(get_model());
  }
  if (has_extendeddata()) {
    driver->Visit(get_extendeddata());
  }
}

void GxTrack::Parse(const string& char_data, std::vector<Vec3>* out) {
  if (!out) {
    return;
  }
  // TODO: this is a little heavy. Optimization along the lines of
  // Coordinates::Parse may be required.
  std::vector<string> s;
  kmlbase::SplitStringUsing(char_data, " ", &s);
  kmlbase::Vec3 vec;
  for (size_t i = 0; i < s.size(); i++) {
    vec.set(i, kml_strtod(s[i].c_str(), NULL));
    if (i > 2) break;
  }
  out->push_back(vec);
}

GxMultiTrack::GxMultiTrack()
    : gx_interpolate_(false), has_gx_interpolate_(false) {
  set_xmlns(kmlbase::XMLNS_GX22);
}

GxMultiTrack::~GxMultiTrack() {
}

void GxMultiTrack::add_gx_track(const GxTrackPtr& gx_track) {
  AddComplexChild(gx_track, &gx_track_array_);
}

void GxMultiTrack::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->Type() == Type_GxInterpolate) {
    has_gx_interpolate_ = element->SetBool(&gx_interpolate_);
    return;
  }
  if (element->IsA(Type_GxTrack)) {
    add_gx_track(AsGxTrack(element));
    return;
  }
  Geometry::AddElement(element);
}

void GxMultiTrack::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  Geometry::Serialize(serializer);
  if (has_gx_interpolate_) {
    serializer.SaveFieldById(Type_GxInterpolate, gx_interpolate_);
  }
  serializer.SaveElementGroupArray(gx_track_array_, Type_GxTrack);
}

void GxMultiTrack::Accept(Visitor* visitor) {
  visitor->VisitGxMultiTrack(GxMultiTrackPtr(this));
}

void GxMultiTrack::AcceptChildren(VisitorDriver* driver) {
  Geometry::AcceptChildren(driver);
  Element::AcceptRepeated<GxTrackPtr>(&gx_track_array_, driver);
}

void Coordinates::add_latlngalt(double latitude, double longitude,
                                double altitude) {
  coordinates_array_.push_back(kmlbase::Vec3(longitude, latitude, altitude));
}

void Coordinates::add_latlng(double latitude, double longitude) {
  coordinates_array_.push_back(kmlbase::Vec3(longitude, latitude));
}

void Coordinates::insert_latlng(std::size_t index, double latitude,
                                double longitude) {
  coordinates_array_.insert(coordinates_array_.begin() + index,
                            kmlbase::Vec3(longitude, latitude));
}

void Coordinates::remove_at(std::size_t index) {
  coordinates_array_.erase(coordinates_array_.begin() + index);
}

void Coordinates::add_vec3(const kmlbase::Vec3& vec3) {
  coordinates_array_.push_back(vec3);
}

size_t Coordinates::get_coordinates_array_size() const {
  return coordinates_array_.size();
}

const kmlbase::Vec3& Coordinates::get_coordinates_array_at(size_t index) const {
  return coordinates_array_[index];
}

kmlbase::Vec3& Coordinates::get_coordinates_array_at(size_t index) {
  return coordinates_array_[index];
}

void Coordinates::erase_coordinates_array_at(size_t index) {
  coordinates_array_.erase(coordinates_array_.begin() + index);
}

void Coordinates::set_coordinates_array_at(const kmlbase::Vec3& vec,
                                           size_t index) {
  coordinates_array_[index] = vec;
}

void Coordinates::Clear() {
  coordinates_array_.clear();
}

kmldom::KmlDomType Geometry::Type() const {
  return Type_Geometry;
}

bool Geometry::IsA(kmldom::KmlDomType type) const {
  return type == Type_Geometry || Object::IsA(type);
}

int AltitudeGeometryCommon::get_altitudemode() const {
  return altitudemode_;
}

bool AltitudeGeometryCommon::has_altitudemode() const {
  return has_altitudemode_;
}

void AltitudeGeometryCommon::set_altitudemode(int value) {
  altitudemode_ = value;
  has_altitudemode_ = true;
}

void AltitudeGeometryCommon::clear_altitudemode() {
  altitudemode_ = ALTITUDEMODE_CLAMPTOGROUND;
  has_altitudemode_ = false;
}

int AltitudeGeometryCommon::get_gx_altitudemode() const {
  return gx_altitudemode_;
}

bool AltitudeGeometryCommon::has_gx_altitudemode() const {
  return has_gx_altitudemode_;
}

void AltitudeGeometryCommon::set_gx_altitudemode(int value) {
  gx_altitudemode_ = value;
  has_gx_altitudemode_ = true;
}

void AltitudeGeometryCommon::clear_gx_altitudemode() {
  gx_altitudemode_ = GX_ALTITUDEMODE_CLAMPTOSEAFLOOR;
  has_gx_altitudemode_ = false;
}

bool ExtrudeGeometryCommon::get_extrude() const {
  return extrude_;
}

bool ExtrudeGeometryCommon::has_extrude() const {
  return has_extrude_;
}

void ExtrudeGeometryCommon::set_extrude(bool value) {
  extrude_ = value;
  has_extrude_ = true;
}

void ExtrudeGeometryCommon::clear_extrude() {
  extrude_ = false;
  has_extrude_ = false;
}

const CoordinatesPtr& CoordinatesGeometryCommon::get_coordinates() const {
  return coordinates_;
}

bool CoordinatesGeometryCommon::has_coordinates() const {
  return coordinates_ != nullptr;
}

void CoordinatesGeometryCommon::set_coordinates(
    const CoordinatesPtr& coordinates) {
  SetComplexChild(coordinates, &coordinates_);
}

void CoordinatesGeometryCommon::clear_coordinates() {
  set_coordinates(NULL);
}

kmldom::KmlDomType Point::Type() const {
  return Type_Point;
}

bool Point::IsA(kmldom::KmlDomType type) const {
  return type == Type_Point || Geometry::IsA(type);
}

bool LineCommon::get_tessellate() const {
  return tessellate_;
}

bool LineCommon::has_tessellate() const {
  return has_tessellate_;
}

void LineCommon::set_tessellate(bool value) {
  tessellate_ = value;
  has_tessellate_ = true;
}

void LineCommon::clear_tessellate() {
  tessellate_ = false;
  has_tessellate_ = false;
}

kmldom::KmlDomType LineString::Type() const {
  return Type_LineString;
}

bool LineString::IsA(kmldom::KmlDomType type) const {
  return type == Type_LineString || Geometry::IsA(type);
}

kmldom::KmlDomType LinearRing::Type() const {
  return Type_LinearRing;
}

bool LinearRing::IsA(kmldom::KmlDomType type) const {
  return type == Type_LinearRing || Geometry::IsA(type);
}

const LinearRingPtr& BoundaryCommon::get_linearring() const {
  return linearring_;
}

bool BoundaryCommon::has_linearring() const {
  return linearring_ != nullptr;
}

void BoundaryCommon::set_linearring(const LinearRingPtr& linearring) {
  SetComplexChild(linearring, &linearring_);
}

void BoundaryCommon::clear_linearring() {
  set_linearring(NULL);
}

kmldom::KmlDomType OuterBoundaryIs::Type() const {
  return Type_outerBoundaryIs;
}

bool OuterBoundaryIs::IsA(kmldom::KmlDomType type) const {
  return type == Type_outerBoundaryIs;
}

kmldom::KmlDomType InnerBoundaryIs::Type() const {
  return Type_innerBoundaryIs;
}

bool InnerBoundaryIs::IsA(kmldom::KmlDomType type) const {
  return type == Type_innerBoundaryIs;
}

kmldom::KmlDomType Polygon::Type() const {
  return Type_Polygon;
}

bool Polygon::IsA(kmldom::KmlDomType type) const {
  return type == Type_Polygon || Geometry::IsA(type);
}

bool Polygon::get_tessellate() const {
  return tessellate_;
}

bool Polygon::has_tessellate() const {
  return has_tessellate_;
}

void Polygon::set_tessellate(bool value) {
  tessellate_ = value;
  has_tessellate_ = true;
}

void Polygon::clear_tessellate() {
  tessellate_ = false;
  has_tessellate_ = false;
}

const OuterBoundaryIsPtr& Polygon::get_outerboundaryis() const {
  return outerboundaryis_;
}

bool Polygon::has_outerboundaryis() const {
  return outerboundaryis_ != nullptr;
}

void Polygon::set_outerboundaryis(const OuterBoundaryIsPtr& outerboundaryis) {
  SetComplexChild(outerboundaryis, &outerboundaryis_);
}

void Polygon::clear_outerboundaryis() {
  set_outerboundaryis(NULL);
}

void Polygon::add_innerboundaryis(const InnerBoundaryIsPtr& innerboundaryis) {
  AddComplexChild(innerboundaryis, &innerboundaryis_array_);
}

size_t Polygon::get_innerboundaryis_array_size() const {
  return innerboundaryis_array_.size();
}

const InnerBoundaryIsPtr& Polygon::get_innerboundaryis_array_at(size_t index) {
  return innerboundaryis_array_[index];
}

kmldom::KmlDomType MultiGeometry::Type() const {
  return Type_MultiGeometry;
}

bool MultiGeometry::IsA(kmldom::KmlDomType type) const {
  return type == Type_MultiGeometry || Geometry::IsA(type);
}

size_t MultiGeometry::get_geometry_array_size() const {
  return geometry_array_.size();
}

const GeometryPtr& MultiGeometry::get_geometry_array_at(size_t index) const {
  return geometry_array_[index];
}

kmldom::KmlDomType GxTrack::ElementType() {
  return Type_GxTrack;
}

kmldom::KmlDomType GxTrack::Type() const {
  return ElementType();
}

bool GxTrack::IsA(kmldom::KmlDomType type) const {
  return type == ElementType() || Geometry::IsA(type);
}

size_t GxTrack::get_when_array_size() {
  return when_array_.size();
}

void GxTrack::add_when(const string& when) {
  when_array_.push_back(when);
}

const string& GxTrack::get_when_array_at(size_t index) const {
  return when_array_[index];
}

size_t GxTrack::get_gx_coord_array_size() {
  return gx_coord_array_.size();
}

void GxTrack::add_gx_coord(const kmlbase::Vec3& gx_coord) {
  gx_coord_array_.push_back(gx_coord);
}

const kmlbase::Vec3& GxTrack::get_gx_coord_array_at(size_t index) const {
  return gx_coord_array_[index];
}

size_t GxTrack::get_gx_angles_array_size() {
  return gx_angles_array_.size();
}

void GxTrack::add_gx_angles(const kmlbase::Vec3& gx_angles) {
  gx_angles_array_.push_back(gx_angles);
}

const kmlbase::Vec3& GxTrack::get_gx_angles_array_at(size_t index) const {
  return gx_angles_array_[index];
}

const ModelPtr& GxTrack::get_model() const {
  return model_;
}

void GxTrack::set_model(const ModelPtr& model) {
  SetComplexChild(model, &model_);
}

bool GxTrack::has_model() const {
  return model_ != nullptr;
}

void GxTrack::clear_model() {
  set_model(NULL);
}

const ExtendedDataPtr& GxTrack::get_extendeddata() const {
  return extendeddata_;
}

bool GxTrack::has_extendeddata() const {
  return extendeddata_ != nullptr;
}

void GxTrack::set_extendeddata(const ExtendedDataPtr& extendeddata) {
  SetComplexChild(extendeddata, &extendeddata_);
}

void GxTrack::clear_extendeddata() {
  set_extendeddata(NULL);
}

kmldom::KmlDomType GxMultiTrack::ElementType() {
  return Type_GxMultiTrack;
}

kmldom::KmlDomType GxMultiTrack::Type() const {
  return ElementType();
}

bool GxMultiTrack::IsA(kmldom::KmlDomType type) const {
  return type == ElementType() || Geometry::IsA(type);
}

bool GxMultiTrack::get_gx_interpolate() const {
  return gx_interpolate_;
}

bool GxMultiTrack::has_gx_interpolate() const {
  return has_gx_interpolate_;
}

void GxMultiTrack::set_gx_interpolate(bool value) {
  gx_interpolate_ = value;
  has_gx_interpolate_ = true;
}

void GxMultiTrack::clear_gx_interpolate() {
  gx_interpolate_ = false;  // Default <gx:interpolate> is false.
  has_gx_interpolate_ = false;
}

size_t GxMultiTrack::get_gx_track_array_size() const {
  return gx_track_array_.size();
}

const GxTrackPtr& GxMultiTrack::get_gx_track_array_at(size_t index) const {
  return gx_track_array_[index];
}

kmldom::KmlDomType Location::Type() const {
  return Type_Location;
}

bool Location::IsA(kmldom::KmlDomType type) const {
  return type == Type_Location || Object::IsA(type);
}

double Location::get_longitude() const {
  return longitude_;
}

bool Location::has_longitude() const {
  return has_longitude_;
}

void Location::set_longitude(double longitude) {
  longitude_ = longitude;
  has_longitude_ = true;
}

void Location::clear_longitude() {
  longitude_ = 0.0;
  has_longitude_ = false;
}

double Location::get_latitude() const {
  return latitude_;
}

bool Location::has_latitude() const {
  return has_latitude_;
}

void Location::set_latitude(double latitude) {
  latitude_ = latitude;
  has_latitude_ = true;
}

void Location::clear_latitude() {
  latitude_ = 0.0;
  has_latitude_ = false;
}

double Location::get_altitude() const {
  return altitude_;
}

bool Location::has_altitude() const {
  return has_altitude_;
}

void Location::set_altitude(double altitude) {
  altitude_ = altitude;
  has_altitude_ = true;
}

void Location::clear_altitude() {
  altitude_ = 0.0;
  has_altitude_ = false;
}

kmldom::KmlDomType Orientation::Type() const {
  return Type_Orientation;
}

bool Orientation::IsA(kmldom::KmlDomType type) const {
  return type == Type_Orientation || Object::IsA(type);
}

double Orientation::get_heading() const {
  return heading_;
}

bool Orientation::has_heading() const {
  return has_heading_;
}

void Orientation::set_heading(double heading) {
  heading_ = heading;
  has_heading_ = true;
}

void Orientation::clear_heading() {
  heading_ = 0.0;
  has_heading_ = false;
}

double Orientation::get_tilt() const {
  return tilt_;
}

bool Orientation::has_tilt() const {
  return has_tilt_;
}

void Orientation::set_tilt(double tilt) {
  tilt_ = tilt;
  has_tilt_ = true;
}

void Orientation::clear_tilt() {
  tilt_ = 0.0;
  has_tilt_ = false;
}

double Orientation::get_roll() const {
  return roll_;
}

void Orientation::set_roll(double roll) {
  roll_ = roll;
  has_roll_ = true;
}

bool Orientation::has_roll() const {
  return has_roll_;
}

void Orientation::clear_roll() {
  roll_ = 0.0;
  has_roll_ = false;
}

kmldom::KmlDomType Scale::Type() const {
  return Type_Scale;
}

bool Scale::IsA(kmldom::KmlDomType type) const {
  return type == Type_Scale || Object::IsA(type);
}

double Scale::get_x() const {
  return x_;
}

bool Scale::has_x() const {
  return has_x_;
}

void Scale::set_x(double x) {
  x_ = x;
  has_x_ = true;
}

void Scale::clear_x() {
  x_ = 1.0;
  has_x_ = false;
}

double Scale::get_y() const {
  return y_;
}

bool Scale::has_y() const {
  return has_y_;
}

void Scale::set_y(double y) {
  y_ = y;
  has_y_ = true;
}

void Scale::clear_y() {
  y_ = 1.0;
  has_y_ = false;
}

double Scale::get_z() const {
  return z_;
}

bool Scale::has_z() const {
  return has_z_;
}

void Scale::set_z(double z) {
  z_ = z;
  has_z_ = true;
}

void Scale::clear_z() {
  z_ = 1.0;
  has_z_ = false;
}

kmldom::KmlDomType Alias::Type() const {
  return Type_Alias;
}

bool Alias::IsA(kmldom::KmlDomType type) const {
  return type == Type_Alias || Object::IsA(type);
}

const string& Alias::get_targethref() const {
  return targethref_;
}

bool Alias::has_targethref() const {
  return has_targethref_;
}

void Alias::set_targethref(const string& targethref) {
  targethref_ = targethref;
  has_targethref_ = true;
}

void Alias::clear_targethref() {
  targethref_.clear();
  has_targethref_ = false;
}

const string& Alias::get_sourcehref() const {
  return sourcehref_;
}

bool Alias::has_sourcehref() const {
  return has_sourcehref_;
}

void Alias::set_sourcehref(const string& sourcehref) {
  sourcehref_ = sourcehref;
  has_sourcehref_ = true;
}

void Alias::clear_sourcehref() {
  sourcehref_.clear();
  has_sourcehref_ = false;
}

kmldom::KmlDomType ResourceMap::Type() const {
  return Type_ResourceMap;
}

bool ResourceMap::IsA(kmldom::KmlDomType type) const {
  return type == Type_ResourceMap || Object::IsA(type);
}

size_t ResourceMap::get_alias_array_size() const {
  return alias_array_.size();
}

const AliasPtr& ResourceMap::get_alias_array_at(size_t index) const {
  return alias_array_[index];
}

kmldom::KmlDomType Model::Type() const {
  return Type_Model;
}

bool Model::IsA(kmldom::KmlDomType type) const {
  return type == Type_Model || Geometry::IsA(type);
}

const LocationPtr& Model::get_location() const {
  return location_;
}

bool Model::has_location() const {
  return location_ != nullptr;
}

void Model::set_location(const LocationPtr& location) {
  SetComplexChild(location, &location_);
}

void Model::clear_location() {
  set_location(NULL);
}

const OrientationPtr& Model::get_orientation() const {
  return orientation_;
}

bool Model::has_orientation() const {
  return orientation_ != nullptr;
}

void Model::set_orientation(const OrientationPtr& orientation) {
  SetComplexChild(orientation, &orientation_);
}

void Model::clear_orientation() {
  set_orientation(NULL);
}

const ScalePtr& Model::get_scale() const {
  return scale_;
}

bool Model::has_scale() const {
  return scale_ != nullptr;
}

void Model::set_scale(const ScalePtr& scale) {
  SetComplexChild(scale, &scale_);
}

void Model::clear_scale() {
  set_scale(NULL);
}

const LinkPtr& Model::get_link() const {
  return link_;
}

bool Model::has_link() const {
  return link_ != nullptr;
}

void Model::set_link(const LinkPtr& link) {
  SetComplexChild(link, &link_);
}

void Model::clear_link() {
  set_link(NULL);
}

const ResourceMapPtr& Model::get_resourcemap() const {
  return resourcemap_;
}

bool Model::has_resourcemap() const {
  return resourcemap_ != nullptr;
}

void Model::set_resourcemap(const ResourceMapPtr& resourcemap) {
  SetComplexChild(resourcemap, &resourcemap_);
}

void Model::clear_resourcemap() {
  resourcemap_ = NULL;
}
}  // end namespace kmldom
