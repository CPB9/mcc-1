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

#include "kml/dom/stats_serializer.h"

namespace kmldom {

void StatsSerializer::BeginById(int type_id,
                                const kmlbase::Attributes& attributes) {
  ++begin_count_;
}

void StatsSerializer::End() {
  ++end_count_;
}

void StatsSerializer::SaveStringFieldById(int type_id,
                                          string value) {
  ++field_count_;
}

void StatsSerializer::SaveContent(const string& content,
                                  bool maybe_quote) {
  ++content_count_;
}

void StatsSerializer::SaveElement(const ElementPtr& element) {
  ++element_count_;
  Serializer::SaveElement(element);
}

void StatsSerializer::SaveElementGroup(const ElementPtr& element,
                                       int group_id) {
  ++element_group_count_;
  SaveElement(element);  // To count elements and recurse.
}

int StatsSerializer::get_begin_count() const {
  return begin_count_;
}

int StatsSerializer::get_end_count() const {
  return end_count_;
}

int StatsSerializer::get_field_count() const {
  return field_count_;
}

int StatsSerializer::get_element_count() const {
  return element_count_;
}

int StatsSerializer::get_element_group_count() const {
  return element_group_count_;
}

StatsSerializer::StatsSerializer()
    : begin_count_(0),
      end_count_(0),
      field_count_(0),
      element_count_(0),
      element_group_count_(0),
      content_count_(0) {
}
}  // end namespace kmldom
