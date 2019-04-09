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

#ifndef KML_DOM_KML_PTR_H__
#define KML_DOM_KML_PTR_H__

#include "kml/base/rc.h"

namespace kmldom {

class Element;
class Field;

class AbstractLatLonBox;
class AbstractLink;
class AbstractView;
class BasicLink;
class ColorStyle;
class Container;
class Feature;
class Geometry;
class Object;
class Overlay;
class StyleSelector;
class SubStyle;
class TimePrimitive;
class Vec2;

class Alias;
class AtomAuthor;
class AtomCategory;
class AtomContent;
class AtomEntry;
class AtomFeed;
class AtomLink;
class BalloonStyle;
class Camera;
class Change;
class Coordinates;
class Create;
class Data;
class Delete;
class Document;
class ExtendedData;
class Folder;
class GroundOverlay;
class Icon;
class IconStyle;
class IconStyleIcon;
class ImagePyramid;
class ItemIcon;
class LabelStyle;
class LatLonAltBox;
class LatLonBox;
class LineString;
class LineStyle;
class LinearRing;
class Link;
class ListStyle;
class Location;
class Lod;
class LookAt;
class Metadata;
class Model;
class MultiGeometry;
class NetworkLink;
class NetworkLinkControl;
class Orientation;
class Pair;
class PhotoOverlay;
class Placemark;
class Point;
class PolyStyle;
class Polygon;
class Region;
class ResourceMap;
class Scale;
class Schema;
class SchemaData;
class ScreenOverlay;
class SimpleData;
class SimpleField;
class Snippet;
class Style;
class StyleMap;
class TimeSpan;
class TimeStamp;
class Update;
class UpdateOperation;
class Url;
class ViewVolume;
class HotSpot;
class InnerBoundaryIs;
class Kml;
class LinkSnippet;
class OuterBoundaryIs;
class OverlayXY;
class RotationXY;
class ScreenXY;
class Size;

class XalAddressDetails;
class XalAdministrativeArea;
class XalCountry;
class XalLocality;
class XalPostalCode;
class XalSubAdministrativeArea;
class XalThoroughfare;

class GxAnimatedUpdate;
class GxFlyTo;
class GxLatLonQuad;
class GxMultiTrack;
class GxPlaylist;
class GxSimpleArrayField;
class GxSimpleArrayData;
class GxSoundCue;
class GxTimeSpan;
class GxTimeStamp;
class GxTimePrimitive;
class GxTour;
class GxTourControl;
class GxTourPrimitive;
class GxTrack;
class GxWait;

typedef kmlbase::Rc<Element> ElementPtr;
typedef kmlbase::Rc<Field> FieldPtr;

typedef kmlbase::Rc<AbstractLatLonBox> AbstractLatLonBoxPtr;
typedef kmlbase::Rc<AbstractLink> AbstractLinkPtr;
typedef kmlbase::Rc<AbstractView> AbstractViewPtr;
typedef kmlbase::Rc<BasicLink> BasicLinkPtr;
typedef kmlbase::Rc<ColorStyle> ColorStylePtr;
typedef kmlbase::Rc<Container> ContainerPtr;
typedef kmlbase::Rc<Feature> FeaturePtr;
typedef kmlbase::Rc<Geometry> GeometryPtr;
typedef kmlbase::Rc<Object> ObjectPtr;
typedef kmlbase::Rc<Overlay> OverlayPtr;
typedef kmlbase::Rc<StyleSelector> StyleSelectorPtr;
typedef kmlbase::Rc<SubStyle> SubStylePtr;
typedef kmlbase::Rc<TimePrimitive> TimePrimitivePtr;
typedef kmlbase::Rc<Vec2> Vec2Ptr;

typedef kmlbase::Rc<Alias> AliasPtr;
typedef kmlbase::Rc<AtomAuthor> AtomAuthorPtr;
typedef kmlbase::Rc<AtomCategory> AtomCategoryPtr;
typedef kmlbase::Rc<AtomContent> AtomContentPtr;
typedef kmlbase::Rc<AtomEntry> AtomEntryPtr;
typedef kmlbase::Rc<AtomFeed> AtomFeedPtr;
typedef kmlbase::Rc<AtomLink> AtomLinkPtr;
typedef kmlbase::Rc<BalloonStyle> BalloonStylePtr;
typedef kmlbase::Rc<Camera> CameraPtr;
typedef kmlbase::Rc<Change> ChangePtr;
typedef kmlbase::Rc<Coordinates> CoordinatesPtr;
typedef kmlbase::Rc<Create> CreatePtr;
typedef kmlbase::Rc<Data> DataPtr;
typedef kmlbase::Rc<Delete> DeletePtr;
typedef kmlbase::Rc<Document> DocumentPtr;
typedef kmlbase::Rc<ExtendedData> ExtendedDataPtr;
typedef kmlbase::Rc<Folder> FolderPtr;
typedef kmlbase::Rc<GroundOverlay> GroundOverlayPtr;
typedef kmlbase::Rc<Icon> IconPtr;
typedef kmlbase::Rc<IconStyle> IconStylePtr;
typedef kmlbase::Rc<IconStyleIcon> IconStyleIconPtr;
typedef kmlbase::Rc<ImagePyramid> ImagePyramidPtr;
typedef kmlbase::Rc<ItemIcon> ItemIconPtr;
typedef kmlbase::Rc<LabelStyle> LabelStylePtr;
typedef kmlbase::Rc<LatLonAltBox> LatLonAltBoxPtr;
typedef kmlbase::Rc<LatLonBox> LatLonBoxPtr;
typedef kmlbase::Rc<LineString> LineStringPtr;
typedef kmlbase::Rc<LineStyle> LineStylePtr;
typedef kmlbase::Rc<LinearRing> LinearRingPtr;
typedef kmlbase::Rc<Link> LinkPtr;
typedef kmlbase::Rc<ListStyle> ListStylePtr;
typedef kmlbase::Rc<Location> LocationPtr;
typedef kmlbase::Rc<Lod> LodPtr;
typedef kmlbase::Rc<LookAt> LookAtPtr;
typedef kmlbase::Rc<Metadata> MetadataPtr;
typedef kmlbase::Rc<Model> ModelPtr;
typedef kmlbase::Rc<MultiGeometry> MultiGeometryPtr;
typedef kmlbase::Rc<NetworkLink> NetworkLinkPtr;
typedef kmlbase::Rc<NetworkLinkControl> NetworkLinkControlPtr;
typedef kmlbase::Rc<Orientation> OrientationPtr;
typedef kmlbase::Rc<Pair> PairPtr;
typedef kmlbase::Rc<PhotoOverlay> PhotoOverlayPtr;
typedef kmlbase::Rc<Placemark> PlacemarkPtr;
typedef kmlbase::Rc<Point> PointPtr;
typedef kmlbase::Rc<PolyStyle> PolyStylePtr;
typedef kmlbase::Rc<Polygon> PolygonPtr;
typedef kmlbase::Rc<Region> RegionPtr;
typedef kmlbase::Rc<ResourceMap> ResourceMapPtr;
typedef kmlbase::Rc<Scale> ScalePtr;
typedef kmlbase::Rc<Schema> SchemaPtr;
typedef kmlbase::Rc<SchemaData> SchemaDataPtr;
typedef kmlbase::Rc<ScreenOverlay> ScreenOverlayPtr;
typedef kmlbase::Rc<SimpleData> SimpleDataPtr;
typedef kmlbase::Rc<SimpleField> SimpleFieldPtr;
typedef kmlbase::Rc<Snippet> SnippetPtr;
typedef kmlbase::Rc<Style> StylePtr;
typedef kmlbase::Rc<StyleMap> StyleMapPtr;
typedef kmlbase::Rc<TimeSpan> TimeSpanPtr;
typedef kmlbase::Rc<TimeStamp> TimeStampPtr;
typedef kmlbase::Rc<Update> UpdatePtr;
typedef kmlbase::Rc<UpdateOperation> UpdateOperationPtr;
typedef kmlbase::Rc<Url> UrlPtr;
typedef kmlbase::Rc<ViewVolume> ViewVolumePtr;
typedef kmlbase::Rc<HotSpot> HotSpotPtr;
typedef kmlbase::Rc<InnerBoundaryIs> InnerBoundaryIsPtr;
typedef kmlbase::Rc<Kml> KmlPtr;
typedef kmlbase::Rc<LinkSnippet> LinkSnippetPtr;
typedef kmlbase::Rc<OuterBoundaryIs> OuterBoundaryIsPtr;
typedef kmlbase::Rc<OverlayXY> OverlayXYPtr;
typedef kmlbase::Rc<RotationXY> RotationXYPtr;
typedef kmlbase::Rc<ScreenXY> ScreenXYPtr;
typedef kmlbase::Rc<Size> SizePtr;

typedef kmlbase::Rc<XalAddressDetails> XalAddressDetailsPtr;
typedef kmlbase::Rc<XalAdministrativeArea> XalAdministrativeAreaPtr;
typedef kmlbase::Rc<XalCountry> XalCountryPtr;
typedef kmlbase::Rc<XalLocality> XalLocalityPtr;
typedef kmlbase::Rc<XalPostalCode> XalPostalCodePtr;
typedef kmlbase::Rc<XalSubAdministrativeArea> XalSubAdministrativeAreaPtr;
typedef kmlbase::Rc<XalThoroughfare> XalThoroughfarePtr;

typedef kmlbase::Rc<GxAnimatedUpdate> GxAnimatedUpdatePtr;
typedef kmlbase::Rc<GxFlyTo> GxFlyToPtr;
typedef kmlbase::Rc<GxLatLonQuad> GxLatLonQuadPtr;
typedef kmlbase::Rc<GxMultiTrack> GxMultiTrackPtr;
typedef kmlbase::Rc<GxPlaylist> GxPlaylistPtr;
typedef kmlbase::Rc<GxSimpleArrayField> GxSimpleArrayFieldPtr;
typedef kmlbase::Rc<GxSimpleArrayData> GxSimpleArrayDataPtr;
typedef kmlbase::Rc<GxSoundCue> GxSoundCuePtr;
typedef kmlbase::Rc<GxTimeSpan> GxTimeSpanPtr;
typedef kmlbase::Rc<GxTimeStamp> GxTimeStampPtr;
typedef kmlbase::Rc<GxTour> GxTourPtr;
typedef kmlbase::Rc<GxTourControl> GxTourControlPtr;
typedef kmlbase::Rc<GxTourPrimitive> GxTourPrimitivePtr;
typedef kmlbase::Rc<GxTrack> GxTrackPtr;
typedef kmlbase::Rc<GxWait> GxWaitPtr;

}  // end namespace kmldom

#endif  // KML_DOM_KML_PTR_H__
