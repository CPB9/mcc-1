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

// This file contains the declaration of the BalloonStyle element.

#ifndef KML_DOM_BALLOONSTYLE_H__
#define KML_DOM_BALLOONSTYLE_H__

#include "kml/base/color32.h"
#include "kml/config.h"
#include "kml/dom/kml22.h"
#include "kml/dom/substyle.h"

namespace kmldom {

class Visitor;

class KML_EXPORT BalloonStyle : public SubStyle {
 public:
  virtual ~BalloonStyle();
  virtual KmlDomType Type() const;
  virtual bool IsA(KmlDomType type) const;

  // <bgColor>
  const kmlbase::Color32& get_bgcolor() const;
  bool has_bgcolor() const;
  void set_bgcolor(const kmlbase::Color32& bgcolor);
  void clear_bgcolor();

  // <textColor>
  const kmlbase::Color32& get_textcolor() const;
  bool has_textcolor() const;
  void set_textcolor(const kmlbase::Color32& textcolor);
  void clear_textcolor();

  // <text>
  const string& get_text() const;
  bool has_text() const;
  void set_text(const string& text);
  void clear_text();

  // <displayMode>
  int get_displaymode() const;
  bool has_displaymode() const;
  void set_displaymode(int displaymode);
  void clear_displaymode();

  // Visitor API methods, see visitor.h.
  virtual void Accept(Visitor* visitor);

 private:
  friend class KmlFactory;
  BalloonStyle();
  friend class KmlHandler;
  virtual void AddElement(const ElementPtr& element);
  friend class Serializer;
  virtual void Serialize(Serializer& serialize) const;
  kmlbase::Color32 bgcolor_;
  bool has_bgcolor_;
  kmlbase::Color32 textcolor_;
  bool has_textcolor_;
  string text_;
  bool has_text_;
  int displaymode_;
  bool has_displaymode_;
  LIBKML_DISALLOW_EVIL_CONSTRUCTORS(BalloonStyle);
};

}  // end namespace kmldom

#endif  // KML_DOM_BALLOONSTYLE_H__
