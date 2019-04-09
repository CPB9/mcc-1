// Copyright 2009, Google Inc. All rights reserved.
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

#include "kml/convenience/kml_feature_list_saver.h"
#include "kml/convenience/feature_list.h"
#include "kml/dom/feature.h"
#include "kml/dom/kml_cast.h"
#include "kml/dom/styleselector.h"

namespace kmlconvenience {

KmlFeatureListSaver::KmlFeatureListSaver(
    FeatureList* feature_list, kmlengine::SharedStyleMap* shared_style_map,
    const char* style_base)
    : feature_list_(feature_list),
      shared_style_map_(shared_style_map),
      in_update_(false) {
  if (style_base) {
    style_base_ = string(style_base);
  }
}

bool KmlFeatureListSaver::EndElement(const kmldom::ElementPtr& parent,
                                     const kmldom::ElementPtr& child) {
  if (child->Type() == kmldom::Type_Update) {  // </Update>
    in_update_ = false;
    return false;
  }
  if (in_update_) {
    return true;
  }
  if (child->IsA(kmldom::Type_Feature) && !child->IsA(kmldom::Type_Container)) {
    kmldom::FeaturePtr feature = kmldom::AsFeature(child);
    if (!style_base_.empty() && feature->has_styleurl()) {
      const string& styleurl = feature->get_styleurl();
      if (styleurl.size() > 2 && styleurl[0] == '#') {
        feature->set_styleurl(style_base_ + styleurl);
      }
    }
    feature_list_->PushBack(feature);
    return false;
  }
  if (shared_style_map_ && child->IsA(kmldom::Type_StyleSelector) &&
      parent->IsA(kmldom::Type_Document)) {
    const kmldom::StyleSelectorPtr ss = kmldom::AsStyleSelector(child);
    if (ss->has_id()) {
      (*shared_style_map_)[ss->get_id()] = ss;
      return false;
    }
  }
  return true;
}

bool KmlFeatureListSaver::StartElement(const kmldom::ElementPtr& element) {
  if (element->Type() == kmldom::Type_Update) {
    in_update_ = true;
  }
  return true;
}
}  // end namespace kmlconvenience
