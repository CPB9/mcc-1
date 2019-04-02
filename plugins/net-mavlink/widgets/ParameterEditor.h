#pragma once

#include "../Firmware.h"
#include "mcc/ui/Dialog.h"

class QLineEdit;
class QComboBox;
class QCheckBox;

namespace mccmav {

class EditorWidget : public QWidget
{
public:
    EditorWidget(QWidget* parent) : QWidget(parent) {}

    virtual void setValue(const ParameterDescription& desc, const mccmsg::NetVariant& value) = 0;
    virtual mccmsg::NetVariant value() const = 0;
};

class NumberEditor : public EditorWidget
{
public:
    explicit NumberEditor(QWidget* parent);

    virtual void setValue(const ParameterDescription& desc, const mccmsg::NetVariant& value) override;
    virtual mccmsg::NetVariant value() const override;

private:
    ParameterDescription _desc;
    QLineEdit* _editor;
};

class EnumEditor : public EditorWidget
{
public:
    explicit EnumEditor(QWidget* parent);

    virtual void setValue(const ParameterDescription& desc, const mccmsg::NetVariant& value) override;
    virtual mccmsg::NetVariant value() const override;
private:
    QComboBox* _editor;
};

class ParameterEditor : public mccui::Dialog
{
public:
    ParameterEditor(QWidget* parent, const ParameterDescription& description, const mccmsg::NetVariant& param);

    mccmsg::NetVariant value() const;
private:
    EditorWidget* createEditor(const ParameterDescription& desc, const mccmsg::NetVariant& value);

    ParameterDescription _description;
    EditorWidget* _editor;
};
}