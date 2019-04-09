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

// This file contains the declarations for the abstract Geometry element
// and concrete coordinates, Point, LineString, LinearRing, Polygon,
// outerBoundaryIs, and innerBoundaryIs elements.

// In addition to classes for the abstract and concrete elements in the
// KML standard there are internal convenience classes used here to hold
// common code.  Each such class is named *GeometryCommon and follows
// this general pattern: constructor is protected, implements set,get,has,clear
// for the field it owns, and parses that field (implements AddElement).
// Each concrete element owns serialization of all fields for itself as per
// the order the KML standard.  The KML standard does not specify the common
// simple elements in an order that maps well to a type hierarchy hence
// the more typical pattern of abstract types serializing their own
// fields is not followed here.
//
// Here is a quick summary of the type hierarchy used and what fields
// are associated with each type:
//
// class Geometry : public Object
//   AbstractGeometryGroup in the KML standard.  No child elements.
// class AltitudeGeometryCommon : public Geometry
//   Geometry with <altitudeMode>
// class ExtrudeGeometryCommon : public AltitudeGeometryCommon
//   Geometry with <altitudeMode> + <extrude>
// class CoordinatesGeometryCommon : public ExtrudeGeometryCommon
//   Geometry with <altitudeMode> + <extrude> + <coordinates>
// class Point : public CoordinatesGeometryCommon
//   <Point> has <altitudeMode> + <extrude> + <coordinates>
// class LineCommon : public CoordinatesGeometryCommon
//   LineCommon has <altitudeMode> + <extrude> + <coordinates> + <tessellate>
// class LineString : public LineCommon
//   <LineString> is an instantiation of LineCommon
// class LinearRing : public LineCommon
//   <LinearRing> is an instantiation of LineCommon
// class BoundaryCommon : public Element
//   BoundaryCommon has <LinearRing>
// class OuterBoundaryIs : public BoundaryCommon
//  <outerBoundaryIs> is an instantiation of BoundaryCommon
// class InnerBoundaryIs : public BoundaryCommon
//  <innerBoundaryIs> is an instantiation of BoundaryCommon
// class Polygon : public ExtrudeGeometryCommon
//   <Polygon> has <altitudeMode> + <extrude> + <tessellate> +
//      <outerBoundaryIs> and N x <innerBoundaryIs>
// class MultiGeometry : public Geometry
// Note: class Model : public AltitudeGeometryCommon

#ifndef KML_DOM_GEOMETRY_H__
#define KML_DOM_GEOMETRY_H__

#include <vector>
#include "kml/base/util.h"
#include "kml/base/vec3.h"
#include "kml/config.h"
#include "kml/dom/element.h"
#include "kml/dom/kml22.h"
#include "kml/dom/kml_ptr.h"
#include "kml/dom/object.h"

namespace kmldom {

class Serializer;
class Visitor;
class VisitorDriver;

// <coordinates>
class KML_EXPORT Coordinates : public BasicElement<Type_coordinates> {
 public:
  virtual ~Coordinates();

  // The main KML-specific API
  void add_latlngalt(double latitude, double longitude, double altitude);

  void add_latlng(double latitude, double longitude);

  void insert_latlng(std::size_t index, double latitude, double longitude);

  void remove_at(std::size_t index);

  void add_vec3(const kmlbase::Vec3& vec3);

  size_t get_coordinates_array_size() const;

  const kmlbase::Vec3& get_coordinates_array_at(size_t index) const;

  kmlbase::Vec3& get_coordinates_array_at(size_t index);

  void erase_coordinates_array_at(size_t index);

  void set_coordinates_array_at(const kmlbase::Vec3& vec, size_t index);

  // Internal methods used in parser.  Public for unittest purposes.
  // See .cc for more details.
  void Parse(const string& char_data);
  static bool ParseVec3(const char* coords, char** nextp, kmlbase::Vec3* vec);

  // This clears the internal coordinates array.
  void Clear();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  Coordinates();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;

  std::vector<kmlbase::Vec3> coordinates_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Coordinates);
};

// OGC KML 2.2 Standard: 10.1 kml:AbstractGeometryGroup
// OGC KML 2.2 XSD: <element name="AbstractGeometryGroup"...
class KML_EXPORT Geometry : public Object {
 public:
  virtual ~Geometry();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

 protected:
  // Geometry is abstract.
  Geometry();

 private:
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Geometry);
};

// Internal convenience class for any Geometry with <altitudeMode>.
// This is not in the KML standard, hence there is no type info.
class KML_EXPORT AltitudeGeometryCommon : public Geometry {
 public:
  virtual ~AltitudeGeometryCommon();

 protected:
  AltitudeGeometryCommon();

 public:
  // <altitudeMode>
  int get_altitudemode() const;
  bool has_altitudemode() const;
  void set_altitudemode(int value);
  void clear_altitudemode();

  // <gx:altitudeMode>
  int get_gx_altitudemode() const;
  bool has_gx_altitudemode() const;
  void set_gx_altitudemode(int value);
  void clear_gx_altitudemode();

  virtual void AddElement(const ElementPtr& element);

 private:
  int altitudemode_;
  bool has_altitudemode_;
  int gx_altitudemode_;
  bool has_gx_altitudemode_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(AltitudeGeometryCommon);
};

// Internal convenience class for any Geometry with <altitudeMode> + <extrude>
// This is not in the KML standard, hence there is no type info.
class KML_EXPORT ExtrudeGeometryCommon : public AltitudeGeometryCommon {
 public:
  virtual ~ExtrudeGeometryCommon();

  // <extrude>
  bool get_extrude() const;
  bool has_extrude() const;
  void set_extrude(bool value);
  void clear_extrude();

 protected:
  ExtrudeGeometryCommon();
  virtual void AddElement(const ElementPtr& element);

 private:
  bool extrude_;
  bool has_extrude_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(ExtrudeGeometryCommon);
};

// Internal convenience class for any Geometry with
// <altitudeMode> + <extrude> + <coordinates>.
// This is not in the KML standard, hence there is no type info.
class KML_EXPORT CoordinatesGeometryCommon : public ExtrudeGeometryCommon {
 public:
  virtual ~CoordinatesGeometryCommon();

 public:
  // <coordinates>
  const CoordinatesPtr& get_coordinates() const;
  bool has_coordinates() const;
  void set_coordinates(const CoordinatesPtr& coordinates);
  void clear_coordinates();

  // Visitor API methods, see visitor.h.
  virtual void AcceptChildren(VisitorDriver* driver);

 protected:
  CoordinatesGeometryCommon();
  // Parser support
  virtual void AddElement(const ElementPtr& element);

 private:
  CoordinatesPtr coordinates_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(CoordinatesGeometryCommon);
};

// <Point>
class KML_EXPORT Point : public CoordinatesGeometryCommon {
 public:
  virtual ~Point();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  Point();
  friend class Serializer;
  void Serialize(Serializer& serializer) const;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Point);
};

// Internal convenience class for code common to LineString and LinearRing.
// This is not in the KML standard, hence there is no type info.
class KML_EXPORT LineCommon : public CoordinatesGeometryCommon {
 public:
  virtual ~LineCommon();

 public:
  // <tessellate>
  bool get_tessellate() const;
  bool has_tessellate() const;
  void set_tessellate(bool value);
  void clear_tessellate();

 protected:
  LineCommon();
  // Parser support
  virtual void AddElement(const ElementPtr& element);

 private:
  friend class Serializer;
  void Serialize(Serializer& serializer) const;
  bool tessellate_;
  bool has_tessellate_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(LineCommon);
};

// <LineString>
class KML_EXPORT LineString : public LineCommon {
 public:
  virtual ~LineString();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  LineString();
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(LineString);
};

// <LinearRing>
class KML_EXPORT LinearRing : public LineCommon {
 public:
  virtual ~LinearRing();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  LinearRing();
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(LinearRing);
};

// Internal class for code common to OuterBoundaryIs and InnerBoundaryIs.
// This is not in the KML standard, hence there is no type info.
class KML_EXPORT BoundaryCommon : public Element {
 public:
  virtual ~BoundaryCommon();

 public:
  const LinearRingPtr& get_linearring() const;
  bool has_linearring() const;
  void set_linearring(const LinearRingPtr& linearring);
  void clear_linearring();

  // Parser support
  virtual void AddElement(const ElementPtr& element);

  // Visitor API methods, see visitor.h.
  virtual void AcceptChildren(VisitorDriver* driver);

 protected:
  BoundaryCommon();
  virtual void Serialize(Serializer& serializer) const;

 private:
  LinearRingPtr linearring_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(BoundaryCommon);
};

// <outerBoundaryIs>
class KML_EXPORT OuterBoundaryIs : public BoundaryCommon {
 public:
  virtual ~OuterBoundaryIs();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  OuterBoundaryIs();
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(OuterBoundaryIs);
};

// <innerBoundaryIs>
class KML_EXPORT InnerBoundaryIs : public BoundaryCommon {
 public:
  virtual ~InnerBoundaryIs();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  InnerBoundaryIs();
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(InnerBoundaryIs);
};

// <Polygon>
class KML_EXPORT Polygon : public ExtrudeGeometryCommon {
 public:
  virtual ~Polygon();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <tessellate>
  bool get_tessellate() const;
  bool has_tessellate() const;
  void set_tessellate(bool value);
  void clear_tessellate();

  // <outerBoundaryIs>
  const OuterBoundaryIsPtr& get_outerboundaryis() const;
  bool has_outerboundaryis() const;
  void set_outerboundaryis(const OuterBoundaryIsPtr& outerboundaryis);
  void clear_outerboundaryis();

  // <innerBoundaryIs>
  void add_innerboundaryis(const InnerBoundaryIsPtr& innerboundaryis);

  size_t get_innerboundaryis_array_size() const;

  const InnerBoundaryIsPtr& get_innerboundaryis_array_at(size_t index);

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  Polygon();

  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);

  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;

  bool tessellate_;
  bool has_tessellate_;
  OuterBoundaryIsPtr outerboundaryis_;
  std::vector<InnerBoundaryIsPtr> innerboundaryis_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Polygon);
};

// <MultiGeometry>
class KML_EXPORT MultiGeometry : public Geometry {
 public:
  virtual ~MultiGeometry();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // The main KML-specific API
  void add_geometry(const GeometryPtr& geometry);

  size_t get_geometry_array_size() const;

  const GeometryPtr& get_geometry_array_at(size_t index) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  MultiGeometry();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  std::vector<GeometryPtr> geometry_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(MultiGeometry);
};

// <gx:Track>
class KML_EXPORT GxTrack : public AltitudeGeometryCommon {
 public:
  virtual ~GxTrack();
  static KmlDomType ElementType();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <when>
  size_t get_when_array_size();
  void add_when(const string& when);
  const string& get_when_array_at(size_t index) const;

  // <gx:coord>
  size_t get_gx_coord_array_size();
  void add_gx_coord(const kmlbase::Vec3& gx_coord);
  const kmlbase::Vec3& get_gx_coord_array_at(size_t index) const;

  // <gx:angles>
  size_t get_gx_angles_array_size();
  void add_gx_angles(const kmlbase::Vec3& gx_angles);
  const kmlbase::Vec3& get_gx_angles_array_at(size_t index) const;

  // <Model>
  const ModelPtr& get_model() const;
  void set_model(const ModelPtr& model);
  bool has_model() const;
  void clear_model();

  // <ExtendedData>
  const ExtendedDataPtr& get_extendeddata() const;
  bool has_extendeddata() const;
  void set_extendeddata(const ExtendedDataPtr& extendeddata);
  void clear_extendeddata();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

  // Internal methods used in parser.  Public for unittest purposes.
  // See .cc for more details.
  void Parse(const string& char_data, std::vector<kmlbase::Vec3>* out);

 private:
  friend class KmlFactory;
  GxTrack();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  std::vector<string> when_array_;
  std::vector<kmlbase::Vec3> gx_coord_array_;
  std::vector<kmlbase::Vec3> gx_angles_array_;
  ModelPtr model_;
  ExtendedDataPtr extendeddata_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(GxTrack);
};

// <gx:MultiTrack>
class KML_EXPORT GxMultiTrack : public Geometry {
 public:
  virtual ~GxMultiTrack();
  static KmlDomType ElementType();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  bool get_gx_interpolate() const;
  bool has_gx_interpolate() const;
  void set_gx_interpolate(bool value);
  void clear_gx_interpolate();

  void add_gx_track(const GxTrackPtr& gx_track);

  size_t get_gx_track_array_size() const;

  const GxTrackPtr& get_gx_track_array_at(size_t index) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  GxMultiTrack();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  bool gx_interpolate_;
  bool has_gx_interpolate_;
  std::vector<GxTrackPtr> gx_track_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(GxMultiTrack);
};

// HACK: the rest of this file contains what was in, and what should return to,
// kml/dom/model.h. GxTrack was added to this file, which has a <Model>. Since
// Model is defined in its own file, this double inclusion, coupled with the
// inline implementation of most methods in the headers, caused the builds of
// other dependent projects to break. The correct solution is to ensure that
// the headers are pure and all implementation is in the .cc files.

// <Location>
class KML_EXPORT Location : public Object {
 public:
  virtual ~Location();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <longitude>
  double get_longitude() const;
  bool has_longitude() const;
  void set_longitude(double longitude);
  void clear_longitude();

  // <latitude>
  double get_latitude() const;
  bool has_latitude() const;
  void set_latitude(double latitude);
  void clear_latitude();

  // <altitude>
  double get_altitude() const;
  bool has_altitude() const;
  void set_altitude(double altitude);
  void clear_altitude();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  Location();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  double longitude_;
  bool has_longitude_;
  double latitude_;
  bool has_latitude_;
  double altitude_;
  bool has_altitude_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Location);
};

// <Orientation>
class KML_EXPORT Orientation : public Object {
 public:
  virtual ~Orientation();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <heading>
  double get_heading() const;
  bool has_heading() const;
  void set_heading(double heading);
  void clear_heading();

  // <tilt>
  double get_tilt() const;
  bool has_tilt() const;
  void set_tilt(double tilt);
  void clear_tilt();

  // <roll>
  double get_roll() const;
  bool has_roll() const;
  void set_roll(double roll);
  void clear_roll();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  Orientation();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  double heading_;
  bool has_heading_;
  double tilt_;
  bool has_tilt_;
  double roll_;
  bool has_roll_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Orientation);
};

// <Scale>
class KML_EXPORT Scale : public Object {
 public:
  virtual ~Scale();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <x>
  double get_x() const;
  bool has_x() const;
  void set_x(double x);
  void clear_x();

  // <y>
  double get_y() const;
  bool has_y() const;
  void set_y(double y);
  void clear_y();

  // <z>
  double get_z() const;
  bool has_z() const;
  void set_z(double z);
  void clear_z();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  Scale();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  double x_;
  bool has_x_;
  double y_;
  bool has_y_;
  double z_;
  bool has_z_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Scale);
};

// <Alias>
class KML_EXPORT Alias : public Object {
 public:
  virtual ~Alias();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <targetHref>
  const string& get_targethref() const;
  bool has_targethref() const;
  void set_targethref(const string& targethref);
  void clear_targethref();

  // <sourceHref>
  const string& get_sourcehref() const;
  bool has_sourcehref() const;
  void set_sourcehref(const string& sourcehref);
  void clear_sourcehref();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  Alias();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  string targethref_;
  bool has_targethref_;
  string sourcehref_;
  bool has_sourcehref_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Alias);
};

// <ResourceMap>
class KML_EXPORT ResourceMap : public Object {
 public:
  virtual ~ResourceMap();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  void add_alias(const AliasPtr& alias);

  size_t get_alias_array_size() const;

  const AliasPtr& get_alias_array_at(size_t index) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  ResourceMap();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  std::vector<AliasPtr> alias_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(ResourceMap);
};

// <Model>
class KML_EXPORT Model : public AltitudeGeometryCommon {
 public:
  virtual ~Model();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <Location>
  const LocationPtr& get_location() const;
  bool has_location() const;
  void set_location(const LocationPtr& location);
  void clear_location();

  // <Orientation>
  const OrientationPtr& get_orientation() const;
  bool has_orientation() const;
  void set_orientation(const OrientationPtr& orientation);
  void clear_orientation();

  // <Scale>
  const ScalePtr& get_scale() const;
  bool has_scale() const;
  void set_scale(const ScalePtr& scale);
  void clear_scale();

  // <Link>
  const LinkPtr& get_link() const;
  bool has_link() const;
  void set_link(const LinkPtr& link);
  void clear_link();

  // <ResourceMap>
  const ResourceMapPtr& get_resourcemap() const;
  bool has_resourcemap() const;
  void set_resourcemap(const ResourceMapPtr& resourcemap);
  void clear_resourcemap();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  Model();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  LocationPtr location_;
  OrientationPtr orientation_;
  ScalePtr scale_;
  LinkPtr link_;
  ResourceMapPtr resourcemap_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Model);
};

}  // namespace kmldom

#endif  // KML_DOM_GEOMETRY_H__
