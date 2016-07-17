#include <iostream>
#include <chrono>
#include <vector>
#include <deque>
#include <algorithm>
#include <iterator>

#include <boost/heap/fibonacci_heap.hpp>
#include <boost/heap/pairing_heap.hpp>
#include <boost/heap/skew_heap.hpp>
#include <boost/heap/binomial_heap.hpp>
#include <boost/heap/d_ary_heap.hpp>

std::vector<std::deque<int>> data;

auto comp = [](const auto& left_vec, const auto& right_vec)
{
    return left_vec.front() > right_vec.front();
};

template<typename V>
void print_v(V& vec)
{
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<int>(std::cout, ","));
    std::cout << std::endl;
}

void std_heap()
{
	std::cout << "std_heap" << std::endl;  
    std::vector<std::deque<int>> heap(data);

#ifdef CHECK
	std::vector<int> check;
#endif
   
    auto t0 = std::chrono::steady_clock::now();
    std::make_heap(heap.begin(), heap.end(), comp);
	auto t1 = std::chrono::steady_clock::now();
    while (not heap.empty())
    {
        std::pop_heap(heap.begin(), heap.end(), comp);
        auto& vec = heap.back();
       
        if (not vec.empty())
        {
#ifdef CHECK
			check.push_back(vec.front());
#endif
			//std::cout << vec.front() << ",";
            vec.pop_front();
			//print_v(vec);
            std::push_heap(heap.begin(), heap.end(), comp);
        }
        else
        {
            heap.pop_back();
        }
    }

#ifdef CHECK
	std::cout << "check: " << std::is_sorted(check.begin(), check.end()) << std::endl;
#endif
   
    auto t2 = std::chrono::steady_clock::now();
    using fmilliseconds = std::chrono::duration<double, std::milli>;
    auto elapsed_ms_01 = fmilliseconds(t1 - t0).count();
	auto elapsed_ms_02 = fmilliseconds(t2 - t1).count();
    std::cout << elapsed_ms_01 << " ms.\t make" << std::endl;
	std::cout << elapsed_ms_02 << " ms.\t merge" << std::endl;
}

template<typename T, template<typename...> class Heap, typename... Options>
struct heap_data
{
	using handle_t = typename Heap<heap_data, Options...>::handle_type;
	handle_t handle;
	T& data;

	heap_data(T& d) : data(d) {}

	bool operator<(const heap_data& rhs) const
	{
		return data.front() > rhs.data.front();
	}
};

void set_handle(auto&& h)
{
	(*h).handle = h;
}

template<template<typename...> class Heap, typename ... Options>
void boost_heap(const char* name, auto fn)
{
	std::cout << "boost_heap: " << name << std::endl;
  
    using T = std::deque<int>;
    std::vector<T> local_data(data);

#ifdef CHECK
	std::vector<int> check;
#endif
 
    auto t0 = std::chrono::steady_clock::now();
      
    Heap<heap_data<T, Heap, Options...>, Options...> heap;
    std::for_each(local_data.begin(), local_data.end(), [&heap](auto& x) { set_handle(heap.emplace(heap_data<T, Heap, Options...>(x))); });

	auto t1 = std::chrono::steady_clock::now();
    while (not heap.empty())
    {
		auto top_iter = heap.ordered_begin();       
        if (not top_iter->data.empty())
        {
			auto& d = const_cast<heap_data<T, Heap, Options...>&>(*top_iter);

#ifdef CHECK
			check.push_back(d.data.front());
#endif
			//std::cout << d.data.front() << ",";

			d.data.pop_front();
			
			//print_v(d.data);
          
			fn(heap, d.handle);
        }
        else
        {
            heap.pop();
        }
    }

#ifdef CHECK
	std::cout << "check: " << std::is_sorted(check.begin(), check.end()) << std::endl;
#endif
   
    auto t2 = std::chrono::steady_clock::now();
    using fmilliseconds = std::chrono::duration<double, std::milli>;
    auto elapsed_ms_01 = fmilliseconds(t1 - t0).count();
	auto elapsed_ms_02 = fmilliseconds(t2 - t1).count();
    std::cout << elapsed_ms_01 << " ms.\t make" << std::endl;
	std::cout << elapsed_ms_02 << " ms.\t merge" << std::endl;
}

template<template<typename...> class Heap, typename ... Options>
void boost_heap_merge(const char* name)
{
	std::cout << "boost_heap_merge: " << name << std::endl;
  
    using T = std::deque<int>;
    std::vector<T> local_data(data);
 
    auto t0 = std::chrono::steady_clock::now();

	using heap_type = Heap<int, boost::heap::compare<std::greater<int>>, Options...>;
	std::vector<heap_type> heaps;
    std::for_each(local_data.begin(), local_data.end(),
		[&heaps](auto& x)
		{
			heap_type heap;
			for (auto v : x)
			{
				heap.emplace(v);
			}
			heaps.emplace_back(heap);
		});

	auto t1 = std::chrono::steady_clock::now();

	heap_type result;
    while (not heaps.empty())
    {
		auto& back = heaps.back();
		result.merge(back);
		heaps.pop_back();
    }
   
	auto t2 = std::chrono::steady_clock::now();

#ifdef CHECK
	std::vector<int> check0;
	std::for_each(result.ordered_begin(), result.ordered_end(),
	[&check0](auto x)
	{
		check0.push_back(x);
	});
	std::cout << "check: " << std::is_sorted(check0.begin(), check0.end()) << std::endl;
	//print_v(check);
#endif
	auto t3 = std::chrono::steady_clock::now();
#ifdef CHECK
	std::vector<int> check;
	while (not result.empty())
	{
		check.push_back(result.top());
		result.pop();
	}
	std::cout << "check: " << std::is_sorted(check.begin(), check.end()) << std::endl;
	//print_v(check);
#endif

	auto t4 = std::chrono::steady_clock::now();

	using fmilliseconds = std::chrono::duration<double, std::milli>;
    auto elapsed_ms_01 = fmilliseconds(t1 - t0).count();
	auto elapsed_ms_02 = fmilliseconds(t2 - t1).count();
	auto elapsed_ms_03 = fmilliseconds(t3 - t2).count();
	auto elapsed_ms_04 = fmilliseconds(t4 - t3).count();
    std::cout << elapsed_ms_01 << " ms.\t make" << std::endl;
	std::cout << elapsed_ms_02 << " ms.\t merge" << std::endl;
	std::cout << elapsed_ms_03 << " ms.\t ordered traverse" << std::endl;
	std::cout << elapsed_ms_04 << " ms.\t top/pop traverse" << std::endl;
}

int main(int argc, char** argv)
{
    std::srand(0);
    int n = std::stoi(argv[1]);
    int m = std::stoi(argv[2]);
    int k = std::stoi(argv[3]);
    for (int i=0;i<n;++i)
    {
        std::deque<int> v(m);
        std::iota(v.begin(), v.end(), std::rand() % k);
        //print_v(v);
        data.emplace_back(v);
    }
	std::cout << "***" << std::endl;
   
    std_heap();

	using boost::heap::fibonacci_heap;
//	boost_heap<fibonacci_heap>("fibonacci update", [](auto& heap, auto& handle) {heap.update(handle);});
//	boost_heap<fibonacci_heap>("fibonacci decrease", [](auto& heap, auto& handle) {heap.decrease(handle);});
//	boost_heap<fibonacci_heap>("fibonacci increase", [](auto& heap, auto& handle) {heap.increase(handle);});

	using boost::heap::pairing_heap;
	boost_heap<pairing_heap>("pairing_heap update", [](auto& heap, auto& handle) {heap.update(handle);});
	//boost_heap<pairing_heap>("pairing_heap decrease", [](auto& heap, auto& handle) {heap.decrease(handle);});
	//boost_heap<pairing_heap>("pairing_heap increase", [](auto& heap, auto& handle) {heap.increase(handle);});

	using boost::heap::skew_heap;
	boost_heap<skew_heap, boost::heap::mutable_<true>>("skew_heap update", [](auto& heap, auto& handle) {heap.update(handle);});
	boost_heap<skew_heap, boost::heap::mutable_<true>>("skew_heap decrease", [](auto& heap, auto& handle) {heap.decrease(handle);});
	//boost_heap<skew_heap>("skew_heap increase", [](auto& heap, auto& handle) {heap.increase(handle);});

//	using boost::heap::binomial_heap;
//	boost_heap<binomial_heap>("binomial_heap update", [](auto& heap, auto& handle) {heap.update(handle);});
//	boost_heap<binomial_heap>("binomial_heap decrease", [](auto& heap, auto& handle) {heap.decrease(handle);});
//	boost_heap<binomial_heap>("binomial_heap increase", [](auto& heap, auto& handle) {heap.increase(handle);});

	using boost::heap::d_ary_heap;
	boost_heap<d_ary_heap, boost::heap::arity<2>, boost::heap::mutable_<true>>("d_ary_heap update", [](auto& heap, auto& handle) {heap.update(handle);});
	//boost_heap<d_ary_heap, boost::heap::arity<2>, boost::heap::mutable_<true>>("d_ary_heap decrease", [](auto& heap, auto& handle) {heap.decrease(handle);});
	//boost_heap<d_ary_heap, boost::heap::arity<2>, boost::heap::mutable_<true>>("d_ary_heap increase", [](auto& heap, auto& handle) {heap.increase(handle);});

	std::cout << "***" << std::endl;

	boost_heap_merge<fibonacci_heap>("fibonacci_heap");
	boost_heap_merge<pairing_heap>("pairing_heap");
	boost_heap_merge<skew_heap>("skew_heap");
	//boost_heap_merge<d_ary_heap, boost::heap::arity<2>>("d_ary_heap");
   
    return 0;
}
