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

#include "kml_cast.h"
#include "kml/base/rc.h"
#include "kml/dom/kmldom.h"

namespace kmldom {

template <class T>
inline const kmlbase::Rc<T> ElementCast(const ElementPtr& element) {
  if (element && element->IsA(T::ElementType())) {
    return kmlbase::static_pointer_cast<T>(element);
  }
  return nullptr;
}

const ElementPtr AsElement(const kmlbase::XmlElementPtr& xml_element) {
  return kmlbase::static_pointer_cast<Element>(xml_element);
}

const AbstractLatLonBoxPtr AsAbstractLatLonBox(const ElementPtr element) {
  if (element && element->IsA(Type_AbstractLatLonBox)) {
    return kmlbase::static_pointer_cast<AbstractLatLonBox>(element);
  }
  return nullptr;
}

const AbstractViewPtr AsAbstractView(const ElementPtr element) {
  if (element && element->IsA(Type_AbstractView)) {
    return kmlbase::static_pointer_cast<AbstractView>(element);
  }
  return nullptr;
}

const ColorStylePtr AsColorStyle(const ElementPtr element) {
  if (element && element->IsA(Type_ColorStyle)) {
    return kmlbase::static_pointer_cast<ColorStyle>(element);
  }
  return nullptr;
}

const ContainerPtr AsContainer(const ElementPtr element) {
  if (element && element->IsA(Type_Container)) {
    return kmlbase::static_pointer_cast<Container>(element);
  }
  return nullptr;
}

const FeaturePtr AsFeature(const ElementPtr element) {
  if (element && element->IsA(Type_Feature)) {
    return kmlbase::static_pointer_cast<Feature>(element);
  }
  return nullptr;
}

const GeometryPtr AsGeometry(const ElementPtr element) {
  if (element && element->IsA(Type_Geometry)) {
    return kmlbase::static_pointer_cast<Geometry>(element);
  }
  return nullptr;
}

const ObjectPtr AsObject(const ElementPtr element) {
  if (element && element->IsA(Type_Object)) {
    return kmlbase::static_pointer_cast<Object>(element);
  }
  return nullptr;
}

const OverlayPtr AsOverlay(const ElementPtr element) {
  if (element && element->IsA(Type_Overlay)) {
    return kmlbase::static_pointer_cast<Overlay>(element);
  }
  return nullptr;
}

const StyleSelectorPtr AsStyleSelector(const ElementPtr element) {
  if (element && element->IsA(Type_StyleSelector)) {
    return kmlbase::static_pointer_cast<StyleSelector>(element);
  }
  return nullptr;
}

const SubStylePtr AsSubStyle(const ElementPtr element) {
  if (element && element->IsA(Type_SubStyle)) {
    return kmlbase::static_pointer_cast<SubStyle>(element);
  }
  return nullptr;
}

const TimePrimitivePtr AsTimePrimitive(const ElementPtr element) {
  if (element && element->IsA(Type_TimePrimitive)) {
    return kmlbase::static_pointer_cast<TimePrimitive>(element);
  }
  return nullptr;
}

const AliasPtr AsAlias(const ElementPtr element) {
  if (element && element->Type() == Type_Alias) {
    return kmlbase::static_pointer_cast<Alias>(element);
  }
  return nullptr;
}

const BalloonStylePtr AsBalloonStyle(const ElementPtr element) {
  if (element && element->Type() == Type_BalloonStyle) {
    return kmlbase::static_pointer_cast<BalloonStyle>(element);
  }
  return nullptr;
}

const CameraPtr AsCamera(const ElementPtr element) {
  if (element && element->Type() == Type_Camera) {
    return kmlbase::static_pointer_cast<Camera>(element);
  }
  return nullptr;
}

const ChangePtr AsChange(const ElementPtr element) {
  if (element && element->Type() == Type_Change) {
    return kmlbase::static_pointer_cast<Change>(element);
  }
  return nullptr;
}

const CreatePtr AsCreate(const ElementPtr element) {
  if (element && element->Type() == Type_Create) {
    return kmlbase::static_pointer_cast<Create>(element);
  }
  return nullptr;
}

const DataPtr AsData(const ElementPtr element) {
  if (element && element->Type() == Type_Data) {
    return kmlbase::static_pointer_cast<Data>(element);
  }
  return nullptr;
}

const DeletePtr AsDelete(const ElementPtr element) {
  if (element && element->Type() == Type_Delete) {
    return kmlbase::static_pointer_cast<Delete>(element);
  }
  return nullptr;
}

const DocumentPtr AsDocument(const ElementPtr element) {
  if (element && element->Type() == Type_Document) {
    return kmlbase::static_pointer_cast<Document>(element);
  }
  return nullptr;
}

const FolderPtr AsFolder(const ElementPtr element) {
  if (element && element->Type() == Type_Folder) {
    return kmlbase::static_pointer_cast<Folder>(element);
  }
  return nullptr;
}

const GroundOverlayPtr AsGroundOverlay(const ElementPtr element) {
  if (element && element->Type() == Type_GroundOverlay) {
    return kmlbase::static_pointer_cast<GroundOverlay>(element);
  }
  return nullptr;
}

const HotSpotPtr AsHotSpot(const ElementPtr element) {
  if (element && element->Type() == Type_hotSpot) {
    return kmlbase::static_pointer_cast<HotSpot>(element);
  }
  return nullptr;
}

const IconPtr AsIcon(const ElementPtr element) {
  if (element && element->Type() == Type_Icon) {
    return kmlbase::static_pointer_cast<Icon>(element);
  }
  return nullptr;
}

const IconStylePtr AsIconStyle(const ElementPtr element) {
  if (element && element->Type() == Type_IconStyle) {
    return kmlbase::static_pointer_cast<IconStyle>(element);
  }
  return nullptr;
}

const IconStyleIconPtr AsIconStyleIcon(const ElementPtr element) {
  if (element && element->Type() == Type_IconStyleIcon) {
    return kmlbase::static_pointer_cast<IconStyleIcon>(element);
  }
  return nullptr;
}

const ImagePyramidPtr AsImagePyramid(const ElementPtr element) {
  if (element && element->Type() == Type_ImagePyramid) {
    return kmlbase::static_pointer_cast<ImagePyramid>(element);
  }
  return nullptr;
}

const InnerBoundaryIsPtr AsInnerBoundaryIs(const ElementPtr element) {
  if (element && element->Type() == Type_innerBoundaryIs) {
    return kmlbase::static_pointer_cast<InnerBoundaryIs>(element);
  }
  return nullptr;
}

const ItemIconPtr AsItemIcon(const ElementPtr element) {
  if (element && element->Type() == Type_ItemIcon) {
    return kmlbase::static_pointer_cast<ItemIcon>(element);
  }
  return nullptr;
}

const LabelStylePtr AsLabelStyle(const ElementPtr element) {
  if (element && element->Type() == Type_LabelStyle) {
    return kmlbase::static_pointer_cast<LabelStyle>(element);
  }
  return nullptr;
}

const LatLonAltBoxPtr AsLatLonAltBox(const ElementPtr element) {
  if (element && element->Type() == Type_LatLonAltBox) {
    return kmlbase::static_pointer_cast<LatLonAltBox>(element);
  }
  return nullptr;
}

const LatLonBoxPtr AsLatLonBox(const ElementPtr element) {
  if (element && element->Type() == Type_LatLonBox) {
    return kmlbase::static_pointer_cast<LatLonBox>(element);
  }
  return nullptr;
}

const LineStringPtr AsLineString(const ElementPtr element) {
  if (element && element->Type() == Type_LineString) {
    return kmlbase::static_pointer_cast<LineString>(element);
  }
  return nullptr;
}

const LineStylePtr AsLineStyle(const ElementPtr element) {
  if (element && element->Type() == Type_LineStyle) {
    return kmlbase::static_pointer_cast<LineStyle>(element);
  }
  return nullptr;
}

const LinearRingPtr AsLinearRing(const ElementPtr element) {
  if (element && element->Type() == Type_LinearRing) {
    return kmlbase::static_pointer_cast<LinearRing>(element);
  }
  return nullptr;
}

const LinkPtr AsLink(const ElementPtr element) {
  if (element && element->Type() == Type_Link) {
    return kmlbase::static_pointer_cast<Link>(element);
  }
  return nullptr;
}

const LinkSnippetPtr AsLinkSnippet(const ElementPtr element) {
  if (element && element->Type() == Type_linkSnippet) {
    return kmlbase::static_pointer_cast<LinkSnippet>(element);
  }
  return nullptr;
}

const ListStylePtr AsListStyle(const ElementPtr element) {
  if (element && element->Type() == Type_ListStyle) {
    return kmlbase::static_pointer_cast<ListStyle>(element);
  }
  return nullptr;
}

const LocationPtr AsLocation(const ElementPtr element) {
  if (element && element->Type() == Type_Location) {
    return kmlbase::static_pointer_cast<Location>(element);
  }
  return nullptr;
}

const LodPtr AsLod(const ElementPtr element) {
  if (element && element->Type() == Type_Lod) {
    return kmlbase::static_pointer_cast<Lod>(element);
  }
  return nullptr;
}

const LookAtPtr AsLookAt(const ElementPtr element) {
  if (element && element->Type() == Type_LookAt) {
    return kmlbase::static_pointer_cast<LookAt>(element);
  }
  return nullptr;
}

const ModelPtr AsModel(const ElementPtr element) {
  if (element && element->Type() == Type_Model) {
    return kmlbase::static_pointer_cast<Model>(element);
  }
  return nullptr;
}

const MultiGeometryPtr AsMultiGeometry(const ElementPtr element) {
  if (element && element->Type() == Type_MultiGeometry) {
    return kmlbase::static_pointer_cast<MultiGeometry>(element);
  }
  return nullptr;
}

const NetworkLinkPtr AsNetworkLink(const ElementPtr element) {
  if (element && element->Type() == Type_NetworkLink) {
    return kmlbase::static_pointer_cast<NetworkLink>(element);
  }
  return nullptr;
}

const OrientationPtr AsOrientation(const ElementPtr element) {
  if (element && element->Type() == Type_Orientation) {
    return kmlbase::static_pointer_cast<Orientation>(element);
  }
  return nullptr;
}

const OuterBoundaryIsPtr AsOuterBoundaryIs(const ElementPtr element) {
  if (element && element->Type() == Type_outerBoundaryIs) {
    return kmlbase::static_pointer_cast<OuterBoundaryIs>(element);
  }
  return nullptr;
}

const OverlayXYPtr AsOverlayXY(const ElementPtr element) {
  if (element && element->Type() == Type_overlayXY) {
    return kmlbase::static_pointer_cast<OverlayXY>(element);
  }
  return nullptr;
}

const PairPtr AsPair(const ElementPtr element) {
  if (element && element->Type() == Type_Pair) {
    return kmlbase::static_pointer_cast<Pair>(element);
  }
  return nullptr;
}

const PhotoOverlayPtr AsPhotoOverlay(const ElementPtr element) {
  if (element && element->Type() == Type_PhotoOverlay) {
    return kmlbase::static_pointer_cast<PhotoOverlay>(element);
  }
  return nullptr;
}

const PlacemarkPtr AsPlacemark(const ElementPtr element) {
  if (element && element->Type() == Type_Placemark) {
    return kmlbase::static_pointer_cast<Placemark>(element);
  }
  return nullptr;
}

const PointPtr AsPoint(const ElementPtr element) {
  if (element && element->Type() == Type_Point) {
    return kmlbase::static_pointer_cast<Point>(element);
  }
  return nullptr;
}

const PolyStylePtr AsPolyStyle(const ElementPtr element) {
  if (element && element->Type() == Type_PolyStyle) {
    return kmlbase::static_pointer_cast<PolyStyle>(element);
  }
  return nullptr;
}

const PolygonPtr AsPolygon(const ElementPtr element) {
  if (element && element->Type() == Type_Polygon) {
    return kmlbase::static_pointer_cast<Polygon>(element);
  }
  return nullptr;
}

const RegionPtr AsRegion(const ElementPtr element) {
  if (element && element->Type() == Type_Region) {
    return kmlbase::static_pointer_cast<Region>(element);
  }
  return nullptr;
}

const ResourceMapPtr AsResourceMap(const ElementPtr element) {
  if (element && element->Type() == Type_ResourceMap) {
    return kmlbase::static_pointer_cast<ResourceMap>(element);
  }
  return nullptr;
}

const RotationXYPtr AsRotationXY(const ElementPtr element) {
  if (element && element->Type() == Type_rotationXY) {
    return kmlbase::static_pointer_cast<RotationXY>(element);
  }
  return nullptr;
}

const ScalePtr AsScale(const ElementPtr element) {
  if (element && element->Type() == Type_Scale) {
    return kmlbase::static_pointer_cast<Scale>(element);
  }
  return nullptr;
}

const SchemaPtr AsSchema(const ElementPtr element) {
  if (element && element->Type() == Type_Schema) {
    return kmlbase::static_pointer_cast<Schema>(element);
  }
  return nullptr;
}

const SchemaDataPtr AsSchemaData(const ElementPtr element) {
  if (element && element->Type() == Type_SchemaData) {
    return kmlbase::static_pointer_cast<SchemaData>(element);
  }
  return nullptr;
}

const ScreenOverlayPtr AsScreenOverlay(const ElementPtr element) {
  if (element && element->Type() == Type_ScreenOverlay) {
    return kmlbase::static_pointer_cast<ScreenOverlay>(element);
  }
  return nullptr;
}

const ScreenXYPtr AsScreenXY(const ElementPtr element) {
  if (element && element->Type() == Type_screenXY) {
    return kmlbase::static_pointer_cast<ScreenXY>(element);
  }
  return nullptr;
}

const SizePtr AsSize(const ElementPtr element) {
  if (element && element->Type() == Type_size) {
    return kmlbase::static_pointer_cast<Size>(element);
  }
  return nullptr;
}

const SnippetPtr AsSnippet(const ElementPtr element) {
  if (element && element->Type() == Type_Snippet) {
    return kmlbase::static_pointer_cast<Snippet>(element);
  }
  return nullptr;
}

const StylePtr AsStyle(const ElementPtr element) {
  if (element && element->Type() == Type_Style) {
    return kmlbase::static_pointer_cast<Style>(element);
  }
  return nullptr;
}

const StyleMapPtr AsStyleMap(const ElementPtr element) {
  if (element && element->Type() == Type_StyleMap) {
    return kmlbase::static_pointer_cast<StyleMap>(element);
  }
  return nullptr;
}

const TimeSpanPtr AsTimeSpan(const ElementPtr element) {
  if (element && element->Type() == Type_TimeSpan) {
    return kmlbase::static_pointer_cast<TimeSpan>(element);
  }
  return nullptr;
}

const TimeStampPtr AsTimeStamp(const ElementPtr element) {
  if (element && element->Type() == Type_TimeStamp) {
    return kmlbase::static_pointer_cast<TimeStamp>(element);
  }
  return nullptr;
}

const UpdatePtr AsUpdate(const ElementPtr element) {
  if (element && element->Type() == Type_Update) {
    return kmlbase::static_pointer_cast<Update>(element);
  }
  return nullptr;
}

const ViewVolumePtr AsViewVolume(const ElementPtr element) {
  if (element && element->Type() == Type_ViewVolume) {
    return kmlbase::static_pointer_cast<ViewVolume>(element);
  }
  return nullptr;
}

const CoordinatesPtr AsCoordinates(const ElementPtr& element) {
  return ElementCast<Coordinates>(element);
}

const ExtendedDataPtr AsExtendedData(const ElementPtr& element) {
  return ElementCast<ExtendedData>(element);
}

const KmlPtr AsKml(const ElementPtr& element) {
  return ElementCast<Kml>(element);
}

const MetadataPtr AsMetadata(const ElementPtr& element) {
  return ElementCast<Metadata>(element);
}

const NetworkLinkControlPtr AsNetworkLinkControl(const ElementPtr& element) {
  return ElementCast<NetworkLinkControl>(element);
}

const SimpleDataPtr AsSimpleData(const ElementPtr& element) {
  return ElementCast<SimpleData>(element);
}

const SimpleFieldPtr AsSimpleField(const ElementPtr& element) {
  return ElementCast<SimpleField>(element);
}

const UpdatePtr AsUpdate(const ElementPtr& element) {
  return ElementCast<Update>(element);
}

const AtomAuthorPtr AsAtomAuthor(const ElementPtr& element) {
  return ElementCast<AtomAuthor>(element);
}

const AtomCategoryPtr AsAtomCategory(const ElementPtr& element) {
  return ElementCast<AtomCategory>(element);
}

const AtomContentPtr AsAtomContent(const ElementPtr& element) {
  return ElementCast<AtomContent>(element);
}

const AtomEntryPtr AsAtomEntry(const ElementPtr& element) {
  return ElementCast<AtomEntry>(element);
}

const AtomFeedPtr AsAtomFeed(const ElementPtr& element) {
  return ElementCast<AtomFeed>(element);
}

const AtomLinkPtr AsAtomLink(const ElementPtr& element) {
  return ElementCast<AtomLink>(element);
}

const XalAddressDetailsPtr AsXalAddressDetails(const ElementPtr& element) {
  return ElementCast<XalAddressDetails>(element);
}

const XalAdministrativeAreaPtr AsXalAdministrativeArea(
    const ElementPtr& element) {
  return ElementCast<XalAdministrativeArea>(element);
}

const XalCountryPtr AsXalCountry(const ElementPtr& element) {
  return ElementCast<XalCountry>(element);
}

const XalLocalityPtr AsXalLocality(const ElementPtr& element) {
  return ElementCast<XalLocality>(element);
}

const XalPostalCodePtr AsXalPostalCode(const ElementPtr& element) {
  return ElementCast<XalPostalCode>(element);
}

const XalSubAdministrativeAreaPtr AsXalSubAdministrativeArea(
    const ElementPtr& element) {
  return ElementCast<XalSubAdministrativeArea>(element);
}

const XalThoroughfarePtr AsXalThoroughfare(const ElementPtr& element) {
  return ElementCast<XalThoroughfare>(element);
}

const GxAnimatedUpdatePtr AsGxAnimatedUpdate(const ElementPtr element) {
  return ElementCast<GxAnimatedUpdate>(element);
}

const GxFlyToPtr AsGxFlyTo(const ElementPtr element) {
  return ElementCast<GxFlyTo>(element);
}

const GxLatLonQuadPtr AsGxLatLonQuad(const ElementPtr element) {
  return ElementCast<GxLatLonQuad>(element);
}

const GxMultiTrackPtr AsGxMultiTrack(const ElementPtr element) {
  return ElementCast<GxMultiTrack>(element);
}

const GxPlaylistPtr AsGxPlaylist(const ElementPtr element) {
  return ElementCast<GxPlaylist>(element);
}

const GxSimpleArrayFieldPtr AsGxSimpleArrayField(const ElementPtr element) {
  return ElementCast<GxSimpleArrayField>(element);
}

const GxSimpleArrayDataPtr AsGxSimpleArrayData(const ElementPtr element) {
  return ElementCast<GxSimpleArrayData>(element);
}

const GxSoundCuePtr AsGxSoundCue(const ElementPtr element) {
  return ElementCast<GxSoundCue>(element);
}

const GxTimeSpanPtr AsGxTimeSpan(const ElementPtr element) {
  return ElementCast<GxTimeSpan>(element);
}

const GxTimeStampPtr AsGxTimeStamp(const ElementPtr element) {
  return ElementCast<GxTimeStamp>(element);
}

const GxTourPtr AsGxTour(const ElementPtr element) {
  return ElementCast<GxTour>(element);
}

const GxTourControlPtr AsGxTourControl(const ElementPtr element) {
  return ElementCast<GxTourControl>(element);
}

const GxTourPrimitivePtr AsGxTourPrimitive(const ElementPtr element) {
  return ElementCast<GxTourPrimitive>(element);
}

const GxTrackPtr AsGxTrack(const ElementPtr element) {
  return ElementCast<GxTrack>(element);
}

const GxWaitPtr AsGxWait(const ElementPtr element) {
  return ElementCast<GxWait>(element);
}
}  // end namespace kmldom
