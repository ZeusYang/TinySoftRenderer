#ifndef TRPARALLEL_WRAPPER_H
#define TRPARALLEL_WRAPPER_H

#include <vector>
#include <algorithm>
#include <functional>

#include "tbb/spin_mutex.h"
#include "tbb/parallel_for.h"
#include "tbb/concurrent_vector.h"

namespace TinyRenderer
{

	
	//! Execution policy tag.
	enum class TRExecutionPolicy { TR_SERIAL, TR_PARALLEL };

	//!
	//! \brief      Fills from \p begin to \p end with \p value in parallel.
	//!
	//! This function fills a container specified by begin and end iterators in
	//! parallel. The order of the filling is not guaranteed due to the nature of
	//! parallel execution.
	//!
	//! \param[in]  begin          The begin iterator of a container.
	//! \param[in]  end            The end iterator of a container.
	//! \param[in]  value          The value to fill a container.
	//! \param[in]  policy         The execution policy (parallel or serial).
	//!
	//! \tparam     RandomIterator Random iterator type.
	//! \tparam     T              Value type of a container.
	//!
	template <typename RandomIterator, typename T>
	void parallelFill(const RandomIterator& begin, const RandomIterator& end,
		const T& value, TRExecutionPolicy policy = TRExecutionPolicy::TR_PARALLEL);

	//!
	//! \brief      Makes a for-loop from \p beginIndex \p to endIndex in parallel.
	//!
	//! This function makes a for-loop specified by begin and end indices in
	//! parallel. The order of the visit is not guaranteed due to the nature of
	//! parallel execution.
	//!
	//! \param[in]  beginIndex The begin index.
	//! \param[in]  endIndex   The end index.
	//! \param[in]  function   The function to call for each index.
	//! \param[in]  policy     The execution policy (parallel or serial).
	//!
	//! \tparam     IndexType  Index type.
	//! \tparam     Function   Function type.
	//!
	template <typename IndexType, typename Function>
	void parallelFor(IndexType beginIndex, IndexType endIndex,
		const Function& function, TRExecutionPolicy policy = TRExecutionPolicy::TR_PARALLEL);

	//!
	//! \brief      Makes a for-loop from \p beginIndex \p to endIndex in parallel with \p given chunk size.
	//!
	//! This function makes a for-loop specified by begin and end indices in
	//! parallel. The order of the visit is not guaranteed due to the nature of
	//! parallel execution.
	//!
	//! \param[in]  beginIndex The begin index.
	//! \param[in]  endIndex   The end index.
	//! \param[in]  grainSize  The chunk size.
	//! \param[in]  function   The function to call for each index.
	//! \param[in]  policy     The execution policy (parallel or serial).
	//!
	//! \tparam     IndexType  Index type.
	//! \tparam     Function   Function type.
	//!
	template <typename Function>
	void parallelForWithAffinity(size_t beginIndex, size_t endIndex,
		const Function& function, TRExecutionPolicy policy = TRExecutionPolicy::TR_PARALLEL);

	//! --------------------------------------Definition---------------------------------------------

	template <typename RandomIterator, typename T>
	void parallelFill(const RandomIterator& begin, const RandomIterator& end, const T& value, TRExecutionPolicy policy)
	{
		auto diff = end - begin;
		if (diff <= 0)
			return;

		size_t size = static_cast<size_t>(diff);
		parallelFor(0, size, [begin, value](size_t i) { begin[i] = value; }, policy);
	}

	template <typename IndexType, typename Function>
	void parallelFor(IndexType start, IndexType end, const Function& func, TRExecutionPolicy policy)
	{
		if (start > end)
			return;
		if (policy == TRExecutionPolicy::TR_PARALLEL)
			tbb::parallel_for(start, end, func);
		else
		{
			for (auto i = start; i < end; ++i)
				func(i);
		}
	}

	template <typename Function>
	void parallelForWithAffinity(size_t start, size_t end,
		const Function& func, TRExecutionPolicy policy)
	{
		if (start > end)
			return;
		static tbb::affinity_partitioner ap;
		if (policy == TRExecutionPolicy::TR_PARALLEL)
			tbb::parallel_for(start, end, func, ap);
		else
		{
			for (auto i = start; i < end; ++i)
				func(i);
		}
	}
	
}

#endif