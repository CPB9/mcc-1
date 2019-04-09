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

// This file contains the implementation of the abstract Feature element.

#include "kml/dom/feature.h"
#include "kml/dom/abstractview.h"
#include "kml/dom/atom.h"
#include "kml/dom/extendeddata.h"
#include "kml/dom/kml_cast.h"
#include "kml/dom/region.h"
#include "kml/dom/serializer.h"
#include "kml/dom/snippet.h"
#include "kml/dom/styleselector.h"
#include "kml/dom/timeprimitive.h"
#include "kml/dom/xal.h"

namespace kmldom {

Feature::Feature()
    : has_name_(false),
      visibility_(true),
      has_visibility_(false),
      open_(false),
      has_open_(false),
      has_address_(false),
      has_phonenumber_(false),
      has_description_(false),
      has_styleurl_(false),
      gx_balloonvisibility_(false),
      has_gx_balloonvisibility_(false) {
}

Feature::~Feature() {
}

void Feature::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  // Substitution groups.
  if (element->IsA(Type_AbstractView)) {
    set_abstractview(AsAbstractView(element));
    return;
  }
  if (element->IsA(Type_TimePrimitive)) {
    set_timeprimitive(AsTimePrimitive(element));
    return;
  }
  if (element->IsA(Type_StyleSelector)) {
    set_styleselector(AsStyleSelector(element));
    return;
  }

  // Explicit child elements.
  switch (element->Type()) {
    case Type_name:
      has_name_ = element->SetString(&name_);
      break;
    case Type_visibility:
      has_visibility_ = element->SetBool(&visibility_);
      break;
    case Type_open:
      has_open_ = element->SetBool(&open_);
      break;
    case Type_AtomAuthor:
      set_atomauthor(AsAtomAuthor(element));
      break;
    case Type_AtomLink:
      set_atomlink(AsAtomLink(element));
      break;
    case Type_address:
      has_address_ = element->SetString(&address_);
      break;
    case Type_XalAddressDetails:
      set_xaladdressdetails(AsXalAddressDetails(element));
      break;
    case Type_phoneNumber:
      has_phonenumber_ = element->SetString(&phonenumber_);
      break;
    case Type_Snippet:
      set_snippet(AsSnippet(element));
      break;
// TODO: intentionally do not process <snippet> and let it fall to unknown
#if 0
    case Type_snippet:
      // Recognize (little) <snippet> and save as a big <Snippet>.
      SnippetPtr snippet = KmlFactory::GetFactory()->CreateSnippet();
      snippet->set_text(element->get_char_data());
      set_snippet(snippet);
#endif
      break;
    case Type_description:
      has_description_ = element->SetString(&description_);
      break;
    case Type_styleUrl:
      has_styleurl_ = element->SetString(&styleurl_);
      break;
    case Type_Region:
      set_region(AsRegion(element));
      break;
// TODO: intentionally do not process <Metadata> and let it fall to unknown
#if 0
    case Type_Metadata:
      // Recognize <Metdata> and save into <ExtendedData>.
      break;
#endif
    case Type_ExtendedData:
      set_extendeddata(AsExtendedData(element));
      break;
    case Type_GxBalloonVisibility:
      has_gx_balloonvisibility_ = element->SetBool(&gx_balloonvisibility_);
      break;
    default:
      Object::AddElement(element);
  }
}

void Feature::SerializeBeforeStyleSelector(Serializer& serializer) const {
  if (has_name()) {
    serializer.SaveFieldById(Type_name, name_);
  }
  if (has_visibility()) {
    serializer.SaveFieldById(Type_visibility, visibility_);
  }
  if (has_open()) {
    serializer.SaveFieldById(Type_open, open_);
  }
  if (has_atomauthor()) {
    serializer.SaveElement(get_atomauthor());
  }
  if (has_atomlink()) {
    serializer.SaveElement(get_atomlink());
  }
  if (has_address()) {
    serializer.SaveFieldById(Type_address, get_address());
  }
  if (has_phonenumber()) {
    serializer.SaveFieldById(Type_phoneNumber, get_phonenumber());
  }
  if (has_xaladdressdetails()) {
    serializer.SaveElement(get_xaladdressdetails());
  }
  if (has_snippet()) {
    serializer.SaveElement(get_snippet());
  }
  if (has_description()) {
    serializer.SaveFieldById(Type_description, description_);
  }
  if (has_abstractview()) {
    serializer.SaveElementGroup(get_abstractview(), Type_AbstractView);
  }
  if (has_timeprimitive()) {
    serializer.SaveElementGroup(get_timeprimitive(), Type_TimePrimitive);
  }
  if (has_styleurl()) {
    serializer.SaveFieldById(Type_styleUrl, styleurl_);
  }
}

void Feature::SerializeAfterStyleSelector(Serializer& serializer) const {
  if (has_region()) {
    serializer.SaveElement(get_region());
  }
  if (has_extendeddata()) {
    serializer.SaveElement(get_extendeddata());
  }
  if (has_gx_balloonvisibility()) {
    serializer.SaveFieldById(Type_GxBalloonVisibility, gx_balloonvisibility_);
  }
}

void Feature::Serialize(Serializer& serializer) const {
  Feature::SerializeBeforeStyleSelector(serializer);
  if (has_styleselector()) {
    serializer.SaveElementGroup(get_styleselector(), Type_StyleSelector);
  }
  Feature::SerializeAfterStyleSelector(serializer);
}

void Feature::AcceptChildren(VisitorDriver* driver) {
  Object::AcceptChildren(driver);
  if (has_snippet()) {
    driver->Visit(get_snippet());
  }
  if (has_abstractview()) {
    driver->Visit(get_abstractview());
  }
  if (has_timeprimitive()) {
    driver->Visit(get_timeprimitive());
  }
  if (has_styleselector()) {
    driver->Visit(get_styleselector());
  }
  if (has_region()) {
    driver->Visit(get_region());
  }
  if (has_extendeddata()) {
    driver->Visit(get_extendeddata());
  }
}

kmldom::KmlDomType Feature::Type() const {
  return Type_Feature;
}

bool Feature::IsA(kmldom::KmlDomType type) const {
  return type == Type_Feature || Object::IsA(type);
}

const string& Feature::get_name() const {
  return name_;
}

bool Feature::has_name() const {
  return has_name_;
}

void Feature::set_name(const string& value) {
  name_ = value;
  has_name_ = true;
}

void Feature::clear_name() {
  name_.clear();
  has_name_ = false;
}

bool Feature::get_visibility() const {
  return visibility_;
}

bool Feature::has_visibility() const {
  return has_visibility_;
}

void Feature::set_visibility(bool value) {
  visibility_ = value;
  has_visibility_ = true;
}

void Feature::clear_visibility() {
  visibility_ = true;  // Default <visibility> is true.
  has_visibility_ = false;
}

bool Feature::get_open() const {
  return open_;
}

bool Feature::has_open() const {
  return has_open_;
}

void Feature::set_open(bool value) {
  open_ = value;
  has_open_ = true;
}

void Feature::clear_open() {
  open_ = false;
  has_open_ = false;
}

const AtomAuthorPtr& Feature::get_atomauthor() const {
  return atomauthor_;
}

bool Feature::has_atomauthor() const {
  return atomauthor_ != nullptr;
}

void Feature::set_atomauthor(const AtomAuthorPtr& atomauthor) {
  SetComplexChild(atomauthor, &atomauthor_);
}

void Feature::clear_atomauthor() {
  set_atomauthor(NULL);
}

const AtomLinkPtr& Feature::get_atomlink() const {
  return atomlink_;
}

bool Feature::has_atomlink() const {
  return atomlink_ != nullptr;
}

void Feature::set_atomlink(const AtomLinkPtr& atomlink) {
  SetComplexChild(atomlink, &atomlink_);
}

const string& Feature::get_address() const {
  return address_;
}

bool Feature::has_address() const {
  return has_address_;
}

void Feature::set_address(const string& value) {
  address_ = value;
  has_address_ = true;
}

void Feature::clear_address() {
  address_.clear();
  has_address_ = false;
}

const XalAddressDetailsPtr& Feature::get_xaladdressdetails() const {
  return xaladdressdetails_;
}

bool Feature::has_xaladdressdetails() const {
  return xaladdressdetails_ != nullptr;
}

void Feature::set_xaladdressdetails(
    const XalAddressDetailsPtr& xaladdressdetails) {
  SetComplexChild(xaladdressdetails, &xaladdressdetails_);
}

void Feature::clear_xaladdressdetails() {
  set_xaladdressdetails(NULL);
}

const string& Feature::get_phonenumber() const {
  return phonenumber_;
}

bool Feature::has_phonenumber() const {
  return has_phonenumber_;
}

void Feature::set_phonenumber(const string& value) {
  phonenumber_ = value;
  has_phonenumber_ = true;
}

void Feature::clear_phonenumber() {
  phonenumber_.clear();
  has_phonenumber_ = false;
}

const SnippetPtr& Feature::get_snippet() const {
  return snippet_;
}

bool Feature::has_snippet() const {
  return snippet_ != nullptr;
}

void Feature::set_snippet(const SnippetPtr& snippet) {
  SetComplexChild(snippet, &snippet_);
}

void Feature::clear_snippet() {
  set_snippet(NULL);
}

const string& Feature::get_description() const {
  return description_;
}

bool Feature::has_description() const {
  return has_description_;
}

void Feature::set_description(const string& value) {
  description_ = value;
  has_description_ = true;
}

void Feature::clear_description() {
  description_.clear();
  has_description_ = false;
}

const AbstractViewPtr& Feature::get_abstractview() const {
  return abstractview_;
}

bool Feature::has_abstractview() const {
  return abstractview_ != nullptr;
}

void Feature::set_abstractview(const AbstractViewPtr& abstractview) {
  SetComplexChild(abstractview, &abstractview_);
}

void Feature::clear_abstractview() {
  set_abstractview(NULL);
}

const TimePrimitivePtr& Feature::get_timeprimitive() const {
  return timeprimitive_;
}

bool Feature::has_timeprimitive() const {
  return timeprimitive_ != nullptr;
}

void Feature::set_timeprimitive(const TimePrimitivePtr& timeprimitive) {
  SetComplexChild(timeprimitive, &timeprimitive_);
}

void Feature::clear_timeprimitive() {
  set_timeprimitive(NULL);
}

const string& Feature::get_styleurl() const {
  return styleurl_;
}

string& Feature::styleurl() {
  return styleurl_;
}

bool Feature::has_styleurl() const {
  return has_styleurl_;
}

void Feature::set_styleurl(const string& value) {
  styleurl_ = value;
  has_styleurl_ = true;
}

void Feature::clear_styleurl() {
  styleurl_.clear();
  has_styleurl_ = false;
}

const StyleSelectorPtr& Feature::get_styleselector() const {
  return styleselector_;
}

bool Feature::has_styleselector() const {
  return styleselector_ != nullptr;
}

void Feature::set_styleselector(const StyleSelectorPtr& styleselector) {
  SetComplexChild(styleselector, &styleselector_);
}

void Feature::clear_styleselector() {
  set_styleselector(NULL);
}

const RegionPtr& Feature::get_region() const {
  return region_;
}

bool Feature::has_region() const {
  return region_ != nullptr;
}

void Feature::set_region(const RegionPtr& region) {
  SetComplexChild(region, &region_);
}

void Feature::clear_region() {
  set_region(NULL);
}

const ExtendedDataPtr& Feature::get_extendeddata() const {
  return extendeddata_;
}

bool Feature::has_extendeddata() const {
  return extendeddata_ != nullptr;
}

void Feature::set_extendeddata(const ExtendedDataPtr& extendeddata) {
  SetComplexChild(extendeddata, &extendeddata_);
}

void Feature::clear_extendeddata() {
  set_extendeddata(NULL);
}

bool Feature::get_gx_balloonvisibility() const {
  return gx_balloonvisibility_;
}

bool Feature::has_gx_balloonvisibility() const {
  return has_gx_balloonvisibility_;
}

void Feature::set_gx_balloonvisibility(bool value) {
  gx_balloonvisibility_ = value;
  has_gx_balloonvisibility_ = true;
}

void Feature::clear_gx_balloonvisibility() {
  gx_balloonvisibility_ = false;
  has_gx_balloonvisibility_ = false;
}

void Feature::clear_atomlink() {
  set_atomlink(NULL);
}
}  // namespace kmldom
