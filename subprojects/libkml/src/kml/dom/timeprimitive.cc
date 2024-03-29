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

// This file contains the implementation of the abstract element TimePrimitive
// and the concrete elements TimeSpan, TimeStamp.

#include "kml/dom/timeprimitive.h"
#include "kml/base/attributes.h"
#include "kml/dom/serializer.h"
#include "kml/dom/visitor.h"

using kmlbase::Attributes;

namespace kmldom {

TimePrimitive::TimePrimitive() {
}

TimePrimitive::~TimePrimitive() {
}

void TimePrimitive::AddElement(const ElementPtr& element) {
  Object::AddElement(element);
}

TimeSpan::TimeSpan() : has_begin_(false), has_end_(false) {
}

TimeSpan::~TimeSpan() {
}

void TimeSpan::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  switch (element->Type()) {
    case Type_begin:
      has_begin_ = element->SetString(&begin_);
      break;
    case Type_end:
      has_end_ = element->SetString(&end_);
      break;
    default:
      TimePrimitive::AddElement(element);
  }
}

void TimeSpan::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  TimePrimitive::Serialize(serializer);
  if (has_begin()) {
    serializer.SaveFieldById(Type_begin, begin_);
  }
  if (has_end()) {
    serializer.SaveFieldById(Type_end, end_);
  }
}

void TimeSpan::Accept(Visitor* visitor) {
  visitor->VisitTimeSpan(TimeSpanPtr(this));
}

TimeStamp::TimeStamp() : has_when_(false) {
}

TimeStamp::~TimeStamp() {
}

void TimeStamp::AddElement(const ElementPtr& element) {
  if (!element) {
    return;
  }
  if (element->Type() == Type_when) {
    has_when_ = element->SetString(&when_);
  } else {
    TimePrimitive::AddElement(element);
  }
}

void TimeStamp::Serialize(Serializer& serializer) const {
  ElementSerializer element_serializer(*this, serializer);
  TimePrimitive::Serialize(serializer);
  if (has_when()) {
    serializer.SaveFieldById(Type_when, when_);
  }
}

void TimeStamp::Accept(Visitor* visitor) {
  visitor->VisitTimeStamp(TimeStampPtr(this));
}

kmldom::KmlDomType TimePrimitive::Type() const {
  return Type_TimePrimitive;
}

bool TimePrimitive::IsA(kmldom::KmlDomType type) const {
  return type == Type_TimePrimitive || Object::IsA(type);
}

kmldom::KmlDomType TimeSpan::Type() const {
  return Type_TimeSpan;
}

bool TimeSpan::IsA(kmldom::KmlDomType type) const {
  return type == Type_TimeSpan || TimePrimitive::IsA(type);
}

const string& TimeSpan::get_begin() const {
  return begin_;
}

bool TimeSpan::has_begin() const {
  return has_begin_;
}

void TimeSpan::set_begin(const string& value) {
  begin_ = value;
  has_begin_ = true;
}

void TimeSpan::clear_begin() {
  begin_.clear();
  has_begin_ = false;
}

const string& TimeSpan::get_end() const {
  return end_;
}

bool TimeSpan::has_end() const {
  return has_end_;
}

void TimeSpan::set_end(const string& value) {
  end_ = value;
  has_end_ = true;
}

void TimeSpan::clear_end() {
  end_.clear();
  has_end_ = false;
}

kmldom::KmlDomType TimeStamp::Type() const {
  return Type_TimeStamp;
}

bool TimeStamp::IsA(kmldom::KmlDomType type) const {
  return type == Type_TimeStamp || TimePrimitive::IsA(type);
}

const string& TimeStamp::get_when() const {
  return when_;
}

bool TimeStamp::has_when() const {
  return has_when_;
}

void TimeStamp::set_when(const string& value) {
  when_ = value;
  has_when_ = true;
}

void TimeStamp::clear_when() {
  when_.clear();
  has_when_ = false;
}
}  // end namespace kmldom
