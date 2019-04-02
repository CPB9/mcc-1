#include "mcc/ui/CoordinateEditor.h"

#include <QHBoxLayout>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QKeyEvent>
#include <QLabel>
#include <QDebug>
#include <QLocale>
#include <QToolTip>

#include <bmcl/Option.h>

#include <cmath>

namespace mccui {

static inline double tryConvertToDouble(const QString& text, bool* ok = 0)
{
    QLocale locale = QLocale::system();
    QString tmp = text;
    tmp = tmp.replace('.', locale.decimalPoint());
    tmp = tmp.replace(',', locale.decimalPoint());

    tmp.remove('+');

    double result = locale.toDouble(tmp, ok);
    return result;
}

CoordinateEditor::CoordinateEditor(QWidget *parent) :
    QFrame(parent),
    _layout(new QHBoxLayout(this)),
    _editors(),
    _separators(),
    _valueKeeper(0)
{
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Sunken);

    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->setSpacing(0);
    _layout->addStretch(1);

    addLineEdit();
    addSeparator(QChar(0260));
    addLineEdit();
    addSeparator('\'');
    addLineEdit();
    addSeparator('\"');

    _layout->addStretch(1);


    _valueKeeper = new CoordinateValueKeeper(&_editors);
    createValidators();
    setCoordinateFormat(CoordinateFormat(AngularFormat::DegreesMinutesSeconds));
    _editors.at(0)->setStyleSheet("QToolTip{"
                                  "background: red;"
                                  "color: white;"
                                  "border-color: white;"
                                  "border-style: solid;"
                                  "border-width: 1px;"
#ifndef _MSC_VER
                                  "border-radius: 6px;"
                                  "margin: 3px;"
#endif
                                  "}");
}

CoordinateEditor::~CoordinateEditor()
{
    delete _valueKeeper;
}

bool CoordinateEditor::eventFilter(QObject *obj, QEvent *event)
{
    const auto it = std::find(_editors.begin(), _editors.end(), obj);
    if (it == _editors.end())
        return QFrame::eventFilter(obj, event);

    CoordinateLineEdit* pEdit = *it;
    size_t index = std::distance(_editors.begin(), it);

    if (event->type() == QEvent::KeyPress)
    {
        if(!pEdit->hasSelectedText())
        {
            QKeyEvent* pEvent = static_cast<QKeyEvent*>(event);
            switch (pEvent->key())
            {
            case Qt::Key_Left:
                if (pEdit->cursorPosition() == 0)
                {
                    movePrevLineEdit(index);
                    return true;
                }
                break;

            case Qt::Key_Right:
                if (pEdit->text().isEmpty() || (pEdit->text().size() == pEdit->cursorPosition()))
                {
                    moveNextLineEdit(index);
                    return true;
                }
                break;
            case Qt::Key_Backspace:
                if (pEdit->text().isEmpty() || pEdit->cursorPosition() == 0)
                {
                    movePrevLineEdit(index);
                    return true;
                }
                break;
            }
        }
    }
    else if (event->type() == QEvent::FocusIn ||
             event->type() == QEvent::MouseButtonPress)
    {
        QString text = pEdit->text();

        if(!text.isEmpty())
        {
            if(text.startsWith("00")) // remove double zero at editing start
            {
                text.remove(0, 1);
            }

            int dotPos = text.indexOf(QRegularExpression("[.,]"));
            if(dotPos != -1)
            {
                int delCount(0);

                for(int i = text.size() - 1; i >= dotPos; --i)
                {
                    if(text.at(i) == '0' || text.at(i) == '.' || text.at(i) == ',')
                    {
                            ++delCount;
                    }
                    else
                    {
                        break;
                    }
                }
                text = text.left(text.size() - delCount);
            }

            _valueKeeper->updateEditorText(pEdit, text);
        }

        if(event->type() == QEvent::FocusIn)
        {
            QFocusEvent *fEvent = dynamic_cast<QFocusEvent*>(event);
            if(fEvent != nullptr && fEvent->reason() != Qt::MouseFocusReason)
            {
                pEdit->setCursorPosition(0);
                pEdit->selectAll();
            }
        }
    }
    else if (event->type() == QEvent::FocusOut)
    {
        _valueKeeper->generateTexts();

        return false;
    }
    return QFrame::eventFilter(obj, event);
}

void CoordinateEditor::createValidators()
{
    if(_valueKeeper)
    {
        for(std::vector<CoordinateLineEdit*>::iterator it = _editors.begin();
            it != _editors.end(); ++it)
        {
            CoordinateLineEdit* pEdit = *it;

            CoordinateValidator *validator = new CoordinateValidator(pEdit, _valueKeeper, this);
            pEdit->setValidator(validator);
            connect(validator, &CoordinateValidator::valueChanged, this,
                [this]()
                {
                    if(QToolTip::isVisible())
                    {
                        QToolTip::hideText();
                    }
                    emit valueChanged(value());
                }
            );
            connect(validator, &CoordinateValidator::incorrectInput, this, &CoordinateEditor::informIncorrectInput);
        }
    }
}

double CoordinateEditor::value() const // WGS, SK or LCS
{
    return _valueKeeper->value();
}

void CoordinateEditor::setMinMax(double min, double max)
{
    _valueKeeper->setRange(min, max);
}

void CoordinateEditor::selectInput()
{
    _editors[0]->setFocus();
}

void CoordinateEditor::addLineEdit()
{
    CoordinateLineEdit* pEdit = new CoordinateLineEdit(this);
    pEdit->installEventFilter(this);
    pEdit->setFrame(false);
    pEdit->setAlignment(Qt::AlignCenter);

    QFont font = pEdit->font();
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    pEdit->setFont(font);

    addToLayout(pEdit, 1);

    connect(pEdit, &CoordinateLineEdit::textChanged, this,
        [this, pEdit]()
        {
            QString text = pEdit->text();
            if(text.isEmpty())
                text = "0"; // minimum width for one symbol
            QFontMetrics fm = pEdit->fontMetrics();
            int w = fm.width(text);
            pEdit->setFixedWidth(w + 5);
        }
    );

    pEdit->setFixedWidth(0);

    _editors.push_back(pEdit);
}

void CoordinateEditor::addSeparator(QChar symbol)
{
    QLabel* pDot = new QLabel(symbol, this);
    pDot->setStyleSheet("background: white;");
    addToLayout(pDot, 0);
    _separators.push_back(pDot);
}

void CoordinateEditor::addToLayout(QWidget* widget, int stretch)
{
    _layout->addWidget(widget);
    _layout->setStretch(_layout->count(), stretch);
}

void CoordinateEditor::moveNextLineEdit(std::size_t i)
{
    ++i;
    if (i < activeEditorsCount())
    {
        _editors[i]->setFocus();
    }
}

void CoordinateEditor::movePrevLineEdit(std::size_t i)
{
    --i;
    if (i < activeEditorsCount())
    {
        _editors[i]->setFocus();
        _editors[i]->setCursorPosition(_editors[i]->text().size());
    }
}

void CoordinateEditor::clear()
{
    setDigitGroupVisible(0, false);
    setDigitGroupVisible(1, false);
    setDigitGroupVisible(2, false);
}

void CoordinateEditor::setDigitGroupVisible(size_t index, bool visible)
{
    if (index >= _editors.size())
    {
        Q_ASSERT(false);
        return;
    }

    _editors[index]->setVisible(visible);
    _separators[index]->setVisible(visible);
}

size_t CoordinateEditor::activeEditorsCount() const
{
    return _valueKeeper->activeEditorsCount();
}

void CoordinateEditor::informIncorrectInput()
{
    if(isVisible() && _valueKeeper->hasErrors())
    {
        QString mistakeText("<b>%1</b>");

        switch (_valueKeeper->lastError())
        {
            case CoordinateValueKeeper::InputErrors::IncorrectSymbols:
                mistakeText = mistakeText.arg("Недопустимый&nbsp;символ!");
                break;
            case CoordinateValueKeeper::InputErrors::Oversize:
                mistakeText = mistakeText.arg("Превышение&nbsp;длины!");
                break;
            case CoordinateValueKeeper::InputErrors::ValueLimits:
                mistakeText = mistakeText.arg("Выход&nbsp;за&nbsp;пределы!");
                break;
            case CoordinateValueKeeper::InputErrors::MinutesLimits:
                mistakeText = mistakeText.arg("Некорректные&nbsp;минуты!");
                break;
            case CoordinateValueKeeper::InputErrors::SecondsLimits:
                mistakeText = mistakeText.arg("Некорректные&nbsp;секунды!");
                break;
            default:
                break;
        }

        QToolTip::showText(mapToGlobal(QPoint(width(), height())), mistakeText, _editors.at(0));
    }
}

void CoordinateEditor::setCoordinateFormat(const CoordinateFormat& fmt)
{
    clear();

    if (fmt.isLinear()) {
        setDigitGroupVisible(0, true);
        _separators[0]->setText(fmt.unwrapLinear());
    } else {
        switch (fmt.unwrapAngular())
        {
        case AngularFormat::Degrees:
            setDigitGroupVisible(0, true);
            break;
        case AngularFormat::DegreesMinutes:
            setDigitGroupVisible(0, true);
            setDigitGroupVisible(1, true);
            break;
        case AngularFormat::DegreesMinutesSeconds:
            setDigitGroupVisible(0, true);
            setDigitGroupVisible(1, true);
            setDigitGroupVisible(2, true);
            break;
        }
        _separators[0]->setText("°");
    }

    _valueKeeper->setFormat(fmt);
    _valueKeeper->generateTexts();
}

void CoordinateEditor::setValue(double value) //WGS, SK or LCS
{
    _valueKeeper->setValue(value);
    _valueKeeper->generateTexts();
}

// CoordinateValueKeeper
CoordinateValueKeeper::CoordinateValueKeeper(const std::vector<CoordinateLineEdit *> *editors) :
    _maxValue(40000000.0), // for metrics
    _validators(),
    _value(0.0),
    _bottom(- _maxValue),
    _top(_maxValue),
    _format(AngularFormat::DegreesMinutesSeconds),
    _editors(editors),
    _activeEditorsCount(0),
    _lastError(InputErrors::NoErrors)
{
    _validators.reserve(4);

    const QString d("([,.]|[1-9]+\\d*[,.]?)\\d+");
    const QString i("[1-9]+\\d*");

    _validators.push_back(new QRegularExpressionValidator(QRegularExpression("[+-]?[0]?" + d)));
    _validators.push_back(new QRegularExpressionValidator(QRegularExpression("[0]{0,2}" + d)));

    _validators.push_back(new QRegularExpressionValidator(QRegularExpression("[+-]?[0]?" + i)));
    _validators.push_back(new QRegularExpressionValidator(QRegularExpression("[0]{0,2}" + i)));

    setFormat(CoordinateFormat(AngularFormat::DegreesMinutesSeconds));
}

CoordinateValueKeeper::~CoordinateValueKeeper()
{
    for(std::vector<QRegularExpressionValidator *>::iterator it = _validators.begin();
        it != _validators.end(); ++it)
    {
        delete (*it);
    }
}

QRegularExpressionValidator *CoordinateValueKeeper::validator(CoordinateLineEdit::ValidatorType type) const
{
    return _validators.at(static_cast<size_t>(type));
}

void CoordinateValueKeeper::setMaxValue(double maxValue)
{
    _maxValue = maxValue;
    setRange(_bottom, _top);
}

void CoordinateValueKeeper::setValue(double value)
{
    if(value > _top)
    {
        value = _top;
    }
    if(value < _bottom)
    {
        value = _bottom;
    }

    //int prec = CoordinateLineEdit::precision() + 2 * 2; // degree precision


    //double newValue = rounded(value, prec);

    //if(std::abs(newValue - _value) >= std::pow(10.0, - (prec + 1)))
    //{
    //    _value = newValue;
    //}

    _value = value;

    setLastError(CoordinateValueKeeper::InputErrors::NoErrors);
}

void CoordinateValueKeeper::generateTexts() const
{
    if(areEditorsCorrect())
    {
        CoordinateLineEdit *floatEditor = nullptr;
        if (_format.isLinear()) {
            floatEditor = _editors->at(0);
            double metersFloat = rounded(value(), floatEditor->rightSize());
            updateEditorText(floatEditor, QString::number(metersFloat, 'f', floatEditor->rightSize()));
            return;
        }
        floatEditor = _editors->at((size_t)_format.unwrapAngular());

        switch (_format.unwrapAngular())
        {
        case AngularFormat::DegreesMinutesSeconds:
        {
            double minutesFloat;
            double secondsFloat;
            int degrees = CoordinateSystemController::decomposeDegree(value(), &minutesFloat);
            int minutes = CoordinateSystemController::decomposeDegree(minutesFloat, &secondsFloat);

            secondsFloat = rounded(secondsFloat, floatEditor->rightSize());

            if(secondsFloat >= 60.0)
            {
                secondsFloat = secondsFloat - 60.0;
                minutes = minutes + 1;

                if(minutes >= 60.0)
                {
                    minutes = minutes - 60;
                    degrees = degrees + (value() < 0 ? -1 : 1);
                }
            }

            updateEditorText(_editors->at(0), rememberMinus(degrees, value()));
            updateEditorText(_editors->at(1), addZero(minutes, QString::number(minutes)));
            updateEditorText(floatEditor, addZero(secondsFloat, QString::number(secondsFloat, 'f', floatEditor->rightSize())));
            break;
        }
        case AngularFormat::DegreesMinutes:
        {
            CoordinateLineEdit *floatEditorDm = _editors->at(1);

            double minutesFloat;
            int degrees = CoordinateSystemController::decomposeDegree(value(), &minutesFloat);

            minutesFloat = rounded(minutesFloat, floatEditorDm->rightSize());

            if(minutesFloat >= 60.0)
            {
                minutesFloat = minutesFloat - 60.0;
                degrees = degrees + (value() < 0 ? -1 : 1);
            }

            updateEditorText(_editors->at(0), rememberMinus(degrees, value()));
            updateEditorText(floatEditorDm, addZero(minutesFloat, QString::number(minutesFloat, 'f', floatEditorDm->rightSize())));
            break;
        }
        case AngularFormat::Degrees:
        {
            double degreesFloat = rounded(value(), floatEditor->rightSize());
            updateEditorText(floatEditor, QString::number(degreesFloat, 'f', floatEditor->rightSize()));
            break;
        }
        }
    }
}

void CoordinateValueKeeper::setRange(double bottom, double top)
{
    if(std::abs(bottom) > _maxValue)
    {
        bottom = _maxValue * (bottom < 0.0 ? -1.0 : 1.0);
    }
    if(std::abs(top) > _maxValue)
    {
        top = _maxValue * (top < 0.0 ? -1.0 : 1.0);
    }

    _bottom = bottom;
    _top = top;
}

CoordinateLineEdit *CoordinateValueKeeper::editor(size_t index) const
{
    if(_editors && index < _editors->size())
    {
        return _editors->at(index);
    }

    return 0;
}

size_t CoordinateValueKeeper::editorsCount() const
{
    if(_editors)
        return _editors->size();
    else
        return 0;
}

void CoordinateValueKeeper::setFormat(const CoordinateFormat& format)
{
    _format = format;

    if(!areEditorsCorrect())
    {
        _activeEditorsCount = 0;
        return;
    }

    if (format.isLinear()) {
        _activeEditorsCount = 1;
        _editors->at(0)->setType(CoordinateLineEdit::EditorType::LinearEditor, format);
    } else {
        switch (format.unwrapAngular())
        {
            case AngularFormat::DegreesMinutesSeconds:
                _activeEditorsCount = 3;
                _editors->at(0)->setType(CoordinateLineEdit::EditorType::DegreesEditor, format);
                _editors->at(1)->setType(CoordinateLineEdit::EditorType::MinutesEditor, format);
                _editors->at(2)->setType(CoordinateLineEdit::EditorType::SecondsEditor, format);
                break;
            case AngularFormat::DegreesMinutes:
                _activeEditorsCount = 2;
                _editors->at(0)->setType(CoordinateLineEdit::EditorType::DegreesEditor, format);
                _editors->at(1)->setType(CoordinateLineEdit::EditorType::MinutesEditor, format);
                break;
            case AngularFormat::Degrees:
                _activeEditorsCount = 1;
                _editors->at(0)->setType(CoordinateLineEdit::EditorType::DegreesEditor, format);
                break;
        }
    }
}

void CoordinateValueKeeper::setLastError(InputErrors error)
{
    _lastError = error;
}

double CoordinateValueKeeper::rounded(double value, int precision)
{
    return std::round(value * std::pow(10, precision)) * std::pow(10, - precision);
}

QString CoordinateValueKeeper::rememberMinus(int degrees, double value)
{
    if(degrees == 0 && value < 0.0)
        return QString::number(degrees).insert(0, '-');
    else
        return QString::number(degrees);
}

void CoordinateValueKeeper::updateEditorText(CoordinateLineEdit *editor, const QString &text)
{
    if(editor)
    {
        if(text != editor->text())
        {
            editor->setInternalUpdating(true);
            editor->setText(text);
        }
    }
}



// CoordinateLineEdit
CoordinateLineEdit::CoordinateLineEdit(QWidget *parent) :
    QLineEdit(parent),
    _internalUpdating(false)
{}

void CoordinateLineEdit::setType(CoordinateLineEdit::EditorType type, const CoordinateFormat& format)
{
    _editorType = type;

    if (format.isLinear()) {
        switch (_editorType)
        {
            case EditorType::LinearEditor: // M: meters
                _validatorType = ValidatorType::SignedDouble;
                _leftSize  = 9; // 20'000 km + sign
                _rightSize = 2; // cm
                break;
            default:
                break;
        }
        return;
    }

    switch (format.unwrapAngular())
    {
        case AngularFormat::DegreesMinutesSeconds:
            switch (_editorType)
            {
                case EditorType::DegreesEditor: // DMS: degrees
                    _validatorType = ValidatorType::SignedInt;
                    _leftSize  = 4;
                    _rightSize = 0;
                    break;
                case EditorType::MinutesEditor: // DMS: minutes
                    _validatorType = ValidatorType::UnsignedInt;
                    _leftSize  = 2;
                    _rightSize = 0;
                    break;
                case EditorType::SecondsEditor: // DMS: seconds
                    _validatorType = ValidatorType::UnsignedDouble;
                    _leftSize  = 2;
                    _rightSize = precision();
                    break;
                default:
                    break;
            }
            break;
        case AngularFormat::DegreesMinutes:
            switch (_editorType)
            {
                case EditorType::DegreesEditor: // DM: degrees
                    _validatorType = ValidatorType::SignedInt;
                    _leftSize  = 4;
                    _rightSize = 0;
                    break;
                case EditorType::MinutesEditor: // DM: minutes
                    _validatorType = ValidatorType::UnsignedDouble;
                    _leftSize  = 2;
                    _rightSize = precision() + 2;
                    break;
                default:
                    break;
            }
            break;
        case AngularFormat::Degrees:
            switch (_editorType)
            {
                case EditorType::DegreesEditor: // D: degrees
                    _validatorType = ValidatorType::SignedDouble;
                    _leftSize  = 4;
                    _rightSize = precision() + 2 * 2;
                    break;
                default:
                    break;
            }
            break;
    }
}

void CoordinateLineEdit::setInternalUpdating(bool internalUpdating)
{
    if(validator() || !internalUpdating)
    {
        _internalUpdating = internalUpdating;
    }
}



// CoordinateValidator
CoordinateValidator::CoordinateValidator(CoordinateLineEdit *editor, CoordinateValueKeeper *valueKeeper, QObject *parent) :
    QValidator(parent),
    _editor(editor),
    _valueKeeper(valueKeeper)
{}

QValidator::State CoordinateValidator::validate(QString &input, int &pos) const
{
    QValidator::State result(QValidator::Invalid);

    if(!isEditorCorrect())
        return result;

    // for internal setText from numeral value
    if(_editor->isInternalUpdating())
    {
        _editor->setInternalUpdating(false);
        return QValidator::Acceptable;
    }

    // playing with first zero
    if(input.contains(QRegularExpression("(^0[1-9]+|^[+][0-9,.])")))
    {
        input = input.remove(0, 1);
        --pos;
    }
    else if(input.contains(QRegularExpression("^[+-]0[1-9]+")))
    {
        input = input.remove(1, 1);
        --pos;
    }

    if(input.contains(QRegularExpression("^[,.]")))
    {
        input.insert(0, '0');
        ++pos;
    }
    else if(input.contains(QRegularExpression("^[+-][,.]")))
    {
        input.insert(1, '0');
        ++pos;
    }

    // text validation
    result = tryValidate(input, pos);

    double oldValue = _valueKeeper->value();
    result = tryUnderstandEditor(input, result);

    if(result != QValidator::Invalid)
    {
        if(oldValue != _valueKeeper->value())
        {
            emit valueChanged(_valueKeeper->value());
        }
    }
    else
    {
        emit incorrectInput();
    }

    return result;
}

QValidator::State CoordinateValidator::tryValidate(QString &input, int &pos) const
{
    QValidator::State result(QValidator::Invalid);

    if(!isEditorCorrect())
        return result;

    int fullSize = (_editor->rightSize() > 0 ? (_editor->leftSize() + 1 + _editor->rightSize()) : _editor->leftSize()); // adding dot-symbol

    if(input.size() <= fullSize)
    {
        int dotPos = input.indexOf(QRegularExpression("[.,]"));

        if(dotPos == -1) // natural
        {
            if(input.size() <= _editor->leftSize())
            {
                return _valueKeeper->validator(_editor->validatorType())->validate(input, pos);
            }
        }
        else // float
        {
            if(dotPos <= _editor->leftSize() &&
               (input.size() - (dotPos + 1)) <= _editor->rightSize())
            {
                return _valueKeeper->validator(_editor->validatorType())->validate(input, pos);
            }
        }
    }

    _valueKeeper->setLastError(CoordinateValueKeeper::InputErrors::Oversize);
    return QValidator::Invalid;
}

bmcl::Option<double> CoordinateValidator::valueFromText(const QString &input) const
{
    double partValue(0.0);

    QString textLine = input.simplified();
    if(!textLine.isEmpty() && textLine != "-" && textLine != "+")
    {
        bool ok = false;
        partValue = tryConvertToDouble(textLine, &ok);
        if(!ok)
        {
            return bmcl::None;
        }
    }

    return partValue;
}

QValidator::State CoordinateValidator::trySetValue(const std::vector<double> &values, QValidator::State previousResult, bool negative) const
{
    if(values.size() != 3)
        return QValidator::Invalid;
    else
        return trySetValue(values.at(0), values.at(1), values.at(2), previousResult, negative);
}

QValidator::State CoordinateValidator::trySetValue(double degrees, double minutes, double seconds, QValidator::State previousResult, bool negative) const
{
    if(previousResult != QValidator::Invalid)
    {
        if(minutes < 0.0 || minutes >= 60.0)
        {
            _valueKeeper->setLastError(CoordinateValueKeeper::InputErrors::MinutesLimits);
            return QValidator::Invalid;
        }
        else if(seconds < 0.0 || seconds >= 60.0)
        {
            _valueKeeper->setLastError(CoordinateValueKeeper::InputErrors::SecondsLimits);
            return QValidator::Invalid;
        }
        else
        {
            double value = std::abs(degrees) + minutes / 60.0 + seconds / 3600.0;

            if((degrees < 0.0) ||
               (degrees == 0.0 && negative))
            {
                value *= -1.0;
            }

            if(value > _valueKeeper->top() || value < _valueKeeper->bottom())
            {
                _valueKeeper->setLastError(CoordinateValueKeeper::InputErrors::ValueLimits);
                return QValidator::Invalid;
            }
            else
            {
                _valueKeeper->setValue(value);
            }
        }
    }

    return previousResult;
}

QValidator::State CoordinateValidator::tryUnderstandEditor(const QString &text, QValidator::State result) const
{
    if(!isEditorCorrect())
        return QValidator::Invalid;

    if(result != QValidator::Invalid)
    {
        std::vector<double> values(_valueKeeper->editorsCount(), 0.0);

        bool negative(false);

        for(size_t i = 0; i < _valueKeeper->activeEditorsCount(); ++i)
        {
            if(i == 0) // degree line (for values below 1.0)
            {
                negative = _valueKeeper->editor(i)->text().startsWith('-');
            }

            if(_valueKeeper->editor(i) == _editor)
            {
                bmcl::Option<double> val = valueFromText(text);
                if(val.isSome())
                {
                    values[i] = val.unwrap();
                }
                else
                {
                    _valueKeeper->setLastError(CoordinateValueKeeper::InputErrors::IncorrectSymbols);
                    return QValidator::Invalid;
                }
            }
            else
            {
                values[i] = tryConvertToDouble(_valueKeeper->editor(i)->text());
            }
        }

        result = trySetValue(values, result, negative);
    }
    else
    {
        if(!_valueKeeper->hasErrors())
            _valueKeeper->setLastError(CoordinateValueKeeper::InputErrors::IncorrectSymbols);
    }

    return result;
}
}
