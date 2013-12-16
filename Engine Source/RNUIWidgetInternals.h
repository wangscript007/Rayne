//
//  RNUIWidgetInternals.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_UIWIDGETINTERNALS_H__
#define __RAYNE_UIWIDGETINTERNALS_H__

#include "RNBase.h"
#include "RNArray.h"
#include "RNUIWidget.h"
#include "RNUIView.h"
#include "RNUIImage.h"
#include "RNUIImageView.h"
#include "RNUILabel.h"
#include "RNUIButton.h"
#include "RNUIControl.h"

namespace RN
{
	namespace UI
	{
		class WidgetBackgroundView : public View
		{
		public:
			WidgetBackgroundView(Widget *widget, Widget::Style style, Dictionary *tstyle);
			~WidgetBackgroundView();
			
			void SetState(Control::State state);
			void SetTitle(String *title);
			
			const EdgeInsets& GetBorder() const { return _border; }
			
			virtual void LayoutSubviews() override;
			
		private:
			void ParseStyle(Dictionary *style, Control::State state);
			void CreateButton(Widget::TitleControl style);
			
			ControlStateStore<Image> _backdropImages;
			ControlStateStore<Image> _shadowImages;
			ControlStateStore<Dictionary> _shadowExtents;
			
			Widget *_container;
			Widget::Style _style;
			Control::State _state;
			EdgeInsets _border;
			
			Label *_title;
			ImageView *_backdrop;
			ImageView *_shadow;
			
			Button *_closeButton;
			Button *_minimizeButton;
			Button *_maximizeButton;
			
			Array _controlButtons;
			
			RNDefineMeta(WidgetBackgroundView, View)
		};
	}
}

#endif /* __RAYNE_UIWIDGETINTERNALS_H__ */
