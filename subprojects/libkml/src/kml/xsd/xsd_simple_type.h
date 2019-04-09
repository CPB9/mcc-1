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

#ifndef KML_XSD_XSD_SIMPLE_TYPE_H__
#define KML_XSD_XSD_SIMPLE_TYPE_H__

#include "kml/base/rc.h"
#include <vector>
#include "kml/base/util.h"
#include "kml/config.h"
#include "kml/xsd/xsd_type.h"

namespace kmlbase {
class Attributes;
}

namespace kmlxsd {

class XsdSimpleType;

typedef kmlbase::Rc<XsdSimpleType> XsdSimpleTypePtr;

// Corresponds to <xs:simpleType>.
class KML_EXPORT XsdSimpleType : public XsdType {
 public:
  ~XsdSimpleType();
  static XsdSimpleType* Create(const kmlbase::Attributes& attributes);

  static XsdSimpleTypePtr AsSimpleType(const XsdTypePtr& xsd_type);

  virtual XsdTypeEnum get_xsd_type_id() const;

  virtual bool is_complex() const;

  // <xs:simpleType name="NAME"/>
  virtual const string get_name() const;

  virtual const string get_base() const;

  // <xs:restriction base="BASE"/>
  void set_restriction_base(const string& base);
  const string& get_restriction_base() const;

  // <xs:enumeration value="VALUE"/>
  void add_enumeration(const string& value);

  // Return the number of <xs:enumeration value="..."/>'s.
  size_t get_enumeration_size() const;

  // Return the index'th <xs:enumeration value="..."/>.  The order is preserved
  // as added in add_enumeration_value().
  const string& get_enumeration_at(size_t index) const;

  // Returns true if this is an enumerated type.
  bool IsEnumeration() const;

 private:
  // Client code should use Create().
  XsdSimpleType(const string& name);

  const string name_;
  string restriction_base_;
  std::vector<string> enumeration_;
};

}  // end namespace kmlxsd

#endif  // KML_XSD_XSD_SIMPLE_TYPE_H__
