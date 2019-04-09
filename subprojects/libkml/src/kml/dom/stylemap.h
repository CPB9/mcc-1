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

// This file contains the declarations of the Pair and StyleMap elements.

#ifndef KML_DOM_STYLEMAP_H__
#define KML_DOM_STYLEMAP_H__

#include <vector>
#include "kml/dom/kml22.h"
#include "kml/dom/kml_ptr.h"
#include "kml/dom/object.h"
#include "kml/dom/styleselector.h"

namespace kmldom {

class Serializer;
class Visitor;
class VisitorDriver;

// <Pair>
class KML_EXPORT Pair : public Object {
 public:
  virtual ~Pair();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <key>
  int get_key() const;
  bool has_key() const;
  void set_key(int key);
  void clear_key();

  // <styleUrl>
  const string& get_styleurl() const;
  bool has_styleurl() const;
  void set_styleurl(const string& styleurl);
  void clear_styleurl();

  // StyleSelector
  const StyleSelectorPtr& get_styleselector() const;
  bool has_styleselector() const;
  void set_styleselector(const StyleSelectorPtr& styleselector);
  void clear_styleselector();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  Pair();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  int key_;
  bool has_key_;
  string styleurl_;
  bool has_styleurl_;
  StyleSelectorPtr styleselector_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(Pair);
};

// <StyleMap>
class KML_EXPORT StyleMap : public StyleSelector {
 public:
  virtual ~StyleMap();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  void add_pair(const PairPtr& pair);

  size_t get_pair_array_size() const;

  const PairPtr& get_pair_array_at(size_t index) const;

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);
  virtual void AcceptChildren(VisitorDriver* driver);

 private:
  friend class KmlFactory;
  StyleMap();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serializer) const;
  std::vector<PairPtr> pair_array_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(StyleMap);
};

}  // end namespace kmldom

#endif  // KML_DOM_STYLEMAP_H__
