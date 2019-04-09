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

// This file contains the implementation of the <xal:AddressDetails> elements.
// TODO: handle <xs:choice> better.

#include "kml/dom/xal.h"
#include "kml/base/attributes.h"
#include "kml/dom/kml_cast.h"
#include "kml/dom/serializer.h"

using kmlbase::Attributes;

namespace kmldom {

// <xal:AddressDetails>
XalAddressDetails::XalAddressDetails() {
  set_xmlns(kmlbase::XMLNS_XAL);
}

void XalAddressDetails::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }

  switch (element->Type()) {
    case Type_XalCountry:
      set_country(AsXalCountry(element));
      break;
    default:
      Element::AddElement(element);
  }
}

void XalAddressDetails::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  // <xal:Country>
  if (has_country()) {
    serializer.SaveElement(get_country());
  }
}

// <xal:AdministrativeArea>
XalAdministrativeArea::XalAdministrativeArea()
    : has_administrativeareaname_(false) {
  set_xmlns(kmlbase::XMLNS_XAL);
}

void XalAdministrativeArea::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_xalAdministrativeAreaName:
      has_administrativeareaname_ =
          element->SetString(&administrativeareaname_);
      break;
    case Type_XalLocality:
      set_locality(AsXalLocality(element));
      break;
    case Type_XalSubAdministrativeArea:
      set_subadministrativearea(AsXalSubAdministrativeArea(element));
      break;
    default:
      Element::AddElement(element);
  }
}

void XalAdministrativeArea::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  if (has_administrativeareaname()) {
    serializer.SaveFieldById(Type_xalAdministrativeAreaName,
                             get_administrativeareaname());
  }
  if (has_locality()) {
    serializer.SaveElement(get_locality());
  }
  if (has_subadministrativearea()) {
    serializer.SaveElement(get_subadministrativearea());
  }
}

// <xal:Country>
XalCountry::XalCountry() : has_countrynamecode_(false) {
  set_xmlns(kmlbase::XMLNS_XAL);
}

void XalCountry::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_xalCountryNameCode:
      has_countrynamecode_ = element->SetString(&countrynamecode_);
      break;
    case Type_XalAdministrativeArea:
      set_administrativearea(AsXalAdministrativeArea(element));
      break;
    default:
      Element::AddElement(element);
  }
}

void XalCountry::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  // <xal:CountryNameCode>
  if (has_countrynamecode()) {
    serializer.SaveFieldById(Type_xalCountryNameCode, get_countrynamecode());
  }
  // <xal:AdministrativeArea>
  if (has_administrativearea()) {
    serializer.SaveElement(get_administrativearea());
  }
}

// <xal:Locality>
XalLocality::XalLocality() : has_localityname_(false) {
  set_xmlns(kmlbase::XMLNS_XAL);
}

void XalLocality::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_xalLocalityName:
      has_localityname_ = element->SetString(&localityname_);
      break;
    case Type_XalPostalCode:
      set_postalcode(AsXalPostalCode(element));
      break;
    case Type_XalThoroughfare:
      set_thoroughfare(AsXalThoroughfare(element));
      break;
    default:
      Element::AddElement(element);
      break;
  }
}

void XalLocality::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  if (has_thoroughfare()) {
    serializer.SaveElement(get_thoroughfare());
  }
  if (has_postalcode()) {
    serializer.SaveElement(get_postalcode());
  }
}

// <xal:PostalCode>
XalPostalCode::XalPostalCode() : has_postalcodenumber_(false) {
  set_xmlns(kmlbase::XMLNS_XAL);
}

void XalPostalCode::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_xalPostalCodeNumber:
      has_postalcodenumber_ = element->SetString(&postalcodenumber_);
      break;
    default:
      Element::AddElement(element);
      break;
  }
}

void XalPostalCode::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  if (has_postalcodenumber()) {
    serializer.SaveFieldById(Type_xalPostalCodeNumber, get_postalcodenumber());
  }
}

// <xal:SubAdministrativeArea>
XalSubAdministrativeArea::XalSubAdministrativeArea()
    : has_subadministrativeareaname_(false) {
  set_xmlns(kmlbase::XMLNS_XAL);
}

void XalSubAdministrativeArea::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_xalSubAdministrativeAreaName:
      has_subadministrativeareaname_ =
          element->SetString(&subadministrativeareaname_);
      break;
    case Type_XalLocality:
      set_locality(AsXalLocality(element));
      break;
    default:
      Element::AddElement(element);
  }
}

void XalSubAdministrativeArea::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  if (has_subadministrativeareaname()) {
    serializer.SaveFieldById(Type_xalSubAdministrativeAreaName,
                             get_subadministrativeareaname());
  }
  if (has_locality()) {
    serializer.SaveElement(get_locality());
  }
}

// <xal:Thoroughfare>
XalThoroughfare::XalThoroughfare()
    : has_thoroughfarename_(false), has_thoroughfarenumber_(false) {
  set_xmlns(kmlbase::XMLNS_XAL);
}

void XalThoroughfare::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_xalThoroughfareName:
      has_thoroughfarename_ = element->SetString(&thoroughfarename_);
      break;
    case Type_xalThoroughfareNumber:
      has_thoroughfarenumber_ = element->SetString(&thoroughfarenumber_);
      break;
    default:
      Element::AddElement(element);
      break;
  }
}

void XalThoroughfare::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  if (has_thoroughfarenumber()) {
    serializer.SaveFieldById(Type_xalThoroughfareNumber,
                             get_thoroughfarenumber());
  }
  if (has_thoroughfarename()) {
    serializer.SaveFieldById(Type_xalThoroughfareName, get_thoroughfarename());
  }
}

XalAddressDetails::~XalAddressDetails() {
}

const XalCountryPtr& XalAddressDetails::get_country() const {
  return country_;
}

bool XalAddressDetails::has_country() const {
  return country_ != nullptr;
}

void XalAddressDetails::set_country(const XalCountryPtr& country) {
  SetComplexChild(country, &country_);
}

void XalAddressDetails::clear_country() {
  set_country(NULL);
}

XalAdministrativeArea::~XalAdministrativeArea() {
}

const string& XalAdministrativeArea::get_administrativeareaname()
    const {
  return administrativeareaname_;
}

bool XalAdministrativeArea::has_administrativeareaname() const {
  return has_administrativeareaname_;
}

void XalAdministrativeArea::set_administrativeareaname(
    const string& value) {
  administrativeareaname_ = value;
  has_administrativeareaname_ = true;
}

void XalAdministrativeArea::clear_administrativeareaname() {
  administrativeareaname_.clear();
  has_administrativeareaname_ = false;
}

const XalLocalityPtr& XalAdministrativeArea::get_locality() const {
  return locality_;
}

bool XalAdministrativeArea::has_locality() const {
  return locality_ != nullptr;
}

void XalAdministrativeArea::set_locality(const XalLocalityPtr& locality) {
  SetComplexChild(locality, &locality_);
}

void XalAdministrativeArea::clear_locality() {
  set_locality(NULL);
}

const XalSubAdministrativeAreaPtr&
XalAdministrativeArea::get_subadministrativearea() const {
  return subadministrativearea_;
}

bool XalAdministrativeArea::has_subadministrativearea() const {
  return subadministrativearea_ != nullptr;
}

void XalAdministrativeArea::set_subadministrativearea(
    const XalSubAdministrativeAreaPtr& subadministrativearea) {
  SetComplexChild(subadministrativearea, &subadministrativearea_);
}

void XalAdministrativeArea::clear_subadministrativearea() {
  set_subadministrativearea(NULL);
}

XalCountry::~XalCountry() {
}

const string& XalCountry::get_countrynamecode() const {
  return countrynamecode_;
}

bool XalCountry::has_countrynamecode() const {
  return has_countrynamecode_;
}

void XalCountry::set_countrynamecode(const string& value) {
  countrynamecode_ = value;
  has_countrynamecode_ = true;
}

void XalCountry::clear_countrynamecode() {
  countrynamecode_.clear();
  has_countrynamecode_ = false;
}

const XalAdministrativeAreaPtr& XalCountry::get_administrativearea() const {
  return administrativearea_;
}

bool XalCountry::has_administrativearea() const {
  return administrativearea_ != nullptr;
}

void XalCountry::set_administrativearea(
    const XalAdministrativeAreaPtr& administrativearea) {
  SetComplexChild(administrativearea, &administrativearea_);
}

void XalCountry::clear_administrativearea() {
  set_administrativearea(NULL);
}

XalLocality::~XalLocality() {
}

const string& XalLocality::get_localityname() const {
  return localityname_;
}

bool XalLocality::has_localityname() const {
  return has_localityname_;
}

void XalLocality::set_localityname(const string& value) {
  localityname_ = value;
  has_localityname_ = true;
}

void XalLocality::clear_localityname() {
  localityname_.clear();
  has_localityname_ = false;
}

const XalThoroughfarePtr& XalLocality::get_thoroughfare() const {
  return thoroughfare_;
}

bool XalLocality::has_thoroughfare() const {
  return thoroughfare_ != nullptr;
}

void XalLocality::set_thoroughfare(const XalThoroughfarePtr& thoroughfare) {
  SetComplexChild(thoroughfare, &thoroughfare_);
}

void XalLocality::clear_thoroughfare() {
  set_thoroughfare(NULL);
}

const XalPostalCodePtr& XalLocality::get_postalcode() const {
  return postalcode_;
}

bool XalLocality::has_postalcode() const {
  return postalcode_ != nullptr;
}

void XalLocality::set_postalcode(const XalPostalCodePtr& postalcode) {
  SetComplexChild(postalcode, &postalcode_);
}

void XalLocality::clear_postalcode() {
  set_postalcode(NULL);
}

const string& XalPostalCode::get_postalcodenumber() const {
  return postalcodenumber_;
}

bool XalPostalCode::has_postalcodenumber() const {
  return has_postalcodenumber_;
}

void XalPostalCode::set_postalcodenumber(const string& value) {
  postalcodenumber_ = value;
  has_postalcodenumber_ = true;
}

void XalPostalCode::clear_postalcodenumber() {
  postalcodenumber_.clear();
  has_postalcodenumber_ = false;
}

XalSubAdministrativeArea::~XalSubAdministrativeArea() {
}

const string&
XalSubAdministrativeArea::get_subadministrativeareaname() const {
  return subadministrativeareaname_;
}

bool XalSubAdministrativeArea::has_subadministrativeareaname() const {
  return has_subadministrativeareaname_;
}

void XalSubAdministrativeArea::set_subadministrativeareaname(
    const string& value) {
  subadministrativeareaname_ = value;
  has_subadministrativeareaname_ = true;
}

void XalSubAdministrativeArea::clear_subadministrativeareaname() {
  subadministrativeareaname_.clear();
  has_subadministrativeareaname_ = false;
}

const XalLocalityPtr& XalSubAdministrativeArea::get_locality() const {
  return locality_;
}

bool XalSubAdministrativeArea::has_locality() const {
  return locality_ != nullptr;
}

void XalSubAdministrativeArea::set_locality(const XalLocalityPtr& locality) {
  SetComplexChild(locality, &locality_);
}

void XalSubAdministrativeArea::clear_locality() {
  set_locality(NULL);
}

XalThoroughfare::~XalThoroughfare() {
}

const string& XalThoroughfare::get_thoroughfarename() const {
  return thoroughfarename_;
}

bool XalThoroughfare::has_thoroughfarename() const {
  return has_thoroughfarename_;
}

void XalThoroughfare::set_thoroughfarename(const string& value) {
  thoroughfarename_ = value;
  has_thoroughfarename_ = true;
}

void XalThoroughfare::clear_thoroughfarename() {
  thoroughfarename_.clear();
  has_thoroughfarename_ = false;
}

const string& XalThoroughfare::get_thoroughfarenumber() const {
  return thoroughfarenumber_;
}

bool XalThoroughfare::has_thoroughfarenumber() const {
  return has_thoroughfarenumber_;
}

void XalThoroughfare::set_thoroughfarenumber(
    const string& value) {
  thoroughfarenumber_ = value;
  has_thoroughfarenumber_ = true;
}

void XalThoroughfare::clear_thoroughfarenumber() {
  thoroughfarenumber_.clear();
  has_thoroughfarenumber_ = false;
}
}  // end namespace kmldom
