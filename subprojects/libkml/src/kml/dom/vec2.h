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

// This file contains the declaration of the abstract Vec2 element.

#ifndef KML_DOM_VEC2_H__
#define KML_DOM_VEC2_H__

#include "kml/base/util.h"
#include "kml/config.h"
#include "kml/dom/element.h"
#include "kml/dom/kml22.h"

namespace kmlbase {
class Attributes;
}

namespace kmldom {

class Serializer;
class Visitor;

// OGC KML 2.2 Standard: 16.21 kml:vec2Type
// OGC KML 2.2 XSD: <complexType name="vec2Type"...
class KML_EXPORT Vec2 : public Element {
 public:
  virtual ~Vec2();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  double get_x() const;
  bool has_x() const;
  void set_x(double value);
  void clear_x();

  double get_y() const;
  bool has_y() const;
  void set_y(double value);
  void clear_y();

  int get_xunits() const;
  bool has_xunits() const;
  void set_xunits(int value);
  void clear_xunits();

  int get_yunits() const;
  bool has_yunits() const;
  void set_yunits(int value);
  void clear_yunits();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 protected:
  // Vec2 is abstract, derived class access only.
  Vec2();
  virtual void ParseAttributes(kmlbase::Attributes* attributes);
  virtual void SerializeAttributes(kmlbase::Attributes* attributes) const;
  void Serialize(Serializer& serializer) const;

 private:
  double x_;
  bool has_x_;
  double y_;
  bool has_y_;
  int xunits_;
  bool has_xunits_;
  int yunits_;
  bool has_yunits_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Vec2);
};

}  // end namespace kmldom

#endif  // KML_DOM_VEC2_H__
