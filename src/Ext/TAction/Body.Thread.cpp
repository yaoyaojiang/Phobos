#include "Body.h"

#include<ScenarioClass.h>
#include <filesystem>
#include <Ext/Scenario/Body.h>
#include <TlHelp32.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <MessageListClass.h>
#include <TacticalClass.h>
#include <HouseClass.h>

std::atomic<bool> g_bCheatFileDetected = false;   // 文件作弊标志（子线程→主线程）
std::atomic<bool> g_bCheatProcessDetected = false; // 进程作弊标志（子线程→主线程）
std::atomic<bool> g_bStopThread = false;          // 主线程→子线程：停止信号
std::atomic<bool> g_bThreadRunning = false;       // 线程是否在运行
std::thread g_DetectionThread;
std::mutex g_cvMutex;
std::condition_variable g_cv;

// 作弊进程名字符串（提前加载，避免线程中反复加载）
static std::wstring g_CheatNames[5];
static std::wstring level_name;

std::wstring replaceSubstringWithIntegerTH(const std::wstring& wstr, const std::wstring& wss, int replacementValue)
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
wchar_t* csfConvertTH(const wchar_t* wstr)
{
	std::wstring inputStr(wstr);
	std::map<int, ExtendedVariable> variables = ScenarioExt::Global()->Variables[0];
	int i = 0;
	for (const auto& variable : variables)
	{
		std::wstring pattern = L"%var" + std::to_wstring(i) + L"%";
		inputStr = replaceSubstringWithIntegerTH(inputStr, pattern, variable.second.Value);
		i++;
	}

	// 转换std::wstring为wchar_t*  
	size_t size = inputStr.size() + 1; // +1 for the null-terminator  
	wchar_t* result = new wchar_t[size];
	std::wcscpy(result, inputStr.c_str());

	return result;
}
std::string WStringToAnsi(const std::wstring& wStr)
{
	if (wStr.empty())
		return "";

	// 计算需要缓冲区大小
	int bufLen = WideCharToMultiByte(
		CP_ACP,
		0,
		wStr.c_str(),
		-1,        // 自动识别字符串末尾 \0
		nullptr,
		0,
		nullptr,
		nullptr
	);

	std::string ans(bufLen, 0);
	WideCharToMultiByte(
		CP_ACP,
		0,
		wStr.c_str(),
		-1,
		&ans[0],
		bufLen,
		nullptr,
		nullptr
	);
	return ans;
}
void __stdcall StopCheatThreadForDll()
{
	g_bStopThread = true; // 发停止信号
	g_cv.notify_one();

	if (g_DetectionThread.joinable())
	{
		HANDLE hThread = g_DetectionThread.native_handle();
		DWORD result = WaitForSingleObject(hThread, 500); // 等 500ms

		if (result == WAIT_OBJECT_0)
		{
			g_DetectionThread.join(); // 正常结束
		}
		else
		{
			g_DetectionThread.detach(); // 超时则分离，避免卡死
		}
	}
}
static bool IsGameActive()
{
	// 1. 先检查指针是否为空（安全，不会崩溃）
	if (HouseClass::CurrentPlayer == nullptr)
	{
		Debug::Log("Not game playing.\n");
		return false;
	}


	std::string keyAnsi = WStringToAnsi(ScenarioClass::Instance->Name);
	Debug::Log("Now ScenarioClass Name is %s.\n", keyAnsi);
	if (level_name != ScenarioClass::Instance->Name) return false;
	// 2. 指针非空，但可能是野指针，用 SEH 安全访问
	__try
	{
		// 访问一个稳定的成员变量（GameMode 或 Theather）
		bool test = ScenarioClass::Instance->EndOfGame;
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;  // 野指针访问触发异常
	}
}
static void DetectionThreadFunc()
{
	g_bThreadRunning = true;
	Debug::Log("Cheat detection thread started.\n");

	while (!g_bStopThread)
	{
		// ===== 先检查停止信号和游戏状态 =====
		if (g_bStopThread)
			break;

		if (!IsGameActive())
		{
			Debug::Log("No active scenario, thread exit automatically\n");
			g_bStopThread = true;
			break;
		}

		// ===== 文件检测 =====
		auto FileExists = [](const wchar_t* path) -> bool
		{
			DWORD attr = GetFileAttributesW(path);
			return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
		};

		if (FileExists(L"shroud.shp") || FileExists(L"ecache03.mix"))
		{
			g_bCheatFileDetected = true;
			// 检测到作弊文件，跳过本次等待，立即进入下一次循环（继续检测）
			continue;
		}

		// ===== 进程检测 =====
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE)
		{
			// 快照失败，也跳过等待，立即重试（但避免频繁重试，可以加个小延时，但保持原逻辑）
			// 原代码是 continue，但这里不能 continue，因为会跳到循环开头跳过等待，但这样会频繁重试。
			// 但原代码也是 continue，此处保持一致。
			continue;
		}

		PROCESSENTRY32W pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32W);
		if (!Process32FirstW(hSnapshot, &pe32))
		{
			CloseHandle(hSnapshot);
			continue;
		}

		do
		{
			if (g_bStopThread)
				break;

			if (pe32.th32ProcessID == 0 || pe32.th32ProcessID == GetCurrentProcessId())
				continue;

			std::wstring processName(pe32.szExeFile);
			Debug::Log("Process name: %s\n", WStringToAnsi(processName).c_str());

			for (const auto& name : g_CheatNames)
			{
				std::string logText = std::to_string((unsigned long long)std::wstring::npos) + ".\n";
				Debug::Log(logText.c_str());
				std::string keyAnsi = WStringToAnsi(name);
				Debug::Log(keyAnsi.c_str());

				if (processName.find(name) != std::wstring::npos)
				{
					g_bCheatProcessDetected = true;
					Debug::Log("Check detected.\n");
					break;
				}
			}
			if (g_bCheatProcessDetected) break;
		}
		while (Process32NextW(hSnapshot, &pe32) && !g_bStopThread);

		CloseHandle(hSnapshot);

		// ===== 检测完成，等待 30 秒（放在最后） =====
		std::unique_lock<std::mutex> lock(g_cvMutex);
		g_cv.wait_for(lock, std::chrono::seconds(30), [] { return g_bStopThread.load(); });
		lock.unlock();
	}

	g_bThreadRunning = false;
	Debug::Log("Cheat detection thread stopped.\n");
}

bool TActionExt::CreateCheatDetectionThread(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	// 如果线程正在运行，不能初始化
	if (g_bThreadRunning)
		return false;

	// 如果还有旧的线程对象，先清理
	if (g_DetectionThread.joinable())
		g_DetectionThread.join();

	// 加载作弊进程名字符串（只加载一次）
	static bool bLoaded = false;
	if (!bLoaded)
	{
		wchar_t* pTmp = nullptr;
		pTmp = csfConvertTH(StringTable::LoadString("a:xiu"));

		g_CheatNames[0] = pTmp;  // 拷贝字符串内容
		delete[] pTmp;            // 马上释放new的堆内存

		pTmp = csfConvertTH(StringTable::LoadString("a:xiu2"));
		g_CheatNames[1] = pTmp;
		delete[] pTmp;

		pTmp = csfConvertTH(StringTable::LoadString("a:xiu3"));
		g_CheatNames[2] = pTmp;
		delete[] pTmp;

		pTmp = csfConvertTH(StringTable::LoadString("a:xiu4"));
		g_CheatNames[3] = pTmp;
		delete[] pTmp;

		pTmp = csfConvertTH(StringTable::LoadString("a:xiu5"));
		g_CheatNames[4] = pTmp;
		delete[] pTmp;

		pTmp = ScenarioClass::Instance->Name;
		level_name = pTmp;
		delete[] pTmp;

		bLoaded = true;
	}

	// 重置所有标志（但线程尚未启动）
	g_bCheatFileDetected = false;
	g_bCheatProcessDetected = false;
	g_bStopThread = false;
	g_bThreadRunning = false;

	// 清空线程对象（确保未启动）
	g_DetectionThread = std::thread(); // 默认构造，不启动
	return true;
}

// ==================== 【函数2】启动线程（开始循环检测） ====================
// 返回值：true=启动成功，false=启动失败
bool StartCheatDetectionThread()
{
	// 如果已经在运行，不能重复启动
	if (g_bThreadRunning)
		return false;

	// 如果线程对象还残留，先清理
	if (g_DetectionThread.joinable())
		g_DetectionThread.join();

	// 重置停止标志
	g_bStopThread = false;

	// 启动线程
	try
	{
		g_DetectionThread = std::thread(DetectionThreadFunc);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

// ==================== 【函数3】停止线程 ====================
bool TActionExt::StopCheatDetectionThread(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	g_bStopThread = true;
	g_cv.notify_one();
	if (g_DetectionThread.joinable())
	{
		HANDLE hThread = g_DetectionThread.native_handle();
		DWORD result = WaitForSingleObject(hThread, 1000); // 等待1秒
		if (result == WAIT_OBJECT_0)
		{
			g_DetectionThread.join();
		}
		else
		{
			g_DetectionThread.detach();  // 超时就分离，避免卡死
			Debug::Log("StopCheatDetectionThread: detached due to timeout.\n");
		}
	}
	return true;
}


// ==================== 【函数4】线程工作函数（子线程循环） ====================

// ==================== 【函数5】原有的触发行为（改造为读取两个独立标志） ====================
bool TActionExt::CheckCheating(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{

	// ---- 优先处理文件作弊 ----
	if (g_bCheatFileDetected)
	{
		ScenarioExt::Global()->IsUsingHalfShadow = true;
		CRT::swprintf(Phobos::wideBuffer, L"%d", 11);
		MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
		g_bCheatFileDetected = false; // 消费掉标志
		return true;
	}

	// ---- 再处理进程作弊 ----
	if (g_bCheatProcessDetected)
	{
		ScenarioExt::Global()->IsCheating = true;
		CRT::swprintf(Phobos::wideBuffer, L"%d", 21);
		MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
		g_bCheatProcessDetected = false; // 消费掉标志
		return true;
	}

	return false;
}

// ==================== 【函数6】触发行为里调用“启动线程” ====================
// 这个函数可以挂载到任何触发行为上（比如游戏开始、进入关卡）
bool TActionExt::StartCheatThread(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	return StartCheatDetectionThread();
}
