#pragma once

#include <Utilities/Enum.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class BannerTypeClass final : public Enumerable<BannerTypeClass>
{
public:

	//PCX
	Valueable<BSurface*> PCX;

	//SHP
	Valueable<SHPStruct*> Shape;
	CustomPalette Palette;

	//CSF
	Valueable<CSFText> CSF;
	Valueable<ColorStruct> CSF_Color;
	Valueable<bool> CSF_Background;

	//VariableFormat
	Valueable<BannerNumberType> VariableFormat;
	Valueable<CSFText> VariableFormat_Label;
	Valueable<CSFText> CSF2;
	Valueable<CSFText> CSF3;
	Valueable<CSFText> CSF4;
	Valueable<CSFText> CSF5;
	Valueable<CSFText> CSF6;
	Valueable<CSFText> CSF7;
	Valueable<CSFText> CSF8;
	Valueable<CSFText> CSF9;
	Valueable<CSFText> CSF10;
	Valueable<CSFText> CSF11;
	Valueable<CSFText> CSF12;
	Valueable<CSFText> CSF13;
	Valueable<CSFText> CSF14;
	Valueable<CSFText> CSF15;
	Valueable<CSFText> CSF16;
	Valueable<CSFText> CSF17;
	Valueable<CSFText> CSF18;
	Valueable<CSFText> CSF19;
	Valueable<CSFText> CSF20;

	BannerType BannerType;

	Valueable<bool> CsfConvert;

	BannerTypeClass(const char* pTitle = NONE_STR);
	~BannerTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& stm) override;
	virtual void SaveToStream(PhobosStreamWriter& stm) override;

private:

	void DetermineType();

	template <typename T>
	void Serialize(T& Stm);
};
