/// \file win_core.h
/// \brief Classes and methods to manage the backbone of an application under windows.
///
/// \author Raffaele D. Facendola

#ifdef _WIN32

#pragma once

#include "..\core.h"

namespace gi_lib{

	namespace windows{

		/// \brief Exposes methods to query system's capabilities.
		/// \author Raffaele D. Facendola
		class System : public gi_lib::System{

		public:

			virtual OperatingSystem GetOperatingSystem() override;

			virtual CpuProfile GetCPUProfile() override;

			virtual MemoryProfile GetMemoryProfile() override;

			virtual StorageProfile GetStorageProfile() override;

			virtual DesktopProfile GetDesktopProfile() override;

		};

	}

}

#endif