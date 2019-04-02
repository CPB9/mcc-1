#pragma once

#include "mcc/map/TilePosition.h"

#include <string>
#include <random>
#include <cstring>

namespace mccmap {

struct EmptyGen {
public:
    std::string operator()(const TilePosition& pos)
    {
        (void)pos;
        return std::string();
    }
};

class Rng {
public:
    Rng(int min, int max)
        : dist(min, max)
    {
    }

    int generate()
    {
        return dist(randomEndine);
    }

private:
    std::default_random_engine randomEndine;
    std::uniform_int_distribution<int> dist;
};

struct GenericGen {
public:
    GenericGen(const char* first, const char* second,
               const char* zoom, const char* key, const char* keyValue,
               int min, int max,
               const std::function<int(int)>& zoomConverter)
        : _firstStr(first)
        , _secondStr(second)
        , _zoomStr(zoom)
        , _keyStr(key)
        , _keyValueStr(keyValue)
        , _zoomConverter(zoomConverter)
        , hostRng(min, max)
        , sRng(1, std::strlen(keyValue))
    {
    }

    std::string operator()(const TilePosition& pos)
    {
        std::string url = "http://";
        url.append(_firstStr);
        url.push_back('0' + hostRng.generate());
        url.append(_secondStr);
        url.append("&x=");
        url.append(std::to_string(pos.globalOffsetX));
        url.append("&y=");
        url.append(std::to_string(pos.globalOffsetY));
        url.push_back('&');
        url.append(_zoomStr);
        url.push_back('=');
        url.append(std::to_string(_zoomConverter(pos.zoomLevel)));
        url.push_back('&');
        url.append(_keyStr);
        url.push_back('=');
        url.append(_keyValueStr, sRng.generate());
        return url;
    }

private:
    const char* _firstStr;
    const char* _secondStr;
    const char* _zoomStr;
    const char* _keyStr;
    const char* _keyValueStr;
    std::function<int(int)> _zoomConverter;
    Rng hostRng;
    Rng sRng;
};

struct OsmGen {
    OsmGen(const char* baseUrlPrefix, const char* baseUrl, char randomCharStart, int maxChar)
        : _baseUrlPrefix(baseUrlPrefix)
        , _baseUrl(baseUrl)
        , _randomCharStart(randomCharStart)
        , _rng(0, maxChar - 1)
    {
    }

    std::string operator()(const TilePosition& pos)
    {
        std::string url = "https://";
        url.append(_baseUrlPrefix);
        url.push_back(_randomCharStart + _rng.generate());
        url.append(_baseUrl);
        url.push_back('/');
        url.append(std::to_string(pos.zoomLevel));
        url.push_back('/');
        url.append(std::to_string(pos.globalOffsetX));
        url.push_back('/');
        url.append(std::to_string(pos.globalOffsetY));
        url.append(".png");
        return url;
    }

private:
    const char* _baseUrlPrefix;
    const char* _baseUrl;
    char _randomCharStart;
    Rng _rng;
};

struct NokiaGen {
    NokiaGen(const char* name)
        : _hostRng(0, 2)
        , _name(name)
    {
    }

    std::string operator()(const TilePosition& pos)
    {
        std::string url = "http://";
        url.push_back('1' + _hostRng.generate());
        url.append(".maps.nlp.nokia.com/maptile/2.1/maptile/newest/");
        url.append(_name);
        url.append(".day/");
        url.append(std::to_string(pos.zoomLevel));
        url.push_back('/');
        url.append(std::to_string(pos.globalOffsetX));
        url.push_back('/');
        url.append(std::to_string(pos.globalOffsetY));
        url.push_back('/');
        url.append("/256/png8?lg=RUS&app_id=SqE1xcSngCd3m4a1zEGb&token=r0sR1DzqDkS6sDnh902FWQ&xnlp=CL_JSMv2.5.0.3,SID_34725629-4FBE-4EA5-A2AB-52EA24DEA26F");
        return url;
    }

private:
    Rng _hostRng;
    const char* _name;
};
}
