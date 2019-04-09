#include "mcc/vis/ReportGen.h"
#include "mcc/vis/Region.h"
#include "mcc/vis/Profile.h"
#include "mcc/vis/RegionViewer.h"
#include "mcc/vis/ProfileViewer.h"

#include <QEventLoop>
#include <QProgressDialog>
#include <QDir>

#include <xlnt/xlnt.hpp>

#include <bmcl/OptionPtr.h>
#include <bmcl/ArrayView.h>

#include <chrono>
#include <cstdint>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace mccvis {

// refact
static std::int64_t decompose(double inputDeg, double* outputMin)
{
    double deg;
    double min = std::modf(inputDeg, &deg);
    *outputMin = min * 60;
    return deg;
}

static std::string doubleToString(double value, const char* postfix = "")
{
    std::ostringstream rv;
    rv << std::setw(2) << std::setprecision(2) << std::fixed;
    rv << value;
    rv << postfix;
    return rv.str();
}

static std::string toDegreesMinutesSeconds(double value)
{
    double minutesFloat;
    double secondsFloat;
    std::int64_t degrees = decompose(value, &minutesFloat);
    std::int64_t minutes = decompose(minutesFloat, &secondsFloat);
    std::ostringstream rv;
    if (value < 0) {
        rv << "-";
    }
    rv << std::abs(degrees);
    rv << "°";
    rv << std::abs(minutes);
    rv << "'";
    rv << std::setw(2) << std::setprecision(2) << std::fixed;
    rv << std::abs(secondsFloat);
    rv << "\"";
    return rv.str();
}

static std::size_t calcStages(const ReportConfig& config, std::size_t numProfs)
{
    std::size_t stages = 0;
    if (config.genExcelReport) {
        stages += 1 + 1;
        if (config.profilesPerExcelPage) {
            stages += numProfs;
        }
    }
    if (config.genProfileImages) {
        stages += numProfs / config.profilesPerImage;
        if (numProfs % config.profilesPerImage) {
            stages++;
        }
    }
    if (config.genZoneImages) {
        stages += 1;
    }
    if (config.genAnglesImages) {
        stages += 1;
    }
    return stages;
}

static const char* boolToString(bool b, const char* first, const char* second)
{
    if (b) {
        return first;
    }
    return second;
}

static xlnt::color colorFromArgb(uint32_t argb)
{
    return xlnt::rgb_color(
        (argb & 0x00ff0000) >> 16,
        (argb & 0x0000ff00) >>  8,
        (argb & 0x000000ff)      ,
        (argb & 0xff000000) >> 24
    );
}

static xlnt::fill fillFromArgb(uint32_t argb)
{
    return xlnt::fill::solid(colorFromArgb(argb));
}

void ReportGen::gen()
{
    std::size_t stages = calcStages(_config, _region->profiles().size());

    double progressDelta = double(100.0) / double(stages);
    double progress = 0;


    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            if (_config.genExcelReport) {
                xlnt::workbook wb;

                xlnt::format degreeFormat = wb.create_format().number_format(xlnt::number_format("0.00°"));
                xlnt::format percentFormat = wb.create_format().number_format(xlnt::number_format("0.00%"));
                xlnt::format meterFormat = wb.create_format().number_format(xlnt::number_format("0.00м"));
                xlnt::format msFormat = wb.create_format().number_format(xlnt::number_format("0.00м/с"));
                xlnt::format mhzFormat = wb.create_format().number_format(xlnt::number_format("0.00МГц"));
                xlnt::format secondsFormat = wb.create_format().number_format(xlnt::number_format("0.00с"));
                xlnt::alignment rightAlignment;
                rightAlignment.horizontal(xlnt::horizontal_alignment::right);
                xlnt::format rightTextFormat = wb.create_format().alignment(rightAlignment);
                //xlnt::format doubleFormat = wb.create_format().number_format(xlnt::number_format("0.00"));


                xlnt::border b;
                xlnt::border::border_property prop;
                prop.color(xlnt::color::black());
                prop.style(xlnt::border_style::thin);
                b.side(xlnt::border_side::top, prop);
                b.side(xlnt::border_side::bottom, prop);
                b.side(xlnt::border_side::start, prop);
                b.side(xlnt::border_side::end, prop);

                xlnt::format borderFormat = wb.create_format().border(b);
                xlnt::format doubleAndBorderFormat = wb.create_format().border(b).number_format(xlnt::number_format("0.00"));

                wb.remove_sheet(wb.active_sheet());

                const ViewParams& p = _region->params();
                xlnt::worksheet ws = wb.create_sheet();
                ws.title("Параметры");
                std::size_t rowOffset = 1;
                std::size_t columnOffset = 1;
                ws.cell(1, rowOffset).value("Имя антенны");
                ws.cell(2, rowOffset).value(p.name);
                ws.cell(2, rowOffset).format(rightTextFormat);
                rowOffset++;
                rowOffset++;
                ws.cell(1, rowOffset).value("Распространение радиоволн");
                ws.cell(2, rowOffset).value(boolToString(p.hasRefraction, "С рефракцией", "Без рефракции"));
                ws.cell(2, rowOffset).format(rightTextFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Тип антенны");
                ws.cell(2, rowOffset).value(boolToString(p.isBidirectional, "Всенаправленная", "Направленная"));
                ws.cell(2, rowOffset).format(rightTextFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Минимальный азимут");
                ws.cell(2, rowOffset).value(p.minAzimuth);
                ws.cell(2, rowOffset).format(degreeFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Максимальный азимут");
                ws.cell(2, rowOffset).value(p.maxAzimuth);
                ws.cell(2, rowOffset).format(degreeFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Нижний угол раскрытия");
                ws.cell(2, rowOffset).value(p.minAngle);
                ws.cell(2, rowOffset).format(degreeFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Верхний угол раскрытия");
                ws.cell(2, rowOffset).value(p.maxAngle);
                ws.cell(2, rowOffset).format(degreeFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Минимальная дальность");
                ws.cell(2, rowOffset).value(p.minBeamDistance);
                ws.cell(2, rowOffset).format(meterFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Максимальная дальность");
                ws.cell(2, rowOffset).value(p.maxBeamDistance);
                ws.cell(2, rowOffset).format(meterFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Высота радара задается");
                ws.cell(2, rowOffset).value(boolToString(p.isRelativeHeight, "Относительно местности", "Относительно эллипсойда"));
                ws.cell(2, rowOffset).format(rightTextFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Высота радара");
                if (p.isRelativeHeight && !_region->profiles().empty()) {
                    double totalHeight = _region->profiles()[0]->totalRadarHeight();
                    std::string h =  doubleToString(totalHeight, " (");
                    h += doubleToString(totalHeight - p.radarHeight, "+");
                    h += doubleToString(p.radarHeight, ")м");
                    ws.cell(2, rowOffset).value(h);
                } else {
                    ws.cell(2, rowOffset).value(p.radarHeight);
                    ws.cell(2, rowOffset).format(meterFormat);
                }
                ws.cell(2, rowOffset).format(rightTextFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Обнаружение на фоне земли");
                ws.cell(2, rowOffset).value(boolToString(p.canViewGround, "Да", "Нет"));
                ws.cell(2, rowOffset).format(rightTextFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Учет зон Френеля");
                ws.cell(2, rowOffset).value(boolToString(p.useFresnelRegion, "Да", "Нет"));
                ws.cell(2, rowOffset).format(rightTextFormat);
                rowOffset++;
                if (p.useFresnelRegion) {
                    ws.cell(1, rowOffset).value("Частота передатчика");
                    ws.cell(2, rowOffset).value(p.frequency);
                    ws.cell(2, rowOffset).format(mhzFormat);
                    rowOffset++;
                }
                rowOffset++;

                ws.cell(1, rowOffset).value("Профиль полета ЛА");
                ws.cell(2, rowOffset).value(boolToString(p.isTargetRelativeHeight, "Относительно местности", "Относительно эллипсойда"));
                ws.cell(2, rowOffset).format(rightTextFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Направление полета ЛА");
                ws.cell(2, rowOffset).value(boolToString(p.isTargetDirectedTowards, "Навстречу радару", "От радара"));
                ws.cell(2, rowOffset).format(rightTextFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Высота ЛА");
                ws.cell(2, rowOffset).value(p.objectHeight);
                ws.cell(2, rowOffset).format(meterFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Скорость ЛА");
                ws.cell(2, rowOffset).value(p.targetSpeed);
                ws.cell(2, rowOffset).format(msFormat);
                rowOffset++;
                rowOffset++;

                ws.cell(1, rowOffset).value("Минимальная дальность поражения");
                ws.cell(2, rowOffset).value(p.minHitDistance);
                ws.cell(2, rowOffset).format(meterFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Максимальная дальность поражения");
                ws.cell(2, rowOffset).value(p.maxHitDistance);
                ws.cell(2, rowOffset).format(meterFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Время реакции комплекса");
                ws.cell(2, rowOffset).value(p.reactionTime);
                ws.cell(2, rowOffset).format(secondsFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Время реакции внешних источников");
                ws.cell(2, rowOffset).value(p.externReactionTime);
                ws.cell(2, rowOffset).format(secondsFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Скорость изделия");
                ws.cell(2, rowOffset).value(p.missleSpeed);
                ws.cell(2, rowOffset).format(msFormat);
                rowOffset++;
                rowOffset++;

                ws.cell(1, rowOffset).value("Постоянный шаг расчета по дальности");
                ws.cell(2, rowOffset).value(boolToString(p.useCalcStep, "Да", "Нет"));
                ws.cell(2, rowOffset).format(rightTextFormat);
                rowOffset++;
                if (p.useCalcStep) {
                    ws.cell(1, rowOffset).value("Шаг расчета по дальности");
                    ws.cell(2, rowOffset).value(p.calcStep);
                    ws.cell(2, rowOffset).format(meterFormat);
                    rowOffset++;
                }
                ws.cell(1, rowOffset).value("Шаг расчета по углу");
                ws.cell(2, rowOffset).value(p.angleStep);
                ws.cell(2, rowOffset).format(degreeFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Запас расчета по дальности");
                ws.cell(2, rowOffset).value(double(p.additionalDistancePercent) / 100);
                ws.cell(2, rowOffset).format(percentFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Расчет зон поражения");
                ws.cell(2, rowOffset).value(boolToString(p.calcHits, "Да", "Нет"));
                ws.cell(2, rowOffset).format(rightTextFormat);
                rowOffset++;
                ws.cell(1, rowOffset).value("Цвет зон обнаружения");
                ws.cell(2, rowOffset).fill(fillFromArgb(p.viewZonesColorArgb));
                rowOffset++;
                ws.cell(1, rowOffset).value("Цвет зон поражения");
                ws.cell(2, rowOffset).fill(fillFromArgb(p.hitZonesColorArgb));
                rowOffset++;

                ws = wb.create_sheet();
                ws.title("Расчет");

                rowOffset = 1;
                ws.cell(columnOffset + 1, rowOffset).value("k обн.");
                ws.cell(columnOffset + 2, rowOffset).value("k пор.");
                ws.cell(columnOffset + 1, rowOffset).format(borderFormat);
                ws.cell(columnOffset + 2, rowOffset).format(borderFormat);

                rowOffset++;
                ws.cell(columnOffset + 1, rowOffset).value(_region->viewCoeff());
                ws.cell(columnOffset + 2, rowOffset).value(_region->hitCoeff());
                ws.cell(columnOffset + 1, rowOffset).format(doubleAndBorderFormat);
                ws.cell(columnOffset + 2, rowOffset).format(doubleAndBorderFormat);

                rowOffset += 2;
                ws.cell(columnOffset + 1, rowOffset).value("β напр.");
                ws.cell(columnOffset + 2, rowOffset).value("ε закр.");
                ws.cell(columnOffset + 3, rowOffset).value("d препят.(м)");
                ws.cell(columnOffset + 4, rowOffset).value("d обн.min.(м)");
                ws.cell(columnOffset + 5, rowOffset).value("d обн.max.(м)");
                ws.cell(columnOffset + 6, rowOffset).value("d пор.min.(м)");
                ws.cell(columnOffset + 7, rowOffset).value("d пор.max.(м)");
                ws.cell(columnOffset + 1, rowOffset).format(borderFormat);
                ws.cell(columnOffset + 2, rowOffset).format(borderFormat);
                ws.cell(columnOffset + 3, rowOffset).format(borderFormat);
                ws.cell(columnOffset + 4, rowOffset).format(borderFormat);
                ws.cell(columnOffset + 5, rowOffset).format(borderFormat);
                ws.cell(columnOffset + 6, rowOffset).format(borderFormat);
                ws.cell(columnOffset + 7, rowOffset).format(borderFormat);

                rowOffset++;
                xlnt::column_properties cprops;
                cprops.hidden = true;
                ws.add_column_properties(1, cprops);
                ws.freeze_panes(ws.cell(1, rowOffset));

                for (std::size_t i = 0; i < _region->profiles().size(); i++) {
                    const Profile* prof = _region->profiles()[i].get();
                    ws.cell(columnOffset + 1, i + rowOffset).value(_region->profiles()[i]->direction());
                    ws.cell(columnOffset + 2, i + rowOffset).value(toDegreesMinutesSeconds(prof->viewAngle()));
                    ws.cell(columnOffset + 3, i + rowOffset).value(prof->peakDistance());
                    ws.cell(columnOffset + 4, i + rowOffset).value(prof->minViewDistance());
                    ws.cell(columnOffset + 5, i + rowOffset).value(prof->maxViewDistance());
                    ws.cell(columnOffset + 6, i + rowOffset).value(prof->minHitDistance());
                    ws.cell(columnOffset + 7, i + rowOffset).value(prof->maxHitDistance());

                    ws.cell(columnOffset + 1, i + rowOffset).format(doubleAndBorderFormat);
                    ws.cell(columnOffset + 2, i + rowOffset).format(borderFormat);
                    ws.cell(columnOffset + 3, i + rowOffset).format(doubleAndBorderFormat);
                    ws.cell(columnOffset + 4, i + rowOffset).format(doubleAndBorderFormat);
                    ws.cell(columnOffset + 5, i + rowOffset).format(doubleAndBorderFormat);
                    ws.cell(columnOffset + 6, i + rowOffset).format(doubleAndBorderFormat);
                    ws.cell(columnOffset + 7, i + rowOffset).format(doubleAndBorderFormat);
                }
                #pragma omp atomic
                progress += progressDelta;
                emit progressChanged(progress);

                if (_config.profilesPerExcelPage) {
                    for (std::size_t i = 0; i < _region->profiles().size();) {
                        std::size_t rowOffset = 1;
                        std::size_t columnOffset = 1;
                        xlnt::worksheet ws = wb.create_sheet();
                        std::ostringstream name;
                        for (std::size_t k = 0; k < _config.profilesPerExcelPage; k++) {
                            if (i >= _region->profiles().size()) {
                                break;
                            }
                            const Profile* prof = _region->profiles()[i].get();
                            name << std::fixed << std::setprecision(2) << _region->profiles()[i]->direction();
                            name << " ";
                            ws.cell(columnOffset + 1, 1).value("D(км)");
                            ws.cell(columnOffset + 2, 1).value("H(м)");
                            ws.cell(columnOffset + 3, 1).value("Hц(м)");
                            ws.cell(columnOffset + 4, 1).value("dH(м)");
                            for (std::size_t j = 0; j < prof->data().size(); j++) {
                                ws.cell(columnOffset + 1, j + rowOffset + 1).value(prof->data()[j].profile.x() / 1000);
                                ws.cell(columnOffset + 2, j + rowOffset + 1).value(prof->data()[j].profile.y());
                                ws.cell(columnOffset + 3, j + rowOffset + 1).value(prof->data()[j].target.y());
                                ws.cell(columnOffset + 4, j + rowOffset + 1).value(prof->data()[j].dy);

                                ws.cell(columnOffset + 1, j + rowOffset + 1).format(doubleAndBorderFormat);
                                ws.cell(columnOffset + 2, j + rowOffset + 1).format(doubleAndBorderFormat);
                                ws.cell(columnOffset + 3, j + rowOffset + 1).format(doubleAndBorderFormat);
                                ws.cell(columnOffset + 4, j + rowOffset + 1).format(doubleAndBorderFormat);
                            }
                            //for (std::size_t j = 0; j < (prof->data().size() + rowOffset); j++) {
                            //    ws.cell(columnOffset + 1, j + 1).format(borderFormat);
                            //    ws.cell(columnOffset + 2, j + 1).format(borderFormat);
                            //    ws.cell(columnOffset + 3, j + 1).format(borderFormat);
                            //    ws.cell(columnOffset + 4, j + 1).format(borderFormat);
                            //}
                            i++;
                            columnOffset += 5;
                            #pragma omp atomic
                            progress += progressDelta;
                            emit progressChanged(progress);
                        }
                        std::string title = name.str();
                        if (title.size() > 31) {
                            title.resize(31);
                        }
                        ws.title(title);
                        xlnt::column_properties cprops;
                        cprops.hidden = true;
                        ws.add_column_properties(1, cprops);
                        ws.freeze_panes(ws.cell(1, 2));
                    }
                }

                QByteArray xlPath = QString(_path + QDir::separator() + "report.xlsx").toUtf8();
                wb.save(xlPath.data());
            }
        }

        #pragma omp single nowait
        {
            if (_config.genZoneImages) {
                RegionViewer regionViewer;
                regionViewer.setRegion(_region.get());
                QString fname = _path + QDir::separator() + "zone.png";
                QImage img(_config.zoneImageWidth, _config.zoneImageHeight, QImage::Format_ARGB32_Premultiplied);
                regionViewer.renderPlot(&img, _config.zoneRenderCfg);
                img.save(fname);
                #pragma omp atomic
                progress += progressDelta;
                emit progressChanged(progress);
            }
        }

        #pragma omp single nowait
        {
            if (_config.genAnglesImages) {
                RegionViewer regionViewer;
                regionViewer.setRegion(_region.get());
                regionViewer.setMode(RegionViewer::AnglesMode);
                QString fname = _path + QDir::separator() + "angles.png";
                QImage img(_config.anglesImageWidth, _config.anglesImageHeight, QImage::Format_ARGB32_Premultiplied);
                regionViewer.renderPlot(&img, _config.anglesRenderCfg);
                img.save(fname);
                #pragma omp atomic
                progress += progressDelta;
                emit progressChanged(progress);
            }
        }

        if (_config.genProfileImages) {
            std::vector<Rc<const Profile>> slice;
            slice.reserve(_config.profilesPerImage);
            ProfileViewer profViewer;
            QImage img(_config.profileImageWidth, _config.profileImageHeight, QImage::Format_ARGB32_Premultiplied);

            #pragma omp for schedule(dynamic, 1)
            for (int i = 0; i < _region->profiles().size(); i += _config.profilesPerImage) {
                QString name;
                std::size_t minK = std::min<std::size_t>(i + _config.profilesPerImage, _region->profiles().size());
                slice.clear();
                for (std::size_t k = i; k < minK; k++) {
                    const auto& prof = _region->profiles()[k];
                    slice.emplace_back(prof);
                    name += QString::number(prof->direction());
                    name += '_';
                }

                profViewer.setProfiles(slice);
                QString fname = _path + QDir::separator() + name + ".png";
                profViewer.renderPlot(&img, _config.profileRenderCfg);
                img.save(fname);
                #pragma omp atomic
                progress += progressDelta;
                emit progressChanged(progress);
            }
        }
    }
}

void ReportGen::run()
{
    gen();
    emit finished(0);
}

ReportGen::ReportGen()
{
}

ReportGen::~ReportGen()
{
}

void ReportGen::generateReport(const Region* region, const QString& path, const ReportConfig& conf)
{
    _region = region;
    _path = path;
    _config = conf;
    start();
}
}
