/// \file dx11committable.h
/// \brief Defines a base class for all whose resource that can be committed and bound to a DirectX11 device context.
///
/// \author Raffaele D. Facendola

#pragma once

#include <d3d11.h>

#include "object.h"

namespace gi_lib{

	namespace dx11{

		/// \brief Base interface for object that can commit resources to the video memory.
		class ICommitter : public Object{

		public:

			/// \brief Commit the resource using the given context.
			virtual void operator()(ID3D11DeviceContext& context) = 0;

		};

		/// \brief Concrete object used to commit a resource to the video memory.
		/// To be used along with ICommitter for type erasure.
		template <typename TType>
		class Committer : public ICommitter{

		public:

			/// \brief Create a new committer.
			/// The committer must commit the resource at least once before releasing the ownership!
			/// \param subject Resource to commit.
			Committer(const ObjectPtr<TType>& subject);

			/// \brief Commit the resource using the given context.
			virtual void operator()(ID3D11DeviceContext& context) override;

		private:

			ObjectPtr<TType> ownership_;	///< \brief Used to hold the ownership for the first time.

			ObjectWeakPtr<TType> subject_;	///< \brief Object that needs to be committed.

		};

		///////////////////////// COMMITTER /////////////////////////////////

		template <typename TType>
		Committer<TType>::Committer(const ObjectPtr<TType>& subject) :
			subject_(subject),
			ownership_(subject){}

		template <typename TType>
		inline void Committer<TType>::operator()(ID3D11DeviceContext& context){

			auto ptr = subject_.Lock();

			if (ptr){

				ptr->Commit(context);

				ownership_ = nullptr;	// Release the ownership

			}

		}

	}

}
