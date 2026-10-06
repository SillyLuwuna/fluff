#pragma once

#include <mpfr.h>
#include <string>
#include <cstdint>

namespace fluff::mpfr_extensions
{
	struct str
	{
		static inline std::string to_str(const mpfr_t x, int decimals)
		{
			char* buf = nullptr;
			int n = mpfr_asprintf(&buf, "%.*Rf", decimals, x);
			if (n < 0 || !buf) return "";
			std::string str(buf);
			mpfr_free_str(buf);
			return str;
		}

		static inline std::string to_str_scientific(const mpfr_t x, int decimals)
		{
			char* buf = nullptr;
			int n = mpfr_asprintf(&buf, "%.*Re", decimals, x);
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

