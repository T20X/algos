#pragma once

#include <type_traits>
#include <memory>
#include <list>
#include <array>

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



    template <class M>
    struct Action
    {
        M& m_;
        Action(M& m) :m_(m) { std::cout << "\n m is set"; }
        M& m() { return m_; }
        void complete(M& m)
        {
        }
    };

    template <class M>
    struct ActionOrder : public Action<M>
    {
        using Action<M>::Action;
        void doIt()
        {
            this->m().doIt();
        }
    };

    struct Event
    {
    };

    struct OrderAck : Event
    {
    };

    struct OrderRej : Event
    {
    };

    using events = type_pack<OrderAck>;
    constexpr size_t _N_EVENTS_ = type_pack_size(events());

    struct EventHandle {
        virtual ~EventHandle() = 0;
    };

    inline EventHandle::~EventHandle() {}

    template <class E>
    struct BaseHandle : EventHandle {
        virtual void process(const E& e) = 0;      
    };

    template <class E, class A, class D>
    struct EventActionHandle : BaseHandle<E>
    {
        using Event = E;
        using Action = A;

        Action* a_;
        EventActionHandle(Action* a) :a_(a) {}
        Action& getAction() { return *a_; }

        void process(const E& e) override
        {
            D& derived = static_cast<D&>(*this);
            derived.handle(e, getAction());
        }
    };

    template <class E, class A>
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
    };
    
    struct BaseExpectation {
        virtual ~BaseExpectation() = 0;
    };

    inline BaseExpectation::~BaseExpectation() {}

    template <class A, class... E>
    struct Expectation : public BaseExpectation, A, DoHandle<E, A>...
    {
        Expectation(A&& a):A(std::move(a)), 
            DoHandle<E, A>(this)...
        {}
    };

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
                    static_cast<BaseHandle<Event>&>(*ptr).process(e);
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

        template<class E,class A>
        using handle_t = DoHandle<E, A>;

        void work()
        {
            ActionOrder<Mediator> a(*this);
            a.doIt();

            {
                std::list< std::shared_ptr<BaseExpectation>> exps;

                {
                    auto ptr = std::make_shared<Expectation<ActionOrder<Mediator>, OrderAck>>(std::move(a));
                    exps.emplace_back(ptr);
                    ptr->m().doIt();
                    ptr->m().doIt();
                    ptr->m().doIt();
                    ptr->m().doIt();
                    ptr->m().doIt();
                    ptr->m().doIt();
                    ptr->m().doIt();
                    ptr->m().doIt();

                    t_._table[find_same_idx(events{}, just_type<OrderAck>{})].emplace_back(
                        std::move(std::weak_ptr<EventHandle>(ptr))
                    );
                    process(OrderAck());
                    process(OrderAck());
                    process(OrderAck());
                    process(OrderAck());
                }
                process(OrderAck());
            }
            process(OrderAck());

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
