
#pragma once

#include "rh_texture_loader.h"

#include <type_traits>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <cassert>
#include <memory>

namespace rh {

	class TexturePak;

	class Texture {

		rh_texpak_idx idx;

		friend class TexturePak;

		std::shared_ptr<TexturePak> pak; // only to prevent destruction of parent TexturePak

		Texture( std::shared_ptr<TexturePak> pak, rh_texpak_idx idx )
			:	pak(pak),
				idx(idx)
		{}

		template<typename _T> _T GetDepth(std::true_type) const {

			unsigned int native_depth;
			rh_texpak_get_depthi(idx, &native_depth);
			return static_cast<_T>(native_depth);
		}

		template<typename _T> _T GetDepth(std::false_type) const {

			float native_depth;
			rh_texpak_get_depthf(idx, &native_depth);
			return static_cast<_T>(native_depth);
		}

	public:

		~Texture() {
			rh_texpak_release( idx );
		}

		GLuint GetTexture() const {

			GLuint t;
			rh_texpak_get_texture(idx, &t);
			return t;
		}

		template<typename _T>
		void GetPixelSize(_T & out_w, _T & out_h) const {

			unsigned int native_w, native_h;
			rh_texpak_get_size(idx, &native_w, &native_h);
			out_w = static_cast<_T>(native_w);
			out_h = static_cast<_T>(native_h);
		}

		template<typename _T>
		_T GetPixelWidth() const {

			unsigned int native_w;
			rh_texpak_get_size(idx, &native_w, NULL);
			return static_cast<_T>(native_w);
		}

		template<typename _T>
		_T GetPixelHeight() const {

			unsigned int native_h;
			rh_texpak_get_size(idx, NULL, &native_h);
			return static_cast<_T>(native_h);
		}

		template<typename _T> _T GetDepth() const {

			return GetDepth<_T>(std::is_integral<_T>());
		}

		void ReadCoords(int dim, int stride, float * coords) const {

			rh_texpak_get_coords(idx, dim, stride, coords);
		}
	};

	class TexturePak {

		rh_texpak_handle handle;

		bool isloaded{false};

		std::weak_ptr<TexturePak> wthis;

		TexturePak(const char * pakfile, int flags) {

			if( rh_texpak_open( pakfile, &handle, flags ) != 0 )
				throw OpenException();
		}

	public:

		class OpenException 	: public std::exception {public: const char * what() const throw() { return "TexturePak::OpenException"; } };
		class LoadException 	: public std::exception {public: const char * what() const throw() { return "TexturePak::LoadException"; } };
		class LookupException 	: public std::runtime_error { using std::runtime_error::runtime_error; };

		typedef enum {

			FILESYSTEM 				= RH_TEXPAK_FILESYSTEM,				/* load from the filesystem */
			ANDROID_APK 			= RH_TEXPAK_ANDROID_APK,			/* load from android APK */
			PLATFORM_DEFAULT		= RH_TEXPAK_APP,					/* load from default for this platform */
			ENABLE_TEXTURE_ARRAY 	= RH_TEXPAK_ENABLE_TEXTURE_ARRAY,	/* enable use of GL_EXT_texture_array */
		} texture_pak_flags_enum_t;

		static std::shared_ptr<TexturePak> MakeShared(const char * pakfile, int flags) {

			std::shared_ptr<TexturePak> obj =
				std::shared_ptr<TexturePak>( new TexturePak( pakfile, flags ) );

			obj->wthis = obj;

			return obj;
		}

		virtual ~TexturePak() {

			// _SHOULD_ be impossible for refcount to be nonzero.
			int e = rh_texpak_forceclose(handle);
			assert(e == 0);
		}

		void Load() {

			if(isloaded)
				return;

			if(rh_texpak_load(handle) != 0)
				throw LoadException();

			isloaded = true;
		}

		std::shared_ptr<Texture> GetTexture(const char * name) {

			rh_texpak_idx idx;

			if( rh_texpak_get(handle, name, &idx) == 0) {

				try {
					return std::shared_ptr<Texture>( new Texture( wthis.lock(), idx ) );

				} catch( const std::exception & e) {

					rh_texpak_release(idx);
					throw;
				}
			}
			else
				throw LookupException(name);
		}

		GLenum GetTarget() const {

			GLenum target;
			rh_texpak_get_textarget(handle, &target);
			return target;
		}
	};
}

