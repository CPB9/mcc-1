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

// This file contains XSD convenience utilities.

#include "kml/base/util.h"
#include "kml/config.h"

namespace kmlxsd {

class XsdComplexType;
class XsdElement;
class XsdSchema;

extern KML_EXPORT const char kAbstract[];
extern KML_EXPORT const char kBase[];
extern KML_EXPORT const char kComplexType[];
extern KML_EXPORT const char kDefault[];
extern KML_EXPORT const char kElement[];
extern KML_EXPORT const char kEnumeration[];
extern KML_EXPORT const char kExtension[];
extern KML_EXPORT const char kName[];
extern KML_EXPORT const char kSchema[];
extern KML_EXPORT const char kSimpleType[];
extern KML_EXPORT const char kSubstitutionGroup[];
extern KML_EXPORT const char kRestriction[];
extern KML_EXPORT const char kTargetNamespace[];
extern KML_EXPORT const char kType[];
extern KML_EXPORT const char kValue[];

// Convenience utility to create a <xs:complexType name="TYPE_NAME"/>.
KML_EXPORT XsdComplexType* CreateXsdComplexType(const string& type_name);

// Convenience utility ot create a <xs:element name="NAME" type="TYPE"/>.
KML_EXPORT XsdElement* CreateXsdElement(const string& name, const string& type);

// Convenience utilty to create an XsdSchema based on:
// <schema xmlns:PREFIX="TARGET_NAMESPACE"
//         targetNamespace="TARGET_NAMESPACE"/>
KML_EXPORT XsdSchema* CreateXsdSchema(const string& prefix,
                                      const string& target_namespace);

}  // end namespace kmlxsd
