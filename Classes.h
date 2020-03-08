#pragma once
#include "XorCompileTime.hpp"

template< typename T >
T GetVFunc(void* vTable, int iIndex) {
	return (*(T**)vTable)[iIndex];
}

class ConVar;

class IAppSystem {
public:
	virtual void func0() = 0;
	virtual void func1() = 0;
	virtual void func2() = 0;
	virtual void func3() = 0;
	virtual void func4() = 0;
	virtual void func5() = 0;
	virtual void func6() = 0;
	virtual void func7() = 0;
	virtual void func8() = 0;
	virtual void func9() = 0;
};

class Color {
public:
	unsigned char _color[4];

	Color() {
		*((int*)this) = 0;
	}

	Color(int color32) {
		*((int*)this) = color32;
	}

	Color(int _r, int _g, int _b) {
		SetColor(_r, _g, _b, 255);
	}

	Color(int _r, int _g, int _b, int _a) {
		SetColor(_r, _g, _b, _a);
	}

	void SetColor(int _r, int _g, int _b, int _a = 255) {
		_color[0] = (unsigned char)_r;
		_color[1] = (unsigned char)_g;
		_color[2] = (unsigned char)_b;
		_color[3] = (unsigned char)_a;
	}

	void SetColor(float _r, float _g, float _b, float _a = 1.f) {
		_color[0] = (unsigned char)(_r * 255);
		_color[1] = (unsigned char)(_g * 255);
		_color[2] = (unsigned char)(_b * 255);
		_color[3] = (unsigned char)(_a * 255);
	}

	void SetColor(float* colorf) {
		_color[0] = (unsigned char)(colorf[0] * 255);
		_color[1] = (unsigned char)(colorf[1] * 255);
		_color[2] = (unsigned char)(colorf[2] * 255);
		_color[3] = (unsigned char)(colorf[3] * 255);
	}

	void GetColor(int& _r, int& _g, int& _b, int& _a) const {
		_r = _color[0];
		_g = _color[1];
		_b = _color[2];
		_a = _color[3];
	}

	void SetRawColor(int color32) {
		*((int*)this) = color32;
	}

	int GetRawColor() const {
		return *((int*)this);
	}

	int GetD3DColor() const {
		return ((int)((((_color[3]) & 0xff) << 24) | (((_color[0]) & 0xff) << 16) | (((_color[1]) & 0xff) << 8) | ((_color[2]) & 0xff)));
	}

	inline int r() const {
		return _color[0];
	}

	inline int g() const {
		return _color[1];
	}

	inline int b() const {
		return _color[2];
	}

	inline int a() const {
		return _color[3];
	}

	inline float rBase() const {
		return _color[0] / 255.0f;
	}

	inline float gBase() const {
		return _color[1] / 255.0f;
	}

	inline float bBase() const {
		return _color[2] / 255.0f;
	}

	inline float aBase() const {
		return _color[3] / 255.0f;
	}

	unsigned char& operator[](int index) {
		return _color[index];
	}

	const unsigned char& operator[](int index) const {
		return _color[index];
	}

	bool operator ==(const Color& rhs) const {
		return (*((int*)this) == *((int*)&rhs));
	}

	bool operator !=(const Color& rhs) const {
		return !(operator==(rhs));
	}

	Color& operator=(const Color& rhs) {
		SetRawColor(rhs.GetRawColor());
		return *this;
	}

	float* Base() {
		static float clr[3];

		clr[0] = _color[0] / 255.0f;
		clr[1] = _color[1] / 255.0f;
		clr[2] = _color[2] / 255.0f;
#pragma warning(suppress:4172)
		return &clr[0];
	}

	float* BaseAlpha() {
		float clr[4];

		clr[0] = _color[0] / 255.0f;
		clr[1] = _color[1] / 255.0f;
		clr[2] = _color[2] / 255.0f;
		clr[3] = _color[3] / 255.0f;
#pragma warning(suppress:4172)
#pragma warning(suppress:4616)
		return &clr[0];
	}

	float Hue() const {
		if (_color[0] == _color[1] && _color[1] == _color[2]) {
			return 0.0f;
		}

		float r = _color[0] / 255.0f;
		float g = _color[1] / 255.0f;
		float b = _color[2] / 255.0f;

		float max = r > g ? r : g > b ? g : b,
			min = r < g ? r : g < b ? g : b;
		float delta = max - min;
		float hue = 0.0f;

		if (r == max) {
			hue = (g - b) / delta;
		} else if (g == max) {
			hue = 2 + (b - r) / delta;
		} else if (b == max) {
			hue = 4 + (r - g) / delta;
		}
		hue *= 60;

		if (hue < 0.0f) {
			hue += 360.0f;
		}
		return hue;
	}

	float Saturation() const {
		float r = _color[0] / 255.0f;
		float g = _color[1] / 255.0f;
		float b = _color[2] / 255.0f;

		float max = r > g ? r : g > b ? g : b,
			min = r < g ? r : g < b ? g : b;
		float l, s = 0;

		if (max != min) {
			l = (max + min) / 2;
			if (l <= 0.5f)
				s = (max - min) / (max + min);
			else
				s = (max - min) / (2 - max - min);
		}
		return s;
	}

	float Brightness() const {
		float r = _color[0] / 255.0f;
		float g = _color[1] / 255.0f;
		float b = _color[2] / 255.0f;

		float max = r > g ? r : g > b ? g : b,
			min = r < g ? r : g < b ? g : b;
		return (max + min) / 2;
	}

	static Color FromHSB(float hue, float saturation, float brightness) {
		float h = hue == 1.0f ? 0 : hue * 6.0f;
		float f = h - (int)h;
		float p = brightness * (1.0f - saturation);
		float q = brightness * (1.0f - saturation * f);
		float t = brightness * (1.0f - (saturation * (1.0f - f)));

		if (h < 1) {
			return Color(
				(unsigned char)(brightness * 255),
				(unsigned char)(t * 255),
				(unsigned char)(p * 255)
			);
		} else if (h < 2) {
			return Color(
				(unsigned char)(q * 255),
				(unsigned char)(brightness * 255),
				(unsigned char)(p * 255)
			);
		} else if (h < 3) {
			return Color(
				(unsigned char)(p * 255),
				(unsigned char)(brightness * 255),
				(unsigned char)(t * 255)
			);
		} else if (h < 4) {
			return Color(
				(unsigned char)(p * 255),
				(unsigned char)(q * 255),
				(unsigned char)(brightness * 255)
			);
		} else if (h < 5) {
			return Color(
				(unsigned char)(t * 255),
				(unsigned char)(p * 255),
				(unsigned char)(brightness * 255)
			);
		} else {
			return Color(
				(unsigned char)(brightness * 255),
				(unsigned char)(p * 255),
				(unsigned char)(q * 255)
			);
		}
	}

	static Color FromHSBA(float hue, float saturation, float brightness, float alpha) {
		Color c = FromHSB(hue, saturation, brightness);
		c[3] = (unsigned char)(alpha * 255);
		return c;
	}

	static Color Red() {
		return Color(255, 0, 0);
	}

	static Color Green() {
		return Color(0, 255, 0);
	}

	static Color Blue() {
		return Color(0, 0, 255);
	}

	static Color LightBlue() {
		return Color(100, 100, 255);
	}

	static Color Grey() {
		return Color(128, 128, 128);
	}

	static Color DarkGrey() {
		return Color(45, 45, 45);
	}

	static Color Black() {
		return Color(0, 0, 0);
	}

	static Color White() {
		return Color(255, 255, 255);
	}

	static Color Purple() {
		return Color(220, 0, 220);
	}

	//Menu
	static Color Background() {
		return Color(55, 55, 55);
	}

	static Color FrameBorder() {
		return Color(80, 80, 80);
	}

	static Color MainText() {
		return Color(230, 230, 230);
	}

	static Color HeaderText() {
		return Color(49, 124, 230);
	}

	static Color CurrentTab() {
		return Color(55, 55, 55);
	}

	static Color Tabs() {
		return Color(23, 23, 23);
	}

	static Color Highlight() {
		return Color(49, 124, 230);
	}

	static Color ElementBorder() {
		return Color(0, 0, 0);
	}

	static Color SliderScroll() {
		return Color(78, 143, 230);
	}

};

class ICVar : public IAppSystem {
public:
	virtual void func10() = 0;
	virtual void RegisterConCommand(ConVar* pCommandBase) = 0;
	virtual void UnregisterConCommand(ConVar* pCommandBase) = 0;
	virtual void func13() = 0;
	virtual ConVar* FindVar(const char* var_name) = 0;
	virtual void func15() = 0;
	virtual void func16() = 0;
	virtual void func17() = 0;
	virtual void func18() = 0;
	virtual void func19() = 0;
	virtual void func20() = 0;

	void const ConsoleColorPrintf(const Color& clr, const char* pFormat, ...) {
		typedef void(__cdecl* OriginalFn)(void*, const Color&, const char*, ...);

		if (pFormat == nullptr)
			return;

		char buf[8192];

		va_list list;
		va_start(list, pFormat);
		_vsnprintf(buf, sizeof(buf) - 1, pFormat, list);
		va_end(list);
		buf[sizeof(buf) - 1] = 0;

		GetVFunc<OriginalFn>(this, 25)(this, clr, buf, list);
	}

	char pad_0x0000[0x4]; //0x0000
	ConVar* pNext; //0x0004 
	__int32 bRegistered; //0x0008 
	char* pszName; //0x000C 
	char* pszHelpString; //0x0010 
	__int32 nFlags; //0x0014 
	char pad_0x0018[0x4]; //0x0018
	ConVar* pParent; //0x001C 
	char* pszDefaultValue; //0x0020 
	char* strString; //0x0024 
	__int32 StringLength; //0x0028 
	float fValue; //0x002C 
	__int32 nValue; //0x0030 
	__int32 bHasMin; //0x0034 
	float fMinVal; //0x0038 
	__int32 bHasMax; //0x003C 
	float fMaxVal; //0x0040 
	void* fnChangeCallback; //0x0044 
};

class ConVar {
public:
	inline void SetValue(const char* value) {
		typedef void(__thiscall* OriginalFn)(void*, const char*);
		return  GetVFunc<OriginalFn>(this, 14)(this, value);
	}

	inline void SetValue(float value) {
		typedef void(__thiscall* OriginalFn)(void*, float);
		return GetVFunc<OriginalFn>(this, 15)(this, value);
	}

	inline void SetValue(int value) {
		typedef void(__thiscall* OriginalFn)(void*, int);
		return GetVFunc<OriginalFn>(this, 16)(this, value);
	}
	float GetFloat(void) const {
		auto temp = *(int*)(&fValue);
		auto temp_result = (int)(temp ^ (DWORD)this);
		return *(float*)(&temp_result);
	}

	char pad_0x0000[0x4]; //0x0000
	ConVar* pNext; //0x0004 
	__int32 bRegistered; //0x0008 
	char* pszName; //0x000C 
	char* pszHelpString; //0x0010 
	__int32 nFlags; //0x0014 
	char pad_0x0018[0x4]; //0x0018
	ConVar* pParent; //0x001C 
	char* pszDefaultValue; //0x0020 
	char* strString; //0x0024 
	__int32 StringLength; //0x0028 
	float fValue; //0x002C 
	__int32 nValue; //0x0030 
	__int32 bHasMin; //0x0034 
	float fMinVal; //0x0038 
	__int32 bHasMax; //0x003C 
	float fMaxVal; //0x0040 
	void* fnChangeCallback; //0x0044 
};//Size=0x0048

class IEngine {
public:
	virtual int GetIntersectingSurfaces(const void* model, const int& vCenter, const float radius, const bool bOnlyVisibleSurfaces, void* pInfos, const int nMaxInfos) = 0;
	virtual int GetLightForPoint(const int& pos, bool bClamp) = 0;
	virtual void* TraceLineMaterialAndLighting(const int& start, const int& end, int& diffuseLightColor, int& baseColor) = 0;
	virtual const char* ParseFile(const char* data, char* token, int maxlen) = 0;
	virtual bool CopyFile(const char* source, const char* destination) = 0;
	virtual void GetScreenSize(int& width, int& height) = 0;
	virtual void ServerCmd(const char* szCmdString, bool bReliable = true) = 0;
	virtual void ClientCmd(const char* szCmdString) = 0;
	virtual bool GetPlayerInfo(int ent_num, void* pinfo) = 0; //8
	virtual int GetPlayerForUserID(int userID) = 0;
	virtual void* TextMessageGet(const char* pName) = 0; // 10
	virtual bool Con_IsVisible(void) = 0;
	virtual int GetLocalPlayer(void) = 0;
	virtual const void* LoadModel(const char* pName, bool bProp = false) = 0;
	virtual float GetLastTimeStamp(void) = 0;
	virtual void* GetSentence(void* pAudioSource) = 0; // 15
	virtual float GetSentenceLength(void* pAudioSource) = 0;
	virtual bool IsStreaming(void* pAudioSource) const = 0;
	virtual void GetViewAngles(int& va) = 0;
	virtual void SetViewAngles(int& va) = 0;
	virtual int GetMaxClients(void) = 0; // 20
	virtual const char* Key_LookupBinding(const char* pBinding) = 0;
	virtual const char* Key_BindingForKey(int& code) = 0;
	virtual void Key_SetBinding(int, char const*) = 0;
	virtual void StartKeyTrapMode(void) = 0;
	virtual bool CheckDoneKeyTrapping(int& code) = 0;
	virtual bool IsInGame(void) = 0;
	virtual bool IsConnected(void) = 0;
	virtual bool IsDrawingLoadingImage(void) = 0;
	virtual void HideLoadingPlaque(void) = 0;
	virtual void Con_NPrintf(int pos, const char* fmt, ...) = 0; // 30
	virtual void Con_NXPrintf(const struct con_nprint_s* info, const char* fmt, ...) = 0;
	virtual int IsBoxVisible(const int& mins, const int& maxs) = 0;
	virtual int IsBoxInViewCluster(const int& mins, const int& maxs) = 0;
	virtual bool CullBox(const int& mins, const int& maxs) = 0;
	virtual void Sound_ExtraUpdate(void) = 0;
	virtual const char* GetGameDirectory(void) = 0;
	virtual const int WorldToScreenMatrix() = 0;
	virtual const int WorldToViewMatrix() = 0;
	virtual int GameLumpVersion(int lumpId) const = 0;
	virtual int GameLumpSize(int lumpId) const = 0; // 40
	virtual bool LoadGameLump(int lumpId, void* pBuffer, int size) = 0;
	virtual int xd() const = 0;
	virtual int* GetBSPTreeQuery() = 0;
	virtual void LinearToGamma(float* linear, float* gamma) = 0;
	virtual float LightStyleValue(int style) = 0; // 45
	virtual void ComputeDynamicLighting(const int& pt, const int* pNormal, int& color) = 0;
	virtual void GetAmbientLightColor(int& color) = 0;
	virtual int GetDXSupportLevel() = 0;
	virtual bool SupportsHDR() = 0;
	virtual void Mat_Stub(int* pMatSys) = 0; // 50
	virtual void GetChapterName(char* pchBuff, int iMaxLength) = 0;
	virtual char const* GetLevelName(void) = 0;
	virtual char const* GetLevelNameShort(void) = 0;
	virtual char const* GetMapGroupName(void) = 0;
	virtual struct IVoiceTweak_s* GetVoiceTweakAPI(void) = 0;
	virtual void SetVoiceCasterID(unsigned int someint) = 0; // 56
	virtual void EngineStats_BeginFrame(void) = 0;
	virtual void EngineStats_EndFrame(void) = 0;
	virtual void FireEvents() = 0;
	virtual int GetLeavesArea(unsigned short* pLeaves, int nLeaves) = 0;
	virtual bool DoesBoxTouchAreaFrustum(const int& mins, const int& maxs, int iArea) = 0; // 60
	virtual int GetFrustumList(int** pList, int listMax) = 0;
	virtual bool ShouldUseAreaFrustum(int i) = 0;
	virtual void SetAudioState(const int& state) = 0;
	virtual int SentenceGroupPick(int groupIndex, char* name, int nameBufLen) = 0;
	virtual int SentenceGroupPickSequential(int groupIndex, char* name, int nameBufLen, int sentenceIndex, int reset) = 0;
	virtual int SentenceIndexFromName(const char* pSentenceName) = 0;
	virtual const char* SentenceNameFromIndex(int sentenceIndex) = 0;
	virtual int SentenceGroupIndexFromName(const char* pGroupName) = 0;
	virtual const char* SentenceGroupNameFromIndex(int groupIndex) = 0;
	virtual float SentenceLength(int sentenceIndex) = 0;
	virtual void ComputeLighting(const int& pt, const int* pNormal, bool bClamp, int& color, int* pBoxColors = NULL) = 0;
	virtual void ActivateOccluder(int nOccluderIndex, bool bActive) = 0;
	virtual bool IsOccluded(const int& vecAbsMins, const int& vecAbsMaxs) = 0; // 74
	virtual int GetOcclusionViewId(void) = 0;
	virtual void* SaveAllocMemory(size_t num, size_t size) = 0;
	virtual void SaveFreeMemory(void* pSaveMem) = 0;
	virtual int* GetNetChannelInfo(void) = 0;
	virtual void DebugDrawPhysCollide(const int* pCollide, int* pMaterial, const int& transform, const Color& color) = 0; //79
	virtual void CheckPoint(const char* pName) = 0; // 80
	virtual void DrawPortals() = 0;
	virtual bool IsPlayingDemo(void) = 0;
	virtual bool IsRecordingDemo(void) = 0;
	virtual bool IsPlayingTimeDemo(void) = 0;
	virtual int GetDemoRecordingTick(void) = 0;
	virtual int GetDemoPlaybackTick(void) = 0;
	virtual int GetDemoPlaybackStartTick(void) = 0;
	virtual float GetDemoPlaybackTimeScale(void) = 0;
	virtual int GetDemoPlaybackTotalTicks(void) = 0;
	virtual bool IsPaused(void) = 0; // 90
	virtual float GetTimescale(void) const = 0;
	virtual bool IsTakingScreenshot(void) = 0;
	virtual bool IsHLTV(void) = 0;
	virtual bool IsLevelMainMenuBackground(void) = 0;
	virtual void GetMainMenuBackgroundName(char* dest, int destlen) = 0;
	virtual void SetOcclusionParameters(const int /*OcclusionParams_t*/& params) = 0; // 96
	virtual void GetUILanguage(char* dest, int destlen) = 0;
	virtual int IsSkyboxVisibleFromPoint(const int& vecPoint) = 0;
	virtual const char* GetMapEntitiesString() = 0;
	virtual bool IsInEditMode(void) = 0; // 100
	virtual float GetScreenAspectRatio(int viewportWidth, int viewportHeight) = 0;
	virtual bool REMOVED_SteamRefreshLogin(const char* password, bool isSecure) = 0; // 100
	virtual bool REMOVED_SteamProcessCall(bool& finished) = 0;
	virtual unsigned int GetEngineBuildNumber() = 0; // engines build
	virtual const char* GetProductVersionString() = 0; // mods version number (steam.inf)
	virtual void GrabPreColorCorrectedFrame(int x, int y, int width, int height) = 0;
	virtual bool IsHammerRunning() const = 0;
	virtual void ExecuteClientCmd(const char* szCmdString) = 0; //108
	virtual bool MapHasHDRLighting(void) = 0;
	virtual bool MapHasLightMapAlphaData(void) = 0;
	virtual int GetAppID() = 0;
	virtual int GetLightForPointFast(const int& pos, bool bClamp) = 0;
	virtual void ClientCmd_Unrestricted1(char const*, int, bool);
	virtual void ClientCmd_Unrestricted(const char* szCmdString, const char* newFlag) = 0; // 114, new flag, quick testing shows setting 0 seems to work, haven't looked into it.
	//Forgot to add this line, but make sure to format all unrestricted calls now with an extra , 0
	//Ex:
	//	I::Engine->ClientCmd_Unrestricted( charenc( "cl_mouseenable 1" ) , 0);
	//	I::Engine->ClientCmd_Unrestricted( charenc( "crosshair 1" ) , 0);
	virtual void SetRestrictServerCommands(bool bRestrict) = 0;
	virtual void SetRestrictClientCommands(bool bRestrict) = 0;
	virtual void SetOverlayBindProxy(int iOverlayID, void* pBindProxy) = 0;
	virtual bool CopyFrameBufferToMaterial(const char* pMaterialName) = 0;
	virtual void ReadConfiguration(const int iController, const bool readDefault) = 0;
	virtual void SetAchievementMgr(void* pAchievementMgr) = 0;
	virtual void* GetAchievementMgr() = 0;
	virtual bool MapLoadFailed(void) = 0;
	virtual void SetMapLoadFailed(bool bState) = 0;
	virtual bool IsLowViolence() = 0;
	virtual const char* GetMostRecentSaveGame(void) = 0;
	virtual void SetMostRecentSaveGame(const char* lpszFilename) = 0;
	virtual void StartXboxExitingProcess() = 0;
	virtual bool IsSaveInProgress() = 0;
	virtual bool IsAutoSaveDangerousInProgress(void) = 0;
	virtual unsigned int OnStorageDeviceAttached(int iController) = 0;
	virtual void OnStorageDeviceDetached(int iController) = 0;
	virtual char* const GetSaveDirName(void) = 0;
	virtual void WriteScreenshot(const char* pFilename) = 0;
	virtual void ResetDemoInterpolation(void) = 0;
	virtual int GetActiveSplitScreenPlayerSlot() = 0;
	virtual int SetActiveSplitScreenPlayerSlot(int slot) = 0;
	virtual bool SetLocalPlayerIsResolvable(char const* pchContext, int nLine, bool bResolvable) = 0;
	virtual bool IsLocalPlayerResolvable() = 0;
	virtual int GetSplitScreenPlayer(int nSlot) = 0;
	virtual bool IsSplitScreenActive() = 0;
	virtual bool IsValidSplitScreenSlot(int nSlot) = 0;
	virtual int FirstValidSplitScreenSlot() = 0; // -1 == invalid
	virtual int NextValidSplitScreenSlot(int nPreviousSlot) = 0; // -1 == invalid
	virtual void* GetSinglePlayerSharedMemorySpace(const char* szName, int ent_num = (1 << 11)) = 0;
	virtual void ComputeLightingCube(const int& pt, bool bClamp, int* pBoxColors) = 0;
	virtual void RegisterDemoCustomDataCallback(const char* szCallbackSaveID, int pCallback) = 0;
	virtual void RecordDemoCustomData(int pCallback, const void* pData, size_t iDataLength) = 0;
	virtual void SetPitchScale(float flPitchScale) = 0;
	virtual float GetPitchScale(void) = 0;
	virtual bool LoadFilmmaker() = 0;
	virtual void UnloadFilmmaker() = 0;
	virtual void SetLeafFlag(int nLeafIndex, int nFlagBits) = 0;
	virtual void RecalculateBSPLeafFlags(void) = 0;
	virtual bool DSPGetCurrentDASRoomNew(void) = 0;
	virtual bool DSPGetCurrentDASRoomChanged(void) = 0;
	virtual bool DSPGetCurrentDASRoomSkyAbove(void) = 0;
	virtual float DSPGetCurrentDASRoomSkyPercent(void) = 0;
	virtual void SetMixGroupOfCurrentMixer(const char* szgroupname, const char* szparam, float val, int setMixerType) = 0;
	virtual int GetMixLayerIndex(const char* szmixlayername) = 0;
	virtual void SetMixLayerLevel(int index, float level) = 0;
	virtual int GetMixGroupIndex(char const* groupname) = 0;
	virtual void SetMixLayerTriggerFactor(int i1, int i2, float fl) = 0;
	virtual void SetMixLayerTriggerFactor(char const* char1, char const* char2, float fl) = 0;
	virtual bool IsCreatingReslist() = 0;
	virtual bool IsCreatingXboxReslist() = 0;
	virtual void SetTimescale(float flTimescale) = 0;
	virtual void SetGamestatsData(void* pGamestatsData) = 0;
	virtual void* GetGamestatsData() = 0;
	virtual void GetMouseDelta(int& dx, int& dy, bool b) = 0; // unknown
	virtual const char* Key_LookupBindingEx(const char* pBinding, int iUserId = -1, int iStartCount = 0, int iAllowJoystick = -1) = 0;
	virtual int Key_CodeForBinding(char const*, int, int, int) = 0;
	virtual void UpdateDAndELights(void) = 0;
	virtual int GetBugSubmissionCount() const = 0;
	virtual void ClearBugSubmissionCount() = 0;
	virtual bool DoesLevelContainWater() const = 0;
	virtual float GetServerSimulationFrameTime() const = 0;
	virtual void SolidMoved(class IClientEntity* pSolidEnt, class ICollideable* pSolidCollide, const int* pPrevAbsOrigin, bool accurateBboxTriggerChecks) = 0;
	virtual void TriggerMoved(class IClientEntity* pTriggerEnt, bool accurateBboxTriggerChecks) = 0;
	virtual void ComputeLeavesConnected(const int& vecOrigin, int nCount, const int* pLeafIndices, bool* pIsConnected) = 0;
	virtual bool IsInCommentaryMode(void) = 0;
	virtual void SetBlurFade(float amount) = 0;
	virtual bool IsTransitioningToLoad() = 0;
	virtual void SearchPathsChangedAfterInstall() = 0;
	virtual void ConfigureSystemLevel(int nCPULevel, int nGPULevel) = 0;
	virtual void SetConnectionPassword(char const* pchCurrentPW) = 0;
	virtual void* GetSteamAPIContext() = 0;
	virtual void SubmitStatRecord(char const* szMapName, unsigned int uiBlobVersion, unsigned int uiBlobSize, const void* pvBlob) = 0;
	virtual void ServerCmdKeyValues(void* pKeyValues) = 0; // 203
	virtual void SpherePaintSurface(const void* model, const int& location, unsigned char chr, float fl1, float fl2) = 0;
	virtual bool HasPaintmap(void) = 0;
	virtual void EnablePaintmapRender() = 0;
	//virtual void                TracePaintSurface( const model_t *model, const int& position, float radius, CUtlvec<Color>& surfColors ) = 0;
	virtual void SphereTracePaintSurface(const void* model, const int& position, const int& vec2, float radius, /*CUtlvec<unsigned char, CUtlMemory<unsigned char, int>>*/ int& utilVecShit) = 0;
	virtual void RemoveAllPaint() = 0;
	virtual void PaintAllSurfaces(unsigned char uchr) = 0;
	virtual void RemovePaint(const void* model) = 0;
	virtual bool IsActiveApp() = 0;
	virtual bool IsClientLocalToActiveServer() = 0;
	virtual void TickProgressBar() = 0;
	virtual void GetInputContext(int /*EngineInputContextId_t*/ id) = 0;
	virtual void GetStartupImage(char* filename, int size) = 0;
	virtual bool IsUsingLocalNetworkBackdoor(void) = 0;
	virtual void SaveGame(const char*, bool, char*, int, char*, int) = 0;
	virtual void GetGenericMemoryStats( /* GenericMemoryStat_t */ void**) = 0;
	virtual bool GameHasShutdownAndFlushedMemory(void) = 0;
	virtual int GetLastAcknowledgedCommand(void) = 0;
	virtual void FinishContainerWrites(int i) = 0;
	virtual void FinishAsyncSave(void) = 0;
	virtual int GetServerTick(void) = 0;
	virtual const char* GetModDirectory(void) = 0;
	virtual bool AudioLanguageChanged(void) = 0;
	virtual bool IsAutoSaveInProgress(void) = 0;
	virtual void StartLoadingScreenForCommand(const char* command) = 0;
	virtual void StartLoadingScreenForKeyValues(void* values) = 0;
	virtual void SOSSetOpvarFloat(const char*, float) = 0;
	virtual void SOSGetOpvarFloat(const char*, float&) = 0;
	virtual bool IsSubscribedMap(const char*, bool) = 0;
	virtual bool IsFeaturedMap(const char*, bool) = 0;
	virtual void GetDemoPlaybackParameters(void) = 0;
	virtual int GetClientVersion(void) = 0;
	virtual bool IsDemoSkipping(void) = 0;
	virtual void SetDemoImportantEventData(const void* values) = 0;
	virtual void ClearEvents(void) = 0;
	virtual int GetSafeZoneXMin(void) = 0;
	virtual bool IsVoiceRecording(void) = 0;
	virtual void ForceVoiceRecordOn(void) = 0;
	virtual bool IsReplay(void) = 0;
};

class IGameEvent {
public:

	virtual ~IGameEvent() = 0;
	virtual const char* GetName() const = 0;

	virtual bool  IsReliable() const = 0;
	virtual bool  IsLocal() const = 0;
	virtual bool  IsEmpty(const char* keyName = NULL) = 0;

	virtual bool  GetBool(const char* keyName = NULL, bool defaultValue = false) = 0;
	virtual int   GetInt(const char* keyName = NULL, int defaultValue = 0) = 0;
	virtual unsigned long GetUint64(const char* keyName = NULL, unsigned long defaultValue = 0) = 0;
	virtual float GetFloat(const char* keyName = NULL, float defaultValue = 0.0f) = 0;
	virtual const char* GetString(const char* keyName = NULL, const char* defaultValue = "") = 0;
	virtual const wchar_t* GetWString(const char* keyName, const wchar_t* defaultValue = L"") = 0;

	virtual void SetBool(const char* keyName, bool value) = 0;
	virtual void SetInt(const char* keyName, int value) = 0;
	virtual void SetUint64(const char* keyName, unsigned long value) = 0;
	virtual void SetFloat(const char* keyName, float value) = 0;
	virtual void SetString(const char* keyName, const char* value) = 0;
	virtual void SetWString(const char* keyName, const wchar_t* value) = 0;
};

class IGameEventListener {
public:
	virtual ~IGameEventListener() {}
	virtual void FireGameEvent(IGameEvent* Event) = 0;
	virtual int GetEventDebugID() { return 42; }
};

struct bf_write;
class bf_read {
public:
	uintptr_t base_address;
	uintptr_t cur_offset;

	bf_read(uintptr_t addr) {
		base_address = addr;
		cur_offset = 0;
	}

	void SetOffset(uintptr_t offset) {
		cur_offset = offset;
	}

	void Skip(uintptr_t length) {
		cur_offset += length;
	}

	int ReadByte() {
		auto val = *reinterpret_cast<char*>(base_address + cur_offset);
		++cur_offset;
		return val;
	}

	bool ReadBool() {
		auto val = *reinterpret_cast<bool*>(base_address + cur_offset);
		++cur_offset;
		return val;
	}

	std::string ReadString() {
		char buffer[256];
		auto str_length = *reinterpret_cast<char*>(base_address + cur_offset);
		++cur_offset;
		memcpy(buffer, reinterpret_cast<void*>(base_address + cur_offset), str_length > 255 ? 255 : str_length);
		buffer[str_length > 255 ? 255 : str_length] = '\0';
		cur_offset += str_length + 1;
		return std::string(buffer);
	}
};

class IGameEventManager {
public:
	virtual int __Unknown_1(int* dwUnknown) = 0;

	// load game event descriptions from a file eg "resource\gameevents.res"
	virtual int LoadEventsFromFile(const char* filename) = 0;

	// removes all and anything
	virtual void Reset() = 0;

	// adds a listener for a particular event
	virtual bool AddListener(IGameEventListener* listener, const char* name, bool bServerSide) = 0;

	// returns true if this listener is listens to given event
	virtual bool FindListener(IGameEventListener* listener, const char* name) = 0;

	// removes a listener 
	virtual int RemoveListener(IGameEventListener* listener) = 0;

	// create an event by name, but doesn't fire it. returns NULL is event is not
	// known or no listener is registered for it. bForce forces the creation even if no listener is active
	virtual IGameEvent* CreateEvent(const char* name, bool bForce, unsigned int dwUnknown) = 0;

	// fires a server event created earlier, if bDontBroadcast is set, event is not send to clients
	virtual bool FireEvent(IGameEvent* event, bool bDontBroadcast = false) = 0;

	// fires an event for the local client only, should be used only by client code
	virtual bool FireEventClientSide(IGameEvent* event) = 0;

	// create a new copy of this event, must be free later
	virtual IGameEvent* DuplicateEvent(IGameEvent* event) = 0;

	// if an event was created but not fired for some reason, it has to bee freed, same UnserializeEvent
	virtual void FreeEvent(IGameEvent* event) = 0;

	// write/read event to/from bitbuffer
	virtual bool SerializeEvent(IGameEvent* event, bf_write* buf) = 0;

	// create new KeyValues, must be deleted
	virtual IGameEvent* UnserializeEvent(bf_read* buf) = 0;
};

class chat_spam {
	class player_hurt_listener
		: public IGameEventListener {
	public:
		void start(IGameEventManager * GameEventManager) {
			GameEventManager->AddListener(this, _("round_start"), false);
		}
		void stop(IGameEventManager* GameEventManager) {
			GameEventManager->RemoveListener(this);
		}
		void FireGameEvent(IGameEvent* event) override {
			chat_spam::singleton()->on_fire_event(event);
		}
		int GetEventDebugID(void) override {
			return 0x2A;
		}
	};
public:
	static chat_spam* singleton() {
		static chat_spam* instance = new chat_spam();
		return instance;
	}
	void initialize(IGameEventManager* GameEventManager, IEngine* Engine) {
		this->engine = Engine;
		_listener.start(GameEventManager);
	}
	void Unload(IGameEventManager* GameEventManager) {
		_listener.stop(GameEventManager);
	}
	void on_fire_event(IGameEvent* event) {
		if (!strcmp(event->GetName(), _("round_start"))) {
			engine->ClientCmd_Unrestricted(_("say \"I'm using a CR4CK3D otc by nezu!\""), 0);
		}
	}
private:
	player_hurt_listener    _listener;
	IEngine* engine;
};