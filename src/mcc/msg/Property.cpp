#include "mcc/msg/Objects.h"
#include "mcc/msg/Property.h"
#include <bmcl/StringView.h>
#include <bmcl/Result.h>
#include <bmcl/Uuid.h>
#include <bmcl/StringView.h>
#include <bmcl/Logging.h>
#include <bmcl/OptionUtils.h>
#include <bmcl/Option.h>
#include <bmcl/MakeRc.h>

namespace mccmsg
{

PropertyDescription::PropertyDescription(const Property& property, bmcl::StringView info, bmcl::StringView pixmap, PropertyEditorCreator&& editor, DefaultMaker&& maker, PropertyDecoder&& decoder)
    : _property(property), _info(info.toStdString()), _pixmap(pixmap.toStdString()), _editor(std::move(editor)), _defaultMaker(std::move(maker)), _decoder(std::move(decoder))
{
}
PropertyDescription::~PropertyDescription() {}
const Property& PropertyDescription::name() const { return _property; }
const std::string& PropertyDescription::pixmap() const { return _pixmap; }
const std::string& PropertyDescription::info() const { return _info; }
const PropertyEditorCreator& PropertyDescription::editor() const { assert(_editor); return _editor; }
IPropertyValuePtr PropertyDescription::defaultValue() const { assert(_defaultMaker); return _defaultMaker(bmcl::wrapRc(this)); }
bmcl::OptionRc<const IPropertyValue> PropertyDescription::decode(bmcl::StringView value) const { assert(_decoder); return _decoder(this, value); }

IPropertyValue::IPropertyValue(const PropertyDescriptionPtr& p) : _p(p) {}
IPropertyValue::~IPropertyValue() {}
const PropertyDescriptionPtr& IPropertyValue::property() const { return _p; }

PropertyEditor::~PropertyEditor() {}

PropertyValues::PropertyValues() {}
PropertyValues::~PropertyValues() {}
PropertyValues::PropertyValues(IPropertyValues&& values) : _values(std::move(values)) {}
PropertyValues::PropertyValues(const IPropertyValues& values) : _values(values) {}
const IPropertyValues& PropertyValues::values() const { return _values; }

void PropertyValues::sort()
{
    std::sort(_values.begin(), _values.end(), [](const auto& l, const auto& r) { return r->property()->name() > l->property()->name(); });
}

bool PropertyValues::add(IPropertyValuePtr&& value)
{
    auto i = std::find_if(_values.begin(), _values.end(), [&value](const IPropertyValuePtr& p) { return value->property()->name() == p->property()->name(); });
    if (i != _values.end())
    {
        *i = std::move(value);
    }
    else
    {
        _values.emplace_back(std::move(value));
        sort();
    }
    return false;
}

bool PropertyValues::add(const PropertyDescriptionPtr& d)
{
    auto i = std::find_if(_values.begin(), _values.end(), [&d](const IPropertyValuePtr& p) { return d->name() == p->property()->name(); });
    if (i != _values.end())
        return false;

    _values.emplace_back(d->defaultValue());
    sort();
    return false;
}

bool PropertyValues::set(IPropertyValuePtr&& value)
{
    auto i = std::find_if(_values.begin(), _values.end(), [&value](const IPropertyValuePtr& p) { return value->property()->name() == p->property()->name(); });
    if (i == _values.end())
        return false;
    *i = std::move(value);
    return true;
}

bool PropertyValues::remove(const IPropertyValuePtr& p)
{
    return remove(p->property()->name());
}

bool PropertyValues::remove(const Property& p)
{
    const auto& i = std::find_if(_values.begin(), _values.end(), [&p](const auto& f) { return p == f->property()->name(); });
    if (i == _values.end())
        return false;
    _values.erase(i);
    return true;
}

bool PropertyValues::operator!=(const PropertyValues& other) const { return !(*this == other); }

bool PropertyValues::operator==(const PropertyValues& other) const
{
    if (_values.size() != other._values.size())
        return false;

    for (std::size_t i = 0; i < _values.size(); ++i)
    {
        if (_values[i]->property()->name() != other._values[i]->property()->name())
            return false;
        if (_values[i] != other._values[i])
            return false;
    }
    return true;
}

bool PropertyValues::contains(const PropertyDescriptionPtr& f) const { return contains(f->name()); }
bool PropertyValues::contains(const PropertyDescription& f) const { return contains(f.name()); }
bool PropertyValues::contains(const Property& f) const
{
    const auto i = std::find_if(_values.begin(), _values.end(), [&f](const IPropertyValuePtr& p) { return f == p->property()->name(); });
    return i != _values.end();
}

bmcl::OptionRc<const IPropertyValue> PropertyValues::get(const PropertyDescription& p) const { return get(p.name()); }
bmcl::OptionRc<const IPropertyValue> PropertyValues::get(const PropertyDescriptionPtr& p) const { return get(p->name()); }
bmcl::OptionRc<const IPropertyValue> PropertyValues::get(const Property& f) const
{
    const auto i = std::find_if(_values.begin(), _values.end(), [&f](const IPropertyValuePtr& p) { return f == p->property()->name(); });
    if (i == _values.end())
        return bmcl::None;
    return *i;
}

// PropertyValues PropertyValues::makeDefault(const PropertyDescriptionPtrs& ps)
// {
//     IPropertyValues values;
//     values.reserve(ps.size());
//     for (const auto& i : ps)
//     {
//         values.push_back(i->makeDefault());
//     }
//     return PropertyValues(std::move(values));
// }

}