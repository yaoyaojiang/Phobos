#include "BannerClass.h"

#include <Ext/Scenario/Body.h>

#include <New/Type/BannerTypeClass.h>

#include <Utilities/SavegameDef.h>
#include <MessageListClass.h>
#include <cwchar>  
#include <cstdlib>  
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
wchar_t* replaceSubstringWithCsf(const std::wstring& wstr, const std::wstring& wss, const std::wstring replaceStr)
{
	std::wstring result = wstr; // 初始化为原始宽字符字符串  
	size_t pos = result.find(wss); // 查找子字符串的位置
	if (pos != std::wstring::npos)
	{
		// 创建整数的字符串表示  
		// 替换子字符串  
		result.replace(pos, wss.length(), replaceStr);
	}
	size_t size = result.size() + 1;
	wchar_t* r = new wchar_t[size];
	std::wcscpy(r, result.c_str());
	return r;
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
wchar_t* concatenateWStrings(const wchar_t* const* strings, int count)
{
	// 初始化结果字符串的总长度为0  
	size_t totalLength = 0;

	// 计算所有非空字符串的总长度  
	for (int i = 0; i < count; ++i)
	{
		if (strings[i] != nullptr)
		{ // 检查指针是否非空  
			totalLength += wcslen(strings[i]);
		}
	}
	// 加上一个额外的 wchar_t 用于存储空字符 '\0'  
	totalLength += 1;
	// 分配足够的空间来存储结果字符串  
	wchar_t* result = static_cast<wchar_t*>(malloc(totalLength * sizeof(wchar_t)));
	if (!result)
	{
		// 内存分配失败，返回 nullptr  
		return nullptr;
	}

	// 初始化一个指针来跟踪 result 的当前位置  
	wchar_t* current = result;

	// 遍历字符串数组并将它们复制到结果字符串中，跳过 nullptr  
	for (int i = 0; i < count; ++i)
	{
		if (strings[i] != nullptr)
		{
			const wchar_t* str = strings[i];
			while (*str != L'\0')
			{
				*current++ = *str++; // 递增 str 指针来移动到下一个字符  
			}
		}
	}
	// 在结果字符串的末尾添加一个空字符  
	*current = L'\0';
	return result;
}
void BannerClass::RenderCSF(int x, int y)
{
	RectangleStruct rect;
	DSurface::Composite->GetRect(&rect);

	auto textc1 = this->Type->CSF.Get().Text;
	auto textc2 = this->Type->CSF2.Get().Text;
	auto textc3 = this->Type->CSF3.Get().Text;
	auto textc4 = this->Type->CSF4.Get().Text;
	auto textc5 = this->Type->CSF5.Get().Text;
	auto textc6 = this->Type->CSF6.Get().Text;
	auto textc7 = this->Type->CSF7.Get().Text;
	auto textc8 = this->Type->CSF8.Get().Text;
	auto textc9 = this->Type->CSF9.Get().Text;
	auto textc10 = this->Type->CSF10.Get().Text;
	auto textc11 = this->Type->CSF11.Get().Text;
	auto textc12 = this->Type->CSF12.Get().Text;
	auto textc13 = this->Type->CSF13.Get().Text;
	auto textc14 = this->Type->CSF14.Get().Text;
	auto textc15 = this->Type->CSF15.Get().Text;
	auto textc16 = this->Type->CSF16.Get().Text;
	auto textc17 = this->Type->CSF17.Get().Text;
	auto textc18 = this->Type->CSF18.Get().Text;
	auto textc19 = this->Type->CSF19.Get().Text;
	auto textc20 = this->Type->CSF20.Get().Text;
	const wchar_t* strings[] = { textc1, textc2 , textc3, textc4, textc5, textc6, textc7, textc8, textc9, textc10, textc11, textc12, textc13, textc14, textc15, textc16, textc17, textc18, textc19, textc20 };
	int count = sizeof(strings) / sizeof(strings[0]);
    wchar_t* textc = concatenateWStrings(reinterpret_cast<const wchar_t* const*>(strings), count);
	if (textc)
	{
		if (this->Type->CsfConvert)
		{
			textc = csfConvertA(textc);
		}
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
	free(textc);
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
