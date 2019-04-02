#pragma once

#include <QCheckBox>
#include <bmcl/Uuid.h>

#include "mcc/Config.h"

class QSize;

namespace mccui {

class MCC_UI_DECLSPEC SliderCheckBox : public QCheckBox
{
    Q_OBJECT

public:
    explicit SliderCheckBox(QWidget* parent = nullptr);
    explicit SliderCheckBox(bool smallSize, QWidget* parent = nullptr);
    SliderCheckBox(bmcl::Uuid uuid, bool smallSize = false, QWidget* parent = nullptr);
    ~SliderCheckBox() override;

    void mousePressEvent(QMouseEvent* e) override;

    void paintEvent(QPaintEvent* e) override;
    QSize sizeHint() const override;

    void setSmallSize(bool smallSize);
    bool smallSize() const {return _smallSize;}

    inline void setUuid(const bmcl::Uuid& uuid) { _uuid = uuid; }
    inline bmcl::Uuid uuid() const { return _uuid; }

signals:
    void sliderStateChanged(const bmcl::Uuid& uuid, bool checked);

private:
    void init();

    bmcl::Uuid _uuid;
    bool _smallSize;

    Q_DISABLE_COPY(SliderCheckBox)
};

class MCC_UI_DECLSPEC OnOffSliderCheckBox : public SliderCheckBox
{
    Q_OBJECT

public:
    explicit OnOffSliderCheckBox(QWidget* parent = nullptr);
    explicit OnOffSliderCheckBox(bool smallSize, QWidget* parent = nullptr);
    explicit OnOffSliderCheckBox(bmcl::Uuid uuid, bool smallSize = false, QWidget* parent = nullptr);
    ~OnOffSliderCheckBox() override;

    void mousePressEvent(QMouseEvent* e) override;

private:
    Q_DISABLE_COPY(OnOffSliderCheckBox)
};
}
