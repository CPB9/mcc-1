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

// This file declares the Region, LatLonAltBox and Lod elements.
// LatLonAltBox and Lod are here because they are used only with Region.

#ifndef KML_DOM_REGION_H__
#define KML_DOM_REGION_H__

#include "kml/base/util.h"
#include "kml/config.h"
#include "kml/dom/abstractlatlonbox.h"
#include "kml/dom/kml22.h"
#include "kml/dom/kml_ptr.h"
#include "kml/dom/object.h"

namespace kmldom {

class Visitor;
class VisitorDriver;

// <LatLonAltBox>
class KML_EXPORT LatLonAltBox : public AbstractLatLonBox {
 public:
  virtual ~LatLonAltBox();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <minAltitude>
  double get_minaltitude() const;
  bool has_minaltitude() const;
  void set_minaltitude(double minaltitude);
  void clear_minaltitude();

  // <maxAltitude>
  double get_maxaltitude() const;
  bool has_maxaltitude() const;
  void set_maxaltitude(double maxaltitude);
  void clear_maxaltitude();

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

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  LatLonAltBox();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  double minaltitude_;
  bool has_minaltitude_;
  double maxaltitude_;
  bool has_maxaltitude_;
  int altitudemode_;
  bool has_altitudemode_;
  int gx_altitudemode_;
  bool has_gx_altitudemode_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(LatLonAltBox);
};

// <Lod>
class KML_EXPORT Lod : public Object {
 public:
  virtual ~Lod();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <minLodPixels>
  double get_minlodpixels() const;
  bool has_minlodpixels() const;
  void set_minlodpixels(double minlodpixels);
  void clear_minlodpixels();

  // <maxLodPixels>
  double get_maxlodpixels() const;
  bool has_maxlodpixels() const;
  void set_maxlodpixels(double minlodpixels);
  void clear_maxlodpixels();

  // <minFadeExtent>
  double get_minfadeextent() const;
  bool has_minfadeextent() const;
  void set_minfadeextent(double minlodpixels);
  void clear_minfadeextent();

  // <maxFadeExtent>
  double get_maxfadeextent() const;
  bool has_maxfadeextent() const;
  void set_maxfadeextent(double maxlodpixels);
  void clear_maxfadeextent();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  Lod();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  double minlodpixels_;
  bool has_minlodpixels_;
  double maxlodpixels_;
  bool has_maxlodpixels_;
  double minfadeextent_;
  bool has_minfadeextent_;
  double maxfadeextent_;
  bool has_maxfadeextent_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Lod);
};

// <Region>
class KML_EXPORT Region : public Object {
 public:
  virtual ~Region();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <LatLonAltBox>
  const LatLonAltBoxPtr& get_latlonaltbox() const;
  bool has_latlonaltbox() const;
  void set_latlonaltbox(const LatLonAltBoxPtr& latlonaltbox);
  void clear_latlonaltbox();

  // <Lod>
  const LodPtr& get_lod() const;
  bool has_lod() const;
  void set_lod(const LodPtr& lod);
  void clear_lod();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  Region();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  LatLonAltBoxPtr latlonaltbox_;
  LodPtr lod_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Region);
};

}  // end namespace kmldom

#endif  // KML_DOM_REGION_H__
