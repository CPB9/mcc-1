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

#ifndef KML_DOM_KML_CAST_H__
#define KML_DOM_KML_CAST_H__

#include <exception>  // Needed by boost::instrusive_ptr.

#include "kml/base/xml_element.h"
#include "kml/config.h"
#include "kml/dom/kml_ptr.h"

namespace kmldom {

// This function template operates akin to dynamic_cast.  If the given
// Element-derived type is of the template type then a pointer is returned,
// else NULL.  It is safe to pass a NULL to this function.
KML_EXPORT const ElementPtr
AsElement(const kmlbase::XmlElementPtr& xml_element);

// Abstract element groups.
KML_EXPORT const AbstractLatLonBoxPtr
AsAbstractLatLonBox(const ElementPtr element);
KML_EXPORT const AbstractViewPtr AsAbstractView(const ElementPtr element);
KML_EXPORT const ColorStylePtr AsColorStyle(const ElementPtr element);
KML_EXPORT const ContainerPtr AsContainer(const ElementPtr element);
KML_EXPORT const FeaturePtr AsFeature(const ElementPtr element);
KML_EXPORT const GeometryPtr AsGeometry(const ElementPtr element);
KML_EXPORT const ObjectPtr AsObject(const ElementPtr element);
KML_EXPORT const OverlayPtr AsOverlay(const ElementPtr element);
KML_EXPORT const StyleSelectorPtr AsStyleSelector(const ElementPtr element);
KML_EXPORT const SubStylePtr AsSubStyle(const ElementPtr element);
KML_EXPORT const TimePrimitivePtr AsTimePrimitive(const ElementPtr element);

// Concrete elements.
KML_EXPORT const AliasPtr AsAlias(const ElementPtr element);
KML_EXPORT const BalloonStylePtr AsBalloonStyle(const ElementPtr element);
KML_EXPORT const CameraPtr AsCamera(const ElementPtr element);
KML_EXPORT const ChangePtr AsChange(const ElementPtr element);
KML_EXPORT const CoordinatesPtr AsCoordinates(const ElementPtr& element);
KML_EXPORT const CreatePtr AsCreate(const ElementPtr element);
KML_EXPORT const DataPtr AsData(const ElementPtr element);
KML_EXPORT const DeletePtr AsDelete(const ElementPtr element);
KML_EXPORT const DocumentPtr AsDocument(const ElementPtr element);
KML_EXPORT const ExtendedDataPtr AsExtendedData(const ElementPtr& element);
KML_EXPORT const FolderPtr AsFolder(const ElementPtr element);
KML_EXPORT const GroundOverlayPtr AsGroundOverlay(const ElementPtr element);
KML_EXPORT const HotSpotPtr AsHotSpot(const ElementPtr element);
KML_EXPORT const IconPtr AsIcon(const ElementPtr element);
KML_EXPORT const IconStylePtr AsIconStyle(const ElementPtr element);
KML_EXPORT const IconStyleIconPtr AsIconStyleIcon(const ElementPtr element);
KML_EXPORT const ImagePyramidPtr AsImagePyramid(const ElementPtr element);
KML_EXPORT const InnerBoundaryIsPtr AsInnerBoundaryIs(const ElementPtr element);
KML_EXPORT const ItemIconPtr AsItemIcon(const ElementPtr element);
KML_EXPORT const KmlPtr AsKml(const ElementPtr& element);
KML_EXPORT const LabelStylePtr AsLabelStyle(const ElementPtr element);
KML_EXPORT const LatLonAltBoxPtr AsLatLonAltBox(const ElementPtr element);
KML_EXPORT const LatLonBoxPtr AsLatLonBox(const ElementPtr element);
KML_EXPORT const LineStringPtr AsLineString(const ElementPtr element);
KML_EXPORT const LineStylePtr AsLineStyle(const ElementPtr element);
KML_EXPORT const LinearRingPtr AsLinearRing(const ElementPtr element);
KML_EXPORT const LinkPtr AsLink(const ElementPtr element);
KML_EXPORT const LinkSnippetPtr AsLinkSnippet(const ElementPtr element);
KML_EXPORT const ListStylePtr AsListStyle(const ElementPtr element);
KML_EXPORT const LocationPtr AsLocation(const ElementPtr element);
KML_EXPORT const LodPtr AsLod(const ElementPtr element);
KML_EXPORT const LookAtPtr AsLookAt(const ElementPtr element);
KML_EXPORT const MetadataPtr AsMetadata(const ElementPtr& element);
KML_EXPORT const ModelPtr AsModel(const ElementPtr element);
KML_EXPORT const MultiGeometryPtr AsMultiGeometry(const ElementPtr element);
KML_EXPORT const NetworkLinkPtr AsNetworkLink(const ElementPtr element);
KML_EXPORT const NetworkLinkControlPtr
AsNetworkLinkControl(const ElementPtr& element);
KML_EXPORT const OrientationPtr AsOrientation(const ElementPtr element);
KML_EXPORT const OuterBoundaryIsPtr AsOuterBoundaryIs(const ElementPtr element);
KML_EXPORT const OverlayXYPtr AsOverlayXY(const ElementPtr element);
KML_EXPORT const PairPtr AsPair(const ElementPtr element);
KML_EXPORT const PhotoOverlayPtr AsPhotoOverlay(const ElementPtr element);
KML_EXPORT const PlacemarkPtr AsPlacemark(const ElementPtr element);
KML_EXPORT const PointPtr AsPoint(const ElementPtr element);
KML_EXPORT const PolyStylePtr AsPolyStyle(const ElementPtr element);
KML_EXPORT const PolygonPtr AsPolygon(const ElementPtr element);
KML_EXPORT const RegionPtr AsRegion(const ElementPtr element);
KML_EXPORT const ResourceMapPtr AsResourceMap(const ElementPtr element);
KML_EXPORT const RotationXYPtr AsRotationXY(const ElementPtr element);
KML_EXPORT const ScalePtr AsScale(const ElementPtr element);
KML_EXPORT const SchemaPtr AsSchema(const ElementPtr element);
KML_EXPORT const SchemaDataPtr AsSchemaData(const ElementPtr element);
KML_EXPORT const ScreenOverlayPtr AsScreenOverlay(const ElementPtr element);
KML_EXPORT const ScreenXYPtr AsScreenXY(const ElementPtr element);
KML_EXPORT const SimpleDataPtr AsSimpleData(const ElementPtr& element);
KML_EXPORT const SimpleFieldPtr AsSimpleField(const ElementPtr& element);
KML_EXPORT const SizePtr AsSize(const ElementPtr element);
KML_EXPORT const SnippetPtr AsSnippet(const ElementPtr element);
KML_EXPORT const StylePtr AsStyle(const ElementPtr element);
KML_EXPORT const StyleMapPtr AsStyleMap(const ElementPtr element);
KML_EXPORT const TimeSpanPtr AsTimeSpan(const ElementPtr element);
KML_EXPORT const TimeStampPtr AsTimeStamp(const ElementPtr element);
KML_EXPORT const UpdatePtr AsUpdate(const ElementPtr& element);
KML_EXPORT const ViewVolumePtr AsViewVolume(const ElementPtr element);

// Atom
KML_EXPORT const AtomAuthorPtr AsAtomAuthor(const ElementPtr& element);
KML_EXPORT const AtomCategoryPtr AsAtomCategory(const ElementPtr& element);
KML_EXPORT const AtomContentPtr AsAtomContent(const ElementPtr& element);
KML_EXPORT const AtomEntryPtr AsAtomEntry(const ElementPtr& element);
KML_EXPORT const AtomFeedPtr AsAtomFeed(const ElementPtr& element);
KML_EXPORT const AtomLinkPtr AsAtomLink(const ElementPtr& element);

// xAL
KML_EXPORT const XalAddressDetailsPtr
AsXalAddressDetails(const ElementPtr& element);
KML_EXPORT const XalAdministrativeAreaPtr
AsXalAdministrativeArea(const ElementPtr& element);
KML_EXPORT const XalCountryPtr AsXalCountry(const ElementPtr& element);
KML_EXPORT const XalLocalityPtr AsXalLocality(const ElementPtr& element);
KML_EXPORT const XalPostalCodePtr AsXalPostalCode(const ElementPtr& element);
KML_EXPORT const XalSubAdministrativeAreaPtr
AsXalSubAdministrativeArea(const ElementPtr& element);
KML_EXPORT const XalThoroughfarePtr
AsXalThoroughfare(const ElementPtr& element);

// gx
KML_EXPORT const GxAnimatedUpdatePtr
AsGxAnimatedUpdate(const ElementPtr element);
KML_EXPORT const GxFlyToPtr AsGxFlyTo(const ElementPtr element);
KML_EXPORT const GxLatLonQuadPtr AsGxLatLonQuad(const ElementPtr element);
KML_EXPORT const GxMultiTrackPtr AsGxMultiTrack(const ElementPtr element);
KML_EXPORT const GxPlaylistPtr AsGxPlaylist(const ElementPtr element);
KML_EXPORT const GxSimpleArrayFieldPtr
AsGxSimpleArrayField(const ElementPtr element);
KML_EXPORT const GxSimpleArrayDataPtr
AsGxSimpleArrayData(const ElementPtr element);
KML_EXPORT const GxSoundCuePtr AsGxSoundCue(const ElementPtr element);
KML_EXPORT const GxTimeSpanPtr AsGxTimeSpan(const ElementPtr element);
KML_EXPORT const GxTimeStampPtr AsGxTimeStamp(const ElementPtr element);
KML_EXPORT const GxTourPtr AsGxTour(const ElementPtr element);
KML_EXPORT const GxTourControlPtr AsGxTourControl(const ElementPtr element);
KML_EXPORT const GxTourPrimitivePtr AsGxTourPrimitive(const ElementPtr element);
KML_EXPORT const GxTrackPtr AsGxTrack(const ElementPtr element);
KML_EXPORT const GxWaitPtr AsGxWait(const ElementPtr element);

}  // end namespace kmldom

#endif  // KML_DOM_KML_CAST_H__
