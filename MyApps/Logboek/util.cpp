#include "util.h"

String FormatElapsedMin (int m) {
	int h=0, _m=0;
	String result;
	
	if (m >= 60) {
		h = ffloor(m/60);
		_m = m - (h*60);
		result = Format("%i`u", h);
	}
	else _m = m;

	if (h==0 || _m > 0) result += Format("%i`min", _m);
	
	return result;
}

String FormatElapsedSec (int s) {
	int m=0, _s=0;
	String result;
	
	if (s >= 60) {
		m = ffloor(s/60);
		_s = s - (m*60);
		result = FormatElapsedMin(m);
	}
	else _s = s;
	
	if (m==0 || _s > 0) result += Format("%i`sec", _s);
	
	return result;
}

String FormatElapsedMsec(int ms) {
	int s=0, _ms=0;
	String result;

	if (ms >= 1000) {
		s = ffloor(ms/1000);
		_ms = ms - (s*1000);
		result = FormatElapsedSec(s);
	}
	else _ms = ms;
	
	if (s==0 || _ms > 0) result += Format("%i`ms", _ms);
	
	return result;
}
