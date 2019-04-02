#include "../widgets/ParameterEditor.h"

#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>

namespace mccmav {

ParameterEditor::ParameterEditor(QWidget* parent, const ParameterDescription& description, const mccmsg::NetVariant& param)
    : mccui::Dialog(parent)
    , _description(description)
{
    setModal(true);
    setWindowTitle("Изменить параметр");

    setStyleSheet("background-color:white");

    QLabel* idLabel = new QLabel(QString::fromStdString(description.name), this);
    QLabel* shortDesc = new QLabel(QString::fromStdString(description.shortDesc), this);
    QLabel* longDesc = new QLabel(QString::fromStdString(description.longDesc), this);

    shortDesc->setWordWrap(true);
    longDesc->setWordWrap(true);

    QVBoxLayout* rootLayout = new QVBoxLayout();
    setLayout(rootLayout);

    QFormLayout* infoLayout = new QFormLayout();
    infoLayout->addRow("Имя:", idLabel);
    infoLayout->addRow("Краткое описание:", shortDesc);
    infoLayout->addRow("Полное описание:", longDesc);
    infoLayout->addRow("По умолчанию:", new QLabel(QString::number(description.defaultValue)));
    infoLayout->addRow("Минимум:", new QLabel(QString::number(description.min)));
    infoLayout->addRow("Максимум:", new QLabel(QString::number(description.max)));
    rootLayout->addLayout(infoLayout);

    QHBoxLayout* valueLayout = new QHBoxLayout();
    _editor = createEditor(description, param);
    QPushButton* resetButton = new QPushButton("Reset", this);
    valueLayout->addWidget(_editor);
    valueLayout->addWidget(resetButton);
    rootLayout->addLayout(valueLayout);

    rootLayout->addStretch();
    QDialogButtonBox* dialogButtons = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, this);
    rootLayout->addWidget(dialogButtons);

    connect(dialogButtons, &QDialogButtonBox::accepted, this, &ParameterEditor::accept);
    connect(dialogButtons, &QDialogButtonBox::rejected, this, &ParameterEditor::reject);
    connect(resetButton, &QPushButton::clicked, this,
            [this]() {
                _editor->setValue(_description, _description.defaultValue);
            }
    );
}

mccmsg::NetVariant ParameterEditor::value() const
{
    assert(_editor);

    return _editor->value();
}

EditorWidget* ParameterEditor::createEditor(const ParameterDescription& desc, const mccmsg::NetVariant& value)
{
    EditorWidget* editor;
    if (!desc.values.empty())
    {
        editor = new EnumEditor(this);
    }
    else
    {
        editor = new NumberEditor(this);
    }
    editor->setValue(desc, value);
    return editor;
}

NumberEditor::NumberEditor(QWidget* parent)
    : EditorWidget(parent)
    , _editor(new QLineEdit(this))
{
    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget(_editor);
    setLayout(layout);
}

void NumberEditor::setValue(const ParameterDescription& desc, const mccmsg::NetVariant& value)
{
    _desc = desc;
    _editor->setText(value.qstringify());
}

mccmsg::NetVariant NumberEditor::value() const
{
    if (_desc.type == "u32")
    {
        return mccmsg::NetVariant((float)_editor->text().toUInt());
    }
    else if (_desc.type == "i32")
    {
        return mccmsg::NetVariant((float)_editor->text().toInt());
    }
    else if (_desc.type == "f32")
    {
        return mccmsg::NetVariant((float)_editor->text().toFloat());
    }
    assert(false);

    return mccmsg::NetVariant();
}

EnumEditor::EnumEditor(QWidget* parent)
    : EditorWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout();
    _editor = new QComboBox(this);
    layout->addWidget(_editor);
    setLayout(layout);
}

void EnumEditor::setValue(const ParameterDescription& desc, const mccmsg::NetVariant& value)
{
    assert(!desc.values.empty());

    _editor->clear();
    for (const auto& v : desc.values)
    {
        _editor->addItem(QString::fromStdString(v.second), v.first);
    }

    auto currentIndex = _editor->findData(value.toQVariant());
    if (currentIndex == -1)
    {
        assert(false);
        return;
    }
    _editor->setCurrentIndex(currentIndex);
}

mccmsg::NetVariant EnumEditor::value() const
{
    return  _editor->currentData().toUInt();
}
}
