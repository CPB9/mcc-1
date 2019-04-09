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

// This file contains the definition of the SharedStyleParserObserver class.

#ifndef KML_ENGINE_SHARED_STYLE_PARSER_OBSERVER_H__
#define KML_ENGINE_SHARED_STYLE_PARSER_OBSERVER_H__

#include <map>
#include <string>
#include "kml/config.h"
#include "kml/dom/kml_ptr.h"
#include "kml/dom/parser_observer.h"
#include "kml/engine/engine_types.h"

namespace kmlengine {

// The SharedStyleParserObserver is a kmldom::ParserObserver which gathers all
// all shared StyleSelectors into the supplied SharedStyleMap.  If strict_parse
// is true AddChild() returns false on any id collision causing the parse to
// terminate when this is used with kmldom::Parse::AddObserver().  If
// strict_parse is false the "last one wins" on any duplicate.
// TODO: provide an error log for this latter case.
class KML_EXPORT SharedStyleParserObserver : public kmldom::ParserObserver {
 public:
  // A SharedStyleMap must be supplied.
  SharedStyleParserObserver(SharedStyleMap* shared_style_map,
                            bool strict_parse);

  virtual ~SharedStyleParserObserver();

  // ParserObserver::AddChild()
  virtual bool AddChild(const kmldom::ElementPtr& parent,
                        const kmldom::ElementPtr& child);

 private:
  SharedStyleMap* shared_style_map_;
  bool strict_parse_;
};

}  // end namespace kmlengine

#endif  // KML_ENGINE_SHARED_STYLE_PARSER_OBSERVER_H__
