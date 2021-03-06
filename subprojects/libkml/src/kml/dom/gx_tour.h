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

// This file contains the declaration of <gx:Tour>, <gx:Playlist>,
// <gx:AnimatedUpdate>, <gx:Wait>, <gx:FlyTo>, <gx:SoundCue>, and
// <gx:TourControl>.

#ifndef KML_GX_DOM_TOUR_H__
#define KML_GX_DOM_TOUR_H__

#include "kml/config.h"
#include "kml/dom/feature.h"
#include "kml/dom/kml22.h"
#include "kml/dom/kml_ptr.h"
#include "kml/dom/object.h"

namespace kmlbase {
class Attributes;
}

namespace kmldom {

class Serializer;
class Visitor;
class VisitorDriver;

// <gx:Tour>
class KML_EXPORT GxTour : public Feature {
 public:
  virtual ~GxTour();
  static KmlDomType ElementType();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <gx:Playlist>
  const GxPlaylistPtr& get_gx_playlist() const;
  bool has_gx_playlist() const;
  void set_gx_playlist(const GxPlaylistPtr& gx_playlist);
  void clear_gx_playlist();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  GxTour();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  GxPlaylistPtr gx_playlist_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(GxTour);
};

// <gx:Playlist>
class KML_EXPORT GxPlaylist : public Object {
 public:
  virtual ~GxPlaylist();
  static KmlDomType ElementType();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // GxTourPrimitive...
  void add_gx_tourprimitive(const GxTourPrimitivePtr& tourprimitive);
  size_t get_gx_tourprimitive_array_size() const;
  const GxTourPrimitivePtr& get_gx_tourprimitive_array_at(size_t index) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  GxPlaylist();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  std::vector<GxTourPrimitivePtr> gx_tourprimitive_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(GxPlaylist);
};

// GxTourPrimitive is an abstract base type.  This corresponds to
// gx:AbstractGxTourPrimitiveType/Group in KML 2.2 gx.
class KML_EXPORT GxTourPrimitive : public Object {
 public:
  static KmlDomType ElementType();
  virtual KmlDomType Type() const;
  bool IsA(KmlDomType type) const;

 protected:
  GxTourPrimitive();

 private:
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(GxTourPrimitive);
};

// Intermediate common class for GxTourPrimitive with gx_duration.
class KML_EXPORT GxTourPrimitiveCommon : public GxTourPrimitive {
 public:
  //  <gx:duration>
  double get_gx_duration() const;
  bool has_gx_duration() const;
  void set_gx_duration(double gx_duration);
  void clear_gx_duration();

 protected:
  // This is an internal abstract element and is inherited only and never
  // instantiated directly.
  GxTourPrimitiveCommon();

  virtual void AddElement(const ElementPtr& element);
  virtual void Serialize(Serializer& serializer) const;

 private:
  bool has_gx_duration_;
  double gx_duration_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(GxTourPrimitiveCommon);
};

// <gx:AnimatedUpdate>
class KML_EXPORT GxAnimatedUpdate : public GxTourPrimitiveCommon {
 public:
  virtual ~GxAnimatedUpdate();
  static KmlDomType ElementType();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <Update>
  const UpdatePtr& get_update() const;
  bool has_update() const;
  void set_update(const UpdatePtr& update);
  void clear_update();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  GxAnimatedUpdate();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  UpdatePtr update_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(GxAnimatedUpdate);
};

// <gx:FlyTo>
class KML_EXPORT GxFlyTo : public GxTourPrimitiveCommon {
 public:
  virtual ~GxFlyTo();
  static KmlDomType ElementType();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <gx:flyToMode>.
  int get_gx_flytomode() const;
  bool has_gx_flytomode() const;
  void set_gx_flytomode(int value);
  void clear_gx_flytomode();

  // AbstractView
  const AbstractViewPtr& get_abstractview() const;
  bool has_abstractview() const;
  void set_abstractview(const AbstractViewPtr& abstractview);
  void clear_abstractview();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  int gx_flytomode_;
  bool has_gx_flytomode_;
  friend class KmlFactory;
  GxFlyTo();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  AbstractViewPtr abstractview_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(GxFlyTo);
};

// <gx:Wait>
class KML_EXPORT GxWait : public GxTourPrimitiveCommon {
 public:
  virtual ~GxWait();
  static KmlDomType ElementType();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  GxWait();
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(GxWait);
};

// <gx:SoundCue>
class KML_EXPORT GxSoundCue : public GxTourPrimitive {
 public:
  virtual ~GxSoundCue();
  static KmlDomType ElementType();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <href>
  const string& get_href() const;
  bool has_href() const;
  void set_href(const string& href);
  void clear_href();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  string href_;
  bool has_href_;
  friend class KmlFactory;
  GxSoundCue();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(GxSoundCue);
};

// <gx:TourControl>
class KML_EXPORT GxTourControl : public GxTourPrimitive {
 public:
  virtual ~GxTourControl();
  static KmlDomType ElementType();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <gx:playMode>
  int get_gx_playmode() const;
  bool has_gx_playmode() const;
  void set_gx_playmode(int value);
  void clear_gx_playmode();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  int gx_playmode_;
  bool has_gx_playmode_;
  friend class KmlFactory;
  GxTourControl();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(GxTourControl);
};

}  // end namespace kmldom

#endif  // KML_GX_DOM_TOUR_H__
