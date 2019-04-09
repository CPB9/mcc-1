#pragma once
#include "mcc/Config.h"

namespace mccmsg
{

    enum class EventKind
    {
        Registered,
        Activated,
    };

    template <typename T, EventKind EventId>
    struct MCC_MSG_DECLSPEC EventTmpl
    {
        bool _state;
        T _name;
        EventTmpl() {}
        EventTmpl(const T& name, bool state) : _state(state), _name(name) {}
    };

    enum class Field : uint8_t
    {
        All, Pixmap, Firmware, Info, RegFirst, Settings, Ui, Log
    };
    using Fields = std::vector<Field>;

    template <typename D>
    class Updater
    {
    public:
        Updater(D&& d, Fields&& fs) : _d(std::move(d)), _fs(std::move(fs)) {}
        Updater(const D& d) : _d(d) {}
        const D& dscr() const { return _d; }
        const Fields& fields() const { return _fs; }
    private:
        D _d;
        Fields _fs;
    };
}

#if !defined(_MSC_VER) || !defined(BUILDING_MCC_MSG)
# define MCC_MSG_EXPIMP_TEMPLATE extern
#else
# define MCC_MSG_EXPIMP_TEMPLATE
#endif

#define MSG_DECLARE_REQ_FWD(nmsp, req_name, req_obj, rep_name, rep_obj)                 \
    namespace nmsp { using rep_name = ResponseImpl<rep_obj>; }                          \
    namespace nmsp { using rep_name ## Ptr = bmcl::Rc<const rep_name>;}                 \
    namespace nmsp { class req_name ## _Tag;}                                           \
    namespace nmsp { using req_name = RequestImpl<req_obj, req_name ## _Tag, rep_obj>;} \
    namespace nmsp { using req_name ## Ptr = bmcl::Rc<const req_name>;}

#define MSG_DECLARE_REQ(nmsp, req_name, req_obj, rep_name, rep_obj)                         \
    MSG_DECLARE_REQ_FWD(nmsp, req_name, req_obj, rep_name, rep_obj)                         \
    namespace nmsp { class MCC_MSG_DECLSPEC req_name ## _Tag{};}                            \
    MCC_MSG_EXPIMP_TEMPLATE template class MCC_MSG_DECLSPEC ResponseImpl<nmsp::rep_obj>;    \
    MCC_MSG_EXPIMP_TEMPLATE template class MCC_MSG_DECLSPEC RequestImpl<nmsp::req_obj, nmsp::req_name ## _Tag, nmsp::rep_obj>;

#define MSG_DECLARE_NOT_FWD(nmsp, name, obj)        \
    namespace nmsp { using name = NoteImpl<obj>; }  \
    namespace nmsp { using name ## Ptr = bmcl::Rc<const name>; }

#define MSG_DECLARE_NOT(nmsp, name, obj)    \
    MSG_DECLARE_NOT_FWD(nmsp, name, obj)    \
    MCC_MSG_EXPIMP_TEMPLATE template class MCC_MSG_DECLSPEC NoteImpl<obj>;


#define MSG_REGISTER_REQUEST(nmsp) MSG_DECLARE_REQ(nmsp, Register_Request, ObjDscr, Register_Response, ObjDscr)
#define MSG_REGISTER_REQUEST_FWD(nmsp) MSG_DECLARE_REQ_FWD(nmsp, Register_Request, ObjDscr, Register_Response, ObjDscr)

#define MSG_UNREGISTER_REQUEST(nmsp) namespace nmsp {class MCC_MSG_DECLSPEC UnRegister_Response_Tag{};} MSG_DECLARE_REQ(nmsp, UnRegister_Request, ObjName, UnRegister_Response, UnRegister_Response_Tag)
#define MSG_UNREGISTER_REQUEST_FWD(nmsp) namespace nmsp {class UnRegister_Response_Tag;} MSG_DECLARE_REQ_FWD(nmsp, UnRegister_Request, ObjName, UnRegister_Response, UnRegister_Response_Tag)

#define MSG_LIST_REQUEST(nmsp) MSG_DECLARE_REQ(nmsp, List_Request, List_Request_Tag, List_Response, ObjNames);
#define MSG_LIST_REQUEST_FWD(nmsp) MSG_DECLARE_REQ_FWD(nmsp, List_Request, List_Request_Tag, List_Response, ObjNames);

#define MSG_DESCRIPTION_REQUEST(nmsp) MSG_DECLARE_REQ(nmsp, Description_Request, ObjName, Description_Response, ObjDscr);
#define MSG_DESCRIPTION_REQUEST_FWD(nmsp) MSG_DECLARE_REQ_FWD(nmsp, Description_Request, ObjName, Description_Response, ObjDscr);

#define MSG_DESCRIPTIONS_REQUEST(nmsp) MSG_DECLARE_REQ(nmsp, DescriptionS_Request, ProtocolValue, DescriptionS_Response, ObjDscr);
#define MSG_DESCRIPTIONS_REQUEST_FWD(nmsp) MSG_DECLARE_REQ_FWD(nmsp, DescriptionS_Request, ProtocolValue, DescriptionS_Response, ObjDscr);

#define MSG_DESCRIPTIONLIST_REQUEST(nmsp) MSG_DECLARE_REQ(nmsp, DescriptionList_Request, DescriptionList_Request_Tag, DescriptionList_Response, ObjDscrs);
#define MSG_DESCRIPTIONLIST_REQUEST_FWD(nmsp) MSG_DECLARE_REQ_FWD(nmsp, DescriptionList_Request, DescriptionList_Request_Tag, DescriptionList_Response, ObjDscrs);

#define MSG_UPDATE_REQUEST(nmsp) namespace nmsp {using Updater = ::mccmsg::Updater<nmsp::ObjDscr>;} MSG_DECLARE_REQ(nmsp, Update_Request, Updater, Update_Response, ObjDscr);
#define MSG_UPDATE_REQUEST_FWD(nmsp) namespace nmsp {using Updater = ::mccmsg::Updater<nmsp::ObjDscr>;} MSG_DECLARE_REQ_FWD(nmsp, Update_Request, Updater, Update_Response, ObjDscr);

#define MSG_ACTIVATE_REQUEST(nmsp) namespace nmsp {class MCC_MSG_DECLSPEC Activate_Response_Tag{}; using EventType = EventTmpl<ObjName, EventKind::Activated>;} MSG_DECLARE_REQ(nmsp, Activate_Request, EventType, Activate_Response, Activate_Response_Tag);
#define MSG_ACTIVATE_REQUEST_FWD(nmsp) namespace nmsp {class Activate_Response_Tag; using EventType = EventTmpl<ObjName, EventKind::Activated>;} MSG_DECLARE_REQ_FWD(nmsp, Activate_Request, EventType, Activate_Response, Activate_Response_Tag);

#define MSG_REGISTERED(nmsp) namespace nmsp {using RegisteredType = EventTmpl<nmsp::ObjName, EventKind::Registered>; } MSG_DECLARE_NOT(nmsp, Registered, nmsp::RegisteredType);
#define MSG_REGISTERED_FWD(nmsp) namespace nmsp {using RegisteredType = EventTmpl<nmsp::ObjName, EventKind::Registered>; } MSG_DECLARE_NOT_FWD(nmsp, Registered, nmsp::RegisteredType);

#define MSG_UPDATED(nmsp) MSG_DECLARE_NOT(nmsp, Updated, nmsp::ObjDscr)
#define MSG_UPDATED_FWD(nmsp) MSG_DECLARE_NOT_FWD(nmsp, Updated, nmsp::ObjDscr)

#define MSG_ACTIVATED(nmsp) namespace nmsp {using ActivatedType = EventTmpl<nmsp::ObjName, EventKind::Activated>;} MSG_DECLARE_NOT(nmsp, Activated, nmsp::ActivatedType)
#define MSG_ACTIVATED_FWD(nmsp) namespace nmsp {using ActivatedType = EventTmpl<nmsp::ObjName, EventKind::Activated>;} MSG_DECLARE_NOT_FWD(nmsp, Activated, nmsp::ActivatedType)


#define MSG_STANDART_SET(nmsp)          \
    MSG_REGISTER_REQUEST(nmsp);         \
    MSG_UNREGISTER_REQUEST(nmsp);       \
    MSG_REGISTERED(nmsp);               \
    MSG_LIST_REQUEST(nmsp);             \
    MSG_DESCRIPTION_REQUEST(nmsp);      \
    MSG_DESCRIPTIONS_REQUEST(nmsp);     \
    MSG_DESCRIPTIONLIST_REQUEST(nmsp);  \
    MSG_UPDATE_REQUEST(nmsp);           \
    MSG_UPDATED(nmsp);

#define MSG_STANDART_SET_FWD(nmsp)          \
    MSG_REGISTER_REQUEST_FWD(nmsp);         \
    MSG_UNREGISTER_REQUEST_FWD(nmsp);       \
    MSG_REGISTERED_FWD(nmsp);               \
    MSG_LIST_REQUEST_FWD(nmsp);             \
    MSG_DESCRIPTION_REQUEST_FWD(nmsp);      \
    MSG_DESCRIPTIONS_REQUEST_FWD(nmsp);     \
    MSG_DESCRIPTIONLIST_REQUEST_FWD(nmsp);  \
    MSG_UPDATE_REQUEST_FWD(nmsp);           \
    MSG_UPDATED_FWD(nmsp);

#define MSG_DYNAMIC_SET(nmsp)       \
    MSG_ACTIVATED(nmsp);            \
    MSG_ACTIVATE_REQUEST(nmsp);

#define MSG_DYNAMIC_SET_FWD(nmsp)       \
    MSG_ACTIVATED_FWD(nmsp);            \
    MSG_ACTIVATE_REQUEST_FWD(nmsp);
