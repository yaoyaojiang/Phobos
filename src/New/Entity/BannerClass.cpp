#include "BannerClass.h"

#include <Ext/Scenario/Body.h>

#include <New/Type/BannerTypeClass.h>

#include <Utilities/SavegameDef.h>
#include <MessageListClass.h>

std::vector<std::unique_ptr<BannerClass>> BannerClass::Array;

BannerClass::BannerClass
(
	BannerTypeClass* pBannerType,
	int id,
	const CoordStruct& position,
	int variable,
	bool isGlobalVariable
)
	: Type(pBannerType)
	, ID(id)
	, Position(position)
	, Variable(variable)
	, IsGlobalVariable(isGlobalVariable)
{
}

void BannerClass::Render()
{
	int x = static_cast<int>(this->Position.X / 100.0 * DSurface::Composite->Width);
	int y = static_cast<int>(this->Position.Y / 100.0 * DSurface::Composite->Height);

	switch (this->Type->BannerType)
	{
	case BannerType::PCX:
		this->RenderPCX(x, y);
		break;
	case BannerType::SHP:
		this->RenderSHP(x, y);
		break;
	case BannerType::CSF:
		this->RenderCSF(x, y);
		break;
	case BannerType::VariableFormat:
		this->RenderVariable(x, y);
		break;
	default:
		break;
	}
}
std::wstring replaceSubstringWithVar(const std::wstring& wstr, const std::wstring& wss, int replacementValue)
{
	std::wstring result = wstr; // 初始化为原始宽字符字符串  
	size_t pos = result.find(wss); // 查找子字符串的位置  
	if (pos != std::wstring::npos)
	{
		// 创建整数的字符串表示  
		std::wstring replacementStr = std::to_wstring(replacementValue);
		// 替换子字符串  
		result.replace(pos, wss.length(), replacementStr);
	}
	return result;
}

wchar_t* csfConvertA(const wchar_t* wstr)
{
	std::wstring inputStr(wstr);
	std::map<int, ExtendedVariable> variables = ScenarioExt::Global()->Variables[0];
	int i = 0;
	for (const auto& variable : variables)
	{
		std::wstring pattern = L"%var" + std::to_wstring(i) + L"%";
		inputStr = replaceSubstringWithVar(inputStr, pattern, variable.second.Value);
		i++;
	}

	// 转换std::wstring为wchar_t*  
	size_t size = inputStr.size() + 1; // +1 for the null-terminator  
	wchar_t* result = new wchar_t[size];
	std::wcscpy(result, inputStr.c_str());

	return result;
}
void BannerClass::RenderPCX(int x, int y)
{
	int xsize = 0;
	int ysize = 0;

	if (this->Type->BannerType == BannerType::PCX)
	{
		BSurface* pcx = this->Type->PCX;

		if (pcx == nullptr)
			return;

		x = x - pcx->Width / 2;
		y = y - pcx->Height / 2;
		xsize = pcx->Width;
		ysize = pcx->Height;

		RectangleStruct bounds(x, y, xsize, ysize);
		PCX::Instance->BlitToSurface(&bounds, DSurface::Composite, pcx);
	}
}

void BannerClass::RenderSHP(int x, int y)
{
	if (this->Type->BannerType == BannerType::SHP)
	{
		SHPStruct* shape = this->Type->Shape;
		ConvertClass* palette = this->Type->Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);

		if (shape == nullptr)
			return;

		x = x - shape->Width / 2;
		y = y - shape->Height / 2;

		Point2D pos(x, y);
		DSurface::Composite->DrawSHP
		(
			palette,
			shape,
			this->ShapeFrameIndex,
			&pos,
			&DSurface::ViewBounds,
			BlitterFlags::None,
			0,
			0,
			ZGradient::Ground,
			1000,
			0,
			nullptr,
			0,
			0,
			0
		);

		this->ShapeFrameIndex++;

		if (this->ShapeFrameIndex >= shape->Frames)
			this->ShapeFrameIndex = 0;
	}
}


void BannerClass::RenderCSF(int x, int y)
{
	RectangleStruct rect;
	DSurface::Composite->GetRect(&rect);

	auto textc = this->Type->CSF.Get().Text;
	if (this->Type->CsfConvert)
	{
		textc = csfConvertA(textc);
	}

	auto wanted = Drawing::GetTextDimensions(textc, { 0,0 }, 0, 2, 0);

	/*RectangleStruct rectangle = {右上
		DSurface::Composite->GetWidth()-wanted.Width/2,
		0 ,
		wanted.Width + 10,
		wanted.Height + 10
	}; */
	RectangleStruct rectangle = {
		 wanted.Width / 2,
		DSurface::Composite->GetHeight() - wanted.Height * 3 ,
		wanted.Width + 10,
		wanted.Height + 10
	};
	int t, p;
	if (y > 0)
		t = y;
	else
		t = rectangle.Y;
	if (x > 0)
		p = x;
	else
		p = rectangle.X;
	//Point2D location { rectangle.X + 5,5 };
	Point2D pos(p, t);

	std::wstring text(textc);
	if (this->Type->VariableFormat!=BannerNumberType::None)
	{
		const auto& variables = ScenarioExt::Global()->Variables[this->IsGlobalVariable != 0];
    	const auto& it = variables.find(this->Variable);

	    if (it != variables.end())
	    	text += std::to_wstring(it->second.Value);
    }

	TextPrintType textFlags = TextPrintType::UseGradPal
		| TextPrintType::Center
		| TextPrintType::Metal12
		| (this->Type->CSF_Background
			? TextPrintType::Background
			: TextPrintType::LASTPOINT);

	DSurface::Composite->DrawText
	(
		text.c_str(),
		&rect,
		&pos,
		Drawing::RGB_To_Int(this->Type->CSF_Color),
		0,
		textFlags
	);
	delete textc;
}

void BannerClass::RenderVariable(int x, int y)
{
	//nothing here
}

template <typename T>
bool BannerClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->ID)
		.Process(this->Type)
		.Process(this->Position)
		.Process(this->Variable)
		.Process(this->ShapeFrameIndex)
		.Process(this->IsGlobalVariable)
		.Success();
}

bool BannerClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return Serialize(stm);
}

bool BannerClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<BannerClass*>(this)->Serialize(stm);
}

void BannerClass::Clear()
{
	Array.clear();
}

bool BannerClass::LoadGlobals(PhobosStreamReader& stm)
{
	return stm
		.Process(Array)
		.Success();
}

bool BannerClass::SaveGlobals(PhobosStreamWriter& stm)
{
	return stm
		.Process(Array)
		.Success();
}
