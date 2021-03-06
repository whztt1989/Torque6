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

#include "console/consoleTypes.h"
#include "console/consoleInternal.h"
#include "materials.h"
#include "materialTemplate.h"
#include "graphics/core.h"
#include "scene/components/baseComponent.h"

// Script bindings.
#include "materialTemplate_Binding.h"

// Debug Profiling.
#include "debug/profiler.h"

// bgfx/bx
#include <bgfx/bgfx.h>
#include <bx/fpumath.h>

// Assimp - Asset Import Library
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/types.h>

#include "nodes/opaqueNode.h"

namespace Materials
{
   IMPLEMENT_CONOBJECT(BaseNode);

   void BaseNode::initPersistFields()
   {
      // Call parent.
      Parent::initPersistFields();

      addField("Position", TypePoint2I, Offset(mPosition, BaseNode), "");
   }

   BaseNode* BaseNode::findNode(const MaterialGenerationSettings &settings, const char* name)
   {
      if ( dStrlen(name) < 1 )
         return NULL;

      BaseNode* node = dynamic_cast<BaseNode*>(settings.matTemplate->findObjectByInternalName(name));

      if (settings.matVariant != NULL)
         return Materials::getVariantNode(settings.matVariant, node);

      return node;
   }

   const char* BaseNode::getVertexReference(const MaterialGenerationSettings &settings, ReturnType refType, U32 flags)
   {
      switch(refType)
      {
         case ReturnFloat:
            return "0.0";
         case ReturnVec2:
            return "vec2(0.0, 0.0)";
         case ReturnVec3:
            return "vec3(0.0, 0.0, 0.0)";
         case ReturnVec4:
            return "vec4(0.0, 0.0, 0.0, 0.0)";
      }
      return "";
   }

   const char* BaseNode::getPixelReference(const MaterialGenerationSettings &settings, ReturnType refType, U32 flags)
   {
      switch(refType)
      {
         case ReturnFloat:
            return "0.0";
         case ReturnVec2:
            return "vec2(0.0, 0.0)";
         case ReturnVec3:
            return "vec3(0.0, 0.0, 0.0)";
         case ReturnVec4:
            return "vec4(0.0, 0.0, 0.0, 0.0)";
      }
      return "";
   }

   IMPLEMENT_CONOBJECT(MaterialTemplate);

   MaterialTemplate::MaterialTemplate()
   {
      clearShader();
   }

   MaterialTemplate::~MaterialTemplate()
   {
      //
   }

   void MaterialTemplate::addObject(SimObject* obj)
   {
      Parent::addObject(obj);
   }

   U8 MaterialTemplate::getUnusedTextureSlot()
   {
      U8 slot = 0;
      for (U8 n = 0; n < 16; ++n)
      {
         if (mUsedTextureSlots[n])
            slot = (n + 1);
      }
      return slot;
   }

   void MaterialTemplate::clearShader()
   {
      for (U8 n = 0; n < 16; ++n)
         mUsedTextureSlots[n] = false;

      vertexShaderInputs[0] = '\0';
      vertexShaderInputsPos = 0;
      vertexShaderOutputs[0] = '\0';
      vertexShaderOutputsPos = 0;

      vertexShaderHeader[0] = '\0';
      vertexShaderHeaderPos = 0;
      vertexShaderBody[0] = '\0';
      vertexShaderBodyPos = 0;

      pixelShaderHeader[0] = '\0';
      pixelShaderHeaderPos = 0;
      pixelShaderBody[0] = '\0';
      pixelShaderBodyPos = 0;
   }

   bool MaterialTemplate::isValid(const MaterialGenerationSettings &settings)
   {
      Materials::BaseNode* rootNode = getRootNode(settings);
      if (rootNode != NULL)
         return true;

      return false;
   }

   Materials::BaseNode* MaterialTemplate::getRootNode(const MaterialGenerationSettings &settings)
   {
      for( S32 n = 0; n < this->size(); ++n )
      {
         Materials::BaseNode* node = dynamic_cast<Materials::BaseNode*>(this->at(n));
         if ( !node->isRootNode ) continue;

         if (settings.matVariant != NULL)
            return Materials::getVariantNode(settings.matVariant, node);

         return node;
      }

      return NULL;
   }

   const char* MaterialTemplate::getVertexShaderOutput(const MaterialGenerationSettings &settings)
   {
      Materials::BaseNode* rootNode = getRootNode(settings);
      if (!rootNode) return "";
      rootNode->generateVertex(settings);

      U32 pos = 0;

      dStrcpy(vertexShaderFinal, "$input ");
      pos += dStrlen("$input ");
      vertexShaderInputs[vertexShaderInputsPos - 2] = ' ';
      vertexShaderInputs[vertexShaderInputsPos - 1] = '\n';
      dStrcpy(&vertexShaderFinal[pos], vertexShaderInputs);
      pos += vertexShaderInputsPos;

      dStrcpy(&vertexShaderFinal[pos], "$output ");
      pos += dStrlen("$output ");
      vertexShaderOutputs[vertexShaderOutputsPos - 2] = ' ';
      vertexShaderOutputs[vertexShaderOutputsPos - 1] = '\n';
      dStrcpy(&vertexShaderFinal[pos], vertexShaderOutputs);
      pos += vertexShaderOutputsPos;

      dStrcpy(&vertexShaderFinal[pos], vertexShaderHeader);
      pos += vertexShaderHeaderPos;

      dStrcpy(&vertexShaderFinal[pos], "void main()\n{");
      pos += dStrlen("void main()\n{");

      dStrcpy(&vertexShaderFinal[pos], vertexShaderBody);
      pos += vertexShaderBodyPos;

      dStrcpy(&vertexShaderFinal[pos], "}");
      pos += dStrlen("}");

      return vertexShaderFinal;
   }

   const char* MaterialTemplate::getPixelShaderOutput(const MaterialGenerationSettings &settings)
   {
      Materials::BaseNode* rootNode = getRootNode(settings);
      if (!rootNode) return "";
      rootNode->generatePixel(settings);

      U32 pos = 0;

      dStrcpy(pixelShaderFinal, "$input ");
      pos += dStrlen("$input ");
      vertexShaderOutputs[vertexShaderOutputsPos - 2] = ' ';
      vertexShaderOutputs[vertexShaderOutputsPos - 1] = '\n';
      dStrcpy(&pixelShaderFinal[pos], vertexShaderOutputs);
      pos += vertexShaderOutputsPos;

      dStrcpy(&pixelShaderFinal[pos], pixelShaderHeader);
      pos += pixelShaderHeaderPos;

      dStrcpy(&pixelShaderFinal[pos], "void main()\n{");
      pos += dStrlen("void main()\n{");

      dStrcpy(&pixelShaderFinal[pos], pixelShaderBody);
      pos += pixelShaderBodyPos;

      dStrcpy(&pixelShaderFinal[pos], "}");
      pos += dStrlen("}");

      return pixelShaderFinal;
   }

   void MaterialTemplate::addVertexHeader(const char *format, ...)
   {
      char text[512];

      va_list args;
      va_start(args, format);
      dVsprintf(text, 512, format, args);
      va_end(args);

      dStrcpy(&vertexShaderHeader[vertexShaderHeaderPos], text);
      vertexShaderHeaderPos += dStrlen(text);
      dStrcpy(&vertexShaderHeader[vertexShaderHeaderPos], "\n");
      vertexShaderHeaderPos += dStrlen("\n");
   }

   void MaterialTemplate::addVertexInput(const char *format, ...)
   {
      char text[512];

      va_list args;
      va_start(args, format);
      dVsprintf(text, 512, format, args);
      va_end(args);

      dStrcpy(&vertexShaderInputs[vertexShaderInputsPos], text);
      vertexShaderInputsPos += dStrlen(text);
      dStrcpy(&vertexShaderInputs[vertexShaderInputsPos], ", ");
      vertexShaderInputsPos += dStrlen(", ");
   }

   void MaterialTemplate::addVertexOutput(const char *format, ...)
   {
      char text[512];

      va_list args;
      va_start(args, format);
      dVsprintf(text, 512, format, args);
      va_end(args);

      dStrcpy(&vertexShaderOutputs[vertexShaderOutputsPos], text);
      vertexShaderOutputsPos += dStrlen(text);
      dStrcpy(&vertexShaderOutputs[vertexShaderOutputsPos], ", ");
      vertexShaderOutputsPos += dStrlen(", ");
   }

   void MaterialTemplate::addVertexBody(const char *format, ...)
   {
      char text[512];

      va_list args;
      va_start(args, format);
      dVsprintf(text, 512, format, args);
      va_end(args);

      dStrcpy(&vertexShaderBody[vertexShaderBodyPos], text);
      vertexShaderBodyPos += dStrlen(text);
      dStrcpy(&vertexShaderBody[vertexShaderBodyPos], "\n");
      vertexShaderBodyPos += dStrlen("\n");
   }

   void MaterialTemplate::addPixelHeader(const char *format, ...)
   {
      char text[512];

      va_list args;
      va_start(args, format);
      dVsprintf(text, 512, format, args);
      va_end(args);

      dStrcpy(&pixelShaderHeader[pixelShaderHeaderPos], text);
      pixelShaderHeaderPos += dStrlen(text);
      dStrcpy(&pixelShaderHeader[pixelShaderHeaderPos], "\n");
      pixelShaderHeaderPos += dStrlen("\n");
   }

   void MaterialTemplate::addPixelBody(const char *format, ...)
   {
      char text[512];

      va_list args;
      va_start(args, format);
      dVsprintf(text, 512, format, args);
      va_end(args);

      dStrcpy(&pixelShaderBody[pixelShaderBodyPos], text);
      pixelShaderBodyPos += dStrlen(text);
      dStrcpy(&pixelShaderBody[pixelShaderBodyPos], "\n");
      pixelShaderBodyPos += dStrlen("\n");
   }
}