//
//  RNUILabel.cpp
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNUILabel.h"
#include "RNResourcePool.h"

namespace RN
{
	namespace UI
	{
		RNDeclareMeta(Label)
		
		Label::Label()
		{
			Initialize();
		}
		
		Label::~Label()
		{
			_string->Release();
			_font->Release();
			
			delete _typesetter;
		}
		
		void Label::Initialize()
		{
			_isDirty = false;
			
			_font      = nullptr;
			_model     = nullptr;
			
			_string     = new AttributedString(RNCSTR(""));
			_typesetter = new Typesetter(_string, Bounds());
			_typesetter->SetAllowPartiallyClippedLined(false);
			
			SetFont(ResourcePool::SharedInstance()->ResourceWithName<Font>(kRNResourceKeyDefaultFont));
			SetTextColor(Color::White());
			SetInteractionEnabled(false);
			
			SetAlignment(TextAlignment::Left);
			SetLineBreak(LineBreakMode::TruncateTail);
			SetNumberOfLines(1);
		}
		
		void Label::SetFont(Font *font)
		{
			RN_ASSERT(font, "Font mustn't be NULL!");
			
			if(_font)
				_font->Release();
			
			_font    = font->Retain();
			_isDirty = true;

			if(_string)
			{
				String *text = _string->String()->Retain();
				SetText(text);
				text->Release();
			}
		}
		
		void Label::SetText(String *text)
		{
			RN_ASSERT(text, "Text mustn't be NULL!");
			
			if(_string)
				_string->Release();
			
			_string = new AttributedString(text);
			_string->AddAttribute(kRNTypesetterFontAttribute, _font, Range(0, text->Length()));
			
			_typesetter->SetText(_string);
			_isDirty = true;
		}
		
		void Label::SetAttributedText(AttributedString *text)
		{
			RN_ASSERT(text, "Text mustn't be NULL!");
			
			if(_string)
				_string->Release();
			
			_string  = text->Retain();
			_isDirty = true;
			
			_typesetter->SetText(_string);
		}
		
		void Label::SetTextColor(const Color& color)
		{
			_color = color;
			DrawMaterial()->ambient = _color;
		}
		
		void Label::SetAlignment(TextAlignment alignment)
		{
			_alignment = alignment;
			_typesetter->SetAlignment(_alignment);
			_isDirty = true;
		}
		
		void Label::SetLineBreak(LineBreakMode mode)
		{
			_lineBreak = mode;
			_typesetter->SetLineBreak(_lineBreak);
			_isDirty = true;
		}
		
		void Label::SetNumberOfLines(uint32 lines)
		{
			_lines = lines;
			_typesetter->SetMaximumLines(_lines);
			_isDirty = true;
		}
		
		Vector2 Label::SizeThatFits()
		{
			return _typesetter->Dimensions();
		}
		
		void Label::SetFrame(const Rect& frame)
		{
			View::SetFrame(frame);
			
			_typesetter->SetFrame(Bounds());
			_isDirty = true;
		}
		
		void Label::Update()
		{
			View::Update();
			
			if(_isDirty)
			{
				if(_model)
				{
					_model->Release();
					_model = nullptr;
				}
				
				if(!_string)
					return;
				
				_model = _typesetter->LineModel()->Retain();
				_isDirty = false;
			}
		}
		
		void Label::Render(Renderer *renderer)
		{
			Update();
			
			if(_model)
			{
				RenderingObject object;
				
				object.transform = &_finalTransform;
				
				uint32 count = _model->Meshes(0);
				for(uint32 i=0; i<count; i++)
				{
					object.mesh     = _model->MeshAtIndex(0, i);
					object.material = _model->MaterialAtIndex(0, i);
					
					renderer->RenderObject(object);
				}
			}
			
			RenderChilds(renderer);
		}
	}
}
