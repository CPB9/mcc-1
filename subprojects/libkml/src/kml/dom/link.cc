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
//

#include "kml/dom/link.h"
#include <cstring>
#include "kml/base/attributes.h"
#include "kml/dom/serializer.h"
#include "kml/dom/visitor.h"

using kmlbase::Attributes;

namespace kmldom {

BasicLink::BasicLink() : has_href_(false) {
}

BasicLink::~BasicLink() {
}

// TODO: fix CDATA parsing in general.
static const char* kCdataOpen = "<![CDATA[";

static bool SetStringInsideCdata(ElementPtr element, const string& char_data,
                                 string* val) {
  if (!element) {
    return false;
  }
  string::size_type offset = strlen(kCdataOpen);
  if (char_data.compare(0, offset, kCdataOpen, offset) == 0) {
    *val = char_data.substr(offset, char_data.size() - offset - 3);
    return true;
  }
  return element->SetString(val);
}

void IconStyleIcon::clear_gx_h() {
  gx_h_ = 0.0;
  has_gx_h_ = false;
}

void IconStyleIcon::set_gx_h(const double h) {
  gx_h_ = h;
  has_gx_h_ = true;
}

bool IconStyleIcon::has_gx_h() const {
  return has_gx_h_;
}

double IconStyleIcon::get_gx_h() const {
  return gx_h_;
}

void IconStyleIcon::clear_gx_w() {
  gx_w_ = 0.0;
  has_gx_w_ = false;
}

void IconStyleIcon::set_gx_w(const double w) {
  gx_w_ = w;
  has_gx_w_ = true;
}

bool IconStyleIcon::has_gx_w() const {
  return has_gx_w_;
}

double IconStyleIcon::get_gx_w() const {
  return gx_w_;
}

void IconStyleIcon::clear_gx_y() {
  gx_y_ = 0.0;
  has_gx_y_ = false;
}

void IconStyleIcon::set_gx_y(const double y) {
  gx_y_ = y;
  has_gx_y_ = true;
}

bool IconStyleIcon::has_gx_y() const {
  return has_gx_y_;
}

double IconStyleIcon::get_gx_y() const {
  return gx_y_;
}

void IconStyleIcon::clear_gx_x() {
  gx_x_ = 0.0;
  has_gx_x_ = false;
}

void IconStyleIcon::set_gx_x(const double x) {
  gx_x_ = x;
  has_gx_x_ = true;
}

bool IconStyleIcon::has_gx_x() const {
  return has_gx_x_;
}

double IconStyleIcon::get_gx_x() const {
  return gx_x_;
}

bool IconStyleIcon::IsA(kmldom::KmlDomType type) const {
  return type == Type_IconStyleIcon || BasicLink::IsA(type);
}

kmldom::KmlDomType IconStyleIcon::Type() const {
  return Type_IconStyleIcon;
}

bool Url::IsA(kmldom::KmlDomType type) const {
  return type == Type_Url || AbstractLink::IsA(type);
}

kmldom::KmlDomType Url::Type() const {
  return Type_Url;
}

bool Icon::IsA(kmldom::KmlDomType type) const {
  return type == Type_Icon || AbstractLink::IsA(type);
}

kmldom::KmlDomType Icon::Type() const {
  return Type_Icon;
}

bool Link::IsA(kmldom::KmlDomType type) const {
  return type == Type_Link || AbstractLink::IsA(type);
}

kmldom::KmlDomType Link::Type() const {
  return Type_Link;
}

void AbstractLink::clear_httpquery() {
  httpquery_.clear();
  has_httpquery_ = false;
}

void AbstractLink::set_httpquery(const string& httpquery) {
  httpquery_ = httpquery;
  has_httpquery_ = true;
}

bool AbstractLink::has_httpquery() const {
  return has_httpquery_;
}

const string& AbstractLink::get_httpquery() const {
  return httpquery_;
}

void AbstractLink::clear_viewformat() {
  viewformat_.clear();
  has_viewformat_ = false;
}

void AbstractLink::set_viewformat(const string& viewformat) {
  viewformat_ = viewformat;
  has_viewformat_ = true;
}

bool AbstractLink::has_viewformat() const {
  return has_viewformat_;
}

const string& AbstractLink::get_viewformat() const {
  return viewformat_;
}

void AbstractLink::clear_viewboundscale() {
  viewboundscale_ = 1.0;
  has_viewboundscale_ = false;
}

void AbstractLink::set_viewboundscale(const double viewboundscale) {
  viewboundscale_ = viewboundscale;
  has_viewboundscale_ = true;
}

bool AbstractLink::has_viewboundscale() const {
  return has_viewboundscale_;
}

double AbstractLink::get_viewboundscale() const {
  return viewboundscale_;
}

void AbstractLink::clear_viewrefreshtime() {
  viewrefreshtime_ = 4.0;
  has_viewrefreshtime_ = false;
}

void AbstractLink::set_viewrefreshtime(const double viewrefreshtime) {
  viewrefreshtime_ = viewrefreshtime;
  has_viewrefreshtime_ = true;
}

bool AbstractLink::has_viewrefreshtime() const {
  return has_viewrefreshtime_;
}

double AbstractLink::get_viewrefreshtime() const {
  return viewrefreshtime_;
}

void AbstractLink::clear_viewrefreshmode() {
  viewrefreshmode_ = VIEWREFRESHMODE_NEVER;
  has_viewrefreshmode_ = false;
}

void AbstractLink::set_viewrefreshmode(const int viewrefreshmode) {
  viewrefreshmode_ = viewrefreshmode;
  has_viewrefreshmode_ = true;
}

bool AbstractLink::has_viewrefreshmode() const {
  return has_viewrefreshmode_;
}

int AbstractLink::get_viewrefreshmode() const {
  return viewrefreshmode_;
}

void AbstractLink::clear_refreshinterval() {
  refreshinterval_ = 4.0;
  has_refreshinterval_ = false;
}

void AbstractLink::set_refreshinterval(const double refreshinterval) {
  refreshinterval_ = refreshinterval;
  has_refreshinterval_ = true;
}

bool AbstractLink::has_refreshinterval() const {
  return has_refreshinterval_;
}

double AbstractLink::get_refreshinterval() const {
  return refreshinterval_;
}

void AbstractLink::clear_refreshmode() {
  refreshmode_ = REFRESHMODE_ONCHANGE;
  has_refreshmode_ = false;
}

void AbstractLink::set_refreshmode(const int refreshmode) {
  refreshmode_ = refreshmode;
  has_refreshmode_ = true;
}

bool AbstractLink::has_refreshmode() const {
  return has_refreshmode_;
}

int AbstractLink::get_refreshmode() const {
  return refreshmode_;
}

void BasicLink::clear_href() {
  href_.clear();
  has_href_ = false;
}

void BasicLink::set_href(const string& href) {
  href_ = href;
  has_href_ = true;
}

bool BasicLink::has_href() const {
  return has_href_;
}

const string& BasicLink::get_href() const {
  return href_;
}

bool BasicLink::IsA(kmldom::KmlDomType type) const {
  return type == Type_BasicLink || Object::IsA(type);
}

kmldom::KmlDomType BasicLink::Type() const {
  return Type_BasicLink;
}

void BasicLink::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_href:
      // TODO: use a generalized approach
      // has_href_ = element->SetString(&href_);
      has_href_ =
          SetStringInsideCdata(element, element->get_char_data(), &href_);
      break;
    default:
      Object::AddElement(element);
      break;
  }
}

void BasicLink::Serialize(Serializer& serializer) const {
  Object::Serialize(serializer);
  if (has_href()) {
    serializer.SaveFieldById(Type_href, get_href());
  }
}

void BasicLink::Accept(Visitor* visitor) {
  visitor->VisitBasicLink(BasicLinkPtr(this));
}

// Construct with defaults as per KML standard.
AbstractLink::AbstractLink()
    : refreshmode_(REFRESHMODE_ONCHANGE),
      has_refreshmode_(false),
      refreshinterval_(4.0),
      has_refreshinterval_(false),
      viewrefreshmode_(VIEWREFRESHMODE_NEVER),
      has_viewrefreshmode_(false),
      viewrefreshtime_(4.0),
      has_viewrefreshtime_(false),
      viewboundscale_(1.0),
      has_viewboundscale_(false),
      has_viewformat_(false),
      has_httpquery_(false) {
}

AbstractLink::~AbstractLink() {
}

void AbstractLink::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_refreshMode:
      has_refreshmode_ = element->SetEnum(&refreshmode_);
      break;
    case Type_refreshInterval:
      has_refreshinterval_ = element->SetDouble(&refreshinterval_);
      break;
    case Type_viewRefreshMode:
      has_viewrefreshmode_ = element->SetEnum(&viewrefreshmode_);
      break;
    case Type_viewRefreshTime:
      has_viewrefreshtime_ = element->SetDouble(&viewrefreshtime_);
      break;
    case Type_viewBoundScale:
      has_viewboundscale_ = element->SetDouble(&viewboundscale_);
      break;
    case Type_viewFormat:
      has_viewformat_ = element->SetString(&viewformat_);
      break;
    case Type_httpQuery:
      has_httpquery_ = element->SetString(&httpquery_);
      break;
    default:
      BasicLink::AddElement(element);
      break;
  }
}

void AbstractLink::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  BasicLink::Serialize(serializer);
  if (has_refreshmode()) {
    serializer.SaveEnum(Type_refreshMode, get_refreshmode());
  }
  if (has_refreshinterval()) {
    serializer.SaveFieldById(Type_refreshInterval, get_refreshinterval());
  }
  if (has_viewrefreshmode()) {
    serializer.SaveEnum(Type_viewRefreshMode, get_viewrefreshmode());
  }
  if (has_viewrefreshtime()) {
    serializer.SaveFieldById(Type_viewRefreshTime, get_viewrefreshtime());
  }
  if (has_viewboundscale()) {
    serializer.SaveFieldById(Type_viewBoundScale, get_viewboundscale());
  }
  if (has_viewformat()) {
    serializer.SaveFieldById(Type_viewFormat, get_viewformat());
  }
  if (has_httpquery()) {
    serializer.SaveFieldById(Type_httpQuery, get_httpquery());
  }
}

Link::Link() {
}

Link::~Link() {
}

void Link::Accept(Visitor* visitor) {
  visitor->VisitLink(LinkPtr(this));
}

Icon::Icon() {
}

Icon::~Icon() {
}

void Icon::Accept(Visitor* visitor) {
  visitor->VisitIcon(IconPtr(this));
}

Url::Url() {
}

Url::~Url() {
}

void Url::Accept(Visitor* visitor) {
  visitor->VisitUrl(UrlPtr(this));
}

IconStyleIcon::IconStyleIcon()
    : gx_x_(0.0),
      has_gx_x_(false),
      gx_y_(0.0),
      has_gx_y_(false),
      gx_w_(0.0),
      has_gx_w_(false),
      gx_h_(0.0),
      has_gx_h_(false) {
}

IconStyleIcon::~IconStyleIcon() {
}

void IconStyleIcon::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_GxX:
      has_gx_x_ = element->SetDouble(&gx_x_);
      break;
    case Type_GxY:
      has_gx_y_ = element->SetDouble(&gx_y_);
      break;
    case Type_GxW:
      has_gx_w_ = element->SetDouble(&gx_w_);
      break;
    case Type_GxH:
      has_gx_h_ = element->SetDouble(&gx_h_);
      break;
    default:
      BasicLink::AddElement(element);
  }
}

void IconStyleIcon::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  BasicLink::Serialize(serializer);
  if (has_gx_x_) {
    serializer.SaveFieldById(Type_GxX, gx_x_);
  }
  if (has_gx_y_) {
    serializer.SaveFieldById(Type_GxY, gx_y_);
  }
  if (has_gx_w_) {
    serializer.SaveFieldById(Type_GxW, gx_w_);
  }
  if (has_gx_h_) {
    serializer.SaveFieldById(Type_GxH, gx_h_);
  }
}

void IconStyleIcon::Accept(Visitor* visitor) {
  visitor->VisitIconStyleIcon(IconStyleIconPtr(this));
}

}  // end namespace kmldom
