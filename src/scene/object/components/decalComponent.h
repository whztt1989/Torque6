//-----------------------------------------------------------------------------
// Copyright (c) 2015 Andrew Mac
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _DECAL_COMPONENT_H_
#define _DECAL_COMPONENT_H_

#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif

#ifndef _VERTEXLAYOUTS_H_
#include "graphics/core.h"
#endif

#ifndef _TEXTURE_MANAGER_H_
#include "graphics/TextureManager.h"
#endif

#ifndef _SHADERS_H_
#include "graphics/shaders.h"
#endif

#ifndef _MESH_ASSET_H_
#include "scene/object/meshAsset.h"
#endif

#ifndef _RENDERING_H_
#include "rendering/rendering.h"
#endif

#ifndef _BASE_COMPONENT_H_
#include "baseComponent.h"
#endif

#ifndef _RENDERABLE_H_
#include <rendering/renderable.h>
#endif

#ifndef NANOVG_H
#include <../common/nanovg/nanovg.h>
#endif

namespace Scene 
{
   class DLL_PUBLIC DecalComponent : public virtual Rendering::Renderable, public BaseComponent
   {
      private:
         typedef BaseComponent Parent;

         Vector<Rendering::UniformData>         mUniforms;
         Vector<Rendering::TextureData>         mTextures;
         Rendering::RenderData*                 mRenderData;
         AssetPtr<Graphics::ShaderAsset>        mShaderAsset;
         StringTableEntry                       mTexturePath;
         bgfx::TextureHandle                    mTexture;

      public:
         DecalComponent();
         ~DecalComponent();

         virtual void preRender();
         virtual void render();
         virtual void postRender();
         virtual void onAddToScene();
         virtual void onRemoveFromScene();
         
         void refresh();
         void loadTexture(StringTableEntry path);

         static void initPersistFields();
         static bool setTexture(void* obj, const char* data) { static_cast<DecalComponent*>(obj)->loadTexture(StringTable->insert(data)); return false; }

         DECLARE_CONOBJECT(DecalComponent);
   };
}

#endif // _DECAL_COMPONENT_H_