#include "mcc/calib/RadioCalibrationWidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>

namespace mcccalib {

RadioCalibrationWidget::~RadioCalibrationWidget()
{
}

VarCondition::Condition nonZeroCondition = [](const mccmsg::NetVariant& value)
{
    return value.toInt() != 0;
};

RadioCalibrationWidget::RadioCalibrationWidget(mccuav::UavController* uavController)
    : CalibrationControllerAbstract(mccmsg::CalibrationSensor::Radio)
    , _uavController(uavController)
{
    _layout = new QVBoxLayout();
    setLayout(_layout);
    _instructions = new QLabel();
    _image = new QLabel();
    _nextButton = new QPushButton("Next");
    _skipButton = new QPushButton("Skip");
    _nextButton->setEnabled(false);
    _skipButton->setEnabled(false);

    auto buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(_nextButton);
    buttonsLayout->addWidget(_skipButton);
    buttonsLayout->addStretch();

    _channelsLayout = new QVBoxLayout();

    auto centerLayout = new QHBoxLayout();
    centerLayout->addWidget(_image);
    centerLayout->addLayout(_channelsLayout);

    _layout->addWidget(_instructions);
    _layout->addLayout(buttonsLayout);
    _layout->addLayout(centerLayout, 1);

//     connect(mccContext, &mccui::Context::systemStarted, this,
//             [this]()
//             {
//                 connect(mccContext->exchangeService(), &mccuav::ExchangeService::traitCalibration, this, &RadioCalibrationWidget::onCalibrationState);
//             }
//     );

    connect(_nextButton, &QPushButton::pressed, this,
            [this]()
            {
                _uavController->sendCmdYY(new mccmsg::CmdCalibrationStart(device(), sensor(), mccmsg::CalibrationCmd::NextStep));
            }
    );

    connect(_skipButton, &QPushButton::pressed, this,
            [this]()
            {
                _uavController->sendCmdYY(new mccmsg::CmdCalibrationStart(device(), sensor(), mccmsg::CalibrationCmd::SkipStep));
            }
    );
}

void RadioCalibrationWidget::start()
{
    _uavController->sendCmdYY(new mccmsg::CmdCalibrationStart(device(), sensor()));
}

void RadioCalibrationWidget::cancel()
{
    _uavController->sendCmdYY(new mccmsg::CmdCalibrationCancel(device()));
}

void RadioCalibrationWidget::onTraitCalibrationState(const mccmsg::TmCalibrationPtr& s)
{
    const mccmsg::Calibration* state = &s->state();
    if (state->_sensor != mccmsg::CalibrationSensor::Radio)
        return;

    _nextButton->setEnabled(state->nextEnabled);
    _skipButton->setEnabled(state->skipEnabled);

    _instructions->setText(QString::fromStdString(state->message));
    if (!state->image.empty())
    {
        QString res = QString(":/resources/%1").arg(state->image.c_str());
        _image->setPixmap(res);
    }

    if (_channels.empty())
    {
        for (size_t i = 0; i < state->pwmValues.size(); ++i)
        {
            auto bar = new QProgressBar();
            bar->setMinimum(800);
            bar->setMaximum(2200);
            _channels.push_back(bar);
            _channelsLayout->addWidget(bar);
        }
    }

    for (size_t i = 0; i < state->pwmValues.size(); ++i)
    {
        auto bar = _channels[i];
        if (state->pwmValues[i].isNone())
        {
            bar->setEnabled(false);
            continue;
        }
        bar->setEnabled(true);
        bar->setValue(*state->pwmValues[i]);
    }

    if (state->done)
    {
        emit completed();
    }
}
}
