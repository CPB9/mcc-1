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

// This file contains the implementation of the Atom elements.

#include "kml/dom/atom.h"
#include "kml/base/attributes.h"
#include "kml/dom/kml_cast.h"
#include "kml/dom/serializer.h"

using kmlbase::Attributes;

namespace kmldom {

// Attributes.
static const char kHref[] = "href";
static const char kHrefLang[] = "hreflang";
static const char kLabel[] = "label";
static const char kLength[] = "length";
static const char kRel[] = "rel";
static const char kScheme[] = "scheme";
static const char kSrc[] = "src";
static const char kTerm[] = "term";
static const char kTitle[] = "title";
static const char kType[] = "type";

// <atom:author>
AtomAuthor::AtomAuthor()
    : has_name_(false), has_uri_(false), has_email_(false) {
  set_xmlns(kmlbase::XMLNS_ATOM);
}

AtomAuthor::~AtomAuthor() {
}

void AtomAuthor::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }

  // Explicit child elements.
  switch (element->Type()) {
    case Type_atomEmail:
      has_email_ = element->SetString(&email_);
      break;
    case Type_atomName:
      has_name_ = element->SetString(&name_);
      break;
    case Type_atomUri:
      has_uri_ = element->SetString(&uri_);
      break;
    default:
      Element::AddElement(element);
  }
}

void AtomAuthor::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  // In order of http://schemas.opengis.net/kml/2.2.0/atom-author-link.xsd
  // although no order is specified (this is not an XSD sequence, for example).
  if (has_name()) {
    serializer.SaveFieldById(Type_atomName, get_name());
  }
  if (has_uri()) {
    serializer.SaveFieldById(Type_atomUri, get_uri());
  }
  if (has_email()) {
    serializer.SaveFieldById(Type_atomEmail, get_email());
  }
}

// <atom:category>
AtomCategory::AtomCategory()
    : has_term_(false), has_scheme_(false), has_label_(false) {
  set_xmlns(kmlbase::XMLNS_ATOM);
}

AtomCategory::~AtomCategory() {
}

void AtomCategory::AddElement(const ElementPtr& element) {
  // Any element passed in here is unknown.
  Element::AddElement(element);
}

void AtomCategory::ParseAttributes(Attributes* attributes) {
  if (!attributes) {
    return;
  }
  has_term_ = attributes->CutValue(kTerm, &term_);
  has_scheme_ = attributes->CutValue(kScheme, &scheme_);
  has_label_ = attributes->CutValue(kLabel, &label_);
  AddUnknownAttributes(attributes);
}

void AtomCategory::SerializeAttributes(Attributes* attributes) const {
  Element::SerializeAttributes(attributes);
  if (has_scheme()) {
    attributes->SetValue(kScheme, get_scheme());
  }
  if (has_term()) {
    attributes->SetValue(kTerm, get_term());
  }
  if (has_label()) {
    attributes->SetValue(kLabel, get_label());
  }
}

void AtomCategory::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
}

// <atom:content>
AtomContent::AtomContent() : has_src_(false), has_type_(false) {
  set_xmlns(kmlbase::XMLNS_ATOM);
}

AtomContent::~AtomContent() {
}

void AtomContent::ParseAttributes(Attributes* attributes) {
  if (!attributes) {
    return;
  }
  has_src_ = attributes->CutValue(kSrc, &src_);
  has_type_ = attributes->CutValue(kType, &type_);
  AddUnknownAttributes(attributes);
}

void AtomContent::SerializeAttributes(Attributes* attributes) const {
  Element::SerializeAttributes(attributes);
  if (has_src()) {
    attributes->SetValue(kSrc, get_src());
  }
  if (has_type()) {
    attributes->SetValue(kType, get_type());
  }
}

void AtomContent::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
}

// Common children of <atom:feed> and <atom:entry>.
AtomCommon::AtomCommon()
    : has_id_(false), has_title_(false), has_updated_(false) {
}

void AtomCommon::add_category(const AtomCategoryPtr& category) {
  AddComplexChild(category, &category_array_);
}

void AtomCommon::add_link(const AtomLinkPtr& link) {
  AddComplexChild(link, &link_array_);
}

void AtomCommon::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }

  // Explicit child elements.
  switch (element->Type()) {
    case Type_atomId:
      has_id_ = element->SetString(&id_);
      break;
    case Type_atomTitle:
      has_title_ = element->SetString(&title_);
      break;
    case Type_atomUpdated:
      has_updated_ = element->SetString(&updated_);
      break;
    case Type_AtomCategory:
      add_category(AsAtomCategory(element));
      break;
    case Type_AtomLink:
      add_link(AsAtomLink(element));
      break;
    default:
      Element::AddElement(element);
  }
}

void AtomCommon::Serialize(Serializer& serializer) const {
  Element::Serialize(serializer);
  if (has_id()) {
    serializer.SaveFieldById(Type_atomId, get_id());
  }
  if (has_title()) {
    serializer.SaveFieldById(Type_atomTitle, get_title());
  }
  if (has_updated()) {
    serializer.SaveFieldById(Type_atomUpdated, get_updated());
  }
  serializer.SaveElementArray(category_array_);
  serializer.SaveElementArray(link_array_);
}

// <atom:entry>
AtomEntry::AtomEntry() : has_summary_(false) {
  set_xmlns(kmlbase::XMLNS_ATOM);
}

AtomEntry::~AtomEntry() {
}

void AtomEntry::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_atomSummary:
      has_summary_ = element->SetString(&summary_);
      break;
    case Type_AtomContent:
      set_content(AsAtomContent(element));
      break;
    default:
      AtomCommon::AddElement(element);
  }
}

void AtomEntry::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  AtomCommon::Serialize(serializer);
  if (has_summary()) {
    serializer.SaveFieldById(Type_atomSummary, get_summary());
  }
  if (has_content()) {
    serializer.SaveElement(get_content());
  }
}

// <atom:feed>
AtomFeed::AtomFeed() {
  set_xmlns(kmlbase::XMLNS_ATOM);
}

AtomFeed::~AtomFeed() {
}

void AtomFeed::add_entry(const AtomEntryPtr& atom_entry) {
  AddComplexChild(atom_entry, &entry_array_);
}

void AtomFeed::AddElement(const ElementPtr& element) {
  if (AtomEntryPtr entry = AsAtomEntry(element)) {
    add_entry(entry);
  } else {
    AtomCommon::AddElement(element);
  }
}

void AtomFeed::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  AtomCommon::Serialize(serializer);
  serializer.SaveElementArray(entry_array_);
}

// <atom:link>
AtomLink::AtomLink()
    : has_href_(false),
      has_rel_(false),
      has_type_(false),
      has_hreflang_(false),
      has_title_(false),
      has_length_(false),
      length_(0) {
  set_xmlns(kmlbase::XMLNS_ATOM);
}

AtomLink::~AtomLink() {
}

void AtomLink::AddElement(const ElementPtr& element) {
  // Any element passed in here is by definition unknown, or "undefinedContent"
  // in the atom standard.
  Element::AddElement(element);
}

void AtomLink::ParseAttributes(Attributes* attributes) {
  if (!attributes) {
    return;
  }
  has_href_ = attributes->CutValue(kHref, &href_);
  has_rel_ = attributes->CutValue(kRel, &rel_);
  has_type_ = attributes->CutValue(kType, &type_);
  has_hreflang_ = attributes->CutValue(kHrefLang, &hreflang_);
  has_title_ = attributes->CutValue(kTitle, &title_);
  has_length_ = attributes->CutValue(kLength, &length_);
  AddUnknownAttributes(attributes);
}

void AtomLink::SerializeAttributes(Attributes* attributes) const {
  Element::SerializeAttributes(attributes);
  if (has_href()) {
    attributes->SetValue(kHref, get_href());
  }
  if (has_rel()) {
    attributes->SetValue(kRel, get_rel());
  }
  if (has_type()) {
    attributes->SetValue(kType, get_type());
  }
  if (has_hreflang()) {
    attributes->SetValue(kHrefLang, get_hreflang());
  }
  if (has_title()) {
    attributes->SetValue(kTitle, get_title());
  }
  if (has_length()) {
    attributes->SetValue(kLength, get_length());
  }
}

void AtomLink::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
}

const string& AtomAuthor::get_name() const {
  return name_;
}

bool AtomAuthor::has_name() const {
  return has_name_;
}

void AtomAuthor::set_name(const string& value) {
  name_ = value;
  has_name_ = true;
}

void AtomAuthor::clear_name() {
  name_.clear();
  has_name_ = false;
}

const string& AtomAuthor::get_uri() const {
  return uri_;
}

bool AtomAuthor::has_uri() const {
  return has_uri_;
}

void AtomAuthor::set_uri(const string& value) {
  uri_ = value;
  has_uri_ = true;
}

const string& AtomAuthor::get_email() const {
  return email_;
}

bool AtomAuthor::has_email() const {
  return has_email_;
}

void AtomAuthor::set_email(const string& value) {
  email_ = value;
  has_email_ = true;
}

void AtomAuthor::clear_email() {
  email_.clear();
  has_email_ = false;
}

void AtomAuthor::clear_uri() {
  uri_.clear();
  has_uri_ = false;
}

void AtomCommon::set_id(const string& value) {
  id_ = value;
  has_id_ = true;
}

void AtomCommon::clear_id() {
  id_.clear();
  has_id_ = false;
}

const string& AtomCommon::get_title() const {
  return title_;
}

bool AtomCommon::has_title() const {
  return has_title_;
}

void AtomCommon::set_title(const string& value) {
  title_ = value;
  has_title_ = true;
}

void AtomCommon::clear_title() {
  title_.clear();
  has_title_ = false;
}

const string& AtomCommon::get_updated() const {
  return updated_;
}

bool AtomCommon::has_updated() const {
  return has_updated_;
}

void AtomCommon::set_updated(const string& value) {
  updated_ = value;
  has_updated_ = true;
}

void AtomCommon::clear_updated() {
  updated_.clear();
  has_updated_ = false;
}

size_t AtomCommon::get_category_array_size() const {
  return category_array_.size();
}

const AtomCategoryPtr& AtomCommon::get_category_array_at(size_t index) const {
  return category_array_[index];
}

size_t AtomCommon::get_link_array_size() const {
  return link_array_.size();
}

const AtomLinkPtr& AtomCommon::get_link_array_at(size_t index) const {
  return link_array_[index];
}

const string& AtomCategory::get_term() const {
  return term_;
}

bool AtomCategory::has_term() const {
  return has_term_;
}

void AtomCategory::set_term(const string& value) {
  term_ = value;
  has_term_ = true;
}

void AtomCategory::clear_term() {
  term_.clear();
  has_term_ = false;
}

const string& AtomCategory::get_scheme() const {
  return scheme_;
}

bool AtomCategory::has_scheme() const {
  return has_scheme_;
}

void AtomCategory::set_scheme(const string& value) {
  scheme_ = value;
  has_scheme_ = true;
}

void AtomCategory::clear_scheme() {
  scheme_.clear();
  has_scheme_ = false;
}

const string& AtomCategory::get_label() const {
  return label_;
}

bool AtomCategory::has_label() const {
  return has_label_;
}

void AtomCategory::set_label(const string& value) {
  label_ = value;
  has_label_ = true;
}

void AtomCategory::clear_label() {
  label_.clear();
  has_label_ = false;
}

const string& AtomContent::get_src() const {
  return src_;
}

bool AtomContent::has_src() const {
  return has_src_;
}

void AtomContent::set_src(const string& value) {
  src_ = value;
  has_src_ = true;
}

void AtomContent::clear_src() {
  src_.clear();
  has_src_ = false;
}

const string& AtomContent::get_type() const {
  return type_;
}

bool AtomContent::has_type() const {
  return has_type_;
}

void AtomContent::set_type(const string& value) {
  type_ = value;
  has_type_ = true;
}

void AtomContent::clear_type() {
  type_.clear();
  has_type_ = false;
}

kmldom::KmlDomType AtomEntry::Type() const {
  return Type_AtomEntry;
}

bool AtomEntry::IsA(kmldom::KmlDomType type) const {
  return type == Type_AtomEntry;
}

kmldom::KmlDomType AtomEntry::ElementType() {
  return static_cast<KmlDomType>(Type_AtomEntry);
}

const string& AtomEntry::get_summary() const {
  return summary_;
}

bool AtomEntry::has_summary() const {
  return has_summary_;
}

void AtomEntry::set_summary(const string& value) {
  summary_ = value;
  has_summary_ = true;
}

void AtomEntry::clear_summary() {
  summary_.clear();
  has_summary_ = false;
}

const AtomContentPtr& AtomEntry::get_content() const {
  return content_;
}

bool AtomEntry::has_content() const {
  return content_ != nullptr;
}

void AtomEntry::set_content(const AtomContentPtr& content) {
  SetComplexChild(content, &content_);
}

void AtomEntry::clear_content() {
  set_content(NULL);
}

kmldom::KmlDomType AtomFeed::Type() const {
  return Type_AtomFeed;
}

bool AtomFeed::IsA(kmldom::KmlDomType type) const {
  return type == Type_AtomFeed;
}

kmldom::KmlDomType AtomFeed::ElementType() {
  return static_cast<KmlDomType>(Type_AtomFeed);
}

size_t AtomFeed::get_entry_array_size() const {
  return entry_array_.size();
}

const AtomEntryPtr& AtomFeed::get_entry_array_at(size_t index) const {
  return entry_array_[index];
}

const string& AtomLink::get_href() const {
  return href_;
}

bool AtomLink::has_href() const {
  return has_href_;
}

void AtomLink::set_href(const string& value) {
  href_ = value;
  has_href_ = true;
}

void AtomLink::clear_href() {
  href_.clear();
  has_href_ = false;
}

const string& AtomLink::get_rel() const {
  return rel_;
}

bool AtomLink::has_rel() const {
  return has_rel_;
}

void AtomLink::set_rel(const string& value) {
  rel_ = value;
  has_rel_ = true;
}

void AtomLink::clear_rel() {
  rel_.clear();
  has_rel_ = false;
}

const string& AtomLink::get_type() const {
  return type_;
}

bool AtomLink::has_type() const {
  return has_type_;
}

void AtomLink::set_type(const string& value) {
  type_ = value;
  has_type_ = true;
}

void AtomLink::clear_type() {
  type_.clear();
  has_type_ = false;
}

const string& AtomLink::get_hreflang() const {
  return hreflang_;
}

bool AtomLink::has_hreflang() const {
  return has_hreflang_;
}

void AtomLink::set_hreflang(const string& value) {
  hreflang_ = value;
  has_hreflang_ = true;
}

void AtomLink::clear_hreflang() {
  hreflang_.clear();
  has_hreflang_ = false;
}

const string& AtomLink::get_title() const {
  return title_;
}

bool AtomLink::has_title() const {
  return has_title_;
}

void AtomLink::set_title(const string& value) {
  title_ = value;
  has_title_ = true;
}

void AtomLink::clear_title() {
  title_.clear();
  has_title_ = false;
}

int AtomLink::get_length() const {
  return length_;
}

bool AtomLink::has_length() const {
  return has_length_;
}

void AtomLink::set_length(const int value) {
  length_ = value;
  has_length_ = true;
}

void AtomLink::clear_length() {
  length_ = 0;
  has_length_ = false;
}
}  // end namespace kmldom
