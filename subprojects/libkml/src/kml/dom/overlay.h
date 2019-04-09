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

// This file contains the declarations for the abstract Overlay element
// and the concrete GroundOverlay, ScreenOverlay, and PhotoOverlay elements
// and their child elements LatLonBox, OverlayXY, ScreenXY, RotationXY,
// Size, ViewVolume, and ImagePyramid.

#ifndef KML_DOM_OVERLAY_H__
#define KML_DOM_OVERLAY_H__

#include "kml/base/color32.h"
#include "kml/config.h"
#include "kml/dom/abstractlatlonbox.h"
#include "kml/dom/feature.h"
#include "kml/dom/kml22.h"
#include "kml/dom/kml_ptr.h"
#include "kml/dom/object.h"
#include "kml/dom/vec2.h"

namespace kmldom {

class Serializer;
class Visitor;
class VisitorDriver;

// OGC KML 2.2 Standard: 11.1 kml:AbstractOverlayGroup
// OGC KML 2.2 XSD: <element name="AbstractOverlayGroup"...
class KML_EXPORT Overlay : public Feature {
 public:
  virtual ~Overlay();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <color>
  const kmlbase::Color32& get_color() const;
  bool has_color() const;
  void set_color(const kmlbase::Color32& color);
  void clear_color();

  // <drawOrder>
  int get_draworder() const;
  bool has_draworder() const;
  void set_draworder(int draworder);
  void clear_draworder();

  // <Icon>
  const IconPtr& get_icon() const;
  bool has_icon() const;
  void set_icon(const IconPtr& icon);
  void clear_icon();

  // Visitor API methods, see visitor.h.
  virtual void AcceptChildren(VisitorDriver* driver);

 protected:
  // Overlay is abstract.
  Overlay();
  virtual void AddElement(const ElementPtr& element);
  virtual void Serialize(Serializer& serializer) const;

 private:
  kmlbase::Color32 color_;
  bool has_color_;
  int draworder_;
  bool has_draworder_;
  IconPtr icon_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Overlay);
};

// <LatLonBox>
class KML_EXPORT LatLonBox : public AbstractLatLonBox {
 public:
  virtual ~LatLonBox();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <rotation>
  double get_rotation() const;
  bool has_rotation() const;
  void set_rotation(double rotation);
  void clear_rotation();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  LatLonBox();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;

 private:
  double rotation_;
  bool has_rotation_;

  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(LatLonBox);
};

// <gx:LatLonQuad>
class KML_EXPORT GxLatLonQuad : public Object {
 public:
  virtual ~GxLatLonQuad();
  static KmlDomType ElementType();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <coordinates>
  const CoordinatesPtr& get_coordinates() const;
  bool has_coordinates() const;
  void set_coordinates(const CoordinatesPtr& coordinates);
  void clear_coordinates();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  GxLatLonQuad();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  CoordinatesPtr coordinates_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(GxLatLonQuad);
};

// <GroundOverlay>
class KML_EXPORT GroundOverlay : public Overlay {
 public:
  virtual ~GroundOverlay();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <altitude>
  double get_altitude() const;
  bool has_altitude() const;
  void set_altitude(double altitude);
  void clear_altitude();

  // <altitudeMode>
  int get_altitudemode() const;
  bool has_altitudemode() const;
  void set_altitudemode(int altitudemode);
  void clear_altitudemode();

  // <gx:altitudeMode>
  int get_gx_altitudemode() const;
  bool has_gx_altitudemode() const;
  void set_gx_altitudemode(int gx_altitudemode);
  void clear_gx_altitudemode();

  // <LatLonBox>
  const LatLonBoxPtr& get_latlonbox() const;
  bool has_latlonbox() const;
  void set_latlonbox(const LatLonBoxPtr& latlonbox);
  void clear_latlonbox();

  // <gx:LatLonQuad>
  const GxLatLonQuadPtr& get_gx_latlonquad() const;
  bool has_gx_latlonquad() const;
  void set_gx_latlonquad(const GxLatLonQuadPtr& gx_latlonquad);
  void clear_gx_latlonquad();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  GroundOverlay();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;

 private:
  double altitude_;
  bool has_altitude_;
  int altitudemode_;
  bool has_altitudemode_;
  int gx_altitudemode_;
  bool has_gx_altitudemode_;
  LatLonBoxPtr latlonbox_;
  GxLatLonQuadPtr gx_latlonquad_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(GroundOverlay);
};

// <overlayXY>
class KML_EXPORT OverlayXY : public Vec2 {
 public:
  virtual ~OverlayXY();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  OverlayXY();
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(OverlayXY);
};

// <screenXY>
class KML_EXPORT ScreenXY : public Vec2 {
 public:
  virtual ~ScreenXY();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  ScreenXY();
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(ScreenXY);
};

// <rotationXY>
class KML_EXPORT RotationXY : public Vec2 {
 public:
  virtual ~RotationXY();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  RotationXY();
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(RotationXY);
};

// <size>
class KML_EXPORT Size : public Vec2 {
 public:
  virtual ~Size();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  Size();
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Size);
};

// <ScreenOverlay>
class KML_EXPORT ScreenOverlay : public Overlay {
 public:
  virtual ~ScreenOverlay();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <overlayXY>
  const OverlayXYPtr& get_overlayxy() const;
  bool has_overlayxy() const;
  void set_overlayxy(const OverlayXYPtr& overlayxy);
  void clear_overlayxy();

  // <screenXY>
  const ScreenXYPtr& get_screenxy() const;
  bool has_screenxy() const;
  void set_screenxy(const ScreenXYPtr& screenxy);
  void clear_screenxy();

  // <rotationXY>
  const RotationXYPtr& get_rotationxy() const;
  bool has_rotationxy() const;
  void set_rotationxy(const RotationXYPtr& rotationxy);
  void clear_rotationxy();

  // <size>
  const SizePtr& get_size() const;
  bool has_size() const;
  void set_size(const SizePtr& size);
  void clear_size();

  // <rotation>
  double get_rotation() const;
  bool has_rotation() const;
  void set_rotation(double rotation);
  void clear_rotation();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  ScreenOverlay();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  OverlayXYPtr overlayxy_;
  ScreenXYPtr screenxy_;
  RotationXYPtr rotationxy_;
  SizePtr size_;
  double rotation_;
  bool has_rotation_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(ScreenOverlay);
};

// <ViewVolume>
class KML_EXPORT ViewVolume : public Object {
 public:
  virtual ~ViewVolume();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <leftFov>
  double get_leftfov() const;
  bool has_leftfov() const;
  void set_leftfov(double leftfov);
  void clear_leftfov();

  // <rightFov>
  double get_rightfov() const;
  bool has_rightfov() const;
  void set_rightfov(double rightfov);
  void clear_rightfov();

  // <bottomFov>
  double get_bottomfov() const;
  bool has_bottomfov() const;
  void set_bottomfov(double altitude);
  void clear_bottomfov();

  // <topFov>
  double get_topfov() const;
  bool has_topfov() const;
  void set_topfov(double topfov);
  void clear_topfov();

  // <near>
  double get_near() const;
  bool has_near() const;
  void set_near(double val);
  void clear_near();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  ViewVolume();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  double leftfov_;
  bool has_leftfov_;
  double rightfov_;
  bool has_rightfov_;
  double bottomfov_;
  bool has_bottomfov_;
  double topfov_;
  bool has_topfov_;
  double near_;
  bool has_near_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(ViewVolume);
};

// <ImagePyramid>
class KML_EXPORT ImagePyramid : public Object {
 public:
  virtual ~ImagePyramid();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <tileSize>
  int get_tilesize() const;
  bool has_tilesize() const;
  void set_tilesize(int tilesize);
  void clear_tilesize();

  // <maxWidth>
  int get_maxwidth() const;
  bool has_maxwidth() const;
  void set_maxwidth(int maxwidth);
  void clear_maxwidth();

  // <maxHeight>
  int get_maxheight() const;
  bool has_maxheight() const;
  void set_maxheight(int altitude);
  void clear_maxheight();

  // <gridOrigin>
  int get_gridorigin() const;
  bool has_gridorigin() const;
  void set_gridorigin(int gridorigin);
  void clear_gridorigin();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  ImagePyramid();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  int tilesize_;
  bool has_tilesize_;
  int maxwidth_;
  bool has_maxwidth_;
  int maxheight_;
  bool has_maxheight_;
  int gridorigin_;
  bool has_gridorigin_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(ImagePyramid);
};

// <PhotoOverlay>
class KML_EXPORT PhotoOverlay : public Overlay {
 public:
  virtual ~PhotoOverlay();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <rotation>
  double get_rotation() const;
  bool has_rotation() const;
  void set_rotation(double rotation);
  void clear_rotation();

  // <ViewVolume>
  const ViewVolumePtr& get_viewvolume() const;
  bool has_viewvolume() const;
  void set_viewvolume(const ViewVolumePtr& viewvolume);
  void clear_viewvolume();

  // <ImagePyramid>
  const ImagePyramidPtr& get_imagepyramid() const;
  bool has_imagepyramid() const;
  void set_imagepyramid(const ImagePyramidPtr& imagepyramid);
  void clear_imagepyramid();

  // <Point>
  const PointPtr& get_point() const;
  bool has_point() const;
  void set_point(const PointPtr& point);
  void clear_point();

  // <shape>
  int get_shape() const;
  bool has_shape() const;
  void set_shape(int shape);
  void clear_shape();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  PhotoOverlay();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  double rotation_;
  bool has_rotation_;
  ViewVolumePtr viewvolume_;
  ImagePyramidPtr imagepyramid_;
  PointPtr point_;
  int shape_;
  bool has_shape_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(PhotoOverlay);
};

}  // end namespace kmldom

#endif  // KML_DOM_OVERLAY_H__
