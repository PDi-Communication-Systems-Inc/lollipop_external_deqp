#ifndef _TCUFORMATUTIL_HPP
#define _TCUFORMATUTIL_HPP
/*-------------------------------------------------------------------------
 * drawElements Quality Program Tester Core
 * ----------------------------------------
 *
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *//*!
 * \file
 * \brief String format utilities.
 *//*--------------------------------------------------------------------*/

#include "tcuDefs.hpp"
#include "deString.h"

#include <ostream>
#include <string>

namespace tcu
{
namespace Format
{

// Hexadecimal value formatter.
template <int NumDigits>
class Hex
{
public:
	Hex (deUint64 value_) : value(value_) {}

	std::ostream& toStream (std::ostream& stream) const
	{
		return stream << this->toString();
	}

	std::string toString (void) const
	{
		DE_STATIC_ASSERT(0 < NumDigits && NumDigits <= 16);

		const char longFmt[]	= {'0', 'x', '%', '0', '0' + NumDigits/10, '0' + NumDigits%10, 'l', 'l', 'x', 0};
		const char shortFmt[]	= {'0', 'x', '%', '0', '0' + NumDigits, 'l', 'l', 'x', 0};

		char buf[sizeof(deUint64)*2 + 3];
		deSprintf(buf, sizeof(buf), NumDigits > 9 ? longFmt : shortFmt, value);

		return std::string(buf);
	}

private:
	deUint64 value;
};

template <int NumDigits>
std::ostream& operator<< (std::ostream& stream, tcu::Format::Hex<NumDigits> hex)
{
	return hex.toStream(stream);
}

// Bitfield formatter.

class BitDesc
{
public:
	deUint64	bit;
	const char*	name;

	BitDesc (deUint64 bit_, const char* name_) : bit(bit_), name(name_) {}
};

#define TCU_BIT_DESC(BIT) tcu::Format::BitDesc(BIT, #BIT)

template <int BitfieldSize>
class Bitfield
{
public:
	Bitfield (deUint64 value, const BitDesc* begin, const BitDesc* end)
		: m_value	(value)
		, m_begin	(begin)
		, m_end		(end)
	{
	}

	std::ostream& toStream (std::ostream& stream)
	{
		deUint64 bitsLeft = m_value;
		for (const BitDesc* curDesc = m_begin; curDesc != m_end; curDesc++)
		{
			if (curDesc->bit & bitsLeft)
			{
				if (bitsLeft != m_value)
					stream << "|";
				stream << curDesc->name;
				bitsLeft ^= curDesc->bit;
			}
		}

		if (bitsLeft != 0)
		{
			if (bitsLeft != m_value)
				stream << "|";
			stream << Hex<BitfieldSize/4>(bitsLeft);
		}

		return stream;
	}

private:
	deUint64			m_value;
	const BitDesc*		m_begin;
	const BitDesc*		m_end;
};

template <int BitfieldSize>
inline std::ostream& operator<< (std::ostream& stream, Bitfield<BitfieldSize> decoder)
{
	return decoder.toStream(stream);
}

// Enum formatter.
// \todo [2012-10-30 pyry] Use template for GetName.

class Enum
{
public:
	typedef const char* (*GetNameFunc) (int value);

	Enum (GetNameFunc getName, int value)
		: m_getName	(getName)
		, m_value	(value)
	{
	}

	std::ostream& toStream (std::ostream& stream) const
	{
		const char* name = m_getName(m_value);
		if (name)
			return stream << name;
		else
			return stream << Hex<sizeof(int)*2>(m_value);
	}

	std::string toString (void) const
	{
		const char* name = m_getName(m_value);
		if (name)
			return std::string(name);
		else
			return Hex<sizeof(int)*2>(m_value).toString();
	}

private:
	GetNameFunc		m_getName;
	int				m_value;
};

inline std::ostream& operator<< (std::ostream& stream, Enum fmt) { return fmt.toStream(stream); }

// Array formatters.

template <typename Iterator>
class Array
{
public:
	Iterator	begin;
	Iterator	end;

	Array (const Iterator& begin_, const Iterator& end_) : begin(begin_), end(end_) {}
};

template <typename T>
class ArrayPointer
{
public:
	const T*	arr;
	int			size;

	ArrayPointer (const T* arr_, int size_) : arr(arr_), size(size_) {}
};

template <typename Iterator>
std::ostream& operator<< (std::ostream& str, const Array<Iterator>& fmt)
{
	str << "{ ";
	for (Iterator cur = fmt.begin; cur != fmt.end; ++cur)
	{
		if (cur != fmt.begin)
			str << ", ";
		str << *cur;
	}
	str << " }";
	return str;
}

template <typename T>
std::ostream& operator<< (std::ostream& str, const ArrayPointer<T>& fmt)
{
	if (fmt.arr != DE_NULL)
		return str << Array<const T*>(fmt.arr, fmt.arr+fmt.size);
	else
		return str << "(null)";
}

// Hex format iterator (useful for combining with ArrayFormatter).
// \todo [2012-10-30 pyry] Implement more generic format iterator.

template <typename T, typename Iterator = const T*>
class HexIterator
{
public:
										HexIterator			(Iterator iter) : m_iter(iter) {}

	HexIterator<T, Iterator>&			operator++			(void)	{ ++m_iter; return *this;		}
	HexIterator<T, Iterator>			operator++			(int)	{ return HexIterator(m_iter++);	}

	bool								operator==			(const HexIterator<T, Iterator>& other) const { return m_iter == other.m_iter; }
	bool								operator!=			(const HexIterator<T, Iterator>& other) const { return m_iter != other.m_iter; }

#if !defined(__INTELLISENSE__)
	// Intellisense in VS2013 crashes when parsing this.
	Hex<sizeof(T)*2>					operator*			(void) const { return Hex<sizeof(T)*2>(*m_iter);	}
#endif

private:
	Iterator							m_iter;
};

} // Format

template <int Bits>		inline deUint64 makeMask64			(void)				{ return (1ull<<Bits)-1;								}
template <>				inline deUint64 makeMask64<64>		(void)				{ return ~0ull;											}
template <typename T>	inline deUint64	toUint64			(T value)			{ return (deUint64)value & makeMask64<sizeof(T)*8>();	}

/** Format value as hexadecimal number. */
template <int NumDigits, typename T>
inline Format::Hex<NumDigits> toHex (T value)
{
	return Format::Hex<NumDigits>(toUint64(value));
}

/** Format value as hexadecimal number. */
template <typename T>
inline Format::Hex<sizeof(T)*2> toHex (T value)
{
	return Format::Hex<sizeof(T)*2>(toUint64(value));
}

/** Decode and format bitfield. */
template <typename T, size_t Size>
inline Format::Bitfield<sizeof(T)*8> formatBitfield (T value, const Format::BitDesc (&desc)[Size])
{
	return Format::Bitfield<sizeof(T)*8>((deUint64)value, &desc[0], &desc[Size]);
}

/** Format array contents. */
template <typename Iterator>
inline Format::Array<Iterator> formatArray (const Iterator& begin, const Iterator& end)
{
	return Format::Array<Iterator>(begin, end);
}

/** Format array contents. */
template <typename T>
inline Format::ArrayPointer<T> formatArray (const T* arr, int size)
{
	return Format::ArrayPointer<T>(arr, size);
}

} // tcu

#endif // _TCUFORMATUTIL_HPP
