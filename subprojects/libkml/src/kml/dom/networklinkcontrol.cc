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

// This file contains the implementation of NetworkLinkControl and its
// children.

#include "kml/dom/networklinkcontrol.h"
#include "kml/base/attributes.h"
#include "kml/base/xml_namespaces.h"
#include "kml/dom/abstractview.h"
#include "kml/dom/container.h"
#include "kml/dom/feature.h"
#include "kml/dom/kml_cast.h"
#include "kml/dom/object.h"
#include "kml/dom/serializer.h"
#include "kml/dom/snippet.h"
#include "kml/dom/visitor.h"

using kmlbase::Attributes;

namespace kmldom {

// UpdateOperation
UpdateOperation::UpdateOperation() {
}

UpdateOperation::~UpdateOperation() {
}

void UpdateOperation::Accept(Visitor* visitor) {
  visitor->VisitUpdateOperation(UpdateOperationPtr(this));
}

// <Create>
Create::Create() {
  set_xmlns(kmlbase::XMLNS_KML22);
}

Create::~Create() {
}

void Create::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->IsA(Type_Container)) {
    add_container(AsContainer(element));
  } else {
    Element::AddElement(element);
  }
}

void Create::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  serializer.SaveElementGroupArray(container_array_, Type_Container);
}

void Create::Accept(Visitor* visitor) {
  visitor->VisitCreate(CreatePtr(this));
}

void Create::AcceptChildren(VisitorDriver* driver) {
  UpdateOperation::AcceptChildren(driver);
  Element::AcceptRepeated<ContainerPtr>(&container_array_, driver);
}

// <Delete>
Delete::Delete() {
  set_xmlns(kmlbase::XMLNS_KML22);
}

Delete::~Delete() {
}

void Delete::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->IsA(Type_Feature)) {
    add_feature(AsFeature(element));
  } else {
    Element::AddElement(element);
  }
}

void Delete::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  serializer.SaveElementGroupArray(feature_array_, Type_Feature);
}

void Delete::Accept(Visitor* visitor) {
  visitor->VisitDelete(DeletePtr(this));
}

void Delete::AcceptChildren(VisitorDriver* driver) {
  UpdateOperation::AcceptChildren(driver);
  Element::AcceptRepeated<FeaturePtr>(&feature_array_, driver);
}

// <Change>
Change::Change() {
  set_xmlns(kmlbase::XMLNS_KML22);
}

Change::~Change() {
}

void Change::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->IsA(Type_Object)) {
    add_object(AsObject(element));
  } else {
    Element::AddElement(element);
  }
}

void Change::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  serializer.SaveElementGroupArray(object_array_, Type_Object);
}

void Change::Accept(Visitor* visitor) {
  visitor->VisitChange(ChangePtr(this));
}

void Change::AcceptChildren(VisitorDriver* driver) {
  UpdateOperation::AcceptChildren(driver);
  Element::AcceptRepeated<ObjectPtr>(&object_array_, driver);
}

// <Update>
Update::Update() : has_targethref_(false) {
  set_xmlns(kmlbase::XMLNS_KML22);
}

Update::~Update() {
}

void Update::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_targetHref:
      has_targethref_ = element->SetString(&targethref_);
      break;
    case Type_Create:
      add_updateoperation(AsCreate(element));
      break;
    case Type_Delete:
      add_updateoperation(AsDelete(element));
      break;
    case Type_Change:
      add_updateoperation(AsChange(element));
      break;
    default:
      Element::AddElement(element);
      break;
  }
}

void Update::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  if (has_targethref()) {
    serializer.SaveFieldById(Type_targetHref, get_targethref());
  }
  for (size_t i = 0; i < updateoperation_array_.size(); ++i) {
    serializer.SaveElement(get_updateoperation_array_at(i));
  }
}

void Update::Accept(Visitor* visitor) {
  visitor->VisitUpdate(UpdatePtr(this));
}

void Update::AcceptChildren(VisitorDriver* driver) {
  Element::AcceptChildren(driver);
  Element::AcceptRepeated<UpdateOperationPtr>(&updateoperation_array_, driver);
}

// <NetworkLinkControl>
NetworkLinkControl::NetworkLinkControl()
    : minrefreshperiod_(0.0),
      has_minrefreshperiod_(false),
      maxsessionlength_(0.0),
      has_maxsessionlength_(false),
      has_cookie_(false),
      has_message_(false),
      has_linkname_(false),
      has_linkdescription_(false),
      linksnippet_(NULL),
      has_expires_(false),
      update_(NULL),
      abstractview_(NULL) {
  set_xmlns(kmlbase::XMLNS_KML22);
}

NetworkLinkControl::~NetworkLinkControl() {
}

void NetworkLinkControl::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->IsA(Type_AbstractView)) {
    set_abstractview(AsAbstractView(element));
    return;
  }
  switch (element->Type()) {
    case Type_minRefreshPeriod:
      has_minrefreshperiod_ = element->SetDouble(&minrefreshperiod_);
      break;
    case Type_maxSessionLength:
      has_maxsessionlength_ = element->SetDouble(&maxsessionlength_);
      break;
    case Type_cookie:
      has_cookie_ = element->SetString(&cookie_);
      break;
    case Type_message:
      has_message_ = element->SetString(&message_);
      break;
    case Type_linkName:
      has_linkname_ = element->SetString(&linkname_);
      break;
    case Type_linkDescription:
      has_linkdescription_ = element->SetString(&linkdescription_);
      break;
    case Type_linkSnippet:
      set_linksnippet(AsLinkSnippet(element));
      break;
    case Type_expires:
      has_expires_ = element->SetString(&expires_);
      break;
    case Type_Update:
      set_update(AsUpdate(element));
      break;
    default:
      Element::AddElement(element);
      break;
  }
}

void NetworkLinkControl::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  if (has_minrefreshperiod_) {
    serializer.SaveFieldById(Type_minRefreshPeriod, minrefreshperiod_);
  }
  if (has_maxsessionlength_) {
    serializer.SaveFieldById(Type_maxSessionLength, maxsessionlength_);
  }
  if (has_cookie_) {
    serializer.SaveFieldById(Type_cookie, cookie_);
  }
  if (has_message_) {
    serializer.SaveFieldById(Type_message, message_);
  }
  if (has_linkname_) {
    serializer.SaveFieldById(Type_linkName, linkname_);
  }
  if (has_linkdescription_) {
    serializer.SaveFieldById(Type_linkDescription, linkdescription_);
  }
  if (linksnippet_) {
    serializer.SaveElement(get_linksnippet());
  }
  if (has_expires_) {
    serializer.SaveFieldById(Type_expires, expires_);
  }
  if (update_) {
    serializer.SaveElement(get_update());
  }
  if (abstractview_) {
    serializer.SaveElementGroup(get_abstractview(), Type_AbstractView);
  }
}

void NetworkLinkControl::Accept(Visitor* visitor) {
  visitor->VisitNetworkLinkControl(NetworkLinkControlPtr(this));
}

void NetworkLinkControl::AcceptChildren(VisitorDriver* driver) {
  Element::AcceptChildren(driver);
  if (has_linksnippet()) {
    driver->Visit(get_linksnippet());
  }
  if (has_update()) {
    driver->Visit(get_update());
  }
  if (has_abstractview()) {
    driver->Visit(get_abstractview());
  }
}

kmldom::KmlDomType Create::Type() const {
  return ElementType();
}

bool Create::IsA(kmldom::KmlDomType type) const {
  return type == ElementType();
}

kmldom::KmlDomType Create::ElementType() {
  return Type_Create;
}

void Create::add_container(const ContainerPtr& container) {
  AddComplexChild(container, &container_array_);
}

size_t Create::get_container_array_size() const {
  return container_array_.size();
}

const ContainerPtr& Create::get_container_array_at(size_t index) const {
  return container_array_[index];
}

kmldom::KmlDomType Delete::Type() const {
  return ElementType();
}

kmldom::KmlDomType Delete::ElementType() {
  return Type_Delete;
}

bool Delete::IsA(kmldom::KmlDomType type) const {
  return type == ElementType();
}

void Delete::add_feature(const FeaturePtr& feature) {
  AddComplexChild(feature, &feature_array_);
}

size_t Delete::get_feature_array_size() const {
  return feature_array_.size();
}

const FeaturePtr& Delete::get_feature_array_at(size_t index) const {
  return feature_array_[index];
}

kmldom::KmlDomType Change::Type() const {
  return ElementType();
}

bool Change::IsA(kmldom::KmlDomType type) const {
  return type == ElementType();
}

kmldom::KmlDomType Change::ElementType() {
  return Type_Change;
}

void Change::add_object(const ObjectPtr& object) {
  AddComplexChild(object, &object_array_);
}

size_t Change::get_object_array_size() const {
  return object_array_.size();
}

const ObjectPtr& Change::get_object_array_at(size_t index) const {
  return object_array_[index];
}

const string& Update::get_targethref() const {
  return targethref_;
}

bool Update::has_targethref() const {
  return has_targethref_;
}

void Update::set_targethref(const string& targethref) {
  targethref_ = targethref;
  has_targethref_ = true;
}

void Update::clear_targethref() {
  targethref_.clear();
  has_targethref_ = false;
}

void Update::add_updateoperation(const UpdateOperationPtr& updateoperation) {
  AddComplexChild(updateoperation, &updateoperation_array_);
}

size_t Update::get_updateoperation_array_size() const {
  return updateoperation_array_.size();
}

const UpdateOperationPtr& Update::get_updateoperation_array_at(
    size_t index) const {
  return updateoperation_array_[index];
}

double NetworkLinkControl::get_minrefreshperiod() const {
  return minrefreshperiod_;
}

bool NetworkLinkControl::has_minrefreshperiod() const {
  return has_minrefreshperiod_;
}

void NetworkLinkControl::set_minrefreshperiod(double value) {
  minrefreshperiod_ = value;
  has_minrefreshperiod_ = true;
}

void NetworkLinkControl::clear_minrefreshperiod() {
  minrefreshperiod_ = 0.0;
  has_minrefreshperiod_ = false;
}

double NetworkLinkControl::get_maxsessionlength() const {
  return maxsessionlength_;
}

bool NetworkLinkControl::has_maxsessionlength() const {
  return has_maxsessionlength_;
}

void NetworkLinkControl::set_maxsessionlength(double value) {
  maxsessionlength_ = value;
  has_maxsessionlength_ = true;
}

void NetworkLinkControl::clear_maxsessionlength() {
  maxsessionlength_ = 0.0;
  has_maxsessionlength_ = false;
}

const string& NetworkLinkControl::get_cookie() const {
  return cookie_;
}

bool NetworkLinkControl::has_cookie() const {
  return has_cookie_;
}

void NetworkLinkControl::set_cookie(const string& cookie) {
  cookie_ = cookie;
  has_cookie_ = true;
}

void NetworkLinkControl::clear_cookie() {
  cookie_.clear();
  has_cookie_ = false;
}

const string& NetworkLinkControl::get_message() const {
  return message_;
}

bool NetworkLinkControl::has_message() const {
  return has_message_;
}

void NetworkLinkControl::set_message(const string& message) {
  message_ = message;
  has_message_ = true;
}

void NetworkLinkControl::clear_message() {
  message_.clear();
  has_message_ = false;
}

const string& NetworkLinkControl::get_linkname() const {
  return linkname_;
}

bool NetworkLinkControl::has_linkname() const {
  return has_linkname_;
}

void NetworkLinkControl::set_linkname(const string& linkname) {
  linkname_ = linkname;
  has_linkname_ = true;
}

void NetworkLinkControl::clear_linkname() {
  linkname_.clear();
  has_linkname_ = false;
}

const string& NetworkLinkControl::get_linkdescription() const {
  return linkdescription_;
}

bool NetworkLinkControl::has_linkdescription() const {
  return has_linkdescription_;
}

void NetworkLinkControl::set_linkdescription(
    const string& linkdescription) {
  linkdescription_ = linkdescription;
  has_linkdescription_ = true;
}

void NetworkLinkControl::clear_linkdescription() {
  linkdescription_.clear();
  has_linkdescription_ = false;
}

const LinkSnippetPtr& NetworkLinkControl::get_linksnippet() const {
  return linksnippet_;
}

bool NetworkLinkControl::has_linksnippet() const {
  return linksnippet_ != nullptr;
}

void NetworkLinkControl::set_linksnippet(LinkSnippetPtr linksnippet) {
  SetComplexChild(linksnippet, &linksnippet_);
}

void NetworkLinkControl::clear_linksnippet() {
  set_linksnippet(NULL);
}

const string& NetworkLinkControl::get_expires() const {
  return expires_;
}

bool NetworkLinkControl::has_expires() const {
  return has_expires_;
}

void NetworkLinkControl::set_expires(const string& expires) {
  expires_ = expires;
  has_expires_ = true;
}

void NetworkLinkControl::clear_expires() {
  expires_.clear();
  has_expires_ = false;
}

const UpdatePtr& NetworkLinkControl::get_update() const {
  return update_;
}

bool NetworkLinkControl::has_update() const {
  return update_ != nullptr;
}

void NetworkLinkControl::set_update(const UpdatePtr& update) {
  SetComplexChild(update, &update_);
}

void NetworkLinkControl::clear_update() {
  set_update(NULL);
}

const AbstractViewPtr& NetworkLinkControl::get_abstractview() const {
  return abstractview_;
}

bool NetworkLinkControl::has_abstractview() const {
  return abstractview_ != nullptr;
}

void NetworkLinkControl::set_abstractview(const AbstractViewPtr& abstractview) {
  SetComplexChild(abstractview, &abstractview_);
}

void NetworkLinkControl::clear_abstractview() {
  set_abstractview(NULL);
}
}  // end namespace kmldom
