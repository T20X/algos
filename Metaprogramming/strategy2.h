#pragma once

#include <type_traits>
#include <memory>
#include <list>
#include <array>
#include <functional>

namespace strategy
{
    template <class T>
    struct just_type { using type = T; };

    template <class... T>
    struct type_pack {};

    template <class... T>
    constexpr size_t type_pack_size(type_pack<T...>) { return sizeof...(T); }

    template <template <class...> class F>
    struct FN_TYPE
    {
        template <class... Ts>
        constexpr auto operator()(just_type<Ts>...) {
            return F<Ts...>::type{};
        }
    };

    template <template <class...> class F>
    constexpr FN_TYPE<F> fn_type{};

    template <template <class...> class F>
    struct FN_VAL
    {
        template <class... Ts>
        constexpr bool operator()(just_type<Ts>...) {           
            return F<Ts...>::value;
        }
    };

    template <template <class...> class F>
    constexpr FN_VAL<F> fn_value{};


    template <class F, class... Ts>
    constexpr auto transform(F f, type_pack<Ts...>) {
        return type_pack<decltype(f(just_type<Ts>{}))... > {};
    }

    template <class... T, class O, class F>
    constexpr size_t find_idx_if(type_pack<T...>, just_type<O>, F f)
    {
        size_t N = sizeof...(T);
        bool r[] = { f(just_type<T>{}, just_type<O>{})... };
        for (size_t i = 0; i < N; ++i)
            if (r[i]) return i;
        return N;
    }

    template <class... T, class O>
    constexpr auto find_same_idx(type_pack<T...> p, just_type<O> t) {
        return find_idx_if(p, t, fn_value<std::is_same>);
    }

    template <class... T, class F>
    void apply(type_pack<T...>, F&& f) {
        int a[] = {(std::forward<F>(f)(just_type<T>{}), 0)...};
    }


    struct Event
    {
    };

    struct OrderEvent : Event
    {
    };

    struct OrderAck : OrderEvent
    {
    };

    struct OrderRej : OrderEvent
    {
    };

    using events = type_pack<OrderAck, OrderRej>;
    constexpr size_t _N_EVENTS_ = type_pack_size(events());

    struct EventHandle {
        virtual ~EventHandle() = 0;
    };

    inline EventHandle::~EventHandle() {}

    template <class E>
    struct BaseHandle : EventHandle {
        virtual void onEvent(const E& e) = 0;
    };

   // template <class A, class E>
    //using Processor = std::function<void(const E & event, A & action)>;



    struct Op
    {
        long id;
    };


    template <class M, class... E >
    struct Action : BaseHandle<E>...
    {
        M& m_;
        Action(M& m) :m_(m) { std::cout << "\n m is set"; }
        M& m() { return m_; }
        //Action& getAction() { return static_cast<Action&>(*this); }
        void complete(M& m)
        {
        }

      /*  template <class E>
        void process(const E& e)
        {
            onEvent(e);
        }*/

        template <class F>
        void for_each_event(F f) {          
            apply(type_pack<EventActionHandle<E, Action>::Event...>{}, f);
        }

        virtual void onEvent(const OrderAck& e) 
        {
            std::cout << "\n OrderAck from Action!";
        }

        virtual void onEvent(const OrderRej& e)
        {
            std::cout << "\n OrderRej from Action!";
        }

        std::vector<Op> _ops;
    };

    template <class M, class... E>
    struct ActionOrder : public Action<M, E...>
    {
        using Action<M, E...>::Action;
        void doIt()
        {
            this->m().doIt();
        }
        void onEvent(const OrderAck& e)
        {
            std::cout << "\n OrderAck from ActionOrder!";
        }
    };

   /* template <class E, class A>
    struct DoHandle : EventActionHandle<E, A, DoHandle<E, A>> {};

    template <>
    struct DoHandle<OrderAck, ActionOrder<class Mediator>>
       : EventActionHandle<OrderAck, ActionOrder<class Mediator>,
        DoHandle<OrderAck, ActionOrder<class Mediator>>>
    {        
        using Base = EventActionHandle<OrderAck, ActionOrder<class Mediator>,
            DoHandle<OrderAck, ActionOrder<class Mediator>>>;
        using Base::Base;        
        void handle(const typename Base::Event& e,
                          typename Base::Action& a)
        {
            std::cout << "\n handled OrderAck event for ActionOrder";
            a.doIt();
        }
    };*/
    
    struct BaseExpectation {
        virtual ~BaseExpectation() = 0;
    };

    inline BaseExpectation::~BaseExpectation() {}


   /* template <class A, class... E>
    struct Expectation : public BaseExpectation, A, EventActionHandle<E, A>...
    {
        //template <class S>
        Expectation(A&& a, Processor<A,E>&&... p)
            :A(std::move(a)), EventActionHandle<E, A>(this, std::move(p))...
        {
        }

        template <class F>
        void for_each_event(F f) {
            //auto seq = std::make_index_sequence<sizeof...(E)>;
            apply(type_pack<EventActionHandle<E, A>::Event...>{}, f);
        }
    };*/

    struct EventTable
    {
        template <class Event>
        void process(const Event& e)
        {
            auto& handles = _table[find_same_idx(events{}, just_type<Event>{})];
            auto it = std::begin(handles);
            while (it != std::end(handles))
            {
                auto ptr = it->lock();
                if (ptr)
                {
                    static_cast<BaseHandle<Event>&>(*ptr).onEvent(e);
                    ++it;
                }
                else
                {
                    std::cout << "\n no more expecations!";
                    it = handles.erase(it++);
                }
            }            
        }

        std::array<std::list<std::weak_ptr<
                    EventHandle>>,
                  _N_EVENTS_> _table;
    };

    struct State
    {
        using type = uint8_t;
        static constexpr type EMPTY = 0;
        static constexpr type ACCEPTED = 2;
        static constexpr type SUBMITTED = 4;
        static constexpr type REJECTED = 8;
    };

    template <class E,class A, typename = std::enable_if_t<std::is_base_of<OrderEvent,E>::value>>
    auto mapThis(E&&, A&&,int)
    {
        std::cout << "\n mapthis=";
        return mapS(just_type<E>{});
    }

    template <class E, class A>
    auto mapThis(E&&, A&&, ...)
    {
        static_assert(typename E, "sadf");
        return {};
        //auto r = mapS(just_type<E>{});
        //std::cout << "\n mapthis=" << r;
    }

    /*template <class E, class A>
    void mapThis(E&&,A&&)
    {
    }*/

    template <class E>
    auto mapS(just_type<E>) 
    {
        //assert(0);
        //return State::EMPTY;
        static_assert(typename E, "sadf");
    }

    auto mapS(just_type<OrderAck>)
    {
        return State::ACCEPTED;
    }


    struct Mediator
    {
        EventTable t_;
        template <class E>
        void process(const E& e)
        {
            t_.process(e);
        }

        void doIt()
        {
            cout << "\n M::do()";
        }

        template <class E, class Ptr>
        void expect(just_type<E> e, Ptr p) {
            t_._table[find_same_idx(events{}, e)].emplace_back(
                std::weak_ptr<BaseHandle<E>>(p));
        }

        void work()
        {
            //mapS(just_type<OrderAck>{});
            //mapS(just_type<OrderRej>{});
            auto a = std::make_shared<ActionOrder<Mediator, OrderAck, OrderRej>>(*this);
            mapThis(OrderAck{}, *a,0);
            //mapThis(Event{}, a);
            a->doIt();

            {
               // std::list< std::shared_ptr<BaseExpectation>> exps;

                {
                   // auto ptr = std::make_shared<Expectation<ActionOrder<Mediator>, OrderAck, OrderRej>>(
                     //   std::move(a), [&](const OrderAck& e, ActionOrder<Mediator>&) -> void { std::cout << "\n OrderAck processed"; },
                       //               [&](const OrderRej& e, ActionOrder<Mediator>&) -> void { std::cout << "\n OrderRej processed"; });
                   // exps.emplace_back(a);
                    a->m().doIt();
                    a->m().doIt();
                    a->m().doIt();
                    a->m().doIt();
                    a->m().doIt();
                    a->m().doIt();
                    a->m().doIt();
                    a->m().doIt();

                    //t_._table.for_ea
                    a->for_each_event([&p=a, this](auto jt) {
                        t_._table[find_same_idx(events{}, jt)].emplace_back(
                            std::weak_ptr<BaseHandle<decltype(jt)::type>>(p));
                        });
                    //expect(just_type<OrderAck>{}, ptr);
                    //expect(just_type<OrderRej>{}, ptr);
                   // t_._table[find_same_idx(events{}, just_type<OrderAck>{})].emplace_back(std::weak_ptr<BaseHandle<OrderAck>>(ptr));
                    //t_._table[find_same_idx(events{}, just_type<OrderRej>{})].emplace_back(std::weak_ptr<BaseHandle<OrderRej>>(ptr));
                    
                    process(OrderAck());
                    process(OrderAck());
                    process(OrderAck());
                    process(OrderAck());
                    process(OrderRej());
                }
                process(OrderAck());
            }
            process(OrderRej());

        }
    };

    static void test()
    {
        Mediator m;
        m.work();

    }
}


/*template <class I, class T>
struct IndexedType
{
    static constexpr size_t idx = I;
    using type = T;
};

template <class IS, class... T>
 struct indexed_types;
template <size_t... SZ, class... T>
 struct indexed_types<std::index_sequence<SZ...>,T...>
     : IndexedType<IS, T>...
{
};

 template <class... T>
 constexpr auto make_indexed_types(size_t N, type_pack<T...>) {
     return indexed_types<std::make_index_sequence<N>, T...>{};
 }

 using event_indices = decltype(make_indexed_types(_N_EVENTS_, events{}));

 template <class E>
 auto indexType(just_type<E>) {
     return
 }

template <class E>
constexpr size_t eventIndexFor(just_type<E>) {
    auto indices = static_cast<IndexedType<event_indices;
}*/

