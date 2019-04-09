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

#ifndef KML_DOM_ICONSTYLE_H__
#define KML_DOM_ICONSTYLE_H__

#include "kml/base/util.h"
#include "kml/config.h"
#include "kml/dom/colorstyle.h"
#include "kml/dom/kml22.h"
#include "kml/dom/kml_ptr.h"

namespace kmldom {

class Visitor;
class VisitorDriver;

// <IconStyle>
class KML_EXPORT IconStyle : public ColorStyle {
 public:
  virtual ~IconStyle();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <scale>
  double get_scale() const;
  bool has_scale() const;
  void set_scale(double scale);
  void clear_scale();

  // <heading>
  double get_heading() const;
  bool has_heading() const;
  void set_heading(double heading);
  void clear_heading();

  // <Icon> (different than Overlay Icon)
  const IconStyleIconPtr& get_icon() const;
  bool has_icon() const;
  void set_icon(const IconStyleIconPtr& icon);
  void clear_icon();

  // <hotSpot>
  const HotSpotPtr& get_hotspot() const;
  bool has_hotspot() const;
  void set_hotspot(const HotSpotPtr& hotspot);
  void clear_hotspot();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  IconStyle();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serialize) const;
  double scale_;
  bool has_scale_;
  double heading_;
  bool has_heading_;
  IconStyleIconPtr icon_;
  HotSpotPtr hotspot_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(IconStyle);
};

}  // end namespace kmldom

#endif  // KML_DOM_ICONSTYLE_H__
