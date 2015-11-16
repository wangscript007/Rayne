//
//  RNInputControl.h
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_INPUTCONTROL_H_
#define __RAYNE_INPUTCONTROL_H_

#include "../Base/RNBase.h"
#include "../Data/RNIntrusiveList.h"
#include "../Objects/RNObject.h"
#include "../Objects/RNNumber.h"
#include "../Objects/RNArray.h"
#include "../Objects/RNString.h"
#include "../Objects/RNDictionary.h"

namespace RN
{
	class InputDevice;
	class InputControl : public Object
	{
	public:
		friend class InputDevice;

		enum class Type
		{
			Group,
			DeltaAxis,
			LinearAxis,
			RotationAxis,
			Slider,
			Hatswitch,
			Button,
			KeyButton
		};

		RNAPI ~InputControl() override;

		template<class T>
		T *GetValue() const
		{
			Object *value = __GetValue();
			if(value)
				return value->Downcast<T>();

			return nullptr;
		}

		Type GetType() const { return _type; }
		const String *GetName() const { return _name; }
		InputDevice *GetDevice() const { return _device; }
		InputControl *GetParent() const { return _parent; }

		RNAPI InputControl *GetFirstControl() const;
		RNAPI InputControl *GetNextControl() const;
		RNAPI InputControl *GetControlWithName(const String *name) const;

		RNAPI virtual void AddControl(InputControl *control);
		RNAPI virtual void Update();

		RNAPI bool IsControlToggling(const String *name) const;
		RNAPI Object *GetControlValue(const String *name) const;

		RNAPI virtual bool IsGroup() const;
		RNAPI virtual bool IsToggle() const;
		RNAPI virtual bool IsContinuous() const;

	protected:
		RNAPI InputControl(const String *name, Type type);

		RNAPI virtual void Start();
		RNAPI virtual void End();
		RNAPI virtual void UpdateValue(Object *value);

	private:
		Object *__GetValue() const { return _value; }
		void __LinkIntoDeviceWithParent(InputDevice *device, InputControl *parent);

		String *_name;
		InputDevice *_device;
		InputControl *_parent;

		Type _type;
		Object *_value;
		bool _toggling;

		IntrusiveList<InputControl> _controls;
		IntrusiveList<InputControl>::Member _controlsEntry;
		Dictionary *_controlMap;
		Array *_controlGroups;

		RNDeclareMeta(InputControl)
	};

	class InputControlGroup : public InputControl
	{
	public:
		RNAPI InputControlGroup();

		RNAPI bool IsGroup() const final;

	private:
		RNDeclareMeta(InputControlGroup);
	};

	class ButtonControl : public InputControl
	{
	public:
		RNAPI ButtonControl(const String *name, Type type);

		RNAPI void SetPressed(bool pressed);
		RNAPI bool IsPressed() const { return _pressed; }

		RNAPI bool IsToggle() const final;

	private:
		bool _pressed;
		RNDeclareMeta(ButtonControl)
	};

	class AxisControl : public InputControl
	{
	public:
		enum class Axis
		{
			None,
			X,
			Y,
			Z
		};

		RNAPI AxisControl(const String *name, Type type, Axis axis);

		RNAPI void SetRange(float min, float max, float deadZone);
		RNAPI virtual void SetValue(float value);

		RNAPI bool IsContinuous() const final;

		RNAPI float GetCenter() const { return _center; }
		RNAPI float GetMin() const { return _min; }
		RNAPI float GetMax() const { return _max; }
		RNAPI float GetDeadZone() const { return _deadZone; }
		RNAPI float GetNormalizer() const { return _normalizer; }

	private:
		Axis _axis;

		float _center;
		float _min;
		float _max;
		float _deadZone;
		float _normalizer;

		RNDeclareMeta(AxisControl)
	};

	class DeltaAxisControl : public AxisControl
	{
	public:
		RNAPI DeltaAxisControl(const String *name, Axis axis);

		RNAPI void Update() override;
		RNAPI void SetValue(float value) override;

	private:
		RNDeclareMeta(DeltaAxisControl)
	};
}

#endif /* __RAYNE_INPUTCONTROL_H_ */
