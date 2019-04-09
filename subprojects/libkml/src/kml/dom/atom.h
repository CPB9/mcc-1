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

// This file contains the declaration of Atom elements used in KML.
// See: http://atompub.org/rfc4287.html.

#ifndef KML_DOM_ATOM_H__
#define KML_DOM_ATOM_H__

#include "kml/config.h"
#include "kml/dom/element.h"

namespace kmldom {

// <atom:author>, RFC 4287 4.2.1, and 3.2 (atomPersonConstruct)
class KML_EXPORT AtomAuthor : public BasicElement<Type_AtomAuthor> {
 public:
  virtual ~AtomAuthor();

  // <atom:name>
  const string& get_name() const;
  bool has_name() const;
  void set_name(const string& value);
  void clear_name();

  // <atom:uri>, RFC 3987
  const string& get_uri() const;
  bool has_uri() const;
  void set_uri(const string& value);
  void clear_uri();

  // <atom:email>, RFC 2822
  const string& get_email() const;
  bool has_email() const;
  void set_email(const string& value);
  void clear_email();

 private:
  bool has_name_;
  string name_;
  bool has_uri_;
  string uri_;
  bool has_email_;
  string email_;
  friend class KmlFactory;
  AtomAuthor();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(AtomAuthor);
};

// Elements common to <atom:feed> and <atom:entry>.
class KML_EXPORT AtomCommon : public Element {
 public:
  // <atom:id>
  const string& get_id() const {
    return id_;
  }
  bool has_id() const {
    return has_id_;
  }
  void set_id(const string& value);
  void clear_id();

  // <atom:title>
  const string& get_title() const;
  bool has_title() const;
  void set_title(const string& value);
  void clear_title();

  // <atom:updated>
  const string& get_updated() const;
  bool has_updated() const;
  void set_updated(const string& value);
  void clear_updated();

  // <atom:category>...
  void add_category(const AtomCategoryPtr& entry);
  size_t get_category_array_size() const;
  const AtomCategoryPtr& get_category_array_at(size_t index) const;

  // <atom:link>...
  void add_link(const AtomLinkPtr& entry);
  size_t get_link_array_size() const;
  const AtomLinkPtr& get_link_array_at(size_t index) const;

 protected:
  AtomCommon();
  void AddElement(const ElementPtr& element);
  virtual void Serialize(Serializer& serializer) const;

 private:
  friend class KmlFactory;
  friend class KmlHandler;
  friend class Serializer;
  bool has_id_;
  string id_;
  bool has_title_;
  string title_;
  bool has_updated_;
  string updated_;
  std::vector<AtomCategoryPtr> category_array_;
  std::vector<AtomLinkPtr> link_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(AtomCommon);
};

// <atom:category scheme="..." term="..." label=..."/>, RFC 4287 4.2.2
// NOTE: This element is not part of the OGC KML 2.2 standard.
class KML_EXPORT AtomCategory : public BasicElement<Type_AtomCategory> {
 public:
  virtual ~AtomCategory();

  // term=
  const string& get_term() const;
  bool has_term() const;
  void set_term(const string& value);
  void clear_term();

  // scheme=
  const string& get_scheme() const;
  bool has_scheme() const;
  void set_scheme(const string& value);
  void clear_scheme();

  // label=
  const string& get_label() const;
  bool has_label() const;
  void set_label(const string& value);
  void clear_label();

 private:
  friend class KmlFactory;
  AtomCategory();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  virtual void ParseAttributes(kmlbase::Attributes* attributes);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  virtual void SerializeAttributes(kmlbase::Attributes* attributes) const;
  bool has_term_;
  string term_;
  bool has_scheme_;
  string scheme_;
  bool has_label_;
  string label_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(AtomCategory);
};

// <atom:content src="..."  type="...">, RFC 4287 4.1.3
// NOTE: This element is not part of the OGC KML 2.2 standard.
class KML_EXPORT AtomContent : public BasicElement<Type_AtomContent> {
 public:
  virtual ~AtomContent();

  // src=
  const string& get_src() const;
  bool has_src() const;
  void set_src(const string& value);
  void clear_src();

  // type=
  const string& get_type() const;
  bool has_type() const;
  void set_type(const string& value);
  void clear_type();

 private:
  friend class KmlFactory;
  AtomContent();
  friend class KmlHandler;
  void ParseAttributes(kmlbase::Attributes* attributes);
  void SerializeAttributes(kmlbase::Attributes* attributes) const;
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  bool has_src_;
  string src_;
  bool has_type_;
  string type_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(AtomContent);
};

// <atom:entry>, RFC 4287 4.1.2
// NOTE: This element is not part of the OGC KML 2.2 standard.
class KML_EXPORT AtomEntry : public AtomCommon {
 public:
  virtual ~AtomEntry();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;
  // This static method makes the class useable with ElementCast.
  static KmlDomType ElementType();

  // <atom:summary>
  const string& get_summary() const;
  bool has_summary() const;
  void set_summary(const string& value);
  void clear_summary();

  // <atom:content>
  const AtomContentPtr& get_content() const;
  bool has_content() const;
  void set_content(const AtomContentPtr& content);
  void clear_content();

 private:
  friend class KmlFactory;
  AtomEntry();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  bool has_summary_;
  string summary_;
  AtomContentPtr content_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(AtomEntry);
};

// <atom:feed>, RFC 4287 4.1.1
// NOTE: This element is not part of the OGC KML 2.2 standard.
class KML_EXPORT AtomFeed : public AtomCommon {
 public:
  virtual ~AtomFeed();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;
  // This static method makes the class useable with ElementCast.
  static KmlDomType ElementType();

  // <atom:entry>...
  void add_entry(const AtomEntryPtr& entry);
  size_t get_entry_array_size() const;
  const AtomEntryPtr& get_entry_array_at(size_t index) const;

 private:
  friend class KmlFactory;
  AtomFeed();
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  std::vector<AtomEntryPtr> entry_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(AtomFeed);
};

// <atom:link>, RFC 4287 4.2.7
class KML_EXPORT AtomLink : public BasicElement<Type_AtomLink> {
 public:
  virtual ~AtomLink();

  // href=, RFC 4287 4.2.7.1, RFC 3987
  const string& get_href() const;
  bool has_href() const;
  void set_href(const string& value);
  void clear_href();

  // rel=, RFC 4287 4.2.7.2, RFC 3987
  const string& get_rel() const;
  bool has_rel() const;
  void set_rel(const string& value);
  void clear_rel();

  // type=, RFC 4287 4.2.7.3, MIME
  const string& get_type() const;
  bool has_type() const;
  void set_type(const string& value);
  void clear_type();

  // hreflang=, RFC 4287 4.2.7.4, RFC 3066
  const string& get_hreflang() const;
  bool has_hreflang() const;
  void set_hreflang(const string& value);
  void clear_hreflang();

  // title=, RFC 4287 4.2.7.5
  const string& get_title() const;
  bool has_title() const;
  void set_title(const string& value);
  void clear_title();

  // length=, RFC 4287 4.2.7.6
  int get_length() const;
  bool has_length() const;
  void set_length(const int value);
  void clear_length();

 private:
  bool has_href_;
  string href_;
  bool has_rel_;
  string rel_;
  bool has_type_;
  string type_;
  bool has_hreflang_;
  string hreflang_;
  bool has_title_;
  string title_;
  bool has_length_;
  int length_;
  friend class KmlFactory;
  AtomLink();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  void ParseAttributes(kmlbase::Attributes* attributes);
  void SerializeAttributes(kmlbase::Attributes* attributes) const;
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(AtomLink);
};

}  // end namespace kmldom

#endif  // KML_DOM_ATOM_H__
