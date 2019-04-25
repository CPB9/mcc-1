#include <bmcl/Rc.h>
#include <bmcl/MakeRc.h>
#include <bmcl/ThreadSafeRefCountable.h>

#include <caf/meta/type_name.hpp>
#include <caf/behavior.hpp>
#include <caf/event_based_actor.hpp>
#include <caf/to_string.hpp>
#include <caf/actor_ostream.hpp>
#include <caf/actor_system.hpp>
#include <caf/actor_system_config.hpp>
#include <caf/send.hpp>

#include <type_traits>

class TestStruct : public bmcl::ThreadSafeRefCountable<std::size_t> {
public:
    TestStruct()
        : a(0)
        , b(1)
    {
    }

    int a;
    int b;
};

class TestStruct2 : public TestStruct
{
public:
    TestStruct2() : c(2), d(3){}
    int c;
    int d;
};

//TestStruct
template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, TestStruct& x)
{
    return f(x.a, x.b);
}

//TestStruct2
template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, TestStruct2& x)
{
    return f(*(TestStruct*)&x, x.c, x.d);
}

//rc
template <class Inspector, typename T>
typename std::enable_if<Inspector::reads_state, typename Inspector::result_type>::type
inspect(Inspector& f, bmcl::Rc<T>& x)
{
    return f(*x);
}

template <class Inspector, typename T>
typename std::enable_if<Inspector::writes_state, typename Inspector::result_type>::type
inspect(Inspector& f, bmcl::Rc<T>& x)
{
    if (x.isNull()) {
        x.reset(new T);
    }
    return f(*x);
}

caf::behavior testActor(caf::event_based_actor* self)
{
    return {
        [=](const bmcl::Rc<TestStruct>& x) {
            caf::aout(self) << caf::to_string(x) << std::endl;
            self->send_exit(self, caf::exit_reason::user_shutdown);
        }
    };
}

int main()
{
    caf::actor_system_config cfg;
    caf::actor_system system{cfg};
    auto test = system.spawn(testActor);
    bmcl::Rc<TestStruct> s1 = new TestStruct;
    bmcl::Rc<TestStruct> s2 = new TestStruct;
    s2->a = 100;
    s2->b = 99;
//     caf::anon_send(test, s1);
//     for (int i = 0; i < 100; i++) {
//         caf::anon_send(test, s2);
//     }
   
    bmcl::Rc<TestStruct2> s3 = new TestStruct2;
    s3->a = 81;
    s3->b = 82;
    s3->c = 83;
    s3->d = 84;
    caf::anon_send(test, bmcl::wrapRc<TestStruct>(s3.get()));
    
    
    system.await_all_actors_done();
}

