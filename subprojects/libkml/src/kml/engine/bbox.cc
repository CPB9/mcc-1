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

#include "kml/engine/bbox.h"

namespace kmlengine {

const double kMinLat = -180.0;
const double kMaxLat = 180.0;
const double kMinLon = -180.0;
const double kMaxLon = 180.0;

Bbox::Bbox()
    : north_(kMinLat), south_(kMaxLat), east_(kMinLon), west_(kMaxLon) {
}

Bbox::Bbox(double north, double south, double east, double west)
    : north_(north), south_(south), east_(east), west_(west) {
}

void Bbox::AlignBbox(kmlengine::Bbox* qt, unsigned int max_depth) {
  if (!qt) {
    return;
  }
  double lat = qt->GetCenterLat();
  double lon = qt->GetCenterLon();
  if (ContainedByBox(qt->get_north(), lat, qt->get_east(), lon)) {
    qt->set_south(lat);
    qt->set_west(lon);
  } else if (ContainedByBox(qt->get_north(), lat, lon, qt->get_west())) {
    qt->set_south(lat);
    qt->set_east(lon);
  } else if (ContainedByBox(lat, qt->get_south(), qt->get_east(), lon)) {
    qt->set_north(lat);
    qt->set_west(lon);
  } else if (ContainedByBox(lat, qt->get_south(), lon, qt->get_west())) {
    qt->set_north(lat);
    qt->set_east(lon);
  } else {
    return;  // target not contained by any child quadrant of qt.
  }
  // Fall through from above and recurse.
  if (max_depth > 0) {
    AlignBbox(qt, max_depth - 1);
  }
}

bool Bbox::ContainedByBbox(const kmlengine::Bbox& b) const {
  return ContainedByBox(b.get_north(), b.get_south(), b.get_east(),
                        b.get_west());
}

bool Bbox::ContainedByBox(double north, double south, double east,
                          double west) const {
  return north >= north_ && south <= south_ && east >= east_ && west <= west_;
}

bool Bbox::Contains(double latitude, double longitude) const {
  return north_ >= latitude && south_ <= latitude && east_ >= longitude &&
         west_ <= longitude;
}

void Bbox::ExpandFromBbox(const kmlengine::Bbox& bbox) {
  ExpandLatitude(bbox.get_north());
  ExpandLatitude(bbox.get_south());
  ExpandLongitude(bbox.get_east());
  ExpandLongitude(bbox.get_west());
}

void Bbox::ExpandLatitude(double latitude) {
  if (latitude > north_) {
    north_ = latitude;
  }
  if (latitude < south_) {
    south_ = latitude;
  }
}

void Bbox::ExpandLongitude(double longitude) {
  if (longitude > east_) {
    east_ = longitude;
  }
  if (longitude < west_) {
    west_ = longitude;
  }
}

void Bbox::ExpandLatLon(double latitude, double longitude) {
  ExpandLatitude(latitude);
  ExpandLongitude(longitude);
}

void Bbox::GetCenter(double* latitude, double* longitude) const {
  if (latitude) {
    *latitude = GetCenterLat();
  }
  if (longitude) {
    *longitude = GetCenterLon();
  }
}

double Bbox::get_north() const {
  return north_;
}

double Bbox::get_south() const {
  return south_;
}

double Bbox::get_east() const {
  return east_;
}

double Bbox::get_west() const {
  return west_;
}

double Bbox::GetCenterLat() const {
  return (north_ + south_) / 2.0;
}

double Bbox::GetCenterLon() const {
  return (east_ + west_) / 2.0;
}

void Bbox::set_north(double n) {
  north_ = n;
}

void Bbox::set_south(double s) {
  south_ = s;
}

void Bbox::set_east(double e) {
  east_ = e;
}

void Bbox::set_west(double w) {
  west_ = w;
}
}  // namespace kmlengine
