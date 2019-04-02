#pragma once

#include <QFrame>
#include <QValidator>
#include <QLineEdit>
#include <vector>

#include "mcc/Config.h"
#include "mcc/ui/CoordinateSystemController.h"

#include <bmcl/Fwd.h>

class QEvent;
class QHBoxLayout;
class QLabel;

class QValidator;
class QIntValidator;
class QDoubleValidator;
class QRegExpValidator;

namespace mccui {

class CoordinateLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    enum class ValidatorType
    {
        SignedDouble = 0,
        UnsignedDouble,
        SignedInt,
        UnsignedInt
    };
    enum class EditorType
    {
        DegreesEditor = 0,
        MinutesEditor,
        SecondsEditor,
        LinearEditor
    };

    CoordinateLineEdit(QWidget *parent = nullptr);

    static constexpr int precision() {return 4;} // Base precision. Symbols count after dot.

    void setType(EditorType type, const CoordinateFormat& format);
    EditorType type() const {return _editorType;}
    ValidatorType validatorType() const {return _validatorType;}
    int leftSize() const {return _leftSize;}
    int rightSize() const {return _rightSize;}

    void setInternalUpdating(bool internalUpdating);
    bool isInternalUpdating() const {return _internalUpdating;}

private:
    EditorType    _editorType;
    ValidatorType _validatorType;
    int           _leftSize;
    int           _rightSize;
    bool          _internalUpdating;

    Q_DISABLE_COPY(CoordinateLineEdit)
};

class CoordinateValueKeeper
{
public:
    enum class InputErrors
    {
        NoErrors = 0,
        IncorrectSymbols,
        Oversize,
        ValueLimits,
        MinutesLimits,
        SecondsLimits
    };

    CoordinateValueKeeper(const std::vector<CoordinateLineEdit*>* editors);
    ~CoordinateValueKeeper();

    QRegularExpressionValidator* validator(CoordinateLineEdit::ValidatorType type) const;

    void setMaxValue(double maxValue);
    double maxValue() const {return _maxValue;}

    void setValue(double value);
    double value() const {return _value;}
    void generateTexts() const;

    void setRange(double bottom, double top);
    double bottom() const { return _bottom; }
    double top() const { return _top; }

    CoordinateLineEdit* editor(size_t index) const;
    const std::vector<CoordinateLineEdit*>* editors() const {return _editors;}
    size_t editorsCount() const;
    size_t activeEditorsCount() const {return _activeEditorsCount;}

    void setFormat(const CoordinateFormat& format);
    const CoordinateFormat& format() const {return _format;}

    void setLastError(InputErrors error);
    InputErrors lastError() const {return _lastError;}
    bool hasErrors() const {return _lastError != InputErrors::NoErrors;}

    static void updateEditorText(CoordinateLineEdit *editor, const QString &text);

private:
    static double rounded(double value, int precision);
    template<typename T>
    static QString addZero(T number, const QString &text);
    static QString rememberMinus(int degrees, double value);
    bool areEditorsCorrect() const {return (_editors && _editors->size() >= 3);}

private:
    double                                     _maxValue;
    std::vector<QRegularExpressionValidator *> _validators;

    double _value;
    double _bottom;
    double _top;

    CoordinateFormat                        _format;
    const std::vector<CoordinateLineEdit*>* _editors;
    size_t                                  _activeEditorsCount;
    InputErrors                             _lastError;

private:
    Q_DISABLE_COPY(CoordinateValueKeeper)
};


class CoordinateValidator : public QValidator
{
    Q_OBJECT

public:
    explicit CoordinateValidator(CoordinateLineEdit* editor, CoordinateValueKeeper* valueKeeper, QObject* parent = 0);

    QValidator::State validate(QString &input, int &pos) const;

signals:
    void valueChanged(double value) const;
    void incorrectInput() const;

private:
    QValidator::State tryValidate(QString &input, int &pos) const;
    bmcl::Option<double> valueFromText(const QString &input) const;
    QValidator::State trySetValue(const std::vector<double> &values, QValidator::State previousResult, bool negative = false) const;
    QValidator::State trySetValue(double degrees, double minutes, double seconds, QValidator::State previousResult, bool negative = false) const;
    QValidator::State tryUnderstandEditor(const QString &text, QValidator::State result) const;
    bool isEditorCorrect() const {return (_valueKeeper && _editor);}

private:
    CoordinateLineEdit    *_editor;
    CoordinateValueKeeper *_valueKeeper;

private:
    Q_DISABLE_COPY(CoordinateValidator)
};


class MCC_UI_DECLSPEC CoordinateEditor : public QFrame
{
    Q_OBJECT

public:
    CoordinateEditor(QWidget* parent = nullptr);
    ~CoordinateEditor() override;

    double value() const;
    void setMinMax(double min, double max);

    void selectInput();

signals:
    void valueChanged(double value) const;

public slots:
    void setCoordinateFormat(const CoordinateFormat& fmt);
    void setValue(double value);

private:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void createValidators();

    void addLineEdit();
    void addSeparator(QChar symbol);
    void addToLayout(QWidget* widget, int stretch);
    void moveNextLineEdit(std::size_t i);
    void movePrevLineEdit(std::size_t i);
    void clear();
    void setDigitGroupVisible(size_t index, bool visible);

    size_t activeEditorsCount() const;

private slots:
    void informIncorrectInput();

private:
    QHBoxLayout*                     _layout;
    std::vector<CoordinateLineEdit*> _editors;
    std::vector<QLabel*>             _separators;

    CoordinateValueKeeper*           _valueKeeper;
};


template<typename T>
QString CoordinateValueKeeper::addZero(T number, const QString &text)
{
    if(number < 10)
        return "0" + text;
    else
        return text;
}
}
