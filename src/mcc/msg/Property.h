#pragma once
#include "mcc/Config.h"
#include "mcc/Rc.h"
#include "mcc/msg/Objects.h"
#include <bmcl/Fwd.h>
#include <bmcl/OptionRc.h>
#include <vector>
#include <functional>

class QWidget;

namespace mccmsg
{

class PropertyDescription;
using PropertyDescriptionPtr = bmcl::Rc<const PropertyDescription>;
using PropertyDescriptionPtrs = std::vector<PropertyDescriptionPtr>;
class IPropertyValue;
using IPropertyValuePtr = bmcl::Rc<const IPropertyValue>;
using IPropertyValues = std::vector<IPropertyValuePtr>;

class MCC_MSG_DECLSPEC IPropertyValue : public mcc::RefCountable
{
public:
    IPropertyValue(const PropertyDescriptionPtr&);
    ~IPropertyValue() override;
    const PropertyDescriptionPtr& property() const;
    virtual bool isHidden() const { return false; }
    virtual bool operator==(const mccmsg::IPropertyValuePtr& other) const = 0;
    virtual bool operator!=(const mccmsg::IPropertyValuePtr& other) const = 0;
    virtual std::string encode() const = 0;
private:
    PropertyDescriptionPtr _p;
};

class MCC_MSG_DECLSPEC PropertyEditor : public mcc::RefCountable
{
public:
    using ValueChangedCallback = std::function<void()>;
    PropertyEditor():  _changedCallback([]() {}) {}
    virtual ~PropertyEditor();
    virtual QWidget* widget() const = 0;
    virtual void set(const IPropertyValuePtr& value) = 0;
    virtual IPropertyValuePtr get() const = 0;

    void setValueChangedCallback(const ValueChangedCallback& callback) { _changedCallback = callback; }
    const ValueChangedCallback& valueChangedCallback() const { return _changedCallback; }
private:
    ValueChangedCallback _changedCallback;
};
using PropertyEditorCreator = std::function<PropertyEditor*(const mccmsg::PropertyDescriptionPtr& ptr)>;

class MCC_MSG_DECLSPEC PropertyValues
{
public:
    PropertyValues();
    explicit PropertyValues(IPropertyValues&& values);
    explicit PropertyValues(const IPropertyValues& values);
    ~PropertyValues();
    bool operator==(const PropertyValues& other) const;
    bool operator!=(const PropertyValues& other) const;
    const IPropertyValues& values() const;

    bool contains(const Property&) const;
    bool contains(const PropertyDescription&) const;
    bool contains(const PropertyDescriptionPtr&) const;

    bmcl::OptionRc<const IPropertyValue> get(const Property&) const;
    bmcl::OptionRc<const IPropertyValue> get(const PropertyDescription&) const;
    bmcl::OptionRc<const IPropertyValue> get(const PropertyDescriptionPtr&) const;

    bool set(IPropertyValuePtr&&);
    bool add(const PropertyDescriptionPtr&);
    bool add(IPropertyValuePtr&&);
    bool remove(const Property&);
    bool remove(const IPropertyValuePtr&);
private:
    void sort();
    IPropertyValues _values;
};

using DefaultMaker = std::function<IPropertyValuePtr(const PropertyDescriptionPtr&)>;
using PropertyDecoder = std::function<bmcl::OptionRc<const IPropertyValue>(const PropertyDescription*, bmcl::StringView)>;

class MCC_MSG_DECLSPEC PropertyDescription : public mcc::RefCountable
{
public:
    PropertyDescription(const Property& property, bmcl::StringView info, bmcl::StringView path, PropertyEditorCreator&&, DefaultMaker&&, PropertyDecoder&&);
    virtual ~PropertyDescription();
    const std::string& pixmap() const;
    const std::string& info() const;
    const Property& name() const;
    const PropertyEditorCreator& editor() const;
    IPropertyValuePtr defaultValue() const;
    bmcl::OptionRc<const IPropertyValue> decode(bmcl::StringView) const;
private:
    Property    _property;
    std::string _info;
    std::string _pixmap;
    PropertyEditorCreator _editor;
    DefaultMaker  _defaultMaker;
    PropertyDecoder _decoder;
};

}
