#include "BannerTypeClass.h"

#include <Utilities/GeneralUtils.h>
#include <Utilities/TemplateDef.h>

Enumerable<BannerTypeClass>::container_t Enumerable<BannerTypeClass>::Array;

BannerTypeClass::BannerTypeClass(const char* pTitle)
	: Enumerable<BannerTypeClass>(pTitle)
	, CSF_Color(Drawing::TooltipColor)
	, BannerType(BannerType::PCX)
{
}

const char* Enumerable<BannerTypeClass>::GetMainSection()
{
	return "BannerTypes";
}

inline void BannerTypeClass::DetermineType()
{
	if (PCX)
		BannerType = BannerType::PCX;
	else if (Shape)
		BannerType = BannerType::SHP;
	else if (VariableFormat != BannerNumberType::None)
		BannerType = BannerType::VariableFormat;
	else
		BannerType = BannerType::CSF;
}

void BannerTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;


	INI_EX exINI(pINI);

	this->PCX.Read(exINI, section, "Content.PCX");
	this->Shape.Read(exINI, section, "Content.SHP");
	this->Palette.LoadFromINI(pINI, section, "Content.SHP.Palette");
	this->CSF.Read(exINI, section, "Content.CSF");
	this->CSF_Color.Read(exINI, section, "Content.CSF.Color");
	this->CSF_Background.Read(exINI, section, "Content.CSF.DrawBackground");
	this->VariableFormat.Read(exINI, section, "Content.VariableFormat");
	this->VariableFormat_Label.Read(exINI, section, "Content.VariableFormat.Label");
	this->CsfConvert.Read(exINI, section, "Content.CSF.VariableConvert");
	this->CSF2.Read(exINI, section, "Content.CSF2");
	this->CSF3.Read(exINI, section, "Content.CSF3");
	this->CSF4.Read(exINI, section, "Content.CSF4");
	this->CSF5.Read(exINI, section, "Content.CSF5");
	this->CSF6.Read(exINI, section, "Content.CSF6");
	this->CSF7.Read(exINI, section, "Content.CSF7");
	this->CSF8.Read(exINI, section, "Content.CSF8");
	this->CSF9.Read(exINI, section, "Content.CSF9");
	this->CSF10.Read(exINI, section, "Content.CSF10");
	this->CSF11.Read(exINI, section, "Content.CSF11");
	this->CSF12.Read(exINI, section, "Content.CSF12");
	this->CSF13.Read(exINI, section, "Content.CSF13");
	this->CSF14.Read(exINI, section, "Content.CSF14");
	this->CSF15.Read(exINI, section, "Content.CSF15");
	this->CSF16.Read(exINI, section, "Content.CSF16");
	this->CSF17.Read(exINI, section, "Content.CSF17");
	this->CSF18.Read(exINI, section, "Content.CSF18");
	this->CSF19.Read(exINI, section, "Content.CSF19");
	this->CSF20.Read(exINI, section, "Content.CSF20");

	DetermineType();
}

template <typename T>
void BannerTypeClass::Serialize(T& stm)
{
	stm
		.Process(this->PCX)
		.Process(this->Shape)
		.Process(this->Palette)
		.Process(this->CSF)
		.Process(this->CSF_Color)
		.Process(this->CSF_Background)
		.Process(this->VariableFormat)
		.Process(this->VariableFormat_Label)
		.Process(this->BannerType)
		.Process(this->CsfConvert)
		.Process(this->CSF2)
		.Process(this->CSF3)
		.Process(this->CSF4)
		.Process(this->CSF5)
		.Process(this->CSF6)
		.Process(this->CSF7)
		.Process(this->CSF8)
		.Process(this->CSF9)
		.Process(this->CSF10)
		.Process(this->CSF11)
		.Process(this->CSF12)
		.Process(this->CSF13)
		.Process(this->CSF14)
		.Process(this->CSF15)
		.Process(this->CSF16)
		.Process(this->CSF17)
		.Process(this->CSF18)
		.Process(this->CSF19)
		.Process(this->CSF20)
		;
}

void BannerTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void BannerTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
