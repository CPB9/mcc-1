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

#ifndef KML_DOM_STYLE_H__
#define KML_DOM_STYLE_H__

#include "kml/config.h"
#include "kml/dom/kml22.h"
#include "kml/dom/kml_ptr.h"

#include "kml/dom/styleselector.h"

namespace kmldom {

class Serializer;
class Visitor;
class VisitorDriver;

class KML_EXPORT Style : public StyleSelector {
 public:
  virtual ~Style();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <IconStyle>
  const IconStylePtr& get_iconstyle() const;
  bool has_iconstyle() const;
  void set_iconstyle(const IconStylePtr& iconstyle);
  void clear_iconstyle();

  // <LabelStyle>
  const LabelStylePtr& get_labelstyle() const;
  bool has_labelstyle() const;
  void set_labelstyle(const LabelStylePtr& labelstyle);
  void clear_labelstyle();

  // <LineStyle>
  const LineStylePtr& get_linestyle() const;
  bool has_linestyle() const;
  void set_linestyle(const LineStylePtr& linestyle);
  void clear_linestyle();

  // <PolyStyle>
  const PolyStylePtr& get_polystyle() const;
  bool has_polystyle() const;
  void set_polystyle(const PolyStylePtr& polystyle);
  void clear_polystyle();

  // <BalloonStyle>
  const BalloonStylePtr& get_balloonstyle() const;
  bool has_balloonstyle() const;
  void set_balloonstyle(const BalloonStylePtr& balloonstyle);
  void clear_balloonstyle();

  // <ListStyle>
  const ListStylePtr& get_liststyle() const;
  bool has_liststyle() const;
  void set_liststyle(const ListStylePtr& liststyle);
  void clear_liststyle();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  Style();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  IconStylePtr iconstyle_;
  LabelStylePtr labelstyle_;
  LineStylePtr linestyle_;
  PolyStylePtr polystyle_;
  BalloonStylePtr balloonstyle_;
  ListStylePtr liststyle_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Style);
};

}  // end namespace kmldom

#endif  // KML_DOM_STYLE_H__
