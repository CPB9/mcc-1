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

// This file contains the declaration of the abstract Feature element.

#ifndef KML_DOM_FEATURE_H__
#define KML_DOM_FEATURE_H__

#include "kml/base/util.h"
#include "kml/config.h"
#include "kml/dom/kml22.h"
#include "kml/dom/kml_ptr.h"
#include "kml/dom/object.h"

namespace kmldom {

class VisitorDriver;

// OGC KML 2.2 Standard: 9.1 kml:AbstractFeatureGroup
// OGC KML 2.2 XSD: <element name="AbstractFeatureGroup"...
class KML_EXPORT Feature : public Object {
 public:
  virtual ~Feature();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <name>
  const string& get_name() const;
  bool has_name() const;
  void set_name(const string& value);
  void clear_name();

  // <visibility>
  bool get_visibility() const;
  bool has_visibility() const;
  void set_visibility(bool value);
  void clear_visibility();

  // <open>
  bool get_open() const;
  bool has_open() const;
  void set_open(bool value);
  void clear_open();

  // <atom:author>
  const AtomAuthorPtr& get_atomauthor() const;
  bool has_atomauthor() const;
  void set_atomauthor(const AtomAuthorPtr& atomauthor);
  void clear_atomauthor();

  // <atom:link>
  const AtomLinkPtr& get_atomlink() const;
  bool has_atomlink() const;
  void set_atomlink(const AtomLinkPtr& atomlink);
  void clear_atomlink();

  // <address>
  const string& get_address() const;
  bool has_address() const;
  void set_address(const string& value);
  void clear_address();

  // <xal:AddressDetails>
  const XalAddressDetailsPtr& get_xaladdressdetails() const;
  bool has_xaladdressdetails() const;
  void set_xaladdressdetails(const XalAddressDetailsPtr& xaladdressdetails);
  void clear_xaladdressdetails();

  // <phoneNumber>
  const string& get_phonenumber() const;
  bool has_phonenumber() const;
  void set_phonenumber(const string& value);
  void clear_phonenumber();

  // TODO: "little" <snippet> (presently preserved as a misplaced child)
  // <Snippet>
  const SnippetPtr& get_snippet() const;
  bool has_snippet() const;
  void set_snippet(const SnippetPtr& snippet);
  void clear_snippet();

  // <description>
  const string& get_description() const;
  bool has_description() const;
  void set_description(const string& value);
  void clear_description();

  // AbstractView
  const AbstractViewPtr& get_abstractview() const;
  bool has_abstractview() const;
  void set_abstractview(const AbstractViewPtr& abstractview);
  void clear_abstractview();

  // TimePrimitive
  const TimePrimitivePtr& get_timeprimitive() const;
  bool has_timeprimitive() const;
  void set_timeprimitive(const TimePrimitivePtr& timeprimitive);
  void clear_timeprimitive();

  // <styleUrl>
  const string& get_styleurl() const;
  string& styleurl();
  bool has_styleurl() const;
  void set_styleurl(const string& value);
  void clear_styleurl();

  // StyleSelector
  const StyleSelectorPtr& get_styleselector() const;
  bool has_styleselector() const;
  void set_styleselector(const StyleSelectorPtr& styleselector);
  void clear_styleselector();

  // <Region>
  const RegionPtr& get_region() const;
  bool has_region() const;
  void set_region(const RegionPtr& region);
  void clear_region();

  // TODO: <Metadata> (presently preserved as a misplaced child)
  // <ExtendedData>
  const ExtendedDataPtr& get_extendeddata() const;
  bool has_extendeddata() const;
  void set_extendeddata(const ExtendedDataPtr& extendeddata);
  void clear_extendeddata();

  // From kml:AbstractFeatureSimpleExtensionGroup.

  // <gx:balloonVisibility>
  bool get_gx_balloonvisibility() const;
  bool has_gx_balloonvisibility() const;
  void set_gx_balloonvisibility(bool value);
  void clear_gx_balloonvisibility();

  // Visitor API methods, see visitor.h.
  virtual void AcceptChildren(VisitorDriver* driver);

 protected:
  // Feature is abstract.
  Feature();
  virtual void AddElement(const ElementPtr& element);
  void SerializeBeforeStyleSelector(Serializer& serialize) const;
  void SerializeAfterStyleSelector(Serializer& serialize) const;
  virtual void Serialize(Serializer& serialize) const;

 private:
  string name_;
  bool has_name_;
  bool visibility_;
  bool has_visibility_;
  bool open_;
  bool has_open_;
  AtomAuthorPtr atomauthor_;
  AtomLinkPtr atomlink_;
  string address_;
  bool has_address_;
  XalAddressDetailsPtr xaladdressdetails_;
  string phonenumber_;
  bool has_phonenumber_;
  SnippetPtr snippet_;
  string description_;
  bool has_description_;
  AbstractViewPtr abstractview_;
  TimePrimitivePtr timeprimitive_;
  string styleurl_;
  bool has_styleurl_;
  StyleSelectorPtr styleselector_;
  RegionPtr region_;
  ExtendedDataPtr extendeddata_;
  bool gx_balloonvisibility_;
  bool has_gx_balloonvisibility_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Feature);
};

}  // namespace kmldom

#endif  // KML_DOM_FEATURE_H__
