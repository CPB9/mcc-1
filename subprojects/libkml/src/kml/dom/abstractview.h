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

#ifndef KML_DOM_ABSTRACTVIEW_H__
#define KML_DOM_ABSTRACTVIEW_H__

#include "kml/config.h"
#include "kml/dom/kml_ptr.h"
#include "kml/dom/object.h"

namespace kmldom {

class Visitor;
class VisitorDriver;

// OGC KML 2.2 Standard: 14.1 kml:AbstractViewGroup
// OGC KML 2.2 XSD: <element name="AbstractViewGroup"...
class KML_EXPORT AbstractView : public Object {
 public:
  virtual ~AbstractView();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // From kml:AbstractViewObjectExtensionGroup.
  const TimePrimitivePtr& get_gx_timeprimitive() const;
  bool has_gx_timeprimitive() const;
  void set_gx_timeprimitive(const TimePrimitivePtr& gx_timeprimitive);
  void clear_gx_timeprimitive();

  // Visitor API methods, see visitor.h.
  virtual void AcceptChildren(VisitorDriver* driver);

 protected:
  // AbstractView is abstract.
  AbstractView();
  virtual void AddElement(const ElementPtr& element);
  virtual void Serialize(Serializer& serializer) const;

 private:
  TimePrimitivePtr gx_timeprimitive_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(AbstractView);
};

// This is an internal convenience class for code common to LookAt and Camera.
// This is not part of the OGC KML 2.2 standard.
class KML_EXPORT AbstractViewCommon : public AbstractView {
 public:
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

  // <altitudeMode>
  int get_altitudemode() const;
  bool has_altitudemode() const;
  void set_altitudemode(int altitudemode);
  void clear_altitudemode();

  // <gx:altitudeMode>
  // NOTE: In OGC KML 2.2 altitude mode is a group hence only one of
  // <altitudeMode> _OR_ <gx:altitudeMode> shall be used for XSD validation.
  int get_gx_altitudemode() const;
  bool has_gx_altitudemode() const;
  void set_gx_altitudemode(int gx_altitudemode);
  void clear_gx_altitudemode();

 protected:
  // AbstractViewCommon is abstract.
  AbstractViewCommon();
  ~AbstractViewCommon();
  virtual void AddElement(const ElementPtr& element);
  virtual void SerializeBeforeR(Serializer& serializer) const;
  virtual void SerializeAfterR(Serializer& serializer) const;

 private:
  double longitude_;
  bool has_longitude_;
  double latitude_;
  bool has_latitude_;
  double altitude_;
  bool has_altitude_;
  double heading_;
  bool has_heading_;
  double tilt_;
  bool has_tilt_;
  int altitudemode_;
  bool has_altitudemode_;
  int gx_altitudemode_;
  bool has_gx_altitudemode_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(AbstractViewCommon);
};

// <LookAt>
class KML_EXPORT LookAt : public AbstractViewCommon {
 public:
  virtual ~LookAt();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <range>
  double get_range() const;
  bool has_range() const;
  void set_range(double range);
  void clear_range();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  LookAt();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  double range_;
  bool has_range_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(LookAt);
};

// <Camera>
class KML_EXPORT Camera : public AbstractViewCommon {
 public:
  virtual ~Camera();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <roll>
  double get_roll() const;
  bool has_roll() const;
  void set_roll(double roll);
  void clear_roll();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  Camera();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  double roll_;
  bool has_roll_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Camera);
};

}  // end namespace kmldom

#endif  // KML_DOM_ABSTRACTVIEW_H__
