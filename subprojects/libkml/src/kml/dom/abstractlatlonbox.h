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

#ifndef KML_DOM_ABSTRACTLATLONBOX_H__
#define KML_DOM_ABSTRACTLATLONBOX_H__

#include "kml/config.h"
#include "kml/dom/kml22.h"
#include "kml/dom/object.h"

namespace kmldom {

class Element;
class Serializer;

// OGC KML 2.2 Standard: 9.14 kml:AbstractLatLonAltBox
// OGC KML 2.2 XSD: <complexType name="AbstractLatLonBoxType" abstract="true">
class KML_EXPORT AbstractLatLonBox : public Object {
 public:
  virtual ~AbstractLatLonBox();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <north>
  double get_north() const;
  bool has_north() const;
  void set_north(double north);
  void clear_north();

  // <south>
  double get_south() const;
  bool has_south() const;
  void set_south(double south);
  void clear_south();

  // <east>
  double get_east() const;
  bool has_east() const;
  void set_east(double south);
  void clear_east();

  // <west>
  double get_west() const;
  bool has_west() const;
  void set_west(double south);
  void clear_west();

 protected:
  // Abstract element.  Access for derived types only.
  AbstractLatLonBox();
  virtual void AddElement(const ElementPtr& element);
  virtual void Serialize(Serializer& serializer) const;

 private:
  double north_;
  bool has_north_;
  double south_;
  bool has_south_;
  double east_;
  bool has_east_;
  double west_;
  bool has_west_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(AbstractLatLonBox);
};

}  // end namespace kmldom

#endif  // KML_DOM_ABSTRACTLATLONBOX_H__
