#include <QString>
#include <fmt/format.h>
#include <bmcl/StringView.h>
#include "mcc/msg/Error.h"

namespace mccmsg
{

const char* to_string(Error x)
{
    switch (x)
    {
    case Error::CoreDisconnected: return "нет соединения с ядром";
    case Error::CantCancel: return "отмена невозможна";
    case Error::Canceled: return "отменена";
    case Error::CmdUnknownTrait: return "неизвестный компонент";
    case Error::CmdUnknown: return "неизвестная команда";
    case Error::CmdIncorrect: return "некорректная команда";
    case Error::CmdFailed: return "ошибка исполнения";
    case Error::CmdOtherOngoing: return "исполняется другая команда";
    case Error::GroupUnknown: return "группа неизвестна";
    case Error::GroupWithoutLeader: return "отсутствует лидер";
    case Error::GroupLeaderUnknown: return "лидер неизвестен";
    case Error::GroupNotSet: return "группа не задана";
    case Error::GroupUnreachable: return "группа недостижима";
    case Error::ChannelUnknown: return "неизвестный канал";
    case Error::ChannelClosed: return "канал закрыт";
    case Error::ChannelError: return "ошибка канала обмена";
    case Error::ChannelCantShare: return "канал не может быть разделён между устройствами";
    case Error::DeviceUnregistered: return "устройство разрегистрировано";
    case Error::DeviceUnknown: return "неизвестное устройство";
    case Error::DeviceUiUnknown: return "неизвестный интерфейс устройства";
    case Error::DeviceInactive: return "устройство неактивно";
    case Error::NoChannelAvailable: return "нет доступных каналов обмена";
    case Error::Timeout: return "таймаут";
    case Error::CantRegister: return "не удалось зарегистрировать";
    case Error::CantUpdate: return "не удалось обновить";
    case Error::CantUnRegister: return "не удалось удалить";
    case Error::CantJoin: return "не удалось связать объекты";
    case Error::CantBeNull: return "не может быть пустым";
    case Error::NotFound: return "не найдено";
    case Error::CantGet: return "не удалось извлечь данные";
    case Error::InconsistentData: return "некорректные данные";
    case Error::NotImplemented: return "не реализовано";
    case Error::ProtocolUnknown: return "неизвестный протокол";
    case Error::ProtocolsShouldBeSame: return "протокол должен совпадать";
    case Error::RadarUnknown: return "неизвестный радар";
    case Error::FirmwareUnknown: return "неизвестная прошивка";
    case Error::FirmwareIncompatible: return "несовместимая прошивка";
    case Error::FirmwareChanged: return "прошивка изменилась";
    case Error::CantOpen: return "не удалось открыть";
    case Error::UnknownError: return "неизвестная ошибка";
    case Error::TmSessionUnknown: return "неизвестная тм-сессия";
    case Error::RouteUnknown: return "неизвестный маршрут";
    case Error::RouteBusy: return "осуществляется другая операция с маршрутом";
    case Error::FileBusy: return "файл занят";
    case Error::NotAFile: return "не файл";
    default: break;
    }
  return "error";
}

ErrorDscr::ErrorDscr() : _value((Error)0) {}
ErrorDscr::ErrorDscr(Error value) : _value(value) {}
ErrorDscr::ErrorDscr(Error value, std::string&& text) : _value(value), _text(std::move(text)) {}
ErrorDscr::ErrorDscr(Error value, bmcl::StringView& text) : _value(value), _text(text.toStdString()) {}
ErrorDscr::~ErrorDscr(){}
Error ErrorDscr::value() const { return _value; }
const char* ErrorDscr::valuestr() const { return to_string(_value); }
const std::string& ErrorDscr::text() const { return _text;}
QString ErrorDscr::qfull() const { return QString::fromStdString(full());}
bool ErrorDscr::isCanceled() const { return _value == Error::Canceled; }

std::string ErrorDscr::full() const
{
    if (_text.empty())
        return fmt::format("{}", to_string(_value));
    return fmt::format("{} ({})", to_string(_value), _text);
}

}
