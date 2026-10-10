#ifndef FLUFF_EXTENSIONS_MPFR_STR_HPP
#define FLUFF_EXTENSIONS_MPFR_STR_HPP

#include <mpfr.h>
#include <string>
#include "fluff/types.hpp"

namespace fluff::extensions::mpfr
{
	struct str
	{
		static inline std::string to_str(const mpfr_t x, i32 decimals)
		{
			char* buf = nullptr;
			i32 n = mpfr_asprintf(&buf, "%.*Rf", decimals, x);
			if (n < 0 || !buf) return "";
			std::string str(buf);
			mpfr_free_str(buf);
			return str;
		}

		static inline std::string to_str_scientific(const mpfr_t x, i32 decimals)
		{
			char* buf = nullptr;
			i32 n = mpfr_asprintf(&buf, "%.*Re", decimals, x);
			if (n < 0 || !buf) return "";
			std::string str(buf);
			mpfr_free_str(buf);
			return str;
		}

		static inline std::string to_str_int(const mpfr_t x)
		{
			return to_str(x, 0);
		}
	};
}


#endif
