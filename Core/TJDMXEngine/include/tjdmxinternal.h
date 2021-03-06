/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJ_DMX_INTERNAL_H
#define _TJ_DMX_INTERNAL_H

#include <TJShared/include/tjshared.h>

#ifdef TJ_OS_WIN
	#include <TJSharedUI/include/tjsharedui.h>
	#define TJ_DMX_HAS_TJSHAREDUI 1
#endif

#ifdef TJDMXENGINE_EXPORTS
	#ifdef TJ_OS_WIN
		#define DMX_EXPORTED __declspec(dllexport)
	#else
		#define DMX_EXPORTED __attribute__ ((visibility("default")))
	#endif
#else
	#ifdef TJ_OS_WIN
		#define DMX_EXPORTED __declspec(dllimport)
	#else
		#define DMX_EXPORTED
	#endif
#endif

#endif