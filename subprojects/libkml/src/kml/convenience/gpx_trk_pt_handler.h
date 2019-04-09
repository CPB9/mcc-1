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

// This file contains the declaration of the GpxTrkPtHandler class.

#ifndef KML_CONVENIENCE_GPX_TRK_PT_HANDLER_H__
#define KML_CONVENIENCE_GPX_TRK_PT_HANDLER_H__

#include <cstring>  // strcmp
#include <memory>
#include "kml/base/expat_handler.h"
#include "kml/config.h"

namespace kmlbase {
class Vec3;
}

namespace kmlconvenience {

// Find all <trkpt>'s in the GPX file.
// For example:
// <trkpt lat="-33.911973070" lon="18.422974152">
//   <ele>4.943848</ele>
//   <time>2008-10-11T14:55:41Z</time>
// </trkpt>
// Each <trkpt> results in a call to HandlePoint().
// Overall usage: Derive a class from GpxTrkPtHandler with an implementation of
// HandlePoint().
class KML_EXPORT GpxTrkPtHandler : public kmlbase::ExpatHandler {
 public:
  // ExpatHandler::StartElement()
  virtual void StartElement(const string& name,
                            const std::vector<string>& atts);

  // ExpatHandler::EndElement()
  virtual void EndElement(const string& name);

  // ExpatHandler::CharData()
  virtual void CharData(const string& str);

  // This is called for each <trkpt>.  This default implemenation does nothing.
  virtual void HandlePoint(const kmlbase::Vec3& where, const string& when);
  ;

 private:
  // A fresh Vec3 is created for each <trkpt>.
  std::unique_ptr<kmlbase::Vec3> vec3_;
  string time_;
  bool gather_char_data_;
  string char_data_;
};

}  // end namespace kmlconvenience

#endif  // KML_CONVENIENCE_GPX_TRK_PT_HANDLER_H__
