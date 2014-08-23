/// \file dx11shared.h
/// \brief Shared and utility classes and methods for DirectX11
///
/// \author Raffaele D. Facendola

#include <dxgi.h>

#include "resources.h"
#include "guard.h"

namespace gi_lib{

	namespace dx11{

		/// \brief Convert a resource priority to an eviction priority (DirectX11)
		unsigned int ResourcePriorityToEvictionPriority(ResourcePriority priority);

		/// \brief Convert a resource priority to an eviction priority (DirectX11)
		ResourcePriority EvictionPriorityToResourcePriority(unsigned int priority);

		/// \brief Guard used to release the DirectX11 resources.
		class ReleaseGuard{

		public:

			/// \brief No default constructor.
			ReleaseGuard() = delete;

			/// \brief No copy constructor.
			ReleaseGuard(const ReleaseGuard &) = delete;

			/// \brief No assignment operator.
			ReleaseGuard & operator=(const ReleaseGuard &) = delete;

			/// \brief Create a new release guard.
			/// \param unknown Resource to release.
			ReleaseGuard(IUnknown & unknown) :
				unknown_(&unknown){}

			/// \brief Move constructor. The original copy gets dismissed.
			/// \param other Original release guard to move.
			ReleaseGuard(ReleaseGuard && other){

				unknown_ = other.unknown_;
				other.unknown_ = nullptr;

			}

			/// \brief Destroy this instance and release the underlying resource.
			~ReleaseGuard(){

				if (unknown_){

					unknown_->Release();
					unknown_ = nullptr;

				}

			}

			/// \brief Dismiss the release guard. The resource won't be released upon guard's destruction anymore.
			inline void Dismiss(){

				unknown_ = nullptr;

			}

		private:

			IUnknown * unknown_;

		};

	}

}