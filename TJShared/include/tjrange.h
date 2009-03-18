#ifndef _TJRANGE_H
#define _TJRANGE_H

namespace tj {
	namespace shared {
		template<typename T> class Range {
			public:
				Range(T start, T end) {
					_start = start;
					_end = end;
				}

				inline T& Start() {
					return _start;
				}

				inline T& End() {
					return _end;
				}

				inline void SetStart(T start) {
					_start = start;
				}

				inline void SetEnd(T end) {
					_end = end;
				}
				
				inline bool IsValid() {
					return _start<=_end;
				}

				inline T Length() {
					return _end - _start;
				}

				template<typename Q> static Range<Q>& Widest(Range<Q> a, Range<Q> b) {
					return a.Length()>b.Length()?a:b;
				}

				template<typename Q> static Range<T>& Narrowest(Range<Q> a, Range<Q> b) {
					return a.Length()<b.Length()?a:b;
				}

			protected:
				T _start;
				T _end;
		};
	}
}

#endif