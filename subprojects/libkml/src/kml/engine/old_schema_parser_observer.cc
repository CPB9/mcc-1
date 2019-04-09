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

#include "kml/engine/old_schema_parser_observer.h"
#include "kml/dom/container.h"
#include "kml/dom/kml_cast.h"
#include "kml/dom/placemark.h"
#include "kml/engine/parse_old_schema.h"

namespace kmlengine {

OldSchemaParserObserver::OldSchemaParserObserver(
    const SchemaNameMap& schema_name_map)
    : schema_name_map_(schema_name_map) {
}
OldSchemaParserObserver::~OldSchemaParserObserver() {
}

bool OldSchemaParserObserver::AddChild(const kmldom::ElementPtr& parent,
                                       const kmldom::ElementPtr& child) {
  if (kmldom::ContainerPtr container = kmldom::AsContainer(child)) {
    size_t size = container->get_unknown_elements_array_size();
    for (size_t i = 0; i < size; ++i) {
      string errors;
      // TODO: this can fail if the original pass through had CDATA sections
      // in <description>, for example, which were stripped going into the
      // unknown elements array.
      if (kmldom::PlacemarkPtr placemark =
              ParseOldSchema(container->get_unknown_elements_array_at(i),
                             schema_name_map_, &errors)) {
        container->add_feature(placemark);
        // TODO: container->delete_unknown_elements_array_at(i)
      }  // TODO: else terminate the parse and emit the error.
    }
  }
  return true;  // Keep parsing.
}
}  // namespace kmlengine
