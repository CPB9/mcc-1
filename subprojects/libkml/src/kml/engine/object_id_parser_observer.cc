
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

#include "kml/engine/object_id_parser_observer.h"
#include "kml/dom/kml_cast.h"
#include "kml/dom/object.h"

namespace kmlengine {

ObjectIdParserObserver::ObjectIdParserObserver(ObjectIdMap* object_id_map,
                                               bool strict_parsing)
    : object_id_map_(object_id_map), strict_parse_(strict_parsing) {
}  // TODO: NULL check, or use reference
ObjectIdParserObserver::~ObjectIdParserObserver() {
}

bool ObjectIdParserObserver::NewElement(const kmldom::ElementPtr& element) {
  if (kmldom::ObjectPtr object = kmldom::AsObject(element)) {
    if (object->has_id()) {
      if (object_id_map_->find(object->get_id()) != object_id_map_->end() &&
          strict_parse_) {
        // TODO: create an error message
        return false;  // Duplicate id, fail parse.
      }
      (*object_id_map_)[object->get_id()] = object;  // Last one wins.
    }
  }
  // Not a duplicate id, or strict parsing not enabled, keep parsing.
  return true;
}
}  // namespace kmlengine
