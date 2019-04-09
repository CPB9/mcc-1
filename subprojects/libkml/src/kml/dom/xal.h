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

// This file contains the declaration of the <xal:AddressDetails> elements.
// Note, only a subset of XAL using these elements is implemented here.
// However, note that the normal unknown/misplaced element handling of libkml
// is employed thus all of XAL is preserved on parse and emitted on
// serialization.  The portion implemented here pertains to programmatic
// dom access.
//
// xAL complex elements:
// <xal:AddressDetails>
// <xal:AdministrativeArea>
// <xal:Country>
// <xal:Locality>
// <xal:PostalCode>
// <xal:SubAdministrativeArea>
// <xal:Thoroughfare>
//
// xAL simple elements:
// <xal:AdministrativeAreaName>
// <xal:CountryNameCode>
// <xal:LocalityName>
// <xal:PostalCodeNumber>
// <xal:SubAdministrativeAreaName>
// <xal:ThoroughfareName>
// <xal:ThoroughfareNumber>

#ifndef KML_DOM_XAL_H__
#define KML_DOM_XAL_H__

#include "kml/dom/element.h"

namespace kmldom {

// <xal:AddressDetails>
class KML_EXPORT XalAddressDetails
    : public BasicElement<Type_XalAddressDetails> {
 public:
  virtual ~XalAddressDetails();

  // <xal:Country>
  const XalCountryPtr& get_country() const;
  bool has_country() const;
  void set_country(const XalCountryPtr& country);
  void clear_country();

 private:
  XalAddressDetails();
  XalCountryPtr country_;
  friend class KmlFactory;
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
};

// <xal:AdministrativeArea>
class KML_EXPORT XalAdministrativeArea
    : public BasicElement<Type_XalAdministrativeArea> {
 public:
  virtual ~XalAdministrativeArea();

  // <xal:AdministrativeAreaName>
  const string& get_administrativeareaname() const;
  bool has_administrativeareaname() const;
  void set_administrativeareaname(const string& value);
  void clear_administrativeareaname();

  // <xal:Locality>
  const XalLocalityPtr& get_locality() const;
  bool has_locality() const;
  void set_locality(const XalLocalityPtr& locality);

  void clear_locality();
  // <xal:SubAdministrativeArea>
  const XalSubAdministrativeAreaPtr& get_subadministrativearea() const;
  bool has_subadministrativearea() const;
  void set_subadministrativearea(
      const XalSubAdministrativeAreaPtr& subadministrativearea);
  void clear_subadministrativearea();

 private:
  XalAdministrativeArea();
  bool has_administrativeareaname_;
  string administrativeareaname_;
  XalLocalityPtr locality_;
  XalSubAdministrativeAreaPtr subadministrativearea_;
  friend class KmlFactory;
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
};

// <xal:Country>
class KML_EXPORT XalCountry : public BasicElement<Type_XalCountry> {
 public:
  virtual ~XalCountry();

  // <xal:CountryNameCode>, ISO 3166-1
  const string& get_countrynamecode() const;
  bool has_countrynamecode() const;
  void set_countrynamecode(const string& value);
  void clear_countrynamecode();

  // <xal:AdministrativeArea>
  const XalAdministrativeAreaPtr& get_administrativearea() const;
  bool has_administrativearea() const;
  void set_administrativearea(
      const XalAdministrativeAreaPtr& administrativearea);
  void clear_administrativearea();

 private:
  XalCountry();
  bool has_countrynamecode_;
  string countrynamecode_;
  XalAdministrativeAreaPtr administrativearea_;
  friend class KmlFactory;
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(XalCountry);
};

// <xal:Locality>
class KML_EXPORT XalLocality : public BasicElement<Type_XalLocality> {
 public:
  virtual ~XalLocality();

  // <xal:LocalityName>
  const string& get_localityname() const;
  bool has_localityname() const;
  void set_localityname(const string& value);
  void clear_localityname();

  // <xal:Thoroughfare>
  const XalThoroughfarePtr& get_thoroughfare() const;
  bool has_thoroughfare() const;
  void set_thoroughfare(const XalThoroughfarePtr& thoroughfare);
  void clear_thoroughfare();

  // <xal:PostalCode>
  const XalPostalCodePtr& get_postalcode() const;
  bool has_postalcode() const;
  void set_postalcode(const XalPostalCodePtr& postalcode);
  void clear_postalcode();

 private:
  XalLocality();
  bool has_localityname_;
  string localityname_;
  XalThoroughfarePtr thoroughfare_;
  XalPostalCodePtr postalcode_;
  friend class KmlFactory;
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
};

// <xal:PostalCode>
class KML_EXPORT XalPostalCode : public BasicElement<Type_XalPostalCode> {
 public:
  virtual ~XalPostalCode() {
  }

  // <xal:PostalCodeNumber>
  const string& get_postalcodenumber() const;
  bool has_postalcodenumber() const;
  void set_postalcodenumber(const string& value);
  void clear_postalcodenumber();

 private:
  XalPostalCode();
  bool has_postalcodenumber_;
  string postalcodenumber_;
  friend class KmlFactory;
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
};

// <xal:SubAdministrativeArea>
class KML_EXPORT XalSubAdministrativeArea
    : public BasicElement<Type_XalSubAdministrativeArea> {
 public:
  virtual ~XalSubAdministrativeArea();

  // <xal:SubAdministrativeAreaName>
  const string& get_subadministrativeareaname() const;
  bool has_subadministrativeareaname() const;
  void set_subadministrativeareaname(const string& value);
  void clear_subadministrativeareaname();

  // <xal:Locality>
  const XalLocalityPtr& get_locality() const;
  bool has_locality() const;
  void set_locality(const XalLocalityPtr& locality);
  void clear_locality();

 private:
  XalSubAdministrativeArea();
  bool has_subadministrativeareaname_;
  string subadministrativeareaname_;
  XalLocalityPtr locality_;
  friend class KmlFactory;
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
};

// <xal:Thoroughfare>
class KML_EXPORT XalThoroughfare : public BasicElement<Type_XalThoroughfare> {
 public:
  virtual ~XalThoroughfare();

  // <xal:ThoroughfareName>
  const string& get_thoroughfarename() const;
  bool has_thoroughfarename() const;
  void set_thoroughfarename(const string& value);
  void clear_thoroughfarename();

  // <xal:ThoroughfareNumber>
  const string& get_thoroughfarenumber() const;
  bool has_thoroughfarenumber() const;
  void set_thoroughfarenumber(const string& value);
  void clear_thoroughfarenumber();

 private:
  XalThoroughfare();
  bool has_thoroughfarename_;
  string thoroughfarename_;
  bool has_thoroughfarenumber_;
  string thoroughfarenumber_;
  friend class KmlFactory;
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
};

}  // end namespace kmldom

#endif  // KML_DOM_XAL_H__
