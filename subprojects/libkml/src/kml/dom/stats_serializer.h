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

// This file contains the definition of the StatsSerializer test utility.
// TODO: some ilk of unit test for this class despite this _being_ part of
// some unit tests.

#ifndef KML_DOM_STATS_SERIALIZER_H__
#define KML_DOM_STATS_SERIALIZER_H__

#include "kml/config.h"
#include "kml/dom/kml_ptr.h"
#include "kml/dom/serializer.h"

namespace kmlbase {
class Attributes;
}

namespace kmldom {

// This Serializer implementation counts begin and end tags of complex elements
// and a count of all simple elements (fields).
class KML_EXPORT StatsSerializer : public Serializer {
 public:
  StatsSerializer();
  virtual void BeginById(int type_id, const kmlbase::Attributes& attributes);
  virtual void End();
  virtual void SaveStringFieldById(int type_id, string value);
  virtual void SaveContent(const string& content, bool maybe_quote);
  virtual void SaveElement(const ElementPtr& element);
  virtual void SaveElementGroup(const ElementPtr& element, int group_id);
  int get_begin_count() const;
  int get_end_count() const;
  int get_field_count() const;
  int get_element_count() const;
  int get_element_group_count() const;

 private:
  int begin_count_;
  int end_count_;
  int field_count_;
  int element_count_;
  int element_group_count_;
  int content_count_;
};

}  // end namespace kmldom

#endif  // KML_DOM_STATS_SERIALIZER_H__
